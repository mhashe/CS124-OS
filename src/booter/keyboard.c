#include "ports.h"
#include "keyboard.h"
#include "interrupts.h"
#include "handlers.h"

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
 *
 * This works for PS/2 US QUERTY keyboard only!
 */


static volatile char key_queue[KEYCODE_QUEUE_SIZE];
static uint32_t queue_read_index;
static uint32_t queue_write_index;


void init_keyboard(void) {
    // Initialize state required by the keyboard handler.
    queue_read_index = 0;
    queue_write_index = 0;

    // http://wiki.osdev.org/%228042%22_PS/2_Controller
    // outb(KEYBOARD_CMD_PORT, 0xED);
    // while (inb(KEYBOARD_CMD_PORT) & 0b10) {
    // }
    // outb(KEYBOARD_PORT, 0b111);

    // Install keyboard interrupt handler.
    install_interrupt_handler(KEYBOARD_INTERRUPT, irq1_handler);

}

/*=============================================================================
 * Handles keyboard interrupts by adding scan code to key code queue.
 *
 * Deliberately not exposed to remainder of program.
 */
void key_handler(void) {
    char keycode = inb(KEYBOARD_PORT);          // gets keycode

    key_queue[queue_write_index] = keycode;
    queue_write_index++;
    queue_write_index %= KEYCODE_QUEUE_SIZE;    // wrap index around

}

// reads char from queue. 
char key_queue_pop(void) {
    char keycode;

    disable_interrupts();                       // make interrupt safe

    if (queue_read_index == queue_write_index) {
        enable_interrupts();
        return KEY_QUEUE_EMPTY;
    }

    keycode = key_queue[queue_read_index];

    queue_read_index++;
    queue_read_index %= KEYCODE_QUEUE_SIZE;     // wrap index around

    enable_interrupts();                        // restore system state

    return keycode;
}
