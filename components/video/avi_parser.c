/**
 * AVI File Parser Implementation
 *
 * Tasks: T1.15-T1.17
 */

#include "avi_parser.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "AVI_PARSER";

// Helper function to read 32-bit little-endian integer
static uint32_t read_le32(FILE *f)
{
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return 0;
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// Helper function to read 16-bit little-endian integer
static uint16_t read_le16(FILE *f)
{
    uint8_t buf[2];
    if (fread(buf, 1, 2, f) != 2) return 0;
    return buf[0] | (buf[1] << 8);
}

// Helper function to read FOURCC code
static uint32_t read_fourcc(FILE *f)
{
    return read_le32(f);
}

/**
 * Parse AVI main header (avih chunk)
 */
static esp_err_t parse_avih(avi_parser_t *parser, uint32_t size)
{
    parser->main_header.micro_sec_per_frame = read_le32(parser->file);
    parser->main_header.max_bytes_per_sec = read_le32(parser->file);
    fseek(parser->file, 4, SEEK_CUR);  // padding
    fseek(parser->file, 4, SEEK_CUR);  // flags
    parser->main_header.total_frames = read_le32(parser->file);
    fseek(parser->file, 4, SEEK_CUR);  // initial frames
    parser->main_header.streams = read_le32(parser->file);
    parser->main_header.suggested_buffer_size = read_le32(parser->file);
    parser->main_header.width = read_le32(parser->file);
    parser->main_header.height = read_le32(parser->file);

    ESP_LOGI(TAG, "AVI Header: %lux%lu, %lu frames, %lu streams",
             parser->main_header.width, parser->main_header.height,
             parser->main_header.total_frames, parser->main_header.streams);

    parser->total_frames = parser->main_header.total_frames;

    // Skip remaining header data
    long remaining = size - 48;
    if (remaining > 0) {
        fseek(parser->file, remaining, SEEK_CUR);
    }

    return ESP_OK;
}

/**
 * Parse stream header (strh chunk)
 */
static esp_err_t parse_strh(avi_parser_t *parser, uint32_t size)
{
    avi_stream_header_t strh;

    strh.fourcc_type = read_fourcc(parser->file);
    strh.fourcc_handler = read_fourcc(parser->file);
    fseek(parser->file, 4, SEEK_CUR);  // flags
    fseek(parser->file, 2, SEEK_CUR);  // priority
    fseek(parser->file, 2, SEEK_CUR);  // language
    fseek(parser->file, 4, SEEK_CUR);  // initial frames
    strh.scale = read_le32(parser->file);
    strh.rate = read_le32(parser->file);
    strh.start = read_le32(parser->file);
    strh.length = read_le32(parser->file);
    strh.suggested_buffer_size = read_le32(parser->file);
    strh.quality = read_le32(parser->file);
    strh.sample_size = read_le32(parser->file);

    if (strh.fourcc_type == FOURCC_VIDS) {
        ESP_LOGI(TAG, "Video stream: %lu frames, rate=%lu/%lu fps",
                 strh.length, strh.rate, strh.scale);
    } else if (strh.fourcc_type == FOURCC_AUDS) {
        ESP_LOGI(TAG, "Audio stream: rate=%lu/%lu",
                 strh.rate, strh.scale);
        parser->audio_info.found = true;
    }

    // Skip remaining header
    long remaining = size - 48;
    if (remaining > 0) {
        fseek(parser->file, remaining, SEEK_CUR);
    }

    return ESP_OK;
}

/**
 * Parse stream format (strf chunk) for video
 */
static esp_err_t parse_strf_video(avi_parser_t *parser, uint32_t size)
{
    fseek(parser->file, 4, SEEK_CUR);  // size
    parser->video_info.width = read_le32(parser->file);
    parser->video_info.height = read_le32(parser->file);
    fseek(parser->file, 2, SEEK_CUR);  // planes
    parser->video_info.bit_count = read_le16(parser->file);
    parser->video_info.compression = read_fourcc(parser->file);
    parser->video_info.found = true;

    ESP_LOGI(TAG, "Video format: %dx%d, %d-bit, compression=0x%08lX",
             parser->video_info.width, parser->video_info.height,
             parser->video_info.bit_count, parser->video_info.compression);

    // Skip remaining format data
    long remaining = size - 20;
    if (remaining > 0) {
        fseek(parser->file, remaining, SEEK_CUR);
    }

    return ESP_OK;
}

/**
 * Parse stream format (strf chunk) for audio
 */
static esp_err_t parse_strf_audio(avi_parser_t *parser, uint32_t size)
{
    parser->audio_info.format_tag = read_le16(parser->file);
    parser->audio_info.channels = read_le16(parser->file);
    parser->audio_info.samples_per_sec = read_le32(parser->file);
    parser->audio_info.avg_bytes_per_sec = read_le32(parser->file);
    parser->audio_info.block_align = read_le16(parser->file);
    parser->audio_info.bits_per_sample = read_le16(parser->file);

    ESP_LOGI(TAG, "Audio format: %d Hz, %d ch, %d-bit, format=0x%04X",
             parser->audio_info.samples_per_sec, parser->audio_info.channels,
             parser->audio_info.bits_per_sample, parser->audio_info.format_tag);

    // Skip remaining format data
    long remaining = size - 16;
    if (remaining > 0) {
        fseek(parser->file, remaining, SEEK_CUR);
    }

    return ESP_OK;
}

/**
 * Parse LIST chunk recursively
 */
static esp_err_t parse_list(avi_parser_t *parser, uint32_t list_size)
{
    uint32_t list_type = read_fourcc(parser->file);
    uint32_t bytes_read = 4;

    if (list_type == FOURCC_MOVI) {
        // Found movie data - save offset and return
        parser->movi_offset = ftell(parser->file);
        parser->movi_size = list_size - 4;
        ESP_LOGI(TAG, "Found 'movi' chunk at offset %lu, size %lu",
                 parser->movi_offset, parser->movi_size);
        return ESP_OK;
    }

    // Parse chunks within LIST
    while (bytes_read < list_size - 4) {
        uint32_t fourcc = read_fourcc(parser->file);
        uint32_t size = read_le32(parser->file);
        bytes_read += 8;

        if (fourcc == FOURCC_AVIH) {
            parse_avih(parser, size);
        } else if (fourcc == FOURCC_STRH) {
            parse_strh(parser, size);
        } else if (fourcc == FOURCC_STRF) {
            // Determine if video or audio based on previous strh
            if (parser->video_info.found) {
                parse_strf_video(parser, size);
            } else {
                parse_strf_audio(parser, size);
            }
        } else if (fourcc == FOURCC_LIST) {
            parse_list(parser, size);
        } else {
            // Skip unknown chunk
            fseek(parser->file, size, SEEK_CUR);
        }

        bytes_read += size;

        // Handle padding byte
        if (size & 1) {
            fseek(parser->file, 1, SEEK_CUR);
            bytes_read++;
        }
    }

    return ESP_OK;
}

/**
 * Parse AVI header structure
 */
static esp_err_t parse_avi_header(avi_parser_t *parser)
{
    // Read RIFF header
    uint32_t fourcc = read_fourcc(parser->file);
    if (fourcc != FOURCC_RIFF) {
        ESP_LOGE(TAG, "Not a RIFF file");
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t file_size = read_le32(parser->file);
    fourcc = read_fourcc(parser->file);
    if (fourcc != FOURCC_AVI) {
        ESP_LOGE(TAG, "Not an AVI file");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Parsing AVI file, size: %lu bytes", file_size);

    // Parse chunks
    while (!feof(parser->file)) {
        fourcc = read_fourcc(parser->file);
        if (feof(parser->file)) break;

        uint32_t size = read_le32(parser->file);

        if (fourcc == FOURCC_LIST) {
            parse_list(parser, size);
        } else {
            // Skip unknown chunk
            fseek(parser->file, size, SEEK_CUR);
        }

        // Handle padding
        if (size & 1) {
            fseek(parser->file, 1, SEEK_CUR);
        }

        // Stop after finding movi chunk
        if (parser->movi_offset != 0) {
            break;
        }
    }

    if (parser->movi_offset == 0) {
        ESP_LOGE(TAG, "No 'movi' chunk found");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * Open AVI file
 */
esp_err_t avi_parser_open(avi_parser_t *parser, const char *file_path)
{
    if (parser == NULL || file_path == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Opening AVI file: %s", file_path);

    memset(parser, 0, sizeof(avi_parser_t));

    parser->file = fopen(file_path, "rb");
    if (parser->file == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", file_path);
        return ESP_FAIL;
    }

    // Parse AVI structure
    esp_err_t ret = parse_avi_header(parser);
    if (ret != ESP_OK) {
        fclose(parser->file);
        parser->file = NULL;
        return ret;
    }

    // Seek to start of movie data
    fseek(parser->file, parser->movi_offset, SEEK_SET);
    parser->current_frame = 0;
    parser->initialized = true;

    ESP_LOGI(TAG, "AVI file opened successfully");
    ESP_LOGI(TAG, "Video: %dx%d, %lu frames",
             parser->video_info.width, parser->video_info.height,
             parser->total_frames);
    if (parser->audio_info.found) {
        ESP_LOGI(TAG, "Audio: %lu Hz, %d channels",
                 parser->audio_info.samples_per_sec,
                 parser->audio_info.channels);
    }

    return ESP_OK;
}

/**
 * Close AVI file
 */
void avi_parser_close(avi_parser_t *parser)
{
    if (parser == NULL) return;

    if (parser->file) {
        fclose(parser->file);
        parser->file = NULL;
    }

    parser->initialized = false;

    ESP_LOGI(TAG, "AVI file closed");
}

/**
 * Read next video frame
 */
esp_err_t avi_parser_read_video_frame(avi_parser_t *parser, mjpeg_frame_t *frame)
{
    if (!parser || !parser->initialized || !frame) {
        return ESP_ERR_INVALID_ARG;
    }

    // Search for next video chunk (00dc or 00db)
    while (!feof(parser->file)) {
        uint32_t fourcc = read_fourcc(parser->file);
        if (feof(parser->file)) {
            return ESP_ERR_NOT_FOUND;
        }

        uint32_t size = read_le32(parser->file);

        // Check if this is a video chunk (00dc = compressed, 00db = uncompressed)
        if ((fourcc & 0xFFFF) == 0x6364 || (fourcc & 0xFFFF) == 0x6264) {  // "dc" or "db"
            // Allocate buffer for frame data
            frame->data = malloc(size);
            if (frame->data == NULL) {
                ESP_LOGE(TAG, "Failed to allocate frame buffer (%lu bytes)", size);
                return ESP_ERR_NO_MEM;
            }

            // Read frame data
            size_t bytes_read = fread(frame->data, 1, size, parser->file);
            if (bytes_read != size) {
                ESP_LOGE(TAG, "Failed to read frame data");
                free(frame->data);
                return ESP_FAIL;
            }

            frame->size = size;
            frame->frame_num = parser->current_frame;
            frame->timestamp_ms = (parser->current_frame * parser->main_header.micro_sec_per_frame) / 1000;

            parser->current_frame++;

            // Handle padding
            if (size & 1) {
                fseek(parser->file, 1, SEEK_CUR);
            }

            return ESP_OK;
        } else {
            // Skip non-video chunk
            fseek(parser->file, size, SEEK_CUR);
            if (size & 1) {
                fseek(parser->file, 1, SEEK_CUR);
            }
        }
    }

    return ESP_ERR_NOT_FOUND;
}

/**
 * Read audio chunk
 */
esp_err_t avi_parser_read_audio_chunk(avi_parser_t *parser, uint8_t *buffer,
                                       uint32_t max_size, uint32_t *bytes_read)
{
    if (!parser || !parser->initialized || !buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    // Search for next audio chunk (01wb)
    while (!feof(parser->file)) {
        uint32_t fourcc = read_fourcc(parser->file);
        if (feof(parser->file)) {
            return ESP_ERR_NOT_FOUND;
        }

        uint32_t size = read_le32(parser->file);

        // Check if this is an audio chunk (01wb)
        if ((fourcc & 0xFFFF) == 0x6277) {  // "wb"
            uint32_t read_size = (size < max_size) ? size : max_size;

            size_t read = fread(buffer, 1, read_size, parser->file);
            if (bytes_read) {
                *bytes_read = read;
            }

            // Skip remainder if buffer too small
            if (size > max_size) {
                fseek(parser->file, size - max_size, SEEK_CUR);
            }

            // Handle padding
            if (size & 1) {
                fseek(parser->file, 1, SEEK_CUR);
            }

            return ESP_OK;
        } else {
            // Skip non-audio chunk
            fseek(parser->file, size, SEEK_CUR);
            if (size & 1) {
                fseek(parser->file, 1, SEEK_CUR);
            }
        }
    }

    return ESP_ERR_NOT_FOUND;
}

/**
 * Seek to frame
 */
esp_err_t avi_parser_seek(avi_parser_t *parser, uint32_t frame_num)
{
    if (!parser || !parser->initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    // Simple implementation: seek to start and skip frames
    // TODO: Build index for faster seeking
    fseek(parser->file, parser->movi_offset, SEEK_SET);
    parser->current_frame = 0;

    // Skip to target frame
    mjpeg_frame_t temp_frame;
    while (parser->current_frame < frame_num) {
        if (avi_parser_read_video_frame(parser, &temp_frame) != ESP_OK) {
            return ESP_FAIL;
        }
        free(temp_frame.data);
    }

    return ESP_OK;
}

/**
 * Get current frame number
 */
uint32_t avi_parser_get_current_frame(const avi_parser_t *parser)
{
    return parser ? parser->current_frame : 0;
}

/**
 * Get total frames
 */
uint32_t avi_parser_get_total_frames(const avi_parser_t *parser)
{
    return parser ? parser->total_frames : 0;
}

/**
 * Get frame rate
 */
float avi_parser_get_fps(const avi_parser_t *parser)
{
    if (!parser || parser->main_header.micro_sec_per_frame == 0) {
        return 0.0f;
    }

    return 1000000.0f / parser->main_header.micro_sec_per_frame;
}

/**
 * Free frame data
 */
void avi_parser_free_frame(mjpeg_frame_t *frame)
{
    if (frame && frame->data) {
        free(frame->data);
        frame->data = NULL;
        frame->size = 0;
    }
}
