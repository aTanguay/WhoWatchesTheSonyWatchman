/**
 * Video Player
 * Main video playback engine for MJPEG/AVI files
 */

#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Playback states
typedef enum {
    VIDEO_STATE_STOPPED,
    VIDEO_STATE_PLAYING,
    VIDEO_STATE_PAUSED,
    VIDEO_STATE_ERROR
} video_state_t;

/**
 * Video file information
 */
typedef struct {
    char path[256];
    uint16_t width;
    uint16_t height;
    uint16_t fps;
    uint32_t frame_count;
    uint32_t duration_sec;
} video_info_t;

/**
 * Video player handle
 */
typedef struct video_player_s video_player_t;

/**
 * Video event callbacks
 */
typedef struct {
    void (*on_frame_decoded)(void *user_data, uint32_t frame_num);
    void (*on_playback_complete)(void *user_data);
    void (*on_error)(void *user_data, esp_err_t error);
} video_callbacks_t;

/**
 * Create video player instance
 *
 * @param callbacks Event callbacks (optional, can be NULL)
 * @param user_data User data passed to callbacks
 * @return Video player handle, or NULL on failure
 */
video_player_t *video_player_create(const video_callbacks_t *callbacks, void *user_data);

/**
 * Destroy video player instance
 *
 * @param player Video player handle
 */
void video_player_destroy(video_player_t *player);

/**
 * Open video file for playback
 *
 * @param player Video player handle
 * @param file_path Path to video file
 * @return ESP_OK on success
 */
esp_err_t video_player_open(video_player_t *player, const char *file_path);

/**
 * Close current video file
 *
 * @param player Video player handle
 */
void video_player_close(video_player_t *player);

/**
 * Start/resume playback
 *
 * @param player Video player handle
 * @return ESP_OK on success
 */
esp_err_t video_player_play(video_player_t *player);

/**
 * Pause playback
 *
 * @param player Video player handle
 * @return ESP_OK on success
 */
esp_err_t video_player_pause(video_player_t *player);

/**
 * Stop playback
 *
 * @param player Video player handle
 * @return ESP_OK on success
 */
esp_err_t video_player_stop(video_player_t *player);

/**
 * Seek to specific frame
 *
 * @param player Video player handle
 * @param frame_num Frame number
 * @return ESP_OK on success
 */
esp_err_t video_player_seek(video_player_t *player, uint32_t frame_num);

/**
 * Get current playback state
 *
 * @param player Video player handle
 * @return Current state
 */
video_state_t video_player_get_state(const video_player_t *player);

/**
 * Get video information
 *
 * @param player Video player handle
 * @param info Output video info structure
 * @return ESP_OK on success
 */
esp_err_t video_player_get_info(const video_player_t *player, video_info_t *info);

/**
 * Get current frame number
 *
 * @param player Video player handle
 * @return Current frame number
 */
uint32_t video_player_get_current_frame(const video_player_t *player);

/**
 * Get current playback position in seconds
 *
 * @param player Video player handle
 * @return Position in seconds
 */
uint32_t video_player_get_position_sec(const video_player_t *player);

#endif // VIDEO_PLAYER_H
