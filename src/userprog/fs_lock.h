#ifndef FS_LOCK_H
#define FS_LOCK_H

#include "threads/synch.h"

/* Global filesystem lock - will be removed in project 6. */
struct lock filesys_io;

/* Init the lock. */
void fs_lock_init(void) {
	lock_init(&filesys_io);
}

#endif