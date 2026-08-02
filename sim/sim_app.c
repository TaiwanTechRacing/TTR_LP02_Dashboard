/*
 * sim_app.c
 */

#include "sim_app.h"
#include "sim_data.h"
#include "sim_qspi.h"

#include "ui.h"
#include "ui_bind.h"
#include "debug_overlay.h"
#include "gif_pages.h"
#include "nav.h"
#include "game_tetris.h"
#include "cell_map.h"

#include <stdbool.h>

static bool     s_welcome_done;
static uint32_t s_last_ui_update;
static uint32_t s_last_button_scan;
static uint8_t  s_start_page = 1u;   /* where the welcome hand-off lands */
static bool     s_button1;
static bool     s_button2;

void SimApp_Reset(void)
{
    s_welcome_done = false;
    s_last_ui_update = 0;
    s_last_button_scan = 0;
    s_button1 = false;
    s_button2 = false;

    /* LVGL shows the performance label as soon as a display exists; the
     * firmware hides it at boot and so does this. */
    DebugOverlay_Init();
    Nav_Init();

    /* Same order as main.c: the image has to be readable before the GIF
     * widgets are built, because gif_pages.c points them straight at it. */
    (void)SimQspi_Load(NULL);
    GifPages_Init();
    GameTetris_Init();
    CellMap_Init();
}

void SimApp_ShowPage(uint8_t index)
{
    s_start_page = index;

    /* Before the hand-off the welcome screen is still up and SimApp_Step()
     * will load this page when it fires. Afterwards, switch immediately. */
    if (s_welcome_done) {
        Nav_ShowPage(index);
    }
}

void SimApp_SetButtons(bool button1_pressed, bool button2_pressed)
{
    s_button1 = button1_pressed;
    s_button2 = button2_pressed;
}

void SimApp_Step(uint32_t now)
{
    SimData_Feed(now);

    if (!s_welcome_done && UIBind_BootComplete()) {
        s_welcome_done = true;
        Nav_ShowPage(s_start_page);
        UIBind_ArmStartupSweep();
    }

    if ((now - s_last_ui_update) >= SIM_UI_PERIOD_MS) {
        s_last_ui_update = now;
        ui_tick();
        UIBind_ApplyDynamicStyles();
    }

    /*
     * Same period as the firmware, because nav.c counts samples rather than
     * milliseconds - scanning at a different rate here would change the
     * debounce and the one-second both-held gesture, and the simulator would
     * stop being evidence about either.
     *
     * The buttons do nothing until the splash screen hands over, matching the
     * car: nav.c's page cycle excludes index 0.
     */
    if ((now - s_last_button_scan) >= NAV_SCAN_PERIOD_MS) {
        s_last_button_scan = now;
        if (s_welcome_done) {
            Nav_Scan(s_button1, s_button2);
        }
    }

    GameTetris_Service(now);
    GifPages_ShowcaseService(now);
    CellMap_Service(now);

    lv_timer_handler();
}
