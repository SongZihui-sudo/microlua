cmake_minimum_required(VERSION 3.20)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../../lib/pico-sdk/pico_sdk_init.cmake)

PROJECT(MicroLua)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# minimize mode
set(minimize 1)

# rest of your project
INCLUDE_DIRECTORIES(
    .
    ../../lib/pico-sdk/lib/tinyusb
    ../../lib/littlefs
    ../../lib/stdinit-lib
    ../../lib/lw_oopc
    ../../src/
    ../../src/interface)

LIST(APPEND source_rp2 ../../src/onelua.c)
AUX_SOURCE_DIRECTORY(. source_rp2)
AUX_SOURCE_DIRECTORY(../../src/interface source_rp2)
AUX_SOURCE_DIRECTORY(../../lib/stdinit-lib/ source_rp2)
AUX_SOURCE_DIRECTORY(../../lib/littlefs/ source_rp2)

add_compile_definitions(LUA_USE_RP2040=1)
add_compile_definitions(LUA_USE_LITTLEFS=1)
add_compile_definitions(MINIMIZE_NO_COMPILER=1)

if(${minimize} EQUAL 1)
    add_compile_definitions(MINIMIZE_NO_STANDATD_LIB=1)
    add_compile_options(-Os -ffunction-sections -fdata-sections)
    add_link_options(-W --data-sections)
endif()


ADD_EXECUTABLE(microlua_rp2 ${source_rp2})
target_link_libraries(microlua_rp2 pico_stdlib)

pico_enable_stdio_usb(microlua_rp2 1)
pico_enable_stdio_uart(microlua_rp2 0)

pico_add_extra_outputs(microlua_rp2)