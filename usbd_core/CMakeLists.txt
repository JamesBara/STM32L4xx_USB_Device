cmake_minimum_required (VERSION 3.29.0)
project(usbd_core)
add_library(usbd_core INTERFACE)

enable_language(C)

target_include_directories(usbd_core INTERFACE
    inc
)

target_sources(usbd_core INTERFACE
    src/usbd_core.c
)


target_link_libraries(usbd_core INTERFACE
    STM32L4xx
    bsp
)