#include "interrupts.h"
#include "timer.h"
#include "keyboard.h"
#include "sprites.h"
#include "video.h"
#include "game.h"
#include "sound.h"
#include "keyboard.h"

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

    boot_sound();

    game_loop();
}

/* Global game state. */
static Space_Invaders game;

void init_game_state(void) {
    game.game_over = GO_PLAY;

    /* Define info_bar shape (at top of screen) */
    game.info_bar_height = VID_HEIGHT * INFO_BAR_HEIGHT;
    
    /* User starts off towards bottom, horizontal center of screen. */
    game.user_bar_height = SHIP_SIZE;
    game.user_position_x = (VID_WIDTH - SHIP_SIZE) / 2;

    /* 
     * This 8 is truly a magic number. It seems in the emulator, after you fire
     * a bullet, the bottom 7 rows of the memory buffer stop updating, even
     * though the values are being written to the buffer correctly. This
     * behavior is not present in running outside of the emulator (actually
     * booting into it).
     */
    game.user_position_y = VID_HEIGHT - game.user_bar_height - 8;

    /* Enemies claim all space vertically in-between info and user bars. */
    game.enemy_mat_height = ((VID_HEIGHT - game.info_bar_height 
        - game.user_bar_height) * ENEMY_MAT_HEIGHT);
    game.enemy_mat_width = VID_WIDTH * ENEMY_MAT_WIDTH;
    
    /* Enemies start off in horizontal center, top of screen (under info_bar). */
    game.enemy_mat_position_x = ((VID_WIDTH - game.enemy_mat_width) / 2);
    game.enemy_mat_position_y = game.info_bar_height; // start at top
    game.enemy_direction = NO_DIR;

    /* Number of enemies in enemy matrix (rows and columns). */
    game.num_enemy_rows = (game.enemy_mat_height / 
        (ALIEN_SIZE + ENEMY_SPACING));
    game.num_enemy_cols = ((VID_WIDTH * ENEMY_MAT_WIDTH) / 
        (ALIEN_SIZE + ENEMY_SPACING));
    game.num_enemies_left = (game.num_enemy_rows * game.num_enemy_cols);

    /* Set all enemies as valid/existing (1) in the enemy matrix. */
    for (int c = 0; c < game.num_enemy_cols; c++) {
        for (int r = 0; r < game.num_enemy_rows; r++) {
            game.enemy_mat[c][r] = 1;
        }
    }

    /* No bullets drawn as yet. */
    for (int i = 0; i < MAX_BULLETS; i++) {
        game.bullet_queue[i].x = -1;
        game.bullet_queue[i].y = -1;
    };
    game.bullet_counter = 0;

    for (int i = 0; i < ENEMY_BULLETS; i++) {
        game.en_bullet_queue[i].x = -1;
        game.en_bullet_queue[i].y = -1;
    }
    game.en_bullet_counter = 0;
}



void draw_game_start(void) {
    // draw info bar
    draw_box(0, 0, VID_WIDTH, game.info_bar_height, BLUE);

    /* Draw user in user bar. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, YELLOW);

    // draw enemies
    update_enemies();

    // enemies will start moving in right direction
    game.enemy_direction = RIGHT_DIR;
}


void update_game_progress() {
    uint16_t num_enemies = game.num_enemy_rows * game.num_enemy_cols;
    int progress_width = ((VID_WIDTH * (num_enemies - game.num_enemies_left)) / num_enemies);
    draw_box(0, 0, progress_width, game.info_bar_height, GREEN);
}


void update_enemies(void) {
    /* Clear enemies. */
    draw_box(game.enemy_mat_position_x, game.enemy_mat_position_y, 
        game.enemy_mat_width, game.enemy_mat_height, BLACK);

    /* Redraw bullets, which were just erased. */
    update_bullets(0, 0);
    
    /* Move enemies. */
    if (game.enemy_direction == RIGHT_DIR) {
        game.enemy_mat_position_x += ENEMY_SPEED;

        // If collision with right wall, then move left
        if ((game.enemy_mat_position_x + game.enemy_mat_width + ENEMY_SPEED) 
            > VID_WIDTH) {
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

    /* For every enemy/alien in the enemy matrix, do the following. */
    for (int c = 0; c < game.num_enemy_cols; c++) {
        ey = game.enemy_mat_position_y;

        for (int r = 0; r < game.num_enemy_rows; r++) {
            if (game.enemy_mat[c][r]) {

                // Handle enemy-missile collision
                int cont = 0;
                /* Check for collision with bullet. */
                for (int i = 0; i < MAX_BULLETS; i++) {
                    /* Check if bullet in hitbox. */
                    if    ((game.bullet_queue[i].x >= ex)
                        && (game.bullet_queue[i].x <= ex + ALIEN_SIZE) 
                        && (game.bullet_queue[i].y >= ey)
                        && (game.bullet_queue[i].y <= ey + ALIEN_SIZE)) {
                        /* Collision! */
                        cont = 1;
                        game.bullet_queue[i].x = -1;
                        game.bullet_queue[i].y = -1;
                        break;
                    }
                }

                /* Mark as destroyed; will be removed in next draw cycle. */
                if (cont) {
                    game.enemy_mat[c][r] = 0;
                    game.num_enemies_left--;
                }

                /* Draw alien. */
                draw_sprite(&alien[0][0], ex, ey, ALIEN_SIZE, ALIEN_SIZE, GREEN);
                

                /* Handle collision detection with user. */
                // If enemy's y is below the user height
                if ((ey + ALIEN_SIZE) > (game.user_position_y + 8)) {
                    // and if right or left of enemy is in bound of user
                    if (((ex + ALIEN_SIZE) > (game.user_position_x)) ||
                        (ex < (game.user_position_x + SHIP_SIZE))) {
                        //then set game over so it can be reset
                        game.game_over = GO_LOST;
                    }
                }

                /* If enemy has moved off screen, remove it from the game. */
                if ((ey + ALIEN_SIZE) >= VID_WIDTH) {
                    game.enemy_mat[c][r] = 0;
                    game.num_enemies_left--;
                }
            }

            ey += ALIEN_SIZE + ENEMY_SPACING;
        }

        ex += ALIEN_SIZE + ENEMY_SPACING;
    }

    update_game_progress();

    /* If the game is not lost and all enemies are defeated, user has won. */
    if ((game.num_enemies_left == 0) && (game.game_over == GO_PLAY)) {
        game.game_over = GO_WON;
    }
}


void move_user(int dx) {
    /* Clear user. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, BLACK);
    
    /* Move user. */
    game.user_position_x += dx;

    /* Redraw user. */
    draw_sprite(&ship[0][0], game.user_position_x, game.user_position_y, 
        SHIP_SIZE, SHIP_SIZE, YELLOW);
}

void update_bullets(int dy, int ady) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullet_queue[i].y != -1) {

            /* Erase old bullet. */
            draw_sprite(&bullet[0][0], game.bullet_queue[i].x,
                game.bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);

            /* Check if bullet is still in game.
             * 3 intended as buffer to avoid infringing
             * upon score bar.
             */
            if (game.bullet_queue[i].y <= game.info_bar_height + 3) {
                game.bullet_queue[i].x = -1;
                game.bullet_queue[i].y = -1;
                continue;
            }

            /* Update bullet location. */
            game.bullet_queue[i].y += dy;

            /* Draw bullet. */
            draw_sprite(&bullet[0][0], game.bullet_queue[i].x,
                game.bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                LIGHT_GREEN);            
        }
    }

    for (int i = 0; i < ENEMY_BULLETS; i++) {
        if (game.en_bullet_queue[i].y != -1) {

            /* Erase old bullet. */
            draw_sprite(&alien_bullet[0][0], game.en_bullet_queue[i].x,
                game.en_bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);

            /* Check for collision with bullet. */
            if    ((game.en_bullet_queue[i].x >= game.user_position_x)
                && (game.en_bullet_queue[i].x <= game.user_position_x + SHIP_SIZE) 
                && (game.en_bullet_queue[i].y >= game.user_position_y)
                && (game.en_bullet_queue[i].y <= game.user_position_y + SHIP_SIZE)) {
                /* Collision! */
                game.game_over = GO_LOST;
                return;
            }

            /* Check if bullet is still in game.
             * 3 intended as buffer to avoid infringing
             * upon score bar.
             */
            /* MAGIC NUMBER 8 */
            if (game.en_bullet_queue[i].y >= VID_HEIGHT - 13) {
                game.en_bullet_queue[i].x = -1;
                game.en_bullet_queue[i].y = -1;
                continue;
            }

            /* Update bullet location. */
            game.en_bullet_queue[i].y += ady;

            /* Draw bullet. */
            draw_sprite(&alien_bullet[0][0], game.en_bullet_queue[i].x,
                game.en_bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                RED);            
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
        draw_sprite(&bullet[0][0], game.bullet_queue[game.bullet_counter].x,
            game.bullet_queue[game.bullet_counter].y, BULLET_WIDTH,
            BULLET_HEIGHT, BLACK);
    }

    game.bullet_queue[game.bullet_counter].x = x;
    game.bullet_queue[game.bullet_counter].y = y;
    game.bullet_counter = (game.bullet_counter + 1) % MAX_BULLETS; 


    /* Fire bullet from alien ship. */
    int x2 = game.enemy_mat_position_x;
    int y2 = game.enemy_mat_position_y;

    int col = -1; 
    int row = 0;
    while (col == -1) {
        int c = get_time() % game.num_enemy_cols;
        int r = -1;

        for (int i = 0; i < game.num_enemy_rows; i++) {
            if (game.enemy_mat[c][i]) {
                r = i;
            }
        }

        if (r >= 0) {
            col = c;
            row = r;
        }
    }

    /* Update spacing. */
    y2 += (row + 1) * ALIEN_SIZE + row * ENEMY_SPACING;
    x2 += (col + 1) * ALIEN_SIZE + col * ENEMY_SPACING;

    /* If we're overwriting a bullet (i.e., hit MAX_BULLETS),
     * clear it first.
     */
    if (game.en_bullet_queue[game.en_bullet_counter].y != -1) {
        draw_sprite(&alien_bullet[0][0], game.en_bullet_queue[game.en_bullet_counter].x,
            game.en_bullet_queue[game.en_bullet_counter].y, BULLET_WIDTH,
            BULLET_HEIGHT, BLACK);
    }

    game.en_bullet_queue[game.en_bullet_counter].x = x2;
    game.en_bullet_queue[game.en_bullet_counter].y = y2;
    game.en_bullet_counter = (game.en_bullet_counter + 1) % ENEMY_BULLETS;

    shooting_sound();
}

void reset_game(uint8_t color) {
    draw_box(0, 0, VID_WIDTH, game.info_bar_height, color);
    if (color == RED) {
        // Presume user is dead
        death_sound();
    }
    sleep(RESET_TIME);
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
                reset_game(WHITE);
            }
        }

        if ((current_time - last_enemy_update) > ENEMY_UPDATE_PERIOD) {
            update_enemies();
            last_enemy_update = get_time();
        }
        if ((current_time - last_bullet_update) > BULLET_UPDATE_PERIOD) {
            update_bullets(-BULLET_SPEED, ALIEN_BULLET_SPEED);
            last_bullet_update = get_time();
        }

        if (game.game_over != GO_PLAY) {
            if (game.game_over == GO_WON) {
                reset_game(GREEN);
            }
            else if (game.game_over == GO_LOST) {
                reset_game(RED);
            }
            game.game_over = GO_PLAY;
        }
    }
}
