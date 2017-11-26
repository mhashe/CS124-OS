#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/inttypes.h"
#include "threads/interrupt.h"

/* Process identifier. */
typedef int pid_t;

/* Mapid identifier. */
typedef int mapid_t; // TODO: get from page.h?
#define MAP_FAILED ((mapid_t) -1)

void syscall_init(void);

uint32_t* verify_pointer(uint32_t* p);
uint32_t* verify_user_pointer(uint32_t* p);


#endif /* userprog/syscall.h */

