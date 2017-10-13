#include "video.h"

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



// http://webpages.charter.net/danrollins/techhelp/0089.HTM

/* TODO:  You can create static variables here to hold video display state,
 *        such as the current foreground and background color, a cursor
 *        position, or any other details you might want to keep track of!
 */



void init_video(void) {
    draw_pixel(10, 319, WHITE);
    draw_pixel(15, 319, WHITE);
    draw_pixel(20, 319, BLUE);
    draw_pixel(199, 319, WHITE);

    clear_screen();
}

void draw_pixel(int x, int y, uint8_t color) {
    *(VGA_BUFFER+LOC(x, y)) = color;
}

void clear_screen() {
    uint32_t blank = 0;

    // Write 32 bits at a time (4 pixels) so we access memory less.
    for (int i = 0; i < VID_BUFF_SIZE/4; i++) {
        *((uint32_t*)VGA_BUFFER+i) = blank;
    }
}

