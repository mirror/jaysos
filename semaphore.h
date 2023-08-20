#ifndef SEMAPHORE_H
#define SEMAPHORE_H




struct semaphore_t {
    volatile int    count;
    u32    blocked_thread_bitmask;


};

void
semaphore_create(struct semaphore_t* sem, int initial_count);

bool
semaphore_try(struct semaphore_t* sem);

void
semaphore_wait(struct semaphore_t* sem);

void
semaphore_signal(struct semaphore_t* sem);



#endif
