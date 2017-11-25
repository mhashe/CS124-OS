#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/inttypes.h"
#include "threads/interrupt.h"

/* Process identifier. */
typedef int pid_t;

void syscall_init(void);

uint32_t* verify_pointer(uint32_t* p);

void exit(struct intr_frame *f);


#endif /* userprog/syscall.h */

