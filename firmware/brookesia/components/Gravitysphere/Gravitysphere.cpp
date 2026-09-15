/*
 * SPDX-FileCopyrightText: 2023-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Gravitysphere.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>

#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "esp_brookesia.hpp"
#include "esp_err.h"
#include "esp_lib_utils.h"

#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:Gravitysphere"

LV_IMG_DECLARE(img_app_qmi8658ball);

namespace esp_brookesia::apps {

namespace {
constexpr char APP_NAME[] = "Gravitysphere";
}

Gravitysphere *Gravitysphere::_instance = nullptr;

Gravitysphere *Gravitysphere::requestInstance(bool use_status_bar, bool use_navigation_bar)
{
    if (_instance == nullptr) {
        _instance = new Gravitysphere(use_status_bar, use_navigation_bar);
    }
    return _instance;
}

Gravitysphere::Gravitysphere(bool use_status_bar, bool use_navigation_bar)
    : App(APP_NAME, &img_app_qmi8658ball, true, use_status_bar, use_navigation_bar)
{
}

Gravitysphere::~Gravitysphere()
{
    if (!stopWorker()) {
        ESP_UTILS_LOGE("Waiting for the Gravitysphere worker before destruction");
        while (_worker_task.load() != nullptr) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

bool Gravitysphere::init()
{
    // App installation is not hardware detection.  Defer the shared-bus probe
    // until the user opens the app so a temporarily unavailable IMU cannot
    // prevent the remaining Brookesia applications from being installed.
    ESP_UTILS_LOGI("QMI8658 will be initialized when the app opens");
    return true;
}

bool Gravitysphere::initImu()
{
    ESP_UTILS_LOGI("Initializing the QMI8658 motion sensor");

    if (_imu_initialized) {
        return true;
    }

    if (_imu.dev_handle != nullptr && !releaseImu()) {
        return false;
    }

    const i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    ESP_UTILS_CHECK_NULL_RETURN(bus, false, "Get board I2C bus failed");

    // The module address depends on the board revision/strap.  Probe before
    // registering the driver device so a failed high-address attempt cannot
    // leave a duplicate I2C device handle on the shared BSP bus.
    constexpr uint16_t candidate_addresses[] = {
        QMI8658_ADDRESS_HIGH,
        QMI8658_ADDRESS_LOW,
    };
    uint16_t imu_address = 0;
    for (uint16_t address : candidate_addresses) {
        if (i2c_master_probe(bus, address, 100) == ESP_OK) {
            imu_address = address;
            break;
        }
    }
    if (imu_address == 0) {
        ESP_UTILS_LOGE("QMI8658 not found at 0x%02x or 0x%02x",
                        QMI8658_ADDRESS_HIGH, QMI8658_ADDRESS_LOW);
        return false;
    }

    esp_err_t result = qmi8658_init(&_imu, bus, imu_address);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Initialize QMI8658 failed: %s", esp_err_to_name(result));
        (void)releaseImu();
        return false;
    }

    // The online driver enables both sensors at 1 kHz during qmi8658_init().
    // This app only consumes acceleration, so stop both immediately and enable
    // only the accelerometer while the app is active.
    result = qmi8658_enable_sensors(&_imu, QMI8658_DISABLE_ALL);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Disable QMI8658 sensors after initialization failed: %s",
                       esp_err_to_name(result));
        (void)releaseImu();
        return false;
    }

    result = qmi8658_set_accel_range(&_imu, QMI8658_ACCEL_RANGE_8G);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Set QMI8658 accelerometer range failed: %s", esp_err_to_name(result));
        (void)releaseImu();
        return false;
    }
    result = qmi8658_set_accel_odr(&_imu, QMI8658_ACCEL_ODR_62_5HZ);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Set QMI8658 accelerometer rate failed: %s", esp_err_to_name(result));
        (void)releaseImu();
        return false;
    }
    // qmi8658 v2.0.0 exposes unit selection as a void setter.
    qmi8658_set_accel_unit_mps2(&_imu, true);
    result = qmi8658_write_register(&_imu, QMI8658_CTRL5, 0x03);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Configure QMI8658 filtering failed: %s", esp_err_to_name(result));
        (void)releaseImu();
        return false;
    }

    _imu_initialized = true;
    return true;
}

bool Gravitysphere::deinit()
{
    if (!stopWorker()) {
        return false;
    }
    if (!releaseImu()) {
        return false;
    }
    releaseUi();
    return true;
}

bool Gravitysphere::run()
{
    ESP_UTILS_CHECK_FALSE_RETURN(_worker_task.load() == nullptr, false, "Worker is already running");

    ESP_UTILS_CHECK_FALSE_RETURN(refreshVisualGeometry(false), false, "Invalid visual area");

    const int center_x = _movement_center_x.load();
    const int center_y = _movement_center_y.load();
    _ball_x.store(center_x);
    _ball_y.store(center_y);

    const bool imu_ready = initImu();
    createUi();

    _calibration_progress.store(0);
    _paused.store(false);

    if (!imu_ready || !setImuActive(true)) {
        (void)releaseImu();
        _calibration_state.store(CalibrationState::FAILED);
        _recalibration_requested.store(false);
        updateUi();
        return true;
    }

    _calibration_state.store(CalibrationState::RUNNING);
    _recalibration_requested.store(true);
    if (!startWorker()) {
        (void)setImuActive(false);
        releaseUi();
        return false;
    }
    return true;
}

bool Gravitysphere::back()
{
    return notifyCoreClosed();
}

bool Gravitysphere::close()
{
    if (!stopWorker()) {
        return false;
    }
    if (!releaseImu()) {
        return false;
    }
    releaseUi();
    return true;
}

bool Gravitysphere::pause()
{
    _paused.store(true);
    if (_ui_timer != nullptr) {
        lv_timer_pause(_ui_timer);
    }
    if (!stopWorker()) {
        return false;
    }
    return setImuActive(false);
}

bool Gravitysphere::resume()
{
    if (!_imu_initialized && !initImu()) {
        _calibration_state.store(CalibrationState::FAILED);
        _paused.store(false);
        if (_ui_timer != nullptr) {
            lv_timer_reset(_ui_timer);
            lv_timer_resume(_ui_timer);
        }
        return true;
    }

    if (!setImuActive(true)) {
        (void)releaseImu();
        _calibration_state.store(CalibrationState::FAILED);
        _paused.store(false);
        if (_ui_timer != nullptr) {
            lv_timer_reset(_ui_timer);
            lv_timer_resume(_ui_timer);
        }
        return true;
    }

    _calibration_progress.store(0);
    _calibration_state.store(CalibrationState::RUNNING);
    _recalibration_requested.store(true);
    _paused.store(false);
    if (!startWorker()) {
        (void)setImuActive(false);
        return false;
    }
    if (_ui_timer != nullptr) {
        lv_timer_reset(_ui_timer);
        lv_timer_resume(_ui_timer);
    }
    return true;
}

void Gravitysphere::createUi()
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1020), LV_PART_MAIN);
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(screen, screenEventCallback, LV_EVENT_CLICKED, this);

    _movement_boundary = lv_obj_create(screen);
    lv_obj_remove_flag(_movement_boundary, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(_movement_boundary, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(_movement_boundary, ARENA_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_movement_boundary, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(_movement_boundary, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(_movement_boundary, lv_color_hex(0x52627F), LV_PART_MAIN);
    lv_obj_set_style_border_opa(_movement_boundary, LV_OPA_30, LV_PART_MAIN);

    _hint_label = lv_label_create(screen);
    lv_label_set_text(_hint_label, "Tap the screen\nto recalibrate");
    lv_obj_set_style_text_color(_hint_label, lv_color_hex(0xA9B7D0), LV_PART_MAIN);
    lv_obj_set_style_text_align(_hint_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(_hint_label, LV_OBJ_FLAG_HIDDEN);

    _calibration_bar = lv_bar_create(screen);
    lv_bar_set_range(_calibration_bar, 0, 100);
    lv_bar_set_value(_calibration_bar, 0, LV_ANIM_OFF);

    _calibration_label = lv_label_create(screen);
    lv_label_set_text(_calibration_label, "Keep the board level\nCalibrating 0%");
    lv_obj_set_style_text_color(_calibration_label, lv_color_hex(0xE7ECF5), LV_PART_MAIN);
    lv_obj_set_style_text_align(_calibration_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    _ball = lv_obj_create(screen);
    lv_obj_remove_flag(_ball, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(_ball, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(_ball, BALL_RADIUS * 2, BALL_RADIUS * 2);
    lv_obj_set_style_radius(_ball, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_ball, lv_color_hex(0xFF4F70), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(_ball, lv_color_hex(0xFFB14E), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(_ball, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_border_width(_ball, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(_ball, lv_color_hex(0xFF4F70), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(_ball, 18, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(_ball, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_flag(_ball, LV_OBJ_FLAG_HIDDEN);

    layoutUi();
    _ui_timer = lv_timer_create(uiTimerCallback, SENSOR_PERIOD_MS, this);
}

bool Gravitysphere::refreshVisualGeometry(bool request_recalibration)
{
    const lv_area_t visual_area = getVisualArea();
    const int width = lv_area_get_width(&visual_area);
    const int height = lv_area_get_height(&visual_area);
    // The ball travels the whole visual area. Its centre stays half a ball plus
    // the glow margin away from every edge so the ball and its shadow are never
    // clipped by the arena.
    const int margin = BALL_RADIUS + MOVEMENT_SAFE_MARGIN;
    if (width <= (margin * 2) || height <= (margin * 2)) {
        return false;
    }

    const bool changed = visual_area.x1 != _visual_area.x1 || visual_area.y1 != _visual_area.y1 ||
                         visual_area.x2 != _visual_area.x2 || visual_area.y2 != _visual_area.y2;
    if (!changed) {
        return true;
    }

    _visual_area = visual_area;
    _display_width = width;
    _display_height = height;
    _movement_center_x.store(width / 2);
    _movement_center_y.store(height / 2);
    _min_center_x.store(margin);
    _max_center_x.store(width - margin);
    _min_center_y.store(margin);
    _max_center_y.store(height - margin);

    if (_ball != nullptr) {
        _ball_x.store(_movement_center_x.load());
        _ball_y.store(_movement_center_y.load());
        layoutUi();
    }

    if (request_recalibration && _running.load()) {
        _calibration_progress.store(0);
        _calibration_state.store(CalibrationState::RUNNING);
        _recalibration_requested.store(true);
        const TaskHandle_t task = _worker_task.load();
        if (task != nullptr) {
            xTaskNotifyGive(task);
        }
    }

    return true;
}

void Gravitysphere::layoutUi()
{
    const int center_x = _movement_center_x.load();
    const int center_y = _movement_center_y.load();
    const int screen_radius = std::min(_display_width, _display_height) / 2;
    const int safe_half_extent = std::max(
        1,
        static_cast<int>((screen_radius - UI_SAFE_MARGIN) * 0.70710678F)
    );
    const int safe_width = std::max(1, (safe_half_extent * 2) - (UI_INNER_PADDING * 2));

    if (_movement_boundary != nullptr) {
        lv_obj_set_size(_movement_boundary, _display_width, _display_height);
        lv_obj_set_pos(_movement_boundary, 0, 0);
    }

    if (_hint_label != nullptr) {
        lv_obj_set_width(_hint_label, safe_width);
        lv_obj_set_pos(
            _hint_label,
            center_x - (safe_width / 2),
            center_y - safe_half_extent + UI_INNER_PADDING
        );
    }

    if (_calibration_bar != nullptr) {
        const int bar_width = std::max(1, std::min(280, safe_width - 24));
        lv_obj_set_size(_calibration_bar, bar_width, 18);
        lv_obj_set_pos(_calibration_bar, center_x - (bar_width / 2), center_y - 24);
    }

    if (_calibration_label != nullptr) {
        lv_obj_set_width(_calibration_label, safe_width);
        lv_obj_set_pos(_calibration_label, center_x - (safe_width / 2), center_y + 8);
    }

    if (_ball != nullptr) {
        lv_obj_set_pos(
            _ball,
            _ball_x.load() - BALL_RADIUS,
            _ball_y.load() - BALL_RADIUS
        );
    }
}

void Gravitysphere::screenEventCallback(lv_event_t *event)
{
    auto *app = static_cast<Gravitysphere *>(lv_event_get_user_data(event));
    if (app == nullptr) {
        return;
    }

    app->_recalibration_requested.store(true);
    const TaskHandle_t task = app->_worker_task.load();
    if (task != nullptr) {
        xTaskNotifyGive(task);
    }
}

void Gravitysphere::uiTimerCallback(lv_timer_t *timer)
{
    auto *app = static_cast<Gravitysphere *>(lv_timer_get_user_data(timer));
    if (app != nullptr) {
        app->updateUi();
    }
}

void Gravitysphere::updateUi()
{
    if (!refreshVisualGeometry(true)) {
        return;
    }

    if (_ball == nullptr || _calibration_label == nullptr || _calibration_bar == nullptr) {
        return;
    }

    const CalibrationState state = _calibration_state.load();
    const int progress = _calibration_progress.load();

    if (state != _displayed_state || progress != _displayed_progress) {
        switch (state) {
        case CalibrationState::RUNNING:
            lv_obj_add_flag(_ball, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_hint_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_calibration_bar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_calibration_label, LV_OBJ_FLAG_HIDDEN);
            lv_bar_set_value(_calibration_bar, progress, LV_ANIM_OFF);
            lv_label_set_text_fmt(
                _calibration_label, "Keep the board level\nCalibrating %d%%", progress
            );
            break;
        case CalibrationState::DONE:
            lv_obj_add_flag(_calibration_bar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_calibration_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_hint_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_ball, LV_OBJ_FLAG_HIDDEN);
            break;
        case CalibrationState::FAILED:
            lv_obj_add_flag(_ball, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_calibration_bar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_calibration_label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(
                _calibration_label,
                _imu_initialized ? "Sensor read failed\nRetrying..." :
                                   "QMI8658 unavailable\nClose and reopen to retry"
            );
            break;
        case CalibrationState::IDLE:
        default:
            break;
        }
        _displayed_state = state;
        _displayed_progress = progress;
    }

    if (state == CalibrationState::DONE) {
        // Brookesia already relocates the app screen to visual_area.x1/y1;
        // child coordinates are local to that resized screen.
        lv_obj_set_pos(
            _ball,
            _ball_x.load() - BALL_RADIUS,
            _ball_y.load() - BALL_RADIUS
        );
    }
}

void Gravitysphere::workerTask(void *arg)
{
    static_cast<Gravitysphere *>(arg)->workerLoop();
}

void Gravitysphere::workerLoop()
{
    int x = _movement_center_x.load();
    int y = _movement_center_y.load();

    while (_running.load()) {
        if (_paused.load()) {
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
            continue;
        }

        if (_recalibration_requested.exchange(false) ||
                _calibration_state.load() != CalibrationState::DONE) {
            x = _movement_center_x.load();
            y = _movement_center_y.load();
            _ball_x.store(x);
            _ball_y.store(y);

            if (!performLevelCalibration()) {
                if (!_running.load()) {
                    break;
                }
                _calibration_state.store(CalibrationState::FAILED);
                ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
                continue;
            }
        }

        bool data_ready = false;
        float accel_x = 0.0F;
        float accel_y = 0.0F;
        float accel_z = 0.0F;
        const esp_err_t ready_result = qmi8658_is_data_ready(&_imu, &data_ready);
        if (ready_result == ESP_OK && data_ready &&
                qmi8658_read_accel(&_imu, &accel_x, &accel_y, &accel_z) == ESP_OK) {
            accel_x -= _accel_bias_x;
            accel_y -= _accel_bias_y;
            if (std::fabs(accel_x) < CALIBRATION_DEADZONE) {
                accel_x = 0.0F;
            }
            if (std::fabs(accel_y) < CALIBRATION_DEADZONE) {
                accel_y = 0.0F;
            }

            x -= static_cast<int>(accel_y * ACCEL_SCALE_FACTOR);
            y += static_cast<int>(accel_x * ACCEL_SCALE_FACTOR);
            constrainPosition(x, y);
            _ball_x.store(x);
            _ball_y.store(y);
        }

        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }

    _worker_task.store(nullptr);
    vTaskDelete(nullptr);
}

bool Gravitysphere::performLevelCalibration()
{
    _calibration_state.store(CalibrationState::RUNNING);
    bool have_samples = false;
    float last_bias_x = 0.0F;
    float last_bias_y = 0.0F;

    for (int attempt = 0; attempt < CALIBRATION_MAX_RETRIES && _running.load(); ++attempt) {
        float sum_x = 0.0F;
        float sum_y = 0.0F;
        float min_x = INFINITY;
        float max_x = -INFINITY;
        float min_y = INFINITY;
        float max_y = -INFINITY;
        int valid_samples = 0;

        for (int sample = 0; sample < CALIBRATION_SAMPLES && _running.load(); ++sample) {
            while (_paused.load() && _running.load()) {
                ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
            }
            if (!_running.load()) {
                return false;
            }

            float accel_x = 0.0F;
            float accel_y = 0.0F;
            float accel_z = 0.0F;
            if (qmi8658_read_accel(&_imu, &accel_x, &accel_y, &accel_z) == ESP_OK) {
                sum_x += accel_x;
                sum_y += accel_y;
                min_x = std::min(min_x, accel_x);
                max_x = std::max(max_x, accel_x);
                min_y = std::min(min_y, accel_y);
                max_y = std::max(max_y, accel_y);
                ++valid_samples;
            }

            _calibration_progress.store(((sample + 1) * 100) / CALIBRATION_SAMPLES);
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
        }

        if (valid_samples == 0) {
            ESP_UTILS_LOGW("Calibration attempt %d read no QMI8658 samples", attempt + 1);
            continue;
        }

        have_samples = true;
        last_bias_x = sum_x / valid_samples;
        last_bias_y = sum_y / valid_samples;
        const float range_x = max_x - min_x;
        const float range_y = max_y - min_y;

        if (range_x <= 0.1F && range_y <= 0.1F) {
            _accel_bias_x = last_bias_x;
            _accel_bias_y = last_bias_y;
            _calibration_state.store(CalibrationState::DONE);
            ESP_UTILS_LOGI(
                "QMI8658 calibrated: X %.4f m/s^2, Y %.4f m/s^2",
                _accel_bias_x,
                _accel_bias_y
            );
            return true;
        }

        ESP_UTILS_LOGW(
            "Calibration attempt %d was unstable (X %.3f, Y %.3f)",
            attempt + 1,
            range_x,
            range_y
        );
        _calibration_progress.store(0);
    }

    if (have_samples && _running.load()) {
        // Match the board's Immersive_block example: remain usable with the
        // latest real sample set when a perfectly still calibration is unavailable.
        _accel_bias_x = last_bias_x;
        _accel_bias_y = last_bias_y;
        _calibration_state.store(CalibrationState::DONE);
        ESP_UTILS_LOGW("Using the latest QMI8658 calibration sample set");
        return true;
    }

    return false;
}

void Gravitysphere::constrainPosition(int &x, int &y) const
{
    // The arena is the whole screen, so each axis is clamped independently and
    // the ball slides along an edge instead of being pulled toward the centre.
    x = std::clamp(x, _min_center_x.load(), _max_center_x.load());
    y = std::clamp(y, _min_center_y.load(), _max_center_y.load());
}

void Gravitysphere::releaseUi()
{
    if (_ui_timer != nullptr) {
        lv_timer_delete(_ui_timer);
        _ui_timer = nullptr;
    }

    // The Brookesia application screen owns and deletes these LVGL objects.
    _movement_boundary = nullptr;
    _ball = nullptr;
    _hint_label = nullptr;
    _calibration_label = nullptr;
    _calibration_bar = nullptr;
    _displayed_state = CalibrationState::IDLE;
    _displayed_progress = -1;
}

bool Gravitysphere::releaseImu()
{
    if (_imu.dev_handle == nullptr) {
        _imu = {};
        _imu_initialized = false;
        return true;
    }

    const esp_err_t disable_result = qmi8658_enable_sensors(&_imu, QMI8658_DISABLE_ALL);
    if (disable_result != ESP_OK) {
        ESP_UTILS_LOGW("Disable QMI8658 sensors during release failed: %s",
                       esp_err_to_name(disable_result));
    }

    const esp_err_t result = i2c_master_bus_rm_device(_imu.dev_handle);
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("Remove QMI8658 I2C device failed: %s", esp_err_to_name(result));
        return false;
    }

    _imu = {};
    _imu_initialized = false;
    return true;
}

bool Gravitysphere::setImuActive(bool active)
{
    if (!_imu_initialized || _imu.dev_handle == nullptr) {
        return !active;
    }

    const esp_err_t result = qmi8658_enable_sensors(
        &_imu, active ? QMI8658_ENABLE_ACCEL : QMI8658_DISABLE_ALL
    );
    if (result != ESP_OK) {
        ESP_UTILS_LOGE("%s QMI8658 accelerometer failed: %s",
                       active ? "Enable" : "Disable", esp_err_to_name(result));
        return false;
    }
    return true;
}

bool Gravitysphere::startWorker()
{
    ESP_UTILS_CHECK_FALSE_RETURN(_worker_task.load() == nullptr, false, "Worker is already running");

    _running.store(true);
    TaskHandle_t task = nullptr;
    const BaseType_t created = xTaskCreatePinnedToCore(
        workerTask, "gravitysphere", 6144, this, 4, &task, 1
    );
    if (created != pdPASS) {
        _running.store(false);
        ESP_UTILS_LOGE("Create Gravitysphere worker failed");
        return false;
    }
    _worker_task.store(task);
    return true;
}

bool Gravitysphere::stopWorker()
{
    _running.store(false);
    TaskHandle_t task = _worker_task.load();
    if (task == nullptr) {
        return true;
    }

    xTaskNotifyGive(task);
    // A data-ready read and an accelerometer read can each consume the driver's
    // 1000 ms I2C timeout. Allow 5 s for the worker to
    // release the bus naturally; never delete it while a transaction may run.
    for (int i = 0; i < WORKER_STOP_POLL_COUNT && _worker_task.load() != nullptr; ++i) {
        vTaskDelay(pdMS_TO_TICKS(WORKER_STOP_POLL_PERIOD_MS));
    }

    if (_worker_task.load() != nullptr) {
        ESP_UTILS_LOGE("Gravitysphere worker did not exit within bounded I2C shutdown time");
        return false;
    }

    return true;
}

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(systems::base::App, Gravitysphere, APP_NAME, []() {
    return std::shared_ptr<Gravitysphere>(
        Gravitysphere::requestInstance(), [](Gravitysphere *) {}
    );
})

} // namespace esp_brookesia::apps
