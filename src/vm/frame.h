/*! \file frame.h
 *
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include <stdint.h>

#include "filesys/file.h"
#include "threads/loader.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

#define FRAME_NONE (uint32_t) -1

struct frame_table_entry {
    void* page;     /* pointer to page currently occupying entry */
    bool acc;       /* Bit indicating the page has been accessed. */
    bool dirty;     /* Bit indicating the page has been modified. */
    bool user;      /* Set if allocated from user pool. */
    bool valid;     /* If frame is a valid place in memory. */
};

struct frame_table_entry** frame_table;

void frame_init(size_t user_page_limit);
uint32_t get_frame(bool user);
void free_frame(uint32_t frame_number);
int frame_read(struct file *f, void* buffer, unsigned size, unsigned offset);
int frame_write(struct file *f, void* buffer, unsigned size, unsigned offset);

/*! Returns frame number at which kernel virtual address VADDR
    is mapped. */
static inline uint32_t vtof(const void *vaddr) {
    ASSERT(is_kernel_vaddr(vaddr));

    return ((uintptr_t) vaddr - (uintptr_t) PHYS_BASE) >> PGBITS;
}

/*! Returns kernel virtual address at which frame number refers to. */
static inline void * ftov(uint32_t frame_number) {
    ASSERT(frame_number < init_ram_pages);

    return (void *) ((frame_number << PGBITS) + PHYS_BASE);
}


#endif /* vm/frame.h */

