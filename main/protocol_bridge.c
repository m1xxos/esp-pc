#include "protocol_bridge.h"

#include <string.h>

#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#if APP_PROFILE_MATTER
#include "matter_bridge.h"
#endif

static const char *TAG = "protocol_bridge";

static pc_power_state_t s_last_reported_state = PC_POWER_UNKNOWN;
static QueueHandle_t s_command_queue;
static bool s_started;

typedef struct {
    pc_command_t command;
    char origin[24];
} protocol_command_msg_t;

static void command_worker_task(void *arg)
{
    (void)arg;

    protocol_command_msg_t msg = {0};
    while (true) {
        if (xQueueReceive(s_command_queue, &msg, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        ESP_LOGI(TAG, "execute command=%s origin=%s", pc_control_command_to_str(msg.command), msg.origin);
        esp_err_t err = pc_control_execute(msg.command);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "command=%s failed: %s", pc_control_command_to_str(msg.command), esp_err_to_name(err));
        }

        protocol_bridge_report_state(pc_control_get_state());
    }
}

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
    if (s_started) {
        return ESP_OK;
    }

    s_command_queue = xQueueCreate(8, sizeof(protocol_command_msg_t));
    ESP_RETURN_ON_FALSE(s_command_queue != NULL, ESP_ERR_NO_MEM, TAG, "command queue alloc failed");

    BaseType_t cmd_ok = xTaskCreate(command_worker_task, "cmd_worker", 4096, NULL, 6, NULL);
    ESP_RETURN_ON_FALSE(cmd_ok == pdPASS, ESP_ERR_NO_MEM, TAG, "command worker create failed");

#if APP_PROFILE_MATTER
    ESP_LOGI(TAG, "profile=matter");
    ESP_RETURN_ON_ERROR(matter_bridge_start(), TAG, "matter bridge start failed");
#elif APP_PROFILE_ZIGBEE
    ESP_LOGI(TAG, "profile=zigbee (placeholder transport is active)");
#else
    ESP_LOGI(TAG, "profile=none (local test mode)");
#endif

    BaseType_t ok = xTaskCreate(state_watcher_task, "state_watcher", 3072, NULL, 5, NULL);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    s_started = true;

    return ESP_OK;
}

esp_err_t protocol_bridge_invoke_command(pc_command_t command, const char *origin)
{
    if (!s_started || s_command_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    protocol_command_msg_t msg = {
        .command = command,
        .origin = {0},
    };

    const char *src = (origin != NULL) ? origin : "unknown";
    strlcpy(msg.origin, src, sizeof(msg.origin));

    if (xQueueSend(s_command_queue, &msg, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "command queue full, drop command=%s", pc_control_command_to_str(command));
        return ESP_ERR_TIMEOUT;
    }

    return ESP_OK;
}

void protocol_bridge_report_state(pc_power_state_t state)
{
#if APP_PROFILE_MATTER
    matter_bridge_report_state(state);
    ESP_LOGI(TAG, "matter report state=%s", pc_control_state_to_str(state));
#elif APP_PROFILE_ZIGBEE
    ESP_LOGI(TAG, "zigbee report state=%s", pc_control_state_to_str(state));
#else
    ESP_LOGI(TAG, "local report state=%s", pc_control_state_to_str(state));
#endif
}
