#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"

/* Number of direct, indirect, and double indirect inodes. */
#define NUM_ENTRIES_IN_INDIRECT (BLOCK_SECTOR_SIZE / 4)

struct bitmap;

void inode_init(void);
bool inode_create(block_sector_t, off_t, bool);
struct inode *inode_open(block_sector_t);
struct inode *inode_reopen(struct inode *);
block_sector_t inode_get_inumber(const struct inode *);
void inode_close(struct inode *);
void inode_remove(struct inode *);
off_t inode_read_at(struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at(struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write(struct inode *);
void inode_allow_write(struct inode *);
off_t inode_length(const struct inode *);

bool inode_is_directory(struct inode *inode);
block_sector_t inode_get_sector(struct inode *inode);
void inode_check(struct inode *inode);
bool inode_is_removed(struct inode *inode);

void inode_incr_count(struct inode *inode);
void inode_decr_count(struct inode *inode);
int inode_num_files(struct inode *inode);

void directory_lock(struct inode *inode);
void directory_release(struct inode *inode);

#endif /* filesys/inode.h */
