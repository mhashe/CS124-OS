#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vm/page.h"
#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/thread.h"
#include "filesys/filesys.h"  /* For filesys ops. */
#include "filesys/file.h"     /* For file ops. */
#include "userprog/syscall.h"

static inline struct sup_entry *sup_get_entry(uint32_t *vaddr, struct sup_entry ***sup_pagedir);
static inline void sup_set_entry(uint32_t *vaddr, struct sup_entry ***sup_pagedir, struct sup_entry *entry);
static int filesize(int fd);
static struct file_des *file_from_fd(int fd);
static int read(int fd, void* buffer, unsigned size, unsigned offset);

struct sup_entry *** sup_pagedir_create(void) {
    struct sup_entry ***sup_pagedir;
    struct sup_entry **sup_t;

    sup_pagedir = (struct sup_entry ***) palloc_get_page(PAL_ASSERT | PAL_ZERO);
    sup_t = NULL;
    for (size_t page = 0; page < init_ram_pages; page++) {
        char *vaddr = ptov(page * PGSIZE);
        size_t pde_idx = pd_no(vaddr);
        size_t pte_idx = pt_no(vaddr);

        // TODO: is 0 the same as null? counting on that here...
        if (sup_pagedir[pde_idx] == NULL) {
            sup_t = (struct sup_entry **) palloc_get_page(PAL_ASSERT | PAL_ZERO);
            sup_pagedir[pde_idx] = (struct sup_entry **) sup_t;
        }

        sup_t[pte_idx] = NULL;
    }

    return sup_pagedir;
}


/* Allocates entire file in as many pages as needed in supplementary page 
table. To be called in mmap. Returns 0 on success, -1 on failure. */
int sup_alloc_file(uint32_t * vaddr, int fd) {
    struct sup_entry ***sup_pagedir = thread_current()->sup_pagedir;
    
    int offset = pg_ofs(vaddr);

    // Make sure that this vaddr is empty
    int num_pages = (offset + filesize(fd)) / PGSIZE;
    num_pages += ((num_pages % PGSIZE) != 0);
    vaddr = pg_round_down(vaddr);

    /* Check if needed pages are available. */
    for (int page = 0; page < num_pages; page++) {
        uint32_t* addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        // TODO: If this address is not a valid address for other reasons, return -1

        if (spe != NULL) {
            return -1;
        }
    }

    /* Add the needed pages to the supplemental table with correct file and 
    offset. */
    for (int page = 0; page < num_pages; page ++) {
        uint32_t *addr = (vaddr + (PGSIZE * page));
        struct sup_entry* spe = sup_get_entry(addr, sup_pagedir);

        spe =(struct sup_entry *) malloc(sizeof(struct sup_entry));
        spe->fd = fd;
        spe->file_ofs = offset + (PGSIZE * page);
        sup_set_entry(addr, sup_pagedir, spe);
    }

    return 0;
}


/* Loads part of file needed at vaddr page. Returns 0 on success, -1 on 
failure. */
int sup_load_file(uint32_t *vaddr, bool user) {
    struct sup_entry * spe = sup_get_entry(pg_round_down(vaddr), 
        thread_current()->sup_pagedir);

    if (spe == NULL) {
        return -1;
    }

    uint32_t * frame_page = get_frame(vaddr, user);

    // TODO: load part of file into frame
    // Is it possible to read in a certain offset in a file? 
    // read(spe->fd, (addr of frame), PG_SIZE, spe->file_ofs)
    if (read(spe->fd, frame_page, (unsigned) PGSIZE, (unsigned) spe->file_ofs) == -1) {
        return -1;
    }

    return 0;
}



/* Retreives supplemental entry from sup_pagedir at vaddr, which must be 
page-aligned. */
static inline struct sup_entry *sup_get_entry(uint32_t *vaddr, struct sup_entry ***sup_pagedir) {
    return sup_pagedir[pd_no(vaddr)][pt_no(vaddr)];
}


/* Sets supplemental entry from sup_pagedir at vaddr to be entry. vaddr must 
be page-aligned. */
static inline void sup_set_entry(uint32_t *vaddr, struct sup_entry ***sup_pagedir, struct sup_entry *entry) {
    sup_pagedir[pd_no(vaddr)][pt_no(vaddr)] = entry;
}





/* Returns the size, in bytes, of the file open as fd. */
static int filesize(int fd) {
    int filesize;
    /* Special cases. */
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO) {
        thread_exit();
    }

    /* Return file size. */
    struct file* file = file_from_fd(fd)->file;
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

/* Each thread maintains a list of its file descriptors. Get the file object 
   associated with the file descriptor. */
static struct file_des *file_from_fd(int fd) {
    /* 0, 1 reserved for STDIN, STDOUT. */
    ASSERT(fd > STDIN_FILENO);
    ASSERT(fd > STDOUT_FILENO);

    /* Iterate over file descriptors, return if one matches. */
    struct list_elem *e;
    struct list *lst = &thread_current()->fds;
    struct file_des* fd_s;

    for (e = list_begin(lst); e != list_end(lst); e = list_next(e)) {
        /* File descriptor object. */
        fd_s = list_entry(e, struct file_des, elem);

        /* If a match, return. */
        if (fd_s->fd == fd) {
            return fd_s;
        }
    }

    /* Invalid file descriptor; terminate offending process. */
    thread_exit();
}


static int read(int fd, void* buffer, unsigned size, unsigned offset) {
    int bytes;

    /* Verify arguments. */
    verify_pointer((uint32_t *) buffer);
    verify_pointer((uint32_t *) (buffer + size));

    /* Return number of bytes read. */
    struct file* file = file_from_fd(fd)->file;
    if (file) {
        // lock_acquire(&filesys_io);                // LOCKS HAVE BEEN REMOVED
        file_seek(file, offset);  
        bytes = file_read(file, buffer, size);
        // lock_release(&filesys_io);               // LOCKS HAVE BEEN REMOVED
    } else {
        /* Can't read invalid file. */
        bytes = -1;
    }

    return bytes;
}
