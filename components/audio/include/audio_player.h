/**
 * Audio Player
 * I2S-based audio playback for ESP32
 *
 * Tasks: T1.22-T1.25 - Audio output and playback
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2s_std.h"

// Audio pin definitions (I2S)
#define PIN_AUDIO_BCLK      26  // Bit clock
#define PIN_AUDIO_WS        25  // Word select (LRCLK)
#define PIN_AUDIO_DOUT      22  // Data out

// Audio configuration
#define AUDIO_SAMPLE_RATE   22050   // 22.05 kHz (matches video encoding)
#define AUDIO_BITS          16      // 16-bit samples
#define AUDIO_CHANNELS      1       // Mono
#define AUDIO_BUFFER_SIZE   1024    // Samples per buffer

/**
 * Audio format
 */
typedef enum {
    AUDIO_FORMAT_PCM_16BIT,     // Raw 16-bit PCM
    AUDIO_FORMAT_PCM_8BIT,      // Raw 8-bit PCM
    AUDIO_FORMAT_MP3,           // MP3 (requires decoder)
} audio_format_t;

/**
 * Audio player state
 */
typedef enum {
    AUDIO_STATE_STOPPED,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
} audio_state_t;

/**
 * Audio player configuration
 */
typedef struct {
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    uint8_t channels;
    int pin_bclk;
    int pin_ws;
    int pin_dout;
} audio_config_t;

/**
 * Audio player handle
 */
typedef struct audio_player_s audio_player_t;

/**
 * Initialize audio player with I2S
 *
 * @param config Audio configuration (NULL for defaults)
 * @return Audio player handle, or NULL on failure
 */
audio_player_t *audio_player_init(const audio_config_t *config);

/**
 * Deinitialize audio player
 *
 * @param player Audio player handle
 */
void audio_player_deinit(audio_player_t *player);

/**
 * Start audio playback
 *
 * @param player Audio player handle
 * @return ESP_OK on success
 */
esp_err_t audio_player_start(audio_player_t *player);

/**
 * Stop audio playback
 *
 * @param player Audio player handle
 * @return ESP_OK on success
 */
esp_err_t audio_player_stop(audio_player_t *player);

/**
 * Pause audio playback
 *
 * @param player Audio player handle
 * @return ESP_OK on success
 */
esp_err_t audio_player_pause(audio_player_t *player);

/**
 * Resume audio playback
 *
 * @param player Audio player handle
 * @return ESP_OK on success
 */
esp_err_t audio_player_resume(audio_player_t *player);

/**
 * Write PCM audio data to player
 * Non-blocking - queues data for playback
 *
 * @param player Audio player handle
 * @param data PCM audio data
 * @param size Size in bytes
 * @param bytes_written Actual bytes written (output)
 * @return ESP_OK on success
 */
esp_err_t audio_player_write(audio_player_t *player, const uint8_t *data,
                              size_t size, size_t *bytes_written);

/**
 * Set volume (0-100)
 *
 * @param player Audio player handle
 * @param volume Volume level 0-100
 * @return ESP_OK on success
 */
esp_err_t audio_player_set_volume(audio_player_t *player, uint8_t volume);

/**
 * Get current volume
 *
 * @param player Audio player handle
 * @return Volume level 0-100
 */
uint8_t audio_player_get_volume(const audio_player_t *player);

/**
 * Clear audio buffer
 *
 * @param player Audio player handle
 * @return ESP_OK on success
 */
esp_err_t audio_player_clear_buffer(audio_player_t *player);

/**
 * Get current playback state
 *
 * @param player Audio player handle
 * @return Current state
 */
audio_state_t audio_player_get_state(const audio_player_t *player);

/**
 * Get available buffer space
 *
 * @param player Audio player handle
 * @return Number of bytes available in buffer
 */
size_t audio_player_get_buffer_available(const audio_player_t *player);

#endif // AUDIO_PLAYER_H
