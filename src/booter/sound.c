#include "ports.h"
#include "timer.h"
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


/* Frequency of the PIT's input clock. */
#define PIT_FREQ 1193182


/* Ports for the Programmable Interval Timer (PIT). */
#define PIT_CHAN0_DATA 0x40
#define PIT_CHAN1_DATA 0x41
#define PIT_CHAN2_DATA 0x42
#define PIT_MODE_CMD   0x43


void sound(uint32_t freq) {
    uint32_t div = PIT_FREQ / freq;
    outb(PIT_CHAN2_DATA, (uint8_t) div);
    outb(PIT_CHAN2_DATA, (uint8_t) (div >> 8));

    outb(0x61, 0b11);
}


void no_sound() {
    outb(0x61, 0b00);
}


void shooting_sound() {
    sound(900);
    sleep(.05);
    no_sound();
}

void boot_sound() {
    sound(100);
    sleep(1.5);
    no_sound();
}

void death_sound() {
    sound(500);
    sleep(.2);
    sound(300);
    sleep(.2);
    sound(150);
    sleep(.2);
    no_sound();
}