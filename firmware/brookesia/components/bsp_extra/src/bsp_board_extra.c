/* SPDX-FileCopyrightText: 2015-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0 */

#include <dirent.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "audio_player.h"
#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_codec_dev_defaults.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"

static const char *TAG = "bsp_extra";
static esp_codec_dev_handle_t s_codec_dev;
static const audio_codec_data_if_t *s_i2s_data_if;
static const audio_codec_ctrl_if_t *s_i2c_ctrl_if;
static const audio_codec_gpio_if_t *s_gpio_if;
static const audio_codec_if_t *s_es8311_if;
static i2s_chan_handle_t s_i2s_tx;
static i2s_chan_handle_t s_i2s_rx;
static bool s_audio_initialized;
static bool s_codec_open;
static bool s_player_initialized;
static int s_volume = CODEC_DEFAULT_VOLUME;
static audio_player_cb_t s_player_callback;
static void *s_player_callback_user_data;
static char s_audio_file_path[256];
static portMUX_TYPE s_session_mux = portMUX_INITIALIZER_UNLOCKED;
static bsp_extra_audio_owner_t s_session_owner = BSP_EXTRA_AUDIO_OWNER_NONE;

static esp_err_t codec_result(int result, const char *operation)
{
    if (result == ESP_CODEC_DEV_OK) return ESP_OK;
    ESP_LOGE(TAG, "%s failed: %d", operation, result);
    return ESP_FAIL;
}

/* The BSP I2C bus is shared with display and board services, so this helper
 * only releases interfaces and I2S channels owned by this component. */
static void codec_resources_release(void)
{
    if (s_codec_dev) {
        esp_codec_dev_delete(s_codec_dev);
        s_codec_dev = NULL;
    }
    if (s_es8311_if) {
        (void)audio_codec_delete_codec_if(s_es8311_if);
        s_es8311_if = NULL;
    }
    if (s_i2c_ctrl_if) {
        (void)audio_codec_delete_ctrl_if(s_i2c_ctrl_if);
        s_i2c_ctrl_if = NULL;
    }
    if (s_gpio_if) {
        (void)audio_codec_delete_gpio_if(s_gpio_if);
        s_gpio_if = NULL;
    }
    if (s_i2s_data_if) {
        (void)audio_codec_delete_data_if(s_i2s_data_if);
        s_i2s_data_if = NULL;
    }
    if (s_i2s_tx) {
        (void)i2s_channel_disable(s_i2s_tx);
        (void)i2s_del_channel(s_i2s_tx);
        s_i2s_tx = NULL;
    }
    if (s_i2s_rx) {
        (void)i2s_channel_disable(s_i2s_rx);
        (void)i2s_del_channel(s_i2s_rx);
        s_i2s_rx = NULL;
    }
    s_codec_open = false;
    s_audio_initialized = false;
}

static esp_err_t codec_resources_init(void)
{
    esp_err_t result = bsp_i2c_init();
    if (result != ESP_OK) return result;

    i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_handle();
    if (!i2c_bus) return ESP_ERR_INVALID_STATE;

    i2s_chan_config_t channel_cfg = I2S_CHANNEL_DEFAULT_CONFIG(CONFIG_BSP_I2S_NUM, I2S_ROLE_MASTER);
    channel_cfg.auto_clear = true;
    result = i2s_new_channel(&channel_cfg, &s_i2s_tx, &s_i2s_rx);
    if (result != ESP_OK) goto fail;

    const i2s_std_config_t i2s_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CODEC_DEFAULT_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = BSP_I2S_MCLK,
            .bclk = BSP_I2S_SCLK,
            .ws = BSP_I2S_LCLK,
            .dout = BSP_I2S_DOUT,
            .din = BSP_I2S_DSIN,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    result = i2s_channel_init_std_mode(s_i2s_tx, &i2s_cfg);
    if (result != ESP_OK) goto fail;
    result = i2s_channel_init_std_mode(s_i2s_rx, &i2s_cfg);
    if (result != ESP_OK) goto fail;

    audio_codec_i2s_cfg_t codec_i2s_cfg = {
        .port = CONFIG_BSP_I2S_NUM,
        .tx_handle = s_i2s_tx,
        .rx_handle = s_i2s_rx,
    };
    s_i2s_data_if = audio_codec_new_i2s_data(&codec_i2s_cfg);
    if (!s_i2s_data_if) { result = ESP_ERR_NO_MEM; goto fail; }

    s_gpio_if = audio_codec_new_gpio();
    if (!s_gpio_if) { result = ESP_ERR_NO_MEM; goto fail; }
    audio_codec_i2c_cfg_t codec_i2c_cfg = {
        .port = BSP_I2C_NUM,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = i2c_bus,
    };
    s_i2c_ctrl_if = audio_codec_new_i2c_ctrl(&codec_i2c_cfg);
    if (!s_i2c_ctrl_if) { result = ESP_ERR_NO_MEM; goto fail; }

    const esp_codec_dev_hw_gain_t gain = { .pa_voltage = 5.0, .codec_dac_voltage = 3.3 };
    es8311_codec_cfg_t es8311_cfg = {
        .ctrl_if = s_i2c_ctrl_if,
        .gpio_if = s_gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .pa_pin = BSP_POWER_AMP_IO,
        .pa_reverted = false,
        .master_mode = false,
        .use_mclk = true,
        .digital_mic = false,
        .invert_mclk = false,
        .invert_sclk = false,
        .hw_gain = gain,
    };
    s_es8311_if = es8311_codec_new(&es8311_cfg);
    if (!s_es8311_if) { result = ESP_ERR_NO_MEM; goto fail; }

    esp_codec_dev_cfg_t codec_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
        .codec_if = s_es8311_if,
        .data_if = s_i2s_data_if,
    };
    s_codec_dev = esp_codec_dev_new(&codec_cfg);
    if (!s_codec_dev) { result = ESP_ERR_NO_MEM; goto fail; }
    return ESP_OK;

fail:
    codec_resources_release();
    return result;
}

static bool owner_is_valid(bsp_extra_audio_owner_t owner)
{
    return owner > BSP_EXTRA_AUDIO_OWNER_NONE && owner < BSP_EXTRA_AUDIO_OWNER_MAX;
}

const char *bsp_extra_audio_owner_name(bsp_extra_audio_owner_t owner)
{
    switch (owner) {
    case BSP_EXTRA_AUDIO_OWNER_NONE: return "None";
    case BSP_EXTRA_AUDIO_OWNER_MUSIC: return "MusicPlayer";
    case BSP_EXTRA_AUDIO_OWNER_VIDEO: return "VideoPlayer";
    case BSP_EXTRA_AUDIO_OWNER_RECORDER: return "Recorder";
    case BSP_EXTRA_AUDIO_OWNER_SPEC_ANALYZER: return "SpecAnalyzer";
    case BSP_EXTRA_AUDIO_OWNER_XIAOZHI: return "Xiaozhi";
    default: return "Invalid";
    }
}

bsp_extra_audio_owner_t bsp_extra_audio_session_get_owner(void)
{
    portENTER_CRITICAL(&s_session_mux);
    const bsp_extra_audio_owner_t owner = s_session_owner;
    portEXIT_CRITICAL(&s_session_mux);
    return owner;
}

esp_err_t bsp_extra_audio_session_acquire(bsp_extra_audio_owner_t owner)
{
    if (!owner_is_valid(owner)) return ESP_ERR_INVALID_ARG;
    portENTER_CRITICAL(&s_session_mux);
    const bsp_extra_audio_owner_t previous = s_session_owner;
    if (previous == BSP_EXTRA_AUDIO_OWNER_NONE) s_session_owner = owner;
    portEXIT_CRITICAL(&s_session_mux);
    if (previous != BSP_EXTRA_AUDIO_OWNER_NONE) {
        ESP_LOGW(TAG, "Audio busy: requested=%s current=%s",
                 bsp_extra_audio_owner_name(owner), bsp_extra_audio_owner_name(previous));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t bsp_extra_audio_session_release(bsp_extra_audio_owner_t owner)
{
    if (!owner_is_valid(owner)) return ESP_ERR_INVALID_ARG;
    portENTER_CRITICAL(&s_session_mux);
    const bsp_extra_audio_owner_t previous = s_session_owner;
    if (previous == owner) s_session_owner = BSP_EXTRA_AUDIO_OWNER_NONE;
    portEXIT_CRITICAL(&s_session_mux);
    return previous == owner ? ESP_OK : ESP_ERR_INVALID_STATE;
}

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    const bool mute = setting == AUDIO_PLAYER_MUTE;
    ESP_RETURN_ON_ERROR(bsp_extra_codec_mute_set(mute), TAG, "Set codec mute failed");
    if (!mute) {
        ESP_RETURN_ON_ERROR(codec_result(esp_codec_dev_set_out_vol(s_codec_dev, s_volume),
                                         "Restore codec volume"), TAG, "Restore codec volume failed");
    }
    return ESP_OK;
}

static void audio_event_callback(audio_player_cb_ctx_t *ctx)
{
    if (s_player_callback) {
        ctx->user_ctx = s_player_callback_user_data;
        s_player_callback(ctx);
    }
}

static bool is_supported_audio_file(const char *name)
{
    const char *extension = name ? strrchr(name, '.') : NULL;
    return extension && (strcasecmp(extension, ".mp3") == 0 ||
                         strcasecmp(extension, ".wav") == 0);
}

static void free_file_instance(file_iterator_instance_t *instance)
{
    if (!instance) return;
    if (instance->list) {
        for (size_t i = 0; i < instance->count; ++i) free(instance->list[i]);
        free(instance->list);
    }
    free((void *)instance->directory_path);
    free(instance);
}

esp_err_t bsp_extra_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (bytes_read) *bytes_read = 0;
    ESP_RETURN_ON_FALSE(s_codec_dev && s_codec_open && audio_buffer, ESP_ERR_INVALID_STATE, TAG,
                        "Microphone is not initialized");
    const esp_err_t result = codec_result(esp_codec_dev_read(s_codec_dev, audio_buffer, len), "Microphone read");
    if (result == ESP_OK && bytes_read) *bytes_read = len;
    return result;
}

esp_err_t bsp_extra_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (bytes_written) *bytes_written = 0;
    ESP_RETURN_ON_FALSE(s_codec_dev && s_codec_open && audio_buffer, ESP_ERR_INVALID_STATE, TAG,
                        "Speaker is not initialized");
    const esp_err_t result = codec_result(esp_codec_dev_write(s_codec_dev, audio_buffer, len), "Speaker write");
    if (result == ESP_OK && bytes_written) *bytes_written = len;
    return result;
}

esp_err_t bsp_extra_codec_dev_stop(void)
{
    esp_err_t result = ESP_OK;
    if (s_codec_dev && esp_codec_dev_close(s_codec_dev) != ESP_CODEC_DEV_OK) result = ESP_FAIL;
    s_codec_open = false;
    return result;
}

esp_err_t bsp_extra_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t channel_mode)
{
    ESP_RETURN_ON_FALSE(s_codec_dev, ESP_ERR_INVALID_STATE, TAG,
                        "Codec is not initialized");
    ESP_RETURN_ON_ERROR(bsp_extra_codec_dev_stop(), TAG, "Close codec devices failed");
    esp_codec_dev_sample_info_t sample_info = {
        .sample_rate = rate, .channel = channel_mode, .bits_per_sample = bits_cfg,
    };
    if (esp_codec_dev_open(s_codec_dev, &sample_info) != ESP_CODEC_DEV_OK) {
        (void)bsp_extra_codec_dev_stop();
        return ESP_FAIL;
    }
    s_codec_open = true;

    esp_err_t result = codec_result(
        esp_codec_dev_set_in_gain(s_codec_dev, CODEC_DEFAULT_ADC_VOLUME),
        "Set microphone gain"
    );
    if (result == ESP_OK) {
        result = codec_result(esp_codec_dev_set_out_mute(s_codec_dev, false), "Unmute speaker");
    }
    if (result == ESP_OK) {
        result = codec_result(esp_codec_dev_set_out_vol(s_codec_dev, s_volume),
                              "Set speaker volume");
    }
    if (result != ESP_OK) {
        (void)bsp_extra_codec_dev_stop();
    }
    return result;
}

esp_err_t bsp_extra_codec_set_voice_fs(uint32_t rate, uint32_t bits_cfg)
{
    return bsp_extra_codec_set_fs(rate, bits_cfg, I2S_SLOT_MODE_MONO);
}

bool bsp_extra_codec_pa_is_enabled(void) { return s_codec_dev != NULL && s_codec_open; }

esp_err_t bsp_extra_codec_volume_set(int volume, int *volume_set)
{
    ESP_RETURN_ON_FALSE(volume >= 0 && volume <= 100, ESP_ERR_INVALID_ARG, TAG,
                        "Volume must be between 0 and 100");
    s_volume = volume;
    if (volume_set) *volume_set = volume;
    if (!s_codec_dev || !s_codec_open) return ESP_OK;
    return codec_result(esp_codec_dev_set_out_vol(s_codec_dev, volume), "Set speaker volume");
}

int bsp_extra_codec_volume_get(void) { return s_volume; }

esp_err_t bsp_extra_codec_mute_set(bool enable)
{
    ESP_RETURN_ON_FALSE(s_codec_dev && s_codec_open, ESP_ERR_INVALID_STATE, TAG, "Speaker is not initialized");
    return codec_result(esp_codec_dev_set_out_mute(s_codec_dev, enable), "Set speaker mute");
}

esp_err_t bsp_extra_codec_dev_resume(void)
{
    return bsp_extra_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH,
                                  I2S_SLOT_MODE_STEREO);
}

esp_err_t bsp_extra_codec_init(void)
{
    if (s_audio_initialized) return ESP_OK;
    ESP_RETURN_ON_ERROR(codec_resources_init(), TAG, "Create codec resources failed");
    const esp_err_t result = bsp_extra_codec_dev_resume();
    if (result != ESP_OK) {
        codec_resources_release();
        return result;
    }
    s_audio_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_extra_player_init(void)
{
    if (s_player_initialized) return ESP_OK;
    ESP_RETURN_ON_ERROR(bsp_extra_codec_init(), TAG, "Initialize codec failed");
    const audio_player_config_t config = {
        .mute_fn = audio_mute_function, .write_fn = bsp_extra_i2s_write,
        .clk_set_fn = bsp_extra_codec_set_fs, .priority = 5, .coreID = 0,
        .force_stereo = true,
    };
    ESP_RETURN_ON_ERROR(audio_player_new(config), TAG, "Create audio player failed");
    const esp_err_t result = audio_player_callback_register(audio_event_callback, NULL);
    if (result != ESP_OK) { (void)audio_player_delete(); return result; }
    s_audio_file_path[0] = '\0';
    s_player_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_extra_player_del(void)
{
    if (!s_player_initialized) return ESP_OK;
    ESP_RETURN_ON_ERROR(audio_player_delete(), TAG, "Delete audio player failed");
    s_player_initialized = false;
    s_audio_file_path[0] = '\0';
    return bsp_extra_codec_dev_stop();
}

bool bsp_extra_player_is_initialized(void) { return s_player_initialized; }

esp_err_t bsp_extra_file_instance_init(const char *path, file_iterator_instance_t **ret_instance)
{
    ESP_RETURN_ON_FALSE(path && ret_instance, ESP_ERR_INVALID_ARG, TAG, "Invalid iterator arguments");
    *ret_instance = NULL;
    DIR *directory = opendir(path);
    ESP_RETURN_ON_FALSE(directory, ESP_ERR_NOT_FOUND, TAG, "Audio directory unavailable");
    size_t file_count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory))) if (is_supported_audio_file(entry->d_name)) ++file_count;
    closedir(directory);
    file_iterator_instance_t *instance = calloc(1, sizeof(*instance));
    ESP_RETURN_ON_FALSE(instance, ESP_ERR_NO_MEM, TAG, "Allocate iterator failed");
    instance->directory_path = strdup(path);
    if (!instance->directory_path) { free_file_instance(instance); return ESP_ERR_NO_MEM; }
    if (file_count) {
        instance->list = calloc(file_count, sizeof(*instance->list));
        if (!instance->list) { free_file_instance(instance); return ESP_ERR_NO_MEM; }
        directory = opendir(path);
        if (!directory) { free_file_instance(instance); return ESP_ERR_NOT_FOUND; }
        while (instance->count < file_count && (entry = readdir(directory))) {
            if (is_supported_audio_file(entry->d_name)) {
                instance->list[instance->count] = strdup(entry->d_name);
                if (!instance->list[instance->count]) { closedir(directory); free_file_instance(instance); return ESP_ERR_NO_MEM; }
                ++instance->count;
            }
        }
        closedir(directory);
    }
    *ret_instance = instance;
    return ESP_OK;
}

void bsp_extra_file_instance_deinit(file_iterator_instance_t **instance)
{
    if (instance) { free_file_instance(*instance); *instance = NULL; }
}

esp_err_t bsp_extra_player_play_file(const char *file_path)
{
    ESP_RETURN_ON_FALSE(s_player_initialized && file_path, ESP_ERR_INVALID_STATE, TAG,
                        "Audio player is not initialized");
    ESP_RETURN_ON_FALSE(is_supported_audio_file(file_path), ESP_ERR_NOT_SUPPORTED, TAG,
                        "Only MP3 and WAV files are supported");
    FILE *file = fopen(file_path, "rb");
    ESP_RETURN_ON_FALSE(file, ESP_ERR_NOT_FOUND, TAG, "Open audio file failed");
    const esp_err_t result = audio_player_play(file);
    if (result != ESP_OK) { fclose(file); return result; }
    strlcpy(s_audio_file_path, file_path, sizeof(s_audio_file_path));
    return ESP_OK;
}

esp_err_t bsp_extra_player_play_index(file_iterator_instance_t *instance, int index)
{
    ESP_RETURN_ON_FALSE(instance && index >= 0 && (size_t)index < instance->count,
                        ESP_ERR_INVALID_ARG, TAG, "Invalid track index");
    char path[sizeof(s_audio_file_path)];
    const int length = file_iterator_get_full_path_from_index(instance, (size_t)index, path, sizeof(path));
    ESP_RETURN_ON_FALSE(length > 0 && (size_t)length < sizeof(path), ESP_ERR_INVALID_SIZE, TAG,
                        "Track path too long");
    ESP_RETURN_ON_ERROR(bsp_extra_player_play_file(path), TAG, "Start track failed");
    file_iterator_set_index(instance, (size_t)index);
    return ESP_OK;
}

void bsp_extra_player_register_callback(audio_player_cb_t cb, void *user_data)
{
    s_player_callback = cb;
    s_player_callback_user_data = user_data;
}

bool bsp_extra_player_is_playing_by_path(const char *file_path)
{
    if (!file_path || !s_player_initialized || !s_audio_file_path[0]) return false;
    const audio_player_state_t state = audio_player_get_state();
    return state != AUDIO_PLAYER_STATE_IDLE && state != AUDIO_PLAYER_STATE_SHUTDOWN &&
           strcmp(s_audio_file_path, file_path) == 0;
}

bool bsp_extra_player_is_playing_by_index(file_iterator_instance_t *instance, int index)
{
    if (!instance || index < 0 || (size_t)index >= instance->count) return false;
    char path[sizeof(s_audio_file_path)];
    const int length = file_iterator_get_full_path_from_index(instance, (size_t)index, path, sizeof(path));
    return length > 0 && (size_t)length < sizeof(path) && bsp_extra_player_is_playing_by_path(path);
}
