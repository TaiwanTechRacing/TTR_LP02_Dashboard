/*
 * nav.c
 */

#include "nav.h"
#include "gif_pages.h"
#include "debug_overlay.h"
#include "game_tetris.h"
#include "cell_map.h"

#include "ui.h"
#include "screens.h"

#define NAV_DEBOUNCE_SCANS      5U   /* 5 consecutive samples to accept a press = 25 ms */
#define NAV_DEBUG_TOGGLE_SCANS 200U  /* both held 200 x 5 ms = 1 s toggles the overlay */
#define NAV_GAME_EXIT_SCANS    400U  /* both held 2 s leaves the game */

/*
 * Getting into the game: tap the right button twice quickly, then the left one.
 *
 * The page still steps on each of those presses - there is no way to swallow
 * them without making ordinary paging feel laggy - so the sequence looks like
 * flicking forward twice and back once, and then the game appears. That is the
 * whole trick, and it is why the game is not in the page cycle: paging into it
 * by accident would spoil it.
 *
 * Counted in scans rather than milliseconds so nav.c stays free of the HAL, the
 * same way its debounce does.
 */
#define NAV_EGG_DOUBLE_SCANS    80U  /* 400 ms between the two right taps */
#define NAV_EGG_FOLLOW_SCANS   180U  /* 900 ms to then press left */

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
    SCREEN_ID_DEBUG1,    /* 7 */
    SCREEN_ID_DEBUG2,    /* 8 */
    SCREEN_ID_DEBUG3,    /* 9 */
    SCREEN_ID_GAME1,     /* 10 */
};

#define NAV_PAGE_COUNT ((uint8_t)(sizeof(s_screens) / sizeof(s_screens[0])))
#define NAV_MIN_PAGE   1U

/*
 * The game is last in the list and deliberately outside the cycle, so the
 * buttons walk 1..NAV_MAX_PAGE and never land on it. It is reached only by the
 * sequence above.
 */
#define NAV_GAME_PAGE  (NAV_PAGE_COUNT - 1U)
#define NAV_MAX_PAGE   (NAV_PAGE_COUNT - 2U)

static uint8_t s_page;

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
void Nav_Scan(bool button1_pressed, bool button2_pressed)
{
    static struct {
        int8_t  step;
        uint8_t counter;
    } buttons[2] = {
        { -1, 0 },
        { +1, 0 },
    };

    const bool pressed[2] = { button1_pressed, button2_pressed };

    /* Free running, only ever used as a difference, so wrapping is harmless. */
    static uint32_t scan_tick = 0;
    static uint32_t right_tap_prev = 0;   /* 0 means "no tap recorded yet" */
    static uint32_t right_tap_last = 0;

    scan_tick++;

    /*
     * Handle the both-buttons gesture first and return while it is held, so no
     * page change happens. Pressed within the same sampling window (before the
     * 25 ms debounce elapses) nothing flips at all; slightly staggered presses
     * cost one page change first, which is an acceptable trade.
     */
    static uint16_t both_counter = 0;

    if (button1_pressed && button2_pressed) {
        if (both_counter < NAV_GAME_EXIT_SCANS) {
            both_counter++;
        }

        /*
         * Leaving the game takes a longer hold than the overlay toggle. Both
         * buttons are in constant use while playing, so a second is easy to
         * reach by accident mid-piece; two is not.
         */
        if (GameTetris_IsActive()) {
            if (both_counter == NAV_GAME_EXIT_SCANS) {
                Nav_ShowPage(NAV_MIN_PAGE);
            }
        }
        else if (both_counter == NAV_DEBUG_TOGGLE_SCANS) {
            DebugOverlay_Toggle();   /* fire only on the sample that crosses the threshold */
        }

        return;
    }

    both_counter = 0;

    /*
     * The game owns the buttons while it is on screen. It does its own edge
     * detection and auto-repeat, which is why the raw states go through rather
     * than the debounced page steps below.
     */
    if (GameTetris_IsActive()) {
        GameTetris_Buttons(button1_pressed, button2_pressed);
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

        if (i == 1u) {
            right_tap_prev = right_tap_last;
            right_tap_last = scan_tick;
        }
        else if (right_tap_prev != 0u &&
                 (right_tap_last - right_tap_prev) <= NAV_EGG_DOUBLE_SCANS &&
                 (scan_tick - right_tap_last) <= NAV_EGG_FOLLOW_SCANS) {
            right_tap_prev = 0;
            right_tap_last = 0;
            Nav_ShowPage(NAV_GAME_PAGE);
            return;
        }

        int8_t next = (int8_t)s_page + buttons[i].step;
        if (next < (int8_t)NAV_MIN_PAGE) next = (int8_t)NAV_MAX_PAGE;
        if (next > (int8_t)NAV_MAX_PAGE) next = (int8_t)NAV_MIN_PAGE;

        Nav_ShowPage((uint8_t)next);
    }
}
