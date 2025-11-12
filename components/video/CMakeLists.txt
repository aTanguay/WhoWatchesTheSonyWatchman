idf_component_register(
    SRCS "video_player.c" "mjpeg_decoder.c" "avi_parser.c"
    INCLUDE_DIRS "include"
    REQUIRES display storage esp_timer
)
