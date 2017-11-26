#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "filesys/file.h"     /* For file ops. */
#include "userprog/syscall.h"
#include "threads/thread.h"
#include "vm/page.h"
#include "userprog/pagedir.h"


/* TODO : verify init_ram_pages is the right number. */
struct frame_table_entry** frame_table;

void init_frame_table(void) {
    frame_table = (struct frame_table_entry**) 
                calloc(sizeof(struct frame_table_entry*), init_ram_pages);

    for (uint32_t i = 0; i < init_ram_pages; i++) {
        frame_table[i] = (struct frame_table_entry*) 
                        calloc(sizeof(struct frame_table_entry), 1);
        if (i < 256) {
            /* not free memory */
            frame_table[i]->valid = false;
        } else if (i < 256 + 1) {
            /* kernel pool base map, not available for us */
            frame_table[i]->valid = false;
            frame_table[i]->user  = false;
        } else if (i < 256 + 1 + 367) {
            /* kernel pool, available for us */
            frame_table[i]->valid = true;
            frame_table[i]->user  = false;
        } else if (i < 256 + 1 + 367 + 1) {
            /* user pool base map, not available for us */
            frame_table[i]->valid = false;
            frame_table[i]->user  = true;
        } else if (i < 256 + 1 + 367 + 1 + 367) {
            /* user pool, available for us */
            frame_table[i]->valid = true;
            frame_table[i]->user  = true;
        } else {
            PANIC("Magic numbers have failed us.");
        }
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
	/* Set accessed/dirty bits in all frame table entries. */
	set_bits();

    uint32_t vic = 0;

    /* See if empty frame exists. */
    for (uint32_t i = 0; i < init_ram_pages; i++) {
        if (!frame_table[i]->page && frame_table[i]->valid && 
            frame_table[i]->user == user) {
            return i;
        }
    }

    /* If not, evict a page. */
    /* TODO: evict into the swap */

    // Temp; just free last page.
    vic = init_ram_pages - 1;
    free_frame(vic);
    return vic;
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
        // PANIC("frame table full\n");
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
int frame_read(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    ASSERT(fd > 1);
    ASSERT(is_kernel_vaddr(buffer));

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        bytes = file_read_at(file, buffer, size, offset);
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
int frame_write(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        bytes = file_write_at(file, buffer, size, offset);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}


