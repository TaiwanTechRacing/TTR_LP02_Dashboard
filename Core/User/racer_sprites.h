/*
 * racer_sprites.h
 *
 *  Car artwork for the racer, converted from RacerJS by
 *  tools/make_racer_sprites.py.
 *
 *  The art is by Selim Arsever (https://github.com/onaluf/RacerJS) and is
 *  licensed CC BY-SA 3.0. The code in that repository is MIT, but the art is
 *  not - so this obligation travels with the binary, and is repeated in the
 *  generated .c and in HANDOFF.
 */

#ifndef RACER_SPRITES_H
#define RACER_SPRITES_H

#include <stdint.h>

/** Pixels equal to this are not drawn. */
#define RACER_SPRITE_TRANSPARENT 0xF81Fu

typedef struct {
    const uint16_t *pixels;   /* RGB565, row major */
    uint16_t        w;
    uint16_t        h;
} racer_sprite_t;

/** Straight, turning left, turning right - in that order. */
#define RACER_CAR_FRAMES 3
#define RACER_CAR_STRAIGHT 0
#define RACER_CAR_LEFT     1
#define RACER_CAR_RIGHT    2

extern const racer_sprite_t racer_car[RACER_CAR_FRAMES];

#endif /* RACER_SPRITES_H */
