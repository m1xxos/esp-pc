#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "pc_control.h"
#include "protocol_bridge.h"
#include "uart_console.h"

static const char *TAG = "app_main";

static esp_err_t init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "nvs erase failed");
        err = nvs_flash_init();
    }
    return err;
}

void app_main(void)
{
    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(pc_control_init());
    ESP_ERROR_CHECK(protocol_bridge_start());
    ESP_ERROR_CHECK(uart_console_start());

#if APP_PROFILE_MATTER
    ESP_LOGI(TAG, "booted with matter profile");
#elif APP_PROFILE_ZIGBEE
    ESP_LOGI(TAG, "booted with zigbee profile");
#else
    ESP_LOGI(TAG, "booted with local profile");
#endif

    ESP_LOGI(TAG, "ready: use serial commands help/state/on/off/reboot_soft/reboot_hard");
}
