#ifndef COND_LOCK_H
#define COND_LOCK_H

#include "kernel.h"

struct cond_lock_t {
    volatile bool   triggered;
    u32    blocked_thread_bitmask;
};

void
cond_lock_create(struct cond_lock_t* cond);

void
cond_lock_wait(struct cond_lock_t* cond);

void
cond_lock_signal(struct cond_lock_t* cond);

#endif
