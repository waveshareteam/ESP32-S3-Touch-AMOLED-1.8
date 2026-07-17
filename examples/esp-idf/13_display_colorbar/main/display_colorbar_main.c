#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_lcd_co5300.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LCD_HOST SPI2_HOST
#define LCD_H_RES 368
#define LCD_V_RES 448
#define LCD_BITS_PER_PIXEL 16

#define LCD_CS GPIO_NUM_12
#define LCD_PCLK GPIO_NUM_11
#define LCD_DATA0 GPIO_NUM_4
#define LCD_DATA1 GPIO_NUM_5
#define LCD_DATA2 GPIO_NUM_6
#define LCD_DATA3 GPIO_NUM_7

#define I2C_PORT I2C_NUM_0
#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_14
#define CST816_ADDRESS 0x15
#define V2_PANEL_X_GAP 0x10

#define COLOR_BAR_COUNT 8
#define DRAW_STRIPE_HEIGHT 16

static const char *TAG = "display_colorbar";

static const co5300_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x53, (uint8_t[]){0x20}, 1, 0},
    {0x51, (uint8_t[]){0xFF}, 1, 0},
    {0x63, (uint8_t[]){0xFF}, 1, 0},
    {0x2A, (uint8_t[]){0x00, 0x00, 0x01, 0x6F}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xBF}, 4, 0},
    {0x11, NULL, 0, 100},
    {0x29, NULL, 0, 0},
};

static DMA_ATTR uint16_t color_bar_buffer[LCD_H_RES * DRAW_STRIPE_HEIGHT];

static bool detect_v2_board(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus = NULL;
    esp_err_t ret = i2c_new_master_bus(&bus_config, &bus);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Board revision probe unavailable: %s", esp_err_to_name(ret));
        return false;
    }

    const bool is_v2 = i2c_master_probe(bus, CST816_ADDRESS, 50) == ESP_OK;
    ESP_ERROR_CHECK(i2c_del_master_bus(bus));
    ESP_LOGI(TAG, "Detected %s board revision", is_v2 ? "V2" : "original");
    return is_v2;
}

static esp_lcd_panel_handle_t init_display(bool is_v2)
{
    const spi_bus_config_t bus_config = CO5300_PANEL_BUS_QSPI_CONFIG(
        LCD_PCLK, LCD_DATA0, LCD_DATA1, LCD_DATA2, LCD_DATA3, sizeof(color_bar_buffer));
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io = NULL;
    const esp_lcd_panel_io_spi_config_t io_config =
        CO5300_PANEL_IO_QSPI_CONFIG(LCD_CS, NULL, NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io));

    const co5300_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags.use_qspi_interface = 1,
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        .vendor_config = (void *)&vendor_config,
    };

    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_co5300(io, &panel_config, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel, is_v2 ? V2_PANEL_X_GAP : 0, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    return panel;
}

static void draw_color_bars(esp_lcd_panel_handle_t panel)
{
    static const uint16_t colors[COLOR_BAR_COUNT] = {
        0xFFFF,
        0xFFE0,
        0x07FF,
        0x07E0,
        0xF81F,
        0xF800,
        0x001F,
        0x0000,
    };

    for (int y = 0; y < DRAW_STRIPE_HEIGHT; y++) {
        for (int x = 0; x < LCD_H_RES; x++) {
            const uint16_t color = colors[(x * COLOR_BAR_COUNT) / LCD_H_RES];
            color_bar_buffer[y * LCD_H_RES + x] = SPI_SWAP_DATA_TX(color, LCD_BITS_PER_PIXEL);
        }
    }

    for (int y = 0; y < LCD_V_RES; y += DRAW_STRIPE_HEIGHT) {
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(
            panel, 0, y, LCD_H_RES, y + DRAW_STRIPE_HEIGHT, color_bar_buffer));
    }
}

void app_main(void)
{
    const bool is_v2 = detect_v2_board();

    ESP_LOGI(TAG, "Initialize CO5300 over QSPI");
    esp_lcd_panel_handle_t panel = init_display(is_v2);
    ESP_LOGI(TAG, "Drawing RGB565 color bars");
    draw_color_bars(panel);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
