#include <inttypes.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "nvs_counter";
static const char *NVS_NAMESPACE = "demo";
static const char *NVS_KEY_BOOT_COUNT = "boot_count";

static void init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    nvs_handle_t nvs_handle;
    uint32_t boot_count = 0;

    init_nvs();
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

    esp_err_t err = nvs_get_u32(nvs_handle, NVS_KEY_BOOT_COUNT, &boot_count);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No saved boot count yet. Starting from zero.");
    } else {
        ESP_ERROR_CHECK(err);
    }

    boot_count++;
    ESP_ERROR_CHECK(nvs_set_u32(nvs_handle, NVS_KEY_BOOT_COUNT, boot_count));
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);

    printf("\nNVS Counter Demo\n");
    printf("Saved boot count: %" PRIu32 "\n", boot_count);
    printf("Press reset. The number should increase after each boot.\n");

    while (true) {
        ESP_LOGI(TAG, "running: boot_count=%" PRIu32, boot_count);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
