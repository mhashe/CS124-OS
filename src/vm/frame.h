/*! \file frame.h
 *
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include "threads/vaddr.h"

#define MEM_SIZE (1 << 31)
#define NUM_FRAME_ENTRIES (MEM_SIZE / PGSIZE)

struct frame_table_entry {
    void* page;     /* pointer to page currently occupying entry */
};


struct frame_table_entry* frame_table[NUM_FRAME_ENTRIES];


#endif /* vm/frame.h */

