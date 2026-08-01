/*
 * sim_main.c
 *
 *  PC simulator for the dashboard UI.
 *
 *  Runs the exact screens.c that EEZ generates and the exact ui_bind.c that
 *  ships in the firmware, against a synthetic vehicle_data. Only three things
 *  differ from the target: the display is a Win32 window instead of LTDC, the
 *  data comes from the generator below instead of CAN, and HAL_GetTick() is
 *  backed by the Windows tick count.
 *
 *  What it is for: iterating on layout, fonts, colours and the stale-signal
 *  behaviour without a flash cycle. What it cannot tell you: frame rate,
 *  memory pressure, or anything about SDRAM, cache and LTDC timing - those are
 *  properties of the hardware and still need a real board.
 */

#include "lvgl.h"
#include "ui.h"
#include "ui_bind.h"
#include "vehicle_data.h"
#include "sim_data.h"
#include "sim_app.h"

#include <windows.h>
#include <stdbool.h>

#define SIM_WIDTH   480
#define SIM_HEIGHT  272

/* The panel is small; 200% makes it comfortable to look at on a desktop. */
#define SIM_ZOOM    200


static uint32_t s_start_tick;

uint32_t HAL_GetTick(void)
{
    return (uint32_t)(GetTickCount() - s_start_tick);
}


int main(void)
{
    s_start_tick = GetTickCount();

    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    lv_display_t *display = lv_windows_create_display(
        L"TTR LP02 Dashboard - simulator",
        SIM_WIDTH, SIM_HEIGHT,
        SIM_ZOOM,
        false,      /* follow the Windows DPI setting */
        true);      /* simulator mode: fixed size, matching the real panel */

    if (display == NULL) {
        return 1;
    }

    /* The two dashboard buttons have no equivalent here; a mouse pointer is
     * still useful for poking at widgets while checking hit areas. */
    lv_windows_acquire_pointer_indev(display);

    VehicleData_Init();
    ui_init();
    SimApp_Reset();

    for (;;) {
        SimApp_Step(HAL_GetTick());
        Sleep(1);
    }
}
