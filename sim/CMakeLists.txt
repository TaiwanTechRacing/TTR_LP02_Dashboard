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
get_filename_component(REPO_ROOT_ABS ${REPO_ROOT} ABSOLUTE)

file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    ${REPO_ROOT}/Drivers/lvgl/src/*.c
)

file(GLOB UI_SOURCES CONFIGURE_DEPENDS
    ${REPO_ROOT}/Core/User/ui/*.c
)

# Everything except the entry point, shared by both executables.
set(SHARED_SOURCES
    sim_data.c
    sim_app.c
    sim_qspi.c
    ${UI_SOURCES}
    ${LVGL_SOURCES}
    ${REPO_ROOT}/Core/User/ui_bind.c
    ${REPO_ROOT}/Core/User/vehicle_data.c
    ${REPO_ROOT}/Core/User/debug_overlay.c

    # Page order and button behaviour, shared with main.c so the simulator
    # runs the real thing rather than an imitation of it.
    ${REPO_ROOT}/Core/User/nav.c
    ${REPO_ROOT}/Core/User/game_tetris.c

    # Compiles unmodified. It reaches the flash through
    # BSP_QSPI_GetMappedBase(), which sim_qspi.c answers with a file loaded
    # into ordinary memory instead of the register-level driver.
    ${REPO_ROOT}/Core/User/gif_pages.c
)

# Windowed simulator.
add_executable(${PROJECT_NAME} sim_main.c ${SHARED_SOURCES})

# Headless screenshot tool. Renders into a plain buffer and writes a BMP, so
# the layout can be reviewed over a remote session or checked in CI without
# anyone looking at a screen.
add_executable(dashboard_shot sim_shot.c ${SHARED_SOURCES})

foreach(target ${PROJECT_NAME} dashboard_shot)
target_include_directories(${target} PRIVATE
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
target_compile_definitions(${target} PRIVATE
    LV_CONF_INCLUDE_SIMPLE
    LV_LVGL_H_INCLUDE_SIMPLE
    LV_SIMULATOR

    # So sim_qspi.c can find the GIF image from any working directory. Looking
    # only in the current one made a missing file indistinguishable from a
    # blank flash part.
    SIM_REPO_ROOT="${REPO_ROOT_ABS}"

    # LVGL's Win32 backend uses the touch API (TOUCHINPUT, TOUCHEVENTF_*) and
    # per-monitor DPI messages (WM_DPICHANGED). The MinGW headers gate those
    # behind the target Windows version, so ask for Windows 10.
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00

    # TDM-GCC ships a winuser.h that predates per-monitor DPI, so WM_DPICHANGED
    # is missing no matter what WINVER says. Supply the documented value.
    WM_DPICHANGED=0x02E0
)

target_compile_options(${target} PRIVATE
    -Wall
    -O2
    # Match the firmware's link model. Without this the simulator trips over
    # symbols EEZ declares but never emits data for - an image whose source is
    # in a format its converter silently skips, for instance - while the
    # firmware links cleanly because --gc-sections drops them.
    -ffunction-sections
    -fdata-sections
)

target_link_options(${target} PRIVATE -Wl,--gc-sections)

target_link_libraries(${target} PRIVATE user32 gdi32)
endforeach()
