/*
 * SPDX-FileCopyrightText: 2026 Waveshare
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lvgl.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

namespace esp_brookesia::apps {

class Clock final : public systems::phone::App {
public:
    static Clock *requestInstance(bool use_status_bar = false, bool use_navigation_bar = false);
    ~Clock() override = default;

    using systems::phone::App::endRecordResource;
    using systems::phone::App::startRecordResource;

protected:
    Clock(bool use_status_bar, bool use_navigation_bar);

    bool init() override;
    bool deinit() override;
    bool run() override;
    bool back() override;
    bool close() override;
    bool pause() override;
    bool resume() override;

private:
    static Clock *_instance;
    static void updateTimerCallback(lv_timer_t *timer);

    bool createUi();
    void updateUi();
    void releaseUi();
    void layoutUi();

    lv_area_t _visual_area = {};
    lv_obj_t *_screen = nullptr;
    lv_obj_t *_time_label = nullptr;
    lv_obj_t *_date_label = nullptr;
    lv_obj_t *_status_label = nullptr;
    lv_obj_t *_dial = nullptr;
    lv_timer_t *_update_timer = nullptr;
};

} // namespace esp_brookesia::apps
