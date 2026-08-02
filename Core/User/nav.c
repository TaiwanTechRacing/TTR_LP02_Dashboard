/*
 * nav.c
 */

#include "nav.h"
#include "gif_pages.h"
#include "debug_overlay.h"

#include "ui.h"
#include "screens.h"

#define NAV_DEBOUNCE_SCANS      5U   /* 5 consecutive samples to accept a press = 25 ms */
#define NAV_DEBUG_TOGGLE_SCANS 200U  /* both held 200 x 5 ms = 1 s toggles the overlay */

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
    SCREEN_ID_BATTERY,   /* 4 */
    SCREEN_ID_INVERTER,  /* 5 */
    SCREEN_ID_DEBUG1,    /* 6 */
    SCREEN_ID_DEBUG2,    /* 7 */
    SCREEN_ID_DEBUG3,    /* 8 */
};

#define NAV_PAGE_COUNT ((uint8_t)(sizeof(s_screens) / sizeof(s_screens[0])))
#define NAV_MIN_PAGE   1U
#define NAV_MAX_PAGE   (NAV_PAGE_COUNT - 1U)

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

    /*
     * Handle the both-buttons gesture first and return while it is held, so no
     * page change happens. Pressed within the same sampling window (before the
     * 25 ms debounce elapses) nothing flips at all; slightly staggered presses
     * cost one page change first, which is an acceptable trade.
     */
    static uint16_t both_counter = 0;

    if (button1_pressed && button2_pressed) {
        if (both_counter < NAV_DEBUG_TOGGLE_SCANS) {
            both_counter++;
            if (both_counter == NAV_DEBUG_TOGGLE_SCANS) {
                DebugOverlay_Toggle();   /* fire only on the sample that crosses the threshold */
            }
        }
        return;
    }

    both_counter = 0;

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
