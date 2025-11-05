/**
 * ST7789 Display Controller Driver Implementation
 */

#include <string.h>
#include "st7789.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "ST7789";

// Hardware reset delay
#define RESET_DELAY_MS      10
#define INIT_DELAY_MS       120

/**
 * Send command to ST7789
 */
void st7789_write_command(st7789_handle_t *handle, uint8_t cmd)
{
    gpio_set_level(handle->pin_dc, 0);  // Command mode

    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = SPI_TRANS_USE_TXDATA,
    };
    trans.tx_data[0] = cmd;

    spi_device_polling_transmit(handle->spi, &trans);
}

/**
 * Send data to ST7789
 */
void st7789_write_data(st7789_handle_t *handle, const uint8_t *data, size_t len)
{
    if (len == 0) return;

    gpio_set_level(handle->pin_dc, 1);  // Data mode

    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };

    if (len <= 4) {
        trans.flags = SPI_TRANS_USE_TXDATA;
        memcpy(trans.tx_data, data, len);
    }

    spi_device_polling_transmit(handle->spi, &trans);
}

/**
 * Hardware reset
 */
static void st7789_reset(st7789_handle_t *handle)
{
    if (handle->pin_rst < 0) return;

    gpio_set_level(handle->pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));
    gpio_set_level(handle->pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(INIT_DELAY_MS));
}

/**
 * Initialize ST7789 controller
 */
esp_err_t st7789_init(st7789_handle_t *handle, spi_host_device_t spi_host,
                       int pin_cs, int pin_dc, int pin_rst, int pin_bl,
                       int spi_clock)
{
    ESP_LOGI(TAG, "Initializing ST7789 display driver");

    handle->pin_dc = pin_dc;
    handle->pin_rst = pin_rst;
    handle->pin_bl = pin_bl;
    handle->width = 240;
    handle->height = 320;
    handle->orientation = 0;

    // Configure GPIO pins
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // DC pin
    io_conf.pin_bit_mask = (1ULL << pin_dc);
    gpio_config(&io_conf);

    // Reset pin
    if (pin_rst >= 0) {
        io_conf.pin_bit_mask = (1ULL << pin_rst);
        gpio_config(&io_conf);
        gpio_set_level(pin_rst, 1);
    }

    // Backlight pin - use LEDC for PWM control
    if (pin_bl >= 0) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = LEDC_TIMER_0,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .timer_sel = LEDC_TIMER_0,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = pin_bl,
            .duty = 255,  // Full brightness initially
            .hpoint = 0
        };
        ledc_channel_config(&ledc_channel);
    }

    // Configure SPI bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = spi_clock,
        .mode = 0,                          // SPI mode 0
        .spics_io_num = pin_cs,
        .queue_size = 7,
        .pre_cb = NULL,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    esp_err_t ret = spi_bus_add_device(spi_host, &devcfg, &handle->spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device");
        return ret;
    }

    // Hardware reset
    st7789_reset(handle);

    // Initialize display controller
    st7789_write_command(handle, ST7789_SWRESET);  // Software reset
    vTaskDelay(pdMS_TO_TICKS(150));

    st7789_write_command(handle, ST7789_SLPOUT);   // Sleep out
    vTaskDelay(pdMS_TO_TICKS(10));

    // Color mode - 16-bit RGB565
    st7789_write_command(handle, ST7789_COLMOD);
    uint8_t colmod = 0x55;  // 16-bit/pixel
    st7789_write_data(handle, &colmod, 1);

    // Memory data access control
    st7789_set_orientation(handle, 0);

    // Inversion on
    st7789_write_command(handle, ST7789_INVON);

    // Normal display mode
    st7789_write_command(handle, ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Display on
    st7789_write_command(handle, ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "ST7789 initialization complete (%dx%d)", handle->width, handle->height);

    return ESP_OK;
}

/**
 * Set display orientation
 */
void st7789_set_orientation(st7789_handle_t *handle, uint8_t orientation)
{
    handle->orientation = orientation;

    uint8_t madctl = ST7789_MADCTL_RGB;  // RGB color order

    switch (orientation) {
        case 0:  // Portrait
            handle->width = 240;
            handle->height = 320;
            madctl |= ST7789_MADCTL_MX;
            break;
        case 1:  // Landscape
            handle->width = 320;
            handle->height = 240;
            madctl |= ST7789_MADCTL_MV;
            break;
        case 2:  // Portrait inverted
            handle->width = 240;
            handle->height = 320;
            madctl |= ST7789_MADCTL_MY;
            break;
        case 3:  // Landscape inverted
            handle->width = 320;
            handle->height = 240;
            madctl |= ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_MV;
            break;
    }

    st7789_write_command(handle, ST7789_MADCTL);
    st7789_write_data(handle, &madctl, 1);
}

/**
 * Set drawing window
 */
void st7789_set_window(st7789_handle_t *handle, uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1)
{
    // Column address set
    st7789_write_command(handle, ST7789_CASET);
    uint8_t data[4];
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    st7789_write_data(handle, data, 4);

    // Row address set
    st7789_write_command(handle, ST7789_RASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    st7789_write_data(handle, data, 4);

    // Memory write
    st7789_write_command(handle, ST7789_RAMWR);
}

/**
 * Write pixel data
 */
void st7789_write_pixels(st7789_handle_t *handle, const uint16_t *data, uint32_t len)
{
    if (len == 0) return;

    gpio_set_level(handle->pin_dc, 1);  // Data mode

    // ST7789 expects big-endian RGB565, ESP32 is little-endian
    // We need to swap bytes for each pixel
    spi_transaction_t trans = {
        .length = len * 16,  // bits
        .tx_buffer = data,
        .flags = SPI_TRANS_MODE_DIO,
    };

    spi_device_polling_transmit(handle->spi, &trans);
}

/**
 * Write pixel data using DMA
 */
esp_err_t st7789_write_pixels_dma(st7789_handle_t *handle, const uint16_t *data, uint32_t len)
{
    if (len == 0) return ESP_OK;

    gpio_set_level(handle->pin_dc, 1);  // Data mode

    spi_transaction_t trans = {
        .length = len * 16,  // bits
        .tx_buffer = data,
    };

    return spi_device_queue_trans(handle->spi, &trans, portMAX_DELAY);
}

/**
 * Fill rectangle with solid color
 */
void st7789_fill_rect(st7789_handle_t *handle, uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h, uint16_t color)
{
    st7789_set_window(handle, x, y, x + w - 1, y + h - 1);

    // Fill with color
    uint32_t pixel_count = w * h;

    // Swap bytes for big-endian
    uint16_t color_be = (color >> 8) | (color << 8);

    gpio_set_level(handle->pin_dc, 1);  // Data mode

    // Send color in chunks
    const uint32_t chunk_size = 1024;
    uint16_t buffer[chunk_size];

    for (uint32_t i = 0; i < chunk_size && i < pixel_count; i++) {
        buffer[i] = color_be;
    }

    while (pixel_count > 0) {
        uint32_t send_count = (pixel_count > chunk_size) ? chunk_size : pixel_count;
        st7789_write_pixels(handle, buffer, send_count);
        pixel_count -= send_count;
    }
}

/**
 * Set backlight brightness
 */
void st7789_set_backlight(st7789_handle_t *handle, uint8_t brightness)
{
    if (handle->pin_bl < 0) return;

    // Convert 0-100 to 0-255
    uint32_t duty = (brightness * 255) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

/**
 * Enter sleep mode
 */
void st7789_sleep(st7789_handle_t *handle)
{
    st7789_write_command(handle, ST7789_SLPIN);
    vTaskDelay(pdMS_TO_TICKS(5));
}

/**
 * Exit sleep mode
 */
void st7789_wake(st7789_handle_t *handle)
{
    st7789_write_command(handle, ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(5));
}
