/**
 * Channel Manager
 * Manages TV channels and episodes on SD card
 */

#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define MAX_CHANNELS        16
#define MAX_EPISODES        64
#define MAX_NAME_LEN        64
#define MAX_PATH_LEN        512  // Increased to accommodate full paths safely

/**
 * Episode information
 */
typedef struct {
    char name[MAX_NAME_LEN];
    char path[MAX_PATH_LEN];
    uint32_t duration_sec;  // Duration in seconds
    uint32_t file_size;     // File size in bytes
} episode_t;

/**
 * Channel information
 */
typedef struct {
    char name[MAX_NAME_LEN];
    char path[MAX_PATH_LEN];
    episode_t episodes[MAX_EPISODES];
    uint8_t episode_count;
    uint8_t current_episode;
} channel_t;

/**
 * Channel manager handle
 */
typedef struct {
    channel_t channels[MAX_CHANNELS];
    uint8_t channel_count;
    uint8_t current_channel;
} channel_manager_t;

/**
 * Initialize channel manager and scan SD card for channels
 *
 * @param manager Channel manager handle
 * @return ESP_OK on success
 */
esp_err_t channel_manager_init(channel_manager_t *manager);

/**
 * Scan SD card for channels and episodes
 *
 * @param manager Channel manager handle
 * @return ESP_OK on success
 */
esp_err_t channel_manager_scan(channel_manager_t *manager);

/**
 * Get current channel
 *
 * @param manager Channel manager handle
 * @return Pointer to current channel, or NULL
 */
const channel_t *channel_manager_get_current(const channel_manager_t *manager);

/**
 * Switch to next channel
 *
 * @param manager Channel manager handle
 * @return ESP_OK on success
 */
esp_err_t channel_manager_next_channel(channel_manager_t *manager);

/**
 * Switch to previous channel
 *
 * @param manager Channel manager handle
 * @return ESP_OK on success
 */
esp_err_t channel_manager_prev_channel(channel_manager_t *manager);

/**
 * Set current channel by index
 *
 * @param manager Channel manager handle
 * @param channel_idx Channel index
 * @return ESP_OK on success
 */
esp_err_t channel_manager_set_channel(channel_manager_t *manager, uint8_t channel_idx);

/**
 * Get current episode for current channel
 *
 * @param manager Channel manager handle
 * @return Pointer to current episode, or NULL
 */
const episode_t *channel_manager_get_current_episode(const channel_manager_t *manager);

/**
 * Advance to next episode in current channel
 *
 * @param manager Channel manager handle
 * @return ESP_OK on success
 */
esp_err_t channel_manager_next_episode(channel_manager_t *manager);

/**
 * Get total number of channels
 *
 * @param manager Channel manager handle
 * @return Number of channels
 */
uint8_t channel_manager_get_channel_count(const channel_manager_t *manager);

#endif // CHANNEL_MANAGER_H
