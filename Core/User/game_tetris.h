/*
 * game_tetris.h
 *
 *  Tetris on the GAME1 page, drawn into the two canvases EEZ lays out there.
 *
 *  Why this exists: the dashboard has two buttons and a lot of idle time in the
 *  pits. It is deliberately confined to its own page and its own module - it
 *  runs only while that page is on screen, and nothing else in the firmware
 *  depends on it.
 *
 *  Controls, given two buttons:
 *
 *      tap left / right     move one column that way; the walls block
 *      hold left / right    rotate that way, repeating while held
 *      both, held 2 s       leave the page
 *
 *  Telling a tap from a hold is what gets four actions out of two buttons. The
 *  cost is that a move lands on release rather than on press.
 */

#ifndef GAME_TETRIS_H
#define GAME_TETRIS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Attach the canvases and reset the board. Call once, after ui_init().
 */
void GameTetris_Init(void);

/**
 * Tell the game whether its page is on screen.
 *
 * Entering starts a fresh game. Leaving stops the clock, so a piece does not
 * quietly land while the driver is looking at the battery page.
 */
void GameTetris_SetActive(bool active);

/** Whether the game page is currently on screen. */
bool GameTetris_IsActive(void);

/**
 * Raw button states, sampled at NAV_SCAN_PERIOD_MS like everything else.
 *
 * The game does its own edge detection and auto-repeat rather than reusing
 * nav.c's: a press that steps one column and then repeats while held is what
 * makes crossing the board practical, and nav.c deliberately fires once per
 * press because pages should not scroll past while a button is down.
 */
void GameTetris_Buttons(bool button1_pressed, bool button2_pressed);

/**
 * Advance gravity and redraw if anything moved. Call from the main loop.
 * Does nothing while the page is not on screen.
 */
void GameTetris_Service(uint32_t now_ms);

/** Score as text, for the label on the game page. */
const char *GameTetris_ScoreText(void);

#endif /* GAME_TETRIS_H */
