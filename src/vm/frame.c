#include "vm/frame.h"

#include <debug.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#include "threads/malloc.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"

/* TODO : verify init_ram_pages is the right number. */
struct frame_table_entry** frame_table;

void init_frame_table(void) {
    printf("At least we're initting!\n");

    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), 5);

    for (uint32_t i = 0; i < 5; i++) {
        frame_table[i] = (struct frame_table_entry*) 
                        calloc(sizeof(struct frame_table_entry), 1);
    }
}

/* Select an empty frame if available, otherwise choose a page
   to evict and evict it. */
int evict(void) {
    int vic = 0;

    /* See if empty frame exists. */
    for (uint32_t i = 0; i < 5; i++) {
        if (!frame_table[i]->page) {
            vic = i;
            break;
        }
    }

    /* If not, evict a page. */
    /* TODO */

    return vic;
}

/* Gets the first free frame in the frame table. Returns an index. 
   page is the virtual memory pointer to a page that is occupying this frame. */
int get_frame(bool user) {
    // uint32_t i = 0;
    // for (i = 0; i < 5; i++) {
    //     if (frame_table[i] == NULL) {
    //         break;
    //     }
    // }
    void *frame;

    if (user) {
        frame = palloc_get_page(PAL_ZERO | PAL_USER);
    }
    else {
        frame = palloc_get_page(PAL_ZERO);
    }

    if (frame == NULL) {
        // TODO: evict then palloc again
        PANIC("frame table full\n");
        return -1;
    } else {
        int frame_number = evict();

        frame_table[frame_number]->page = frame;

        return frame_number;
    }
}

