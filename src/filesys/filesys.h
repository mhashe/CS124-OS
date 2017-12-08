#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"

/*! Sectors of system file inodes. @{ */
#define FREE_MAP_SECTOR 0       /*!< Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /*!< Root directory file inode sector. */
#define MAX_FILES_PER_DIR 150
/*! @} */

/*! Block device that contains the file system. */
struct block *fs_device;

void filesys_init(bool format);
void filesys_done(void);
bool filesys_create(const char *path, off_t initial_size, bool is_directory);
struct file *filesys_open(const char *name);
bool filesys_remove(const char *name);

struct dir * filesys_get_parent(const char *path);

#endif /* filesys/filesys.h */

