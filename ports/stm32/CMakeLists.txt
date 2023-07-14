cmake_minimum_required(VERSION 3.16.0)

project(microlua_stm32
    LANGUAGES C ASM
)

add_compile_options(
    -mthumb
    -mcpu=cortex-m3
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wextra
    --specs=nano.specs
    --specs=nosys.specs
)

add_link_options(
    -mthumb
    -mcpu=cortex-m3
    -nostartfiles
    -Wl,--gc-sections
    --specs=nano.specs
    --specs=nosys.specs
)

set(MCU_VARIANT stm32f103cb)
set(OPENCM3_LIB opencm3_stm32f1)
set(LIBOPENCM3_CMAKE_DIR libs/libopencm3_cmake)

add_subdirectory(${LIBOPENCM3_CMAKE_DIR} ${OPENCM3_LIB})

set(APP_SRCS
    src/main.c
)

set(APP_INCS
    src
)

add_executable(opencm3_app ${APP_SRCS})