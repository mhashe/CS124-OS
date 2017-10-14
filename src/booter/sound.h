#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>


/* Generates a tone of frequency freq.
 *
 * Inputs:
 *     freq - frequency to be sounded in Hz. 
 */
void sound(uint32_t freq);


/* Turn off sound.
 */
void no_sound();


/* Sound for shooting a bullet. Sleeps for 0.05 seconds.
 */
void shooting_sound();


/* Sound for booting the game up. Sleeps for 1 second.
 */
void boot_sound();


/* Sound for dying. Sleeps for 0.6 seconds.
 */
void death_sound();


#endif /* SOUND_H */

