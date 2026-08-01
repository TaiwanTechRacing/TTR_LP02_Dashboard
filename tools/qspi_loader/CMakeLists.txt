#
# STM32CubeProgrammer external loader for the W25Q64 on QUADSPI.
#
#   cmake -S tools/qspi_loader -B tools/qspi_loader/build -G "Ninja" \
#         -DCMAKE_TOOLCHAIN_FILE=../../cmake/arm-none-eabi-gcc.cmake
#   cmake --build tools/qspi_loader/build
#
# Produces W25Q64_TTR.stldr. Copy it to the CubeProgrammer external loader
# directory - see README.md.
#
cmake_minimum_required(VERSION 3.22)

project(qspi_loader C ASM)

set(CMAKE_C_STANDARD 11)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(MCU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

add_executable(W25Q64_TTR.stldr
    Loader_Src.c
    Dev_Inf.c

    # Reused verbatim from the firmware rather than reimplemented, so the
    # loader cannot drift from the code that reads the part at runtime.
    ${REPO_ROOT}/Core/User/bsp_qspi.c

    # The loader runs with no startup code, so only the HAL pieces its own
    # setup touches are pulled in.
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
    ${REPO_ROOT}/Core/Src/system_stm32h7xx.c
)

target_include_directories(W25Q64_TTR.stldr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REPO_ROOT}/Core/Inc
    ${REPO_ROOT}/Core/User
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${REPO_ROOT}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    ${REPO_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${REPO_ROOT}/Drivers/CMSIS/Include
)

target_compile_definitions(W25Q64_TTR.stldr PRIVATE
    USE_HAL_DRIVER
    STM32H743xx
    USE_PWR_LDO_SUPPLY
)

target_compile_options(W25Q64_TTR.stldr PRIVATE
    ${MCU_FLAGS}
    -Os                 # the loader lives in RAM; size matters more than speed
    -Wall
    -fdata-sections
    -ffunction-sections
    -fno-builtin
)

target_link_options(W25Q64_TTR.stldr PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/loader.ld
    -nostartfiles       # CubeProgrammer calls the entry points directly
    -nostdlib
    -Wl,--gc-sections

    # Only Init is reachable from ENTRY, so --gc-sections discards the rest.
    # CubeProgrammer looks every one of these up by name in the symbol table,
    # and a loader missing Write fails at the point of writing rather than at
    # load time - so pin them explicitly.
    -Wl,--undefined=Write
    -Wl,--undefined=Read
    -Wl,--undefined=SectorErase
    -Wl,--undefined=MassErase
    -Wl,--undefined=Verify

    -Wl,-Map=${CMAKE_BINARY_DIR}/W25Q64_TTR.map
)
