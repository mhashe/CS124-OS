#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_QUEUE_EMPTY 0xFB // an unused key code

void init_keyboard(void);

char key_queue_pop(void);

#endif /* KEYBOARD_H */

