/**
 * Rotary Encoder Driver
 * Handles rotary encoder input with debouncing and interrupts
 *
 * Tasks: T2.7-T2.12 - Rotary encoder integration
 */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Pin definitions for KY-040 rotary encoder
#define PIN_ENCODER_CLK     32  // A phase
#define PIN_ENCODER_DT      33  // B phase
#define PIN_ENCODER_SW      27  // Switch/button

/**
 * Encoder rotation direction
 */
typedef enum {
    ENCODER_DIRECTION_CW = 1,   // Clockwise
    ENCODER_DIRECTION_CCW = -1, // Counter-clockwise
} encoder_direction_t;

/**
 * Encoder event type
 */
typedef enum {
    ENCODER_EVENT_ROTATE_CW,    // Rotated clockwise
    ENCODER_EVENT_ROTATE_CCW,   // Rotated counter-clockwise
    ENCODER_EVENT_BUTTON_PRESS, // Button pressed
    ENCODER_EVENT_BUTTON_RELEASE, // Button released
    ENCODER_EVENT_BUTTON_LONG_PRESS, // Long press detected
} encoder_event_type_t;

/**
 * Encoder event structure
 */
typedef struct {
    encoder_event_type_t type;
    int32_t position;           // Current encoder position
    uint32_t timestamp_ms;      // Event timestamp
} encoder_event_t;

/**
 * Encoder event callback
 */
typedef void (*encoder_callback_t)(const encoder_event_t *event, void *user_data);

/**
 * Encoder configuration
 */
typedef struct {
    int pin_clk;                // CLK pin (A phase)
    int pin_dt;                 // DT pin (B phase)
    int pin_sw;                 // Switch pin (-1 if not used)
    encoder_callback_t callback; // Event callback
    void *user_data;            // User data for callback
} encoder_config_t;

/**
 * Encoder handle
 */
typedef struct encoder_s encoder_t;

/**
 * Initialize rotary encoder
 *
 * @param config Encoder configuration
 * @return Encoder handle, or NULL on failure
 */
encoder_t *encoder_init(const encoder_config_t *config);

/**
 * Deinitialize rotary encoder
 *
 * @param encoder Encoder handle
 */
void encoder_deinit(encoder_t *encoder);

/**
 * Get current encoder position
 *
 * @param encoder Encoder handle
 * @return Current position (increments/decrements on rotation)
 */
int32_t encoder_get_position(const encoder_t *encoder);

/**
 * Reset encoder position to zero
 *
 * @param encoder Encoder handle
 */
void encoder_reset_position(encoder_t *encoder);

/**
 * Check if button is currently pressed
 *
 * @param encoder Encoder handle
 * @return true if button pressed
 */
bool encoder_is_button_pressed(const encoder_t *encoder);

/**
 * Set long press threshold in milliseconds
 *
 * @param encoder Encoder handle
 * @param threshold_ms Threshold in milliseconds
 */
void encoder_set_long_press_threshold(encoder_t *encoder, uint32_t threshold_ms);

#endif // ROTARY_ENCODER_H
