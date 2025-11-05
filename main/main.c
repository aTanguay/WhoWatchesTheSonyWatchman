/**
 * Sony Watchman ESP32 Retro Media Player
 * Main Application Entry Point
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Component headers (will be created)
// #include "display.h"
// #include "video_player.h"
// #include "audio_player.h"
// #include "sd_card.h"
// #include "channel_manager.h"
// #include "rotary_encoder.h"
// #include "power_manager.h"

static const char *TAG = "WATCHMAN";

/**
 * Initialize all hardware components
 */
static esp_err_t init_hardware(void)
{
    ESP_LOGI(TAG, "Initializing hardware...");

    // Initialize NVS (for storing state)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // TODO: Initialize components in order:
    // 1. Display (show splash screen)
    // 2. SD Card (mount filesystem)
    // 3. Power manager (check battery)
    // 4. Audio subsystem
    // 5. Input (rotary encoder, accelerometer)
    // 6. Channel manager (scan for content)

    ESP_LOGI(TAG, "Hardware initialization complete");
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

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
        vTaskDelete(NULL);
        return;
    }

    // TODO: Load last state from NVS (channel, position)

    // TODO: Start playback from saved position

    // Main event loop
    while (1) {
        // TODO: Handle user input events
        // - Rotary encoder rotation (channel change)
        // - Rotary encoder button press (pause/play)
        // - Shake detection (next episode)

        // TODO: Monitor battery level

        // TODO: Save state periodically

        // Monitor free heap for memory leaks
        static uint32_t last_heap_check = 0;
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (current_time - last_heap_check > 10000) {  // Every 10 seconds
            ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
            last_heap_check = current_time;
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
