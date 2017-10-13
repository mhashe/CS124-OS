#ifndef GAME_H
#define GAME_H

#include "video.h"
#include "sprites.h"

/* Define constants/preferences for the game */

#define RESET_TIME 1.5 // time to hold game between resets, in seconds

/* Game update frequency */
// Centiseconds of interval between movement of enemy matrix
#define ENEMY_UPDATE_PERIOD 30 
// Number of pixels to move every ENEMY_UPDATE_PERIOD
#define ENEMY_SPEED 5 
// Number of pixels to move enemies down when they reach wall
#define ENEMY_DROP_SPEED 4 
// Centiseconds of interval between movement of bullets
#define BULLET_UPDATE_PERIOD 10 
// Number of pixels to move bullets up every BULLET_UPDATE_PERIOD
#define BULLET_SPEED 2
#define ALIEN_BULLET_SPEED 3

/* Floats in [0, 1] of fraction of available canvas's dedicated to for space 
invading enemies. */
#define ENEMY_MAT_WIDTH 0.5
#define ENEMY_MAT_HEIGHT 0.5
#define ENEMY_MAT_WIDTH_PX 160
#define NUM_ENEMY_COLS ENEMY_MAT_WIDTH_PX / (15 + ENEMY_SPACING)

/* Number of pixels in between space invaders on right and left */
#define ENEMY_SPACING 5 // pixels between enemies

/* Floats in [0, 1] */
#define INFO_BAR_HEIGHT 0.05

/* Maximum number of your bullets on screen at once. */
#define MAX_BULLETS     10

/* Maximum number of enemy bullets on screen at once. */
#define ENEMY_BULLETS   10


/* Define key presses */
#define SPACEBAR 0x39
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D
#define KEY_R 0x13

/* Define directions */
#define LEFT_DIR -1
#define RIGHT_DIR 1
#define NO_DIR 0

/* Gameover states */
#define GO_PLAY 0
#define GO_LOST 1
#define GO_WON 2

void c_start(void);

void init_game_state(void);

void draw_game_start(void);

void game_loop(void);

void move_user(int dx);

void game_loop(void);

void fire_bullet(void);

void update_enemies(void);

void update_bullets(int dy, int ady);

void reset_game(uint8_t color);

#endif /* GAME_H */



