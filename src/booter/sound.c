#include "ports.h"
#include "timer.h"
#include "sound.h"

#include <stdint.h>


void sound(uint32_t freq) {
    uint32_t div = PIT_FREQ / freq;
    outb(PIT_CHAN2_DATA, (uint8_t) div);
    outb(PIT_CHAN2_DATA, (uint8_t) (div >> 8));

    outb(0x61, 0b11);
}


void no_sound() {
    outb(0x61, 0b00);
}


void shooting_sound() {
    sound(900);
    sleep(.05);
    no_sound();
}


void boot_sound() {
    sound(100);
    sleep(1);
    no_sound();
}


void death_sound() {
    sound(500);
    sleep(.2);
    sound(300);
    sleep(.2);
    sound(150);
    sleep(.2);
    no_sound();
}

