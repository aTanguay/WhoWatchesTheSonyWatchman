/**
 * Display Driver Interface
 * High-level display abstraction for Sony Watchman
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/spi_master.h"

// Display configuration based on 2" ST7789 IPS display (240x320)
#define DISPLAY_WIDTH       240
#define DISPLAY_HEIGHT      320
#define DISPLAY_ORIENTATION 0  // 0=portrait, 1=landscape, 2=portrait inverted, 3=landscape inverted

// Pin definitions (adjust these for your specific wiring)
#define PIN_DISPLAY_MOSI    19  // Changed from 23 to avoid board labeling issues
#define PIN_DISPLAY_CLK     18
#define PIN_DISPLAY_CS      5
#define PIN_DISPLAY_DC      16
#define PIN_DISPLAY_RST     4
#define PIN_DISPLAY_BL      15  // Backlight (optional, can use PWM)

// SPI configuration
#define DISPLAY_SPI_HOST    SPI2_HOST
#define DISPLAY_SPI_CLOCK   26000000  // 26MHz (ESP32 limit is 26.666MHz for this configuration)

// RGB565 color definitions
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        0xFFE0
#define COLOR_CYAN          0x07FF
#define COLOR_MAGENTA       0xF81F
#define COLOR_GRAY          0x8410
#define COLOR_DARK_GRAY     0x4208

/**
 * RGB888 to RGB565 conversion macro
 */
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

/**
 * Display initialization configuration
 */
typedef struct {
    int pin_mosi;
    int pin_clk;
    int pin_cs;
    int pin_dc;
    int pin_rst;
    int pin_bl;
    int spi_clock_hz;
    uint8_t orientation;
} display_config_t;

/**
 * Frame buffer structure for double buffering
 */
typedef struct {
    uint16_t *buffer;         // RGB565 pixel data
    uint16_t width;
    uint16_t height;
    bool ready;               // True when buffer is ready for display
} frame_buffer_t;

/**
 * Initialize display with given configuration
 * If config is NULL, uses default pin assignments
 *
 * @param config Display configuration (NULL for defaults)
 * @return ESP_OK on success
 */
esp_err_t display_init(const display_config_t *config);

/**
 * Clear entire display to specified color
 *
 * @param color RGB565 color value
 */
void display_clear(uint16_t color);

/**
 * Draw single pixel at coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color value
 */
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * Draw filled rectangle
 *
 * @param x X coordinate (top-left)
 * @param y Y coordinate (top-left)
 * @param w Width
 * @param h Height
 * @param color RGB565 color value
 */
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * Write raw RGB565 buffer to display region
 * This is the main function for video frame display
 *
 * @param x X coordinate (top-left)
 * @param y Y coordinate (top-left)
 * @param w Width
 * @param h Height
 * @param buffer RGB565 pixel data
 */
void display_write_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *buffer);

/**
 * Write frame buffer to display using DMA
 * Non-blocking operation
 *
 * @param fb Frame buffer to display
 * @return ESP_OK on success
 */
esp_err_t display_write_frame_dma(const frame_buffer_t *fb);

/**
 * Wait for DMA transfer to complete
 */
void display_wait_dma(void);

/**
 * Set display backlight brightness
 *
 * @param brightness 0-100 (percentage)
 */
void display_set_brightness(uint8_t brightness);

/**
 * Put display to sleep (low power mode)
 */
void display_sleep(void);

/**
 * Wake display from sleep
 */
void display_wake(void);

/**
 * Get display width in current orientation
 */
uint16_t display_get_width(void);

/**
 * Get display height in current orientation
 */
uint16_t display_get_height(void);

/**
 * Allocate frame buffer for video
 *
 * @param width Frame width
 * @param height Frame height
 * @return Allocated frame buffer or NULL on failure
 */
frame_buffer_t *display_alloc_frame_buffer(uint16_t width, uint16_t height);

/**
 * Free frame buffer
 *
 * @param fb Frame buffer to free
 */
void display_free_frame_buffer(frame_buffer_t *fb);

#endif // DISPLAY_H
