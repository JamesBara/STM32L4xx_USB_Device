cmake_minimum_required (VERSION 3.29.0)
project(bsp)
add_library(bsp INTERFACE)

enable_language(C)

target_compile_definitions(bsp INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
)


target_include_directories(bsp INTERFACE
    inc
)

target_sources(bsp INTERFACE
    src/bsp.c
)


target_link_libraries(bsp INTERFACE
    STM32L4xx
)