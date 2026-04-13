#include "matter_bridge.h"

#if APP_PROFILE_MATTER

#include <stdint.h>

#include "esp_check.h"
#include "esp_log.h"
#include "esp_matter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol_bridge.h"

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

static const char *TAG = "matter_bridge";
static constexpr uint16_t INVALID_ENDPOINT = 0xFFFF;

static uint16_t s_power_endpoint_id = INVALID_ENDPOINT;
static uint16_t s_reboot_soft_endpoint_id = INVALID_ENDPOINT;
static uint16_t s_reboot_hard_endpoint_id = INVALID_ENDPOINT;
static volatile bool s_local_attribute_update = false;
static bool s_started = false;

static esp_err_t set_onoff_attribute(uint16_t endpoint_id, bool value)
{
    if (endpoint_id == INVALID_ENDPOINT) {
        return ESP_ERR_INVALID_ARG;
    }

    attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    ESP_RETURN_ON_FALSE(attribute != nullptr, ESP_ERR_NOT_FOUND, TAG, "on/off attribute not found");

    esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
    ESP_RETURN_ON_ERROR(attribute::get_val(attribute, &val), TAG, "attribute get_val failed");

    if (val.type != ESP_MATTER_VAL_TYPE_BOOLEAN) {
        return ESP_ERR_INVALID_STATE;
    }

    if (val.val.b == value) {
        return ESP_OK;
    }

    val.val.b = value;
    s_local_attribute_update = true;
    esp_err_t err = attribute::update(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id, &val);
    s_local_attribute_update = false;
    return err;
}

static void reset_momentary_endpoint_task(void *arg)
{
    uint16_t endpoint_id = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(arg));
    vTaskDelay(pdMS_TO_TICKS(350));
    ESP_ERROR_CHECK_WITHOUT_ABORT(set_onoff_attribute(endpoint_id, false));
    vTaskDelete(nullptr);
}

static void schedule_momentary_reset(uint16_t endpoint_id)
{
    BaseType_t ok = xTaskCreate(reset_momentary_endpoint_task, "mtr_momentary", 3072,
                                reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint_id)), 4, nullptr);
    if (ok != pdPASS) {
        ESP_LOGW(TAG, "cannot schedule momentary reset for endpoint=%u", endpoint_id);
    }
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    (void)arg;
    if (event) {
        ESP_LOGI(TAG, "event type=%d", static_cast<int>(event->Type));
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    (void)type;
    (void)endpoint_id;
    (void)effect_id;
    (void)effect_variant;
    (void)priv_data;
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    (void)priv_data;

    if (type != PRE_UPDATE) {
        return ESP_OK;
    }

    if (s_local_attribute_update) {
        return ESP_OK;
    }

    if (cluster_id != OnOff::Id || attribute_id != OnOff::Attributes::OnOff::Id || val == nullptr) {
        return ESP_OK;
    }

    if (val->type != ESP_MATTER_VAL_TYPE_BOOLEAN) {
        return ESP_OK;
    }

    const bool is_on = val->val.b;
    pc_command_t command = PC_CMD_POWER_ON;
    bool run_command = false;

    if (endpoint_id == s_power_endpoint_id) {
        command = is_on ? PC_CMD_POWER_ON : PC_CMD_POWER_OFF;
        run_command = true;
    } else if (endpoint_id == s_reboot_soft_endpoint_id) {
        if (is_on) {
            command = PC_CMD_REBOOT_SOFT;
            run_command = true;
            schedule_momentary_reset(endpoint_id);
        }
    } else if (endpoint_id == s_reboot_hard_endpoint_id) {
        if (is_on) {
            command = PC_CMD_REBOOT_HARD;
            run_command = true;
            schedule_momentary_reset(endpoint_id);
        }
    }

    if (!run_command) {
        return ESP_OK;
    }

    return protocol_bridge_invoke_command(command, "matter");
}

extern "C" esp_err_t matter_bridge_start(void)
{
    if (s_started) {
        return ESP_OK;
    }

    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ESP_RETURN_ON_FALSE(node != nullptr, ESP_FAIL, TAG, "node create failed");

    on_off_plugin_unit::config_t power_config;
    endpoint_t *power_ep = on_off_plugin_unit::create(node, &power_config, ENDPOINT_FLAG_NONE, nullptr);
    ESP_RETURN_ON_FALSE(power_ep != nullptr, ESP_FAIL, TAG, "power endpoint create failed");
    s_power_endpoint_id = endpoint::get_id(power_ep);

    on_off_plugin_unit::config_t reboot_soft_config;
    endpoint_t *reboot_soft_ep = on_off_plugin_unit::create(node, &reboot_soft_config, ENDPOINT_FLAG_NONE, nullptr);
    ESP_RETURN_ON_FALSE(reboot_soft_ep != nullptr, ESP_FAIL, TAG, "reboot_soft endpoint create failed");
    s_reboot_soft_endpoint_id = endpoint::get_id(reboot_soft_ep);

    on_off_plugin_unit::config_t reboot_hard_config;
    endpoint_t *reboot_hard_ep = on_off_plugin_unit::create(node, &reboot_hard_config, ENDPOINT_FLAG_NONE, nullptr);
    ESP_RETURN_ON_FALSE(reboot_hard_ep != nullptr, ESP_FAIL, TAG, "reboot_hard endpoint create failed");
    s_reboot_hard_endpoint_id = endpoint::get_id(reboot_hard_ep);

    ESP_RETURN_ON_ERROR(set_onoff_attribute(s_power_endpoint_id, false), TAG, "init power endpoint state failed");
    ESP_RETURN_ON_ERROR(set_onoff_attribute(s_reboot_soft_endpoint_id, false), TAG, "init reboot_soft endpoint state failed");
    ESP_RETURN_ON_ERROR(set_onoff_attribute(s_reboot_hard_endpoint_id, false), TAG, "init reboot_hard endpoint state failed");

    ESP_RETURN_ON_ERROR(esp_matter::start(app_event_cb), TAG, "esp_matter start failed");

    s_started = true;
    ESP_LOGI(TAG, "started endpoints: power=%u reboot_soft=%u reboot_hard=%u", s_power_endpoint_id,
             s_reboot_soft_endpoint_id, s_reboot_hard_endpoint_id);

    return ESP_OK;
}

extern "C" void matter_bridge_report_state(pc_power_state_t state)
{
    if (!s_started) {
        return;
    }

    if (state == PC_POWER_UNKNOWN) {
        return;
    }

    const bool is_on = (state == PC_POWER_ON);
    ESP_ERROR_CHECK_WITHOUT_ABORT(set_onoff_attribute(s_power_endpoint_id, is_on));
}

#else

extern "C" esp_err_t matter_bridge_start(void)
{
    return ESP_OK;
}

extern "C" void matter_bridge_report_state(pc_power_state_t state)
{
    (void)state;
}

#endif
