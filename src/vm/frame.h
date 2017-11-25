/*! \file frame.h
 *
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include <stdint.h>
#include "threads/vaddr.h"

#define MEM_SIZE (1 << 31)
#define NUM_FRAME_ENTRIES ((uint32_t) MEM_SIZE / PGSIZE)

struct frame_table_entry {
    void* page;     /* pointer to page currently occupying entry */
};


struct frame_table_entry** frame_table;

void init_frame_table(void);
int get_frame(void);


#endif /* vm/frame.h */

