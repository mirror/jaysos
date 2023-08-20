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

#include "kernel.h"
#include "semaphore.h"

void
semaphore_create(struct semaphore_t* sem, int initial_count) {
    sem->count = initial_count;
    sem->blocked_thread_bitmask = 0;
}

bool
semaphore_try(struct semaphore_t* sem) {
    DISABLE_INTERRUPTS();

    if (sem->count < 1) {
        ENABLE_INTERRUPTS();
        return FALSE;
    }
    else {
        sem->count--;
        ENABLE_INTERRUPTS();
        return TRUE;
    }
}

/* do NOT call this from an interrupt handler! - use semaphore_try instead
*/
void
semaphore_wait(struct semaphore_t* sem) {
    DISABLE_INTERRUPTS();

    if (sem->count < 1) {
        sem->blocked_thread_bitmask = BITSET(sem->blocked_thread_bitmask, thread_current());
        kern_threads[thread_current()].state = BLOCKED;

        ENABLE_INTERRUPTS();

        //eventually the scheduler will run someone else
        //and we will not run until we are made READY again

        //now sit and wait until the semaphore is available
        //when this happens, we will be made ready and removed from the semaphore's blocked list
        while(1) {
            DISABLE_INTERRUPTS();
            if (sem->count > 0) {
                sem->count--;
                break;
            }
            else {
                //we didn't get it in time, someone else got it
                //re-block this thread and re-add it to the mutex's blocked list
                sem->blocked_thread_bitmask = BITSET(sem->blocked_thread_bitmask, thread_current());
                kern_threads[thread_current()].state = BLOCKED;
            }
            ENABLE_INTERRUPTS();
        };
    }
    else {
        sem->count--;
        ENABLE_INTERRUPTS();
    }
}

void
semaphore_signal(struct semaphore_t* sem) {
    int i;
    DISABLE_INTERRUPTS();

    //wake up *one* blocked thread
    for (i = 0; i < MAX_THREADS; i++) {
        if (BITTEST(sem->blocked_thread_bitmask, i)) {
            sem->blocked_thread_bitmask = BITCLEAR(sem->blocked_thread_bitmask, i);
            kern_threads[i].state = READY;
            break;
        }
    }
    sem->count++;
    ENABLE_INTERRUPTS();
}
