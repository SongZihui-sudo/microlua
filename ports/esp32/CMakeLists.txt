# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(microlua_esp32)

INCLUDE_DIRECTORIES(
    ../../lib/lw_oopc/
    ../../src/lua
    ../../src/interface
    .
    ../rp2/board/pico
    ./board/airesp32c3 
    ../stm32/board/air32f103)

LIST(APPEND source_esp32 ../../src/lua/onelua.c)
AUX_SOURCE_DIRECTORY(. source_esp32)

idf_component_register(SRCS ${source_esp32})
