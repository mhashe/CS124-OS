#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/inttypes.h"
#include "threads/interrupt.h"
#include "vm/page.h"

/* Process identifier. */
typedef int pid_t;

void syscall_init(void);

uint32_t* verify_pointer(uint32_t* p);
uint32_t* verify_user_pointer(uint32_t* p);

struct file_des *file_from_fd(int fd);



#endif /* userprog/syscall.h */

