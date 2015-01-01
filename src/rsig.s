/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

.text
.global rtsig_sigcancel_exit_offset
.global rsig_sigsegv_high_handler
.global rsig_has_segv_rh

/// signal restorer trampoline
.global rsig_restore_rt
rsig_restore_rt:
    mov $15, %eax
    syscall
    hlt

/// function called by signal handler when receiving real-time signal 33 which we define to mean "cancel thread"
/// this is the declaration in c:
/// static void rsig_sigcancel_handler(int sig, siginfo_t* si, struct ucontext* uc) {
///    _exit(si->__si_fields.__rt.si_sigval.sival_int);
/// }
/// we must however implement it in assembler so the signal handler becomes "safe" - i.e. no allocation of memory, etc.
/// %rdi = int sig
/// %rsi = siginfo_t* si
.global rsig_sigcancel_handler
rsig_sigcancel_handler:
    // read si->__si_fields.__rt.si_sigval.sival_int into %edi - first argument to _exit()
    mov rtsig_sigcancel_exit_offset, %eax
    add %rax, %rsi
    movl (%rsi), %edi
    // calling _exit() with %eax = SYS_exit
    mov $60, %eax
    syscall
    hlt

/// function called by signal handler when receiving signal 11 (SIGSEGV)
/// it removes any segmented stack state for the thread so we can safely call the high level handler
/// lwt ensures that worker threads are configured with a signal stack so even if we where in the
/// middle of stacklet allocation we should be safe to use the stack
.global rsig_sigsegv_low_handler
rsig_sigsegv_low_handler:
    // save the three arguments to the high level handler
    push %rdi
    push %rsi
    push %rdx
    // skip printout if rsig_has_segv_rh is true
    movb rsig_has_segv_rh, %al
    test %al, %al
    jnz 1f
    // calling write() with %eax = SYS_write, %rdi = fd, %rsi = buf, %rdx = count
    movl $1, %eax
    movl $2, %edi
    movq $sigsegv_msg, %rsi
    movl $sigsegv_msg_len, %edx
    syscall
    // set end of stack to the special value [0] to disable it, if it was already enabled
1:  mov %fs:0x8, %r14
    xor %r11, %r11
    mov %r11, %fs:0x8
    // restore arguments
    pop %rdx
    pop %rsi
    pop %rdi
    // call the high level handler with aligned stack
    mov %rsp, %r15
    andq $-16, %rsp
    call rsig_sigsegv_high_handler
    // restore stack and end of stack
    mov %r15, %rsp
    mov %r14, %fs:0x8
    // return so the program can continue or crash the normal way
    ret

.data
sigsegv_msg: .string "CRITICAL ERROR: SEGMENTATION FAILURE\n"
sigsegv_msg_len = . -sigsegv_msg
