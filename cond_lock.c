/*
 * Copyright (c) 2002 Justin Armstrong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "cond_lock.h"


void
cond_lock_create(struct cond_lock_t* cond) {
    cond->blocked_thread_bitmask = 0;
    cond->triggered = FALSE;
}


/*
 * don't use this from an interrupt handler, you'll end up blocking some random thread!
 */
void
cond_lock_wait(struct cond_lock_t* cond) {
    DISABLE_INTERRUPTS();
    cond->blocked_thread_bitmask = BITSET(cond->blocked_thread_bitmask, thread_current());
    kern_threads[thread_current()].state = BLOCKED;
    cond->triggered = FALSE;
    ENABLE_INTERRUPTS();

    //eventually the scheduler will run someone else
    //and we will not run until we are made READY again

    //we sit and wait until the condition is triggered
    //when this happens, we will be made ready and removed from the cond_lock's blocked list
    while(1) {
        DISABLE_INTERRUPTS();
        if (cond->triggered) {
            break;
        }
        ENABLE_INTERRUPTS();
    }
}


void
cond_lock_signal(struct cond_lock_t* cond) {
    int i;
    DISABLE_INTERRUPTS();
    cond->triggered = TRUE;
    //wake up all blocked threads
    for (i = 0; i < MAX_THREADS; i++) {

        if (BITTEST(cond->blocked_thread_bitmask, i)) {
            cond->blocked_thread_bitmask = BITCLEAR(cond->blocked_thread_bitmask, i);
            kern_threads[i].state = READY;
        }
    }
    ENABLE_INTERRUPTS();
}
