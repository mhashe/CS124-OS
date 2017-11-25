#include "vm/frame.h"

#include <debug.h>
#include <stddef.h>
#include <stdint.h>

#include "threads/malloc.h"

void init_frame_table(void) {
    frame_table = (struct frame_table_entry**) 
                  malloc(sizeof(struct frame_table_entry*) * NUM_FRAME_ENTRIES);
}

/* Gets the first free frame in the frame table. Returns an index. */
int get_frame(void) {
    // TODO: evict
    uint32_t i = 0;
    for (i = 0; i < NUM_FRAME_ENTRIES; i++) {
        if (frame_table[i] == NULL) {
            break;
        }
    }
    if (i == NUM_FRAME_ENTRIES) {
        return -1;
    } else {
        return i;
    }
}
