#include "pc_control.h"

#include <stdbool.h>
#include <stdint.h>

#include "app_config.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "pc_control";

static SemaphoreHandle_t s_lock;
static int64_t s_last_command_us;
static bool s_initialized;

static inline int inactive_level(void)
{
    return APP_CMD_ACTIVE_LEVEL ? 0 : 1;
}

static pc_power_state_t sample_state_debounced(void)
{
    int on_votes = 0;

    for (int i = 0; i < APP_STATE_DEBOUNCE_SAMPLES; ++i) {
        int level = gpio_get_level(APP_GPIO_STATE_PWR);
        if (level == APP_STATE_ACTIVE_LEVEL) {
            ++on_votes;
        }
        vTaskDelay(pdMS_TO_TICKS(APP_STATE_DEBOUNCE_INTERVAL_MS));
    }

    if (on_votes == APP_STATE_DEBOUNCE_SAMPLES) {
        return PC_POWER_ON;
    }

    if (on_votes == 0) {
        return PC_POWER_OFF;
    }

    return PC_POWER_UNKNOWN;
}

static esp_err_t set_status_led(pc_power_state_t state)
{
    int level = (state == PC_POWER_ON) ? 1 : 0;
    return gpio_set_level(APP_GPIO_STATUS_LED, level);
}

static esp_err_t pulse_gpio(gpio_num_t pin, uint32_t duration_ms)
{
    ESP_RETURN_ON_ERROR(gpio_set_level(pin, APP_CMD_ACTIVE_LEVEL), TAG, "set active level failed");
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ESP_RETURN_ON_ERROR(gpio_set_level(pin, inactive_level()), TAG, "set inactive level failed");
    return ESP_OK;
}

static esp_err_t wait_for_state(pc_power_state_t target, uint32_t timeout_ms)
{
    const int64_t deadline = esp_timer_get_time() + ((int64_t)timeout_ms * 1000);

    while (esp_timer_get_time() < deadline) {
        pc_power_state_t state = sample_state_debounced();
        if (state == target) {
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(APP_STATE_POLL_INTERVAL_MS));
    }

    return ESP_ERR_TIMEOUT;
}

esp_err_t pc_control_init(void)
{
    gpio_config_t output_cfg = {
        .pin_bit_mask = (1ULL << APP_GPIO_CMD_PWR) | (1ULL << APP_GPIO_CMD_RST) | (1ULL << APP_GPIO_STATUS_LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config_t input_cfg = {
        .pin_bit_mask = (1ULL << APP_GPIO_STATE_PWR),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_RETURN_ON_ERROR(gpio_config(&output_cfg), TAG, "output gpio config failed");
    ESP_RETURN_ON_ERROR(gpio_config(&input_cfg), TAG, "input gpio config failed");

    ESP_RETURN_ON_ERROR(gpio_set_level(APP_GPIO_CMD_PWR, inactive_level()), TAG, "init pwr gpio failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(APP_GPIO_CMD_RST, inactive_level()), TAG, "init rst gpio failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(APP_GPIO_STATUS_LED, 0), TAG, "init led gpio failed");

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateMutex();
        if (s_lock == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }

    s_last_command_us = 0;
    s_initialized = true;

    pc_power_state_t state = pc_control_get_state();
    ESP_ERROR_CHECK_WITHOUT_ABORT(set_status_led(state));
    ESP_LOGI(TAG, "initialized, current state: %s", pc_control_state_to_str(state));

    return ESP_OK;
}

pc_power_state_t pc_control_get_state(void)
{
    if (!s_initialized) {
        return PC_POWER_UNKNOWN;
    }

    return sample_state_debounced();
}

esp_err_t pc_control_execute(pc_command_t command)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_lock, pdMS_TO_TICKS(APP_COMMAND_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t result = ESP_OK;
    const int64_t now = esp_timer_get_time();

    if ((now - s_last_command_us) < ((int64_t)APP_COMMAND_COOLDOWN_MS * 1000)) {
        ESP_LOGW(TAG, "command ignored due to cooldown");
        result = ESP_ERR_INVALID_STATE;
        goto done;
    }

    s_last_command_us = now;

    pc_power_state_t state = pc_control_get_state();
    ESP_LOGI(TAG, "exec command=%s state=%s", pc_control_command_to_str(command), pc_control_state_to_str(state));

    switch (command) {
    case PC_CMD_POWER_ON:
        if (state == PC_POWER_ON) {
            ESP_LOGI(TAG, "power on skipped, already on");
            break;
        }
        result = pulse_gpio(APP_GPIO_CMD_PWR, APP_PULSE_POWER_SHORT_MS);
        if (result == ESP_OK) {
            result = wait_for_state(PC_POWER_ON, APP_TIMEOUT_POWER_ON_MS);
        }
        break;

    case PC_CMD_POWER_OFF:
        if (state == PC_POWER_OFF) {
            ESP_LOGI(TAG, "power off skipped, already off");
            break;
        }
        result = pulse_gpio(APP_GPIO_CMD_PWR, APP_PULSE_POWER_SOFT_OFF_MS);
        if (result == ESP_OK) {
            result = wait_for_state(PC_POWER_OFF, APP_TIMEOUT_POWER_OFF_MS);
        }
        break;

    case PC_CMD_REBOOT_SOFT:
        if (state != PC_POWER_ON) {
            result = ESP_ERR_INVALID_STATE;
            break;
        }
        result = pulse_gpio(APP_GPIO_CMD_PWR, APP_PULSE_POWER_SOFT_OFF_MS);
        if (result != ESP_OK) {
            break;
        }
        result = wait_for_state(PC_POWER_OFF, APP_TIMEOUT_POWER_OFF_MS);
        if (result != ESP_OK) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(APP_REBOOT_INTERLOCK_MS));
        result = pulse_gpio(APP_GPIO_CMD_PWR, APP_PULSE_POWER_SHORT_MS);
        if (result == ESP_OK) {
            result = wait_for_state(PC_POWER_ON, APP_TIMEOUT_REBOOT_MS);
        }
        break;

    case PC_CMD_REBOOT_HARD:
        if (state != PC_POWER_ON) {
            result = ESP_ERR_INVALID_STATE;
            break;
        }
        result = pulse_gpio(APP_GPIO_CMD_RST, APP_PULSE_RESET_MS);
        if (result != ESP_OK) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(APP_HARD_RESET_SETTLE_MS));
        if (pc_control_get_state() != PC_POWER_ON) {
            result = ESP_ERR_TIMEOUT;
        }
        break;

    default:
        result = ESP_ERR_INVALID_ARG;
        break;
    }

done:
    pc_power_state_t final_state = pc_control_get_state();
    ESP_ERROR_CHECK_WITHOUT_ABORT(set_status_led(final_state));

    if (result != ESP_OK) {
        ESP_LOGW(TAG, "command result=%s", esp_err_to_name(result));
    }

    xSemaphoreGive(s_lock);
    return result;
}

const char *pc_control_state_to_str(pc_power_state_t state)
{
    switch (state) {
    case PC_POWER_OFF:
        return "off";
    case PC_POWER_ON:
        return "on";
    default:
        return "unknown";
    }
}

const char *pc_control_command_to_str(pc_command_t command)
{
    switch (command) {
    case PC_CMD_POWER_ON:
        return "power_on";
    case PC_CMD_POWER_OFF:
        return "power_off";
    case PC_CMD_REBOOT_SOFT:
        return "reboot_soft";
    case PC_CMD_REBOOT_HARD:
        return "reboot_hard";
    default:
        return "invalid";
    }
}
