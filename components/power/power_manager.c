/**
 * Power Manager Implementation
 *
 * Tasks: T3.9-T3.18
 */

#include "power_manager.h"
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "POWER_MGR";

#define ADC_ATTEN           ADC_ATTEN_DB_12  // 0-3.3V range
#define ADC_UNIT            ADC_UNIT_1
#define ADC_CHANNEL         ADC_CHANNEL_6    // GPIO34
#define VOLTAGE_SAMPLES     10               // Average over multiple samples

/**
 * Power manager structure
 */
struct power_manager_s {
    power_config_t config;
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;

    uint32_t battery_voltage_mv;
    battery_level_t battery_level;
    power_state_t state;

    uint64_t last_activity_time;
    bool auto_sleep_enabled;
    bool auto_dim_enabled;

    power_event_callback_t callback;
    void *user_data;

    TaskHandle_t monitor_task;
    bool initialized;
};

/**
 * Convert voltage to battery percentage
 */
static uint8_t voltage_to_percentage(uint32_t voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_FULL) {
        return 100;
    } else if (voltage_mv <= BATTERY_VOLTAGE_EMPTY) {
        return 0;
    }

    // Linear interpolation between empty and full
    uint32_t range = BATTERY_VOLTAGE_FULL - BATTERY_VOLTAGE_EMPTY;
    uint32_t value = voltage_mv - BATTERY_VOLTAGE_EMPTY;

    return (value * 100) / range;
}

/**
 * Determine battery level from voltage
 */
static battery_level_t voltage_to_level(uint32_t voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_GOOD) {
        return BATTERY_LEVEL_FULL;
    } else if (voltage_mv >= BATTERY_VOLTAGE_LOW) {
        return BATTERY_LEVEL_GOOD;
    } else if (voltage_mv >= BATTERY_VOLTAGE_CRITICAL) {
        return BATTERY_LEVEL_LOW;
    } else if (voltage_mv >= BATTERY_VOLTAGE_EMPTY) {
        return BATTERY_LEVEL_CRITICAL;
    } else {
        return BATTERY_LEVEL_UNKNOWN;
    }
}

/**
 * Read and average ADC samples
 */
static uint32_t read_battery_voltage_internal(power_manager_t *pm)
{
    int adc_raw;
    int voltage_mv = 0;
    uint32_t total = 0;

    // Read multiple samples and average
    for (int i = 0; i < VOLTAGE_SAMPLES; i++) {
        if (adc_oneshot_read(pm->adc_handle, ADC_CHANNEL, &adc_raw) == ESP_OK) {
            if (pm->cali_handle) {
                adc_cali_raw_to_voltage(pm->cali_handle, adc_raw, &voltage_mv);
                total += voltage_mv;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));  // Small delay between samples
    }

    // Average and apply voltage divider ratio
    uint32_t avg_mv = total / VOLTAGE_SAMPLES;
    uint32_t battery_mv = avg_mv * VOLTAGE_DIVIDER_RATIO;

    return battery_mv;
}

/**
 * Battery monitoring task
 */
static void power_monitor_task(void *pvParameters)
{
    power_manager_t *pm = (power_manager_t *)pvParameters;

    ESP_LOGI(TAG, "Power monitor task started");

    while (1) {
        // Read battery voltage
        uint32_t new_voltage = read_battery_voltage_internal(pm);
        battery_level_t new_level = voltage_to_level(new_voltage);

        // Update voltage (smoothing)
        pm->battery_voltage_mv = (pm->battery_voltage_mv * 9 + new_voltage) / 10;

        // Check for level change
        if (new_level != pm->battery_level) {
            battery_level_t old_level = pm->battery_level;
            pm->battery_level = new_level;

            ESP_LOGI(TAG, "Battery level changed: %d -> %d (%lu mV, %d%%)",
                     old_level, new_level, pm->battery_voltage_mv,
                     voltage_to_percentage(pm->battery_voltage_mv));

            // Call callback if set
            if (pm->callback) {
                pm->callback(new_level, pm->user_data);
            }

            // Warn on low battery
            if (new_level == BATTERY_LEVEL_CRITICAL) {
                ESP_LOGW(TAG, "CRITICAL BATTERY LEVEL!");
            } else if (new_level == BATTERY_LEVEL_LOW) {
                ESP_LOGW(TAG, "Low battery warning");
            }
        }

        // Check idle timer for auto-sleep/dim
        uint64_t now = esp_timer_get_time();
        uint32_t idle_time_ms = (now - pm->last_activity_time) / 1000;

        if (pm->auto_dim_enabled && pm->state == POWER_STATE_ACTIVE) {
            if (idle_time_ms >= pm->config.auto_dim_timeout_ms) {
                ESP_LOGI(TAG, "Auto-dimming display");
                pm->state = POWER_STATE_DIMMED;
                // TODO: Signal to display to reduce brightness
            }
        }

        if (pm->auto_sleep_enabled && pm->state != POWER_STATE_LIGHT_SLEEP) {
            if (idle_time_ms >= pm->config.auto_sleep_timeout_ms) {
                ESP_LOGI(TAG, "Entering auto-sleep");
                // TODO: Signal to main app to save state and sleep
            }
        }

        // Monitor every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * Initialize power manager
 */
power_manager_t *power_manager_init(const power_config_t *config)
{
    ESP_LOGI(TAG, "Initializing power manager...");

    power_manager_t *pm = malloc(sizeof(power_manager_t));
    if (pm == NULL) {
        ESP_LOGE(TAG, "Failed to allocate power manager");
        return NULL;
    }

    memset(pm, 0, sizeof(power_manager_t));

    // Use default config if none provided
    if (config) {
        memcpy(&pm->config, config, sizeof(power_config_t));
    } else {
        pm->config.pin_battery_voltage = PIN_BATTERY_VOLTAGE;
        pm->config.auto_sleep_timeout_ms = AUTO_SLEEP_IDLE_MS;
        pm->config.auto_dim_timeout_ms = AUTO_DIM_IDLE_MS;
        pm->config.enable_auto_sleep = true;
        pm->config.enable_auto_dim = true;
    }

    pm->auto_sleep_enabled = pm->config.enable_auto_sleep;
    pm->auto_dim_enabled = pm->config.enable_auto_dim;
    pm->state = POWER_STATE_ACTIVE;
    pm->last_activity_time = esp_timer_get_time();

    // Configure ADC
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT,
    };

    esp_err_t ret = adc_oneshot_new_unit(&adc_config, &pm->adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC: %s", esp_err_to_name(ret));
        free(pm);
        return NULL;
    }

    // Configure ADC channel
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_oneshot_config_channel(pm->adc_handle, ADC_CHANNEL, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(ret));
        adc_oneshot_del_unit(pm->adc_handle);
        free(pm);
        return NULL;
    }

    // Initialize ADC calibration
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &pm->cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC calibration failed, readings may be inaccurate");
        pm->cali_handle = NULL;
    }

    // Read initial battery voltage
    pm->battery_voltage_mv = read_battery_voltage_internal(pm);
    pm->battery_level = voltage_to_level(pm->battery_voltage_mv);

    ESP_LOGI(TAG, "Initial battery: %lu mV (%d%%)",
             pm->battery_voltage_mv,
             voltage_to_percentage(pm->battery_voltage_mv));

    // Create monitoring task
    xTaskCreate(power_monitor_task, "power_monitor", 3072, pm, 3, &pm->monitor_task);

    pm->initialized = true;

    ESP_LOGI(TAG, "Power manager initialized");

    return pm;
}

/**
 * Deinitialize power manager
 */
void power_manager_deinit(power_manager_t *pm)
{
    if (pm == NULL) return;

    if (pm->monitor_task) {
        vTaskDelete(pm->monitor_task);
    }

    if (pm->cali_handle) {
        adc_cali_delete_scheme_curve_fitting(pm->cali_handle);
    }

    if (pm->adc_handle) {
        adc_oneshot_del_unit(pm->adc_handle);
    }

    free(pm);

    ESP_LOGI(TAG, "Power manager deinitialized");
}

/**
 * Read battery voltage
 */
uint32_t power_manager_read_battery_voltage(power_manager_t *pm)
{
    return pm ? pm->battery_voltage_mv : 0;
}

/**
 * Get battery level
 */
battery_level_t power_manager_get_battery_level(power_manager_t *pm)
{
    return pm ? pm->battery_level : BATTERY_LEVEL_UNKNOWN;
}

/**
 * Get battery percentage
 */
uint8_t power_manager_get_battery_percentage(power_manager_t *pm)
{
    if (!pm) return 0;
    return voltage_to_percentage(pm->battery_voltage_mv);
}

/**
 * Check if charging
 */
bool power_manager_is_charging(power_manager_t *pm)
{
    // TODO: Implement charging detection if hardware supports it
    // For now, return false
    return false;
}

/**
 * Reset idle timer
 */
void power_manager_reset_idle_timer(power_manager_t *pm)
{
    if (!pm) return;

    pm->last_activity_time = esp_timer_get_time();

    // Return to active state if dimmed
    if (pm->state == POWER_STATE_DIMMED) {
        pm->state = POWER_STATE_ACTIVE;
        // TODO: Signal to display to restore brightness
    }
}

/**
 * Get idle time
 */
uint32_t power_manager_get_idle_time(power_manager_t *pm)
{
    if (!pm) return 0;

    uint64_t now = esp_timer_get_time();
    return (now - pm->last_activity_time) / 1000;
}

/**
 * Light sleep
 */
esp_err_t power_manager_light_sleep(power_manager_t *pm, uint32_t duration_ms)
{
    if (!pm) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Entering light sleep for %lu ms", duration_ms);

    pm->state = POWER_STATE_LIGHT_SLEEP;

    // Configure sleep timer
    esp_sleep_enable_timer_wakeup(duration_ms * 1000);

    // Enter light sleep
    esp_err_t ret = esp_light_sleep_start();

    pm->state = POWER_STATE_ACTIVE;
    pm->last_activity_time = esp_timer_get_time();

    ESP_LOGI(TAG, "Woke from light sleep");

    return ret;
}

/**
 * Deep sleep
 */
esp_err_t power_manager_deep_sleep(power_manager_t *pm, int wakeup_pin, uint32_t duration_ms)
{
    if (!pm) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Entering deep sleep");

    pm->state = POWER_STATE_DEEP_SLEEP;

    // Configure wakeup sources
    if (duration_ms > 0) {
        esp_sleep_enable_timer_wakeup(duration_ms * 1000);
    }

    if (wakeup_pin >= 0) {
        esp_sleep_enable_ext0_wakeup(wakeup_pin, 0);  // Wake on LOW
    }

    // Enter deep sleep (will not return)
    esp_deep_sleep_start();

    return ESP_OK;  // Never reached
}

/**
 * Set callback
 */
void power_manager_set_callback(power_manager_t *pm, power_event_callback_t callback,
                                 void *user_data)
{
    if (!pm) return;

    pm->callback = callback;
    pm->user_data = user_data;
}

/**
 * Get state
 */
power_state_t power_manager_get_state(const power_manager_t *pm)
{
    return pm ? pm->state : POWER_STATE_ACTIVE;
}

/**
 * Set auto-sleep
 */
void power_manager_set_auto_sleep(power_manager_t *pm, bool enable)
{
    if (pm) {
        pm->auto_sleep_enabled = enable;
        ESP_LOGI(TAG, "Auto-sleep %s", enable ? "enabled" : "disabled");
    }
}

/**
 * Set auto-dim
 */
void power_manager_set_auto_dim(power_manager_t *pm, bool enable)
{
    if (pm) {
        pm->auto_dim_enabled = enable;
        ESP_LOGI(TAG, "Auto-dim %s", enable ? "enabled" : "disabled");
    }
}
