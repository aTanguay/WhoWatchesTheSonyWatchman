/**
 * SD Card Interface Implementation
 */

#include "sd_card.h"
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

static const char *TAG = "SD_CARD";

/**
 * Initialize and mount SD card
 */
esp_err_t sd_card_init(sd_card_handle_t *handle)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Initializing SD card...");

    memset(handle, 0, sizeof(sd_card_handle_t));

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024
    };

    // SPI bus is already initialized by display driver
    // We just need to configure SDSPI host

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;  // Start at default speed

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_SD_CS;
    slot_config.host_id = SPI2_HOST;  // Same host as display

    ESP_LOGI(TAG, "Mounting filesystem...");
    esp_err_t ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config,
                                             &mount_config, &handle->card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "Format card with FAT32 if needed.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    handle->mounted = true;

    // Card info
    sdmmc_card_print_info(stdout, handle->card);
    ESP_LOGI(TAG, "SD card mounted successfully");

    // Get capacity info
    FATFS *fs;
    DWORD fre_clust;
    if (f_getfree("0:", &fre_clust, &fs) == FR_OK) {
        uint64_t total_sectors = (fs->n_fatent - 2) * fs->csize;
        uint64_t free_sectors = fre_clust * fs->csize;

        handle->total_bytes = total_sectors * fs->ssize;
        handle->free_bytes = free_sectors * fs->ssize;

        ESP_LOGI(TAG, "SD card: %.2f GB total, %.2f GB free",
                 handle->total_bytes / (1024.0 * 1024.0 * 1024.0),
                 handle->free_bytes / (1024.0 * 1024.0 * 1024.0));
    }

    return ESP_OK;
}

/**
 * Unmount SD card
 */
void sd_card_deinit(sd_card_handle_t *handle)
{
    if (handle == NULL || !handle->mounted) return;

    ESP_LOGI(TAG, "Unmounting SD card...");
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, handle->card);
    handle->mounted = false;
}

/**
 * Check if SD card is mounted
 */
bool sd_card_is_mounted(const sd_card_handle_t *handle)
{
    return (handle != NULL && handle->mounted);
}

/**
 * Get SD card info
 */
esp_err_t sd_card_get_info(sd_card_handle_t *handle, uint32_t *total_mb, uint32_t *free_mb)
{
    if (!sd_card_is_mounted(handle)) return ESP_ERR_INVALID_STATE;

    if (total_mb) {
        *total_mb = handle->total_bytes / (1024 * 1024);
    }
    if (free_mb) {
        *free_mb = handle->free_bytes / (1024 * 1024);
    }

    return ESP_OK;
}

/**
 * Check if file exists
 */
bool sd_card_file_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

/**
 * Get file size
 */
int64_t sd_card_get_file_size(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return st.st_size;
}
