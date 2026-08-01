/*
 * sim_app.c
 */

#include "sim_app.h"
#include "sim_data.h"

#include "ui.h"
#include "screens.h"
#include "ui_bind.h"
#include "debug_overlay.h"

#include <stdbool.h>

static bool     s_welcome_done;
static uint32_t s_last_ui_update;

void SimApp_Reset(void)
{
    s_welcome_done = false;
    s_last_ui_update = 0;

    /* LVGL shows the performance label as soon as a display exists; the
     * firmware hides it at boot and so does this. */
    DebugOverlay_Init();
}

void SimApp_Step(uint32_t now)
{
    SimData_Feed(now);

    if (!s_welcome_done && (now >= SIM_WELCOME_HOLD_MS)) {
        s_welcome_done = true;
        loadScreen(SCREEN_ID_MAIN);
        UIBind_ArmStartupSweep();
    }

    if ((now - s_last_ui_update) >= SIM_UI_PERIOD_MS) {
        s_last_ui_update = now;
        ui_tick();
        UIBind_ApplyDynamicStyles();
    }

    lv_timer_handler();
}
