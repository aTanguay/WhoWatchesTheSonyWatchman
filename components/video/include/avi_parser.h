/**
 * AVI File Parser
 * Parses AVI container format to extract MJPEG video and audio streams
 *
 * Implements tasks T1.15-T1.17: MJPEG decoder research and AVI parsing
 */

#ifndef AVI_PARSER_H
#define AVI_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"
#include "mjpeg_decoder.h"

// AVI FOURCC codes
#define FOURCC_RIFF     0x46464952  // "RIFF"
#define FOURCC_AVI      0x20495641  // "AVI "
#define FOURCC_LIST     0x5453494C  // "LIST"
#define FOURCC_HDRL     0x6C726468  // "hdrl"
#define FOURCC_STRL     0x6C727473  // "strl"
#define FOURCC_MOVI     0x69766F6D  // "movi"
#define FOURCC_AVIH     0x68697661  // "avih"
#define FOURCC_STRH     0x68727473  // "strh"
#define FOURCC_STRF     0x66727473  // "strf"
#define FOURCC_VIDS     0x73646976  // "vids"
#define FOURCC_AUDS     0x73647561  // "auds"
#define FOURCC_MJPG     0x47504A4D  // "MJPG"
#define FOURCC_00DC     0x63643030  // "00dc" - video chunk
#define FOURCC_01WB     0x62773130  // "01wb" - audio chunk

/**
 * AVI main header structure
 */
typedef struct {
    uint32_t micro_sec_per_frame;
    uint32_t max_bytes_per_sec;
    uint32_t total_frames;
    uint32_t streams;
    uint32_t suggested_buffer_size;
    uint32_t width;
    uint32_t height;
} avi_main_header_t;

/**
 * AVI stream header
 */
typedef struct {
    uint32_t fourcc_type;       // 'vids' or 'auds'
    uint32_t fourcc_handler;    // Codec
    uint32_t scale;
    uint32_t rate;              // fps = rate/scale
    uint32_t start;
    uint32_t length;            // frames
    uint32_t suggested_buffer_size;
    uint32_t quality;
    uint32_t sample_size;
} avi_stream_header_t;

/**
 * AVI video stream info
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t bit_count;
    uint32_t compression;
    bool found;
} avi_video_info_t;

/**
 * AVI audio stream info
 */
typedef struct {
    uint16_t format_tag;
    uint16_t channels;
    uint32_t samples_per_sec;
    uint32_t avg_bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
    bool found;
} avi_audio_info_t;

/**
 * AVI chunk (video frame or audio block)
 */
typedef struct {
    uint32_t fourcc;
    uint32_t size;
    uint32_t offset;            // File offset of chunk data
    bool is_keyframe;
} avi_chunk_t;

/**
 * AVI parser handle
 */
typedef struct {
    FILE *file;
    avi_main_header_t main_header;
    avi_video_info_t video_info;
    avi_audio_info_t audio_info;

    uint32_t movi_offset;       // File offset of 'movi' chunk
    uint32_t movi_size;

    uint32_t current_frame;
    uint32_t total_frames;

    bool initialized;
} avi_parser_t;

/**
 * Initialize AVI parser and open file
 *
 * @param parser Parser handle
 * @param file_path Path to AVI file
 * @return ESP_OK on success
 */
esp_err_t avi_parser_open(avi_parser_t *parser, const char *file_path);

/**
 * Close AVI file and cleanup
 *
 * @param parser Parser handle
 */
void avi_parser_close(avi_parser_t *parser);

/**
 * Read next video frame from AVI
 *
 * @param parser Parser handle
 * @param frame Output MJPEG frame structure (buffer allocated by function)
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no more frames
 */
esp_err_t avi_parser_read_video_frame(avi_parser_t *parser, mjpeg_frame_t *frame);

/**
 * Read next audio chunk from AVI
 *
 * @param parser Parser handle
 * @param buffer Output buffer for audio data
 * @param max_size Maximum buffer size
 * @param bytes_read Actual bytes read
 * @return ESP_OK on success
 */
esp_err_t avi_parser_read_audio_chunk(avi_parser_t *parser, uint8_t *buffer,
                                       uint32_t max_size, uint32_t *bytes_read);

/**
 * Seek to specific frame
 *
 * @param parser Parser handle
 * @param frame_num Frame number to seek to
 * @return ESP_OK on success
 */
esp_err_t avi_parser_seek(avi_parser_t *parser, uint32_t frame_num);

/**
 * Get current frame number
 *
 * @param parser Parser handle
 * @return Current frame number
 */
uint32_t avi_parser_get_current_frame(const avi_parser_t *parser);

/**
 * Get total frame count
 *
 * @param parser Parser handle
 * @return Total number of frames
 */
uint32_t avi_parser_get_total_frames(const avi_parser_t *parser);

/**
 * Get video frame rate
 *
 * @param parser Parser handle
 * @return Frame rate in FPS
 */
float avi_parser_get_fps(const avi_parser_t *parser);

/**
 * Free MJPEG frame data allocated by parser
 *
 * @param frame Frame to free
 */
void avi_parser_free_frame(mjpeg_frame_t *frame);

#endif // AVI_PARSER_H
