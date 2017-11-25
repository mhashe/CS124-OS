#include "vm/frame.h"

#include <debug.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#include "threads/malloc.h"

void init_frame_table(void) {
    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), NUM_FRAME_ENTRIES);

    for (int i = 0; i < NUM_FRAME_ENTRIES; i++) {
        frame_table[i] = NULL; 
    }
}

/* Gets the first free frame in the frame table. Returns an index. 
   page is the virtual memory pointer to a page that is occupying this frame. */
int get_frame(void *page) {
    uint32_t i = 0;
    for (i = 0; i < NUM_FRAME_ENTRIES; i++) {
        if (frame_table[i] == NULL) {
            break;
        }
    }

    if (i == NUM_FRAME_ENTRIES) {
        // TODO: evict
        PANIC("frame table full\n");
        return -1;
    } else {
        frame_table[i] = (struct frame_table_entry*) 
                        calloc(sizeof(struct frame_table_entry), 1);
        frame_table[i]->page = page;

        return i;
    }
}

//  page is the virtual memory pointer to a page that is occupying this frame. 
//    i is the index for this frame. 
// void set_frame_page(void *page, int i) {
//     ASSERT(frame_table[i] != NULL);
// }



