#ifndef TIMER_H
#define TIMER_H


/*=============================================================================
 * Sets up timer channel 0, and bind timer_handler to timer interrupts.
 */
void init_timer(void);

int get_time(void);

/*=============================================================================
 * Sleeps for a given number of seconds.
 *
 * Inputs:
 *      sec: decimal number of seconds.
 */
void sleep(float sec);


#endif /* TIMER_H */

