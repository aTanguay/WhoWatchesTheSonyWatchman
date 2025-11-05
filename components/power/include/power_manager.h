/**
 * Power Manager
 * Battery monitoring, voltage reading, and sleep management
 *
 * Tasks: T3.9-T3.18 - Power management
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Pin definitions
#define PIN_BATTERY_VOLTAGE     34  // ADC1 Channel 6 (GPIO34)

// Voltage thresholds (in millivolts)
// 2x 18650 in series = 6.0V - 8.4V range
#define BATTERY_VOLTAGE_FULL    8400  // 4.2V per cell
#define BATTERY_VOLTAGE_GOOD    7600  // 3.8V per cell
#define BATTERY_VOLTAGE_LOW     7000  // 3.5V per cell
#define BATTERY_VOLTAGE_CRITICAL 6600 // 3.3V per cell
#define BATTERY_VOLTAGE_EMPTY   6000  // 3.0V per cell

// Auto-sleep timing
#define AUTO_SLEEP_IDLE_MS      300000  // 5 minutes
#define AUTO_DIM_IDLE_MS        120000  // 2 minutes

// Voltage divider ratio (R1=10k, R2=10k = 2:1 ratio)
#define VOLTAGE_DIVIDER_RATIO   2.0f

/**
 * Battery level enumeration
 */
typedef enum {
    BATTERY_LEVEL_FULL,         // 100-80%
    BATTERY_LEVEL_GOOD,         // 80-50%
    BATTERY_LEVEL_MEDIUM,       // 50-20%
    BATTERY_LEVEL_LOW,          // 20-10%
    BATTERY_LEVEL_CRITICAL,     // <10%
    BATTERY_LEVEL_UNKNOWN,      // Cannot read
} battery_level_t;

/**
 * Power state
 */
typedef enum {
    POWER_STATE_ACTIVE,         // Normal operation
    POWER_STATE_DIMMED,         // Display dimmed
    POWER_STATE_LIGHT_SLEEP,    // Light sleep (quick wake)
    POWER_STATE_DEEP_SLEEP,     // Deep sleep (longer wake)
} power_state_t;

/**
 * Power manager configuration
 */
typedef struct {
    int pin_battery_voltage;
    uint32_t auto_sleep_timeout_ms;
    uint32_t auto_dim_timeout_ms;
    bool enable_auto_sleep;
    bool enable_auto_dim;
} power_config_t;

/**
 * Power event callback
 */
typedef void (*power_event_callback_t)(battery_level_t level, void *user_data);

/**
 * Power manager handle
 */
typedef struct power_manager_s power_manager_t;

/**
 * Initialize power manager
 *
 * @param config Power configuration (NULL for defaults)
 * @return Power manager handle, or NULL on failure
 */
power_manager_t *power_manager_init(const power_config_t *config);

/**
 * Deinitialize power manager
 *
 * @param pm Power manager handle
 */
void power_manager_deinit(power_manager_t *pm);

/**
 * Read battery voltage in millivolts
 *
 * @param pm Power manager handle
 * @return Battery voltage in mV
 */
uint32_t power_manager_read_battery_voltage(power_manager_t *pm);

/**
 * Get battery level
 *
 * @param pm Power manager handle
 * @return Battery level
 */
battery_level_t power_manager_get_battery_level(power_manager_t *pm);

/**
 * Get battery percentage (0-100)
 *
 * @param pm Power manager handle
 * @return Battery percentage
 */
uint8_t power_manager_get_battery_percentage(power_manager_t *pm);

/**
 * Check if battery is charging
 *
 * @param pm Power manager handle
 * @return true if charging
 */
bool power_manager_is_charging(power_manager_t *pm);

/**
 * Reset idle timer (call on user activity)
 *
 * @param pm Power manager handle
 */
void power_manager_reset_idle_timer(power_manager_t *pm);

/**
 * Get idle time in milliseconds
 *
 * @param pm Power manager handle
 * @return Idle time in ms
 */
uint32_t power_manager_get_idle_time(power_manager_t *pm);

/**
 * Enter light sleep mode
 *
 * @param pm Power manager handle
 * @param duration_ms Sleep duration in milliseconds
 * @return ESP_OK on success
 */
esp_err_t power_manager_light_sleep(power_manager_t *pm, uint32_t duration_ms);

/**
 * Enter deep sleep mode
 *
 * @param pm Power manager handle
 * @param wakeup_pin GPIO pin for wakeup (-1 for timer only)
 * @param duration_ms Sleep duration in milliseconds (0 for indefinite)
 * @return ESP_OK on success (will not return if entering deep sleep)
 */
esp_err_t power_manager_deep_sleep(power_manager_t *pm, int wakeup_pin, uint32_t duration_ms);

/**
 * Set battery level change callback
 *
 * @param pm Power manager handle
 * @param callback Callback function
 * @param user_data User data for callback
 */
void power_manager_set_callback(power_manager_t *pm, power_event_callback_t callback,
                                 void *user_data);

/**
 * Get current power state
 *
 * @param pm Power manager handle
 * @return Current power state
 */
power_state_t power_manager_get_state(const power_manager_t *pm);

/**
 * Enable/disable auto-sleep
 *
 * @param pm Power manager handle
 * @param enable true to enable, false to disable
 */
void power_manager_set_auto_sleep(power_manager_t *pm, bool enable);

/**
 * Enable/disable auto-dim
 *
 * @param pm Power manager handle
 * @param enable true to enable, false to disable
 */
void power_manager_set_auto_dim(power_manager_t *pm, bool enable);

#endif // POWER_MANAGER_H
