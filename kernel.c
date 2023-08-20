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
#include "interrupts.h"
#include "uart.h"
#include "ui_mgr.h"
#include "util.h"

struct thread_t kern_threads[MAX_THREADS];

static int s_running_thread;

static int s_last_thread;

static struct alarm_t s_alarms[MAX_ALARMS];

static int s_last_alarm = -1;

static u32 s_idle_stack[256];


void
asm_thread_switch(void* new_thread_stack);

void
asm_thread_create(void* code_start,
                  void* stack_start,
                  void* stack_current);


void
asm_irq_handler();

void
asm_undef_handler();

/*
 * thread which never blocks, so we have something to run
 * if all the other threads are blocked
 */
static void
idle_thread() {
    while(1) {}
}


//-----------------------------------------------------------
// functions called from supervisor mode (at startup)
//-----------------------------------------------------------

void
interrupts_init(void) {
    REG_INTERRUPT = (u32)asm_irq_handler;
    REG_IE = INT_TIMER2 | INT_KEYBOARD;
    REG_KEYCNT = BIT14 | BUTTON_A | BUTTON_B | BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN |
                 BUTTON_SHOULDER_RIGHT | BUTTON_SHOULDER_LEFT | BUTTON_START;
    REG_TM2D = 1;
    REG_TM2CNT = BIT07 | BIT06;
}



void
threads_init() {
    int i;
    for(i = 0; i < MAX_THREADS; i++) {
        kern_threads[i].state = DEAD;
        kern_threads[i].name = "(dead)";
    }

    s_running_thread = 0;
    s_last_thread = 0;

    thread_create("idle", idle_thread, s_idle_stack, 256, NULL); //thread 0
}

//-----------------------------------------------------------
// functions called from user or supervisor mode
//-----------------------------------------------------------

void
thread_create(const char* name, void* start, void* stack, int stack_size, alarm_cb_t destructor) {
    int i = 0;
    DISABLE_INTERRUPTS();
    while ((i < MAX_THREADS) && (kern_threads[i].state != DEAD)) {
        i++;
    }

    if (i < MAX_THREADS) {
        kern_threads[i].state = READY;
        kern_threads[i].name = name;
        kern_threads[i].stack_end = stack;
        kern_threads[i].destructor = destructor;
        kern_threads[i].thread_local_data = NULL;

        asm_thread_create(start,
                          stack+stack_size,
                          &kern_threads[i].stack);

        if (i > s_last_thread)
            s_last_thread = i;
    }
    else panic("thread_create: out of threads");
    ENABLE_INTERRUPTS();
}

void
thread_destroy_self() {
    DISABLE_INTERRUPTS();
    kern_threads[s_running_thread].state = DEAD;
    kern_threads[s_running_thread].name = "(dead)";

    if (kern_threads[s_running_thread].destructor) {
        alarm_add(kern_threads[s_running_thread].destructor, 10,
                  &kern_threads[s_running_thread]);
    }

    ENABLE_INTERRUPTS();
    while(1) {};    /* stay here until an interrupt happens */
}


void
thread_sleep_self(u32 centisecs) {
    DISABLE_INTERRUPTS();
    kern_threads[s_running_thread].state = SLEEPING;
    kern_threads[s_running_thread].wakeup_time = kern_uptime_centisecs + centisecs;
    ENABLE_INTERRUPTS();

    /* sit and wait to get scheduled out! */
    while(1) {
        if (kern_threads[s_running_thread].state == READY) break;
    }
}

u32
thread_cpu_usage(int thread_id) {
    u32 usage = 0;
    DISABLE_INTERRUPTS();
    if ((thread_id < MAX_THREADS) && (kern_threads[thread_id].state != DEAD)) {
        usage = kern_threads[thread_id].cpu_usage;
    }
    ENABLE_INTERRUPTS();
    return usage;
}


const char*
thread_name(int thread_id) {
    const char* name = NULL;
    DISABLE_INTERRUPTS();
    if (thread_id < MAX_THREADS) {
        name = kern_threads[thread_id].name;
    }
    ENABLE_INTERRUPTS();
    return name;
}

int
thread_current() {
    return s_running_thread;
}


/*
 * the alarms need to be sorted in order of activate_time,
 * so they can be scanned quickly
 */

static void
_sort_alarms() {
    int i, j, s;
    struct alarm_t temp;

    // just a selection sort, no point doing anything fancier here!

    for (i=0; i <= s_last_alarm; i++) {

        s = i;
        for (j=i+1; j <= s_last_alarm; j++)
            if (s_alarms[j].activate_time < s_alarms[s].activate_time)
                s=j;

        if (s!=i) {

            temp = s_alarms[i];
            s_alarms[i] = s_alarms[s];
            s_alarms[s] = temp;
        }
    }
}

/*
 * schedule a callback to be called centisecs_from_now
 * "id" will be passed into the callback and
 *  can be used for application specific needs
 */

bool
alarm_add(alarm_cb_t alarm_cb, u32 centisecs_from_now, void* cb_data) {
    if (s_last_alarm > MAX_ALARMS-2)
        return FALSE;

    DISABLE_INTERRUPTS();

    s_last_alarm++;

    s_alarms[s_last_alarm].alarm_cb = alarm_cb;
    s_alarms[s_last_alarm].cb_data = cb_data;
    s_alarms[s_last_alarm].activate_time = kern_uptime_centisecs + centisecs_from_now;

    _sort_alarms();

    ENABLE_INTERRUPTS();

    return TRUE;
}

//-----------------------------------------------------------
// functions called ONLY from interrupts
//-----------------------------------------------------------

//defined in uart.c
void
uart_isr();


/*
 * called when an interrupt other than a clock interrupt happens
 * - calls the relevant handler function
 */
void
isr_dispatcher() {
    if (REG_IF & INT_COMUNICATION) {
        uart_isr();
        REG_IF = INT_COMUNICATION;
    }
    else if (REG_IF & INT_KEYBOARD){
        button_press_isr();
        REG_IF = INT_KEYBOARD;
    }
}


static u32 s_last_schedule_time = 0;


/*
 * run thru array of alarms, execute callback of any that have expired
 */
static void
check_alarms() {
    int i;
    for (i=0; i <= s_last_alarm; i++) {

        if (kern_uptime_centisecs >= s_alarms[i].activate_time) {
            s_alarms[i].alarm_cb(s_alarms[i].cb_data);
        }
        else break;
    }

    if (i!=0) {
        //we need to move the array down by i spaces
        int j;
        for (j=i; j <= s_last_alarm; j++) {
            s_alarms[j-i] = s_alarms[j];
        }
        s_last_alarm = s_last_alarm-i;

    }

}

/*
 * Find a new thread to run
 * - called after the registers of the current thread have been saved on its stack
 */
void
schedule(void* current_stack) {
    if (current_stack) {
        kern_threads[s_running_thread].stack = current_stack;
    }

    kern_threads[s_running_thread].cpu_usage += kern_uptime_centisecs - s_last_schedule_time;

    s_last_schedule_time = kern_uptime_centisecs;

    s_running_thread++;
    if (s_running_thread > s_last_thread)
        s_running_thread = 1;

    while(kern_threads[s_running_thread].state != READY) {
        /* wake up any sleeping threads */
        if (kern_threads[s_running_thread].state == SLEEPING) {
            if (kern_threads[s_running_thread].wakeup_time < kern_uptime_centisecs) {
                kern_threads[s_running_thread].state = READY;
                break;
            }
        }

        s_running_thread++;
        if (s_running_thread > s_last_thread)
            s_running_thread = 0;
    }

    /* reset timer interrupt flag */
    REG_IF = INT_TIMER2;

    check_alarms();

    /* don't worry, interrupts aren't really re-enabled until we get out of IRQ mode! */
    ENABLE_INTERRUPTS();

    /* switch to new stack, pop registers and run where we left off */
    asm_thread_switch(kern_threads[s_running_thread].stack);
}
