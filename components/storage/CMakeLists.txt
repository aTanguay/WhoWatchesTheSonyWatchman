idf_component_register(
    SRCS "sd_card.c" "channel_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES driver fatfs sdmmc
)
