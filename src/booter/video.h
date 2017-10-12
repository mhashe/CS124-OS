#ifndef VIDEO_H
#define VIDEO_H

#define VID_WIDTH     80
#define VID_HEIGHT    25
#define VID_BUFF_SIZE  VID_WIDTH*VID_HEIGHT
#define BYTES_PER_ELT  2

/* Available colors from the 16-color palette used for EGA and VGA, and
 * also for text-mode VGA output.
 */
#define BLACK          0
#define BLUE           1
#define GREEN          2
#define CYAN           3
#define RED            4
#define MAGENTA        5
#define BROWN          6
#define LIGHT_GRAY     7
#define DARK_GRAY      8
#define LIGHT_BLUE     9
#define LIGHT_GREEN   10
#define LIGHT_CYAN    11
#define LIGHT_RED     12
#define LIGHT_MAGENTA 13
#define YELLOW        14
#define WHITE         15


void init_video(void);

/*
 * Clears the screen by writing the null character everywhere.
 */
void clear_screen(void);

/*
 * Writes a character to the screen.
 *
 * Input:
 *     ch: character to be output
 *     fg: foreground color, must be defined in video.h
 *     bg: background color, must be defined in video.h
 *     x : x location to be placed
 *     y : y location to be placed
 */
void write_char(char ch, int fg, int bg, int x, int y);


#endif /* VIDEO_H */
