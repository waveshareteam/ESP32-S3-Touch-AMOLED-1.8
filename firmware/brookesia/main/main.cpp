/*
 * SPDX-FileCopyrightText: 2023-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2026 Waveshare
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <algorithm>
#include <ctime>
#include <new>

#include "bsp/esp-bsp.h"
#include "esp_brookesia.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "Clock.hpp"
#include "Drawpanel.hpp"
#include "Gallery.hpp"
#include "Gravitysphere.hpp"
#include "MusicPlayer.hpp"
#include "Recorder.hpp"
#include "Settings.hpp"
#include "SpecAnalyzer.hpp"
#include "VideoPlayer.hpp"
#include "XiaozhiApp.hpp"
#include "bsp_board_extra.h"
#include "chat_history.h"
#include "esp_brookesia_app_calculator.hpp"
#include "rtc_service.h"
#include "storage_service.h"
#include "system_status.hpp"

#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BrookesiaFirmware"
#include "esp_lib_utils.h"

using namespace esp_brookesia;
using namespace esp_brookesia::gui;
using namespace esp_brookesia::systems::phone;

namespace {

// esp_lvgl_port's own ESP_LVGL_PORT_INIT_CONFIG() default of 7168 bytes is sized
// for the plain LVGL peripheral demos.  The Brookesia phone launcher resolves a
// much deeper style/event chain while it builds and redraws the app grid, so the
// LVGL worker task overruns that default and takes the system down with a
// "***ERROR*** A stack overflow in task taskLVGL" panic.  Give it the same
// headroom the other Waveshare Brookesia firmwares use, and keep the stack in
// internal RAM as esp_lvgl_port does by default.
#define LVGL_PORT_INIT_CONFIG()                                         \
    {                                                                   \
        .task_priority = 4,                                             \
        .task_stack = 20 * 1024,                                        \
        .task_affinity = -1,                                            \
        .task_max_sleep_ms = 500,                                       \
        .task_stack_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_DEFAULT,    \
        .timer_period_ms = 5,                                           \
    }

esp_err_t init_nvs()
{
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        result = nvs_flash_erase();
        if (result == ESP_OK) {
            result = nvs_flash_init();
        }
    }
    return result;
}

void update_status_bar_clock(lv_timer_t *timer)
{
    auto *phone = static_cast<Phone *>(lv_timer_get_user_data(timer));
    if (phone == nullptr) {
        return;
    }

    time_t now = 0;
    struct tm time_info = {};
    time(&now);
    localtime_r(&now, &time_info);

    if (auto *status_bar = phone->getDisplay().getStatusBar(); status_bar != nullptr) {
        status_bar->setClock(time_info.tm_hour, time_info.tm_min);
    }
}

bool install_app(Phone *phone, systems::base::App *app, const char *name)
{
    if (phone == nullptr || app == nullptr) {
        ESP_UTILS_LOGE("Create %s failed", name);
        return false;
    }
    const int app_id = phone->getManager().installApp(app);
    if (app_id < systems::base::App::APP_ID_MIN) {
        ESP_UTILS_LOGE("Install %s failed", name);
        return false;
    }
    ESP_UTILS_LOGI("Installed %s (id: %d)", name, app_id);
    return true;
}

lv_obj_t *create_boot_splash(lv_display_t *display)
{
    if (display == nullptr) {
        return nullptr;
    }

    lv_obj_t *root = lv_obj_create(lv_display_get_layer_top(display));
    if (root == nullptr) {
        return nullptr;
    }

    const int width = lv_display_get_horizontal_resolution(display);
    const int height = lv_display_get_vertical_resolution(display);
    const int shortest_side = std::max(1, std::min(width, height));
    const int spinner_size = std::max(42, shortest_side / 6);
    const int title_offset = std::max(30, shortest_side / 10);

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, width, height);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x101417), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t *spinner = lv_spinner_create(root);
    lv_obj_t *title = lv_label_create(root);
    lv_obj_t *subtitle = lv_label_create(root);
    if (spinner == nullptr || title == nullptr || subtitle == nullptr) {
        lv_obj_delete(root);
        return nullptr;
    }

    lv_obj_set_size(spinner, spinner_size, spinner_size);
    lv_obj_set_style_arc_width(spinner, std::max(3, spinner_size / 14), LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, std::max(3, spinner_size / 14), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x303C43), LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x27C1A8), LV_PART_INDICATOR);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -title_offset * 2);

    lv_label_set_text(title, "ESP32-S3 Touch AMOLED 1.8");
    lv_obj_set_width(title, width - std::max(24, width / 12));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7F9FA), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -title_offset / 3);

    lv_label_set_text(subtitle, "Starting Brookesia");
    lv_obj_set_width(subtitle, width - std::max(24, width / 12));
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xA8B1B8), LV_PART_MAIN);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, title_offset);
    return root;
}

} // namespace

extern "C" void app_main(void)
{
    ESP_UTILS_LOGI("Starting ESP32-S3-Touch-AMOLED-1.8 Brookesia firmware");

    // Start through the explicit configuration entry point so the LVGL worker
    // task is created with the enlarged stack above.
    //
    // This BSP honors only lvgl_port_cfg here: bsp_display_lcd_init() builds its
    // own lvgl_port_display_cfg_t from the BSP Kconfig, so buffer_size,
    // double_buffer and flags are deliberately left out instead of pretending to
    // configure them.  The draw buffer geometry therefore comes from
    // CONFIG_BSP_DISPLAY_LVGL_BUF_HEIGHT (see sdkconfig.defaults), which must
    // stay small enough that the internal DMA bounce buffer esp_lcd needs for a
    // PSRAM draw buffer always fits (see the note in sdkconfig.defaults).
    bsp_display_cfg_t display_config = {
        .lvgl_port_cfg = LVGL_PORT_INIT_CONFIG(),
    };
    lv_display_t *display = bsp_display_start_with_config(&display_config);
    ESP_UTILS_CHECK_NULL_EXIT(display, "Start display failed");
    ESP_UTILS_CHECK_ERROR_EXIT(
        bsp_display_brightness_set(70), "Set initial display brightness failed"
    );

    LvLock::registerCallbacks([](int timeout_ms) {
        // The BSP uses zero as an indefinite wait. Brookesia's negative
        // default therefore maps directly to that board-level contract.
        return bsp_display_lock(timeout_ms < 0 ? 0U : static_cast<uint32_t>(timeout_ms));
    }, []() {
        bsp_display_unlock();
        return true;
    });

    if (!bsp_display_lock(1000)) {
        ESP_UTILS_LOGE("Lock display for boot splash failed");
        return;
    }
    lv_obj_t *boot_splash = create_boot_splash(display);
    if (boot_splash != nullptr) {
        lv_refr_now(display);
    }
    bsp_display_unlock();

    ESP_UTILS_CHECK_ERROR_EXIT(init_nvs(), "Initialize NVS failed");

    // Bring the Wi-Fi stack up here, while the internal heap is still one large
    // unbroken block. esp_wifi_init() allocates its static RX pool as a single
    // contiguous chunk of DMA-capable internal RAM; if the launcher and its
    // applications have already fragmented that heap, the driver silently falls
    // back to 2 RX buffers and Wi-Fi never starts (ESP_ERR_NO_MEM), which costs
    // the status monitor and the Xiaozhi voice application.
    const esp_err_t wifi_stack_result = brookesia::system_status::init_wifi_stack();
    if (wifi_stack_result != ESP_OK) {
        ESP_UTILS_LOGW(
            "Early Wi-Fi stack initialization failed: %s", esp_err_to_name(wifi_stack_result)
        );
    }

    const esp_err_t rtc_result = rtc_service_init();
    if (rtc_result != ESP_OK) {
        ESP_UTILS_LOGW("RTC synchronization skipped: %s", esp_err_to_name(rtc_result));
    }

    ESP_UTILS_CHECK_ERROR_EXIT(storage_service_init(), "Initialize SD storage service failed");
    const esp_err_t chat_history_result = chat_history_init();
    if (chat_history_result != ESP_OK) {
        ESP_UTILS_LOGW("AIChats history unavailable: %s", esp_err_to_name(chat_history_result));
    }

    const esp_err_t spiffs_result = bsp_spiffs_mount();
    if (spiffs_result != ESP_OK && spiffs_result != ESP_ERR_INVALID_STATE) {
        ESP_UTILS_LOGW("Internal storage unavailable: %s", esp_err_to_name(spiffs_result));
    }

    const esp_err_t audio_result = bsp_extra_codec_init();
    if (audio_result != ESP_OK) {
        ESP_UTILS_LOGW("Audio initialization skipped: %s", esp_err_to_name(audio_result));
    }

    Phone *phone = new (std::nothrow) Phone();
    ESP_UTILS_CHECK_NULL_EXIT(phone, "Create phone failed");
    StatusBar *status_bar = nullptr;

    {
        LvLockGuard gui_guard;
        ESP_UTILS_CHECK_FALSE_EXIT(phone->begin(), "Begin phone failed");

        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Calculator::requestInstance(), "Calculator"),
            "Install Calculator failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Drawpanel::requestInstance(), "Draw"), "Install Draw failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Gravitysphere::requestInstance(), "Gravitysphere"),
            "Install Gravitysphere failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Clock::requestInstance(), "Clock"), "Install Clock failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::SpecAnalyzer::requestInstance(), "SpecAnalyzer"),
            "Install SpecAnalyzer failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::MusicPlayer::requestInstance(), "Music"), "Install Music failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Gallery::requestInstance(), "Gallery"), "Install Gallery failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::VideoPlayer::requestInstance(), "Video"), "Install Video failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Recorder::requestInstance(), "Recorder"), "Install Recorder failed"
        );
        // Settings is an immersive full-screen page: it requests neither the
        // status bar nor the navigation bar, so Brookesia sets the status bar
        // visual mode to HIDE while the app is open.
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::Settings::requestInstance(false, false), "Settings"),
            "Install Settings failed"
        );
        ESP_UTILS_CHECK_FALSE_EXIT(
            install_app(phone, apps::XiaozhiApp::requestInstance(), "Xiaozhi"), "Install Xiaozhi failed"
        );

        if (lv_timer_create(update_status_bar_clock, 1000, phone) == nullptr) {
            ESP_UTILS_LOGW("Create status-bar clock timer failed");
        }
        status_bar = phone->getDisplay().getStatusBar();
        if (boot_splash != nullptr) {
            lv_obj_delete(boot_splash);
        }
        lv_obj_invalidate(lv_display_get_screen_active(display));
    }

    const esp_err_t status_result = brookesia::system_status::start(status_bar, 1000);
    if (status_result != ESP_OK) {
        ESP_UTILS_LOGW(
            "Battery/Wi-Fi status monitor unavailable: %s", esp_err_to_name(status_result)
        );
    }

    ESP_UTILS_LOGI("Brookesia firmware is ready");
}
