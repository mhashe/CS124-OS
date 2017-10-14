#include "video.h"
#include "sprites.h"

#include <stdint.h>

/* This is the address of the VGA text-mode video buffer.  Note that this
 * buffer actually holds 8 pages of text, but only the first page (page 0)
 * will be displayed.
 *
 * Individual characters in text-mode VGA are represented as two adjacent
 * bytes:
 *     Byte 0 = the character value
 *     Byte 1 = the color of the character:  the high nibble is the background
 *              color, and the low nibble is the foreground color
 *
 * See http://wiki.osdev.org/Printing_to_Screen for more details.
 *
 * Also, if you decide to use a graphical video mode, the active video buffer
 * may reside at another address, and the data will definitely be in another
 * format.  It's a complicated topic.  If you are really intent on learning
 * more about this topic, go to http://wiki.osdev.org/Main_Page and look at
 * the VGA links in the "Video" section.
 */


/* An array of sprites representing numbers */
static uint8_t *numbers[10];


void init_video(void) {
    clear_screen();

    /* Set array of sprite numbers (must be set at runtime). */
    numbers[0] = &zero[0][0];
    numbers[1] = &one[0][0];
    numbers[2] = &two[0][0];
    numbers[3] = &three[0][0];
    numbers[4] = &four[0][0];
    numbers[5] = &five[0][0];
    numbers[6] = &six[0][0];
    numbers[7] = &seven[0][0];
    numbers[8] = &eight[0][0];
    numbers[9] = &nine[0][0];
}


void clear_screen() {
    uint32_t blank = 0;

    // Write 32 bits at a time (4 pixels) so we access memory less.
    for (int i = 0; i < VID_BUFF_SIZE/4; i++) {
        *((uint32_t*)VGA_BUFFER+i) = blank;
    }
}



void draw_pixel(int x, int y, uint8_t color) {
    *(VGA_BUFFER+LOC(x, y)) = color;
}


void draw_box(int x, int y, int width, int height, uint8_t color) {
    for (int j = x; j < (x + width); j++) {
        for (int i = y; i < (y + height); i++)
            draw_pixel(j, i, color);
    }
}


void draw_sprite(const uint8_t* sprite, int x, int y, int width, int height, 
                 uint8_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (sprite[i*width + j] == 1) {
                draw_pixel(x+j, y+i, color);
            }
        }
    }
}

void draw_two_digit_number(int number, int x, int y, uint8_t color, 
                int align_right) {
    int tens = (number / 10) % 10;
    int ones = number % 10;

    if (align_right) {
        draw_number(ones, x - FONT_WIDTH, y, color);
        draw_number(tens, x - (FONT_WIDTH * 2) - FONT_SPACING, y, color);
    } else {
        draw_number(tens, x, y, color);
        draw_number(ones, x + FONT_WIDTH + FONT_SPACING, y, color);
    }
}

void draw_number(int number, int x, int y, uint8_t color) {
    draw_sprite(numbers[number % 10], x, y, FONT_WIDTH, FONT_HEIGHT, color);
}
