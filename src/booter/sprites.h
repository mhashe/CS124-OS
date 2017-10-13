/*
 * This file contains all the sprite definitions to be written to the display.
 */

#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

#define ALIEN_SIZE 15
#define SHIP_SIZE  15

#define BULLET_WIDTH  3
#define BULLET_HEIGHT 5

#define FONT_WIDTH  3
#define FONT_HEIGHT 5

static const uint8_t alien[][ALIEN_SIZE] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}, 
    {0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0}, 
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, 
    {0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0}, 
    {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}, 
    {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}, 
    {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0}
};

static const uint8_t ship[][SHIP_SIZE] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0}, 
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

static const uint8_t bullet[BULLET_HEIGHT][BULLET_WIDTH] = {
    {0, 1, 0}, 
    {0, 1, 0}, 
    {1, 1, 1}, 
    {1, 1, 1}, 
    {1, 1, 1}
};

static const uint8_t alien_bullet[BULLET_HEIGHT][BULLET_WIDTH] = {
    {1, 1, 1}, 
    {1, 1, 1}, 
    {1, 1, 1},
    {0, 1, 0}, 
    {0, 1, 0}
};

static const uint8_t one[FONT_HEIGHT][FONT_WIDTH] = {
    {0, 1, 0}, 
    {1, 1, 0}, 
    {0, 1, 0},
    {0, 1, 0}, 
    {1, 1, 1}
};

static const uint8_t two[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 0}, 
    {0, 0, 1}, 
    {0, 1, 0},
    {1, 0, 0}, 
    {1, 1, 1}
};

static const uint8_t three[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 0}, 
    {0, 0, 1}, 
    {0, 1, 0},
    {0, 0, 1}, 
    {1, 1, 0}
};

static const uint8_t four[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 0, 1}, 
    {1, 0, 1}, 
    {1, 1, 1},
    {0, 0, 1}, 
    {0, 0, 1}
};

static const uint8_t five[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {1, 0, 0}, 
    {1, 1, 1},
    {0, 0, 1}, 
    {1, 1, 1}
};

static const uint8_t six[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {1, 0, 0}, 
    {1, 1, 1},
    {1, 0, 1}, 
    {1, 1, 1}
};

static const uint8_t seven[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {0, 0, 1}, 
    {0, 1, 1},
    {1, 1, 0}, 
    {1, 0, 0}
};

static const uint8_t eight[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {1, 0, 1}, 
    {1, 1, 1},
    {1, 0, 1}, 
    {1, 1, 1}
};

static const uint8_t nine[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {1, 0, 1}, 
    {1, 1, 1},
    {0, 0, 1}, 
    {0, 0, 1}
};

static const uint8_t zero[FONT_HEIGHT][FONT_WIDTH] = {
    {1, 1, 1}, 
    {1, 0, 1}, 
    {1, 0, 1},
    {1, 0, 1}, 
    {1, 1, 1}
};

#endif /* SPRITES_H */
