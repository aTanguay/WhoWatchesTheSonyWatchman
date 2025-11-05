/**
 * Rotary Encoder Driver Implementation
 * Uses GPIO interrupts with debouncing
 *
 * Tasks: T2.7-T2.12
 */

#include "rotary_encoder.h"
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "ENCODER";

#define DEBOUNCE_TIME_MS        5       // Debounce time for rotation
#define BUTTON_DEBOUNCE_MS      50      // Debounce time for button
#define LONG_PRESS_DEFAULT_MS   1000    // Default long press threshold
#define EVENT_QUEUE_SIZE        10

/**
 * Encoder structure
 */
struct encoder_s {
    encoder_config_t config;
    int32_t position;
    volatile uint8_t last_encoded;
    volatile uint64_t last_change_time;

    // Button state
    bool button_pressed;
    uint64_t button_press_time;
    uint32_t long_press_threshold;
    bool long_press_fired;

    // Event queue
    QueueHandle_t event_queue;
    TaskHandle_t event_task;
};

// Global encoder instance (for ISR access)
static encoder_t *g_encoder = NULL;

/**
 * GPIO ISR handler for encoder rotation
 */
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    encoder_t *encoder = (encoder_t *)arg;
    if (encoder == NULL) return;

    uint64_t now = esp_timer_get_time();

    // Debounce check
    if ((now - encoder->last_change_time) < (DEBOUNCE_TIME_MS * 1000)) {
        return;
    }

    encoder->last_change_time = now;

    // Read current state
    int clk = gpio_get_level(encoder->config.pin_clk);
    int dt = gpio_get_level(encoder->config.pin_dt);

    // Encode current state
    uint8_t encoded = (clk << 1) | dt;
    uint8_t sum = (encoder->last_encoded << 2) | encoded;

    // Determine direction based on state transition
    // Using standard gray code decoding
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        encoder->position++;

        // Send CW event
        encoder_event_t event = {
            .type = ENCODER_EVENT_ROTATE_CW,
            .position = encoder->position,
            .timestamp_ms = now / 1000
        };
        xQueueSendFromISR(encoder->event_queue, &event, NULL);
    }
    else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        encoder->position--;

        // Send CCW event
        encoder_event_t event = {
            .type = ENCODER_EVENT_ROTATE_CCW,
            .position = encoder->position,
            .timestamp_ms = now / 1000
        };
        xQueueSendFromISR(encoder->event_queue, &event, NULL);
    }

    encoder->last_encoded = encoded;
}

/**
 * GPIO ISR handler for button
 */
static void IRAM_ATTR button_isr_handler(void *arg)
{
    encoder_t *encoder = (encoder_t *)arg;
    if (encoder == NULL) return;

    uint64_t now = esp_timer_get_time();
    bool button_state = !gpio_get_level(encoder->config.pin_sw);  // Active low

    if (button_state && !encoder->button_pressed) {
        // Button pressed
        encoder->button_pressed = true;
        encoder->button_press_time = now;
        encoder->long_press_fired = false;

        encoder_event_t event = {
            .type = ENCODER_EVENT_BUTTON_PRESS,
            .position = encoder->position,
            .timestamp_ms = now / 1000
        };
        xQueueSendFromISR(encoder->event_queue, &event, NULL);
    }
    else if (!button_state && encoder->button_pressed) {
        // Button released
        encoder->button_pressed = false;

        encoder_event_t event = {
            .type = ENCODER_EVENT_BUTTON_RELEASE,
            .position = encoder->position,
            .timestamp_ms = now / 1000
        };
        xQueueSendFromISR(encoder->event_queue, &event, NULL);
    }
}

/**
 * Event processing task
 */
static void encoder_event_task(void *pvParameters)
{
    encoder_t *encoder = (encoder_t *)pvParameters;
    encoder_event_t event;

    while (1) {
        // Check for long press
        if (encoder->button_pressed && !encoder->long_press_fired) {
            uint64_t now = esp_timer_get_time();
            uint64_t press_duration = (now - encoder->button_press_time) / 1000;

            if (press_duration >= encoder->long_press_threshold) {
                encoder->long_press_fired = true;

                encoder_event_t long_press_event = {
                    .type = ENCODER_EVENT_BUTTON_LONG_PRESS,
                    .position = encoder->position,
                    .timestamp_ms = now / 1000
                };

                if (encoder->config.callback) {
                    encoder->config.callback(&long_press_event, encoder->config.user_data);
                }
            }
        }

        // Process queued events
        if (xQueueReceive(encoder->event_queue, &event, pdMS_TO_TICKS(10))) {
            if (encoder->config.callback) {
                encoder->config.callback(&event, encoder->config.user_data);
            }
        }
    }
}

/**
 * Initialize encoder
 */
encoder_t *encoder_init(const encoder_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Invalid config");
        return NULL;
    }

    ESP_LOGI(TAG, "Initializing rotary encoder...");

    encoder_t *encoder = malloc(sizeof(encoder_t));
    if (encoder == NULL) {
        ESP_LOGE(TAG, "Failed to allocate encoder");
        return NULL;
    }

    memset(encoder, 0, sizeof(encoder_t));
    memcpy(&encoder->config, config, sizeof(encoder_config_t));

    encoder->position = 0;
    encoder->long_press_threshold = LONG_PRESS_DEFAULT_MS;
    encoder->last_encoded = 0;
    encoder->last_change_time = 0;

    // Create event queue
    encoder->event_queue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(encoder_event_t));
    if (encoder->event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        free(encoder);
        return NULL;
    }

    // Configure CLK pin
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << config->pin_clk),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);

    // Configure DT pin
    io_conf.pin_bit_mask = (1ULL << config->pin_dt);
    gpio_config(&io_conf);

    // Configure SW pin if used
    if (config->pin_sw >= 0) {
        io_conf.pin_bit_mask = (1ULL << config->pin_sw);
        gpio_config(&io_conf);

        // Install ISR for button
        gpio_isr_handler_add(config->pin_sw, button_isr_handler, encoder);
    }

    // Install ISR service
    gpio_install_isr_service(0);

    // Add ISR handlers
    gpio_isr_handler_add(config->pin_clk, encoder_isr_handler, encoder);
    gpio_isr_handler_add(config->pin_dt, encoder_isr_handler, encoder);

    // Read initial state
    encoder->last_encoded = (gpio_get_level(config->pin_clk) << 1) |
                             gpio_get_level(config->pin_dt);

    // Create event processing task
    xTaskCreate(encoder_event_task, "encoder_event", 2048, encoder, 5, &encoder->event_task);

    g_encoder = encoder;

    ESP_LOGI(TAG, "Rotary encoder initialized (CLK=%d, DT=%d, SW=%d)",
             config->pin_clk, config->pin_dt, config->pin_sw);

    return encoder;
}

/**
 * Deinitialize encoder
 */
void encoder_deinit(encoder_t *encoder)
{
    if (encoder == NULL) return;

    // Remove ISR handlers
    gpio_isr_handler_remove(encoder->config.pin_clk);
    gpio_isr_handler_remove(encoder->config.pin_dt);
    if (encoder->config.pin_sw >= 0) {
        gpio_isr_handler_remove(encoder->config.pin_sw);
    }

    // Delete task
    if (encoder->event_task) {
        vTaskDelete(encoder->event_task);
    }

    // Delete queue
    if (encoder->event_queue) {
        vQueueDelete(encoder->event_queue);
    }

    free(encoder);
    g_encoder = NULL;

    ESP_LOGI(TAG, "Rotary encoder deinitialized");
}

/**
 * Get position
 */
int32_t encoder_get_position(const encoder_t *encoder)
{
    return encoder ? encoder->position : 0;
}

/**
 * Reset position
 */
void encoder_reset_position(encoder_t *encoder)
{
    if (encoder) {
        encoder->position = 0;
    }
}

/**
 * Check button state
 */
bool encoder_is_button_pressed(const encoder_t *encoder)
{
    return encoder ? encoder->button_pressed : false;
}

/**
 * Set long press threshold
 */
void encoder_set_long_press_threshold(encoder_t *encoder, uint32_t threshold_ms)
{
    if (encoder) {
        encoder->long_press_threshold = threshold_ms;
    }
}
