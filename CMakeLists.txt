# ESP-IDF Project: Sony Watchman Retro Media Player
cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set supported ESP32 targets
set(SUPPORTED_TARGETS esp32 esp32s3)

# Project name
project(sony_watchman)
