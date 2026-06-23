#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/esp-bsp.h"
#include "lvgl.h"

static const char *TAG = "bsp_quickstart";

static lv_obj_t *s_heap_label;
static lv_obj_t *s_sd_label;
static lv_obj_t *s_brightness_label;
static char s_sd_status[96] = "SD card: not checked";

static lv_obj_t *example_active_screen(void)
{
#if LVGL_VERSION_MAJOR >= 9
    return lv_screen_active();
#else
    return lv_scr_act();
#endif
}

static void example_set_sd_status(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(s_sd_status, sizeof(s_sd_status), format, args);
    va_end(args);

    if (s_sd_label != NULL) {
        lv_label_set_text(s_sd_label, s_sd_status);
    }
}

static void example_update_status(lv_timer_t *timer)
{
    (void)timer;

    if (s_heap_label == NULL) {
        return;
    }

    char text[128];
    const uint32_t heap_kb = esp_get_free_heap_size() / 1024;
    const uint32_t psram_kb = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
    snprintf(text, sizeof(text), "Free heap: %lu KB\nFree PSRAM: %lu KB",
             (unsigned long)heap_kb, (unsigned long)psram_kb);
    lv_label_set_text(s_heap_label, text);
}

static void example_update_brightness_label(int32_t brightness)
{
    if (s_brightness_label == NULL) {
        return;
    }

    char text[32];
    snprintf(text, sizeof(text), "Brightness %ld%%", (long)brightness);
    lv_label_set_text(s_brightness_label, text);
}

static void example_probe_sdcard(void)
{
    example_set_sd_status("SD card: checking...");

    esp_err_t ret = bsp_sdcard_mount();
    if (ret != ESP_OK) {
        example_set_sd_status("SD card: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "SD card mount skipped or failed: %s", esp_err_to_name(ret));
        return;
    }

    FILE *file = fopen("/sdcard/bsp_quickstart.txt", "w");
    if (file == NULL) {
        example_set_sd_status("SD card: mounted, write failed");
        ESP_LOGW(TAG, "SD card mounted, but marker file could not be written");
    } else {
        fprintf(file, "ESP32-S3-Touch-AMOLED-1.8 BSP quick-start OK\n");
        fclose(file);
        example_set_sd_status("SD card: mounted, marker written");
        ESP_LOGI(TAG, "Wrote /sdcard/bsp_quickstart.txt");
    }

    ret = bsp_sdcard_unmount();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD card unmount returned %s", esp_err_to_name(ret));
    }
}

static void example_brightness_event_cb(lv_event_t *event)
{
    lv_obj_t *slider = lv_event_get_target(event);
    const int32_t brightness = lv_slider_get_value(slider);

    esp_err_t ret = bsp_display_brightness_set(brightness);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Brightness update failed: %s", esp_err_to_name(ret));
        return;
    }

    example_update_brightness_label(brightness);
}

static void example_sd_button_event_cb(lv_event_t *event)
{
    (void)event;
    example_probe_sdcard();
}

static lv_obj_t *example_make_card(lv_obj_t *parent, int y, int height)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 328, height);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x182433), LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(0x2f9bff), LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

static void example_create_dashboard(void)
{
    lv_obj_t *screen = example_active_screen();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x08111f), LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "ESP32-S3 AMOLED BSP");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "Managed component quick start");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xa9c7df), LV_PART_MAIN);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 58);

    lv_obj_t *system_card = example_make_card(screen, 96, 132);
    lv_obj_t *system_title = lv_label_create(system_card);
    lv_label_set_text(system_title, "Runtime");
    lv_obj_set_style_text_color(system_title, lv_color_hex(0x8bd3ff), LV_PART_MAIN);
    lv_obj_align(system_title, LV_ALIGN_TOP_LEFT, 0, 0);

    s_heap_label = lv_label_create(system_card);
    lv_obj_set_style_text_color(s_heap_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(s_heap_label, LV_ALIGN_TOP_LEFT, 0, 34);
    example_update_status(NULL);

    lv_obj_t *brightness_slider = lv_slider_create(system_card);
    lv_obj_set_size(brightness_slider, 190, 14);
    lv_obj_align(brightness_slider, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_slider_set_range(brightness_slider, 10, 100);
    lv_slider_set_value(brightness_slider, 80, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, example_brightness_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_brightness_label = lv_label_create(system_card);
    lv_obj_set_style_text_color(s_brightness_label, lv_color_hex(0xdce9f5), LV_PART_MAIN);
    lv_obj_align_to(s_brightness_label, brightness_slider, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    example_update_brightness_label(80);

    lv_obj_t *storage_card = example_make_card(screen, 252, 144);
    lv_obj_t *storage_title = lv_label_create(storage_card);
    lv_label_set_text(storage_title, "Storage");
    lv_obj_set_style_text_color(storage_title, lv_color_hex(0x8bd3ff), LV_PART_MAIN);
    lv_obj_align(storage_title, LV_ALIGN_TOP_LEFT, 0, 0);

    s_sd_label = lv_label_create(storage_card);
    lv_label_set_text(s_sd_label, s_sd_status);
    lv_obj_set_style_text_color(s_sd_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(s_sd_label, LV_ALIGN_TOP_LEFT, 0, 36);

    lv_obj_t *refresh_button = lv_button_create(storage_card);
    lv_obj_set_size(refresh_button, 132, 38);
    lv_obj_align(refresh_button, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(refresh_button, lv_color_hex(0x2f9bff), LV_PART_MAIN);
    lv_obj_add_event_cb(refresh_button, example_sd_button_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *refresh_label = lv_label_create(refresh_button);
    lv_label_set_text(refresh_label, "Refresh SD");
    lv_obj_center(refresh_label);

    example_probe_sdcard();
    lv_timer_create(example_update_status, 1000, NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Waveshare ESP32-S3-Touch-AMOLED-1.8 BSP quick start");

    lv_display_t *display = bsp_display_start();
    if (display == NULL) {
        ESP_LOGE(TAG, "Display initialization failed");
        abort();
    }

    ESP_ERROR_CHECK(bsp_display_brightness_set(80));

    if (bsp_display_lock(1000)) {
        example_create_dashboard();
        bsp_display_unlock();
    } else {
        ESP_LOGE(TAG, "Could not lock LVGL to create dashboard");
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
