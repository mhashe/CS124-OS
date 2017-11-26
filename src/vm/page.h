/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>

struct sup_entry {
    int fd;             /* Pointer to file to be opened. */
    unsigned file_ofs;  /* File loaded into page at fd's offset. */
    bool writable;      /* Whether the page is writable. */
    bool loaded;        /* Whether data has been successfully loaded. */
    uint32_t frame_no;  /* Frame number which data was loaded into. */
};

struct sup_entry *** sup_pagedir_create(void);
int sup_alloc_file(void * vaddr, int fd, bool writable);
int sup_load_file(void *vaddr, bool user, bool write);
int sup_remove_file(void *vaddr);

#endif /* vm/page.h */

