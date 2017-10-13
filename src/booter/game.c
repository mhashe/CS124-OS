#include "interrupts.h"
#include "timer.h"
#include "keyboard.h"
#include "video.h"
#include "game.h"

#include <stdint.h>

/* This is the entry-point for the game! */
void c_start(void) {
    /* TODO:  You will need to initialize various subsystems here.  This
     *        would include the interrupt handling mechanism, and the various
     *        systems that use interrupts.  Once this is done, you can call
     *        enable_interrupts() to start interrupt handling, and go on to
     *        do whatever else you decide to do!
     */

    init_interrupts();

    init_timer();
    init_keyboard();
    init_video();

    enable_interrupts();

    while(1) {}
}


typedef struct Space_Invaders {
    /* Info presented to user */
    uint8_t info_bar_height;
    uint32_t score; // number of enemies killed
    uint8_t lives_remaining;

    /* Position and direction of enemies */
    // top, left corner off enemy matrix. all are pixel units
    uint16_t enemy_mat_position_x; 
    uint16_t enemy_mat_position_y;
    uint16_t enemy_mat_width;
    uint16_t enemy_mat_height;

    uint16_t num_enemies_left;

    /* array of ints contraining num enemies per column (assume no 
    gaps between enemies in a column, so it represents a matrix) */
    uint8_t num_enemy_cols;
    uint8_t num_enemies_per_col[NUM_ENEMY_COLS];

    // user position
    uint8_t user_bar_height;
    uint8_t user_position_x;
    uint8_t user_position_y; // always the lowest row (not dynamic)

} Space_Invaders;

static Space_Invaders game_state;


void initialize_game() {
    game_state.score = 0;
    game_state.lives_remaining = NUM_LIVES;

    game_state.info_bar_height = INFO_BAR_HEIGHT * VID_HEIGHT;
    game_state.user_bar_height = USER_BAR_HEIGHT * VID_HEIGHT;
    game_state.user_position_y = VID_HEIGHT - game_state.user_bar_height;
    // user starts off in middle of screen
    game_state.user_position_x = (VID_WIDTH - USER_SIZE) / 2;

    // enemies start of in aligned center, right below the info space
    // height enemy mat will use
    game_state.enemy_mat_height = ((VID_HEIGHT - game_state.info_bar_height 
        - game_state.user_bar_height) * ENEMY_MAT_HEIGHT);
    game_state.enemy_mat_width = VID_WIDTH * ENEMY_MAT_WIDTH;
    // enemy starts off at center of screen
    game_state.enemy_mat_position_x = ((VID_WIDTH - 
        game_state.enemy_mat_width) / 2);
    game_state.enemy_mat_position_y = game_state.user_bar_height; // start at top

    // set maximum # enemies per col to each col's num_enemies_per_col
    uint8_t max_enemies_per_col = (game_state.enemy_mat_height / 
        (ENEMY_SIZE + ENEMY_SPACING));
    game_state.num_enemy_cols = ((VID_WIDTH * ENEMY_MAT_WIDTH) / 
        (ENEMY_SIZE + ENEMY_SPACING));

    for (int c = 0; c < game_state.num_enemy_cols; c++) {
        game_state.num_enemies_per_col[c] = max_enemies_per_col;
    }

    game_state.num_enemies_left = (max_enemies_per_col * 
        game_state.num_enemy_cols);
}
