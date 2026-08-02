/*
 * cell_map.h
 *
 *  The 112 cell voltages, drawn as a grid on the battery page.
 *
 *  The AMS has been sending these every 20 ms since the DBC update and nothing
 *  has ever displayed them. The pack summary on the main screen answers "how
 *  much is left"; this answers "which cell is going to end the run", which is
 *  a different question and the one that is hard to get any other way.
 *
 *  One row per segment, fourteen cells across, coloured by how each cell sits
 *  against the rest of the pack.
 */

#ifndef CELL_MAP_H
#define CELL_MAP_H

#include <stdbool.h>
#include <stdint.h>

/** Attach the canvas and draw once. Call after ui_init(). */
void CellMap_Init(void);

/** Tell the map whether the battery page is on screen. */
void CellMap_SetActive(bool active);

/** Redraw periodically while the page is up. Call from the main loop. */
void CellMap_Service(uint32_t now_ms);

#endif /* CELL_MAP_H */
