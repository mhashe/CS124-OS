/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>
// #include <list.h>
#include "userprog/syscall.h"
#include "threads/pte.h"
#include "vm/swap.h"
#include "threads/synch.h"


#define SUP_NO_SWAP (size_t) -1

/* Mapid identifier. */
typedef int mapid_t;

/* Types of pages in supplemental table entry. */
// enum sup_type {
//     SUP_ALL_ZERO,             /* Represents an initially all-zero page. */
//     SUP_FILE,                 /* Represents a page to a file on disk. */
//     SUP_SWAP                  /* Comment. */
// };
// 

struct sup_entry {
    uint32_t frame_no;   /* Frame number which data was loaded into. */
    swapslot_t slot;     /* Swap slot index if mapped into a swap slot. */
    bool all_zero;       /* If an initially all-zero page. Else, a file. */

    /* File-specific fields */  // TODO: Potentially make a sup_file_entry struct?
    struct file * f;     /* Pointer to file to be opened. */
    unsigned file_ofs;   /* File loaded into page at fd's offset. */
    unsigned page_end;   /* File ends at this location in page. */
    bool writable;       /* Whether the page is writable. */
    bool loaded;         /* Whether data has been successfully loaded. */
    mapid_t mapid;       /* Map id if mapped with mmap. */

    /* Lock for loading. */
    struct lock lock; 
};

void sup_init(void);
struct sup_entry *** sup_pagedir_create(void);
int sup_alloc_file(void * vaddr, struct file *file, bool writable);
int sup_load_page(void *vaddr, bool user, bool write);
void sup_remove_map(mapid_t mapid);
void sup_free_table(struct sup_entry ***sup_pagedir);
int sup_alloc_all_zeros(void * vaddr, bool user);
void sup_alloc_segment(void *addr, struct file *file, bool writable, 
        unsigned offset, unsigned page_end, mapid_t mapid);
mapid_t sup_inc_mapid(void);

/* Convert a directory index and table index to a virtual address of a page. */
static inline void* sup_index_to_vaddr(uint32_t di, uint32_t ti) {
    return (void *) (((uintptr_t)di << PDSHIFT) | ((uintptr_t)ti << PTSHIFT));
}

/* Retreives supplemental entry from sup_pagedir at upage, which must be 
page-aligned. */
static inline struct sup_entry *sup_get_entry(void *upage, 
                                            struct sup_entry ***sup_pagedir) {
    uintptr_t pd = pd_no(upage);
    if (sup_pagedir[pd] == NULL) {
        return NULL;
    }
    return sup_pagedir[pd][pt_no(upage)];
}

#endif /* vm/page.h */

