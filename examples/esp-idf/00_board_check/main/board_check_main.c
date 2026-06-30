#include <inttypes.h>
#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "board_check";

static void print_chip_features(uint32_t features)
{
    printf("Features:");
    printf("%s", (features & CHIP_FEATURE_WIFI_BGN) ? " Wi-Fi" : "");
    printf("%s", (features & CHIP_FEATURE_BT) ? " BT" : "");
    printf("%s", (features & CHIP_FEATURE_BLE) ? " BLE" : "");
    printf("%s", (features & CHIP_FEATURE_EMB_FLASH) ? " embedded-flash" : "");
    printf("%s", (features & CHIP_FEATURE_EMB_PSRAM) ? " embedded-psram" : "");
    printf("\n");
}

void app_main(void)
{
    esp_chip_info_t chip_info;
    uint32_t flash_size = 0;

    esp_chip_info(&chip_info);

    printf("\n");
    printf("========================================\n");
    printf(" ESP32-S3-Touch-AMOLED-1.8 Board Check\n");
    printf("========================================\n");
    printf("IDF target: %s\n", CONFIG_IDF_TARGET);
    printf("CPU cores: %d\n", chip_info.cores);
    printf("Silicon revision: v%d.%d\n", chip_info.revision / 100, chip_info.revision % 100);
    print_chip_features(chip_info.features);

    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("Flash size: %" PRIu32 " MB\n", flash_size / (1024U * 1024U));
    } else {
        ESP_LOGW(TAG, "Failed to read flash size");
    }

    printf("BSP display: %s (%dx%d)\n", BSP_CAPS_DISPLAY ? "yes" : "no", BSP_LCD_H_RES, BSP_LCD_V_RES);
    printf("BSP touch: %s\n", BSP_CAPS_TOUCH ? "yes" : "no");
    printf("BSP audio speaker/mic: %s/%s\n", BSP_CAPS_AUDIO_SPEAKER ? "yes" : "no", BSP_CAPS_AUDIO_MIC ? "yes" : "no");
    printf("BSP SD card: %s\n", BSP_CAPS_SDCARD ? "yes" : "no");
    printf("I2C pins: SDA=%d SCL=%d\n", BSP_I2C_SDA, BSP_I2C_SCL);
    printf("SD pins: CMD=%d CLK=%d D0=%d\n", BSP_SD_CMD, BSP_SD_CLK, BSP_SD_D0);

    if (esp_psram_is_initialized()) {
        printf("PSRAM: initialized\n");
        printf("PSRAM free: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    } else {
        printf("PSRAM: not initialized or not enabled\n");
    }

    printf("Internal heap free: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    printf("Minimum free heap: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    printf("\n");
    printf("Next steps: run 00_bsp_quickstart, 08_i2c_tools, 09_sdmmc, 12_i2s_codec, 13_display_colorbar, or 14_lvgl_demo_v9.\n");
    printf("\n");

    while (true) {
        ESP_LOGI(TAG, "alive: internal=%u bytes psram=%u bytes",
                 heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                 heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
