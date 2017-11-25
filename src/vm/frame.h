/*! \file frame.h
 *
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include <stdint.h>
#include "threads/vaddr.h"

struct frame_table_entry {
    void* page;     /* pointer to page currently occupying entry */
};


struct frame_table_entry** frame_table;

void init_frame_table(void);
int get_frame(void* page);


#endif /* vm/frame.h */

