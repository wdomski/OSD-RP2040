cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(osd_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(osd
    src/main.c
    src/fonts.c
    ext/STM32Libs/eprintfs/eprintfs.c
    src/i2c-slave.c
)

target_include_directories(osd 
PUBLIC
inc
ext/STM32Libs/eprintfs)

target_compile_options(osd 
PUBLIC 
-O0
)

pico_enable_stdio_usb(osd 1)
pico_enable_stdio_uart(osd 1)
pico_add_extra_outputs(osd)
target_link_libraries(osd 
    pico_stdlib hardware_spi hardware_dma hardware_i2c pico_i2c_slave)
