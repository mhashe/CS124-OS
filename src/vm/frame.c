#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/malloc.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "filesys/file.h"     /* For file ops. */
#include "userprog/syscall.h"
#include "threads/thread.h"
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/interrupt.h"

/* TODO : verify init_ram_pages is the right number. */
struct frame_table_entry** frame_table;

void frame_init(size_t user_page_limit) {
    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), init_ram_pages);

    /* Shadow code in palloc_init for memory pool init. */
    uint8_t *free_start = ptov(1024 * 1024);
    uint8_t *free_end = ptov(init_ram_pages * PGSIZE);
    size_t reserved_pages = (uint32_t)free_start / PGSIZE;
    printf("%d\n", reserved_pages);
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

        // list_init(&frame_table[i]->sup_entries);
    }
}

static void set_bits(void) {
	struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
	uint32_t *pd = thread_current()->pagedir;

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            if (sup_pagedir[i][j]) {
                /* Check access, dirty bits. */
                void *page = sup_index_to_vaddr(i, j);
                uint32_t frame_no = sup_pagedir[i][j]->frame_no;

                if (pagedir_is_accessed(pd, page)) {
                	/* If accesed since last check, ought to be in a frame. */
                	ASSERT(frame_table[frame_no]);

                	frame_table[frame_no]->acc = 1;

                	/* Mark unaccessed for eviction policy? */
                }

                if (pagedir_is_dirty(pd, page)) {
                	ASSERT(frame_table[frame_no]);

                	frame_table[frame_no]->dirty = 1;
                }
            }
        }
    }
}

/* Select an empty frame if available, otherwise choose a page
   to evict and evict it. */
static uint32_t evict(bool user) {
    // TODO: remove this?
    ASSERT(user);

    /* Set accessed/dirty bits in all frame table entries. */
    // set_bits();

    uint32_t victim = 0;

    /* No empty frame exists. */
    for (uint32_t i = 0; i < init_ram_pages; i++) {
        if (!frame_table[i]->page && frame_table[i]->valid && 
            frame_table[i]->user == user) {
            ASSERT(0);
        }
    }

    /* If not, evict a page. */
    /* TODO: evict into the swap */


    // Temp; just free last page.
    victim = init_ram_pages - 1;

    /* Allocate swap, write frame to swap, and then free frame. */
    swapslot_t new_swap = swap_alloc();
    swap_write(new_swap, ftov(victim));

    // struct list_elem *e;

    // intr_disable();

    // for (e = list_begin(&all_list); e != list_end(&all_list);
    //      e = list_next(e)) {
    //     struct thread *t = list_entry(e, struct thread, allelem);

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
                // printf("HELLO VICTIM\n");
                
                ASSERT(entry->slot == SUP_NO_SWAP);

                entry->slot = new_swap;
                entry->frame_no = FRAME_NONE;

                /* Free victim from user page directory */
                pagedir_clear_page(pd, sup_index_to_vaddr(i, j));
            }

        }
    }
    // }

    free_frame(victim);

    // intr_enable();

    return victim;
}


/* Gets the first free frame in the frame table. Returns an index. 
   page is the virtual memory pointer to a page that is occupying this frame. */
uint32_t get_frame(bool user) {
    // uint32_t i = 0;
    // for (i = 0; i < init_ram_pages; i++) {
    //     if (frame_table[i] == NULL) {
    //         break;
    //     }
    // }
    void *frame;

    if (user) {
        frame = palloc_get_page(PAL_ZERO | PAL_USER);
    }
    else {
        frame = palloc_get_page(PAL_ZERO);
    }

    if (frame == NULL) {
        // TODO: evict then palloc again
        // printf("Frame Table Full\n");
        evict(user);

        // TODO : streamline this code
        if (user) {
        	frame = palloc_get_page(PAL_ZERO | PAL_USER);
        } else {
        	frame = palloc_get_page(PAL_ZERO);
        }
    }
    ASSERT(frame);

    uint32_t frame_number = vtof(frame);

    ASSERT(frame_number < init_ram_pages);

    frame_table[frame_number]->page = frame;

    // frame_table[frame_number]->sup 

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
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        bytes = file_read_at(f, buffer, size, offset);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
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
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        bytes = file_write_at(f, buffer, size, offset);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


