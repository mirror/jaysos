
#include "kernel.h"
#include "interrupts.h"
#include "uart.h"
#include "msg_queue.h"
#include "ui_mgr.h"

#include "util.h"


void
start_shell();

void
start_life();

void
start_breakout();

void
start_waba();

void
start_stats();

void
asm_thread_switch(void* new_thread_stack);



//---------------------------------------------------------------------

void
start_up() {


    threads_init();

    init_uart();
    init_uimgr();

    interrupts_init();

    start_shell();

    start_waba("ImageView/ImageView");
    //start_waba(NULL);
    //start_waba("Lines/Lines");
    //start_waba("CoreTest/CoreTest");
    //start_waba("Controls/Controls");
    //start_waba("ImageSplit/ImageSplit");
    start_life();
    start_breakout();
    //start_life();
    start_stats();


    asm_thread_switch(kern_threads[0].stack);

}
