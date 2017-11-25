#include "vm/frame.h"
#include <debug.h>

/* Gets the first free frame in the frame table. Returns an index. */
int get_frame() {
    // TODO: evict
    int i = 0;
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
