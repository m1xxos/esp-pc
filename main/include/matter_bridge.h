#pragma once

#include "esp_err.h"
#include "pc_control.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t matter_bridge_start(void);

void matter_bridge_report_state(pc_power_state_t state);

#ifdef __cplusplus
}
#endif
