#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "esp_check.h"
#include "esp_codec_dev.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "i2s_codec_bsp";
#define AUDIO_CHANNELS 2
#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_FRAME_COUNT 256
#define AUDIO_TWO_PI 6.28318530717958647692f

static esp_codec_dev_sample_info_t sample_info(void)
{
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
        .channel = AUDIO_CHANNELS,
        .channel_mask = 0,
        .sample_rate = CONFIG_EXAMPLE_SAMPLE_RATE,
        .mclk_multiple = 256,
    };
    return fs;
}

static esp_err_t codec_dev_to_esp_err(int ret, const char *operation)
{
    if (ret == ESP_CODEC_DEV_OK) {
        return ESP_OK;
    }

    ESP_LOGE(TAG, "%s failed: %d", operation, ret);
    return ESP_FAIL;
}

static esp_err_t speaker_codec_init(esp_codec_dev_handle_t *speaker)
{
    ESP_LOGI(TAG, "Initializing ES8311 speaker codec through managed BSP");
    ESP_RETURN_ON_ERROR(bsp_audio_init(NULL), TAG, "bsp audio init failed");

    *speaker = bsp_audio_codec_speaker_init();
    ESP_RETURN_ON_FALSE(*speaker, ESP_FAIL, TAG, "speaker codec init failed");

    esp_codec_dev_sample_info_t fs = sample_info();
    ESP_RETURN_ON_ERROR(codec_dev_to_esp_err(esp_codec_dev_open(*speaker, &fs), "open speaker codec"), TAG, "open failed");
    ESP_RETURN_ON_ERROR(codec_dev_to_esp_err(esp_codec_dev_set_out_vol(*speaker, CONFIG_EXAMPLE_VOICE_VOLUME),
                                             "set speaker volume"),
                        TAG, "volume failed");

    return ESP_OK;
}

static void fill_sine_frame(int16_t *samples, float *phase)
{
    const float step = AUDIO_TWO_PI * CONFIG_EXAMPLE_TONE_HZ / CONFIG_EXAMPLE_SAMPLE_RATE;

    for (size_t i = 0; i < AUDIO_FRAME_COUNT; i++) {
        int16_t sample = (int16_t)(sinf(*phase) * 12000.0f);
        samples[i * 2] = sample;
        samples[i * 2 + 1] = sample;

        *phase += step;
        if (*phase >= AUDIO_TWO_PI) {
            *phase -= AUDIO_TWO_PI;
        }
    }
}

static void speaker_task(void *args)
{
    esp_codec_dev_handle_t speaker = (esp_codec_dev_handle_t)args;
    int16_t samples[AUDIO_FRAME_COUNT * AUDIO_CHANNELS];
    float phase = 0.0f;

    ESP_LOGI(TAG, "Playing %d Hz sine wave", CONFIG_EXAMPLE_TONE_HZ);
    while (true) {
        fill_sine_frame(samples, &phase);

        int ret = esp_codec_dev_write(speaker, samples, sizeof(samples));
        if (ret != ESP_CODEC_DEV_OK) {
            ESP_LOGE(TAG, "speaker write failed: %d", ret);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    esp_codec_dev_handle_t speaker = NULL;

    if (speaker_codec_init(&speaker) != ESP_OK) {
        ESP_LOGE(TAG, "Audio setup failed; leaving task idle instead of restarting");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    xTaskCreate(speaker_task, "speaker_task", 4096, speaker, 5, NULL);
}
