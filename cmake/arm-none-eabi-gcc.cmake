set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(ARM_TOOLCHAIN_HINTS "")

if(DEFINED ENV{ARM_NONE_EABI_TOOLCHAIN_PATH})
    list(APPEND ARM_TOOLCHAIN_HINTS "$ENV{ARM_NONE_EABI_TOOLCHAIN_PATH}")
endif()

file(GLOB STM32CUBEIDE_TOOLCHAIN_DIRS
    "C:/ST/STM32CubeIDE_*/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin"
)
list(APPEND ARM_TOOLCHAIN_HINTS ${STM32CUBEIDE_TOOLCHAIN_DIRS})

find_program(ARM_NONE_EABI_GCC
    NAMES arm-none-eabi-gcc arm-none-eabi-gcc.exe
    HINTS ${ARM_TOOLCHAIN_HINTS}
)

find_program(ARM_NONE_EABI_OBJCOPY
    NAMES arm-none-eabi-objcopy arm-none-eabi-objcopy.exe
    HINTS ${ARM_TOOLCHAIN_HINTS}
)

find_program(ARM_NONE_EABI_SIZE
    NAMES arm-none-eabi-size arm-none-eabi-size.exe
    HINTS ${ARM_TOOLCHAIN_HINTS}
)

if(NOT ARM_NONE_EABI_GCC)
    message(FATAL_ERROR "arm-none-eabi-gcc not found. Add it to PATH or set ARM_NONE_EABI_TOOLCHAIN_PATH to the toolchain bin folder.")
endif()

if(NOT ARM_NONE_EABI_OBJCOPY)
    message(FATAL_ERROR "arm-none-eabi-objcopy not found. Add it to PATH or set ARM_NONE_EABI_TOOLCHAIN_PATH.")
endif()

if(NOT ARM_NONE_EABI_SIZE)
    message(FATAL_ERROR "arm-none-eabi-size not found. Add it to PATH or set ARM_NONE_EABI_TOOLCHAIN_PATH.")
endif()

set(CMAKE_C_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_OBJCOPY ${ARM_NONE_EABI_OBJCOPY} CACHE FILEPATH "objcopy")
set(CMAKE_SIZE ${ARM_NONE_EABI_SIZE} CACHE FILEPATH "size")
