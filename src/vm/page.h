/*! \file page.h
 *
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdint.h>

struct sup_entry {
    int fd;     /* pointer to file to be opened */
    int file_ofs;            /* file loaded into page at fd's offset */
};

struct sup_entry *** sup_pagedir_create(void);
int sup_alloc_file(uint32_t * vaddr, int fd);
int sup_load_file(uint32_t *vaddr, bool user);

#endif /* vm/page.h */

