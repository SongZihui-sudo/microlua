# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(microlua_esp32)

# minimize mode
set(minimize 1)
if(${minimize} EQUAL 1)
    add_compile_options("-DMinimize=1")
endif()
