#include <list.h>
#include "page.h"
#include "threads/thread.h"
#include "frame.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"

#include "clru.h"

void init_clru(void);
void clru_timer_tick(void);
static void set_bits(void);
void clru_enqueue(uint32_t fn);
uint32_t clru_evict(void);

/* Clock-LRU list. Modified by timer interrupts. */
static struct list lru;

/* Max size of queue. */
static uint32_t max = 367;

struct clru_entry {
	uint32_t fn; 			/*!< Frame table number. */

	struct list_elem elem; 	/*!< Pointers to previous, next file descriptors. */
};


void clru_init(void) {
	/* Init queue. */
	list_init(&lru);
}

/* Updates queue according to recent access. */
void clru_timer_tick(void) {
	/* Make sure list has actually been initted. */
	if (lru.head.next == NULL) {
		/* Not initted, nothing to do. */
		return;
	}

	/* If list isn't full, also no reason to updated. */
	if (list_size(&lru) < max) {
		return;
	}

	/* Update bits in frame table. */
	set_bits();

	/* Move all frames that have been accessed to the back of the queue. */
    struct list_elem *e;
    struct list_elem *e_prev;

	/* Start at front of list, go to back. This preserves relative ordering of
	   frames. Reset accessed bit along the way. */
    e = list_begin(&lru);
    ASSERT(e);
    while (e != list_end(&lru)) {
        struct clru_entry *ce = list_entry(e, struct clru_entry, elem);

        e_prev = e;
        e = list_next(e);
        
    	/* If accessed, reset access flag and set to back. */
        if (frame_table[ce->fn]->acc) {

    		/* Critical to reset access flag; otherwise, this loop will continue inifinitely. */
        	frame_table[ce->fn]->acc = 0;

        	/* Move to back of list. */
        	list_remove(e_prev);
        	list_push_back(&lru, e_prev);
        }
    }
    // ASSERT(list_size(&lru) <= max);
}

/* Adds a new frame to the LRU queue. */
void clru_enqueue(uint32_t fn) {
	/* Need to store in this struct to work with list. */
	struct clru_entry* new = malloc(sizeof(struct clru_entry));
	new->fn = fn;

	list_push_back(&lru, &new->elem);

	// ASSERT(list_size(&lru) <= max);
	// printf("SIZE: %d\n", list_size(&lru));
}

/* Returns front of queue. */
uint32_t clru_evict(void) {
	struct list_elem *e = list_pop_front(&lru);

	struct clru_entry* old = list_entry(e, struct clru_entry, elem);

	uint32_t fn = old->fn;

	free(old);

	return fn;
}

static void set_bits(void) {
	struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
	uint32_t *pd = thread_current()->pagedir;

	/* If these haven't been set yet, nothing to do. */
	if (!sup_pagedir || !pd) {
		return;
	}

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            if (sup_pagedir[i][j]) {
                /* Check access, dirty bits. */
                void *page = sup_index_to_vaddr(i, j);
                int frame_no = sup_pagedir[i][j]->frame_no;

                /* If frame_no is -1, it points to swap. */
                if (frame_no == -1) {
                    continue;
                }

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

