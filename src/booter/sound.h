#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>


/*=============================================================================
 * Generates a tone of frequency freq.
 *
 * Inputs:
 *     freq: frequency to be sounded in Hz. 
 */
void sound(uint32_t freq);

/*=============================================================================
 * Turn off sound.
 */
void no_sound();


void shooting_sound();

#endif /* SOUND_H */

