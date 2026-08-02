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
#include <mmsystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SIM_WIDTH   480
#define SIM_HEIGHT  272

/* The panel is small; 200% makes it comfortable to look at on a desktop. */
#define SIM_ZOOM    200


/*
 * The clock, and why it is not GetTickCount().
 *
 * GetTickCount() advances in steps of about 15.6 ms, not 1 ms. Everything in
 * nav.c is counted in 5 ms scans, so with a clock that coarse the scan gate
 * only opened once per step and every threshold stretched by roughly three
 * times: the 25 ms debounce became 78 ms, the one second overlay hold became
 * three, and the three second gesture that opens the game wanted nine and a
 * half. The simulator was not reproducing the car, it was reproducing a much
 * slower version of it - and the headless screenshot tool, which sets its own
 * clock, was right all along, so nothing in the automated checks ever showed it.
 *
 * QueryPerformanceCounter is sub-microsecond and does not depend on the global
 * timer period, so the reading is correct whatever else the system is doing.
 */
static LARGE_INTEGER s_qpc_freq;
static LONGLONG      s_qpc_start;

uint32_t HAL_GetTick(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint32_t)(((now.QuadPart - s_qpc_start) * 1000LL) / s_qpc_freq.QuadPart);
}

static void restore_timer_period(void)
{
    timeEndPeriod(1);
}


int main(void)
{
    /*
     * A correct clock is not enough on its own: Sleep(1) also rounds up to the
     * timer period, so the loop below would still only run every 15 ms and the
     * scans would still be too far apart. This brings both to about 1 ms.
     */
    timeBeginPeriod(1);
    atexit(restore_timer_period);

    QueryPerformanceFrequency(&s_qpc_freq);

    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
    s_qpc_start = start.QuadPart;

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

    /* A mouse pointer is useful for poking at widgets while checking hit
     * areas. The dashboard buttons are on the keyboard, read below. */
    lv_windows_acquire_pointer_indev(display);

    VehicleData_Init();
    ui_init();
    SimApp_Reset();

    puts("left / right arrow  = the two dashboard buttons");
    puts("both held for 1 s   = toggle the FPS overlay");

    for (;;) {
        /*
         * Read the keys as held-or-not rather than as key events, because that
         * is what the real buttons are: nav.c samples them every 5 ms and
         * counts consecutive samples to debounce. Feeding it events instead
         * would bypass the debounce and the one-second hold gesture, and the
         * simulator would stop telling us anything about either.
         */
        SimApp_SetButtons((GetAsyncKeyState(VK_LEFT)  & 0x8000) != 0,
                          (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0);

        SimApp_Step(HAL_GetTick());
        Sleep(1);
    }
}
