/*
 * nav.c
 */

#include "nav.h"
#include "gif_pages.h"
#include "debug_overlay.h"
#include "game_tetris.h"
#include "cell_map.h"
#include "racer.h"

#include "ui.h"
#include "screens.h"

#define NAV_DEBOUNCE_SCANS       5U    /* consecutive samples to accept a press */
#define NAV_DEBUG_TOGGLE_MS   1000U   /* both held this long toggles the overlay */
#define NAV_GAME_EXIT_MS      2000U   /* both held this long leaves the game */

/*
 * Getting into a game: hold both buttons for three seconds or more, then let
 * go of one - the right one for tetris, the left one for the racer.
 *
 * Which button comes up first is the choice, so there is no window to hit and
 * nothing to time. Holding longer than three seconds costs nothing.
 *
 * The hold does the work of keeping it hidden - nothing else on this dashboard
 * asks for three seconds - and it passes the one second mark that toggles the
 * FPS overlay, so the overlay flips on at one second and back off at three.
 * That blink is left in deliberately: it is the only feedback the gesture has,
 * and it ends with the overlay where it started.
 *
 * Letting go of both at once picks nothing. That needs both to come up inside
 * one 5 ms sample, which hands do not really do, and retrying costs a hold.
 */
#define NAV_EGG_ARM_MS        3000U   /* both buttons held this long arms it */

/*
 * Page order. Index 0 is the splash screen shown at boot and is excluded from
 * the button cycle; NAV_MIN_PAGE and NAV_MAX_PAGE bound what the buttons reach.
 *
 * gif_pages.c maps animations to positions in this list, so inserting a page
 * ahead of the debug ones moves them - see GIF_FIRST_SCREEN_INDEX there.
 */
static const enum ScreensEnum s_screens[] = {
    SCREEN_ID_WELCOME,   /* 0  splash, boot only */
    SCREEN_ID_MAIN,      /* 1 */
    SCREEN_ID_SYSTEM_SDC, /* 2 */
    SCREEN_ID_SYSTEM_ECU, /* 3 */
    SCREEN_ID_SYSTEM_SENSOR, /* 4 */
    SCREEN_ID_BATTERY,   /* 5 */
    SCREEN_ID_INVERTER,  /* 6 */
#if NAV_DEBUG_PAGES
    SCREEN_ID_DEBUG1,
    SCREEN_ID_DEBUG2,
    SCREEN_ID_DEBUG3,
#endif
    SCREEN_ID_GAME1,     /* second from last - see NAV_GAME_PAGE */
    SCREEN_ID_GAME2,     /* last */
};

#define NAV_PAGE_COUNT ((uint8_t)(sizeof(s_screens) / sizeof(s_screens[0])))
#define NAV_MIN_PAGE   1U

/*
 * The game is last in the list and deliberately outside the cycle, so the
 * buttons walk 1..NAV_MAX_PAGE and never land on it. It is reached only by the
 * sequence above.
 */
#define NAV_GAME_PAGE  (NAV_PAGE_COUNT - 2U)
#define NAV_RACER_PAGE (NAV_PAGE_COUNT - 1U)
#define NAV_MAX_PAGE   (NAV_PAGE_COUNT - 3U)

static uint8_t s_page;

int8_t Nav_PageIndexOf(enum ScreensEnum id)
{
    for (uint8_t i = 0; i < NAV_PAGE_COUNT; i++) {
        if (s_screens[i] == id) {
            return (int8_t)i;
        }
    }

    return -1;
}

void Nav_Init(void)
{
    s_page = 0;
}

uint8_t Nav_CurrentPage(void)
{
    return s_page;
}

void Nav_ShowPage(uint8_t index)
{
    if (index >= NAV_PAGE_COUNT) {
        return;
    }

    s_page = index;
    loadScreen(s_screens[s_page]);

    /* The game only runs while it is being looked at, and takes the buttons
     * for as long as it does. */
    const bool on_game = (s_screens[s_page] == SCREEN_ID_GAME1);
    GameTetris_SetActive(on_game);
    Racer_SetActive(s_screens[s_page] == SCREEN_ID_GAME2);

    /* The showcase lives in the corner of the same page. */
    GifPages_SetShowcaseActive(on_game);

    CellMap_SetActive(s_screens[s_page] == SCREEN_ID_BATTERY);

    /* Only the visible page's animation should run; the others burn CPU on
     * frames nobody can see. */
    GifPages_SetVisiblePage(s_page);
}

/*
 * This was two nearly identical blocks, each with its own counter and flag.
 * It is now table-driven: the counter fires once on reaching the threshold and
 * then saturates there until the button is released, which gives the
 * fire-once behaviour for free - no separate b1f / b2f flags needed.
 */
void Nav_Scan(uint32_t now_ms, bool button1_pressed, bool button2_pressed)
{
    static struct {
        int8_t  step;
        uint8_t counter;
    } buttons[2] = {
        { -1, 0 },
        { +1, 0 },
    };

    const bool pressed[2] = { button1_pressed, button2_pressed };

    /* Set by the three second hold; the next button to come up is the choice. */
    static bool egg_armed = false;

    /*
     * A game opened this way leaves one button still down. Without this the
     * piece would start rotating, or the car steering, the instant the page
     * appeared - the press that chose the game would also be the first move.
     */
    static bool swallow_until_release = false;

    if (!button1_pressed && !button2_pressed) {
        swallow_until_release = false;
    }

    /*
     * Handle the both-buttons gesture first and return while it is held, so no
     * page change happens. Pressed within the same sampling window (before the
     * 25 ms debounce elapses) nothing flips at all; slightly staggered presses
     * cost one page change first, which is an acceptable trade.
     */
    static uint32_t both_since = 0;   /* 0 while they are not both down */
    static bool     both_done = false;
    static bool     overlay_toggled = false;

    if (button1_pressed && button2_pressed) {
        if (both_since == 0u) {
            both_since = now_ms;
            both_done = false;
        }

        if (both_done) {
            return;         /* this hold has already had its effect */
        }

        const uint32_t held = now_ms - both_since;

        /*
         * Leaving the game takes a longer hold than the overlay toggle. Both
         * buttons are in constant use while playing, so a second is easy to
         * reach by accident mid-piece; two is not.
         */
        if (GameTetris_IsActive() || Racer_IsActive()) {
            if (held >= NAV_GAME_EXIT_MS) {
                both_done = true;
                Nav_ShowPage(NAV_MIN_PAGE);
            }
        }
        else if (held >= NAV_EGG_ARM_MS) {
            /* Put the overlay back where it was before this hold started, and
             * wait to see which button comes up. */
            both_done = true;
            DebugOverlay_Toggle();
            egg_armed = true;
        }
        else if (held >= NAV_DEBUG_TOGGLE_MS && !overlay_toggled) {
            overlay_toggled = true;
            DebugOverlay_Toggle();
        }

        return;
    }

    /*
     * Out of the hold. If it armed, whichever button was released picks the
     * game - the other one is still down, which is what tells them apart.
     */
    if (egg_armed) {
        if (button1_pressed && !button2_pressed) {
            egg_armed = false;
            swallow_until_release = true;
            both_since = 0;
            both_done = false;
            overlay_toggled = false;
            Nav_ShowPage(NAV_GAME_PAGE);        /* let go of the right: tetris */
            return;
        }

        if (button2_pressed && !button1_pressed) {
            egg_armed = false;
            swallow_until_release = true;
            both_since = 0;
            both_done = false;
            overlay_toggled = false;
            Nav_ShowPage(NAV_RACER_PAGE);       /* let go of the left: racer */
            return;
        }

        egg_armed = false;      /* both came up together; nothing chosen */
    }

    both_since = 0;
    both_done = false;
    overlay_toggled = false;

    /*
     * The game owns the buttons while it is on screen. It does its own edge
     * detection and auto-repeat, which is why the raw states go through rather
     * than the debounced page steps below.
     */
    if (GameTetris_IsActive()) {
        GameTetris_Buttons(button1_pressed && !swallow_until_release,
                           button2_pressed && !swallow_until_release);
        return;
    }

    if (Racer_IsActive()) {
        Racer_Buttons(button1_pressed && !swallow_until_release,
                      button2_pressed && !swallow_until_release);
        return;
    }

    for (uint8_t i = 0; i < 2u; i++) {
        if (!pressed[i]) {
            buttons[i].counter = 0;   /* released */
            continue;
        }

        if (buttons[i].counter >= NAV_DEBOUNCE_SCANS) {
            continue;                 /* already flipped for this press; wait for release */
        }

        if (++buttons[i].counter < NAV_DEBOUNCE_SCANS) {
            continue;                 /* still debouncing */
        }

        int8_t next = (int8_t)s_page + buttons[i].step;
        if (next < (int8_t)NAV_MIN_PAGE) next = (int8_t)NAV_MAX_PAGE;
        if (next > (int8_t)NAV_MAX_PAGE) next = (int8_t)NAV_MIN_PAGE;

        Nav_ShowPage((uint8_t)next);
    }
}
