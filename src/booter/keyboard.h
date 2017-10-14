#ifndef KEYBOARD_H
#define KEYBOARD_H


#define KEYBOARD_PORT      0x60
#define KEYBOARD_CMD_PORT  0x64      /* Use for LED functions. */


#define KEYCODE_QUEUE_SIZE 128


/* Define key presses */
#define KEY_QUEUE_EMPTY   0xFB  /* An unused key code. */
#define SPACEBAR          0x39
#define LEFT_ARROW        0x4B
#define RIGHT_ARROW       0x4D
#define KEY_R             0x13
#define KEY_P             0x19


/* Sets up keyboard queue indexes and bind irq1_handler to timer interrupts.
 */
void init_keyboard(void);


/* Returns the oldest key press scan code from the queue. Turns off interrupts
 * to prevent an interrupt from messing up queue state. 
 *
 * Output:
 *     keycode -  key press scan code
 */
char key_queue_pop(void);


#endif /* KEYBOARD_H */

