#include "interrupts.h"
#include "timer.h"
#include "keyboard.h"
#include "sprites.h"
#include "video.h"
#include "game.h"
#include "sound.h"
#include "keyboard.h"


#include <stdint.h>


/* Global game state. */
static Space_Invaders game;


/* This is the entry-point for the game! */
void c_start(void) {
    /* Initialize Interrupt Descriptor Table. */
    init_interrupts();

    /* Configure Programmable Interval Timer. */
    init_timer();

    /* Install keyboard handler. */
    init_keyboard();

    /* Clear screen, prepare for drawing. */
    init_video();

    /* Booting complete; re-enable interrupts. */
    enable_interrupts();

    /* Set game parameters (see game.h). */
    init_game_state();

    /* Draw initial game layout. */
    draw_game_start();

    /* Play "booting" sound. */
    boot_sound();

    /* Loop game mode until player exits. */
    game_loop();
}


void init_game_state(void) {
    /* PLAY as opposed to game over states (WON, LOST). */
    game.game_state = GAME_PLAY;

    /* Define progress_bar shape (at top of screen). */
    game.progress_bar_height = VID_HEIGHT * PROGRESS_BAR_HEIGHT;

    /* Progress bar needs to contain numbers of enemies killed/left. */
    if (game.progress_bar_height < (FONT_HEIGHT + FONT_SPACING)) {
        game.progress_bar_height = FONT_HEIGHT + FONT_SPACING;
    }
    
    /* Player starts off towards bottom, horizontal center of screen. */
    game.player_bar_height = SHIP_SIZE;
    game.player_position_x = (VID_WIDTH - SHIP_SIZE) / 2;

    /* This 8 is truly a magic number. It seems in the emulator, after you fire
     * a bullet, the bottom 7 rows of the memory buffer stop updating, even
     * though new values are written to the buffer correctly. This
     * behavior is not present in running outside of the emulator (actually
     * booting into it).
     */
    game.player_position_y = VID_HEIGHT - game.player_bar_height - 8;

    /* Enemies claim all space vertically in-between info and player bars. */
    game.enemy_mat_height = ((VID_HEIGHT - game.progress_bar_height 
        - game.player_bar_height) * ENEMY_MAT_HEIGHT);
    game.enemy_mat_width = VID_WIDTH * ENEMY_MAT_WIDTH;
    
    /* Enemies start off in horizontal center, top of screen. */
    game.enemy_mat_position_x = ((VID_WIDTH - game.enemy_mat_width) / 2);
    game.enemy_mat_position_y = game.progress_bar_height; /* Start at top. */
    game.enemy_direction = NO_DIR; /* Direction of alien movement. */

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

    /* No bullets drawn as yet. -1 is used to indicate when a bullet pair
     * is empty.
     */
    for (int i = 0; i < MAX_BULLETS; i++) {
        game.bullet_queue[i].x = -1;
        game.bullet_queue[i].y = -1;
    };

    /* Count of how many bullets are currently on-screen; used to limit the 
     * player to a reasonable number of bullets at any given time.
     */
    game.bullet_counter = 0;

    /* Similarly, the aliens have not fired any bullets yet. They will begin
     * firing only as soon as the player does.
     */
    for (int i = 0; i < ENEMY_BULLETS; i++) {
        game.en_bullet_queue[i].x = -1;
        game.en_bullet_queue[i].y = -1;
    }

    /* Naturally, this state is also captured in a counter. */
    game.en_bullet_counter = 0;
}



void draw_game_start(void) {
    /* Draw progress bar. */
    update_game_progress(LIGHT_GREEN);

    /* Draw player in player bar. */
    draw_sprite(&ship[0][0], game.player_position_x, game.player_position_y, 
        SHIP_SIZE, SHIP_SIZE, YELLOW);

    /* Draw enemies. */
    update_enemies();

    /* Enemies will start moving right. */
    game.enemy_direction = RIGHT_DIR;
}


void game_loop(void) {
    unsigned char keycode; /* Dummy for keyboard input. */
    char empty = KEY_QUEUE_EMPTY;

    /* Update different aspects of the game at different rates. */
    uint32_t last_enemy_update = get_time();
    uint32_t last_bullet_update = get_time();
    
    /* Loop until player shuts down program. */
    while (1) {
        uint32_t current_time = get_time();

        /* Handle any relevant keyboard input. */
        keycode = key_queue_pop();
        if (keycode != KEY_QUEUE_EMPTY) {
            if (keycode == LEFT_ARROW) {
                move_player(LEFT_DIR);
            } else if (keycode == RIGHT_ARROW) {
                move_player(RIGHT_DIR);
            } else if (keycode == SPACEBAR) {
                fire_bullet();
            } else if (keycode == KEY_R) {
                /* Reset game. */
                reset_game(WHITE);
            } else if (keycode == KEY_P) {
                /* Pause or un-pause game. */
                pause_game(LIGHT_GRAY);
            }
        }

        /* If game is paused, don't progress game, just continue. */
        if (game.game_state == GAME_PAUSE) {
            continue;
        }

        /* Update positions and redraw enemies/aliens in game. */
        if ((current_time - last_enemy_update) > ENEMY_UPDATE_PERIOD) {
            /* Time step to advance aliens. */
            update_enemies();

            /* Keep track of current time so we know when to update again. */
            last_enemy_update = get_time();
        }

        /* Update positions and redraw bullets/missiles in game. */
        if ((current_time - last_bullet_update) > BULLET_UPDATE_PERIOD) {
            /* Time step to advance bullets. */
            update_bullets(-BULLET_SPEED, ALIEN_BULLET_SPEED);

            /* Keep track of current time so we know when to update again. */
            last_bullet_update = get_time();
        }

        /* A win / loss condition has been met. */
        if (game.game_state != GAME_PLAY) {
            if (game.game_state == GAME_WON) {
                /* Game won! Reset to initial board and play again. */
                reset_game(GREEN);
            }
            else if (game.game_state == GAME_LOST) {
                /* Game lost... Reset to initial board and play again. */
                reset_game(RED);
            }

            /* Mark new game as in progress. */
            game.game_state = GAME_PLAY;
        }
    }
}


void update_enemies(void) {
    /* Clear enemies before updating. */
    draw_box(game.enemy_mat_position_x, game.enemy_mat_position_y, 
        game.enemy_mat_width, game.enemy_mat_height, BLACK);

    /* Redraw bullets, which were just erased. */
    update_bullets(0, 0);
    
    /* Move enemies. */
    if (game.enemy_direction == RIGHT_DIR) {
        game.enemy_mat_position_x += ENEMY_SPEED;

        /* If collision with right wall, then move left. */
        if ((game.enemy_mat_position_x + game.enemy_mat_width + ENEMY_SPEED) 
            > VID_WIDTH) {

            game.enemy_direction = LEFT_DIR;
            game.enemy_mat_position_y += ENEMY_DROP_SPEED;
        }
    } 
    
    else if (game.enemy_direction == LEFT_DIR) {
        game.enemy_mat_position_x -= ENEMY_SPEED;

        /* If collision with left wall, then move right. */
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
                draw_sprite(&alien[0][0], ex, ey, ALIEN_SIZE, ALIEN_SIZE, 
                    GREEN);
                

                /* Handle collision detection with player. */

                /* If enemy at player height... */
                if ((ey + ALIEN_SIZE) > (game.player_position_y + 8)) {

                    /* ... and on top of player... */
                    if (((ex + ALIEN_SIZE) > (game.player_position_x)) ||
                        (ex < (game.player_position_x + SHIP_SIZE))) {

                        /* ... then it's game over. */
                        game.game_state = GAME_LOST;
                    }
                }

                /* If enemy has moved off screen, remove from the game. */
                if ((ey + ALIEN_SIZE) >= VID_WIDTH) {
                    game.enemy_mat[c][r] = 0;
                    game.num_enemies_left--;
                }
            }

            /* Advance to next alien. */
            ey += ALIEN_SIZE + ENEMY_SPACING;
        }

        /* Advance to next column of aliens. */
        ex += ALIEN_SIZE + ENEMY_SPACING;
    }

    /* Update progress bar, in case any aliens were hit. */
    update_game_progress(LIGHT_GREEN);

    /* If the game is not lost and all enemies are defeated, player has won. */
    if ((game.num_enemies_left == 0) && (game.game_state == GAME_PLAY)) {
        game.game_state = GAME_WON;
    }
}


void move_player(int dx) {
    /* Clear player. */
    draw_sprite(&ship[0][0], game.player_position_x, game.player_position_y, 
        SHIP_SIZE, SHIP_SIZE, BLACK);
    
    /* Move player. */
    game.player_position_x += (PLAYER_SPEED * dx);

    /* Redraw player. */
    draw_sprite(&ship[0][0], game.player_position_x, game.player_position_y, 
        SHIP_SIZE, SHIP_SIZE, YELLOW);
}


void fire_bullet(void) {
    /* Determine player location; this is where the bullet starts. */
    int x = game.player_position_x + (SHIP_SIZE / 2) - 1;
    int y = game.player_position_y;

    /* If we're overwriting a bullet (i.e., hit MAX_BULLETS),
     * clear it first.
     */
    if (game.bullet_queue[game.bullet_counter].y != -1) {

        draw_sprite(&bullet[0][0], game.bullet_queue[game.bullet_counter].x,
            game.bullet_queue[game.bullet_counter].y, BULLET_WIDTH,
            BULLET_HEIGHT, BLACK);
    }

    /* Save bullet in bullet queue (circular queue). */
    game.bullet_queue[game.bullet_counter].x = x;
    game.bullet_queue[game.bullet_counter].y = y;
    game.bullet_counter = (game.bullet_counter + 1) % MAX_BULLETS; 


    /* For every bullet the player fires, fire a corresponding
     * bullet from alien ship. 
     */
    fire_alien_bullet();

    /* Played after a pair of bullets (player + enemy) is shot. */
    shooting_sound();
}


void update_bullets(int dy, int ady) {
    /* Loop over bullet queue. Note that many slots might
     * be empty. 
     */
    for (int i = 0; i < MAX_BULLETS; i++) {

        /* Only need to update live bullets. */
        if (game.bullet_queue[i].y != -1) {

            /* Erase old bullet. */
            draw_sprite(&bullet[0][0], game.bullet_queue[i].x,
                game.bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);

            /* Check if bullet is still in game. Magic number 3
             * intended as buffer to avoid infringing upon score bar.
             */
            if (game.bullet_queue[i].y <= game.progress_bar_height + 3) {

                /* -1 as an indicator for unused bullet slot. */
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

    /* Loop over bullet queue of alien_bullets. */
    for (int i = 0; i < ENEMY_BULLETS; i++) {

        /* Only need to update live bullets. */
        if (game.en_bullet_queue[i].y != -1) {

            /* Erase old bullet. */
            draw_sprite(&alien_bullet[0][0], game.en_bullet_queue[i].x,
                game.en_bullet_queue[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);

            /* Check for collision between player and bullet. */
            if    ((game.en_bullet_queue[i].x >= game.player_position_x)
                && (game.en_bullet_queue[i].x <= game.player_position_x + SHIP_SIZE) 
                && (game.en_bullet_queue[i].y >= game.player_position_y)
                && (game.en_bullet_queue[i].y <= game.player_position_y + SHIP_SIZE)) {

                /* Collision! */
                game.game_state = GAME_LOST;
                return;
            }

            /* Check if bullet is still in game. Magic number 3
             * intended as buffer to avoid infringing upon "dead-zone"
             * at bottom of screen (see init_game_start).
             */
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


void fire_alien_bullet(void) {
    /* Use relative position from top left of enemy matrix. */
    int x = game.enemy_mat_position_x;
    int y = game.enemy_mat_position_y;

    /* Choose a random column with enemies left in it. */
    int col = -1; 
    int row = 0;

    while (col == -1) {
        /* Psuedo-psuedo RNG. */
        int c = get_time() % game.num_enemy_cols;
        int r = -1;

        /* Check if any enemies left in this column. */
        for (int i = 0; i < game.num_enemy_rows; i++) {
            if (game.enemy_mat[c][i]) {
                r = i;
            }
        }

        /* If yes, launch a bullet from that enemy. */
        if (r >= 0) {
            col = c;
            row = r;
        }
    }

    /* Update spacing. */
    x += (col + 1) * ALIEN_SIZE + col * ENEMY_SPACING;
    y += (row + 1) * ALIEN_SIZE + row * ENEMY_SPACING;

    /* If we're overwriting a bullet (i.e., hit MAX_BULLETS),
     * clear it first.
     */
    if (game.en_bullet_queue[game.en_bullet_counter].y != -1) {

        draw_sprite(&alien_bullet[0][0], game.en_bullet_queue[game.en_bullet_counter].x,
            game.en_bullet_queue[game.en_bullet_counter].y, BULLET_WIDTH,
            BULLET_HEIGHT, BLACK);
    }

    /* Read new bullet into queue. */
    game.en_bullet_queue[game.en_bullet_counter].x = x;
    game.en_bullet_queue[game.en_bullet_counter].y = y;
    game.en_bullet_counter = (game.en_bullet_counter + 1) % ENEMY_BULLETS;
}


void reset_game(uint8_t color) {
    /* Overwrite progress with a certain color, i.e.
     * to celebrate win condition.
     */
    draw_box(0, 0, VID_WIDTH, game.progress_bar_height, color);

    /* Mock the player for his poor performance. */
    if (color == RED) {
        death_sound();
    }

    /* Break for a fixed amount of time. */
    sleep(RESET_TIME);

    /* Clear screen, and then re-initialize. */
    clear_screen();
    init_game_state();
    draw_game_start();
}


void pause_game(uint8_t color) {
    /* If game is paused, un-pause it. Else, pause it. */
    if (game.game_state == GAME_PAUSE) {
        game.game_state = GAME_PLAY;
        update_game_progress(BLUE);
    } else {
        game.game_state = GAME_PAUSE;
        update_game_progress(color);
    }
}



void update_game_progress(uint8_t color) {
    /* Total number of enemies at start and total enemies killed. */
    uint16_t num_enemies = game.num_enemy_rows * game.num_enemy_cols;
    uint16_t num_enemies_killed = num_enemies - game.num_enemies_left;

    /* Width, in pixels, of progress bar that is filled. */
    int progress_width = (VID_WIDTH * num_enemies_killed) / num_enemies;

    /* Mark progress bar. */
    draw_box(0, 0, progress_width, game.progress_bar_height, color);

    /* Draw background of progress bar, which is blue. */
    draw_box(progress_width, 0, VID_WIDTH - progress_width, 
        game.progress_bar_height, BLUE);

    /* Draw number of enemies killed and left here. */
    draw_two_digit_number(game.num_enemies_left, FONT_SPACING, 
        FONT_SPACING, RED, 0);
    draw_two_digit_number(num_enemies_killed, VID_WIDTH - FONT_SPACING, 
        FONT_SPACING, GREEN, 1);
}

