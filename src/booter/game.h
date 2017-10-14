#ifndef GAME_H
#define GAME_H


#include "video.h"
#include "sprites.h"


/* Define constants/preferences for the game */

/* Time to sleep during game resets, in seconds. */
#define RESET_TIME 1.5


/* Game update frequency. */
/* Centiseconds of interval between movement of enemy matrix. */
#define ENEMY_UPDATE_PERIOD 20 
/* Number of pixels to move every ENEMY_UPDATE_PERIOD. */
#define ENEMY_SPEED 5
/* Number of pixels to move enemies down when they reach wall. */
#define ENEMY_DROP_SPEED 8
/* Centiseconds of interval between movement of bullets. */
#define BULLET_UPDATE_PERIOD 10 
/* Number of pixels to move bullets up every BULLET_UPDATE_PERIOD. */
#define BULLET_SPEED 2  // bullets from user
#define ALIEN_BULLET_SPEED 3
/* Movement sensitivity of player. */
#define PLAYER_SPEED 3


/* Floats in [0, 1] of fraction of available canvas's dedicated to for space 
 * invading enemies. 
 */
#define ENEMY_MAT_WIDTH 0.5
#define ENEMY_MAT_HEIGHT 0.5
#define ENEMY_MAT_WIDTH_PX 160
#define NUM_ENEMY_COLS ENEMY_MAT_WIDTH_PX / (15 + ENEMY_SPACING)


/* Number of pixels in between space invaders on right and left. */
#define ENEMY_SPACING 5 // pixels between enemies


/* Floats in [0, 1]. */
#define PROGRESS_BAR_HEIGHT 0.05


/* Maximum number of player bullets on screen at once. */
#define MAX_BULLETS     5


/* Maximum number of enemy bullets on screen at once. */
#define ENEMY_BULLETS   50 /* Spamming bullets is a bad strategy. */


/* Define directions. */
#define LEFT_DIR -1
#define RIGHT_DIR 1
#define NO_DIR 0


/* Gameover states. */
#define GAME_PLAY 0
#define GAME_LOST 1
#define GAME_WON 2
#define GAME_PAUSE 3


/* A struct used for holding x-y pairs
 * for bullets.
 */
typedef struct pair {
    int x;
    int y;
} Pair;


/* Struct holding global game
 * state, parameters.
 */
typedef struct Space_Invaders {
    /* Info presented to player */
    uint16_t progress_bar_height;
    uint8_t game_state;

    /* Position and direction of enemies */
    /* Top, left corner off enemy matrix. All are pixel units. */
    uint16_t enemy_mat_position_x; 
    uint16_t enemy_mat_position_y;
    uint16_t enemy_mat_width;
    uint16_t enemy_mat_height;
    int8_t enemy_direction;
    uint16_t enemy_speed;

    uint16_t num_enemies_left;

    /* Array of ints containing num enemies per column (assume no 
     * gaps between enemies in a column, so it represents a matrix).
     */
    uint8_t num_enemy_cols;
    uint8_t num_enemy_rows;
    /* Matrix of enemies (1 if alive/visible, 0 if dead/invisible). */
    uint8_t enemy_mat[NUM_ENEMY_COLS][(VID_HEIGHT / (ALIEN_SIZE + ENEMY_SPACING))];

    /* Player position. */
    uint16_t player_bar_height;
    int16_t player_position_x;
    uint16_t player_position_y; /* Always the lowest row (not dynamic). */

    /* Player bullet queue, counter. */
    Pair bullet_queue[MAX_BULLETS];
    int bullet_counter;

    /* Alien bullet queue, counter. */
    Pair en_bullet_queue[ENEMY_BULLETS];
    int en_bullet_counter;

} Space_Invaders;


/* Entry point of function. 
 * Finishes booting / startup procedures, and
 * then calls main game loop.
 */
void c_start(void);


/* Sets parameters of game from constants
 * defined in this file.
 */
void init_game_state(void);


/* Draws initial layout of game (i.e.,
 * progress bar, player, aliens).
 */
void draw_game_start(void);


/* Main game loop. Accept keyboard input
 * and update game progress.
 */
void game_loop(void);


/* Move enemies forward and check for changes
 * to state, namely win / loss conditions, 
 * collisions between bullets and aliens or
 * aliens and players, etc.
 */
void update_enemies(void);


/* Erase the previous player ship, and draw
 * a new one at modified coordinates.
 *
 * Inputs:
 *     dx - horizontal movement of ship 
 */
void move_player(int dx);


/* Fires a bullet from the player's current location.
 * Since players are limited to a fixed number of bullets,
 * overwrites the oldest if necessary.
 */
void fire_bullet(void);


/* Re-initializes game, for instance after a win / loss
 * condition.
 *
 * Inputs:
 *     color - color of progress in progress bar
 */
void reset_game(uint8_t color);


/* Pauses or unpauses game by changing game state 
 * variable and color of progress bar.
 *
 * Inputs:
 *     color - color of progress in paused progress bar
 */
void pause_game(uint8_t color);


/* Advance all live bullets and perform some 
 * checks (remove off-screen bullets, check
 * for collisions with player, etc.).
 *
 * Inputs:
 *     dy - vertical movement of ship's bullets
 *     ady - vertical movement of alien's bullets
 */
void update_bullets(int dy, int ady);


/* Updates progress bar keeping track of fraction
 * of enemies destroyed.
 *
 * Inputs:
 *     color - color of progress in progress bar
 */
void update_game_progress(uint8_t color);


/* Every time the player shoots, an alien shoots too.
 * That change to state is handled here.
 */
void fire_alien_bullet(void);


#endif /* GAME_H */

