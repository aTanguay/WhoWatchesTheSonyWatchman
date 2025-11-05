/**
 * Display Driver Implementation
 * High-level display abstraction
 */

#include "display.h"
#include "st7789.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

static const char *TAG = "DISPLAY";

// Global ST7789 handle
static st7789_handle_t g_st7789;
static bool g_initialized = false;

// SPI bus configuration
static void init_spi_bus(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,  // Not used for display
        .mosi_io_num = PIN_DISPLAY_MOSI,
        .sclk_io_num = PIN_DISPLAY_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2 + 8,  // Full frame + overhead
    };

    esp_err_t ret = spi_bus_initialize(DISPLAY_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
    }
}

/**
 * Initialize display with given configuration
 */
esp_err_t display_init(const display_config_t *config)
{
    if (g_initialized) {
        ESP_LOGW(TAG, "Display already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing display...");

    // Use default config if none provided
    display_config_t default_config = {
        .pin_mosi = PIN_DISPLAY_MOSI,
        .pin_clk = PIN_DISPLAY_CLK,
        .pin_cs = PIN_DISPLAY_CS,
        .pin_dc = PIN_DISPLAY_DC,
        .pin_rst = PIN_DISPLAY_RST,
        .pin_bl = PIN_DISPLAY_BL,
        .spi_clock_hz = DISPLAY_SPI_CLOCK,
        .orientation = DISPLAY_ORIENTATION,
    };

    if (config == NULL) {
        config = &default_config;
    }

    // Initialize SPI bus
    init_spi_bus();

    // Initialize ST7789 controller
    esp_err_t ret = st7789_init(&g_st7789, DISPLAY_SPI_HOST,
                                 config->pin_cs, config->pin_dc,
                                 config->pin_rst, config->pin_bl,
                                 config->spi_clock_hz);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ST7789: %s", esp_err_to_name(ret));
        return ret;
    }

    // Set orientation
    st7789_set_orientation(&g_st7789, config->orientation);

    g_initialized = true;

    ESP_LOGI(TAG, "Display initialized successfully (%dx%d)",
             g_st7789.width, g_st7789.height);

    return ESP_OK;
}

/**
 * Clear entire display
 */
void display_clear(uint16_t color)
{
    if (!g_initialized) return;

    st7789_fill_rect(&g_st7789, 0, 0, g_st7789.width, g_st7789.height, color);
}

/**
 * Draw single pixel
 */
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_initialized) return;
    if (x >= g_st7789.width || y >= g_st7789.height) return;

    st7789_set_window(&g_st7789, x, y, x, y);

    // Swap bytes for big-endian
    uint16_t color_be = (color >> 8) | (color << 8);
    st7789_write_pixels(&g_st7789, &color_be, 1);
}

/**
 * Draw filled rectangle
 */
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_initialized) return;

    // Clip to display bounds
    if (x >= g_st7789.width || y >= g_st7789.height) return;
    if (x + w > g_st7789.width) w = g_st7789.width - x;
    if (y + h > g_st7789.height) h = g_st7789.height - y;

    st7789_fill_rect(&g_st7789, x, y, w, h, color);
}

/**
 * Write raw RGB565 buffer to display
 * This is the main function for video frames
 */
void display_write_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *buffer)
{
    if (!g_initialized) return;
    if (buffer == NULL) return;

    // Clip to display bounds
    if (x >= g_st7789.width || y >= g_st7789.height) return;
    if (x + w > g_st7789.width) w = g_st7789.width - x;
    if (y + h > g_st7789.height) h = g_st7789.height - y;

    st7789_set_window(&g_st7789, x, y, x + w - 1, y + h - 1);

    // Note: Buffer should already be in RGB565 format
    // May need byte swapping depending on video decoder output
    st7789_write_pixels(&g_st7789, buffer, w * h);
}

/**
 * Write frame buffer using DMA
 */
esp_err_t display_write_frame_dma(const frame_buffer_t *fb)
{
    if (!g_initialized) return ESP_FAIL;
    if (fb == NULL || fb->buffer == NULL) return ESP_ERR_INVALID_ARG;

    // Center frame on display if smaller than display
    uint16_t x = (g_st7789.width - fb->width) / 2;
    uint16_t y = (g_st7789.height - fb->height) / 2;

    st7789_set_window(&g_st7789, x, y, x + fb->width - 1, y + fb->height - 1);

    return st7789_write_pixels_dma(&g_st7789, fb->buffer, fb->width * fb->height);
}

/**
 * Wait for DMA transfer to complete
 */
void display_wait_dma(void)
{
    if (!g_initialized) return;

    spi_transaction_t *trans;
    spi_device_get_trans_result(g_st7789.spi, &trans, portMAX_DELAY);
}

/**
 * Set backlight brightness
 */
void display_set_brightness(uint8_t brightness)
{
    if (!g_initialized) return;
    if (brightness > 100) brightness = 100;

    st7789_set_backlight(&g_st7789, brightness);
}

/**
 * Put display to sleep
 */
void display_sleep(void)
{
    if (!g_initialized) return;

    st7789_sleep(&g_st7789);
    st7789_set_backlight(&g_st7789, 0);  // Turn off backlight
}

/**
 * Wake display from sleep
 */
void display_wake(void)
{
    if (!g_initialized) return;

    st7789_wake(&g_st7789);
    st7789_set_backlight(&g_st7789, 100);  // Full brightness
}

/**
 * Get display width
 */
uint16_t display_get_width(void)
{
    return g_initialized ? g_st7789.width : 0;
}

/**
 * Get display height
 */
uint16_t display_get_height(void)
{
    return g_initialized ? g_st7789.height : 0;
}

/**
 * Allocate frame buffer
 */
frame_buffer_t *display_alloc_frame_buffer(uint16_t width, uint16_t height)
{
    frame_buffer_t *fb = malloc(sizeof(frame_buffer_t));
    if (fb == NULL) {
        ESP_LOGE(TAG, "Failed to allocate frame buffer structure");
        return NULL;
    }

    // Allocate buffer (RGB565 = 2 bytes per pixel)
    fb->buffer = heap_caps_malloc(width * height * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (fb->buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate frame buffer memory (%d bytes)",
                 width * height * 2);
        free(fb);
        return NULL;
    }

    fb->width = width;
    fb->height = height;
    fb->ready = false;

    ESP_LOGI(TAG, "Allocated frame buffer: %dx%d (%d bytes)",
             width, height, width * height * 2);

    return fb;
}

/**
 * Free frame buffer
 */
void display_free_frame_buffer(frame_buffer_t *fb)
{
    if (fb == NULL) return;

    if (fb->buffer != NULL) {
        heap_caps_free(fb->buffer);
    }

    free(fb);
}
