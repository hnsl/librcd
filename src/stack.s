/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

.include "rcd.s"
.global setjmp
.global longjmp
.global setjmp_pinj
.global __lwt_fiber_stack_pop_try_catch

.global __morestack_raw_asm
__morestack_raw_asm:
        // we are already using the thread static mega stack if the current stack limit
        // is either the magic value [0] or [UINT64_MAX] (see lwt_physical_executor_thread)
        mov %fs:0x8, %r11
        inc %r11
        and $0xfffffffffffffffe, %r11
        jnz 1f

        // already using thread static mega stack, return by simply jumping over the immediate ret instruction
        pop %r11
        inc %r11
        jmpq *%r11

        // read the physical_jmp_buf (0x98) rsp (0x30) = c8
1:      mov %fs:0xc8, %r11
        // align the new stack and add 0x100 byte padding in case the assembly function makes assumptions about red zone or argument area
        and $-16, %r11
        sub $0x108, %r11
        // flip the old stack with the new stack (%rsp and %r11)
        xchg %r11, %rsp
        // save the old stack (%r11)
        push %r11
        // save the old end of stack
        mov %fs:0x8, %r11
        push %r11
        // set end of stack to the special value [UINT64_MAX - 1]
        xor %r11, %r11
        dec %r11
        mov %r11, %fs:0x8
        // get the original return pointer
        mov 0x8(%rsp), %r11
        mov (%r11), %r11
        // call the raw asm wrapper again, skipping the immediate ret (c3) instruction
        inc %r11
        callq *%r11

.global __releasestack_raw_asm
__releasestack_raw_asm:
        // restore the old end of the stack
        pop %r11
        mov %r11, %fs:0x8
        // restore the old stack pointer and return
        mov (%rsp), %rsp
        ret

// some notes about stack usage:
// the following functions all use one qword of the stack for their call and
// another qword on the stack for calling setjmp_pinj/longjmp. the rcd kernel
// is responsible for setting the stack limit and stacklet allocation size so
// that we never encounter a situation where there are not room to allocate
// these two qwords at the top of the stack by just blindly decrementing %rsp.

/// function called by segmented stacks code generator when more stack is needed.
/// %r10 = stack size
/// %r11 = argument stack area size
.global __morestack
__morestack:
        // standard %rbp saving and setting prolouge for debugger compatibility reasons.
        // we actually store rbp again in the new stacklet and never look at this memory again but debuggers like gdb
        // doesn't understand the virtual frame in the new stacklet unless this standard prolouge exists here.
        push %rbp
        mov %rsp, %rbp
        // we need to save %rdi as the function will need it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_MORE_STACK = 4 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $4, %esi
        call longjmp

/// when a stacklet is allocated (above) a return pointer is injected into the frame
/// so this function is returned to, so the rcd kernel can release up the youngest stacklet.
.global __releasestack
__releasestack:
        // storing 0 in %r10 to indicate no fx save
        xor %r10, %r10

        // since we're about to leave a function %rdi can be thrown away as it's not enforced to be preserved over function calls by the abi.
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_LESS_STACK = 8 to the physical thread context.
        // if this is the last stacklet in the fiber we will not get longjumped back.
        lea 0x88(%rdi), %rdi
        mov $8, %esi
        call longjmp

.global __morestack_fx
__morestack_fx:
        // standard %rbp saving and setting prolouge for debugger compatibility reasons. see __morestack() for explanation.
        push %rbp
        mov %rsp, %rbp
        // we need to save %rdi as the function will need it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // we store the fx state to fxsave_mem @ %fs:0xe0 ptr
        mov %fs:0xe0, %r11
        fxsave (%r11)

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_MORE_STACK = 4 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $4, %esi
        call longjmp

.global __releasestack_fx
__releasestack_fx:
        // we store the fx state to fxsave_mem @ %fs:0xe0 ptr
        mov %fs:0xe0, %r11
        fxsave (%r11)

        // storing 1 in %r10 to indicate fx save
        xor %r10, %r10
        inc %r10

        // since we're about to leave a function %rdi can be thrown away as it's not enforced to be preserved over function calls by the abi.
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_LESS_STACK = 8 to the physical thread context.
        // if this is the last stacklet in the fiber we will not get longjumped back.
        lea 0x88(%rdi), %rdi
        mov $8, %esi
        call longjmp

.global __morestack_ctx_restore_fx
__morestack_ctx_restore_fx:
        // we restore the fx state from fxsave_mem @ %fs:0xe0 ptr
        mov %fs:0xe0, %r11
        fxrstor (%r11)
        jmp *%r10

/// function called by segmented stacks code generator to replace normal dynamic stack allocation.
/// %rdi = requested number of bytes
/// %rax = return pointer to allocation
.global __morestack_allocate_stack_space
__morestack_allocate_stack_space:
        // standard %rbp saving and setting prolouge for debugger compatibility reasons. see __morestack() for explanation.
        push %rbp
        mov %rsp, %rbp
        // %rdi contains one of our arguments and thus need to be saved so the rcd kernel can inspect it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_STACK_ALLOC = 5 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $5, %esi
        call longjmp

/// when using dynamic stack allocation (above) a return pointer is injected
/// into the frame so this function is returned to, so the rcd kernel can
/// clean up all stack allocations we made.
.global __releasestack_free_stack_space
__releasestack_free_stack_space:
        // %rdi is ignored and does not need to be saved as it is not preserved over function calls
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_STACK_FREE = 6 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $6, %esi
        call longjmp

/// used recursively by vm to break out of stacklet allocation when memory mapping to avoid deadlocks.
/// %rdi = size_t min_size
/// %rsi = size_t* size_out
/// %rax = return pointer to allocation
.global __stacklet_mmap_reserve
__stacklet_mmap_reserve:
        // %rdi contains one of our arguments and thus need to be saved so the rcd kernel can inspect it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_MMAP_RESERVE = 9 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $9, %esi
        call longjmp

/// used recursively by vm to break out of stacklet allocation when memory unmapping to avoid deadlocks.
/// %rdi = void* ptr
/// %rsi = size_t size
.global __stacklet_mmap_unreserve
__stacklet_mmap_unreserve:
        // %rdi contains one of our arguments and thus need to be saved so the rcd kernel can inspect it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_MMAP_UNRESERVE = 10 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $10, %esi
        call longjmp

/// called by functions that uses variable arguments. this call is injected by
/// the va_start() macro and allows us to adjust the pointer to the argument
/// stack which would be wrong if the call had a new stacklet injected in the
/// prolouge.
/// %rdi: va_list sruct pointer
.global __morestack_adjust_overflow_arg_area
__morestack_adjust_overflow_arg_area:
        // we are using the thread static mega stack if the current stack limit
        // is either the magic value [0] or [UINT64_MAX] (see lwt_physical_executor_thread)
        // in this case we don't need no adjustment.
        mov %fs:0x8, %r11
        inc %r11
        and $0xfffffffffffffffe, %r11
        jnz 1f
        ret

        // we store the fx state to fxsave_mem @ %fs:0xe0 ptr
1:      mov %fs:0xe0, %r11
        fxsave (%r11)

        // %rdi contains one of our arguments and thus need to be saved so the rcd kernel can inspect it
        mov %rdi, %fs:0x10
        mov %fs:0x0, %rdi
        lea 0x10(%rdi), %rdi
        call setjmp_pinj

        // longjump with buffer @ %fs:0x98 (= %rdi + 0x88) (physical_jmp_buf) and LWT_LONGJMP_VA_START = 11 to the physical thread context
        lea 0x88(%rdi), %rdi
        mov $11, %esi
        call longjmp
