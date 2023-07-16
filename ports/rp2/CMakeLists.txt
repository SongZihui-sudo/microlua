cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../../lib/pico-sdk/pico_sdk_init.cmake)

PROJECT(MicroLua)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
INCLUDE_DIRECTORIES(
    .
    ../../lib/lw_oopc/
    ../../src/lua
    ../../src/interface)

LIST(APPEND source_rp2 ../../src/lua/onelua.c)
AUX_SOURCE_DIRECTORY(. source_rp2)
AUX_SOURCE_DIRECTORY(.././src/interface source_rp2)
ADD_EXECUTABLE(microlua_rp2 ${source_rp2})
target_link_libraries(microlua_rp2 pico_stdlib)
pico_add_extra_outputs(microlua_rp2)
