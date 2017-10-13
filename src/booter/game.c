#include "interrupts.h"
#include "timer.h"
#include "keyboard.h"
#include "sprites.h"
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

    init_game_state();
    draw_game_start();

    game_loop();
}

typedef struct pair {
    int x;
    int y;
} Pair;


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
    int8_t enemy_direction;

    uint16_t num_enemies_left;

    /* array of ints containing num enemies per column (assume no 
    gaps between enemies in a column, so it represents a matrix) */
    uint8_t num_enemy_cols;
    uint8_t num_enemy_rows;
    // matrix of enemies (1 if alive/visible, 0 if dead/invisible)
    uint8_t enemy_mat[NUM_ENEMY_COLS][(VID_HEIGHT / (ALIEN_SIZE + ENEMY_SPACING))];

    // user position
    uint8_t user_bar_height;
    uint8_t user_position_x;
    uint8_t user_position_y; // always the lowest row (not dynamic)

    // bullet queue, counter
    Pair bullet_queue[MAX_BULLETS];
    int bullet_counter;

} Space_Invaders;

static Space_Invaders game;
static uint8_t game_over;

void init_game_state(void) {
    game_over = 0;
    game.score = 0;

    game.info_bar_height = VID_HEIGHT * INFO_BAR_HEIGHT;
    game.user_bar_height = SHIP_SIZE;
    game.user_position_y = VID_HEIGHT - game.user_bar_height;
    // user starts off in middle of screen
    game.user_position_x = (VID_WIDTH - SHIP_SIZE) / 2;

    // enemies start of in aligned center, right below the info space
    // height enemy mat will use
    game.enemy_mat_height = ((VID_HEIGHT - game.info_bar_height 
        - game.user_bar_height) * ENEMY_MAT_HEIGHT);
    game.enemy_mat_width = VID_WIDTH * ENEMY_MAT_WIDTH;
    // enemy starts off at center of screen
    game.enemy_mat_position_x = ((VID_WIDTH - game.enemy_mat_width) / 2);
    game.enemy_mat_position_y = game.info_bar_height; // start at top
    game.enemy_direction = NO_DIR;

    // set maximum # enemies per col to each col's num_enemies_per_col
    game.num_enemy_rows = (game.enemy_mat_height / 
        (ALIEN_SIZE + ENEMY_SPACING));
    game.num_enemy_cols = ((VID_WIDTH * ENEMY_MAT_WIDTH) / 
        (ALIEN_SIZE + ENEMY_SPACING));

    // set all enemies as valid (1) 
    for (int c = 0; c < game.num_enemy_cols; c++) {
        for (int r = 0; r < game.num_enemy_rows; r++) {
            game.enemy_mat[c][r] = 1;
        }
    }

    game.num_enemies_left = (game.num_enemy_rows * game.num_enemy_cols);


    for (int i = 0; i < MAX_BULLETS; i++) {
        game.bullet_queue[i].x = -1;
        game.bullet_queue[i].y = -1;
    }
    game.bullet_counter = 0;
}


void handle_enemy_user_collision(int ex, int ey) {
    /* Check for collision between enemy and user */
    // If enemy's y is below the user height
    if ((ey + ALIEN_SIZE) < game.user_position_y) {
        return;
    }

    // and if right or left of enemy is in bound of user
    if (((ex + ALIEN_SIZE) < (game.user_position_x)) &&
        (ex > (game.user_position_x + SHIP_SIZE))) {
        return;
    }

    // If collision exists, reduce user's lives, and redraw the game
    game_over = 1;
}


void update_enemies(void) {
    /* Clear enemies. */
    draw_box(game.enemy_mat_position_x, game.enemy_mat_position_y, 
        game.enemy_mat_width, game.enemy_mat_height, 0);

    /* Redraw bullets, which were just erased. */
    update_bullets(0);
    
    /* Move enemies. */
    if (game.enemy_direction == RIGHT_DIR) {
        game.enemy_mat_position_x += ENEMY_SPEED;

        // If collision with right wall, then move left
        if ((game.enemy_mat_position_x + game.enemy_mat_width + ENEMY_SPEED) > VID_WIDTH) {
            // game.enemy_mat_position_x = (VID_WIDTH - game.enemy_mat_width);
            game.enemy_direction = LEFT_DIR;
            game.enemy_mat_position_y += ENEMY_DROP_SPEED;
        }
    } 
    
    else if (game.enemy_direction == LEFT_DIR) {
        game.enemy_mat_position_x -= ENEMY_SPEED;

        // If collision with left wall, then move right
        if (game.enemy_mat_position_x < ENEMY_SPEED) {
            game.enemy_direction = RIGHT_DIR;
            game.enemy_mat_position_y += ENEMY_DROP_SPEED;
        }
    }

    /* Redraw enemies. */
    int ex, ey;
    ex = game.enemy_mat_position_x;

    for (int c = 0; c < game.num_enemy_cols; c++) {
        ey = game.enemy_mat_position_y;

        for (int r = 0; r < game.num_enemy_rows; r++) {
            if (game.enemy_mat[c][r]) {
                /* Draw alien. */
                draw_sprite(&alien[0][0], ex, ey, ALIEN_SIZE, ALIEN_SIZE, 2);
                handle_enemy_user_collision(ex, ey);
                ey += ALIEN_SIZE + ENEMY_SPACING;
                // set collision detection with user here
            }
        }

        ex += ALIEN_SIZE + ENEMY_SPACING;
    }
}

void draw_game_start(void) {
    // draw info bar
    draw_box(0, 0, VID_WIDTH, game.info_bar_height, 1); // blue

    /* Draw user in user bar. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, 14);

    // draw enemies
    update_enemies();

    // enemies will start moving in right direction
    game.enemy_direction = RIGHT_DIR;
}


void move_user(int dx) {
    /* Clear user. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, 0);
    
    /* Move user. */
    game.user_position_x += dx;

    /* Redraw user. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, 14);
}

void update_bullets(int dy) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullet_queue[i].y != -1) {

            /* Erase old bullet. */
            draw_bullet(game.bullet_queue[i].x,
                        game.bullet_queue[i].y, 0);

            /* Check if bullet is still in game. */
            if (game.bullet_queue[i].y <= game.info_bar_height) {
                game.bullet_queue[i].x = -1;
                game.bullet_queue[i].y = -1;
                continue;
            }

            /* Update bullet location. */
            game.bullet_queue[i].y += dy;

            /* Draw bullet. */
            draw_bullet(game.bullet_queue[i].x,
                        game.bullet_queue[i].y, 10);
        }
    }
}

void fire_bullet(void) {
    int x = game.user_position_x + (SHIP_SIZE / 2) - 1;
    int y = game.user_position_y;

    /* If we're overwriting a bullet (i.e., hit MAX_BULLETS),
     * clear it first.
     */
    if (game.bullet_queue[game.bullet_counter].y != -1) {
        draw_bullet(game.bullet_queue[game.bullet_counter].x,
                        game.bullet_queue[game.bullet_counter].y, 0);
    }

    game.bullet_queue[game.bullet_counter].x = x;
    game.bullet_queue[game.bullet_counter].y = y;
    game.bullet_counter = (game.bullet_counter + 1) % MAX_BULLETS; 
}

void reset_game(void) {
    draw_box(0, 0, VID_WIDTH, game.info_bar_height, 4);
    sleep(1);
    clear_screen();
    init_game_state();
    draw_game_start();
}

void game_loop(void) {
    unsigned char keycode;
    char empty = KEY_QUEUE_EMPTY;

    uint32_t last_enemy_update = get_time();
    uint32_t last_bullet_update = get_time();
    
    while (1) {
        uint32_t current_time = get_time();

        keycode = key_queue_pop();
        if (keycode != KEY_QUEUE_EMPTY) {
            if (keycode == LEFT_ARROW) {
                move_user(LEFT_DIR);
            } else if (keycode == RIGHT_ARROW) {
                move_user(RIGHT_DIR);
            } else if (keycode == SPACEBAR) {
                fire_bullet();
            } else if (keycode == KEY_R) {
                reset_game();
            }
        }

        if ((current_time - last_enemy_update) > ENEMY_UPDATE_PERIOD) {
            update_enemies();
            last_enemy_update = get_time();
        }
        if ((current_time - last_bullet_update) > BULLET_UPDATE_PERIOD) {
            update_bullets(-1);
            last_bullet_update = get_time();
        }

        if (game_over) {
            reset_game();
            game_over = 0;
        }
    }
}
