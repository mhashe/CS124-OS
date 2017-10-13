#ifndef GAME_H
#define GAME_H

/* Define constants/preferences for the game */

/* Game update frequency */
#define GAME_FPS 60 // user and missile movement udpates at this rate
#define ENEMY_UPDATE_PERIOD 1000 // milliseconds movement of enemy matrix

/* Floats in [0, 1] of fraction of available canvas's dedicated to for space 
invading enemies*/
#define GAME_ENEMY_WIDTH 0.5
#define GAME_ENEMY_HEIGHT 0.5


#endif /* GAME_H */
