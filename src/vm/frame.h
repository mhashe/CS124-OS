/*! \file frame.h
 *
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <debug.h>
#include <stdint.h>
// #include <list.h>

#include "threads/vaddr.h"
#include "threads/loader.h"


#define FRAME_NONE (uint32_t) -1

struct frame_table_entry {
    void* page;     /* pointer to page currently occupying entry */
    bool acc;       /* Bit indicating the page has been accessed. */
    bool dirty;     /* Bit indicating the page has been modified. */
    bool user;      /* Set if allocated from user pool. */
    bool valid;     /* If frame is a valid place in memory. */

    // struct list *sup_entries; /* List of entries in sup_page table 
                                // referencing this frame. */
};

// struct id_to_sup_entries {
//     uint32_t id;
//     bool id_is_frame_or_swap;
//     struct list sup_entries;
// }

// struct swap_id_to_entries {
//     struct list *sup_entries;
// }

struct frame_table_entry** frame_table;

void init_frame_table(void);
uint32_t get_frame(bool user);
void free_frame(uint32_t frame_number);
int frame_read(int fd, void* buffer, unsigned size, unsigned offset);
int frame_write(int fd, void* buffer, unsigned size, unsigned offset);

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

