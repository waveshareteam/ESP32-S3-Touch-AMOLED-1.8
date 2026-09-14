#pragma once

#include <atomic>

#include "systems/phone/esp_brookesia_phone_app.hpp"
#include "esp_dsp.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace esp_brookesia::apps
{
    class SpecAnalyzer : public systems::phone::App
    {
    public:
        static SpecAnalyzer *requestInstance(bool use_status_bar = false, bool use_navigation_bar = false);
        ~SpecAnalyzer();

        using systems::phone::App::endRecordResource;
        using systems::phone::App::startRecordResource;

    protected:
        SpecAnalyzer(bool use_status_bar, bool use_navigation_bar);

        bool run(void) override;
        bool back(void) override;
        bool close(void) override;
        bool init(void) override;
        bool deinit(void) override;
        bool pause(void) override;
        bool resume(void) override;

    private:
        static SpecAnalyzer *_instance;

        enum class CaptureState : uint8_t {
            Idle,
            Starting,
            Running,
            Stopping,
            Error,
        };

        static constexpr uint16_t N_SAMPLES = 1024;
        // The board's ES8311 supplies one standard-I2S microphone stream.
        static constexpr uint16_t CAPTURE_CHANNELS = 1;
        static constexpr uint16_t MIC_COUNT = 1;
        static constexpr uint16_t STRIPE_COUNT = 48;
        // Leave a narrow bezel margin while using the square panel's full
        // horizontal range for both microphone spectra.
        static constexpr uint16_t CANVAS_WIDTH = BSP_LCD_H_RES - 32;
        static constexpr uint16_t CANVAS_HEIGHT = (BSP_LCD_V_RES >= 720) ? 360 : (BSP_LCD_V_RES / 2);

        lv_obj_t *_canvas;
        lv_obj_t *_mic_label;
        lv_timer_t *_timer;
        std::atomic<TaskHandle_t> _audio_task_handle;
        std::atomic<bool> _capture_requested;
        std::atomic<bool> _worker_exit;
        std::atomic<bool> _worker_stopped;
        std::atomic<bool> _codec_released;
        std::atomic<esp_err_t> _codec_release_result;
        std::atomic<bool> _audio_session_acquired;
        std::atomic<CaptureState> _capture_state;
        CaptureState _shown_capture_state;
        portMUX_TYPE _spectrum_mux;

        __attribute__((aligned(16))) int16_t _raw_data[N_SAMPLES * CAPTURE_CHANNELS];
        __attribute__((aligned(16))) float _audio_buffer[MIC_COUNT][N_SAMPLES];
        __attribute__((aligned(16))) float _wind[N_SAMPLES];                      // Shared Hann window
        __attribute__((aligned(16))) float _fft_buffer[MIC_COUNT][N_SAMPLES * 2]; // Complex FFT input
        __attribute__((aligned(16))) float _spectrum[MIC_COUNT][N_SAMPLES / 2];   // Spectrum in dB
        float _display_spectrum[MIC_COUNT][STRIPE_COUNT];                         // Mapped spectrum for bars
        float _peak[MIC_COUNT][STRIPE_COUNT];                                     // Peak marker position
        float _smooth_spectrum[MIC_COUNT][STRIPE_COUNT];                          // Audio-task-owned smoothing state
        float _published_spectrum[MIC_COUNT][STRIPE_COUNT];                       // Protected cross-task snapshot
        float _render_spectrum[MIC_COUNT][STRIPE_COUNT];                          // LVGL-task-owned render snapshot

        uint16_t _bar_colors[MIC_COUNT][STRIPE_COUNT][3];
        uint16_t _peak_colors[MIC_COUNT][STRIPE_COUNT];
        uint16_t *_draw_buf; // PSRAM-preferred RGB565 canvas draw buffer

        bool ensureAudioTask(void);
        bool stopAudioTask(TickType_t timeout_ticks);
        bool releaseCodec(CaptureState success_state);
        void destroyUi(void);
        static void audio_fft_task(void *pvParameters);
        static void timer_cb(lv_timer_t *timer);
    };
}
