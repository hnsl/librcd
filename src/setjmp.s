/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

.include "rcd.s"

SEG_STACK_SYMBOL setjmp
        // rdi is jmp_buf, move registers onto it
        mov %rbx, (%rdi)
        mov %rbp, 0x08(%rdi)
        mov %r12, 0x10(%rdi)
        mov %r13, 0x18(%rdi)
        mov %r14, 0x20(%rdi)
        mov %r15, 0x28(%rdi)
        // this is our rsp WITHOUT current ret addr
        lea 0x8(%rsp), %rdx
        mov %rdx, 0x30(%rdi)
        // save return addr ptr for new rip
        mov (%rsp), %rdx
        mov %rdx, 0x38(%rdi)
        // always return 0
        xor %eax, %eax
        ret

SEG_STACK_SYMBOL longjmp
        // val will be longjmp return
        mov %rsi, %rax
        test %rax, %rax
        jnz 1f
        // if val==0, val=1 per longjmp semantics
        inc %al
        // rdi is the jmp_buf, restore regs from it
1:      mov (%rdi), %rbx
        mov 0x08(%rdi), %rbp
        mov 0x10(%rdi), %r12
        mov 0x18(%rdi), %r13
        mov 0x20(%rdi), %r14
        mov 0x28(%rdi), %r15
        // this ends up being the stack pointer
        mov 0x30(%rdi), %rdx
        mov %rdx, %rsp
        // this is the instruction pointer
        mov 0x38(%rdi), %rdx
        // goto saved address without altering rsp
        jmp *%rdx

/// setjmp for prolouge injection.
/// makes a full context switch since it's not acceptable to throw away any
/// registers in a prolouge.
/// should also run in the aproximate same speed as a normal setjmp and has the
/// advantage of not containing any branches.
/// it is always used for calling back to the system temporarily as it greatly
/// simplifies the "kernel" implementation as the "kernel" can work directly
/// with the registers, the state of the fiber, without requiring additional
/// memory and special schemes for passing information.
/// some registers are specially handled by setjmp_pinj:
/// * r11: saved but not restored.
/// * rdi: the caller is expected to set jmp_buf_pinj[0] to rdi manually (if it needs to be saved) as this saves stack space.
/// * rip: not saved but restored. the caller is expected to set this before calling longjump_pinj.
SEG_STACK_SYMBOL setjmp_pinj
        // rdi is jmp_buf_pinj, save all registers to it
        mov %rax, 0x08(%rdi)
        mov %rbx, 0x10(%rdi)
        mov %rcx, 0x18(%rdi)
        mov %rdx, 0x20(%rdi)
        mov %rbp, 0x28(%rdi)
        mov %rsi, 0x30(%rdi)
        mov %r8,  0x38(%rdi)
        mov %r9,  0x40(%rdi)
        mov %r10, 0x48(%rdi)
        mov %r11, 0x50(%rdi)
        mov %r12, 0x58(%rdi)
        mov %r13, 0x60(%rdi)
        mov %r14, 0x68(%rdi)
        mov %r15, 0x70(%rdi)
        // this is our rsp without current ret addr
        lea 0x8(%rsp), %rdx
        mov %rdx, 0x78(%rdi)
        // the instruction pointer (rip) is expected to be set manually
        ret

/// longjump for setjmp_pinj
SEG_STACK_SYMBOL longjmp_pinj
        // rdi is the jmp_buf_pinj, restore regs from it
        mov 0x08(%rdi), %rax
        mov 0x10(%rdi), %rbx
        mov 0x18(%rdi), %rcx
        mov 0x20(%rdi), %rdx
        mov 0x28(%rdi), %rbp
        mov 0x30(%rdi), %rsi
        mov 0x38(%rdi), %r8
        mov 0x40(%rdi), %r9
        mov 0x48(%rdi), %r10
        mov 0x58(%rdi), %r12
        mov 0x60(%rdi), %r13
        mov 0x68(%rdi), %r14
        mov 0x70(%rdi), %r15
        // restore stack pointer
        mov 0x78(%rdi), %r11
        mov %r11, %rsp
        // read the instruction pointer
        mov 0x80(%rdi), %r11
        // restore rdi
        mov (%rdi), %rdi
        // goto saved address without altering rsp
        jmp *%r11
