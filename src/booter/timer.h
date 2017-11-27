#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>


/* Frequency of the PIT's input clock. */
#define PIT_FREQ 1193182


/* Ports for the Programmable Interval Timer (PIT). */
#define PIT_CHAN0_DATA 0x40
#define PIT_CHAN1_DATA 0x41
#define PIT_CHAN2_DATA 0x42
#define PIT_MODE_CMD   0x43


/* Sets up timer channel 0, and bind timer_handler to irq0_handler interrupts. */
void init_timer(void);

/* Handles timer interrupts. */
void timer_handler(void);

/* Returns time. */
uint32_t get_time(void);


/* Sleeps for a given number of seconds.
 *
 * Inputs:
 *      sec: decimal number of seconds.
 */
void sleep(float sec);


#endif /* TIMER_H */

