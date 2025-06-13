#ifndef __SPINLOCK__H
#define __SPINLOCK__H

#include <stdint.h>
#include <stdatomic.h>

void acquire_lock( atomic_flag * lock );
void release_lock( atomic_flag * lock );
#endif