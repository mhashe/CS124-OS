#ifndef GAME_H
#define GAME_H


/* Declare the entry-points to the interrupt handler assembly-code fragments,
 * so that the C compiler will be happy.
 */


void *(irq0_handler)(void); /* Timer Handler. */
void *(irq1_handler)(void); /* Keyboard Handler. */


#endif /* GAME_H */

