#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*=============================================================================
 * Sets up timer channel 0, and bind timer_handler to irq0_handler interrupts.
 */
void init_timer(void);


/*=============================================================================
 * Returns time.
 */
uint32_t get_time(void);

/*=============================================================================
 * Sleeps for a given number of seconds.
 *
 * Inputs:
 *      sec: decimal number of seconds.
 */
void sleep(float sec);


#endif /* TIMER_H */

