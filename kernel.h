#ifndef KERNEL_H
#define KERNEL_H

#include "gba.h"

#ifndef bool
typedef int bool;
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

enum state_t { READY, BLOCKED, SLEEPING, DEAD };


#define MAX_THREADS 32


#define DEFAULT_STACK_SIZE 2048

#define BITSET(arg, pos) ((arg) | (1L << (pos)))
#define BITCLEAR(arg, pos) ((arg) & ~(1L << (pos)))
#define BITTEST(arg, pos) ((arg) & (1L << (pos)))


typedef void (*alarm_cb_t)(void*);

struct alarm_t {
    alarm_cb_t alarm_cb;
    u32 activate_time;
    void* cb_data;

};

#define MAX_ALARMS 32


struct thread_t {
    const char*  name;
    void* stack;    //current stack pos
    void* stack_end;
    volatile enum state_t state;

    u32 wakeup_time;    //in centiseconds since uptime
    u32 cpu_usage;      //centiseconds spent running this thread
    
    void*   thread_local_data;

    alarm_cb_t destructor;

};

extern struct thread_t kern_threads[MAX_THREADS];

extern volatile u32 kern_uptime_centisecs;


#define DISABLE_INTERRUPTS()  REG_IME = 0;
#define ENABLE_INTERRUPTS()   REG_IME = 1;

void
interrupts_init(void);

void
threads_init();

void
thread_create(const char* name, void* start, void* stack, int stack_size, alarm_cb_t destructor);


u32
thread_cpu_usage(int thread_id);

int
thread_current();

const char*
thread_name(int thread_id);

struct thread_t*
get_thread_by_name(const char* name);

void
thread_destroy_self();

void
thread_sleep_self(u32 centisecs);


bool
alarm_add(alarm_cb_t alarm_cb, u32 centisecs_from_now, void* cb_data);


#endif
