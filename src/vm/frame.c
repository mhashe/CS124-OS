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

void init_frame_table(void) {
    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), init_ram_pages);

    for (uint32_t i = 0; i < init_ram_pages; i++) {
        frame_table[i] = (struct frame_table_entry*) 
                        calloc(sizeof(struct frame_table_entry), 1);
    }
}

/* Select an empty frame if available, otherwise choose a page
   to evict and evict it. */
uint32_t evict(void) {
    uint32_t vic = 0;

    /* See if empty frame exists. */
    for (uint32_t i = 0; i < init_ram_pages; i++) {
        if (frame_table[i]) {
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
uint32_t get_frame(bool user) {
    void *frame;

    // printf("At least we get here!\n");
    // printf("%d\n", init_ram_pages);
    
    if (user) {
        frame = palloc_get_page(PAL_ZERO | PAL_USER);
    }
    else {
        frame = palloc_get_page(PAL_ZERO);
    }

    // printf("And here!\n");

    if (frame == NULL) {
        // TODO: evict then palloc again
        // printf("Oopsy!\n");
        PANIC("frame table full\n");
        return -1;
    } else {
        uint32_t frame_number = evict();

        // printf("What's the issue?\n");
        frame_table[frame_number]->page = frame;
        // printf("Seriously?\n");

        return frame_number;
    }
}

