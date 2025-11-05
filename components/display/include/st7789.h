/**
 * ST7789 Display Controller Driver
 * Low-level driver for ST7789VW chip
 */

#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// ST7789 Commands
#define ST7789_NOP          0x00
#define ST7789_SWRESET      0x01
#define ST7789_RDDID        0x04
#define ST7789_RDDST        0x09
#define ST7789_SLPIN        0x10
#define ST7789_SLPOUT       0x11
#define ST7789_PTLON        0x12
#define ST7789_NORON        0x13
#define ST7789_INVOFF       0x20
#define ST7789_INVON        0x21
#define ST7789_DISPOFF      0x28
#define ST7789_DISPON       0x29
#define ST7789_CASET        0x2A
#define ST7789_RASET        0x2B
#define ST7789_RAMWR        0x2C
#define ST7789_RAMRD        0x2E
#define ST7789_PTLAR        0x30
#define ST7789_COLMOD       0x3A
#define ST7789_MADCTL       0x36
#define ST7789_FRMCTR1      0xB1
#define ST7789_FRMCTR2      0xB2
#define ST7789_FRMCTR3      0xB3
#define ST7789_INVCTR       0xB4
#define ST7789_DISSET5      0xB6
#define ST7789_PWCTR1       0xC0
#define ST7789_PWCTR2       0xC1
#define ST7789_PWCTR3       0xC2
#define ST7789_PWCTR4       0xC3
#define ST7789_PWCTR5       0xC4
#define ST7789_VMCTR1       0xC5
#define ST7789_RDID1        0xDA
#define ST7789_RDID2        0xDB
#define ST7789_RDID3        0xDC
#define ST7789_RDID4        0xDD
#define ST7789_PWCTR6       0xFC
#define ST7789_GMCTRP1      0xE0
#define ST7789_GMCTRN1      0xE1

// MADCTL bits
#define ST7789_MADCTL_MY    0x80
#define ST7789_MADCTL_MX    0x40
#define ST7789_MADCTL_MV    0x20
#define ST7789_MADCTL_ML    0x10
#define ST7789_MADCTL_RGB   0x00
#define ST7789_MADCTL_BGR   0x08
#define ST7789_MADCTL_MH    0x04

/**
 * ST7789 device handle
 */
typedef struct {
    spi_device_handle_t spi;
    int pin_dc;
    int pin_rst;
    int pin_bl;
    uint16_t width;
    uint16_t height;
    uint8_t orientation;
} st7789_handle_t;

/**
 * Initialize ST7789 controller
 *
 * @param handle ST7789 handle
 * @param spi_host SPI host device
 * @param pin_cs Chip select pin
 * @param pin_dc Data/command pin
 * @param pin_rst Reset pin
 * @param pin_bl Backlight pin (-1 if not used)
 * @param spi_clock SPI clock speed in Hz
 * @return ESP_OK on success
 */
esp_err_t st7789_init(st7789_handle_t *handle, spi_host_device_t spi_host,
                       int pin_cs, int pin_dc, int pin_rst, int pin_bl,
                       int spi_clock);

/**
 * Set display orientation
 *
 * @param handle ST7789 handle
 * @param orientation 0=portrait, 1=landscape, 2=portrait inverted, 3=landscape inverted
 */
void st7789_set_orientation(st7789_handle_t *handle, uint8_t orientation);

/**
 * Set drawing window (address window)
 *
 * @param handle ST7789 handle
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void st7789_set_window(st7789_handle_t *handle, uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1);

/**
 * Write pixel data to display
 *
 * @param handle ST7789 handle
 * @param data RGB565 pixel data
 * @param len Number of pixels
 */
void st7789_write_pixels(st7789_handle_t *handle, const uint16_t *data, uint32_t len);

/**
 * Write pixel data using DMA (non-blocking)
 *
 * @param handle ST7789 handle
 * @param data RGB565 pixel data
 * @param len Number of pixels
 * @return ESP_OK on success
 */
esp_err_t st7789_write_pixels_dma(st7789_handle_t *handle, const uint16_t *data, uint32_t len);

/**
 * Fill rectangle with solid color
 *
 * @param handle ST7789 handle
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void st7789_fill_rect(st7789_handle_t *handle, uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h, uint16_t color);

/**
 * Send command to ST7789
 *
 * @param handle ST7789 handle
 * @param cmd Command byte
 */
void st7789_write_command(st7789_handle_t *handle, uint8_t cmd);

/**
 * Send data to ST7789
 *
 * @param handle ST7789 handle
 * @param data Data buffer
 * @param len Data length
 */
void st7789_write_data(st7789_handle_t *handle, const uint8_t *data, size_t len);

/**
 * Set backlight brightness
 *
 * @param handle ST7789 handle
 * @param brightness 0-100 percentage
 */
void st7789_set_backlight(st7789_handle_t *handle, uint8_t brightness);

/**
 * Enter sleep mode
 *
 * @param handle ST7789 handle
 */
void st7789_sleep(st7789_handle_t *handle);

/**
 * Exit sleep mode
 *
 * @param handle ST7789 handle
 */
void st7789_wake(st7789_handle_t *handle);

#endif // ST7789_H
