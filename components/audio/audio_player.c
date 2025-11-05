/**
 * Audio Player Implementation
 * I2S-based audio playback
 *
 * Tasks: T1.22-T1.25
 */

#include "audio_player.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"

static const char *TAG = "AUDIO_PLAYER";

/**
 * Audio player structure
 */
struct audio_player_s {
    i2s_chan_handle_t tx_handle;
    audio_config_t config;
    audio_state_t state;
    uint8_t volume;  // 0-100
    bool initialized;
};

/**
 * Apply volume scaling to audio samples
 */
static void apply_volume(int16_t *samples, size_t count, uint8_t volume)
{
    if (volume >= 100) return;  // No scaling needed

    uint32_t scale = (volume * 65536) / 100;  // Fixed-point scaling

    for (size_t i = 0; i < count; i++) {
        int32_t sample = samples[i];
        sample = (sample * scale) >> 16;

        // Clamp to 16-bit range
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        samples[i] = (int16_t)sample;
    }
}

/**
 * Initialize audio player
 */
audio_player_t *audio_player_init(const audio_config_t *config)
{
    ESP_LOGI(TAG, "Initializing audio player...");

    audio_player_t *player = malloc(sizeof(audio_player_t));
    if (player == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio player");
        return NULL;
    }

    memset(player, 0, sizeof(audio_player_t));

    // Use default config if none provided
    if (config) {
        memcpy(&player->config, config, sizeof(audio_config_t));
    } else {
        player->config.sample_rate = AUDIO_SAMPLE_RATE;
        player->config.bits_per_sample = AUDIO_BITS;
        player->config.channels = AUDIO_CHANNELS;
        player->config.pin_bclk = PIN_AUDIO_BCLK;
        player->config.pin_ws = PIN_AUDIO_WS;
        player->config.pin_dout = PIN_AUDIO_DOUT;
    }

    player->volume = 80;  // Default 80% volume
    player->state = AUDIO_STATE_STOPPED;

    // Configure I2S channel
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;  // Auto clear DMA buffer on underflow

    esp_err_t ret = i2s_new_channel(&chan_cfg, &player->tx_handle, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channel: %s", esp_err_to_name(ret));
        free(player);
        return NULL;
    }

    // Configure I2S standard mode
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(player->config.sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            (i2s_data_bit_width_t)player->config.bits_per_sample,
            (i2s_slot_mode_t)player->config.channels
        ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = player->config.pin_bclk,
            .ws = player->config.pin_ws,
            .dout = player->config.pin_dout,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ret = i2s_channel_init_std_mode(player->tx_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2S std mode: %s", esp_err_to_name(ret));
        i2s_del_channel(player->tx_handle);
        free(player);
        return NULL;
    }

    player->initialized = true;

    ESP_LOGI(TAG, "Audio player initialized: %lu Hz, %d-bit, %s",
             player->config.sample_rate,
             player->config.bits_per_sample,
             player->config.channels == 1 ? "mono" : "stereo");

    return player;
}

/**
 * Deinitialize audio player
 */
void audio_player_deinit(audio_player_t *player)
{
    if (player == NULL) return;

    if (player->initialized) {
        audio_player_stop(player);

        if (player->tx_handle) {
            i2s_del_channel(player->tx_handle);
        }
    }

    free(player);

    ESP_LOGI(TAG, "Audio player deinitialized");
}

/**
 * Start audio playback
 */
esp_err_t audio_player_start(audio_player_t *player)
{
    if (player == NULL || !player->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (player->state == AUDIO_STATE_PLAYING) {
        return ESP_OK;  // Already playing
    }

    esp_err_t ret = i2s_channel_enable(player->tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(ret));
        return ret;
    }

    player->state = AUDIO_STATE_PLAYING;

    ESP_LOGI(TAG, "Audio playback started");

    return ESP_OK;
}

/**
 * Stop audio playback
 */
esp_err_t audio_player_stop(audio_player_t *player)
{
    if (player == NULL || !player->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (player->state == AUDIO_STATE_STOPPED) {
        return ESP_OK;
    }

    esp_err_t ret = i2s_channel_disable(player->tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable I2S channel: %s", esp_err_to_name(ret));
        return ret;
    }

    player->state = AUDIO_STATE_STOPPED;

    ESP_LOGI(TAG, "Audio playback stopped");

    return ESP_OK;
}

/**
 * Pause audio playback
 */
esp_err_t audio_player_pause(audio_player_t *player)
{
    if (player == NULL || !player->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (player->state != AUDIO_STATE_PLAYING) {
        return ESP_OK;
    }

    // I2S doesn't have a pause function, so we stop
    esp_err_t ret = i2s_channel_disable(player->tx_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    player->state = AUDIO_STATE_PAUSED;

    ESP_LOGI(TAG, "Audio playback paused");

    return ESP_OK;
}

/**
 * Resume audio playback
 */
esp_err_t audio_player_resume(audio_player_t *player)
{
    if (player == NULL || !player->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (player->state != AUDIO_STATE_PAUSED) {
        return ESP_OK;
    }

    esp_err_t ret = i2s_channel_enable(player->tx_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    player->state = AUDIO_STATE_PLAYING;

    ESP_LOGI(TAG, "Audio playback resumed");

    return ESP_OK;
}

/**
 * Write PCM audio data
 */
esp_err_t audio_player_write(audio_player_t *player, const uint8_t *data,
                              size_t size, size_t *bytes_written)
{
    if (player == NULL || !player->initialized || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (player->state != AUDIO_STATE_PLAYING) {
        return ESP_ERR_INVALID_STATE;
    }

    // Apply volume scaling if needed
    if (player->volume < 100 && player->config.bits_per_sample == 16) {
        // Create temporary buffer for volume scaling
        size_t sample_count = size / 2;
        int16_t *scaled_samples = malloc(size);
        if (scaled_samples != NULL) {
            memcpy(scaled_samples, data, size);
            apply_volume(scaled_samples, sample_count, player->volume);

            esp_err_t ret = i2s_channel_write(player->tx_handle, scaled_samples,
                                              size, bytes_written, portMAX_DELAY);
            free(scaled_samples);
            return ret;
        }
    }

    // Write directly without volume scaling
    return i2s_channel_write(player->tx_handle, data, size, bytes_written, portMAX_DELAY);
}

/**
 * Set volume
 */
esp_err_t audio_player_set_volume(audio_player_t *player, uint8_t volume)
{
    if (player == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (volume > 100) volume = 100;

    player->volume = volume;

    ESP_LOGI(TAG, "Volume set to %d%%", volume);

    return ESP_OK;
}

/**
 * Get volume
 */
uint8_t audio_player_get_volume(const audio_player_t *player)
{
    return player ? player->volume : 0;
}

/**
 * Clear buffer
 */
esp_err_t audio_player_clear_buffer(audio_player_t *player)
{
    if (player == NULL || !player->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Stop and restart to clear buffer
    audio_state_t prev_state = player->state;

    if (prev_state != AUDIO_STATE_STOPPED) {
        i2s_channel_disable(player->tx_handle);
    }

    if (prev_state == AUDIO_STATE_PLAYING) {
        i2s_channel_enable(player->tx_handle);
    }

    return ESP_OK;
}

/**
 * Get state
 */
audio_state_t audio_player_get_state(const audio_player_t *player)
{
    return player ? player->state : AUDIO_STATE_STOPPED;
}

/**
 * Get available buffer space
 */
size_t audio_player_get_buffer_available(const audio_player_t *player)
{
    // I2S channel doesn't expose buffer info easily
    // Return estimated based on configuration
    return player ? AUDIO_BUFFER_SIZE : 0;
}
