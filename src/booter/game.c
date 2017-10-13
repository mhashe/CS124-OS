#include "interrupts.h"
#include "timer.h"
#include "keyboard.h"
#include "video.h"

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


// we have 80x25 canvas
typedef struct Space_Invaders {
    // info presented to user
    uint32_t score; // number of enemies killed
    uint8_t lives_remaining;

    // position and direction of enemies
    // top, left corner off enemy matrix
    uint8_t enemy_position_x;
    uint8_t enemy_position_y;
    uint16_t num_enemies_left; // (16bit in case resolution were to increase)

    // user position
    uint8_t user_position_x;
    uint8_t user_position_y; // always the lowest row (not dynamic)

    // matrix of booleans stating whether enemy
    uint16_t offset_15_0;      // offset bits 0..15
    uint16_t selector;         // a code segment selector in GDT or LDT
    uint8_t zero;              // unused, set to 0
    uint8_t type_attr;         // descriptor type and attributes
    uint16_t offset_31_16;     // offset bits 16..31
} Space_Invaders;

static Space_Invaders game_state;

