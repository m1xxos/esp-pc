#pragma once

#include "driver/gpio.h"

// GPIO mapping on XIAO ESP32C6 (adjust to your wiring)
#define APP_GPIO_CMD_PWR        GPIO_NUM_1
#define APP_GPIO_CMD_RST        GPIO_NUM_2
#define APP_GPIO_STATE_PWR      GPIO_NUM_0
#define APP_GPIO_STATUS_LED     GPIO_NUM_15

// Output command line logic level
#define APP_CMD_ACTIVE_LEVEL    1

// Input sense line logic level (PWR_LED through isolator)
#define APP_STATE_ACTIVE_LEVEL  1

// Pulse timings
#define APP_PULSE_POWER_SHORT_MS      120
#define APP_PULSE_POWER_SOFT_OFF_MS   6000
#define APP_PULSE_RESET_MS            150

// Sequence timings
#define APP_REBOOT_INTERLOCK_MS       2000
#define APP_HARD_RESET_SETTLE_MS      3000

// State verification timeouts
#define APP_TIMEOUT_POWER_ON_MS       90000
#define APP_TIMEOUT_POWER_OFF_MS      30000
#define APP_TIMEOUT_REBOOT_MS         120000

// Polling and filtering
#define APP_STATE_POLL_INTERVAL_MS       250
#define APP_STATE_DEBOUNCE_SAMPLES       5
#define APP_STATE_DEBOUNCE_INTERVAL_MS   60

// Command safety
#define APP_COMMAND_COOLDOWN_MS       1500
#define APP_COMMAND_LOCK_TIMEOUT_MS   2000
