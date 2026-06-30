#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "bsp/esp-bsp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"

static const char *TAG = "sdmmc_bsp";

static esp_err_t write_file(const char *path, const char *data)
{
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for writing", path);
        return ESP_FAIL;
    }
    fputs(data, file);
    fclose(file);
    return ESP_OK;
}

static esp_err_t read_file(const char *path)
{
    char line[96] = {0};
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return ESP_FAIL;
    }
    fgets(line, sizeof(line), file);
    fclose(file);
    char *newline = strchr(line, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Mounting SD card through BSP");
    esp_err_t ret = bsp_sdcard_mount();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        ESP_LOGI(TAG, "Insert a FAT-formatted microSD card and reset the board.");
        return;
    }

    if (bsp_sdcard != NULL) {
        sdmmc_card_print_info(stdout, bsp_sdcard);
    }

    const char *hello_path = BSP_SD_MOUNT_POINT "/hello.txt";
    const char *renamed_path = BSP_SD_MOUNT_POINT "/foo.txt";
    const char *nihao_path = BSP_SD_MOUNT_POINT "/nihao.txt";

    ESP_ERROR_CHECK(write_file(hello_path, "Hello ESP32-S3-Touch-AMOLED-1.8!\n"));

    struct stat st;
    if (stat(renamed_path, &st) == 0) {
        unlink(renamed_path);
    }
    ESP_LOGI(TAG, "Renaming %s to %s", hello_path, renamed_path);
    ESP_ERROR_CHECK(rename(hello_path, renamed_path));
    ESP_ERROR_CHECK(read_file(renamed_path));

    ESP_ERROR_CHECK(write_file(nihao_path, "Nihao ESP32-S3-Touch-AMOLED-1.8!\n"));
    ESP_ERROR_CHECK(read_file(nihao_path));

    ESP_ERROR_CHECK(bsp_sdcard_unmount());
    ESP_LOGI(TAG, "Card unmounted");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
