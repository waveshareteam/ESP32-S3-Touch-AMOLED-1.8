#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bsp/esp-bsp.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "display_colorbar";

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

static void draw_color_bars(esp_lcd_panel_handle_t panel)
{
    const uint16_t colors[] = {
        rgb565(255, 255, 255),
        rgb565(255, 255, 0),
        rgb565(0, 255, 255),
        rgb565(0, 255, 0),
        rgb565(255, 0, 255),
        rgb565(255, 0, 0),
        rgb565(0, 0, 255),
        rgb565(0, 0, 0),
    };
    const int bar_count = sizeof(colors) / sizeof(colors[0]);
    const int max_bar_width = (BSP_LCD_H_RES + bar_count - 1) / bar_count;
    uint16_t *line = heap_caps_malloc(BSP_LCD_V_RES * max_bar_width * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (line == NULL) {
        line = heap_caps_malloc(BSP_LCD_V_RES * max_bar_width * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    }
    if (line == NULL) {
        ESP_LOGE(TAG, "failed to allocate color buffer");
        return;
    }

    for (int bar = 0; bar < bar_count; bar++) {
        const int x0 = (BSP_LCD_H_RES * bar) / bar_count;
        const int x1 = (BSP_LCD_H_RES * (bar + 1)) / bar_count;
        const int width = x1 - x0;
        for (int i = 0; i < BSP_LCD_V_RES * width; i++) {
            line[i] = colors[bar];
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x0, 0, x1, BSP_LCD_V_RES, line));
    }
    free(line);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initialize AMOLED panel through BSP");
    const bsp_display_config_t display_config = {
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * BSP_LCD_BITS_PER_PIXEL / 8,
    };
    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_io_handle_t io = NULL;
    ESP_ERROR_CHECK(bsp_display_new(&display_config, &panel, &io));
    ESP_ERROR_CHECK(bsp_display_backlight_on());

    ESP_LOGI(TAG, "Drawing RGB565 color bars");
    draw_color_bars(panel);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
