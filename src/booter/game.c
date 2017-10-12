<<<<<<< HEAD
=======
#include "interrupts.h"
#include "timer.h"
>>>>>>> c128adbe68e183d63c5c81b2c8dd2ee43136ee45
#include "keyboard.h"

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

    // Current loops forever (to test keyboard press):
    init_keyboard();
}

