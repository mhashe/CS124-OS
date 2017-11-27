#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "filesys/file.h"     /* For file ops. */
#include "threads/interrupt.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/clru.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"

struct frame_table_entry** frame_table;

void frame_init(size_t user_page_limit) {
    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), init_ram_pages);

    /* Shadow code in palloc_init for memory pool init. */
    uint8_t *free_start = ptov(1024 * 1024);
    uint8_t *free_end = ptov(init_ram_pages * PGSIZE);

    size_t reserved_pages = (uint32_t)vtop(free_start) / PGSIZE;
    size_t free_pages = (free_end - free_start) / PGSIZE;
    size_t user_pages = free_pages / 2;
    size_t kernel_pages;
    
    if (user_pages > user_page_limit)
        user_pages = user_page_limit;
    kernel_pages = free_pages - user_pages;

    for (uint32_t i = 0; i < init_ram_pages; i++) {
        frame_table[i] = (struct frame_table_entry*) 
                        calloc(sizeof(struct frame_table_entry), 1);
        if (i < reserved_pages) {
            /* not free memory */
            frame_table[i]->valid = false;
        } else if (i < reserved_pages + 1) {
            /* kernel pool base map, not available for us */
            frame_table[i]->valid = false;
            frame_table[i]->user  = false;
        } else if (i < reserved_pages + kernel_pages) {
            /* kernel pool, available for us */
            frame_table[i]->valid = true;
            frame_table[i]->user  = false;
        } else if (i < reserved_pages + kernel_pages + 1) {
            /* user pool base map, not available for us */
            frame_table[i]->valid = false;
            frame_table[i]->user  = true;
        } else if (i < reserved_pages + kernel_pages + user_pages) {
            /* user pool, available for us */
            frame_table[i]->valid = true;
            frame_table[i]->user  = true;
        } else {
            PANIC("Frame table init failed.");
        }
    }
}

/* Select an empty frame if available, otherwise choose a page
   to evict and evict it. */
static uint32_t evict(bool user) {
    ASSERT(user);

    uint32_t victim = 0;

    /* No empty frame exists. */
    for (uint32_t i = 0; i < init_ram_pages; i++) {
        if (!frame_table[i]->page && frame_table[i]->valid && 
            frame_table[i]->user == user) {
            ASSERT(0);
        }
    }

    /* If not, evict a page. */
    victim = clru_evict();


    /* Allocate swap, write frame to swap, and then free frame. */
    swapslot_t new_swap = swap_alloc();
    swap_write(new_swap, ftov(victim));


    /* Find all entries that use victim and redirect them to swap. */
    struct thread *t = thread_current();
    struct sup_entry ***sup_pagedir = t->sup_pagedir;
    uint32_t *pd = t->pagedir;

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }

        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            struct sup_entry* entry = sup_pagedir[i][j];

            /* If entry is a victim entry, set redirect it to swap. */
            if (entry && (entry->frame_no == victim)) {
                ASSERT(entry->slot == SUP_NO_SWAP);

                entry->slot = new_swap;
                entry->frame_no = FRAME_NONE;

                /* Free victim from user page directory */
                pagedir_clear_page(pd, sup_index_to_vaddr(i, j));
            }

        }
    }

    return victim;
}


/* Gets the first free frame in the frame table. Returns an index. 
   page is the virtual memory pointer to a page that is occupying this frame. */
uint32_t get_frame(bool user) {
    void *frame;
    uint32_t frame_number;

    if (user) {
        frame = palloc_get_page(PAL_ZERO | PAL_USER);
    }
    else {
        frame = palloc_get_page(PAL_ZERO);
    }

    if (frame == NULL) {
        frame_number = evict(user);
        frame = ftov(frame_number);

        memset(frame, 0, PGSIZE);
    }
    ASSERT(frame);

    frame_number = vtof(frame);

    ASSERT(frame_number < init_ram_pages);
    ASSERT(frame_table[frame_number]->user == user);
    ASSERT(frame_table[frame_number]->valid);


    frame_table[frame_number]->page  = frame;
    frame_table[frame_number]->acc   = 0;
    frame_table[frame_number]->dirty = 0;

    /* Keep track of new frame in clru. */
    if (user) {
        clru_enqueue(frame_number);
    }

    return frame_number;
}

/* Frees the page frame corresponding to the frame number given. */
void free_frame(uint32_t frame_number) {
    ASSERT(frame_number < init_ram_pages);


    if (frame_table[frame_number]->page) {
        palloc_free_page(frame_table[frame_number]->page);
    } else {
        PANIC("freeing frame that doesn't exist\n");
    }

    frame_table[frame_number]->page = NULL;
    frame_table[frame_number]->acc = 0;
    frame_table[frame_number]->dirty = 0;
}


/* Reads size bytes from the file open as fd into buffer. Returns the number of
   bytes actually read (0 at end of file), or -1 if the file could not be read
   (due to a condition other than end of file). Fd 0 reads from the keyboard
   using input_getc(). */
int frame_read(struct file* f, void* buffer, unsigned size, unsigned offset) {
    /* Return number of bytes read. */
    int bytes;

    ASSERT(is_kernel_vaddr(buffer));

    if (f) {
        bytes = file_read_at(f, buffer, size, offset);
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


/* Writes size bytes from buffer to the open file fd. Returns the number of 
   bytes actually written, which may be less than size if some bytes could not 
   be written. */
int frame_write(struct file* f, void* buffer, unsigned size, unsigned offset) {
    /* Return number of bytes read. */
    int bytes;

    if (f) {
        bytes = file_write_at(f, buffer, size, offset);
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


