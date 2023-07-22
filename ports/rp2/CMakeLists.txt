cmake_minimum_required(VERSION 3.20)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../../lib/pico-sdk/pico_sdk_init.cmake)

PROJECT(MicroLua)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
INCLUDE_DIRECTORIES(
    .
    ../../lib/pico-sdk/lib/tinyusb
    ../../lib/littlefs-lib
    ../../lib/stdinit-lib
    ../../lib/lw_oopc
    ../../src/
    ../../src/interface)

LIST(APPEND source_rp2 ../../src/onelua.c)
AUX_SOURCE_DIRECTORY(. source_rp2)
AUX_SOURCE_DIRECTORY(../../src/interface source_rp2)
AUX_SOURCE_DIRECTORY(../../lib/stdinit-lib/ source_rp2)
AUX_SOURCE_DIRECTORY(../../lib/littlefs-lib/ source_rp2)

add_compile_definitions(LUA_USE_RP2040=1)

ADD_EXECUTABLE(microlua_rp2 ${source_rp2})
target_link_libraries(microlua_rp2 pico_stdlib)

pico_enable_stdio_usb(microlua_rp2 1)
pico_enable_stdio_uart(microlua_rp2 0)

pico_add_extra_outputs(microlua_rp2)
