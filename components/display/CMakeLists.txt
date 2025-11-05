idf_component_register(
    SRCS "display.c" "st7789.c"
    INCLUDE_DIRS "include"
    REQUIRES driver spi_flash
)
