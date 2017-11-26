/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>
#include "userprog/syscall.h"

/* Mapid identifier. */
typedef int mapid_t;

struct sup_entry {
    int fd;             /* Pointer to file to be opened. */
    unsigned file_ofs;  /* File loaded into page at fd's offset. */
    bool writable;      /* Whether the page is writable. */
    bool loaded;        /* Whether data has been successfully loaded. */
    uint32_t frame_no;  /* Frame number which data was loaded into. */
    mapid_t mapid;      /* Map id if mapped with mmap. */
};

struct sup_entry *** sup_pagedir_create(void);
int sup_alloc_file(void * vaddr, int fd, bool writable);
int sup_load_file(void *vaddr, bool user, bool write);
int sup_remove_map(mapid_t mapid);
void sup_free_table(struct sup_entry ***sup_pagedir);

#endif /* vm/page.h */

