/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/* Contains lwthread declarations that are internal in librcd and should not
 * be used outside librcd. These functions may change in the future. */

#ifndef LWTHREADS_INTERNAL_H
#define	LWTHREADS_INTERNAL_H

#define LWT_STACK_TOP ({volatile char stack_top; (void*) &stack_top;})

/// The first reserved real time signal in Linux.
#define LWT_ASYNC_CANCEL_SIGNAL 33

/// Expression that yields the current stack limit.
/// Useful for testing if we're running in the rcd system or in a fiber.
#define LWT_READ_STACK_LIMIT ({ \
    void* stack_limit; \
    __asm__ __volatile__( \
        "movq %%fs:0x8, %0\n" \
        :"=r"(stack_limit):: \
    ); \
    stack_limit; \
})

typedef struct {
    uint64_t rdi;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
} jmp_buf_pinj;

noret void longjmp_pinj(jmp_buf_pinj* buf_pinj);

void __morestack();
void __releasestack_fx();
void __releasestack();
void __morestack_ctx_restore_fx();
void __morestack_allocate_stack_space();
void __releasestack_free_stack_space();

void lwt_setup_archaic_physical_thread();

struct lwt_physical_thread;

int32_t lwt_start_new_thread(void (*func)(void* arg_ptr), void* stack, int flags, void* arg_ptr, struct lwt_physical_thread** out_phys_thread);

fstr_t lwt_get_backtrace_archaic(fstr_t buf);

extern const size_t lwt_physical_thread_size;

#endif	/* LWTHREADS_INTERNAL_H */
