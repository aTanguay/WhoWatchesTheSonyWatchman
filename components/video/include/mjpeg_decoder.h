/**
 * MJPEG Decoder
 * Decodes Motion JPEG frames from AVI files
 */

#ifndef MJPEG_DECODER_H
#define MJPEG_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// MJPEG chunk info
typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t frame_num;
    uint32_t timestamp_ms;
} mjpeg_frame_t;

/**
 * MJPEG decoder handle
 */
typedef struct mjpeg_decoder_s mjpeg_decoder_t;

/**
 * Create MJPEG decoder instance
 *
 * @param max_width Maximum frame width
 * @param max_height Maximum frame height
 * @return Decoder handle, or NULL on failure
 */
mjpeg_decoder_t *mjpeg_decoder_create(uint16_t max_width, uint16_t max_height);

/**
 * Destroy MJPEG decoder instance
 *
 * @param decoder Decoder handle
 */
void mjpeg_decoder_destroy(mjpeg_decoder_t *decoder);

/**
 * Decode MJPEG frame to RGB565 buffer
 *
 * @param decoder Decoder handle
 * @param frame MJPEG frame data
 * @param output RGB565 output buffer (must be allocated by caller)
 * @param output_width Output frame width
 * @param output_height Output frame height
 * @return ESP_OK on success
 */
esp_err_t mjpeg_decoder_decode_frame(mjpeg_decoder_t *decoder,
                                      const mjpeg_frame_t *frame,
                                      uint16_t *output,
                                      uint16_t *output_width,
                                      uint16_t *output_height);

/**
 * Get last decode time in milliseconds
 *
 * @param decoder Decoder handle
 * @return Decode time in ms
 */
uint32_t mjpeg_decoder_get_decode_time(const mjpeg_decoder_t *decoder);

#endif // MJPEG_DECODER_H
