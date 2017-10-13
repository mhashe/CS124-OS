#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_PORT 0x60
#define KEYBOARD_CMD_PORT 0x64      // use for LED functions

#define KEYCODE_QUEUE_SIZE 4

#define KEY_QUEUE_EMPTY 0xFB        // an unused key code

/*=============================================================================
 * Sets up keyboard queue indexes and bind irq1_handler to timer interrupts.
 */
void init_keyboard(void);

/*=============================================================================
 * Returns the oldest key press scan code from the queue. Turns off interrupts
 * to prevent an interrupt from messing up queue state. 
 *
 * Output:
 *     keycode: key press scan code
 */
char key_queue_pop(void);

#endif /* KEYBOARD_H */

