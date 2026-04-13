#pragma once

#include "esp_err.h"
#include "pc_control.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t protocol_bridge_start(void);

esp_err_t protocol_bridge_invoke_command(pc_command_t command, const char *origin);

void protocol_bridge_report_state(pc_power_state_t state);

#ifdef __cplusplus
}
#endif
