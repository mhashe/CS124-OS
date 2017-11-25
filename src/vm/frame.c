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
        frame_table[i] = NULL;
    }
}

/* Select an empty frame if available, otherwise choose a page
   to evict and evict it. */
int evict(void) {
    int vic = 0;

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
int get_frame(bool user) {
    // uint32_t i = 0;
    // for (i = 0; i < init_ram_pages; i++) {
    //     if (frame_table[i] == NULL) {
    //         break;
    //     }
    // }
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
        int frame_number = evict();

        // printf("What's the issue?\n");
        frame_table[frame_number]->page = frame;
        // printf("Seriously?\n");

        return frame_number;
    }
}

