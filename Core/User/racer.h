/*
 * racer.h
 *
 *  Pseudo-3D road renderer on the GAME2 page, after RacerJS by onaluf (MIT).
 *
 *  The technique is the OutRun one: the road is a list of segments, each is
 *  projected to the screen, and the shape between two projected segments is a
 *  trapezoid - which is to say a stack of horizontal spans. So the whole
 *  renderer is span fills into an RGB565 buffer, the same thing game_tetris.c
 *  and cell_map.c already do, with no polygon rasteriser and no help from LVGL.
 *
 *  Its buffer is a full screen, 261 KB, which is why it comes from SDRAM rather
 *  than the internal RAM the other two use.
 *
 *  Controls are the ones the dashboard already has: the wheel steers and the
 *  pedals drive when the car is talking, and the buttons stand in for the wheel
 *  when it is not.
 */

#ifndef RACER_H
#define RACER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * The numbers worth arguing about.
 *
 * Runtime rather than compile time so sim/racer_tune.c can nudge them while
 * the road is moving - which is the only way to judge any of them. The
 * firmware never writes them; whatever the tuning rig settles on gets pasted
 * back into Racer_Defaults().
 */
typedef struct {
    float segment_length;   /* world units per road segment */
    float road_width;
    float camera_height;
    float camera_depth;     /* 1/tan(fov/2); smaller is wider */
    int   draw_segments;    /* how far down the road to draw */

    float max_speed;
    float accel;
    float braking;
    float decel;            /* coasting */
    float off_road_decel;
    float steer_rate;       /* how fast the wheel moves the car across */
    float centrifugal;      /* how hard a corner throws it outwards */
} racer_tuning_t;

/** The live set. Writable, but only the tuning rig writes it. */
racer_tuning_t *Racer_Tuning(void);

/** Put the tuning back to the values compiled in. */
void Racer_Defaults(void);

/** How many cones have been clipped since the run started. */
uint32_t Racer_ConesHit(void);

/** Restart the run without changing the tuning. */
void Racer_Restart(void);

/** Claim the buffer and attach the canvas. Call after ui_init(). */
void Racer_Init(void);

/** Tell the game whether its page is on screen. Entering restarts the run. */
void Racer_SetActive(bool active);

/** Whether the game page is currently on screen. */
bool Racer_IsActive(void);

/** Steering from the buttons, for when there is no wheel to read. */
void Racer_Buttons(bool button1_pressed, bool button2_pressed);

/** Advance and redraw. Call from the main loop; cheap while inactive. */
void Racer_Service(uint32_t now_ms);

#endif /* RACER_H */
