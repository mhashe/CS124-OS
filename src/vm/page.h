/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>

struct sup_entry {
    int fd;             /* pointer to file to be opened */
    unsigned file_ofs;       /* file loaded into page at fd's offset */
    bool writable;      /* whether the page is writable */
};

struct sup_entry *** sup_pagedir_create(void);
int sup_alloc_file(void * vaddr, int fd, bool writable);
int sup_load_file(void *vaddr, bool user, bool write);

#endif /* vm/page.h */

