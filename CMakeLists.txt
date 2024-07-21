cmake_minimum_required (VERSION 3.29.0)
project(STM32L4xx_USB_Device)
add_library(STM32L4xx_USB_Device INTERFACE)

enable_language(C)

target_include_directories(STM32L4xx_USB_Device INTERFACE
    inc
)

target_sources(STM32L4xx_USB_Device INTERFACE
    src/usbd_core.c
)

target_link_libraries(STM32L4xx_USB_Device INTERFACE
    STM32L4xx
)