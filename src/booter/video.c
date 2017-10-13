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



/* TODO:  You can create static variables here to hold video display state,
 *        such as the current foreground and background color, a cursor
 *        position, or any other details you might want to keep track of!
 */



void init_video(void) {
    draw_pixel(319, 10, WHITE);
    draw_pixel(319, 15, WHITE);
    draw_pixel(319, 20, BLUE);
    draw_pixel(319, 199, WHITE);

    draw_alien(0, 0, LIGHT_GREEN);
    draw_bullet(5, 100, WHITE);
    draw_ship(0, 150, GREEN);

    // clear_screen();
    // draw_box(10, 10, 100, 200, WHITE);

}

void draw_pixel(int x, int y, uint8_t color) {
    *(VGA_BUFFER+LOC(x, y)) = color;
}

void draw_box(int x, int y, int width, int height, uint8_t color) {
    for (int i = x; i < (x + width); i++) {
        for (int j = y; j < (y + height); j++)
            draw_pixel(i, j, color);
    }
}

void draw_alien(int x, int y, uint8_t color) {
    for (int i = 0; i < ALIEN_SIZE; i++) {
        for (int j = 0; j < ALIEN_SIZE; j++) {
            if (alien[j][i] == 1) {
                draw_pixel(x+i, y+j, color);
            }
        }
    }
}

void draw_ship(int x, int y, uint8_t color) {
    for (int i = 0; i < SHIP_SIZE; i++) {
        for (int j = 0; j < SHIP_SIZE; j++) {
            if (ship[j][i] == 1) {
                draw_pixel(x+i, y+j, color);
            }
        }
    }
}

void draw_bullet(int x, int y, uint8_t color) {
    draw_box(x, y, BULLET_WIDTH, BULLET_HEIGHT, color);
}

void clear_screen() {
    uint32_t blank = 0;

    // Write 32 bits at a time (4 pixels) so we access memory less.
    for (int i = 0; i < VID_BUFF_SIZE/4; i++) {
        *((uint32_t*)VGA_BUFFER+i) = blank;
    }
}

