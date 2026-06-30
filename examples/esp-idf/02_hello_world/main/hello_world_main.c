#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

void app_main(void)
{
    printf("Hello world from ESP32-S3-Touch-AMOLED-1.8!\n");

    esp_chip_info_t chip_info;
    uint32_t flash_size = 0;
    esp_chip_info(&chip_info);

    printf("This is %s with %d CPU core(s), ", CONFIG_IDF_TARGET, chip_info.cores);
    printf("%s%s%s, ",
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "Wi-Fi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "");
    printf("silicon revision v%d.%d, ", chip_info.revision / 100, chip_info.revision % 100);

    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("%" PRIu32 "MB flash\n", flash_size / (1024U * 1024U));
    } else {
        printf("flash size unknown\n");
    }

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
