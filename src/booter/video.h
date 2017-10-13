#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

#define VID_WIDTH        320
#define VID_HEIGHT       200
#define BYTES_PER_PIXEL  1
#define VID_BUFF_SIZE    VID_WIDTH*VID_HEIGHT*BYTES_PER_PIXEL

#define LOC(x, y)       y*VID_WIDTH + x


// Memory locations for the different video buffers.
#define TEXT_BUFFER ((char *) 0xB8000)
#define CGA_EVEN_BUFFER ((char *) 0xB8000)
#define CGA_ODD_BUFFER  ((char *) 0xBA000)
#define CGA_BUFFER ((char *) 0xB8000)
#define VGA_BUFFER ((char *) 0xA0000)

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
 * Draw a pixel on the screen at (x,y).
 *
 * Inputs:
 *     x:     x location of pixel
 *     y:     y location of pixel
 *     color: color to be displayed
 */
void draw_pixel(int x, int y, uint8_t color);

/*
 * Draw a box on the screen.
 *
 * Inputs:
 *     x:      x location of pixel
 *     y:      y location of pixel
 *     width:  width of box
 *     height: height of box
 *     color:  color to be displayed
 */
void draw_box(int x, int y, int width, int height, uint8_t color);

/*
 * Draw an alien on the screen at (x,y).
 *
 * Inputs:
 *     x:      x location of alien
 *     y:      y location of alien
 *     color:  color to be displayed
 */
void draw_alien(int x, int y, uint8_t color);

/*
 * Draw an ship on the screen at (x,y).
 *
 * Inputs:
 *     x:      x location of alien
 *     y:      y location of alien
 *     color:  color to be displayed
 */
void draw_ship(int x, int y, uint8_t color);

/*
 * Draw a bullet on the screen at (x,y).
 *
 * Inputs:
 *     x:      x location of alien
 *     y:      y location of alien
 *     color:  color to be displayed
 */
void draw_bullet(int x, int y, uint8_t color);

/*
 * Clears the screen by writing 0 everywhere in video memory.
 */
void clear_screen();

#endif /* VIDEO_H */
