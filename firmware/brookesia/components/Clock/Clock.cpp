/*
 * SPDX-FileCopyrightText: 2026 Waveshare
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Clock.hpp"

#include <algorithm>
#include <cstdio>
#include <ctime>

#include "esp_brookesia.hpp"
#include "esp_lib_utils.h"
#include "rtc_service.h"

#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:Clock"

namespace esp_brookesia::apps {

namespace {

constexpr char APP_NAME[] = "Clock";
constexpr uint32_t UPDATE_PERIOD_MS = 1000;
constexpr uint32_t COLOR_BACKGROUND = 0x07111F;
constexpr uint32_t COLOR_DIAL = 0x182A4A;
constexpr uint32_t COLOR_DIAL_BORDER = 0x4B75B3;
constexpr uint32_t COLOR_TIME = 0xF4F7FF;
constexpr uint32_t COLOR_DATE = 0xAAB8D1;
constexpr uint32_t COLOR_ERROR = 0xF3BA63;
constexpr time_t MINIMUM_VALID_EPOCH = 1577836800; // 2020-01-01 UTC

constexpr const char *WEEKDAYS[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

void makePassive(lv_obj_t *object)
{
    if (object == nullptr) {
        return;
    }
    lv_obj_remove_flag(object, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
}

} // namespace

Clock *Clock::_instance = nullptr;

Clock *Clock::requestInstance(bool use_status_bar, bool use_navigation_bar)
{
    if (_instance == nullptr) {
        _instance = new Clock(use_status_bar, use_navigation_bar);
    }
    return _instance;
}

Clock::Clock(bool use_status_bar, bool use_navigation_bar)
    : App(APP_NAME, nullptr, true, use_status_bar, use_navigation_bar)
{
}

bool Clock::init()
{
    // rtc_service is the sole owner of the PCF85063A I2C handle.  The clock
    // intentionally renders the synchronized system clock instead of adding
    // a competing device handle on the shared board bus.
    (void)rtc_service_init();
    return true;
}

bool Clock::deinit()
{
    releaseUi();
    return true;
}

bool Clock::run()
{
    _visual_area = getVisualArea();
    if (lv_area_get_width(&_visual_area) < 96 || lv_area_get_height(&_visual_area) < 96) {
        ESP_UTILS_LOGE("Visual area is too small");
        return false;
    }
    return createUi();
}

bool Clock::back()
{
    return notifyCoreClosed();
}

bool Clock::close()
{
    releaseUi();
    return true;
}

bool Clock::pause()
{
    if (_update_timer != nullptr) {
        lv_timer_pause(_update_timer);
    }
    return true;
}

bool Clock::resume()
{
    if (_update_timer != nullptr) {
        lv_timer_reset(_update_timer);
        lv_timer_resume(_update_timer);
    }
    updateUi();
    return true;
}

bool Clock::createUi()
{
    _screen = lv_screen_active();
    ESP_UTILS_CHECK_NULL_RETURN(_screen, false, "Get active screen failed");

    makePassive(_screen);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_screen, lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);

    _dial = lv_obj_create(_screen);
    makePassive(_dial);
    lv_obj_set_style_radius(_dial, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_dial, lv_color_hex(COLOR_DIAL), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_dial, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_color(_dial, lv_color_hex(COLOR_DIAL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(_dial, 2, LV_PART_MAIN);

    _time_label = lv_label_create(_screen);
    makePassive(_time_label);
    lv_obj_set_style_text_font(_time_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(_time_label, lv_color_hex(COLOR_TIME), LV_PART_MAIN);
    lv_obj_set_style_text_align(_time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    _date_label = lv_label_create(_screen);
    makePassive(_date_label);
    lv_obj_set_style_text_font(_date_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(_date_label, lv_color_hex(COLOR_DATE), LV_PART_MAIN);
    lv_obj_set_style_text_align(_date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    _status_label = lv_label_create(_screen);
    makePassive(_status_label);
    lv_obj_set_style_text_font(_status_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(_status_label, lv_color_hex(COLOR_ERROR), LV_PART_MAIN);
    lv_obj_set_style_text_align(_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    layoutUi();
    updateUi();
    _update_timer = lv_timer_create(updateTimerCallback, UPDATE_PERIOD_MS, this);
    return _update_timer != nullptr;
}

void Clock::layoutUi()
{
    if (_screen == nullptr) {
        return;
    }

    _visual_area = getVisualArea();
    const int width = lv_area_get_width(&_visual_area);
    const int height = lv_area_get_height(&_visual_area);
    const int diameter = std::max(1, std::min(width, height) - 52);
    const int center_x = width / 2;
    const int center_y = height / 2;
    const int text_width = std::max(1, std::min(300, diameter - 30));

    if (_dial != nullptr) {
        lv_obj_set_size(_dial, diameter, diameter);
        lv_obj_set_pos(_dial, center_x - (diameter / 2), center_y - (diameter / 2));
    }
    if (_time_label != nullptr) {
        lv_obj_set_width(_time_label, text_width);
        lv_obj_set_pos(_time_label, center_x - (text_width / 2), center_y - 46);
    }
    if (_date_label != nullptr) {
        lv_obj_set_width(_date_label, text_width);
        lv_obj_set_pos(_date_label, center_x - (text_width / 2), center_y + 2);
    }
    if (_status_label != nullptr) {
        lv_obj_set_width(_status_label, text_width);
        lv_obj_set_pos(_status_label, center_x - (text_width / 2), center_y + 46);
    }
}

void Clock::updateTimerCallback(lv_timer_t *timer)
{
    auto *app = static_cast<Clock *>(lv_timer_get_user_data(timer));
    if (app != nullptr) {
        app->updateUi();
    }
}

void Clock::updateUi()
{
    if (_time_label == nullptr || _date_label == nullptr || _status_label == nullptr) {
        return;
    }

    rtc_service_status_t rtc_status = {};
    const bool rtc_ready = rtc_service_get_status(&rtc_status) == ESP_OK &&
                           rtc_status.initialized && rtc_status.time_valid;
    const time_t now = time(nullptr);
    struct tm local_time = {};
    const bool system_time_ready = now >= MINIMUM_VALID_EPOCH &&
                                   localtime_r(&now, &local_time) != nullptr;

    if (!system_time_ready) {
        lv_label_set_text(_time_label, "--:--:--");
        lv_label_set_text(_date_label, "System clock");
        lv_label_set_text(_status_label, rtc_ready ? "Waiting for system clock" : "RTC unavailable");
        lv_obj_set_style_text_color(_status_label, lv_color_hex(COLOR_ERROR), LV_PART_MAIN);
        return;
    }

    char time_text[12] = {};
    char date_text[40] = {};
    std::snprintf(
        time_text, sizeof(time_text), "%02u:%02u:%02u",
        static_cast<unsigned int>(local_time.tm_hour), static_cast<unsigned int>(local_time.tm_min),
        static_cast<unsigned int>(local_time.tm_sec)
    );
    std::snprintf(
        date_text, sizeof(date_text), "%s  %04u-%02u-%02u",
        WEEKDAYS[local_time.tm_wday], static_cast<unsigned int>(local_time.tm_year + 1900),
        static_cast<unsigned int>(local_time.tm_mon + 1), static_cast<unsigned int>(local_time.tm_mday)
    );
    lv_label_set_text(_time_label, time_text);
    lv_label_set_text(_date_label, date_text);
    lv_label_set_text(_status_label, rtc_ready ? "PCF85063A synchronized" : "System time");
    lv_obj_set_style_text_color(_status_label,
                                lv_color_hex(rtc_ready ? COLOR_DATE : COLOR_ERROR), LV_PART_MAIN);
}

void Clock::releaseUi()
{
    if (_update_timer != nullptr) {
        lv_timer_delete(_update_timer);
        _update_timer = nullptr;
    }

    _screen = nullptr;
    _dial = nullptr;
    _time_label = nullptr;
    _date_label = nullptr;
    _status_label = nullptr;
}

} // namespace esp_brookesia::apps
