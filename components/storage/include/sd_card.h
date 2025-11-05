/**
 * SD Card Interface
 * Manages SD card mounting and file access
 */

#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// SD card pin definitions (SPI mode)
#define PIN_SD_MISO     19
#define PIN_SD_MOSI     23
#define PIN_SD_CLK      18
#define PIN_SD_CS       17

// SD card mount point
#define SD_MOUNT_POINT  "/sdcard"

// File paths
#define SD_VIDEO_ROOT   "/sdcard/channels"
#define SD_STATE_FILE   "/sdcard/state.dat"

/**
 * SD card handle
 */
typedef struct {
    sdmmc_card_t *card;
    bool mounted;
    uint64_t total_bytes;
    uint64_t free_bytes;
} sd_card_handle_t;

/**
 * Initialize and mount SD card
 *
 * @param handle SD card handle to initialize
 * @return ESP_OK on success
 */
esp_err_t sd_card_init(sd_card_handle_t *handle);

/**
 * Unmount SD card
 *
 * @param handle SD card handle
 */
void sd_card_deinit(sd_card_handle_t *handle);

/**
 * Check if SD card is mounted
 *
 * @param handle SD card handle
 * @return true if mounted
 */
bool sd_card_is_mounted(const sd_card_handle_t *handle);

/**
 * Get SD card info
 *
 * @param handle SD card handle
 * @param total_mb Total capacity in MB (output)
 * @param free_mb Free space in MB (output)
 * @return ESP_OK on success
 */
esp_err_t sd_card_get_info(sd_card_handle_t *handle, uint32_t *total_mb, uint32_t *free_mb);

/**
 * Check if file exists
 *
 * @param path File path
 * @return true if file exists
 */
bool sd_card_file_exists(const char *path);

/**
 * Get file size
 *
 * @param path File path
 * @return File size in bytes, or -1 on error
 */
int64_t sd_card_get_file_size(const char *path);

#endif // SD_CARD_H
