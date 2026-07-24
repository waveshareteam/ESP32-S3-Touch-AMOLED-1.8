#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "project_template";

void app_main(void)
{
    printf("\nESP32-S3-Touch-AMOLED-1.8 project template\n");
    printf("This project is intentionally small but already uses the managed BSP.\n");
    printf("Board I2C: SDA=%d SCL=%d\n", BSP_I2C_SDA, BSP_I2C_SCL);
    printf("Display: %dx%d\n", BSP_LCD_H_RES, BSP_LCD_V_RES);

    ESP_LOGI(TAG, "Managed BSP linked. Initialize only the board services used by the application.");

    while (true) {
        ESP_LOGI(TAG, "template running");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
