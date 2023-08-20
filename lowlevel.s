
@ All the assembly code used by JaysOS lives here: 
@ the crt0 stuff and the context switching code


@    Copyright (c) Justin Armstrong 2002. All Rights Reserved.
@
@    Permission is hereby granted, free of charge, to any person obtaining
@    a copy of this software and associated documentation files (the
@    "Software"), to deal in the Software without restriction, including
@    without limitation the rights to use, copy, modify, merge, publish,
@    distribute, sublicense, and/or sell copies of the Software, and to
@    permit persons to whom the Software is furnished to do so, subject to
@    the following conditions:
@
@    The above copyright notice and this permission notice shall be included
@    in all copies or substantial portions of the Software.
@
@    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
@    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
@    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
@    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
@    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
@    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
@    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


.extern __bss_start__, __bss_end__, _etext, _sdata
.extern __bss_start, __end__, _edata, _end, _stack, _etext


IWRAM_BASE = 0x03000000     @ end of IWRAM is 0x3008000

Cpu_Mode_USR = 0x10
Cpu_Mode_IRQ = 0x12
Cpu_Mode_SVC = 0x13

Cpu_Stack_IRQ = 0x03007500
Cpu_Stack_SVC = 0x03007A00
  

INTERRUPT_VECTOR = 0x03007FFC

REG_TM2D = 0x4000108

REG_IF = 0x4000202


.globl _start

.globl asm_thread_create
.globl asm_thread_switch
.globl asm_irq_handler
.globl asm_undef_handler

.globl kern_ticks
.globl kern_uptime_centisecs

.globl setjmp
.globl longjmp


.data
        .align  2

kern_ticks:  .word   0

kern_uptime_centisecs:    
        .word 0


.globl done
.text

_start:	b	reset
.include "header.s"

reset:
	mov	r0, #(Cpu_Mode_IRQ | 0x80 | 0x40)
	msr	CPSR_c, r0
	ldr	sp, =Cpu_Stack_IRQ

	mov	r0, #(Cpu_Mode_SVC | 0x80 | 0x40)
	msr	CPSR_c, r0
	ldr	sp, =Cpu_Stack_SVC

	@@@ Code to copy DATA into RAM. 
	ldr	r0, =_sdata			
        mov	r1, #IWRAM_BASE	
	ldr	r3, =_edata
           	
.copy:	cmp	r1, r3
	ldrcc	r2, [r0], #4
	strcc	r2, [r1], #4
	bcc	.copy


.zero:	
        ldr     r3, =_end	
	mov	r2, #0
               
.znxt:	cmp	r1, r3
	strcc	r2, [r1], #4
	bcc	.znxt
   
        
ready_to_begin:        

	bl	start_up

@ on return
done:	
        swi	#3	@ gba sys comand - halt
	bal	done	@ on return

@ -------------------------------------------------------------------------
asm_thread_create:
        @ r0 = code_start
        @ r1 = stack_start
        @ r2 = stack_current
    

        @ we need to put some values on the stack for this new thread
        @ when it starts running it will pop these into its registers


        @ the first register going on the stack is the pc, 
        @ which is the start function address that was passed in in r0
        
        add     r3, r0, #4      @ this will be the pc
        stmfd   r1!, {r3}    

        @ the next values on the stack are r12, r3, r2, r1, r0
        @ these should all have zeroes in them
        
        mov     r3, #0

        stmfd   r1!, {r3}   @ r12
        stmfd   r1!, {r3}   @ r3
        stmfd   r1!, {r3}   @ r2
        stmfd   r1!, {r3}   @ r1
        stmfd   r1!, {r3}   @ r0

            
        @ the next registers are r11-r4
        @ all zeroes here too

        mov     r3, #0

        stmfd   r1!, {r3}   @ r11
        stmfd   r1!, {r3}   @ r10
        stmfd   r1!, {r3}   @ r9
        stmfd   r1!, {r3}   @ r8   
        stmfd   r1!, {r3}   @ r7
        stmfd   r1!, {r3}   @ r6
        stmfd   r1!, {r3}   @ r5
        stmfd   r1!, {r3}   @ r4


        @ next, store the initial lr
        @ we point this at thread_destroy_self so that if this function returns
        @ the thread will be deleted
            
        ldr     r3, =thread_destroy_self
        stmfd   r1!, {r3}   
        
        @ store a reasonable cpsr value
        
        mov	r3, #(Cpu_Mode_USR | 0x40)
        stmfd   r1!, {r3}

        @ finally, the thread's stack_current 

        str     r1, [r2]

        @ all done!

        sub pc, lr, #0


@ -------------------------------------------------------------------------


@ the entry point for all interrupts        
asm_irq_handler:
   
        ldr     r0, =REG_IF
        ldrh    r1, [r0]
        cmp     r1, #0x20      @ timer 2
        beq     got_timer_irq
    
        bl      isr_dispatcher
    
        ldmfd   sp!, {r0, r1, r2, r3, r12, lr}
        subs    pc, lr, #4 

@ clock tick happens approx 256 times a second        
got_timer_irq:

        strh    r1, [r0]
        ldr     r0, =kern_ticks
        ldr     r1, [r0]

        cmp     r1, #2
        bgt     inc_centisecs
                            
        add     r1, r1, #1  @ increment ticks counter
        str     r1, [r0]
        
        ldmfd   sp!, {r0, r1, r2, r3, r12, lr}
        subs    pc, lr, #4 
        
inc_centisecs:
        mov     r1, #0      @ clear ticks counter
        str     r1, [r0]
        ldr     r0, =kern_uptime_centisecs
        ldr     r1, [r0]
        add     r1, r1, #1
        str     r1, [r0]
        
@ we do a context switch every centisecond too..


@ save registers on stack of current thread
asm_irq_save_thread:        


        stmfd  sp, {sp, lr}^
        sub    sp, sp, #8       @ because we can't do writeback (!) for some reason
        
        ldmfd  sp!, {r0, r1}
        nop
        sub     r0, r0, #4      
        sub     r2, r0, #24

        @ copy 6 words from the irq stack to the user stack
        
copy_stk:
        ldmfd   sp!, {r3}
        nop

        stmfa   r2!, {r3}
        cmp     r0, r2
        bne     copy_stk


stk_done:
        sub     r0, r0, #20     @ to point r0 back at the end of the user stack
                
        stmfd   r0!,  {r4-r11}  @ push r4-r11 onto the user stack
        stmfd   r0!, {r1}       @ push the user lr onto the user stack
               
        mrs     r1, spsr
        stmfd   r0!, {r1}

ready_for_new_task:

        @ passsing in r0 as the current thread stack
        b  schedule

@ ---------------------------------------------------

@ called from schedule() after a new thread has been picked
@ pops the stored registers off its stack
@ and starts running where the thread left off

asm_thread_switch:
    @ r0 = new thread stack
        
        mov     sp, r0

changed_sp:

        ldmfd   sp!, {r1}
        nop
        msr     spsr, r1

        ldmfd   sp, {lr}^         @ load the stacked lr into lr_usr
        nop
        add     sp, sp, #4
      
        ldmfd   sp!, {r4-r11}
        nop
        ldmfd   sp!, {r0-r3, r12, lr}       
        nop
        @ lr (the irq lr - not lr_usr!) now contains the pc we are going to resume this thread at

        @ get the current sp into sp_usr
        stmfd   sp, {sp}
        sub     sp, sp, #4
        ldmfd   sp, {sp}^       @ load the stacked sp into sp_usr
        nop     

@ now get back the irq sp  (note this doesn't affect sp_usr)


        ldr     sp, =Cpu_Stack_IRQ
        
ready_to_return:

        subs pc, lr, #4
        
@ we should now be running the new thread!


@ -------------------------------------------------------------------------


.L6:
        .align  2





