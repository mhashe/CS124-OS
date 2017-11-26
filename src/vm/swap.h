/*! \file swap.h
 *
 */

#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <debug.h>
#include <stdint.h>
// #include "userprog/syscall.h"
// #include "threads/pte.h"


struct swap_entry {
    int fd;             /* Pointer to file to be opened. */
    unsigned file_ofs;  /* File loaded into page at fd's offset. */
    unsigned page_end;  /* File ends at this location in page. */
    bool writable;      /* Whether the page is writable. */
    bool loaded;        /* Whether data has been successfully loaded. */
    uint32_t frame_no;  /* Frame number which data was loaded into. */
    mapid_t mapid;      /* Map id if mapped with mmap. */
    bool all_zero;      /* Represents an initially all-zero page. */
};



#endif /* vm/swap.h */

