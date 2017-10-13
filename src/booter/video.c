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


void init_video(void) {
    clear_screen();
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


void clear_screen() {
    uint32_t blank = 0;

    // Write 32 bits at a time (4 pixels) so we access memory less.
    for (int i = 0; i < VID_BUFF_SIZE/4; i++) {
        *((uint32_t*)VGA_BUFFER+i) = blank;
    }
}

