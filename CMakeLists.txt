cmake_minimum_required(VERSION 3.22)

project(DashBoard_re_1010 C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(MCU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

set(COMMON_FLAGS
    -Wall
    -fdata-sections
    -ffunction-sections
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32H743IITX_FLASH.ld)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    Core/Src/*.c
    Core/User/*.c
)

# HAL 用 glob 而不是像 LP02_VCU 那樣逐一列出。這個專案的 HAL 來源資料夾就是
# CubeMX 產生的那一份,每個 .c 內部都有 HAL_xxx_MODULE_ENABLED 保護,沒啟用的
# 模組會編成空的目的檔。glob 的好處是之後用 CubeMX 加周邊時不必回來改這裡。
file(GLOB STM32_HAL_SOURCES CONFIGURE_DEPENDS
    Drivers/STM32H7xx_HAL_Driver/Src/*.c
)

# 只收 lvgl/src。submodule 裡還有 tests/ examples/ demos/,那些是給桌面環境跑的,
# 用的是 <lvgl/lvgl.h> 這種 include 路徑,拉進來會編不過 —— CubeIDE 的 Release
# 組態就是因為沒排除它們才一直建置失敗。
file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    Drivers/lvgl/src/*.c
)

add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${STM32_HAL_SOURCES}
    ${LVGL_SOURCES}
    Core/Startup/startup_stm32h743iitx.s
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    Core/User
    Core/Src/ui
    Drivers                                      # lv_conf.h 放在這一層
    Drivers/lvgl
    Drivers/STM32H7xx_HAL_Driver/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/CMSIS/Include
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    USE_HAL_DRIVER
    STM32H743xx
    USE_PWR_LDO_SUPPLY
    LV_CONF_INCLUDE_SIMPLE
    LV_LVGL_H_INCLUDE_SIMPLE
    $<$<CONFIG:Debug>:DEBUG>
)

# Debug 用 -O2 而不是 LP02_VCU 的 -Og。
#
# 這個專案的主要負載是 LVGL 的軟體繪圖,對優化等級極度敏感:-O0 的時候整個
# 畫面更新率被壓在個位數到十幾 fps。而且 .launch 燒的就是 Debug,等於車上跑的
# 是 Debug —— 所以這裡優先選速度。-O2 連帶讓 Flash 少了約 143 KB。
#
# 需要逐行單步除錯時,把下面這行暫時改成 -Og 即可,-g3 兩者都有。
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${MCU_FLAGS}
    ${COMMON_FLAGS}
    $<$<CONFIG:Debug>:-O2 -g3>
    $<$<CONFIG:Release>:-O2 -g0>
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -specs=nano.specs
    -specs=nosys.specs
    # newlib-nano 預設不含浮點數的 printf。PushValueAsFloat() 用 "%.1f" 印電壓
    # 和溫度,少了這個所有數值都會變成空白或亂碼。
    -u _printf_float
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE c m)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
)
