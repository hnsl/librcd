/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

.include "rcd.s"
.text
.global lwt_physical_thread_size
.global __errno_location
.global memset

/// Called by Linux exec() implementation when the program starts.
.global _start
_start:
        // %rbp is undefined - we initialize it to 0 to indicate that this is the lowest frame (as also specified by the ABI)
        xor %ebp, %ebp
        // 1st arg: argc
        pop %r13
        // 2nd arg: argv
        movq %rsp, %r14
        // 3rd arg: env
        leaq 8(%rsp, %r13, 8), %r15

        // allocate physical thread
        sub lwt_physical_thread_size, %rsp
        and $-16, %rsp

        // initialize phys_thread identity (&phys_thread->identity @ 0x0)
        mov %rsp, %rsi
        mov %rsi, 0x0(%rsi)
        // initialize phys_thread end of stack (&phys_thread->end_of_stack @ 0x8) to
        // the magic value [0] that indicates that the thread static mega stack is in use
        mov %rbp, 0x8(%rsi)

        // call arch_prctl(phys_thread) to set thread context
        mov $158, %eax
        mov $0x1002, %edi
        syscall

        // we expect arch_prctl() to never fail
        test %eax, %eax
        js 1f

        // initialize the rest of the physical thread struct
        mov %rsi, %rdi
        add $0x08, %rdi
        xor %esi, %esi
        mov lwt_physical_thread_size, %rdx
        sub $0x08, %rdx

        // call memset() with aligned stack
        andq $-16, %rsp
        call memset

        // pass execution to lwthreads main and never return
        mov %r13, %rdi
        mov %r14, %rsi
        mov %r15, %rdx
        call __lwthreads_main
1:      hlt

/// This is the librcd implementation of clone() which is adapted to segmented stacks.
// int32_t _start_new_thread(void (*func)(void* arg_ptr), void* stack, int flags, void* arg_ptr, lwt_physical_thread_t** io_phys_thread);
SEG_STACK_FN_PRLG _start_new_thread, 0x80
        push %rbp
        mov %rsp, %rbp

        // save func
        push %rdi
        // save flags
        push %rdx
        // save the arg_ptr
        push %rcx

        // align child stack pointer so it's a multiple of 16
        and $-16, %rsi

        // read io_phys_thread to determine if allocation is required
        mov (%r8), %rdi
        test %rdi, %rdi
        jnz 1f

        // allocate physical thread on child stack
        sub lwt_physical_thread_size, %rsi
        and $-16, %rsi

        // return pointer to it to io_phys_thread, freeing %r8
        mov %rsi, (%r8)

        // store pointer to phys thread in %rdi
        mov %rsi, %rdi

        // save pointer to phys thread (%rdi)
1:      push %rdi

        // initialize the rest of the physical thread struct with memset()

        // rdi: dest (arg 1)
        // esi: char (arg 2)
        xor %esi, %esi
        // rdx: len (arg 3)
        mov lwt_physical_thread_size, %rdx

        // call memset() with aligned stack
        call memset

        // restore phys_thread
        pop %rax

        // restore arg_ptr
        pop %rcx
        // restore flags
        pop %rdx
        // restore func
        pop %rdi

        // initialize phys_thread identity (&phys_thread->identity @ 0x0)
        mov %rax, 0x0(%rax)
        // store the arg_ptr (%rcx) in (&phys_thread->stack_pinj_jmp_buf @ 0x10)
        mov %rcx, 0x10(%rax)


        /* The legacy glibc userland implementation is:
        int clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg),
        the kernel entry is:
        int clone (long flags, void *child_stack).

        The parameters are passed in register and on the stack (legacy clone() interface) and some are abstracted or modified in _start_new_thread:
        rdi: fn                         (passed)
        rsi: child_stack                (passed+modified)
        rdx: flags                      (passed)
        rcx: arg                        (passed)
        r8d TID field in parent         (abstracted)
        r9d: thread pointer             (abstracted)
        %esp+8: TID field in child      (abstracted)

        The kernel expects:
        rax: system call number
        rdi: flags
        rsi: child_stack
        rdx: TID field in parent
        r10: TID field in child
        r8: thread pointer */

        // saving callback function (func arg) in %r9 for the child process
        mov %rdi, %r9

        // storing flags in arg 1 (%rdi)
        mov %rdx, %rdi
        // align the stack entry point and store in arg 2 (%rsi)
        mov %rax, %rsi
        and $-16, %rsi
        // storing ptid in arg 3 (%rdx) (&phys_thread->linux_tid @ 0xd8)
        lea 0xd8(%rax), %rdx
        // storing ctid in arg 4 (%r10) (&phys_thread->linux_tid @ 0xd8)
        mov %rdx, %r10
        // storing tls in arg 5 (%r8) (&phys_thread)
        mov %rax, %r8

        // call clone()
        mov $56,  %eax
        syscall

        // if clone() return 0 we're executing in the new thread or else we return the syscall return code
        test %eax, %eax
        jz 3f

        // return from _start_new_thread with normal function epilogue
        pop %rbp
        ret

        // we're executing in the new thread
        // we initialize %rbp to 0 to indicate that this is the lowest frame (as also specified by the ABI)
3:      xor %ebp, %ebp
        // fetch the argument pointer from (&phys_thread->stack_pinj_jmp_buf @ 0x10)
        mov %fs:0x10, %rdi
        // align stack pointer so it's a multiple of 16
        andq $-16, %rsp

        // call the main function with the arg_ptr as its first argument
        call *%r9

        // call exit() to terminate the thread
        mov %eax, %edi
        mov $60, %eax
        syscall
        hlt
