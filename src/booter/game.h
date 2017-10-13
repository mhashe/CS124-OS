#ifndef GAME_H
#define GAME_H

#include "video.h"
#include "sprites.h"

/* Define constants/preferences for the game */

#define NUM_LIVES 3

/* Game update frequency */
#define GAME_FPS 60 // user and missile movement udpates at this rate
#define ENEMY_UPDATE_PERIOD 40 // centiseconds movement of enemy matrix
#define ENEMY_SPEED 4 // number of pixels to move every ENEMY_UPDATE_PERIOD
#define BULLET_UPDATE_PERIOD 10 // Speed of bullet udates (cs).

/* Floats in [0, 1] of fraction of available canvas's dedicated to for space 
invading enemies*/
#define ENEMY_MAT_WIDTH 0.5
#define ENEMY_MAT_HEIGHT 0.5
#define ENEMY_MAT_WIDTH_PX 160
#define NUM_ENEMY_COLS ENEMY_MAT_WIDTH_PX / (15 + ENEMY_SPACING)

/* Number of pixels in between space invaders on right and left */
#define ENEMY_SPACING 5 // pixels between enemies

/* Floats in [0, 1] */
#define INFO_BAR_HEIGHT 0.05

/* Maximum number of bullets on screen at once. */
#define MAX_BULLETS     25


/* Define key presses */
#define SPACEBAR 0x39
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D

/* Define directions */
#define LEFT_DIR -1
#define RIGHT_DIR 1
#define NO_DIR 0

void c_start(void);

void init_game_state(void);

void draw_game_start(void);

void game_loop(void);

void move_user(int dx);

void game_loop(void);

void fire_bullet(void);

void update_enemies(void);

#endif /* GAME_H */



