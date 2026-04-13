#include "protocol_bridge.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "protocol_bridge";

static pc_power_state_t s_last_reported_state = PC_POWER_UNKNOWN;

static void state_watcher_task(void *arg)
{
    (void)arg;

    while (true) {
        pc_power_state_t state = pc_control_get_state();
        if (state != s_last_reported_state) {
            s_last_reported_state = state;
            protocol_bridge_report_state(state);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t protocol_bridge_start(void)
{
#if APP_PROFILE_MATTER
    ESP_LOGI(TAG, "profile=matter (placeholder transport is active)");
#elif APP_PROFILE_ZIGBEE
    ESP_LOGI(TAG, "profile=zigbee (placeholder transport is active)");
#else
    ESP_LOGI(TAG, "profile=none (local test mode)");
#endif

    BaseType_t ok = xTaskCreate(state_watcher_task, "state_watcher", 3072, NULL, 5, NULL);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t protocol_bridge_invoke_command(pc_command_t command, const char *origin)
{
    const char *src = (origin != NULL) ? origin : "unknown";
    ESP_LOGI(TAG, "invoke command=%s origin=%s", pc_control_command_to_str(command), src);

    esp_err_t err = pc_control_execute(command);
    protocol_bridge_report_state(pc_control_get_state());
    return err;
}

void protocol_bridge_report_state(pc_power_state_t state)
{
#if APP_PROFILE_MATTER
    ESP_LOGI(TAG, "matter report state=%s", pc_control_state_to_str(state));
#elif APP_PROFILE_ZIGBEE
    ESP_LOGI(TAG, "zigbee report state=%s", pc_control_state_to_str(state));
#else
    ESP_LOGI(TAG, "local report state=%s", pc_control_state_to_str(state));
#endif
}
