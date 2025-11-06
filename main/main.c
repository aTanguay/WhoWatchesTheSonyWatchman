/**
 * Sony Watchman ESP32 Retro Media Player
 * Main Application Entry Point
 *
 * Integrates all components: display, video/audio playback, input, power management
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

// Component headers
#include "display.h"
#include "video_player.h"
#include "audio_player.h"
#include "sd_card.h"
#include "channel_manager.h"
#include "rotary_encoder.h"
#include "power_manager.h"

static const char *TAG = "WATCHMAN";

// Global component handles
static sd_card_handle_t g_sd_card;
static channel_manager_t g_channel_mgr;
static video_player_t *g_video_player = NULL;
static audio_player_t *g_audio_player = NULL;
static encoder_t *g_encoder = NULL;
static power_manager_t *g_power_mgr = NULL;

// State variables
static bool g_playback_active = false;
static uint32_t g_current_position_sec = 0;
static nvs_handle_t g_nvs_handle;
static bool g_channel_switching = false;

// OSD state
static bool g_show_osd = false;
static uint32_t g_osd_hide_time = 0;
#define OSD_DISPLAY_DURATION_MS 2000

// NVS keys
#define NVS_NAMESPACE "watchman"
#define NVS_KEY_CHANNEL "channel"
#define NVS_KEY_EPISODE "episode"
#define NVS_KEY_POSITION "position"

/**
 * Save current state to NVS
 */
static void save_state(void)
{
    if (g_nvs_handle == 0) return;

    uint8_t channel = g_channel_mgr.current_channel;
    const channel_t *ch = channel_manager_get_current(&g_channel_mgr);
    uint8_t episode = ch ? ch->current_episode : 0;

    nvs_set_u8(g_nvs_handle, NVS_KEY_CHANNEL, channel);
    nvs_set_u8(g_nvs_handle, NVS_KEY_EPISODE, episode);
    nvs_set_u32(g_nvs_handle, NVS_KEY_POSITION, g_current_position_sec);
    nvs_commit(g_nvs_handle);

    ESP_LOGI(TAG, "State saved: CH=%d EP=%d POS=%lu", channel, episode, g_current_position_sec);
}

/**
 * Load saved state from NVS
 */
static void load_state(void)
{
    if (g_nvs_handle == 0) return;

    uint8_t channel = 0;
    uint8_t episode = 0;
    uint32_t position = 0;

    nvs_get_u8(g_nvs_handle, NVS_KEY_CHANNEL, &channel);
    nvs_get_u8(g_nvs_handle, NVS_KEY_EPISODE, &episode);
    nvs_get_u32(g_nvs_handle, NVS_KEY_POSITION, &position);

    // Validate and restore
    if (channel < g_channel_mgr.channel_count) {
        channel_manager_set_channel(&g_channel_mgr, channel);
        const channel_t *ch = channel_manager_get_current(&g_channel_mgr);
        if (ch && episode < ch->episode_count) {
            ch->current_episode = episode;  // Direct assignment - not ideal but works
        }
        g_current_position_sec = position;

        ESP_LOGI(TAG, "State loaded: CH=%d EP=%d POS=%lu", channel, episode, position);
    }
}

/**
 * Draw OSD overlay (channel name, battery, etc.)
 */
static void draw_osd(void)
{
    if (!g_show_osd) return;

    const channel_t *ch = channel_manager_get_current(&g_channel_mgr);
    if (!ch) return;

    // Draw semi-transparent background bar at top
    display_fill_rect(0, 0, DISPLAY_WIDTH, 30, COLOR_DARK_GRAY);

    // TODO: Add text rendering library to draw channel name and battery icon
    // For now, just show colored indicators

    // Channel number indicator (simple colored square)
    uint16_t channel_color = COLOR_CYAN;
    display_fill_rect(10, 5, 20, 20, channel_color);

    // Battery indicator (right side)
    battery_level_t bat_level = power_manager_get_battery_level(g_power_mgr);
    uint16_t bat_color = COLOR_GREEN;
    if (bat_level == BATTERY_LEVEL_LOW) bat_color = COLOR_YELLOW;
    if (bat_level == BATTERY_LEVEL_CRITICAL) bat_color = COLOR_RED;
    display_fill_rect(DISPLAY_WIDTH - 30, 5, 20, 20, bat_color);

    // Auto-hide after timeout
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (now > g_osd_hide_time) {
        g_show_osd = false;
    }
}

/**
 * Show OSD for a few seconds
 */
static void show_osd(void)
{
    g_show_osd = true;
    g_osd_hide_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) + OSD_DISPLAY_DURATION_MS;
}

/**
 * Video player callbacks
 */
static void on_frame_decoded(void *user_data, uint32_t frame_num)
{
    // Update current frame/position
    video_info_t info;
    if (video_player_get_info(g_video_player, &info) == ESP_OK && info.fps > 0) {
        g_current_position_sec = frame_num / info.fps;
    }
}

static void on_playback_complete(void *user_data)
{
    ESP_LOGI(TAG, "Episode complete - advancing to next");

    // Auto-advance to next episode
    channel_manager_next_episode(&g_channel_mgr);
    const episode_t *ep = channel_manager_get_current_episode(&g_channel_mgr);

    if (ep) {
        ESP_LOGI(TAG, "Starting next episode: %s", ep->name);
        video_player_close(g_video_player);
        if (video_player_open(g_video_player, ep->path) == ESP_OK) {
            g_current_position_sec = 0;
            video_player_play(g_video_player);
            audio_player_start(g_audio_player);
            save_state();
        }
    } else {
        ESP_LOGW(TAG, "No more episodes in channel");
        g_playback_active = false;
    }
}

static void on_video_error(void *user_data, esp_err_t error)
{
    ESP_LOGE(TAG, "Video playback error: %d", error);
    g_playback_active = false;
}

/**
 * Encoder event handler
 */
static void encoder_callback(const encoder_event_t *event, void *user_data)
{
    // Reset power idle timer on any input
    power_manager_reset_idle_timer(g_power_mgr);

    switch (event->type) {
        case ENCODER_EVENT_ROTATE_CW:
            ESP_LOGI(TAG, "Encoder CW - Next channel");
            g_channel_switching = true;

            // Stop current playback
            if (g_playback_active) {
                video_player_stop(g_video_player);
                audio_player_stop(g_audio_player);
            }

            // Switch channel
            channel_manager_next_channel(&g_channel_mgr);
            g_current_position_sec = 0;
            save_state();
            show_osd();

            // Start new channel after brief delay
            vTaskDelay(pdMS_TO_TICKS(200));

            // Load and play first episode of new channel
            const episode_t *ep = channel_manager_get_current_episode(&g_channel_mgr);
            if (ep) {
                video_player_close(g_video_player);
                if (video_player_open(g_video_player, ep->path) == ESP_OK) {
                    video_player_play(g_video_player);
                    audio_player_start(g_audio_player);
                    g_playback_active = true;
                }
            }

            g_channel_switching = false;
            break;

        case ENCODER_EVENT_ROTATE_CCW:
            ESP_LOGI(TAG, "Encoder CCW - Previous channel");
            g_channel_switching = true;

            // Stop current playback
            if (g_playback_active) {
                video_player_stop(g_video_player);
                audio_player_stop(g_audio_player);
            }

            // Switch channel
            channel_manager_prev_channel(&g_channel_mgr);
            g_current_position_sec = 0;
            save_state();
            show_osd();

            // Start new channel after brief delay
            vTaskDelay(pdMS_TO_TICKS(200));

            // Load and play first episode of new channel
            const episode_t *ep2 = channel_manager_get_current_episode(&g_channel_mgr);
            if (ep2) {
                video_player_close(g_video_player);
                if (video_player_open(g_video_player, ep2->path) == ESP_OK) {
                    video_player_play(g_video_player);
                    audio_player_start(g_audio_player);
                    g_playback_active = true;
                }
            }

            g_channel_switching = false;
            break;

        case ENCODER_EVENT_BUTTON_PRESS:
            ESP_LOGI(TAG, "Encoder button - Toggle pause");
            if (g_playback_active) {
                video_state_t state = video_player_get_state(g_video_player);
                if (state == VIDEO_STATE_PLAYING) {
                    video_player_pause(g_video_player);
                    audio_player_pause(g_audio_player);
                    show_osd();
                } else if (state == VIDEO_STATE_PAUSED) {
                    video_player_play(g_video_player);
                    audio_player_resume(g_audio_player);
                }
            }
            break;

        case ENCODER_EVENT_BUTTON_LONG_PRESS:
            ESP_LOGI(TAG, "Encoder long press - Next episode");
            if (g_playback_active) {
                on_playback_complete(NULL);  // Reuse episode advance logic
            }
            break;

        default:
            break;
    }
}

/**
 * Power event handler
 */
static void power_callback(battery_level_t level, void *user_data)
{
    ESP_LOGW(TAG, "Battery level changed: %d", level);

    if (level == BATTERY_LEVEL_CRITICAL) {
        ESP_LOGE(TAG, "CRITICAL BATTERY - Saving state and shutting down");
        save_state();

        // Show critical battery warning
        display_clear(COLOR_RED);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Deep sleep
        power_manager_deep_sleep(g_power_mgr, PIN_ENCODER_SW, 0);
    } else if (level == BATTERY_LEVEL_LOW) {
        // Show low battery warning on OSD
        show_osd();
    }
}

/**
 * Initialize all hardware components
 */
static esp_err_t init_hardware(void)
{
    ESP_LOGI(TAG, "Initializing hardware...");
    esp_err_t ret;

    // Initialize NVS (for storing state)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Open NVS handle
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &g_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %d", ret);
        g_nvs_handle = 0;
    }

    // 1. Initialize display (show splash screen)
    ESP_LOGI(TAG, "Initializing display...");
    ret = display_init(NULL);  // Use default config
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display init failed: %d", ret);
        return ret;
    }

    // Show splash screen
    display_clear(COLOR_BLACK);
    display_fill_rect(60, 140, 120, 40, COLOR_CYAN);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 2. Initialize SD card
    ESP_LOGI(TAG, "Mounting SD card...");
    ret = sd_card_init(&g_sd_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD card init failed: %d", ret);
        display_clear(COLOR_RED);
        return ret;
    }

    uint32_t total_mb, free_mb;
    sd_card_get_info(&g_sd_card, &total_mb, &free_mb);
    ESP_LOGI(TAG, "SD Card: %lu MB total, %lu MB free", total_mb, free_mb);

    // 3. Initialize power manager
    ESP_LOGI(TAG, "Initializing power manager...");
    g_power_mgr = power_manager_init(NULL);  // Use defaults
    if (!g_power_mgr) {
        ESP_LOGE(TAG, "Power manager init failed");
        return ESP_FAIL;
    }

    power_manager_set_callback(g_power_mgr, power_callback, NULL);
    battery_level_t bat = power_manager_get_battery_level(g_power_mgr);
    ESP_LOGI(TAG, "Battery: %d%% (%d)",
             power_manager_get_battery_percentage(g_power_mgr), bat);

    // 4. Initialize audio
    ESP_LOGI(TAG, "Initializing audio...");
    g_audio_player = audio_player_init(NULL);  // Use defaults
    if (!g_audio_player) {
        ESP_LOGE(TAG, "Audio player init failed");
        return ESP_FAIL;
    }
    audio_player_set_volume(g_audio_player, 80);  // 80% volume

    // 5. Initialize rotary encoder
    ESP_LOGI(TAG, "Initializing encoder...");
    encoder_config_t enc_config = {
        .pin_clk = PIN_ENCODER_CLK,
        .pin_dt = PIN_ENCODER_DT,
        .pin_sw = PIN_ENCODER_SW,
        .callback = encoder_callback,
        .user_data = NULL
    };
    g_encoder = encoder_init(&enc_config);
    if (!g_encoder) {
        ESP_LOGE(TAG, "Encoder init failed");
        return ESP_FAIL;
    }

    // 6. Initialize channel manager and scan for content
    ESP_LOGI(TAG, "Scanning for channels...");
    ret = channel_manager_init(&g_channel_mgr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Channel manager init failed: %d", ret);
        return ret;
    }

    ret = channel_manager_scan(&g_channel_mgr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Channel scan failed: %d", ret);
        return ret;
    }

    uint8_t ch_count = channel_manager_get_channel_count(&g_channel_mgr);
    ESP_LOGI(TAG, "Found %d channels", ch_count);

    if (ch_count == 0) {
        ESP_LOGE(TAG, "No channels found on SD card!");
        display_clear(COLOR_RED);
        return ESP_FAIL;
    }

    // 7. Initialize video player
    ESP_LOGI(TAG, "Initializing video player...");
    video_callbacks_t vid_callbacks = {
        .on_frame_decoded = on_frame_decoded,
        .on_playback_complete = on_playback_complete,
        .on_error = on_video_error
    };
    g_video_player = video_player_create(&vid_callbacks, NULL);
    if (!g_video_player) {
        ESP_LOGE(TAG, "Video player creation failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Hardware initialization complete");
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    return ESP_OK;
}

/**
 * Start playback of current channel/episode
 */
static esp_err_t start_playback(void)
{
    const episode_t *ep = channel_manager_get_current_episode(&g_channel_mgr);
    if (!ep) {
        ESP_LOGE(TAG, "No episode to play");
        return ESP_FAIL;
    }

    const channel_t *ch = channel_manager_get_current(&g_channel_mgr);
    ESP_LOGI(TAG, "Starting playback: %s - %s", ch ? ch->name : "?", ep->name);

    // Open video file
    esp_err_t ret = video_player_open(g_video_player, ep->path);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open video: %s", ep->path);
        return ret;
    }

    // Seek to saved position if resuming
    if (g_current_position_sec > 0) {
        video_info_t info;
        if (video_player_get_info(g_video_player, &info) == ESP_OK) {
            uint32_t frame = g_current_position_sec * info.fps;
            video_player_seek(g_video_player, frame);
            ESP_LOGI(TAG, "Resumed from %lu seconds", g_current_position_sec);
        }
    }

    // Start playback
    ret = video_player_play(g_video_player);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start video playback");
        return ret;
    }

    ret = audio_player_start(g_audio_player);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start audio playback");
    }

    g_playback_active = true;
    show_osd();

    return ESP_OK;
}

/**
 * Main application task
 */
static void app_main_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Sony Watchman starting...");

    // Initialize all hardware
    if (init_hardware() != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed!");
        display_clear(COLOR_RED);
        vTaskDelete(NULL);
        return;
    }

    // Load saved state
    load_state();

    // Start playback
    if (start_playback() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start initial playback");
    }

    // Main event loop
    uint32_t last_save_time = 0;
    uint32_t last_heap_check = 0;

    while (1) {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Periodically save state (every 30 seconds)
        if (g_playback_active && (current_time - last_save_time > 30000)) {
            save_state();
            last_save_time = current_time;
        }

        // Draw OSD if needed
        if (g_show_osd && !g_channel_switching) {
            draw_osd();
        }

        // Monitor free heap for memory leaks (every 10 seconds)
        if (current_time - last_heap_check > 10000) {
            ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
            last_heap_check = current_time;

            // Check battery
            uint8_t bat_pct = power_manager_get_battery_percentage(g_power_mgr);
            ESP_LOGI(TAG, "Battery: %d%%", bat_pct);
        }

        // Check for auto-dim/sleep
        uint32_t idle_time = power_manager_get_idle_time(g_power_mgr);
        if (idle_time > AUTO_DIM_IDLE_MS) {
            display_set_brightness(30);  // Dim to 30%
        } else {
            display_set_brightness(100);  // Full brightness
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

/**
 * Application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Sony Watchman Retro Media Player");
    ESP_LOGI(TAG, "  ESP-IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");

    // Create main application task on core 1
    // (Video decoding will run on core 0)
    xTaskCreatePinnedToCore(
        app_main_task,
        "app_main",
        8192,           // Stack size
        NULL,           // Parameters
        5,              // Priority
        NULL,           // Task handle
        1               // Core 1
    );
}
