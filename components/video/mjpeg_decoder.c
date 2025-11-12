/**
 * MJPEG Decoder Implementation
 * Uses ESP-IDF JPEG decoder component
 *
 * NOTE: esp_jpeg component may not be available in all ESP-IDF distributions.
 * This file provides stub functions when JPEG decoder is not available.
 */

#include "mjpeg_decoder.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"

// Check if ESP JPEG decoder component is available
#ifdef __has_include
#  if __has_include("esp_jpeg_dec.h")
#    define HAS_ESP_JPEG 1
#    include "esp_jpeg_dec.h"
#  else
#    define HAS_ESP_JPEG 0
#  endif
#else
#  define HAS_ESP_JPEG 0
#endif

static const char *TAG = "MJPEG_DEC";

/**
 * MJPEG decoder structure
 */
struct mjpeg_decoder_s {
#if HAS_ESP_JPEG
    jpeg_decoder_handle_t jpeg_handle;
#else
    void *jpeg_handle;  // Stub
#endif
    uint16_t max_width;
    uint16_t max_height;
    uint32_t last_decode_ms;
};

/**
 * Create MJPEG decoder
 */
mjpeg_decoder_t *mjpeg_decoder_create(uint16_t max_width, uint16_t max_height)
{
    mjpeg_decoder_t *decoder = malloc(sizeof(mjpeg_decoder_t));
    if (decoder == NULL) {
        ESP_LOGE(TAG, "Failed to allocate decoder");
        return NULL;
    }

    decoder->max_width = max_width;
    decoder->max_height = max_height;
    decoder->last_decode_ms = 0;
    decoder->jpeg_handle = NULL;

#if HAS_ESP_JPEG
    ESP_LOGI(TAG, "Creating MJPEG decoder (max %dx%d)", max_width, max_height);

    // Create JPEG decoder
    jpeg_dec_config_t config = {
        .output_format = JPEG_DECODE_OUT_FORMAT_RGB565,
        .flags = {
            .use_rgb565_bigendian = 0,  // Little endian for ESP32
        },
    };

    esp_err_t ret = jpeg_new_decoder(&config, &decoder->jpeg_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create JPEG decoder: %s", esp_err_to_name(ret));
        free(decoder);
        return NULL;
    }

    ESP_LOGI(TAG, "MJPEG decoder created successfully");
#else
    ESP_LOGW(TAG, "MJPEG decoder created (STUB - esp_jpeg not available)");
    ESP_LOGW(TAG, "Video playback will not work without JPEG decoder");
#endif

    return decoder;
}

/**
 * Destroy MJPEG decoder
 */
void mjpeg_decoder_destroy(mjpeg_decoder_t *decoder)
{
    if (decoder == NULL) return;

#if HAS_ESP_JPEG
    if (decoder->jpeg_handle) {
        jpeg_del_decoder(decoder->jpeg_handle);
    }
#endif

    free(decoder);

    ESP_LOGI(TAG, "MJPEG decoder destroyed");
}

/**
 * Decode MJPEG frame to RGB565
 */
esp_err_t mjpeg_decoder_decode_frame(mjpeg_decoder_t *decoder,
                                      const mjpeg_frame_t *frame,
                                      uint16_t *output,
                                      uint16_t *output_width,
                                      uint16_t *output_height)
{
    if (decoder == NULL || frame == NULL || output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

#if HAS_ESP_JPEG
    uint64_t start_time = esp_timer_get_time();

    // Parse JPEG header to get dimensions
    jpeg_dec_header_info_t *header_info = NULL;
    esp_err_t ret = jpeg_decoder_get_info(frame->data, frame->size, &header_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse JPEG header: %s", esp_err_to_name(ret));
        return ret;
    }

    uint16_t width = header_info->width;
    uint16_t height = header_info->height;

    if (width > decoder->max_width || height > decoder->max_height) {
        ESP_LOGE(TAG, "Frame dimensions (%dx%d) exceed max (%dx%d)",
                 width, height, decoder->max_width, decoder->max_height);
        return ESP_ERR_INVALID_SIZE;
    }

    // Decode JPEG to RGB565
    jpeg_dec_io_t decode_io = {
        .inbuf = frame->data,
        .inbuf_len = frame->size,
        .outbuf = (uint8_t *)output,
    };

    ret = jpeg_decoder_process(decoder->jpeg_handle, &decode_io);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JPEG decode failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Return dimensions
    if (output_width) *output_width = width;
    if (output_height) *output_height = height;

    // Calculate decode time
    uint64_t end_time = esp_timer_get_time();
    decoder->last_decode_ms = (end_time - start_time) / 1000;

    return ESP_OK;
#else
    ESP_LOGE(TAG, "JPEG decoder not available - cannot decode frame");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

/**
 * Get last decode time
 */
uint32_t mjpeg_decoder_get_decode_time(const mjpeg_decoder_t *decoder)
{
    return (decoder != NULL) ? decoder->last_decode_ms : 0;
}
