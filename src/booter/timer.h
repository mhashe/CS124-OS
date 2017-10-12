#ifndef TIMER_H
#define TIMER_H


/*=============================================================================
 * Handles timer interrupts by incrementing the clock (in terms of ms).
 *
 * Deliberately not exposed to remainder of program.
 */
/* void timer_handler(void); */


/*=============================================================================
 * Sets up timer channel 0, and bind timer_handler to timer interrupts.
 */
void init_timer(void);


/*=============================================================================
 * Sleeps for a given number of seconds.
 *
 * Inputs:
 *      sec: decimal number of seconds.
 */
void sleep(float sec);


#endif /* TIMER_H */

