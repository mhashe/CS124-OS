#include "timer.h"
#include "ports.h"
#include "interrupts.h"
#include "handlers.h"
#include "sound.h"

#include <stdint.h>


/*============================================================================
 * PROGRAMMABLE INTERVAL TIMER
 *
 * The Programmable Interrupt Timer receives a clock of 1193182 Hz.  It has
 * three frequency-dividers (called "channels", and numbered from 0 to 2),
 * which can divide the clock by any value from 1 to 65535.  This allows one
 * to generate timer ticks for various intervals (approximately).
 *
 * For example, to generate 8 KHz ticks, divide 1193182 / 8000 to get 149.15.
 * Of course, if you put in 149 then you will get an 8008 Hz output, and if
 * you put in 150 then you will get a 7954.5 Hz output.  So, it is clearly an
 * approximate mechanism.
 *
 * Channel 0 is hooked up to IRQ1 on the Programmable Interrupt Controller
 * (PIC), so we can generate timer interrupts on various intervals.
 *
 * Channel 1 is not widely used, and may not even be available on modern
 * hardware.  So don't use it.
 *
 * Channel 2 can be routed to the PC speaker (!!!), so you can generate
 * various tones if you configure PIT channel 2 properly.  Note that this
 * functionality is emulated badly by most CPU emulators, so don't expect
 * that this will necessarily work unless you can test on physical hardware.
 * (See http://wiki.osdev.org/PC_Speaker for more details.)
 *
 * See http://wiki.osdev.org/Programmable_Interval_Timer for more details
 * about the Programmable Interval Timer.
 */


/* Timer count. */
static volatile uint32_t time;


/* Handles timer interrupts by incrementing the clock (in terms of ms).
 */
void timer_handler(void) {
    /* Advance time ("clock tick"). */
    time++;
}


void init_timer(void) {

    /* Turn on timer channel 0 for generating interrupts. */
    outb(PIT_MODE_CMD, 0x36);               /* 00 11 011 0 */

    /* Tell channel 0 to trigger 100 times per second.  The value we load
     * here is a divider for the 1193182 Hz timer.  1193182 / 100 ~= 11932.
     * 11932 = 0x2e9c.
     */
    outb(PIT_CHAN0_DATA, 0x9c);
    outb(PIT_CHAN0_DATA, 0x2e);

    /* Turn on timer channel 2 for tone generation. */
    outb(PIT_MODE_CMD, 0xb6);               /* 10 11 011 0 */

    no_sound();                             /* Make sure no sound is playing. */

    /* Initialize timer to 0. */
    time = 0;

    /* Install timer interrupt handler. */
    install_interrupt_handler(TIMER_INTERRUPT, irq0_handler);
}


uint32_t get_time(void) {
    return time;
}


void sleep(float sec) {
    /* Sleep for sec seconds. */
    uint32_t low = time;
    
    while ((time - low) * 1. / 100.0 < sec) {
        /* Loop until sec seconds elapsed. */
    }
}

