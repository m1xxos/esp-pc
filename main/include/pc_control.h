#pragma once

#include "esp_err.h"

typedef enum {
    PC_POWER_UNKNOWN = 0,
    PC_POWER_OFF,
    PC_POWER_ON,
} pc_power_state_t;

typedef enum {
    PC_CMD_POWER_ON = 0,
    PC_CMD_POWER_OFF,
    PC_CMD_REBOOT_SOFT,
    PC_CMD_REBOOT_HARD,
} pc_command_t;

esp_err_t pc_control_init(void);

pc_power_state_t pc_control_get_state(void);

esp_err_t pc_control_execute(pc_command_t command);

const char *pc_control_state_to_str(pc_power_state_t state);

const char *pc_control_command_to_str(pc_command_t command);
