#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "esp_codec_dev_defaults.h"
#include "esp_check.h"
#include "esp_codec_dev.h"
#include "esp_err.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "i2s_codec_bsp";
#define AUDIO_CHANNELS 1
#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_FRAME_COUNT 256
#define AUDIO_TWO_PI 6.28318530717958647692f
#define V2_TOUCH_I2C_ADDRESS 0x15
#define TOUCH_PROBE_TIMEOUT_MS 50

static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

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

static bool board_is_v2(void)
{
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    esp_log_level_t i2c_log_level = esp_log_level_get("i2c.master");
    esp_log_level_set("i2c.master", ESP_LOG_NONE);
#endif

    const esp_err_t ret = i2c_master_probe(bsp_i2c_get_handle(), V2_TOUCH_I2C_ADDRESS, TOUCH_PROBE_TIMEOUT_MS);

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    esp_log_level_set("i2c.master", i2c_log_level);
#endif

    return ret == ESP_OK;
}

static esp_err_t i2s_driver_init(const audio_codec_data_if_t **data_if)
{
    ESP_LOGI(TAG, "Initializing I2S with BSP pins");
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(CONFIG_BSP_I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle), TAG, "create i2s channel failed");

    const i2s_std_config_t i2s_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = BSP_I2S_MCLK,
            .bclk = BSP_I2S_SCLK,
            .ws = BSP_I2S_LCLK,
            .dout = BSP_I2S_DOUT,
            .din = BSP_I2S_DSIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx_handle, &i2s_cfg), TAG, "init i2s tx failed");
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(rx_handle, &i2s_cfg), TAG, "init i2s rx failed");

    audio_codec_i2s_cfg_t codec_i2s_cfg = {
        .port = CONFIG_BSP_I2S_NUM,
        .tx_handle = tx_handle,
        .rx_handle = rx_handle,
    };
    *data_if = audio_codec_new_i2s_data(&codec_i2s_cfg);
    ESP_RETURN_ON_FALSE(*data_if, ESP_FAIL, TAG, "create i2s data interface failed");

    return ESP_OK;
}

static esp_err_t speaker_codec_init(const audio_codec_data_if_t *data_if, esp_codec_dev_handle_t *speaker)
{
    ESP_LOGI(TAG, "Initializing ES8311 speaker codec");
    ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "bsp i2c init failed");
    const bool is_v2 = board_is_v2();
    const int speaker_volume = is_v2 ? CONFIG_EXAMPLE_V2_VOICE_VOLUME : CONFIG_EXAMPLE_VOICE_VOLUME;
    ESP_LOGI(TAG, "Using volume %d for %s hardware", speaker_volume, is_v2 ? "V2" : "original");

    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    ESP_RETURN_ON_FALSE(gpio_if, ESP_FAIL, TAG, "create gpio interface failed");

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = BSP_I2C_NUM,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = bsp_i2c_get_handle(),
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    ESP_RETURN_ON_FALSE(i2c_ctrl_if, ESP_FAIL, TAG, "create i2c control interface failed");

    esp_codec_dev_hw_gain_t gain = {
        .pa_voltage = 5.0,
        .codec_dac_voltage = 3.3,
    };

    es8311_codec_cfg_t es8311_cfg = {
        .ctrl_if = i2c_ctrl_if,
        .gpio_if = gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin = BSP_POWER_AMP_IO,
        .pa_reverted = false,
        .master_mode = false,
        .use_mclk = true,
        .digital_mic = false,
        .invert_mclk = false,
        .invert_sclk = false,
        .hw_gain = gain,
    };
    const audio_codec_if_t *es8311_dev = es8311_codec_new(&es8311_cfg);
    ESP_RETURN_ON_FALSE(es8311_dev, ESP_FAIL, TAG, "create es8311 codec failed");

    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .codec_if = es8311_dev,
        .data_if = data_if,
    };
    *speaker = esp_codec_dev_new(&codec_dev_cfg);
    ESP_RETURN_ON_FALSE(*speaker, ESP_FAIL, TAG, "create speaker codec device failed");

    esp_codec_dev_sample_info_t fs = sample_info();
    ESP_RETURN_ON_ERROR(codec_dev_to_esp_err(esp_codec_dev_open(*speaker, &fs), "open speaker codec"), TAG, "open failed");
    ESP_RETURN_ON_ERROR(codec_dev_to_esp_err(esp_codec_dev_set_out_vol(*speaker, speaker_volume),
                                             "set speaker volume"),
                        TAG, "volume failed");

    return ESP_OK;
}

static void fill_sine_frame(int16_t *samples, float *phase)
{
    const float step = AUDIO_TWO_PI * CONFIG_EXAMPLE_TONE_HZ / CONFIG_EXAMPLE_SAMPLE_RATE;

    for (size_t i = 0; i < AUDIO_FRAME_COUNT; i++) {
        int16_t sample = (int16_t)(sinf(*phase) * 12000.0f);
        samples[i] = sample;

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
    const audio_codec_data_if_t *data_if = NULL;

    if (i2s_driver_init(&data_if) != ESP_OK || speaker_codec_init(data_if, &speaker) != ESP_OK) {
        ESP_LOGE(TAG, "Audio setup failed; leaving task idle instead of restarting");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    xTaskCreate(speaker_task, "speaker_task", 4096, speaker, 5, NULL);
}
