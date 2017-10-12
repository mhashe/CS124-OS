#include "ports.h"

#include "keyboard.h"
#include "interrupts.h"

/* This is the IO port of the PS/2 controller, where the keyboard's scan
 * codes are made available.  Scan codes can be read as follows:
 *
 *     unsigned char scan_code = inb(KEYBOARD_PORT);
 *
 * Most keys generate a scan-code when they are pressed, and a second scan-
 * code when the same key is released.  For such keys, the only difference
 * between the "pressed" and "released" scan-codes is that the top bit is
 * cleared in the "pressed" scan-code, and it is set in the "released" scan-
 * code.
 *
 * A few keys generate two scan-codes when they are pressed, and then two
 * more scan-codes when they are released.  For example, the arrow keys (the
 * ones that aren't part of the numeric keypad) will usually generate two
 * scan-codes for press or release.  In these cases, the keyboard controller
 * fires two interrupts, so you don't have to do anything special - the
 * interrupt handler will receive each byte in a separate invocation of the
 * handler.
 *
 * See http://wiki.osdev.org/PS/2_Keyboard for details.
 */
#define KEYBOARD_PORT 0x60
#define KEYBOARD_CMD_PORT 0x64 // use for LED functions

#define KEYCODE_QUEUE_SIZE 128

#include "keyboard.h"
#include "handlers.h"

// this works for a ps/2 US querty keyboard only!

/* TODO:  You can create static variables here to hold keyboard state.
 *        Note that if you create some kind of circular queue (a very good
 *        idea, you should declare it "volatile" so that the compiler knows
 *        that it can be changed by exceptional control flow.
 *
 *        Also, don't forget that interrupts can interrupt *any* code,
 *        including code that fetches key data!  If you are manipulating a
 *        shared data structure that is also manipulated from an interrupt
 *        handler, you might want to disable interrupts while you access it,
 *        so that nothing gets mangled...
 */

static volatile char key_queue[KEYCODE_QUEUE_SIZE];
static uint32_t queue_read_index;
static uint32_t queue_write_index;


void init_keyboard(void) {
    // initialize state required by the keyboard handler
    queue_read_index = 0;
    queue_write_index = 0;

    // http://wiki.osdev.org/%228042%22_PS/2_Controller
    outb(KEYBOARD_CMD_PORT, 0xED);
    while (inb(KEYBOARD_CMD_PORT) & 0b10) {
    }
    outb(KEYBOARD_PORT, 0b111);

    // install your keyboard interrupt handler
    install_interrupt_handler(KEYBOARD_INTERRUPT, irq1_handler);

}

void key_handler(void) {
    char keycode = inb(KEYBOARD_PORT); // puts keycode in rdi register

    disable_interrupts();

    key_queue[queue_write_index] = keycode;
    queue_write_index++;

    enable_interrupts();
}

// reads char from queue. 
char key_queue_pop(void) {
    char keycode;

    disable_interrupts();

    if (queue_read_index == queue_write_index) {
        enable_interrupts();
        return KEY_QUEUE_EMPTY;
    }

    keycode = key_queue[queue_read_index];

    queue_read_index++;

    enable_interrupts();

    return keycode;
}
