#include "video.h"

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
#define VIDEO_BUFFER ((void *) 0xB8000)


/* TODO:  You can create static variables here to hold video display state,
 *        such as the current foreground and background color, a cursor
 *        position, or any other details you might want to keep track of!
 */


void init_video(void) {
    clear_screen();
    // write_char('H', BLUE, GREEN, 0, 0);
    write_str("Hello World", BLUE, GREEN, 0, 0);

}


void clear_screen(void) {
    for (int i = 0; i < VID_HEIGHT; i++) {
        for (int j = 0; j < VID_WIDTH; j++) {
            write_char('\0', BLACK, BLACK, i, j);
        }
    }
}


void write_char(char ch, int fg, int bg, int x, int y) {
    int loc = BYTES_PER_ELT*(VID_WIDTH*x+y);
    *((char*)VIDEO_BUFFER+loc)   = ch;
    *((char*)VIDEO_BUFFER+loc+1) = fg << 4 | bg;    
}

void write_str(const char* str, int fg, int bg, int start_x, int start_y) {
    int i = 0;
    while (1) {
        char ch = str[i];
        if (ch == '\0') {
            break;
        }

        write_char(ch, fg, bg, start_x, start_y+i);
        i++;
    }
}