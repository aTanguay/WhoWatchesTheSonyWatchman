/**
 * Video Player Implementation
 * High-level video playback engine
 */

#include "video_player.h"
#include "mjpeg_decoder.h"
#include "avi_parser.h"
#include "display.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "VIDEO_PLAYER";

#define PLAYBACK_TASK_STACK_SIZE    8192
#define PLAYBACK_TASK_PRIORITY      10
#define PLAYBACK_TASK_CORE          0  // Core 0 for video decoding

/**
 * Video player structure
 */
struct video_player_s {
    // State
    video_state_t state;
    video_info_t info;

    // AVI Parser
    avi_parser_t avi_parser;

    // Decoder
    mjpeg_decoder_t *decoder;

    // Frame buffers (double buffering)
    frame_buffer_t *frame_buffer[2];
    uint8_t current_buffer;

    // Playback control
    uint32_t current_frame;
    TaskHandle_t playback_task;

    // Callbacks
    video_callbacks_t callbacks;
    void *user_data;

    // Timing
    uint64_t frame_time_us;  // Microseconds per frame
    uint64_t last_frame_time;
};

/**
 * Playback task
 */
static void video_playback_task(void *pvParameters)
{
    video_player_t *player = (video_player_t *)pvParameters;
    esp_err_t ret;
    mjpeg_frame_t frame = {0};
    uint16_t width, height;

    ESP_LOGI(TAG, "Playback task started on core %d", xPortGetCoreID());

    // We use the pre-allocated frame buffers for double buffering
    // player->frame_buffer[0] and [1]
    
    // Ensure buffers are ready
    if (!player->frame_buffer[0] || !player->frame_buffer[1]) {
        ESP_LOGE(TAG, "Frame buffers not allocated");
        player->state = VIDEO_STATE_ERROR;
        vTaskDelete(NULL);
        return;
    }

    int decode_idx = 0;  // Index of buffer we are decoding INTO
    int display_idx = 1; // Index of buffer being displayed (initially unused)
    bool first_frame = true;

    while (player->state == VIDEO_STATE_PLAYING || player->state == VIDEO_STATE_PAUSED) {
        // Handle pause state
        if (player->state == VIDEO_STATE_PAUSED) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        uint64_t frame_start = esp_timer_get_time();

        // 1. Read next MJPEG frame from file
        
        // Wait, I can't abort a tool call once I've started typing it in the thought process? 
        // No, I can just change what I send.
        
        // I will switch to multi_replace_file_content to:
        // 1. Add #include "avi_parser.h"
        // 2. Update struct video_player_s
        // 3. Update video_player_open
        // 4. Update video_player_close
        // 5. Implement video_playback_task
        
        // Let's do that.
    }
}

/**
 * Create video player
 */
video_player_t *video_player_create(const video_callbacks_t *callbacks, void *user_data)
{
    ESP_LOGI(TAG, "Creating video player...");

    video_player_t *player = malloc(sizeof(video_player_t));
    if (player == NULL) {
        ESP_LOGE(TAG, "Failed to allocate video player");
        return NULL;
    }

    memset(player, 0, sizeof(video_player_t));
    player->state = VIDEO_STATE_STOPPED;

    // Set callbacks
    if (callbacks) {
        memcpy(&player->callbacks, callbacks, sizeof(video_callbacks_t));
    }
    player->user_data = user_data;

    // Create MJPEG decoder
    player->decoder = mjpeg_decoder_create(320, 240);  // Max resolution
    if (player->decoder == NULL) {
        ESP_LOGE(TAG, "Failed to create MJPEG decoder");
        free(player);
        return NULL;
    }

    // Allocate frame buffers for double buffering
    for (int i = 0; i < 2; i++) {
        player->frame_buffer[i] = display_alloc_frame_buffer(240, 240);
        if (player->frame_buffer[i] == NULL) {
            ESP_LOGE(TAG, "Failed to allocate frame buffer %d", i);
            video_player_destroy(player);
            return NULL;
        }
    }

    player->current_buffer = 0;

    ESP_LOGI(TAG, "Video player created successfully");
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    return player;
}

/**
 * Destroy video player
 */
void video_player_destroy(video_player_t *player)
{
    if (player == NULL) return;

    // Stop playback
    video_player_stop(player);

    // Close file
    video_player_close(player);

    // Free decoder
    if (player->decoder) {
        mjpeg_decoder_destroy(player->decoder);
    }

    // Free frame buffers
    for (int i = 0; i < 2; i++) {
        if (player->frame_buffer[i]) {
            display_free_frame_buffer(player->frame_buffer[i]);
        }
    }

    free(player);

    ESP_LOGI(TAG, "Video player destroyed");
}

/**
 * Open video file
 */
esp_err_t video_player_open(video_player_t *player, const char *file_path)
{
    if (player == NULL || file_path == NULL) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Opening video file: %s", file_path);

    // Close existing file
    video_player_close(player);

    // Open AVI file using parser
    esp_err_t ret = avi_parser_open(&player->avi_parser, file_path);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open AVI file: %s", file_path);
        return ret;
    }

    // Copy info from parser
    strncpy(player->info.path, file_path, sizeof(player->info.path) - 1);
    player->info.width = player->avi_parser.video_info.width;
    player->info.height = player->avi_parser.video_info.height;
    player->info.fps = (uint16_t)avi_parser_get_fps(&player->avi_parser);
    player->info.frame_count = avi_parser_get_total_frames(&player->avi_parser);
    player->info.duration_sec = player->info.frame_count / (player->info.fps > 0 ? player->info.fps : 15);

    // Use default FPS if parser returned 0
    if (player->info.fps == 0) player->info.fps = 15;

    player->frame_time_us = 1000000 / player->info.fps;
    player->current_frame = 0;

    ESP_LOGI(TAG, "Video opened: %dx%d @ %d fps, %d frames",
             player->info.width, player->info.height,
             player->info.fps, player->info.frame_count);

    return ESP_OK;
}

/**
 * Close video file
 */
void video_player_close(video_player_t *player)
{
    if (player == NULL) return;

    avi_parser_close(&player->avi_parser);

    player->current_frame = 0;
}

/**
 * Start playback
 */
esp_err_t video_player_play(video_player_t *player)
{
    if (player == NULL || !player->avi_parser.initialized) return ESP_FAIL;

    if (player->state == VIDEO_STATE_PLAYING) {
        ESP_LOGW(TAG, "Already playing");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting playback...");

    player->state = VIDEO_STATE_PLAYING;
    player->last_frame_time = esp_timer_get_time();

    // Create playback task on core 0
    BaseType_t ret = xTaskCreatePinnedToCore(
        video_playback_task,
        "video_playback",
        PLAYBACK_TASK_STACK_SIZE,
        player,
        PLAYBACK_TASK_PRIORITY,
        &player->playback_task,
        PLAYBACK_TASK_CORE
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create playback task");
        player->state = VIDEO_STATE_ERROR;
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * Pause playback
 */
esp_err_t video_player_pause(video_player_t *player)
{
    if (player == NULL) return ESP_FAIL;

    if (player->state == VIDEO_STATE_PLAYING) {
        ESP_LOGI(TAG, "Pausing playback at frame %lu", player->current_frame);
        player->state = VIDEO_STATE_PAUSED;
    }

    return ESP_OK;
}

/**
 * Stop playback
 */
esp_err_t video_player_stop(video_player_t *player)
{
    if (player == NULL) return ESP_FAIL;

    if (player->state == VIDEO_STATE_PLAYING || player->state == VIDEO_STATE_PAUSED) {
        ESP_LOGI(TAG, "Stopping playback");
        player->state = VIDEO_STATE_STOPPED;

        // Wait for playback task to finish
        if (player->playback_task) {
            vTaskDelay(pdMS_TO_TICKS(100));  // Give time for task to exit
            player->playback_task = NULL;
        }

        player->current_frame = 0;
    }

    return ESP_OK;
}

/**
 * Seek to frame
 */
esp_err_t video_player_seek(video_player_t *player, uint32_t frame_num)
{
    if (player == NULL || !player->avi_parser.initialized) return ESP_FAIL;

    esp_err_t ret = avi_parser_seek(&player->avi_parser, frame_num);
    if (ret == ESP_OK) {
        player->current_frame = frame_num;
    }
    return ret;
}

/**
 * Get playback state
 */
video_state_t video_player_get_state(const video_player_t *player)
{
    return (player != NULL) ? player->state : VIDEO_STATE_ERROR;
}

/**
 * Get video info
 */
esp_err_t video_player_get_info(const video_player_t *player, video_info_t *info)
{
    if (player == NULL || info == NULL) return ESP_ERR_INVALID_ARG;

    memcpy(info, &player->info, sizeof(video_info_t));

    return ESP_OK;
}

/**
 * Get current frame
 */
uint32_t video_player_get_current_frame(const video_player_t *player)
{
    return (player != NULL) ? player->current_frame : 0;
}

/**
 * Get position in seconds
 */
uint32_t video_player_get_position_sec(const video_player_t *player)
{
    if (player == NULL || player->info.fps == 0) return 0;

    return player->current_frame / player->info.fps;
}
