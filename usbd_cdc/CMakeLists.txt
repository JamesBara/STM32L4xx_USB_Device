cmake_minimum_required (VERSION 3.29.0)
project(usbd_cdc)
add_library(usbd_cdc INTERFACE)

enable_language(C)

target_compile_definitions(usbd_cdc INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
)


target_include_directories(usbd_cdc INTERFACE
    inc
)

target_sources(usbd_cdc INTERFACE
    src/usbd_cdc.c
)


target_link_libraries(usbd_cdc INTERFACE
    usbd_core
)