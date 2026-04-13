#include "uart_console.h"

#include <string.h>

#include "driver/uart.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pc_control.h"
#include "protocol_bridge.h"

static const char *TAG = "uart_console";
static const uart_port_t APP_UART_PORT = UART_NUM_0;

static void print_help(void)
{
    ESP_LOGI(TAG, "commands: help | state | on | off | reboot_soft | reboot_hard");
}

static void execute_line(const char *line)
{
    if (strcmp(line, "help") == 0) {
        print_help();
        return;
    }

    if (strcmp(line, "state") == 0) {
        ESP_LOGI(TAG, "state=%s", pc_control_state_to_str(pc_control_get_state()));
        return;
    }

    if (strcmp(line, "on") == 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(protocol_bridge_invoke_command(PC_CMD_POWER_ON, "uart"));
        return;
    }

    if (strcmp(line, "off") == 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(protocol_bridge_invoke_command(PC_CMD_POWER_OFF, "uart"));
        return;
    }

    if (strcmp(line, "reboot_soft") == 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(protocol_bridge_invoke_command(PC_CMD_REBOOT_SOFT, "uart"));
        return;
    }

    if (strcmp(line, "reboot_hard") == 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(protocol_bridge_invoke_command(PC_CMD_REBOOT_HARD, "uart"));
        return;
    }

    ESP_LOGW(TAG, "unknown command: %s", line);
}

static void console_task(void *arg)
{
    (void)arg;

    uint8_t ch = 0;
    char line[64];
    size_t len = 0;

    print_help();

    while (true) {
        int read = uart_read_bytes(APP_UART_PORT, &ch, 1, pdMS_TO_TICKS(200));
        if (read <= 0) {
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            if (len > 0) {
                line[len] = '\0';
                execute_line(line);
                len = 0;
            }
            continue;
        }

        if (len < sizeof(line) - 1) {
            line[len++] = (char)ch;
        } else {
            len = 0;
            ESP_LOGW(TAG, "input line too long, buffer reset");
        }
    }
}

esp_err_t uart_console_start(void)
{
    const uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_RETURN_ON_ERROR(uart_param_config(APP_UART_PORT, &cfg), TAG, "uart_param_config failed");
    ESP_RETURN_ON_ERROR(uart_driver_install(APP_UART_PORT, 1024, 0, 0, NULL, 0), TAG, "uart_driver_install failed");

    BaseType_t ok = xTaskCreate(console_task, "uart_console", 4096, NULL, 4, NULL);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
