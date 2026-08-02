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
