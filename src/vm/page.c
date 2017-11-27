#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/thread.h"
#include "filesys/file.h"     /* For file ops. */
#include "userprog/pagedir.h"
// #include "filesys/filesys.h"  /* For filesys ops. */
// #include "userprog/syscall.h"


inline mapid_t sup_inc_mapid(void);
static void sup_set_entry(void *vaddr, struct sup_entry *** sup_pagedir, 
    struct sup_entry *entry);

static void sup_remove_entry(void *upage, struct sup_entry *** 
    sup_pagedir);

/* Functions from syscall. TODO: how to not reimplement them here? */
static int filesize(struct file *file);


/* Allocates and returns a pointer to an empty supplementary table. */
struct sup_entry *** sup_pagedir_create(void) {
    return (struct sup_entry ***) palloc_get_page(PAL_ASSERT | PAL_ZERO);
}


static mapid_t last_mapid;

/* Initialize the first mapid (representing mapping of file) as 0. */
void sup_init(void) {
    last_mapid = 0;
}


/* Increments mapid so that it is unique and returns it. */
inline mapid_t sup_inc_mapid(void) {
    return last_mapid++;
}


/* Allocate an all zero page and Returns 0 on success, -1 on failure. */
int sup_alloc_all_zeros(void * vaddr, bool user) {
    struct thread *cur = thread_current();
    struct sup_entry *spe;

    /* Allocate and get physical frame for data to be loaded into. */
    uint32_t frame_no = get_frame(user);
    void *kpage = ftov(frame_no);

    /* If the provided address is not page-aligned, return failure. */
    int offset = pg_ofs(vaddr);
    if (offset != 0) {
        lock_acquire(&frame_table[frame_no]->fte_lock);
        free_frame(frame_no);
        lock_release(&frame_table[frame_no]->fte_lock);
        return -1;
    }

    /* If the page specified by vaddr is already occupied, return failure. */
    if (sup_get_entry(vaddr, cur->sup_pagedir) != NULL) {
        lock_acquire(&frame_table[frame_no]->fte_lock);
        free_frame(frame_no);
        lock_release(&frame_table[frame_no]->fte_lock);
        return -1;
    }
 
    /* Linking frame to virtual address failed, so remove and deallocate the 
    page instantiated for it. */
    if (!pagedir_set_page(cur->pagedir, vaddr, kpage, true)) {
        lock_acquire(&frame_table[frame_no]->fte_lock);
        free_frame(frame_no);
        lock_release(&frame_table[frame_no]->fte_lock);
        return -1;
    }

    /* Create supplmentary entry corresponding to an initially all zero page. */
    spe = (struct sup_entry *) malloc(sizeof(struct sup_entry));
    spe->f = NULL;
    spe->file_ofs = 0;
    spe->page_end = PGSIZE;
    spe->writable = true;
    spe->loaded = true;
    spe->all_zero = true;
    spe->slot = SUP_NO_SWAP;
    spe->frame_no = frame_no;
    spe->mapid = MAP_FAILED;
    lock_init(&spe->spe_lock);

    sup_set_entry(vaddr, cur->sup_pagedir, spe);
    // lock_release(&frame_table[frame_no]->fte_lock);

    return 0;
}


/* Allocates entire file in as many pages as needed in supplementary page table.
   The file is given by "file" and it is writable if "writable". To be called
   in mmap. Returns entry on success, NULL on failure. Note this function does
   not actually load the pages into memory. That is done on subsequent page
   faults. */
int sup_alloc_file(void * vaddr, struct file *file, bool writable) {
    /* The provided address must be page aligned. */
    int offset = pg_ofs(vaddr);
    if (offset != 0) {
        return MAP_FAILED;
    }

    /* Current thread's supplemental page directory. */
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    

    /* Calculate the number of pages required to allocate file. */
    int file_size = filesize(file);
    int num_pages = file_size / PGSIZE;
    num_pages += ((file_size % PGSIZE) != 0);

    /* Start allocating page for file in supplemental table at page-align. */
    vaddr = pg_round_down(vaddr);

    /* Check if needed pages are available in the supplementary table. */
    for (int page = 0; page < num_pages; page++) {
        void* addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        // TODO: If this address is not a valid address for other reasons, return -1

        /* There are not enough free pages to allocate the file, so fail. */
        if (spe != NULL) {
            return MAP_FAILED;
        }
    }

    mapid_t last_mapid = sup_inc_mapid();

    unsigned page_end;

    /* Add the needed pages to the supplemental table with correct file. */
    for (int page = 0; page < num_pages; page ++) {
        void *addr = (vaddr + (PGSIZE * page));
        if (page == num_pages - 1) {
            page_end = file_size % PGSIZE;
        } else {
            page_end = PGSIZE;
        }
        sup_alloc_segment(addr, file, writable, (unsigned) (PGSIZE * page), 
            page_end, last_mapid);
        
    }

    return last_mapid;
}

void sup_alloc_segment(void *addr, struct file *file, bool writable, 
        unsigned offset, unsigned page_end, mapid_t mapid) {

    /* Current thread's supplemental page directory. */
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    
    ASSERT(sup_get_entry(addr, sup_pagedir) == NULL);

    struct sup_entry *spe = (struct sup_entry *) malloc(sizeof(struct sup_entry));
    ASSERT(spe != NULL);

    /* Validity of a mapped file is independent of the original file so create
       a copy of the file. */
    spe->f = file_reopen(file);
    spe->file_ofs = offset;
    spe->page_end = page_end;
    spe->writable = writable;
    spe->loaded = false;
    spe->frame_no = FRAME_NONE;
    spe->mapid = mapid;
    spe->all_zero = false;
    spe->slot = SUP_NO_SWAP;
    lock_init(&spe->spe_lock);

    sup_set_entry(addr, sup_pagedir, spe);
}


/* Loads part of file needed at vaddr page. Returns 0 on success, -1 on 
   failure. */
int sup_load_page(void *vaddr, bool user, bool write) {
    struct thread *cur = thread_current();
    void *upage = pg_round_down(vaddr);
    struct sup_entry * spe = sup_get_entry(upage, cur->sup_pagedir);

    /* If entry in supplementary page table does not exist, then failure. */
    if (spe == NULL) {
        printf("LOAD ERR: 1:: %p, %d, %d\n", vaddr, user, write);
        return -1;
    }
    lock_acquire(&spe->spe_lock);

    /* Unknown page fault since data has been loaded from disk already and isn't
       in memory or swap. */
    if (spe->loaded && (spe->slot == SUP_NO_SWAP)) {
        printf("LOAD ERR: 2\n");
        lock_release(&spe->spe_lock);
        return -1;
    }

    /* Invalid access if page fault was due to write attempt on r/only page. */
    if (write && (!spe->writable)) {
        printf("LOAD ERR: 3\n");
        lock_release(&spe->spe_lock);
        return -1;
    }

    /* Allocate and get physical frame for data to be loaded into. */
    uint32_t frame_no = get_frame(user);
    void *kpage = ftov(frame_no);


    lock_acquire(&frame_table[frame_no]->fte_lock);
    /* If not present in swap, load one page of the file at file_ofs into the frame. */
    if (spe->slot == SUP_NO_SWAP) {
        if (frame_read(spe->f, kpage, spe->page_end, spe->file_ofs) == -1) {
            printf("LOAD ERR: 4\n");
            free_frame(frame_no);
            lock_release(&spe->spe_lock);
            return -1;
        }
    } else {
        /* Else if in swap, just load the page from swap into the frame. */
        swap_read(spe->slot, kpage);
        swap_free(spe->slot);
        spe->slot = SUP_NO_SWAP;
    }

    /* Linking frame to virtual address failed, so remove and deallocate the 
    page instantiated for it. */
    if (!pagedir_set_page(cur->pagedir, upage, kpage, spe->writable)) {
        printf("LOAD ERR: 5\n");
        free_frame(frame_no);
        lock_release(&spe->spe_lock);
        return -1;
    }

    spe->frame_no = frame_no;
    spe->loaded = true;
    lock_release(&frame_table[frame_no]->fte_lock);

    /* Release victim frame. */
    // lock_release(&frame_table[frame_no]->fte_lock);

    lock_release(&spe->spe_lock);
    return 0;
}


/* Deallocate and remove file from supplementary page table. 
TODO: Deal with multiple maps to single frame!
*/
void sup_remove_map(mapid_t mapid) {
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    struct sup_entry *entry;
    void *temp_swap_frame = NULL;
    uint32_t temp_swap_frame_no;

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            entry = sup_pagedir[i][j];
            if (!entry || entry->mapid != mapid) {
                continue;
            }
            lock_acquire(&entry->spe_lock);
            void *vaddr = sup_index_to_vaddr(i, j);

            if (entry->loaded) {
                if (entry->slot == SUP_NO_SWAP) {
                    /* Write frame to disk and free frame. */

                    lock_acquire(&frame_table[entry->frame_no]->fte_lock);
                    // TODO: check if dirty
                    if (entry->writable && !entry->all_zero) {
                        frame_write(entry->f, ftov(entry->frame_no), 
                            entry->page_end, entry->file_ofs);
                    }
                    pagedir_clear_page(thread_current()->pagedir, vaddr);
                    entry->loaded = false;
                    free_frame(entry->frame_no);
                    lock_release(&frame_table[entry->frame_no]->fte_lock);
                } else {
                    /* Write swap to frame, write frame to disk, delloc swap */

                    // TODO: check if dirty
                    if (entry->writable && !entry->all_zero) {
                        if (!temp_swap_frame) {
                            temp_swap_frame_no = get_frame(true);
                            temp_swap_frame = ftov(temp_swap_frame_no);
                        }
                        lock_acquire(&frame_table[temp_swap_frame_no]->fte_lock);
                        swap_read(entry->slot, temp_swap_frame);
                        frame_write(entry->f, temp_swap_frame, 
                            entry->page_end, entry->file_ofs);
                        lock_release(&frame_table[temp_swap_frame_no]->fte_lock);
                    }
                    swap_free(entry->slot);
                }
            }

            lock_release(&entry->spe_lock);
            sup_remove_entry(vaddr, sup_pagedir);
        }
    }

    if (temp_swap_frame) {
        lock_acquire(&frame_table[temp_swap_frame_no]->fte_lock);
        free_frame(temp_swap_frame_no);
        lock_release(&frame_table[temp_swap_frame_no]->fte_lock);
    }
}


/* Free all allocated pages and entries in the supplementary page table. 
TODO: Deal with multiple maps to single frame! 
*/
void sup_free_table(struct sup_entry ***sup_pagedir, uint32_t *pd) {
    struct sup_entry *entry;
    void *temp_swap_frame = NULL;

    uint32_t temp_swap_frame_no;

    for (uint32_t i = 0; i < PGSIZE / sizeof(struct sup_entry **); i++) {
        if (!sup_pagedir[i]) {
            continue;
        }
        for (uint32_t j = 0; j < PGSIZE / sizeof(struct sup_entry *); j++) {
            entry = sup_pagedir[i][j];
            if (!entry) {
                continue;
            }
            lock_acquire(&entry->spe_lock);
            void *vaddr = sup_index_to_vaddr(i, j);

            if (entry->loaded) {
                if (entry->slot == SUP_NO_SWAP) {
                    /* Write frame to disk and free frame if want to save. */

                    // TODO: check if dirty
                    lock_acquire(&frame_table[entry->frame_no]->fte_lock);
                    if (entry->writable && !entry->all_zero) {
                        frame_write(entry->f, ftov(entry->frame_no), 
                            entry->page_end, entry->file_ofs);
                    }
                    pagedir_clear_page(pd, vaddr);
                    entry->loaded = false;
                    if (frame_table[entry->frame_no]->page) {
                        free_frame(entry->frame_no);
                    }
                    lock_release(&frame_table[entry->frame_no]->fte_lock);
                } else {
                    // Write swap to frame, write frame to disk, delloc swap 
                    ASSERT(entry->frame_no == FRAME_NONE);

                    // TODO: check if dirty
                    if (entry->writable && !entry->all_zero) {
                        if (!temp_swap_frame) {
                            temp_swap_frame_no = get_frame(true);
                            temp_swap_frame = ftov(temp_swap_frame_no);
                        }
                        lock_acquire(&frame_table[temp_swap_frame_no]->fte_lock);
                        swap_read(entry->slot, temp_swap_frame);
                        frame_write(entry->f, temp_swap_frame, 
                            entry->page_end, entry->file_ofs);
                        lock_release(&frame_table[temp_swap_frame_no]->fte_lock);
                    }
                    swap_free(entry->slot);
                }
            }
            lock_release(&entry->spe_lock);
            sup_remove_entry(vaddr, sup_pagedir);
        }
        palloc_free_page(sup_pagedir[i]);
        sup_pagedir[i] = NULL;
    }
    if (temp_swap_frame) {
        lock_acquire(&frame_table[temp_swap_frame_no]->fte_lock);
        free_frame(temp_swap_frame_no);
        lock_release(&frame_table[temp_swap_frame_no]->fte_lock);
    }
    palloc_free_page(sup_pagedir);
    sup_pagedir = NULL;
}


/* Removes supplemental entry from sup_pagedir at upage, which must be 
page-aligned. Assumes enty exists. */
static void sup_remove_entry(void *upage, struct sup_entry 
    *** sup_pagedir) {
    // TODO: make this computationally more efficient with local vars
    struct sup_entry *sup_pte = sup_pagedir[pd_no(upage)][pt_no(upage)];
    sup_pagedir[pd_no(upage)][pt_no(upage)] = NULL;
    if(sup_pte->f) {
        file_close(sup_pte->f);
    }
    free(sup_pte);
    // TODO: free entire table if nothing is left?
}


/* Sets supplemental entry from sup_pagedir at upage to be entry. upage must 
be page-aligned. */
static void sup_set_entry(void *upage, struct sup_entry ***sup_pagedir, 
                                struct sup_entry *entry) {
    uintptr_t pd = pd_no(upage);
    if (sup_pagedir[pd] == NULL) {
        sup_pagedir[pd] = palloc_get_page(PAL_ASSERT | PAL_ZERO);
    }
    sup_pagedir[pd][pt_no(upage)] = entry;
}



/* Returns the size, in bytes, of the file. */
static int filesize(struct file* file) {
    int filesize;

    /* Return file size. */
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        filesize = file_length(file);
        // lock_release(&filesys_io);                // LOCKS HAVE BEEN REMOVED
    } else {
        /* Invalid file has no size. */
        thread_exit();
    }

    return filesize;
}
