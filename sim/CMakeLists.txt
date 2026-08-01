#
# PC simulator for the dashboard UI.
#
# Separate from the firmware build on purpose: this one targets the host, so it
# must not inherit the ARM toolchain file. Configure it from this directory:
#
#     cmake -S sim -B sim/build -G "MinGW Makefiles"
#     cmake --build sim/build
#     sim/build/dashboard_sim.exe
#
# What it compiles is deliberately the same code that runs in the car -
# Core/User/ui (EEZ output), ui_bind.c and vehicle_data.c are shared verbatim.
# Only the display backend, the data source and HAL_GetTick() are swapped.
#
cmake_minimum_required(VERSION 3.22)

project(dashboard_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    ${REPO_ROOT}/Drivers/lvgl/src/*.c
)

file(GLOB UI_SOURCES CONFIGURE_DEPENDS
    ${REPO_ROOT}/Core/User/ui/*.c
)

add_executable(${PROJECT_NAME}
    sim_main.c
    ${UI_SOURCES}
    ${LVGL_SOURCES}
    ${REPO_ROOT}/Core/User/ui_bind.c
    ${REPO_ROOT}/Core/User/vehicle_data.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    # shim first: it supplies the stand-in stm32h7xx_hal.h so the shared
    # firmware sources compile unmodified
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${REPO_ROOT}/Drivers          # lv_conf.h
    ${REPO_ROOT}/Drivers/lvgl
    ${REPO_ROOT}/Core/User
    ${REPO_ROOT}/Core/User/ui
)

# LV_SIMULATOR switches the few target-specific settings in lv_conf.h: the LVGL
# heap moves off the SDRAM address, and the Win32 display backend is enabled.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    LV_CONF_INCLUDE_SIMPLE
    LV_LVGL_H_INCLUDE_SIMPLE
    LV_SIMULATOR

    # LVGL's Win32 backend uses the touch API (TOUCHINPUT, TOUCHEVENTF_*) and
    # per-monitor DPI messages (WM_DPICHANGED). The MinGW headers gate those
    # behind the target Windows version, so ask for Windows 10.
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00

    # TDM-GCC ships a winuser.h that predates per-monitor DPI, so WM_DPICHANGED
    # is missing no matter what WINVER says. Supply the documented value.
    WM_DPICHANGED=0x02E0
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -O2
)

target_link_libraries(${PROJECT_NAME} PRIVATE user32 gdi32)
