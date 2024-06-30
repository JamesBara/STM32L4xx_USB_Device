cmake_minimum_required(VERSION 3.29)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME STM32L4xx_USB_Device)

# Include toolchain file
include("gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for C languages
enable_language(C)

# Core project settings
project(${CMAKE_PROJECT_NAME})

message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})


add_subdirectory(STM32L4xx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    
 "inc/usbd.h" "src/main.c" "src/usbd_hw.c" "inc/usbd_hw.h" "inc/usbd_hw_user_definitions.h")

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    STM32L4xx

    # Add user defined libraries
)