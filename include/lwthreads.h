/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef LWTHREADS_H
#define	LWTHREADS_H

#include "rcd.h"

/// Prevents unsupported TLS from compiling.
#define __thread @error_thread##__LINE__

/// Initialization for a global lwt_once_t.
#define LWT_ONCE_INIT (lwt_once_t) {0}

/// Initialization for a global lwt_once_t that causes it to already be tripped.
/// This prevents the init function from ever being executed.
#define LWT_ONCE_INIT_TRIPPED (lwt_once_t) {(UINT128_MAX - 1)}

/// Created with RCD macros, do not use.
typedef struct lwt_fiber_options {
    fstr_t name;
    /* future extensions: */
    /* size_t stack_size; */
    /* size_t guard_size; */
    /* bool disabled_stack_cleanup; */
} lwt_fiber_options_t;

typedef struct lwt_heap lwt_heap_t;

typedef struct lwt_once { rcd_fid_t v; } lwt_once_t;

typedef enum lwt_fd_event {
    lwt_fd_event_read,
    lwt_fd_event_write,
} lwt_fd_event_t;

/// Reads a random 64 bit integer from the x86_64 CPU random generator.
static inline uint64_t lwt_rdrand64() {
    uint64_t r;
    __asm__(
        "1: rdrand %%rax\n\t"
        "jnc 1b\n\t"
        : "=a" (r) // Output
        : // Input
        : "%rax" // Clobbered register
    );
    return r;
}

// Called when assertions fail.
void __assert_fail(fstr_t expr, fstr_t file, int line, fstr_t function);

/// Translates a pointer to thread local static memory.
void* lwt_get_thread_static_ptr(const void* ptr);

/// Cancels a fiber. Has no effect if the fiber is already canceled or doesn't exist.
void lwt_cancel_fiber_id(rcd_fid_t fiber_id);

/// Atomically checks if the fiber has a pending cancellation and if so,
/// throws a cancellation exception, otherwise cancels the specified fiber.
/// Used to build race free exclusive cross-cancellation relationships.
void lwt_exclusive_cancel_fiber_id(rcd_fid_t fiber_id);

/// If just did a temporary and deep stack allocation(s) this call should be used to free the now unused pages to the system.
void lwt_clean_up_stack();

/// Yields the fiber while calling waitpid().
/// The function only waits for the process to exit, it does not garbage
/// collect the zombie. It would be unsafe to do that as the wait can
/// canceled and the process id would then become unsafe to use as it could
/// refer to another process. More specifically this is caused by the
/// combination of pid recycling and lack of an asynchronous facility to wait
/// for children in Linux.
/// It returns the exit code or the signal that caused the process to exit.
int32_t lwt_waitpid(int32_t pid);

/// Yields the fiber while calling flock().
int32_t lwt_flock(int32_t fd, int32_t operation);

/// Allows global state to be initialized once with minimal overhead. Works
/// exactly like pthread_once but allows an extra argument to be specified.
/// The once struct must already be initialized before this call.
/// Modifying the once struct while it is possibly in use is undefined
/// behavior and incorrect concurrency design.
void lwt_once(lwt_once_t* once_ctrl, void (*init_fn)(void*), void* arg_ptr);

/// Parses /proc/processors and counts cores in the system
uint32_t lwt_system_cpu_count();

/// Yields the currently running fiber.
void lwt_yield();

void lwt_block_until_edge_level_io_event(int fd, lwt_fd_event_t event);
void lwt_block_until_epoll_ready(int fd, lwt_fd_event_t event);
void lwt_io_free_fd_tracking(int fd);

/// This function is overridden in librcd programs that need to be able
/// to run as an init process. Since init processes does not support
/// fundamental concepts that librcd require to function this funciton is
/// called during system init and runs in a system thread (single threaded)
/// with several features like concurrency, rio subprocesses and asynchronous
/// reading/writing disabled.
int32_t lwt_init_process(int argc, char** argv, char** env);

/// Creates a cancellation point.
void lwt_cancellation_point();

/// Tests if there is a pending cancellation without creating a cancellation point.
/// Useful for uninterruptable contexts that implements custom cancellation logic.
bool lwt_is_cancel_pending();

/// When catching join races the join race might have been intended for a
/// parent join effectively defusing it unintentionally. To prevent this when
/// catching join race exceptions this function should be called, creating
/// a reactivation point for live join races that separates obligatory cleanup
/// from logic that asserts that we got the race because of a child join race.
/// Instead of doing this immediately in a catch handler, use
/// exception_inner_join_race instead.
void lwt_test_live_join_race();

/// Immediately terminates the program with the specified status code.
void lwt_exit(int status) __attribute__((__noreturn__));

// These functions are used in RCD macros and should never be called directly.

void __rcd_escape_sh_txn(lwt_heap_t** __rcd_txn_aheap);
void __rcd_escape_gh_txn(lwt_heap_t** __rcd_global_heap_tail);
void __lwt_fiber_stack_push_sub_heap();
void __lwt_fiber_stack_pop_sub_heap(void* arg_ptr);
void __lwt_fiber_stack_push_switch_heap(lwt_heap_t* extra_fiber_heap);
void __lwt_fiber_stack_pop_switch_heap(void* arg_ptr);
rcd_fid_t __lwt_fiber_stack_push_mitosis();
rcd_fid_t __lwt_fiber_stack_pop_mitosis(void (*start_fn)(void *), void* arg_ptr, fstr_t fiber_name, const struct lwt_fiber_options* fiber_options);
void __lwt_fiber_stack_pop_mitosis_and_abort(uint8_t* _rcd_mitosis_used);
void __lwt_ifc_accept(void** fn_list, size_t n_fn_list, void* server_state, bool is_auto_reaccepting, bool is_server_side_accept);
void* __lwt_fiber_stack_push_ifc_call_join(void* ifc_fn_ptr, rcd_fid_t fiber_id, rcd_ifc_call_type_t call_type);
void __lwt_fiber_stack_pop_ifc_call_join();
void __lwt_fiber_stack_push_flip_server_heap();
void __lwt_fiber_stack_pop_flip_server_heap(void* arg_ptr);
void __lwt_fiber_stack_push_try_catch(jmp_buf* jbuf, rcd_exception_type_t exceptions_to_catch, rcd_exception_t** exception_caught_out);
void __rcd_escape_try(__rcd_try_prop_t** prop_ptr);
void __lwt_fiber_stack_push_uninterruptible();
void __lwt_fiber_stack_pop_uninterruptible(void* arg_ptr);
void __lwt_fiber_set_uninterruptible(bool new_state, bool* old_state);

/// Wraps a fiber id in a struct that garbage collects it,
/// essentially creating a sub fiber relationship with the fiber.
rcd_sub_fiber_t* lwt_wrap_sub_fiber(rcd_fid_t fiber_id);

/// Creates a copy of the exception. Useful when passing an exception object
/// (an exotic data structure) to lwt_throw_exception() or
/// lwt_throw_new_exception() while still keeping a local reference to it.
rcd_exception_t* lwt_copy_exception(rcd_exception_t* exception);

/// Creates a new exception and throws it. The ownership of the forwarded
/// exception is transfered to the exception handler which passes it to the
/// next exception handler that catches it.
__attribute__((__noreturn__))
void lwt_throw_new_exception(fstr_t message, fstr_t file, uint64_t line, rcd_exception_type_t exception_type, rcd_exception_t* fwd_exception/*, rcd_fid_t server_fid*/);

/// Throws an existing exception. Takes ownership over the exception passed to
/// it and transfers the ownership to the exception handler that catches it.
__attribute__((__noreturn__))
void lwt_throw_exception(rcd_exception_t* exception);

/// Thread local heap allocation that can be returned from a sub heap context
/// and is free'd automatically when exiting sub heap.
void* lwt_alloc_new(size_t size);

/// Like lwt_alloc_new() but automatically zeroes the memory before returning
/// it.
void* lwt_alloc_zero(size_t size);

/// See lwt_alloc_new(). Same function but has the ability to report the actual
/// size allocated that was rounded up due to alignment. Useful for allocations
/// of buffers that does not have a well defined size.
void* lwt_alloc_buffer(size_t min_size, size_t* size_out);

/// Escapes an allocation in a sub heap to the parent heap, making it survive
/// when the sub heap is unwound. Note that this call has immediate effect
/// and the memory will leak up even if the sub heap is unwound by throwing
/// an exception at a later time.
void lwt_alloc_escape(void* ptr);

/// Imports an allocation from a remote heap into the current. Make sure that
/// the remote heap is synchronized (it should be joined with the current
/// fiber), otherwise your program will have undefined behavior.
void lwt_alloc_import(void* ptr);

/// Returns the total allocation size of an allocated segment.
/// Make sure that the remote heap is synchronized (it should be joined with
/// the current fiber), otherwise your program will have undefined behavior.
size_t lwt_alloc_get_size(void* ptr);

/// Frees an allocation in the current heap or a parent heap of it.
/// Causes any associated destructors to run. Attempting to free an allocation
/// in a different heap than the current will result in a fatal error as this
/// anti-pattern indicates a synchronization error.
void lwt_alloc_free(void* ptr);

/// Allocates an alternative heap for the fiber associated with the current heap.
/// When the alternative heap is destroyed (either by catalyzing it or by
/// leaving the sub heap).
lwt_heap_t* lwt_alloc_heap();

/// Convenience function for using lwt_allocate_heap() and allocating an
/// initial object on it with specified size.
void* lwt_alloc_heaped_object(size_t size, lwt_heap_t** out_fiber_heap);

/// Allocates some memory that is destructable, as in a destructor runs before
/// the memory is free'd when leaving the sub-heap or when manually catalyzing
/// the destruction.
void* lwt_alloc_destructable(size_t size, vm_destructor_t destructor_fn);

/// Like lwt_alloc_destructable() but returns the actual final usable size
/// of the allocation in size_out.
void* lwt_alloc_buffer_destructable(size_t size, size_t* size_out, vm_destructor_t destructor_fn);

/// Imports the specified extra fiber heap to the current heap context.
lwt_heap_t* lwt_import_heap(lwt_heap_t* extra_fiber_heap);

/// Returns the exception type as a human readable string.
fstr_t lwt_get_exception_type_str(rcd_exception_type_t exception_type);

/// Dumps the exception as a human readable string.
fstr_mem_t* lwt_get_exception_dump(rcd_exception_t* exception);

/// Returns fiber id for the current fiber.
rcd_fid_t lwt_get_fiber_id();

/// Returns fiber id of the sub fiber.
rcd_fid_t lwt_get_sub_fiber_id(rcd_sub_fiber_t* sub_fiber);

/// Returns an allocated string containing the program path.
fstr_mem_t* lwt_get_program_path();

/// Returns the memory area where the argument and environment for the program is stored.
/// This is the part of the memory read when reading /proc/$pid/cmdline.
fstr_t lwt_get_program_cmdline_mem();

/// Overwrites the program cmdline.
void lwt_set_program_cmdline(fstr_t new_cmd_line);

/// Writes a full fiber dump with wait graph and join state to the specified
/// file descriptor. No scheduling is performed while writing and errors
/// are ignored. If the file descriptor blocks, so will the thread.
void lwt_write_fiber_dump_fd(int32_t write_fd);

/// Calls lwt_write_fiber_dump_fd() on STDERR.
void lwt_write_fiber_dump_debug();

#endif	/* LWTHREADS_H */
