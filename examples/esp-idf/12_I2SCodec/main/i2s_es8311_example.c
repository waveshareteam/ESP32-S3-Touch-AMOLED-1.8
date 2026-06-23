#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bsp/esp-bsp.h"
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

static void check_codec_result(int ret, const char *operation)
{
    if (ret != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "%s failed: %d", operation, ret);
        abort();
    }
}

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

#if CONFIG_EXAMPLE_MODE_MUSIC
static void speaker_task(void *args)
{
    esp_codec_dev_handle_t speaker = (esp_codec_dev_handle_t)args;
    int16_t samples[AUDIO_FRAME_COUNT * AUDIO_CHANNELS];
    float phase = 0.0f;
    const float step = AUDIO_TWO_PI * CONFIG_EXAMPLE_TONE_HZ / CONFIG_EXAMPLE_SAMPLE_RATE;

    while (true) {
        for (size_t i = 0; i < AUDIO_FRAME_COUNT; i++) {
            int16_t sample = (int16_t)(sinf(phase) * 12000.0f);
            samples[i * 2] = sample;
            samples[i * 2 + 1] = sample;
            phase += step;
            if (phase >= AUDIO_TWO_PI) {
                phase -= AUDIO_TWO_PI;
            }
        }
        int ret = esp_codec_dev_write(speaker, samples, sizeof(samples));
        if (ret != ESP_CODEC_DEV_OK) {
            ESP_LOGE(TAG, "speaker write failed: %d", ret);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
#else
static void echo_task(void *args)
{
    esp_codec_dev_handle_t speaker = ((esp_codec_dev_handle_t *)args)[0];
    esp_codec_dev_handle_t microphone = ((esp_codec_dev_handle_t *)args)[1];
    int16_t samples[AUDIO_FRAME_COUNT * AUDIO_CHANNELS];

    while (true) {
        int ret = esp_codec_dev_read(microphone, samples, sizeof(samples));
        if (ret != ESP_CODEC_DEV_OK) {
            ESP_LOGE(TAG, "microphone read failed: %d", ret);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ret = esp_codec_dev_write(speaker, samples, sizeof(samples));
        if (ret != ESP_CODEC_DEV_OK) {
            ESP_LOGE(TAG, "speaker write failed: %d", ret);
        }
    }
}
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing ES8311 codec through managed BSP");
    ESP_ERROR_CHECK(bsp_audio_init(NULL));

    esp_codec_dev_sample_info_t fs = sample_info();
    esp_codec_dev_handle_t speaker = bsp_audio_codec_speaker_init();
    if (speaker == NULL) {
        ESP_LOGE(TAG, "speaker codec init failed");
        return;
    }
    check_codec_result(esp_codec_dev_open(speaker, &fs), "open speaker codec");
    check_codec_result(esp_codec_dev_set_out_vol(speaker, CONFIG_EXAMPLE_VOICE_VOLUME), "set speaker volume");

#if CONFIG_EXAMPLE_MODE_MUSIC
    ESP_LOGI(TAG, "Playing %d Hz sine wave", CONFIG_EXAMPLE_TONE_HZ);
    xTaskCreate(speaker_task, "speaker_task", 4096, speaker, 5, NULL);
#else
    esp_codec_dev_handle_t microphone = bsp_audio_codec_microphone_init();
    if (microphone == NULL) {
        ESP_LOGE(TAG, "microphone codec init failed");
        return;
    }
    check_codec_result(esp_codec_dev_open(microphone, &fs), "open microphone codec");
    static esp_codec_dev_handle_t codec_pair[2];
    codec_pair[0] = speaker;
    codec_pair[1] = microphone;
    ESP_LOGI(TAG, "Starting microphone echo");
    xTaskCreate(echo_task, "echo_task", 8192, codec_pair, 5, NULL);
#endif
}
