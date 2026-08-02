/*
 * sim_app.c
 */

#include "sim_app.h"
#include "sim_data.h"

#include "ui.h"
#include "screens.h"
#include "ui_bind.h"
#include "debug_overlay.h"
#include "gif_pages.h"
#include "sim_qspi.h"

#include <stdbool.h>

/*
 * Mirrors screens[] in Core/Src/main.c. gif_pages.c maps animations to
 * positions in this list, so the order has to match or the wrong animation
 * runs - which is exactly the kind of mistake the simulator is here to catch.
 */
static const enum ScreensEnum s_screens[] = {
    SCREEN_ID_WELCOME,
    SCREEN_ID_MAIN,
    SCREEN_ID_SYSTEM,
    SCREEN_ID_BATTERY,
    SCREEN_ID_INVERTER,
    SCREEN_ID_DEBUG1,
    SCREEN_ID_DEBUG2,
    SCREEN_ID_DEBUG3,
};

#define SIM_PAGE_COUNT ((uint8_t)(sizeof(s_screens) / sizeof(s_screens[0])))

static bool     s_welcome_done;
static uint32_t s_last_ui_update;
static uint8_t  s_page = 1u;        /* where the welcome hand-off lands */

void SimApp_Reset(void)
{
    s_welcome_done = false;
    s_last_ui_update = 0;

    /* LVGL shows the performance label as soon as a display exists; the
     * firmware hides it at boot and so does this. */
    DebugOverlay_Init();

    /* Same order as main.c: the image has to be readable before the GIF
     * widgets are built, because gif_pages.c points them straight at it. */
    (void)SimQspi_Load(NULL);
    GifPages_Init();
}

void SimApp_ShowPage(uint8_t index)
{
    if (index >= SIM_PAGE_COUNT) {
        return;
    }

    s_page = index;

    /* Before the hand-off the welcome screen is still up; SimApp_Step() will
     * load this page when it fires. Afterwards, switch immediately. */
    if (s_welcome_done) {
        loadScreen(s_screens[s_page]);
        GifPages_SetVisiblePage(s_page);
    }
}

void SimApp_Step(uint32_t now)
{
    SimData_Feed(now);

    if (!s_welcome_done && (now >= SIM_WELCOME_HOLD_MS)) {
        s_welcome_done = true;
        loadScreen(s_screens[s_page]);
        GifPages_SetVisiblePage(s_page);
        UIBind_ArmStartupSweep();
    }

    if ((now - s_last_ui_update) >= SIM_UI_PERIOD_MS) {
        s_last_ui_update = now;
        ui_tick();
        UIBind_ApplyDynamicStyles();
    }

    lv_timer_handler();
}
