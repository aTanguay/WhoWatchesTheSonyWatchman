/**
 * Channel Manager Implementation
 */

#include "channel_manager.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_log.h"

static const char *TAG = "CHANNEL_MGR";

// Supported video file extensions
static const char *video_extensions[] = {".avi", ".mjpeg", ".mjpg", NULL};

/**
 * Check if file has video extension
 */
static bool is_video_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if (ext == NULL) return false;

    for (int i = 0; video_extensions[i] != NULL; i++) {
        if (strcasecmp(ext, video_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * Scan directory for episodes
 */
static esp_err_t scan_channel_episodes(channel_t *channel, const char *channel_path)
{
    DIR *dir = opendir(channel_path);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open channel directory: %s", channel_path);
        return ESP_FAIL;
    }

    channel->episode_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && channel->episode_count < MAX_EPISODES) {
        if (entry->d_type != DT_REG) continue;  // Skip non-files

        if (!is_video_file(entry->d_name)) continue;

        episode_t *ep = &channel->episodes[channel->episode_count];

        // Build full path using local buffer to avoid aliasing warnings
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, MAX_PATH_LEN, "%s/%s", channel_path, entry->d_name);
        strncpy(ep->path, full_path, MAX_PATH_LEN - 1);
        ep->path[MAX_PATH_LEN - 1] = '\0';  // Ensure null termination

        // Get file name without extension
        strncpy(ep->name, entry->d_name, MAX_NAME_LEN - 1);
        char *ext = strrchr(ep->name, '.');
        if (ext) *ext = '\0';

        // Get file size
        struct stat st;
        if (stat(ep->path, &st) == 0) {
            ep->file_size = st.st_size;
        } else {
            ep->file_size = 0;
        }

        // Duration will be determined during playback
        ep->duration_sec = 0;

        ESP_LOGI(TAG, "  Episode %d: %s (%.2f MB)",
                 channel->episode_count + 1, ep->name,
                 ep->file_size / (1024.0 * 1024.0));

        channel->episode_count++;
    }

    closedir(dir);

    if (channel->episode_count > 0) {
        ESP_LOGI(TAG, "Found %d episodes in channel '%s'",
                 channel->episode_count, channel->name);
    }

    return ESP_OK;
}

/**
 * Initialize channel manager
 */
esp_err_t channel_manager_init(channel_manager_t *manager)
{
    if (manager == NULL) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Initializing channel manager...");

    memset(manager, 0, sizeof(channel_manager_t));

    return channel_manager_scan(manager);
}

/**
 * Scan SD card for channels
 */
esp_err_t channel_manager_scan(channel_manager_t *manager)
{
    if (manager == NULL) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Scanning for channels...");

    manager->channel_count = 0;
    manager->current_channel = 0;

    // Open channels root directory
    DIR *dir = opendir("/sdcard/channels");
    if (dir == NULL) {
        ESP_LOGW(TAG, "Channels directory not found, creating...");
        mkdir("/sdcard/channels", 0755);
        ESP_LOGW(TAG, "No channels found. Please add video files to /sdcard/channels/<channel_name>/");
        return ESP_OK;
    }

    struct dirent *entry;

    // Scan for channel directories
    while ((entry = readdir(dir)) != NULL && manager->channel_count < MAX_CHANNELS) {
        if (entry->d_type != DT_DIR) continue;  // Skip non-directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        channel_t *ch = &manager->channels[manager->channel_count];

        // Channel name
        strncpy(ch->name, entry->d_name, MAX_NAME_LEN - 1);

        // Channel path
        snprintf(ch->path, MAX_PATH_LEN, "/sdcard/channels/%s", entry->d_name);

        ESP_LOGI(TAG, "Channel %d: %s", manager->channel_count + 1, ch->name);

        // Scan for episodes in this channel
        if (scan_channel_episodes(ch, ch->path) == ESP_OK && ch->episode_count > 0) {
            ch->current_episode = 0;
            manager->channel_count++;
        } else {
            ESP_LOGW(TAG, "Channel '%s' has no episodes, skipping", ch->name);
        }
    }

    closedir(dir);

    ESP_LOGI(TAG, "Found %d channels with content", manager->channel_count);

    return ESP_OK;
}

/**
 * Get current channel
 */
const channel_t *channel_manager_get_current(const channel_manager_t *manager)
{
    if (manager == NULL || manager->channel_count == 0) return NULL;

    return &manager->channels[manager->current_channel];
}

/**
 * Switch to next channel
 */
esp_err_t channel_manager_next_channel(channel_manager_t *manager)
{
    if (manager == NULL || manager->channel_count == 0) return ESP_FAIL;

    manager->current_channel = (manager->current_channel + 1) % manager->channel_count;

    const channel_t *ch = channel_manager_get_current(manager);
    ESP_LOGI(TAG, "Switched to channel %d: %s", manager->current_channel + 1, ch->name);

    return ESP_OK;
}

/**
 * Switch to previous channel
 */
esp_err_t channel_manager_prev_channel(channel_manager_t *manager)
{
    if (manager == NULL || manager->channel_count == 0) return ESP_FAIL;

    if (manager->current_channel == 0) {
        manager->current_channel = manager->channel_count - 1;
    } else {
        manager->current_channel--;
    }

    const channel_t *ch = channel_manager_get_current(manager);
    ESP_LOGI(TAG, "Switched to channel %d: %s", manager->current_channel + 1, ch->name);

    return ESP_OK;
}

/**
 * Set current channel by index
 */
esp_err_t channel_manager_set_channel(channel_manager_t *manager, uint8_t channel_idx)
{
    if (manager == NULL || channel_idx >= manager->channel_count) {
        return ESP_ERR_INVALID_ARG;
    }

    manager->current_channel = channel_idx;

    const channel_t *ch = channel_manager_get_current(manager);
    ESP_LOGI(TAG, "Set to channel %d: %s", channel_idx + 1, ch->name);

    return ESP_OK;
}

/**
 * Get current episode
 */
const episode_t *channel_manager_get_current_episode(const channel_manager_t *manager)
{
    const channel_t *ch = channel_manager_get_current(manager);
    if (ch == NULL || ch->episode_count == 0) return NULL;

    return &ch->episodes[ch->current_episode];
}

/**
 * Next episode
 */
esp_err_t channel_manager_next_episode(channel_manager_t *manager)
{
    if (manager == NULL || manager->channel_count == 0) return ESP_FAIL;

    channel_t *ch = &manager->channels[manager->current_channel];
    if (ch->episode_count == 0) return ESP_FAIL;

    ch->current_episode = (ch->current_episode + 1) % ch->episode_count;

    const episode_t *ep = &ch->episodes[ch->current_episode];
    ESP_LOGI(TAG, "Next episode: %s", ep->name);

    return ESP_OK;
}

/**
 * Get channel count
 */
uint8_t channel_manager_get_channel_count(const channel_manager_t *manager)
{
    return (manager != NULL) ? manager->channel_count : 0;
}
