set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_CROSSCOMPILING ON)
set(CMAKE_C_COMPILER_ID GNU)

set(TOOLCHAIN_PREFIX "arm-none-eabi-")

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_LINKER ${CMAKE_C_COMPILER})
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_STRIP ${TOOLCHAIN_PREFIX}strip)

set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# STM32L4xx specific flags
set(TARGET_LINKER_SCRIPT "STM32L412.ld")
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections -fno-delete-null-pointer-checks") # -ffreestanding


if(CMAKE_BUILD_TYPE MATCHES Debug)
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb3")
   # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ggdb3 -DNDEBUG")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ggdb3")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ggdb0 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 --ggdb -DNDEBUG")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -ggdb0 -DNDEBUG")
endif()

# Adds some preprocessor options that I am unsure if they are needed.
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")

set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${CMAKE_SOURCE_DIR}/${TARGET_LINKER_SCRIPT}\"")

set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref -Wl,--gc-sections -lgcc -Wl,--print-memory-usage") # -nostdlib.
