/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "vm-internal.h"
#include "rsig-internal.h"
#include "lwthreads-internal.h"

#include "linux.h"
#include "atomic.h"
#include "hmap.h"

#pragma librcd

#define LWT_LONGJMP_DIRECT 0
#define LWT_LONGJMP_SWITCH 1
#define LWT_LONGJMP_TEARDOWN 2
#define LWT_LONGJMP_YIELD 3
#define LWT_LONGJMP_MORE_STACK 4
#define LWT_LONGJMP_STACK_ALLOC 5
#define LWT_LONGJMP_STACK_FREE 6
#define LWT_LONGJMP_THROW_UNWIND_STACK 7
#define LWT_LONGJMP_LESS_STACK 8
#define LWT_LONGJMP_MMAP_RESERVE 9
#define LWT_LONGJMP_MMAP_UNRESERVE 10
#define LWT_LONGJMP_VA_START 11
#define LWT_LONGJMP_PANIC 12

#define LWT_INIT_PHYSICAL_MUTEX 1
#define LWT_INIT_PHYSICAL_COND 0
#define LWT_INIT_PHYSICAL_RWLOCK 0

#define LWT_PHYS_THREAD (lwt_get_physical_thread_from_fsreg())

#define LWT_GET_LOCAL_FIBER(fiber) \
    struct lwt_fiber* fiber = LWT_PHYS_THREAD->current_fiber; \
    assert(fiber != 0);

#define LWT_INIT_FIBER_ID 1

#define LWT_SYNC_ERROR_MSG "synchronization error"

#define LWT_SYS_SPINLOCK_RLOCK(rwspinlock) { \
    bool _rlock = true; \
    rwspinlock_t* _prev_system_rwspinlock; \
    lwt_sys_spinlock_rlock(rwspinlock, &_prev_system_rwspinlock); \
    do {

#define LWT_SYS_SPINLOCK_WLOCK(rwspinlock) { \
    bool _rlock = false; \
    rwspinlock_t* _prev_system_rwspinlock; \
    lwt_sys_spinlock_wlock(rwspinlock, &_prev_system_rwspinlock); \
    do {

#define LWT_SYS_SPINLOCK_UNLOCK(rwspinlock) \
    } while (false); \
    lwt_sys_spinlock_unlock(rwspinlock, &_prev_system_rwspinlock, _rlock); \
}

/*#define LWT_FIBER_FLAG_DONE             0x01
#define LWT_FIBER_FLAG_UNINTR           0x02
#define LWT_FIBER_FLAG_CANCELED         0x04
#define LWT_FIBER_FLAG_JOIN_RACE        0x08
#define LWT_FIBER_FLAG_DEFERRED         0x10*/

/// The inside of a va_list struct according to the linux x86_64 abi.
typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void* overflow_arg_area;
    void* reg_save_area;
} lwt_abi_va_list;

struct lwt_heap {
    vm_heap_t* vm_heap;
};

/// Maps file descriptors to the corresponding fibers that wait
/// for them. Only one fiber can wait for either read or write for a fd at the
/// same time - more does not make sense and will trigger an IO exception.
typedef struct lwt_blocking_fd {
    // True if a read event was raised for the file descriptor but no fiber was registered to handle it. Mutualy exclusive with read_fiber_ctrl != 0.
    bool read_ready;
    /// Control struct of fiber that are waiting for read.
    struct lwt_fiber* read_fiber;
    // True if a write event was raised for the file descriptor but no fiber was registered to handle it. Mutualy exclusive with write_fiber_ctrl != 0.
    bool write_ready;
    /// Control struct of fiber that are waiting for write.
    struct lwt_fiber* write_fiber;
    /// If the fd is epoll it must be polled on because epoll in epoll works edgetriggered.
    bool is_epoll;
} lwt_blocking_fd_t;

typedef enum lwt_fiber_event_type {
    /// Switches to an existing side-heap. Used for storing complex data
    /// structures and doing memory optimizations.
    lwt_fiber_event_sub_stack,
    /// Creates a vm sub heap and switching to it. Allocations can be
    /// specified to survive when popping.
    /// Note: This event has no related data as the vm references the parent
    /// context internally.
    lwt_fiber_event_sub_heap,
    /// Switches to an existing side-heap. Used for storing complex data
    /// structures and doing memory optimizations.
    lwt_fiber_event_switch_heap,
    /// Creates a new vm heap that will specifically be used to perform mitosis
    /// and switches to it. Done as a last step when creating a new fiber.
    lwt_fiber_event_mitosis,
    /// Used inter fiber communication (ifc) to join with a server fiber and it's state.
    lwt_fiber_event_ifc_join,
    /// Flips active heap from server to client or vice versa.
    lwt_fiber_event_flip_server_heap,
    /// Registered handler that catches certain exceptions. Associated with a setjmp structure for
    /// long-jumping back to the thread contents.
    lwt_fiber_event_try_catch,
    /// The fiber entered an uninterruptable section where it's immune against cancellations and join races.
    lwt_fiber_event_unintr,
    /// The fiber was deferred while waiting for I/O. When the fiber is woken
    /// up again the associated jmpbuf should be used to long jump back to
    /// thread execution.
    lwt_fiber_event_deferred,
    /// TODO: Add fiber event for plain co-routine calling.
} lwt_fiber_event_type_t;

typedef struct lwt_edata_switch_heap {
    vm_heap_t* prev_heap;
} lwt_edata_switch_heap_t;

typedef struct lwt_edata_mitosis {
    struct lwt_fiber* new_fiber;
    vm_heap_t* post_mitosis_heap;
} lwt_edata_mitosis_t;

typedef struct lwt_edata_ifc_join {
    struct lwt_ifc_client* ifc_client;
    /// Used by heap switch: call type.
    rcd_ifc_call_type_t call_type;
    /// Used by heap switch: previous ifc call join event in client fiber event stack.
    struct lwt_edata_ifc_join* prev;
} lwt_edata_ifc_join_t;

typedef struct lwt_edata_try_catch {
    jmp_buf* jbuf;
    /// The top stack allocation at the jbuf context for restoring the stack allocation state.
    struct lwt_stack_alloc* stack_alloc_top;
    rcd_exception_type_t exceptions_to_catch;
    rcd_exception_t* volatile* exception_caught_out;
} lwt_edata_try_catch_t;

typedef struct lwt_edata_unintr {
    bool prev_unintr;
} lwt_edata_unintr_t;

typedef struct lwt_edata_deferred {
    jmp_buf* jbuf;
} lwt_edata_deferred_t;

typedef union lwt_fiber_event_data {
    lwt_edata_switch_heap_t switch_heap;
    lwt_edata_mitosis_t* mitosis;
    lwt_edata_ifc_join_t* ifc_call_join;
    lwt_edata_try_catch_t* try_catch;
    lwt_edata_unintr_t unintr;
    lwt_edata_deferred_t deferred;
} lwt_fiber_event_data_t;

/// Fibers go through a number of events that changes the state of the thread.
/// These events are connected to the thread stack and used to unwind it to
/// a previous state, either during exceptions or simply due to popping the
/// event from the stack.
typedef struct lwt_fiber_event {
    lwt_fiber_event_type_t type;
    lwt_fiber_event_data_t data;
    struct lwt_fiber_event* next;
} lwt_fiber_event_t;

typedef struct exec_blocked_fiber {
    struct exec_blocked_fiber* next;
} exec_blocked_fiber_t;

typedef struct lwt_ifc_client {
    struct lwt_fiber* fiber;
    /// Global call id so ifc calls can be ordered across different functions queues.
    uint128_t call_id;
    /// If the client is waiting for the server this is set to the server state
    /// before it's woken up to indicate that it's now running.
    void* server_state;
    /// True if wait was aborted because the target fiber exited before
    /// accepting the request or if already executing true if there is a
    /// pending join race (when the server is waiting for client to cancel).
    bool live_join_race;
    // When the client is running, this is the related ifc server fiber.
    struct lwt_ifc_server* ifc_server;
    rcd_ifc_call_type_t call_type;
    struct lwt_ifc_client* prev;
    struct lwt_ifc_client* next;
} lwt_ifc_client_t;

/// Struct which only exists on the server stack as long as the server is either accepting clients or waiting for clients to complete.
typedef struct lwt_ifc_server {
    /// The fiber with the accept context. Might be a different server than what fiber clients join with the joined server accepts instead of the client.
    struct lwt_fiber* fiber;
    /// The server fiber heap. Used when flipping between client and server heap.
    /// Cannot touch fiber->current_heap as that reference is owned by the server.
    vm_heap_t* heap;
    /// Double linked list of running clients. Canceled if server is canceled. Server waits for the clients here to be canceled first (server_ref cannot be set to false).
    struct lwt_ifc_client* running_clients;
    /// If server is listening for clients, this server state is not 0. If accepting a locked request this state is set to zero, otherwise it's untouched.
    void* server_state;
    /// If the above server state is not 0 and this is true the above server state is read locked and accepting shared requests only.
    bool read_locked_state;
} lwt_ifc_server_t;

/// Reference to an ifc server from an ifc function queue.
typedef struct lwt_ifc_server_ref {
    lwt_ifc_server_t* server;
    /// Head of linked list ifc server ref attached, either a pointer to lwt_ifc_fn_queue->freeish_servers, lwt_ifc_fn_queue->sharedish_servers or 0.
    struct lwt_ifc_server_ref** head;
    struct lwt_ifc_server_ref* prev;
    struct lwt_ifc_server_ref* next;
} lwt_ifc_server_ref_t;

typedef struct lwt_ifc_fn_queue {
    /// Pointer to the related function clients call and servers accept on.
    void* fn_ptr;
    /// Count of ifc servers that reference this object.
    size_t ifc_server_ref_count;
    /// Linked list of all ifc servers that has a non zero server_state and possibly other ifc servers that has not been dereferenced yet.
    lwt_ifc_server_ref_t* freeish_servers;
    /// Linked list of all ifc servers that has a non zero server_state and possibly other ifc servers that has not been dereferenced yet.
    lwt_ifc_server_ref_t* sharedish_servers;
    /// Non zero if still indexed in this server fiber. When server fiber
    /// is teared down this is set to null to indicate that the server is dead
    /// and the struct no longer requires unindexing in the server_fiber->ifc_fn_queues
    struct lwt_fiber* server_fiber;
    /// Queue of waiting clients. Canceled if server is canceled. Server does not wait for the clients here to be canceled first.
    struct {
        struct lwt_ifc_client* first;
        struct lwt_ifc_client* last;
    } waiting_clients;
    /// Indexed in server_fiber->ifc_fn_queues if server_fiber is still alive.
    rbtree_node_t rb_node;
} lwt_ifc_fn_queue_t;

typedef struct lwt_start_args {
    void (*start_fn)(void *arg_ptr);
    void* arg_ptr;
} lwt_start_cb_t;

typedef struct lwt_stack_alloc {
    /// For non root stack allocations this is 0 and for root stack allocations this is a pointer to the frame we have redirected.
    uint64_t* redirected_frame_ptr;
    /// Pointer to next stacked stack allocation down in stack.
    struct lwt_stack_alloc* next;
    /// Total length of the stack allocation, including this header.
    size_t len;
    uint64_t mem[];
} lwt_stack_alloc_t;

typedef struct lwt_stacklet {
    /// Pointer to next stacked stacklet down in stack.
    struct lwt_stacklet* next;
    /// Total final length of the stacklet, including this header.
    size_t len;
    // The rsp in the older stacklet context.
    uint64_t* older_rsp;
    // Raw stacklet memory.
    uint64_t mem[];
} lwt_stacklet_t;

typedef struct lwt_fiber {
    struct {
        /// Fibers are indexed by id in lwt_all_fibers.
        rcd_fid_t id;
        /// An exception object created by the fiber that is the source of the cancel or null if not canceled.
        rcd_exception_t* canceled;
        /// True if the fiber has a hidden pending cancellation and are not interested in new cancellations as it is waiting for another event to occur first.
        bool hidden_cancel;
        /// True if what the fiber is currently waiting for is complete, like other fibers or I/O. Mutually exclusive with "deferred".
        /// This is simply a pending wake up signal that prevents the fiber from becoming deferred if the party that it's waiting for is completed before it has a chance to sleep.
        /// It does not need to be set if already deferred when waking up and does not flag for any specific data allocations or return values.
        bool done;
        /// True if the fiber has entered a block where it cannot be interrupted by cancellation or join race.
        bool unintr;
        /// Not null if the client has a pending join race. Mutually exclusive with "deferred".
        bool join_race;
        /// True if the fiber is deferred.
        /// The thread which sets this to true is the physical thread and ensures that "canceled" is not true while it's
        /// doing so in a transaction as "deferred" and "canceled" can never be true at the same time.
        /// The thread which sets this to false is responsible for enqueueing it again in the exec blocked fibers queue.
        bool deferred;
        /// True if the fiber is started.
        /// If this field is false "event_stack" is the pointer to the function that should be started and "current_ifc_join_event" is the pointer to its argument.
        bool started;
    } ctrl;
    /// Fiber local errno.
    int32_t local_errno;
    /// Name of fiber main. (e.g. generic function name of main function)
    fstr_t main_name;
    /// Estimated maximum stack size that is worth pre-allocating for performance.
    /// This is determined in post-link time by analyzing the complete call graph of the program.
    size_t est_stack_size;
    /// Name of fiber instance. (e.g. ID of related object)
    fstr_t instance_name;
    /// Stack of dynamic stack allocations.
    lwt_stack_alloc_t* stack_alloc_stack;
    /// The current stacklet.
    lwt_stacklet_t* current_stacklet;
    /// The current fiber heap.
    vm_heap_t* current_heap;
    /// started == true: Currently stacked events. started == false: Pointer to a callback function.
    lwt_fiber_event_t* event_stack;
    /// started == true: Current ifc call join event in the top of the event stack. started == false: Argument to the function that should be started.
    lwt_edata_ifc_join_t* current_ifc_join_event;
    /// Static memory used for enqueuing the fiber as execution blocked.
    exec_blocked_fiber_t exec_blocked;
    /// ifc fn queues for this server.
    rbtree_t ifc_fn_queues;
    /// If the fiber is deferred accepting, this is its ifc server.
    lwt_ifc_server_t* defer_ifc_server;
    /// If the fiber is deferred and defer_ifc_server is 0, this is the specific fiber it is waiting for or 0.
    /// If the fiber is deferred and defer_ifc_server is set, this is the server fiber it is accepting on.
    rcd_fid_t defer_wait_fid;
    /// If the fiber is deferred, this is one of the waiting file descriptors or -1.
    int32_t defer_wait_fd;
    /// Previous fiber in the linked list. (shared_fiber_mem.fiber_list)
    struct lwt_fiber* prev;
    /// Next fiber in the linked list. (shared_fiber_mem.fiber_list)
    struct lwt_fiber* next;
} lwt_fiber_t;

/// Memory used by Intel to Save x87 FPU, MMX Technology, and SSE State
typedef struct lwt_fxsave_mem {
    uint8_t _[512];
} lwt_fxsave_mem_t;

typedef struct lwt_physical_thread {
    /// %fs:0x0 - The thread identity. Pointer to the struct itself. The address of this field is the same as the address of the struct.
    struct lwt_physical_thread* identity;
    /// %fs:0x8
    /// - The current end of stack for the thread frame. (Segmented stacks implementation.)
    /// - Is zero in physical threads as they use the thread static mega stack.
    /// - Has the special value [UINT64_MAX - 1] when a new fiber is started and has no stack yet.
    /// - Has the special value [UINT64_MAX] when the thread static mega stack is used temporarily while a raw assembly function is running.
    ///   In this state the running fiber is not allowed to call back to high level code.
    ///   Raw assembly functions pay the price of assuming that there is infinite stack by being required to be leaf functions.
    ///   Declared volatile to prevent compiler from messing with instruction order and loads/stores of this variable which are huge side effects.
    void* volatile end_of_stack;
    /// %fs:0x10 - The jump buffer for jumping back to the morestack context.
    jmp_buf_pinj stack_pinj_jmp_buf;
    /// %fs:0x98 - The jump buffer for returning to the physical context.
    jmp_buf physical_jmp_buf;
    /// %fs:0xd8 - The physical thread identifier set by linux (pointer passed to CLONE_PARENT_SETTID).
    int64_t linux_tid;
    /// %fs:0xe0 - Pointer to fxsave memory for context switching from fibers and back without loosing the floating point and other special register state.
    lwt_fxsave_mem_t* fxsave_mem;
    /// The pid returned from clone.
    int32_t pid;
    /// When unwinding try/catch: The jump buffer of the block.
    jmp_buf* try_jmp_buf;
    /// When unwinding try/catch: The top stack allocation at the block.
    struct lwt_stack_alloc* try_stack_alloc_top;
    /// The currently held system rwspinlock. Needs to be registered in physical thread so non fatal exceptions can be upgraded to fatal.
    rwspinlock_t* system_rwspinlock;
    /// The system fiber used by the physical thread when not executing another fiber.
    lwt_fiber_t system_fiber;
    /// The context the physical thread is currently running. Is never 0, instead the system fiber is used when not executing a real fiber.
    lwt_fiber_t* current_fiber;
    /// Thread static heap.
    struct lwt_heap thread_static_heap;
    /// Thread static memory segment. Initially a copy of the librcd_thread_static_memory section.
    void* thread_static_memory;
    /// Main function for the fiber we are currently starting.
    void (*main_fn_ptr)(void*);
} lwt_physical_thread_t;

// This constant is used by _start to allocate an initial physical thread struct.
const size_t lwt_physical_thread_size = sizeof(lwt_physical_thread_t);

typedef struct lwt_executor_thread {
    lwt_physical_thread_t* phys_thread;
    struct lwt_executor_thread* next;
} lwt_executor_thread_t;

struct rcd_sub_fiber {
    rcd_fid_t fiber_id;
};

static const uint128_t lwt_once_status_init = 0;
static const uint128_t lwt_once_status_spin = (UINT128_MAX);
static const uint128_t lwt_once_status_done = (UINT128_MAX - 1);

// Hash map types declarations that map a fiber id to a lwt_fiber_t*.
HMAP_DEFINE_TYPE(fid, rcd_fid_t, lwt_fiber_t*, false, 1000, 0.5, false);

// Hash map types declarations that map a fd to a lwt_blocking_fd_t*.
HMAP_DEFINE_TYPE(bfd, int32_t, lwt_blocking_fd_t*, false, 1000, 0.5, false);

// Physical LWT threads.
lwt_executor_thread_t* lwt_executor_threads = 0;
size_t lwt_executor_thread_count = 0;
int8_t lwt_executor_match_lock = 0;

/// The global heap and synchronization to access it.
static struct {
    vm_heap_t* heap;
    int32_t rwlock;
} lwt_global_heap = {0};

/// Memory for shared fiber control and ifc.
static struct {
    int32_t rwlock;
    /// Hash map of all fiber control structs indexed by fiber->id.
    hmap_fid_t fiber_map;
    /// Linked list of all fibers.
    lwt_fiber_t* fiber_list;
    /// Queue of execution blocked fibers that await execution by a physical thread.
    struct {
        exec_blocked_fiber_t* first;
        exec_blocked_fiber_t* last;
    } exec_block_queue;
    /// Futex that is incremented every time the blocked fibers list changes and used to yield the cpu to the kernel when no fiber requires execution by futex(2).
    uint32_t exec_block_futex;
    // If this is true only one executor is allowed and the rest should block until it's disabled. Used for debugging purposes where multiple threads mess with the debugger.
    bool debug_choke_enabled;
    // Number of threads that is stuck waiting for debug_choke_enabled to change to false. Should never be higher than lwt_executor_thread_count - 1.
    uint64_t debug_choke_count;
    // Futex that is incremented every time debug_choke_enabled changes.
    uint32_t debug_choke_futex;
    /// This ifc call id is incremented every time a new ifc call is made so ifc calls can be ordered across different functions queues.
    uint128_t ifc_call_id;
} shared_fiber_mem = {0};

/// Free list allocator for lwt_ifc_fn_queue_t structs. Is externally synchronized with shared_fiber_mem.rwlock.
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_ifc_fn_queue_t, lwt_ifc_fn_queue_allocate, lwt_ifc_fn_queue_free, false);

static struct {
    int32_t rwlock;
    /// Hash map of all fiber control structs indexed by blocking_fd->fd.
    hmap_bfd_t blocking_fd_map;
} shared_bfd_mem = {0};

/// Free list allocator for main fiber structs.
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_fiber_t, lwt_fiber_allocate, lwt_fiber_free, true);

/// Free list allocator for lwt_blocking_fd_t structs. Is externally synchronized with shared_bfd_mem.rwlock.
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_blocking_fd_t, lwt_blocking_fd_allocate, lwt_blocking_fd_free, false);

/// First argument passed to program containing the program path.
fstr_mem_t* lwt_program_path;

/// Memory used for storing program arguments and environment.
fstr_t lwt_program_cmdline_mem;

/// Shared file descriptor for epoll operations.
int lwt_shared_epoll_fd;

/// Limit for worker count that is set when debugging.
volatile uint64_t lwt_debug_max_worker_count = UINT64_MAX;

/// 40 pages of memory that can be used by debuggers as a dummy stack. This fixes a problem where gdb would blow the stack in stacklets when injecting dummy frames.
/// If this memory is placed below the previous stacklet (the expected case) we will get a real stacklet as soon as the first function is called as we are apparently out of stack.
/// If this memory is placed above the previous stacklet we will not run out of stack because its large like a mega stack.
volatile uint8_t __debug_call_dummy_stack[40 * PAGE_SIZE]
__attribute__((aligned(PAGE_SIZE), unused));

#ifdef RCD_SELF_TEST
void rcd_primitive_self_test();
void rcd_advanced_self_test(list(fstr_t)* main_args, list(fstr_t)* main_env);
#endif

/// Defined in _start.s. Replaces clone().
int32_t _start_new_thread(void (*func)(void* arg_ptr), void* stack, int flags, void* arg_ptr, lwt_physical_thread_t** io_phys_thread);

long __syscall_ret(unsigned long);

/// Stdio compatibility layer constructor.
void stdio_init();

static void lwt_signal_init();

static list(void*)* lwt_get_backtrace_for_frame_ptr(void* current_frame_ptr, uint32_t skip_n);

static list(void*)* lwt_get_backtrace();

static void lwt_fatal_exception_handler(rcd_exception_t* exception);

static int32_t lwt_sigtkill_thread(int32_t tid, int32_t signal, union sigval value) {
    int32_t pid = getpid();
    siginfo_t uinfo = {
        .si_signo = signal,
        .si_code = SI_QUEUE,
        .__si_fields.__rt.si_pid = pid,
        .__si_fields.__rt.si_uid = getuid(),
        .__si_fields.__rt.si_sigval = value,
    };
    return rt_tgsigqueueinfo(pid, tid, signal, &uinfo);
}

static int lwt_cmp_ifc_fn_queues(const rbtree_node_t* node1, const rbtree_node_t* node2) {
    lwt_ifc_fn_queue_t* ifc_fn_queue1 = RBTREE_NODE2ELEM(lwt_ifc_fn_queue_t, rb_node, node1);
    lwt_ifc_fn_queue_t* ifc_fn_queue2 = RBTREE_NODE2ELEM(lwt_ifc_fn_queue_t, rb_node, node2);
    return RBTREE_CMP_V(ifc_fn_queue1->fn_ptr, ifc_fn_queue2->fn_ptr, 0);
}

static void lwt_fiber_heap_destructor(void* arg_ptr) {
    lwt_heap_t* fiber_heap = arg_ptr;
    if (fiber_heap->vm_heap != 0) {
        vm_heap_t* parent_heap = vm_heap_release(fiber_heap->vm_heap, 0, 0);
        assert(parent_heap == 0);
    }
}

/// Constructs an alternative lwt heap handle that, when destroyed, releases the alternative heap associated with it.
static lwt_heap_t* lwt_construct_alternative_heap(vm_heap_t* main_heap, vm_heap_t* alt_heap) {
    lwt_heap_t* fiber_heap = vm_heap_alloc_destructable(main_heap, sizeof(lwt_heap_t), 0, lwt_fiber_heap_destructor);
    fiber_heap->vm_heap = alt_heap;
    return fiber_heap;
}

/// Deactivates the extra fiber heap from freeing the underlying vm heap when destroyed.
static void lwt_fiber_heap_deactivate(lwt_heap_t* fiber_heap) {
    fiber_heap->vm_heap = 0;
}

// We use free list allocators for all event stack allocations getting the most compact and less contended allocation possible.

VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_edata_mitosis_t, lwt_edata_mitosis_allocate, lwt_edata_mitosis_free, true);
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_edata_ifc_join_t, lwt_edata_ifc_join_allocate, lwt_edata_ifc_join_free, true);
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_edata_try_catch_t, lwt_edata_try_catch_allocate, lwt_edata_try_catch_free, true);
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_edata_deferred_t, lwt_edata_deferred_allocate, lwt_edata_deferred_free, true);
VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_fiber_event_t, lwt_fiber_event_allocate, lwt_fiber_event_free, true);

static __inline void lwt_fiber_event_push(lwt_fiber_t* fiber, lwt_fiber_event_type_t type, lwt_fiber_event_data_t data) {
    lwt_fiber_event_t* event = lwt_fiber_event_allocate();
    event->type = type;
    event->data = data;
    event->next = fiber->event_stack;
    fiber->event_stack = event;
}

static __inline lwt_fiber_event_data_t lwt_fiber_event_pop(lwt_fiber_t* fiber, lwt_fiber_event_type_t type) {
    lwt_fiber_event_t* event = fiber->event_stack;
    assert(event != 0 && event->type == type);
    if (event == 0 || event->type != type)
        throw("lwthreads corrupt state: invalid event type on fiber stack", exception_fatal);
    fiber->event_stack = fiber->event_stack->next;
    lwt_fiber_event_data_t event_data = event->data;
    lwt_fiber_event_free(event);
    return event_data;
}

static __inline lwt_physical_thread_t* lwt_get_physical_thread_from_fsreg() {
    void* fs;
    __asm__ __volatile__(
        "mov %%fs:0, %%rax;\n\t"
        : // Output
        "=a" (fs)
        : // Input
        : "%rax" // Clobbered register
    );
    return fs;
}

static inline void lwt_sys_spinlock_rlock(rwspinlock_t* rwspinlock, rwspinlock_t** prev_system_rwspinlock) {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    *prev_system_rwspinlock = phys_thread->system_rwspinlock;
    phys_thread->system_rwspinlock = rwspinlock;
    atomic_spinlock_rlock(rwspinlock);
}

static inline void lwt_sys_spinlock_wlock(rwspinlock_t* rwspinlock, rwspinlock_t** prev_system_rwspinlock) {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    *prev_system_rwspinlock = phys_thread->system_rwspinlock;
    phys_thread->system_rwspinlock = rwspinlock;
    atomic_spinlock_wlock(rwspinlock);
}

static inline void lwt_sys_spinlock_unlock(rwspinlock_t* rwspinlock, rwspinlock_t** prev_system_rwspinlock, bool rlock) {
    if (rlock) {
        atomic_spinlock_urlock(rwspinlock);
    } else {
        atomic_spinlock_uwlock(rwspinlock);
    }
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    phys_thread->system_rwspinlock = *prev_system_rwspinlock;
}

/// Enqueues a fiber scheduled for execution and wakes any waiting physical thread in the process.
static void lwt_scheduler_exec_block_enqueue(lwt_fiber_t* fiber) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    for (;;) {
        uint32_t exec_blocked_futex_v = shared_fiber_mem.exec_block_futex;
        if (atomic_cas_uint32(&shared_fiber_mem.exec_block_futex, exec_blocked_futex_v, exec_blocked_futex_v + 1))
            break;
        sync_synchronize();
    }
    QUEUE_ENQUEUE_SL(&shared_fiber_mem.exec_block_queue, &fiber->exec_blocked);
    int32_t futex_r = futex((int*) &shared_fiber_mem.exec_block_futex, FUTEX_WAKE, 1, 0, 0, 0);
    if (futex_r == -1)
        RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
}

/// Dequeues a fiber scheduled for execution and yields to Linux until one becomes available.
static lwt_fiber_t* lwt_scheduler_exec_block_dequeue() {
    exec_blocked_fiber_t* execb_fiber = 0;
    for (;;) {
        // Implementation of debug choke here.
        for (;;) {
            uint32_t debug_choke_futex_v = shared_fiber_mem.debug_choke_futex;
            sync_synchronize(); // Barrier to guarantee memory ordering as the previous futex load must be executed before the following load.
            if (!shared_fiber_mem.debug_choke_enabled)
                break;
            uint64_t debug_choke_count_v = shared_fiber_mem.debug_choke_count;
            if (debug_choke_count_v >= (lwt_executor_thread_count - 1))
                break;
            if (atomic_cas_uint64(&shared_fiber_mem.debug_choke_count, debug_choke_count_v, debug_choke_count_v + 1)) {
                // Wait until the debug_choke_futex is triggered.
                int32_t futex_r = futex((int*) &shared_fiber_mem.debug_choke_futex, FUTEX_WAIT, (int) debug_choke_futex_v, 0, 0, 0);
                if (futex_r != 0 && errno != ETIMEDOUT && errno != EWOULDBLOCK && errno != EINTR)
                    RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
                for (;;) {
                    uint64_t debug_choke_count_v = shared_fiber_mem.debug_choke_count;
                    if (atomic_cas_uint64(&shared_fiber_mem.debug_choke_count, debug_choke_count_v, debug_choke_count_v - 1))
                        break;
                    sync_synchronize();
                }
            }
        }
        // Try to dequeue the next execution blocked fiber.
        uint32_t exec_blocked_futex_v;
        LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
            exec_blocked_futex_v = shared_fiber_mem.exec_block_futex;
            sync_synchronize(); // Barrier to guarantee memory ordering as the previous futex load must be executed before the following load.
            execb_fiber = QUEUE_DEQUEUE_SL(&shared_fiber_mem.exec_block_queue);
        } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
        if (execb_fiber != 0)
            break;
        int32_t futex_r = futex((int*) &shared_fiber_mem.exec_block_futex, FUTEX_WAIT, (int) exec_blocked_futex_v, 0, 0, 0);
        if (futex_r != 0 && errno != ETIMEDOUT && errno != EWOULDBLOCK && errno != EINTR)
            RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
        sync_synchronize();
    }
    return ((void*) execb_fiber) - offsetof(lwt_fiber_t, exec_blocked);
}

void lwt_debug_choke_fibers(bool trigger) {
    for (;;) {
        uint32_t debug_choke_futex_v = shared_fiber_mem.debug_choke_futex;
        if (atomic_cas_uint32(&shared_fiber_mem.debug_choke_futex, debug_choke_futex_v, debug_choke_futex_v + 1))
            break;
        sync_synchronize();
    }
    shared_fiber_mem.debug_choke_enabled = trigger;
    int32_t futex_r = futex((int*) &shared_fiber_mem.debug_choke_futex, FUTEX_WAKE, INT_MAX, 0, 0, 0);
    if (futex_r == -1)
        RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
}

/// Defers fiber until canceled or done with whatever pending operation we have.
static void lwt_scheduler_fiber_defer(bool is_yielding, lwt_ifc_server_t* accept_ifc_server, rcd_fid_t wait_fid, int32_t wait_fd) {
    LWT_GET_LOCAL_FIBER(fiber);
    // Just return immediately if already canceled or done. (Optimization that saves one "context switch" however we still check this after the context switch in a race-free manner).
    if ((!fiber->ctrl.unintr && (fiber->ctrl.canceled || fiber->ctrl.join_race)) || fiber->ctrl.done)
        return;
    // Store information on why the fiber is deferred to allow dumping wait graph for further analyzation.
    fiber->defer_ifc_server = accept_ifc_server;
    fiber->defer_wait_fid = wait_fid;
    fiber->defer_wait_fd = wait_fd;
    // Push deferred event.
    jmp_buf jbuf;
    lwt_fiber_event_data_t event;
    event.deferred.jbuf = &jbuf;
    lwt_fiber_event_push(fiber, lwt_fiber_event_deferred, event);
    // Commence context switch.
    int setjmp_r = setjmp(jbuf);
    if (setjmp_r == LWT_LONGJMP_DIRECT) {
        lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
        //phys-scheduler/ DBG("fiber [", DBG_INT(fiber->ctrl->id), "]: going to sleep on phys thread ", DBG_INT(phys_thread->linux_tid), " [st=", DBG_PTR(stack_top), "]");
        longjmp(phys_thread->physical_jmp_buf, is_yielding? LWT_LONGJMP_YIELD: LWT_LONGJMP_SWITCH);
        unreachable();
    } else {
        assert(!fiber->ctrl.deferred);
        lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
        assert(phys_thread->current_fiber == fiber);
        //phys-scheduler/ DBG("fiber [", DBG_INT(fiber->ctrl->id), "]: woke up on phys thread ", DBG_INT(phys_thread->linux_tid), " [st=", DBG_PTR(LWT_STACK_TOP), "]");
    }
}

static void lwt_scheduler_fiber_wake_done_raw(lwt_fiber_t* fiber) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    if (fiber->ctrl.deferred) {
        fiber->ctrl.deferred = false;
        lwt_scheduler_exec_block_enqueue(fiber);
    } else {
        fiber->ctrl.done = true;
    }
}

static void lwt_scheduler_fiber_wake_done(lwt_fiber_t* fiber) {
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        lwt_scheduler_fiber_wake_done_raw(fiber);
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
}

static void lwt_scheduler_fiber_wake_join_race_raw(lwt_fiber_t* fiber) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    if (fiber != 0 && !fiber->ctrl.join_race) {
        fiber->ctrl.join_race = true;
        if (!fiber->ctrl.unintr && fiber->ctrl.deferred) {
            fiber->ctrl.deferred = false;
            lwt_scheduler_exec_block_enqueue(fiber);
        }
    }
}

noret static void lwt_throw_cancel_exception(lwt_fiber_t* fiber) {
    rcd_exception_t* src_cancel_e = fiber->ctrl.canceled;
    fiber->ctrl.canceled = 0;
    if (src_cancel_e != 0) {
        // Allocate a new lwt heap with a real destructor and throw away the dummy one.
        lwt_heap_t* dummy_exception_heap = src_cancel_e->exception_heap;
        vm_heap_t* exception_vm_heap = dummy_exception_heap->vm_heap;
        LWT_GET_LOCAL_FIBER(fiber);
        lwt_heap_t* real_exception_heap = lwt_construct_alternative_heap(fiber->current_heap, exception_vm_heap);
        src_cancel_e->exception_heap = real_exception_heap;
        vm_heap_free(exception_vm_heap, dummy_exception_heap);
    }
    lwt_throw_new_exception("fiber canceled", "", 0, exception_canceled, 0, 0, 0, src_cancel_e);
}

noret static void lwt_throw_join_race_exception(lwt_fiber_t* fiber) {
    fiber->ctrl.join_race = false;
    lwt_throw_new_exception("fiber got join race exception", "", 0, exception_join_race, 0, 0, 0, 0);
}

noret static void lwt_throw_no_such_fiber_exception() {
    lwt_throw_new_exception("uninterruptible join failed: no such fiber", "", 0, exception_no_such_fiber, 0, 0, 0, 0);
}

static __inline void lwt_cancellation_point_raw(lwt_fiber_t* fiber) {
    if (!fiber->ctrl.unintr) {
        if (fiber->ctrl.canceled != 0)
            lwt_throw_cancel_exception(fiber);
        if (fiber->ctrl.join_race)
            lwt_throw_join_race_exception(fiber/*, fiber->ctrl.join_race*/);
    }
}

static void lwt_cancel_fiber_id_raw(rcd_fid_t fiber_id) {
    hmap_fid_lookup_t lu = hmap_fid_lookup(&shared_fiber_mem.fiber_map, fiber_id, true);
    if (!hmap_fid_found(lu))
        return;
    lwt_fiber_t* remote_fiber = hmap_fid_value(lu);
    if (remote_fiber->ctrl.canceled != 0 || remote_fiber->ctrl.hidden_cancel)
        return;
    LWT_GET_LOCAL_FIBER(local_fiber);
    {
        // Create a root heap and allocate the cancel source exception on.
        // The lwt_heap_t object is just a dummy reference box without the destructor it normally has.
        vm_heap_t* vm_heap = vm_heap_create(0);
        lwt_heap_t* dummy_exception_heap = vm_heap_alloc(vm_heap, sizeof(lwt_heap_t), 0);
        dummy_exception_heap->vm_heap = vm_heap;
        switch_heap(dummy_exception_heap) {
            rcd_exception_t* src_cancel_e = vm_heap_alloc(vm_heap, sizeof(rcd_exception_t), 0);
            src_cancel_e->type = exception_cancel_source;
            sub_heap {
                src_cancel_e->message = fss(escape(conc("fiber [#", ui2fs(local_fiber->ctrl.id), "] is canceling fiber [#", ui2fs(remote_fiber->ctrl.id), "]")));
            }
            src_cancel_e->file = "";
            src_cancel_e->line = 0;
            src_cancel_e->backtrace_calls = lwt_get_backtrace_for_frame_ptr(0, 2);
            src_cancel_e->fwd_exception = 0;
            src_cancel_e->exception_heap = dummy_exception_heap;
            remote_fiber->ctrl.canceled = src_cancel_e;
        }
    }
    if (!remote_fiber->ctrl.unintr && remote_fiber->ctrl.deferred) {
        remote_fiber->ctrl.deferred = false;
        lwt_scheduler_exec_block_enqueue(remote_fiber);
    }
}

void lwt_cancel_fiber_id(rcd_fid_t fiber_id) {
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        lwt_cancel_fiber_id_raw(fiber_id);
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
}

void lwt_exclusive_cancel_fiber_id(rcd_fid_t fiber_id) {
    LWT_GET_LOCAL_FIBER(fiber);
    bool has_cancel;
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        if (fiber->ctrl.canceled == 0) {
            lwt_cancel_fiber_id_raw(fiber_id);
            has_cancel = false;
        } else {
            has_cancel = true;
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    if (has_cancel)
        lwt_throw_cancel_exception(fiber);
}

void lwt_setup_archaic_physical_thread() {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    phys_thread->system_fiber.main_name = "[librcd archaic fiber]";
    phys_thread->current_fiber = &phys_thread->system_fiber;
}

void lwt_setup_basic_thread(vm_heap_t* vm_heap, fstr_t sys_fiber_name) {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    assert(phys_thread->linux_tid == gettid());
    phys_thread->system_rwspinlock = 0;
    phys_thread->system_fiber = (lwt_fiber_t) {0};
    phys_thread->system_fiber.main_name = sys_fiber_name;
    phys_thread->current_fiber = &phys_thread->system_fiber;
    phys_thread->thread_static_heap.vm_heap = 0;
    phys_thread->system_fiber.current_heap = vm_heap;
}

/// Called by the physical thread after the clone to set up important things it requires for proper thread local execution.
static void lwt_setup_physical_thread() {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    assert(phys_thread->linux_tid == gettid());
    phys_thread->system_rwspinlock = 0;
    phys_thread->system_fiber = (lwt_fiber_t) {0};
    phys_thread->system_fiber.main_name = "[librcd system fiber]";
    phys_thread->current_fiber = &phys_thread->system_fiber;
    phys_thread->thread_static_heap.vm_heap = vm_heap_create(0);
    phys_thread->system_fiber.current_heap = phys_thread->thread_static_heap.vm_heap;
    // Defined by the auto linker. Bounds of the librcd_thread_static_memory section.
    extern const void *__start_librcd_thread_static_memory, *__stop_librcd_thread_static_memory;
    // Initialize thread static memory.
    size_t thread_static_memory_size = (void*) &__stop_librcd_thread_static_memory - (void*) &__start_librcd_thread_static_memory;
    phys_thread->thread_static_memory = vm_heap_alloc(phys_thread->thread_static_heap.vm_heap, thread_static_memory_size, 0);
    memcpy(phys_thread->thread_static_memory, (void*) &__start_librcd_thread_static_memory, thread_static_memory_size);
}

void* lwt_get_thread_static_ptr(const void* ptr) {
    extern const void *__start_librcd_thread_static_memory;
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    return phys_thread->thread_static_memory + ((void*) ptr - (void*) &__start_librcd_thread_static_memory);
}

int32_t lwt_get_thread_pid() {
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    return phys_thread->pid;
}

/// Fiber finalization that is run by fiber when it shuts down.
/// To avoid a permanent return pointer in all root stacklets the control flow of the fiber is manipulated directly to run it.
noret static void lwt_fiber_finalize() {
    // We must release the root heap here rather than doing it in the physical executor context as releasing a heap invokes destructors which may invoke userspace code
    // and running user space code in the physical executor thread context is a bad, bad idea.
    // (For example it could block and recursively longjump back to the executor context which would be undefined behavior.)
    LWT_GET_LOCAL_FIBER(fiber);
    vm_heap_t* parent_heap = vm_heap_release(fiber->current_heap, 0, 0);
    if (parent_heap != 0)
        throw("failed to release root heap, detected parents. possible memory corruption", exception_fatal);
    // Signal physical thread executor context to tear down thread.
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    longjmp(phys_thread->physical_jmp_buf, LWT_LONGJMP_TEARDOWN);
    unreachable();
}

/// Returns the end of stack limit for the specified stacklet.
static inline void* lwt_get_end_of_stack_limit(lwt_stacklet_t* stacklet) {
    // ===== Example prologue: =====
    // lea -0x40(%rsp),%r11
    // cmp %fs:0x8,%r11
    // ja 1f
    // movabs $0x40,%r10
    // movabs $0x0,%r11
    // callq  __morestack
    // retq
    // 1: push rbp
    //
    // ===== Worst case memory layout @ prologue oosm check: =====
    //   |__stack_ptr_|______description_______|_stacklet->mem_|
    //   |  0x18(rsp) [ stack eightbyte n - 2  ]  0x38(slm)    |
    //   |  0x10(rsp) [ stack eightbyte n - 1  ]  0x30(slm)    |
    //   |  0x08(rsp) [ stack eightbyte n      ]  0x28(slm)    |
    //   |  0x00(rsp) [  call return address   ]  0x20(slm)    |
    //                    (irrelevant frame)
    //   | -0x08(rsp) [next call return address]  0x18(slm)    |
    //   | -0x10(rsp) [morestack return address]  0x10(slm)    |
    //   | -0x18(rsp) [  morestack pushed rbp  ]  0x08(slm)    |
    //   | -0x20(rsp) [ setjmp return address  ]  0x00(slm)    |
    //   | -0x28(rsp) [ << stack header mem >> ] -0x08(slm)    |
    //
    // ===== Abstract: =====
    // The size of the other stack memory used by the frame is irrelevant since
    // the prologue automatically subtracts this so in the simplest case we
    // just assume it is zero. In this case r11 = rsp in the prologue so we
    // never mention r11.
    // The worst possible case in the prologue is essentially a request that
    // after branching over the stacklet call would have no memory left after
    // calling the next function, detecting oosm and using the morestack and
    // setjmp qword to call morestack. In this case %rsp equals 0x20(slm).
    // Since the branching over takes place if %rsp is higher than the
    // stack limit the tightest memory limit is one byte less (0x1f(slm)).
    void* end_of_stack = (void*) ((uint64_t)(stacklet->mem) + 0x1f);
#ifdef DEBUG
    // Protect stack overflow canary when debugging.
    end_of_stack += 8;
#endif
    return end_of_stack;
}

static void lwt_init_signal_stack() {
    // Lets reserve about 0x40 pages of virtual memory for that alternative stack and notify Linux about it.
    // Since physical executor threads is static in librcd we don't need to eventually free this memory.
    size_t sstack_size;
    void* sstack_mem = vm_mmap_reserve(40 * PAGE_SIZE, &sstack_size);
    assert(sstack_size > MINSIGSTKSZ);
    stack_t sstack = {
        .ss_sp = sstack_mem,
        .ss_flags = 0,
        .ss_size = sstack_size,
    };
    stack_t prev_sstack;
    int32_t signalstack_r = sigaltstack(&sstack, &prev_sstack);
    if (signalstack_r == -1)
        RCD_SYSCALL_EXCEPTION(sigaltstack, exception_fatal);
    // We sanity check that the previous signal stack was disabled. If it isn't the system might not have per-thread settings for
    // signal stacks which would cause memory corruption if different threads received signals at the same time and used the same stack.
    if ((prev_sstack.ss_flags & SS_DISABLE) == 0)
        throw(concs("sanity check failed: when setting up signal stack, the previous signal stack value was unexpectedly not disabled [", fss(fstr_from_uint(prev_sstack.ss_flags, 16)), "]"), exception_fatal);
}

static void lwt_physical_executor_thread(void* arg_ptr) {
    const size_t lwt_stacklet_t_size = ((sizeof(lwt_stacklet_t) + 0xfUL) & ~0xfUL);
    const size_t lwt_stack_alloc_t_size = ((sizeof(lwt_stack_alloc_t) + 0xfUL) & ~0xfUL);
    /// The magic stack limit value [0] is used for librcd when using the bottom of the thread static mega stack.
    /// It is used when running unscheduled (system fiber) in high level functions.
    void* const magic_stack_limit_rcd_system = (void*) 0;
    /// The magic stack limit value [UINT64_MAX - 1] is used for fibers that has just been started and have no stack.
    void* const magic_stack_limit_init = (void*) (UINT64_MAX - 1);
    /// The magic stack limit value [UINT64_MAX] is used for raw assembly leaf functions that uses the thread static mega stack.
    void* const magic_stack_limit_raw_asm_leaf = (void*) (UINT64_MAX);
    lwt_physical_thread_t* const phys_thread = LWT_PHYS_THREAD;
    assert(phys_thread->end_of_stack == magic_stack_limit_rcd_system);
    lwt_fiber_t* fiber;
    void* fiber_end_of_stack;
    int32_t setjmp_r;
    // Initialize the fxsave memory.
    phys_thread->fxsave_mem = vm_mmap_reserve(sizeof(lwt_fxsave_mem_t), 0);
    // Since fibers have limited stack memory we must set up an alternative stack that the kernel can use for signal handlers for these physical executor threads.
    lwt_init_signal_stack();
    // Notify any debuggers that we started a new thread.
    raise(SIGUSR1);
    // Get the next execution blocked fiber.
    get_next_execution_blocked_fiber: {
        // Read next execution blocked fiber.
        fiber = lwt_scheduler_exec_block_dequeue();
        phys_thread->current_fiber = fiber;
        if (!fiber->ctrl.started)
            goto start_new_fiber;
        //phys-scheduler/ DBG("thread ", DBG_INT(phys_thread->ptid),  ": undeferring fiber [", DBG_INT(fiber->ctrl->id), "]");
        // Fiber is deferred, switch back to it.
        setjmp_r = setjmp(phys_thread->physical_jmp_buf);
        if (setjmp_r != LWT_LONGJMP_DIRECT)
            goto fiber_returned;
        // Calculate the end of stack limit for the fiber to resume.
        fiber_end_of_stack = fiber->current_stacklet != 0? lwt_get_end_of_stack_limit(fiber->current_stacklet): magic_stack_limit_init;
        goto resume_deferred_fiber;
    } resume_deferred_fiber: {
        lwt_fiber_event_data_t event_data = lwt_fiber_event_pop(fiber, lwt_fiber_event_deferred);
        // Set end of stack and long jump back to the fiber. Setting end of stack has side effects on all function calls so we must do it just before long jumping.
        phys_thread->end_of_stack = fiber_end_of_stack;
        longjmp(*event_data.deferred.jbuf, LWT_LONGJMP_SWITCH);
        unreachable();
    } start_new_fiber: {
        //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": executing new fiber [", DBG_INT(fiber->ctrl.id), "]");
        // Starting a new fiber, prepare arguments.
        fiber->ctrl.started = true;
        void (*start_fn_ptr)(void*) = (void*) fiber->event_stack;
        phys_thread->main_fn_ptr = start_fn_ptr;
        void* start_fn_arg_ptr = (void*) fiber->current_ifc_join_event;
        // Initialize fiber->current_ifc_join_event and fiber->event_stack as we stole these fields for the start_fn_arg_ptr.
        fiber->event_stack = 0;
        fiber->current_ifc_join_event = 0;
        // Perform initial context switch.
        setjmp_r = setjmp(phys_thread->physical_jmp_buf);
        if (setjmp_r != LWT_LONGJMP_DIRECT)
            goto fiber_returned;
        // Set up the magic stack limit and jump directly to the main function with the right argument.
        // The function will call us back with the size of the root stacklet and the stack will be set up ad hoc without any special corner cases for the root.
        phys_thread->end_of_stack = magic_stack_limit_init;
        __asm__ __volatile__("jmp *%%rsi\n\t":: "S"(start_fn_ptr), "D"(start_fn_arg_ptr));
        unreachable();
    } pop_rip_and_return_via_stack_pinj_jmp_buf: {
        // Pop the return address from the %rsp and inject into %rip.
        phys_thread->stack_pinj_jmp_buf.rip = *((uint64_t*) phys_thread->stack_pinj_jmp_buf.rsp);
        phys_thread->stack_pinj_jmp_buf.rsp += 8;
        goto return_via_stack_pinj_jmp_buf;
    } return_via_stack_pinj_jmp_buf: {
        // Set end of stack and long jump back to the fiber. Setting end of stack has side effects on all function calls so we must do it just before long jumping.
        phys_thread->end_of_stack = fiber_end_of_stack;
        longjmp_pinj(&phys_thread->stack_pinj_jmp_buf);
        unreachable();
    } fiber_returned: {
        // Disabling stacklet function prologues by restoring end of stack to
        // zero since we're back using the thread static mega stack now.
        fiber_end_of_stack = phys_thread->end_of_stack;
        phys_thread->end_of_stack = 0;
        // Fibers are not allowed to make syscalls while using the thread static mega stack (e.g. in raw assembly contexts) since this corrupts their memory.
        // Raw assembly contexts that defer to the librcd scheduler is not supported. They should be written as pure lambdas and never call back to stack-managed contexts.
        if (fiber_end_of_stack == magic_stack_limit_raw_asm_leaf)
            throw("fiber syscalled from illegal state, the mega stack was in use in a raw assembly context", exception_fatal);
        if (fiber_end_of_stack == magic_stack_limit_rcd_system)
            throw("fiber syscalled from illegal state, the mega stack was in use in a librcd system context", exception_fatal);
        // Check what kind of system call the fiber made.
        switch (setjmp_r) {{
        } case LWT_LONGJMP_MMAP_RESERVE: {
            // Get the arguments from the registers.
            size_t min_size = (size_t) phys_thread->stack_pinj_jmp_buf.rdi;
            size_t* size_out = (size_t*) phys_thread->stack_pinj_jmp_buf.rsi;
            // Reserve memory and return the pointer.
            phys_thread->stack_pinj_jmp_buf.rax = (uint64_t) vm_mmap_reserve_sys(min_size, size_out);
            // Return to the caller.
            goto pop_rip_and_return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_MMAP_UNRESERVE: {
            // Get the arguments from the registers.
            void* ptr = (void*) phys_thread->stack_pinj_jmp_buf.rdi;
            size_t size = (size_t) phys_thread->stack_pinj_jmp_buf.rsi;
            // Free memory.
            vm_mmap_unreserve_sys(ptr, size);
            // Return to the caller.
            goto pop_rip_and_return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_VA_START: {
            // We detect if a new stacklet was injected in the middle of the variable arguments call by observing if the current frame returns to __releasestack.
            void* return_addr = *((void**) (phys_thread->stack_pinj_jmp_buf.rbp + 8));
            if (return_addr == __releasestack_free_stack_space) {
                // There was a dynamic stack allocation return pointer injected which may shadow the real __releasestack return pointer.
                void* virtual_rbp = *((void**) phys_thread->stack_pinj_jmp_buf.rbp);
                return_addr = *((void**) (virtual_rbp + 8));
            }
            if (return_addr == __releasestack || return_addr == __releasestack_fx) {
                // Variable arguments function with stack split. In this scenario the overflow_arg_area pointer has the right relative offset but he wrong origin.
                lwt_abi_va_list* vl = (void*) phys_thread->stack_pinj_jmp_buf.rdi;
                ssize_t rbp_offset = vl->overflow_arg_area - ((void*) phys_thread->stack_pinj_jmp_buf.rbp);
                // If we apply the offset to the older frame [morestack return address], which is 0x8 bytes above the older rsp,
                // the distance to the argument area is the same and we get the correct argument memory address.
                //                     with stack split         without stack split
                // 0x18(older_rsp) [  argument eightbyte 0  ] [  argument eightbyte 0  ]
                // 0x10(older_rsp) [   call return address  ] [   call return address  ]
                // 0x08(older_rsp) [morestack return address] [  pushed previous rbp   ] <- (rbp)
                // 0x00(older_rsp) [  morestack pushed rbp  ]
                vl->overflow_arg_area = ((void*) fiber->current_stacklet->older_rsp) + 0x08 + rbp_offset;
            }
            // Return to the caller via __morestack_ctx_restore_fx. Pop the return address from the %rsp and inject into %r10.
            phys_thread->stack_pinj_jmp_buf.r10 = *((uint64_t*) phys_thread->stack_pinj_jmp_buf.rsp);
            phys_thread->stack_pinj_jmp_buf.rip = (uint64_t) __morestack_ctx_restore_fx;
            phys_thread->stack_pinj_jmp_buf.rsp += 8;
            goto return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_STACK_ALLOC: {
            // The fiber is allocating dynamic memory on the stack.
            size_t alloc_size = phys_thread->stack_pinj_jmp_buf.rdi;
            // Check if this is the first stack allocation - have we injected the release stack return pointer yet?
            size_t total_length;
            lwt_stack_alloc_t* stack_alloc;
            uint64_t* rbp_1 = *((uint64_t**) phys_thread->stack_pinj_jmp_buf.rbp);
            void* return_ptr = (void*) rbp_1[1];
            if (return_ptr == __releasestack_free_stack_space) {
                // This is not the first dynamic stack allocation in the frame.
                assert(fiber->stack_alloc_stack != 0);
                total_length = lwt_stack_alloc_t_size + alloc_size;
                void* stack_alloc_ptr = vm_mmap_reserve_sys(total_length, 0);
                stack_alloc = stack_alloc_ptr;
                stack_alloc->redirected_frame_ptr = 0;
                phys_thread->stack_pinj_jmp_buf.rax = (uint64_t) (stack_alloc_ptr + lwt_stack_alloc_t_size);
            } else {
                // This is the first dynamic stack allocation in the frame.
                // Allocate 2 extra qwords for a virtual frame and redirect return pointer to __releasestack_free_stack_space.
                // Redirect frame pointer to the virtual frame on the first stack allocation.
                total_length = lwt_stack_alloc_t_size + 0x10 + alloc_size;
                void* stack_alloc_ptr = vm_mmap_reserve_sys(total_length, 0);
                stack_alloc = stack_alloc_ptr;
                stack_alloc->redirected_frame_ptr = rbp_1;
                stack_alloc->mem[0] = rbp_1[0];
                stack_alloc->mem[1] = rbp_1[1];
                rbp_1[0] = (uint64_t) &stack_alloc->mem[0];
                rbp_1[1] = (uint64_t) __releasestack_free_stack_space;
                phys_thread->stack_pinj_jmp_buf.rax = (uint64_t) (stack_alloc_ptr + lwt_stack_alloc_t_size + 0x10);
            }
            // Initialize the stack allocation and add it to the stack alloc stack.
            stack_alloc->len = total_length;
            LL_PREPEND(fiber->stack_alloc_stack, stack_alloc);
            // Virtualize return from __morestack_allocate_stack_space().
            uint64_t* rsp = (uint64_t*) phys_thread->stack_pinj_jmp_buf.rsp;
            phys_thread->stack_pinj_jmp_buf.rbp = rsp[0];
            phys_thread->stack_pinj_jmp_buf.rip = rsp[1];
            phys_thread->stack_pinj_jmp_buf.rsp = (uint64_t) &rsp[2];
            // Return to the caller.
            goto return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_STACK_FREE: {
            // The fiber is freeing dynamic memory on the stack.
            assert(fiber->stack_alloc_stack != 0);
            // At this point the function should have pop'd rbp to our virtual stack frame in the last segment (&stack_alloc->mem[0]).
            // We use this to get the real return pointer and also iterate until we find the last segment.
            uint64_t* virtual_rbp = (uint64_t*) phys_thread->stack_pinj_jmp_buf.rbp;
            phys_thread->stack_pinj_jmp_buf.rbp = virtual_rbp[0];
            phys_thread->stack_pinj_jmp_buf.rip = virtual_rbp[1];
            // Loop through the stack allocation stack and free all segments in the current frame sequence group.
            // The frame sequence group is terminated by a stack allocation that store the virtual rbp.
            lwt_stack_alloc_t *stack_alloc, *next_stack_alloc;
            LL_FOREACH_SAFE(fiber->stack_alloc_stack, stack_alloc, next_stack_alloc) {
                vm_mmap_unreserve_sys(stack_alloc, stack_alloc->len);
                // We don't actually read the free'd memory here - only check it's addressing.
                if (&stack_alloc->mem[0] == virtual_rbp)
                    break;
            }
            fiber->stack_alloc_stack = next_stack_alloc;
            // Return to the caller.
            goto return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_MORE_STACK: {
            // The fiber need an additional stacklet, allocate it.
            lwt_stacklet_t* older_stacklet = fiber->current_stacklet;
#ifdef DEBUG
            void* cur_rsp = (void*) phys_thread->stack_pinj_jmp_buf.rsp;
            // Sanity check that current rsp is not off. It must be part of the current stacklet or the debug dummy stack.
            if (older_stacklet != 0) {
                if ((cur_rsp < (void*) older_stacklet->mem || cur_rsp > ((void*) older_stacklet) + older_stacklet->len)
                && (cur_rsp < (void*) __debug_call_dummy_stack || cur_rsp > ((void*) __debug_call_dummy_stack) + sizeof(__debug_call_dummy_stack))) {
                    throw("fiber requested more stack with invalid stack pointer (stack overflow)", exception_fatal);
                }
            }
#endif
            // The required stack size is stored in r10 (except the msb), round up and 8 byte align it.
            size_t required_stack_size = (((phys_thread->stack_pinj_jmp_buf.r10 & ~0x8000000000000000) + 0x7UL) & ~0x7UL);
            // When allocating the first stacklet we use the estimated stack size for the entire fiber.
            if (older_stacklet == 0)
                required_stack_size = MAX(required_stack_size, fiber->est_stack_size);
            // The system needs to pad the stack with 5 extra qwords: initial call (injected return pointer), last call, morestack call, morestack pushed rbp and setjmp/longjmp call.
            size_t system_stack_pad_size = 5 * 0x8;
            // Let the top area be the argument area plus 2 extra qwords where the old rip and rbp is stored.
            // This ensures that debuggers (like gdb) can understand where the older frame is without special librcd logic.
            // Morestack/releasestack must have it's own virtual 16 byte stack frame because functions are not guaranteed to save rbp, for example functions that don't return.
            // Align the top area size rounded up and 16 byte aligned so the initial entry point gets aligned.
            // We also align the whole argument area and pad it so that the virtual morestack frame above it gets aligned properly as well.
            size_t arg_data_size = phys_thread->stack_pinj_jmp_buf.r11;
            size_t arg_area_size = ((arg_data_size + 0xfUL) & ~0xfUL);
            size_t top_area_size = arg_area_size + 0x10;
            size_t min_alloc_size = lwt_stacklet_t_size + system_stack_pad_size + required_stack_size + top_area_size;
#ifdef DEBUG
            // Allocate stack overflow canary when debugging.
            min_alloc_size += 8;
#endif
            size_t final_alloc_size;
            lwt_stacklet_t* younger_stacklet = vm_mmap_reserve_sys(min_alloc_size, &final_alloc_size);
#ifdef DEBUG
            // When debugging, always allocate new stacklets for every call.
            final_alloc_size = ((min_alloc_size + 0xfUL) & ~0xfUL);
            // Set stack overflow canary. Reserve the lowest 8 bytes in the stack.
            younger_stacklet->mem[0] = 0xaaaaaaaaaaaaaaaa;
#endif
            younger_stacklet->len = final_alloc_size;
            younger_stacklet->next = older_stacklet;
            fiber->current_stacklet = younger_stacklet;
            fiber_end_of_stack = lwt_get_end_of_stack_limit(younger_stacklet);
            // Let stack_top be a pointer to the first qword in the top area that will contain the args followed by the virtual morestack frame.
            // The new stack layout will look like the following:
            // 8n + 0x08(stack_top) [ virt morestack saved rip  ]
            // 8n + 0x08(stack_top) [ virt morestack saved rbp  ]
            // 8n + 0x00(stack_top) [   argument eightbyte n    ]
            //                                  ...
            //      0x00(stack_top) [   argument eightbyte 0    ]
            //     -0x08(stack_top) [ __releasestack return ptr ]
            //                       (the first stacklet frame)
            uint64_t* stack_top = ((void*) younger_stacklet) + final_alloc_size - top_area_size;
            // Stack ep is guaranteed to be 16 bit aligned because all of stacklet, final_alloc_size and arg_area_size is guaranteed to be 16 bit aligned.
            assert(((uint64_t) stack_top & 0xfUL) == 0);
            if (older_stacklet != 0) {
                // Save the rsp and memcopy over arg area to new stacklet.
                uint64_t* older_rsp = (uint64_t*) (phys_thread->stack_pinj_jmp_buf.rsp);
                younger_stacklet->older_rsp = older_rsp;
                if (arg_data_size > 0) {
                    // The old stack layout looks like the following:
                    // 8n + 0x18(rsp) [  argument eightbyte n  ]
                    //                           ...
                    //      0x18(rsp) [  argument eightbyte 0  ]
                    //      0x10(rsp) [   call return address  ]
                    //      0x08(rsp) [morestack return address]
                    //      0x00(rsp) [  morestack pushed rbp  ]
                    memcpy(stack_top, ((void*) older_rsp) + 0x18, arg_data_size);
                }
                // Setting up the virtual morestack frame in the new stacklet that connects to the old stacklet.
                size_t arg_area_qwords = arg_area_size / sizeof(uint64_t);
                stack_top[arg_area_qwords] = older_rsp[0]; // callee rbp
                stack_top[arg_area_qwords + 1] = older_rsp[2]; // callee rip
                // The instruction pointer we will continue to is the morestack return address + 1, skipping the "ret" instruction.
                phys_thread->stack_pinj_jmp_buf.rip = older_rsp[1] + 1;
                // Initialize rbp point to the virtual morestack frame in the top area.
                phys_thread->stack_pinj_jmp_buf.rbp = (uint64_t) &stack_top[arg_area_qwords];
            } else {
                // The fiber had no stacklet yet so rsp is undefined and the call here overwrote whatever stack we used.
                younger_stacklet->older_rsp = 0;
                assert(arg_data_size == 0);
                // We store a null return and frame pointer in the virtual morestack frame so frame walking really stops.
                stack_top[0] = 0; // rbp
                stack_top[1] = 0; // rip
                // In this corner case we jump directly back to the main function that we have conveniently stored.
                phys_thread->stack_pinj_jmp_buf.rip = (uint64_t) phys_thread->main_fn_ptr;
                // Initialize rbp point to the virtual morestack frame in the top area.
                phys_thread->stack_pinj_jmp_buf.rbp = (uint64_t) &stack_top[0];
            }
            // The r10 msb signifies that we are doing a fx save context switch.
            uint64_t releasestack_fn;
            if ((phys_thread->stack_pinj_jmp_buf.r10 & 0x8000000000000000) != 0) {
                // Returning to __morestack_ctx_restore_fx() instead so we can trampoline back with the restored fx. It will jump to the ip specified by r10.
                phys_thread->stack_pinj_jmp_buf.r10 = phys_thread->stack_pinj_jmp_buf.rip;
                phys_thread->stack_pinj_jmp_buf.rip = (uint64_t) __morestack_ctx_restore_fx;
                // Using __releasestack_fx so we save fx again when context switching in releasestack.
                releasestack_fn = (uint64_t) __releasestack_fx;
            } else {
                releasestack_fn = (uint64_t) __releasestack;
            }
            // Fill the return pointer to __releasestack and set the rsp to be this address indicating top of stack. This is 8 bytes unaligned as enforced by ABI standard.
            stack_top[-1] = releasestack_fn;
            phys_thread->stack_pinj_jmp_buf.rsp = (uint64_t) &stack_top[-1];
            // Restore stack limit and return from stack operation.
            goto return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_LESS_STACK: {
            // Tearing down the youngest stacklet.
            lwt_stacklet_t* younger_stacklet = fiber->current_stacklet;
            if (younger_stacklet == 0)
                throw("less stack called from context without stack. possible memory corruption", exception_fatal);
#ifdef DEBUG
            // Check stack overflow canary when debugging.
            if (younger_stacklet->mem[0] != 0xaaaaaaaaaaaaaaaa)
                throw("stack overflow detected - memory is corrupt", exception_fatal);
#endif
            lwt_stacklet_t* older_stacklet = younger_stacklet->next;
            if (older_stacklet != 0) {
                // Switch to the older stacklet.
                fiber->current_stacklet = older_stacklet;
                fiber_end_of_stack = lwt_get_end_of_stack_limit(older_stacklet);
                // Get the stack pointer in the older stacklet. See "old stack layout" above for its alignment and an explanation of these offsets.
                uint64_t* older_rsp = younger_stacklet->older_rsp;
                // When restoring the context we jump to the original call return address and skip returning from morestack or the function altogether.
                phys_thread->stack_pinj_jmp_buf.rbp = older_rsp[0];
                phys_thread->stack_pinj_jmp_buf.rip = older_rsp[1];
                phys_thread->stack_pinj_jmp_buf.rsp = (uint64_t) &older_rsp[2];
                // Free the younger stacklet.
                vm_mmap_unreserve_sys(younger_stacklet, younger_stacklet->len);
            } else {
                // The fiber main function returned, the fiber is shutting down.
                // The instance name will be free'd and become invalid memory if it's allocated in the root of the heap,
                // so we set it to a place holder, making it always safe to read as long as the fiber is indexed.
                fiber->instance_name = "[librcd fiber terminating]";
                // Some finalization still needs to be done in the fiber context.
                // We inject the finalize function so it's called with the root stacklet.
                // This function will call back with teardown once the fiber finalization is complete.
                phys_thread->stack_pinj_jmp_buf.rip = (uint64_t) lwt_fiber_finalize;
                // Set up the rbp and rsp so we get a good stack entry point.
                // Just keeping the same rsp would waste stack and it may be aligned incorrectly and be non compliant with the ABI.
                phys_thread->stack_pinj_jmp_buf.rbp = 0;
                phys_thread->stack_pinj_jmp_buf.rsp = ((uint64_t) younger_stacklet) + younger_stacklet->len - 0x08;
                // Sanity check that rsp is aligned correctly at this point for prologue entry, simulating a call.
                assert((phys_thread->stack_pinj_jmp_buf.rsp & 0xfUL) == 0x08);
            }
            // The r10 lsb signifies that we are doing a fx save context switch.
            if ((phys_thread->stack_pinj_jmp_buf.r10 & 0x1) != 0) {
                // Returning to __morestack_ctx_restore_fx() instead so we can trampoline back with the restored fx. It will jump to the ip specified by r10.
                phys_thread->stack_pinj_jmp_buf.r10 = phys_thread->stack_pinj_jmp_buf.rip;
                phys_thread->stack_pinj_jmp_buf.rip = (uint64_t) __morestack_ctx_restore_fx;
            }
            goto return_via_stack_pinj_jmp_buf;
        } case LWT_LONGJMP_THROW_UNWIND_STACK: {
            // Unwinding the stack for a try/catch block.
            // First unwind all dynamic stack allocations to the same point they where at the point of the try.
            lwt_stack_alloc_t* try_salloc_top = phys_thread->try_stack_alloc_top;
            lwt_stack_alloc_t *salloc, *next_salloc;
            LL_FOREACH_SAFE(fiber->stack_alloc_stack, salloc, next_salloc) {
                if (salloc == try_salloc_top)
                    break;
                if (salloc->redirected_frame_ptr != 0) {
                    // Unredirect the frame as the frame could be the frame we are jumping back to and would be corrupt otherwise.
                    // This happens when dynamic stack allocation is first done inside a try block in a frame.
                    salloc->redirected_frame_ptr[0] = salloc->mem[0];
                    salloc->redirected_frame_ptr[1] = salloc->mem[1];
                }
                vm_mmap_unreserve_sys(salloc, salloc->len);
                fiber->stack_alloc_stack = next_salloc;
            }
            // We free stacklets until we reach the stack allocated jump buffer.
            jmp_buf* try_jmp_buf = phys_thread->try_jmp_buf;
            lwt_stacklet_t* stacklet = fiber->current_stacklet;
            for (;;) {
                if (stacklet == 0)
                    throw("unwinding the stack failed while throwing exception: could not locate the current stacklet context. possible memory corruption", exception_fatal);
                lwt_stacklet_t* prev_stacklet = stacklet->next;
                // Check if the try jump buffer is allocated in this stacklet range.
                void* test_stack_ptr = try_jmp_buf;
                void* stacklet_ptr = stacklet;
                //phys-scheduler/ DBG_RAW("[try unwind]: [", DBG_PTR(test_stack_ptr), "] [", DBG_PTR(stacklet_ptr), "] [", DBG_PTR(stacklet_ptr + stacklet->len), "]");
                if (test_stack_ptr > stacklet_ptr && test_stack_ptr < (stacklet_ptr + stacklet->len)) {
                    fiber->current_stacklet = stacklet;
                    break;
                }
                // Free this stacklet.
                vm_mmap_unreserve_sys(stacklet, stacklet->len);
                stacklet = prev_stacklet;
            }
            // Restore end of stack to whatever it was in the catch context and long jump back to it via the try_jmp_buf.
            phys_thread->end_of_stack = lwt_get_end_of_stack_limit(stacklet);
            longjmp(*try_jmp_buf, 1);
            unreachable();
        } case LWT_LONGJMP_TEARDOWN: {
            // Switch to system fiber context beyond this point.
            phys_thread->current_fiber = &phys_thread->system_fiber;
            // Tearing down the whole fiber, free the last stacklets.
            lwt_stacklet_t *stacklet, *next_stacklet;
            LL_FOREACH_SAFE(fiber->current_stacklet, stacklet, next_stacklet) {
                vm_mmap_unreserve_sys(stacklet, stacklet->len);
            }
            // Free the last dynamic stack allocations.
            lwt_stack_alloc_t *stack_alloc, *next_stack_alloc;
            LL_FOREACH_SAFE(fiber->stack_alloc_stack, stack_alloc, next_stack_alloc) {
                vm_mmap_unreserve_sys(stack_alloc, stack_alloc->len);
            }
            //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": fiber [", DBG_INT(fiber->ctrl.id), "] signaled teardown, tearing down");
            // Remove the fiber from the global index.
            LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                hmap_fid_lookup_t lu = hmap_fid_lookup(&shared_fiber_mem.fiber_map, fiber->ctrl.id, true);
                if (!hmap_fid_found(lu))
                    throw("removing fiber from global index failed: index did not exist. possible memory corruption", exception_fatal);
                hmap_fid_delete(&shared_fiber_mem.fiber_map, lu);
                DL_DELETE(shared_fiber_mem.fiber_list, fiber);
            } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
            // The fiber is no longer reachable by any calls as it doesn't exist in the global index.
            LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                for (lwt_ifc_fn_queue_t* ifc_fn_queue = RBTREE_NODE2ELEM(lwt_ifc_fn_queue_t, rb_node, rbtree_first(&fiber->ifc_fn_queues))
                ; ifc_fn_queue != 0; ifc_fn_queue = RBTREE_NODE2ELEM(lwt_ifc_fn_queue_t, rb_node, rbtree_next(&ifc_fn_queue->rb_node))) {
                    // There can be no ifc server as the accept loop should have free'd it, however there may still be waiting clients. Trigger join race in all of them.
                    assert(ifc_fn_queue->ifc_server_ref_count == 0);
                    for (lwt_ifc_client_t* client = ifc_fn_queue->waiting_clients.first; client != 0; client = client->next) {
                        if (client->fiber->ctrl.unintr) {
                            /// Uninterruptable pending clients cannot be join raced as this is a form of interrupt, instead they are just woken up as they expect to be so if the client no longer existed.
                            lwt_scheduler_fiber_wake_done_raw(client->fiber);
                        } else {
                            /// For normal interruptible clients we use a standard join race.
                            lwt_scheduler_fiber_wake_join_race_raw(client->fiber);
                        }
                    }
                    // We set server fiber to null as the fiber will become invalid memory.
                    ifc_fn_queue->server_fiber = 0;
                }
            } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
            // Free any pending cancellation.
            rcd_exception_t* src_cancel_e = fiber->ctrl.canceled;
            if (src_cancel_e != 0) {
                lwt_heap_t* dummy_exception_heap = src_cancel_e->exception_heap;
                vm_heap_t* exception_vm_heap = dummy_exception_heap->vm_heap;
                vm_heap_t* parent_heap = vm_heap_release(exception_vm_heap, 0, 0);
                assert(parent_heap == 0);
            }
            // Free the root heap and all remaining associated memory/resources.
            //DBG("thread ", DBG_INT(phys_thread->ptid),  ": releasing heap [", DBG_PTR(fiber->current_heap), "] of fiber [", DBG_PTR(fiber), "]");
            // Free the actual fiber struct.
            lwt_fiber_free(fiber);
            break;
        } case LWT_LONGJMP_PANIC: {
            lwt_panic();
            break;
        } case LWT_LONGJMP_SWITCH: {
        } case LWT_LONGJMP_YIELD: {
        } default: {
            //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": fiber [", DBG_INT(fiber->ctrl.id), "] signaled defer, deferring");
            // Mark the thread as deferred unless it's canceled or done in which case it should be immediately resumed as deferred and (canceled or done) cannot be true at the same time.
            bool defer_bounce;
            LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                defer_bounce = ((!fiber->ctrl.unintr && (fiber->ctrl.canceled != 0 || fiber->ctrl.join_race)) || fiber->ctrl.done);
                if (!defer_bounce) {
                    if (setjmp_r == LWT_LONGJMP_YIELD) {
                        // Add to end of defer blocking queue so execution can begin again asap.
                        lwt_scheduler_exec_block_enqueue(fiber);
                    } else {
                        // Set deferred to true allowing physical threads to take control over the fiber.
                        fiber->ctrl.deferred = true;
                    }
                }
            } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
            // Unless defer_bounce is true the physical thread no longer owns this fiber and it would be unsafe to reference here (after the unlock).
            if (defer_bounce) {
                //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": fiber [", DBG_INT(fiber->ctrl.id), "] canceled or done, defer bouncing and resuming execution");
                goto resume_deferred_fiber;
            }
            // Switch to system fiber context beyond this point.
            //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": switching to system fiber");
            phys_thread->current_fiber = &phys_thread->system_fiber;
            break;
        }}
        //phys-scheduler/ DBG_RAW("thread ", DBG_INT(phys_thread->pid),  ": dequeueing next fiber");
        goto get_next_execution_blocked_fiber;
    }
}

static void lwt_physical_thread_main(void* arg_ptr) {
    lwt_setup_physical_thread();
    rsig_thread_signal_mask_reset();
    lwt_start_cb_t* start_cb_arg_ptr = arg_ptr;
    lwt_start_cb_t start_cb = *start_cb_arg_ptr;
    vm_mmap_unreserve(start_cb_arg_ptr, sizeof(lwt_start_cb_t));
    start_cb.start_fn(start_cb.arg_ptr);
    _exit(0);
}

int32_t lwt_start_new_thread(void (*func)(void* arg_ptr), void* stack, int flags, void* arg_ptr, lwt_physical_thread_t** io_phys_thread) {
    return (int32_t) __syscall_ret((unsigned long) _start_new_thread(func, stack, flags, arg_ptr, io_phys_thread));
}

int32_t lwt_get_thread_clone_flags() {
    // Set of flags used by clone when cloning new threads.
    return (CLONE_FILES | CLONE_FS | CLONE_PARENT | CLONE_THREAD | CLONE_VM | CLONE_SYSVSEM | CLONE_SIGHAND | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID);
}

static void* lwt_thread_stack_alloc(size_t stack_size, void** out_entry_pt) {
    // Add implicit space for guard page at end.
    stack_size += PAGE_SIZE;
    // Make mmap allocation.
    void* mmap_r = mmap(0, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_r == MAP_FAILED)
        RCD_SYSCALL_EXCEPTION(mmap, exception_fatal);
    // Protect guard page.
    int32_t mprotect_r = mprotect(mmap_r, PAGE_SIZE, PROT_NONE);
    if (mprotect_r == -1)
        RCD_SYSCALL_EXCEPTION(mprotect, exception_fatal);
    // Calculate entry point.
    *out_entry_pt = (void*) vm_align_floor((uintptr_t) mmap_r + stack_size, VM_ALLOC_ALIGN);
    // Return pointer to allocation.
    return mmap_r;
}

static void lwt_thread_stack_free(void* mmap_alloc, size_t stack_size) {
    // Add implicit space for guard page at end.
    stack_size += PAGE_SIZE;
    int32_t munmap_r = munmap(mmap_alloc, stack_size);
    if (munmap_r == -1)
        RCD_SYSCALL_EXCEPTION(munmap, exception_fatal);
}

void lwt_thread_cancel_sync(int32_t tid) {
    // After sending the async cancel signal the thread is just a few
    // instructions away from terminating. This means that we should never do
    // more than a few loops in the worst case.
    for (int32_t signal = LWT_ASYNC_CANCEL_SIGNAL;; signal = 0) {
        int32_t rt_tgsigqueueinfo_r = lwt_sigtkill_thread(tid, signal, (union sigval) {0});
        if (rt_tgsigqueueinfo_r == -1) {
            int32_t errno_v = errno;
            if (errno_v == ESRCH)
                break;
            else if (errno_v == EAGAIN)
                continue;
            RCD_SYSCALL_EXCEPTION(rt_tgsigqueueinfo, exception_fatal);
        }
        sched_yield();
    }
}

static lwt_physical_thread_t* lwt_start_physical_thread(lwt_start_cb_t start_cb) {
    // Prepare base stack and physical thread struct allocated on it.
    const size_t lwt_physical_stack_size = PAGE_SIZE * 0x40;
    void* stack_entry;
    lwt_thread_stack_alloc(lwt_physical_stack_size, &stack_entry);
    lwt_start_cb_t* start_cb_arg_ptr = vm_mmap_reserve(sizeof(lwt_start_cb_t), 0);
    *start_cb_arg_ptr = start_cb;
    // DBG("[lwthreads] allocated new stack for physical thread at [", DBG_PTR(stack_mmap_base), "]-[", DBG_PTR(stack_mmap_base + lwt_stack_size_default - 1), "]");
    // DBG("[lwthreads] allocated new physical thread objects at [", DBG_PTR(phys_thread), "]-[", DBG_PTR(phys_thread + sizeof(lwt_physical_thread_t) - 1), "], [", DBG_PTR(exec_thread), "]-[", DBG_PTR(exec_thread + sizeof(lwt_executor_thread_t) - 1), "]");
    // Stack grows downward.
    int32_t clone_flags = lwt_get_thread_clone_flags();
    lwt_physical_thread_t* phys_thread = 0;
    int32_t clone_r = lwt_start_new_thread(lwt_physical_thread_main, stack_entry, clone_flags, start_cb_arg_ptr, &phys_thread);
    if (clone_r == -1)
        RCD_SYSCALL_EXCEPTION(clone, exception_fatal);
    // We store the pid separately even though it should be the same as linux_tid because the kernel will reset linux_tid when the thread exits.
    phys_thread->pid = clone_r;
    // DBGFN("match_executor_count: started new thread with id  #", DBG_INT(clone_r),  " and phys thread @ [", DBG_PTR(phys_thread), "], [", DBG_INT(phys_thread->ptid), "]");
    return phys_thread;
}

typedef struct lwt_real_block_args {
    void (*main_fn)(void*);
    void* arg_ptr;
    lwt_heap_t* heap;
    int32_t event_h;
} lwt_real_block_args_t;

static void lwt_real_block_main(void* arg_ptr) {
    lwt_real_block_args_t* real_block_args = arg_ptr;
    lwt_setup_basic_thread(real_block_args->heap->vm_heap, "[librcd real block fiber]");
    rsig_thread_signal_mask_reset();
    // Call the real block main function.
    real_block_args->main_fn(real_block_args->arg_ptr);
    uint64_t trigger = 1;
    write(real_block_args->event_h, (void*) &trigger, sizeof(trigger));
    _exit(0);
}

static void lwt_block_on_real_thread(void (*main_fn)(void*), void* arg_ptr) {
    // Prepare arguments.
    rio_t* event_h = rio_eventfd_create(0, false);
    lwt_heap_t* post_clone_heap;
    lwt_real_block_args_t* real_block_args = lwt_alloc_heaped_object(sizeof(lwt_real_block_args_t), &post_clone_heap);
    real_block_args->main_fn = main_fn;
    real_block_args->arg_ptr = arg_ptr;
    real_block_args->heap = post_clone_heap;
    real_block_args->event_h = rio_get_fd_write(event_h);
    // Allocate a small stack for the auxiliary thread.
    const size_t aux_thread_stack_size = 0x4000;
    void* stack_entry;
    void* stack_base = lwt_thread_stack_alloc(aux_thread_stack_size, &stack_entry);
    lwt_physical_thread_t* phys_thread = 0;
    int32_t clone_r = lwt_start_new_thread(lwt_real_block_main, stack_entry, lwt_get_thread_clone_flags(), real_block_args, &phys_thread);
    if (clone_r == -1)
        RCD_SYSCALL_EXCEPTION(clone, exception_fatal);
    int32_t waitpid_main_tid = clone_r;
    // Wait for the process nicely.
    try {
        rio_eventfd_wait(event_h);
    } finally {
        // Free kernel thread.
        lwt_thread_cancel_sync(waitpid_main_tid);
        // Free stack for auxiliary thread.
        lwt_thread_stack_free(stack_base, aux_thread_stack_size);
    }
}

typedef struct lwt_waitpid_main_args {
    int32_t pid;
    bool* out_success;
    int32_t* out_exit_code;
    int32_t* out_errno;
} lwt_waitpid_main_args_t;

static void lwt_waitpid_main(void* arg_ptr) {
    lwt_waitpid_main_args_t* waitpid_args = arg_ptr;
    restart_wait4:;
    siginfo_t si;
    // Our only purpose is to wait for the event where the subprocess to terminates.
    // Setting WNOWAIT allows us to be safely canceled and not garbage collect anything.
    int32_t waitid_r = waitid(P_PID, waitpid_args->pid, &si, WEXITED | WNOWAIT, 0);
    if (waitid_r == -1) {
        int32_t errno_v = errno;
        if (errno_v == EINTR)
            goto restart_wait4;
        *waitpid_args->out_success = false;
        *waitpid_args->out_errno = errno_v;
    } else {
        *waitpid_args->out_success = true;
        *waitpid_args->out_exit_code = si.__si_fields.__sigchld.si_status;
    }
}

int32_t lwt_waitpid(int32_t pid) { sub_heap {
    // Start a thread that can block on waitid() since linux does not provide an asynchronous facility for this.
    bool success;
    int32_t exit_code;
    int32_t ret_errno;
    lwt_waitpid_main_args_t waitpid_main_args;
    waitpid_main_args.pid = pid;
    waitpid_main_args.out_success = &success;
    waitpid_main_args.out_exit_code = &exit_code;
    waitpid_main_args.out_errno = &ret_errno;
    lwt_block_on_real_thread(lwt_waitpid_main, &waitpid_main_args);
    if (!success) {
        errno = ret_errno;
        RCD_SYSCALL_EXCEPTION(waitid, exception_io);
    }
    return exit_code;
}}

typedef struct lwt_flock_main_args {
    int32_t fd;
    int32_t operation;
    bool* out_success;
    int32_t* out_errno;
} lwt_flock_main_args_t;

static void lwt_flock_main(void* arg_ptr) {
    lwt_flock_main_args_t* flock_main_args = arg_ptr;
    restart_flock:;
    int32_t flock_r = flock(flock_main_args->fd, flock_main_args->operation);
    if (flock_r == -1) {
        int32_t errno_v = errno;
        if (errno_v == EINTR)
            goto restart_flock;
        *flock_main_args->out_success = false;
        *flock_main_args->out_errno = errno_v;
    } else {
        *flock_main_args->out_success = true;
    }
}

int32_t lwt_flock(int32_t fd, int32_t operation) { sub_heap {
    // Start a thread that can block on flock() since linux does not provide an asynchronous facility that allows us to do wait for this event.
    bool success;
    int32_t ret_errno;
    lwt_flock_main_args_t flock_main_args;
    flock_main_args.fd = fd;
    flock_main_args.operation = operation;
    flock_main_args.out_success = &success;
    flock_main_args.out_errno = &ret_errno;
    lwt_block_on_real_thread(lwt_flock_main, &flock_main_args);
    if (!success)
        errno = ret_errno;
    return success? 0: -1;
}}

fiber_main lwt_once_event_fiber(fiber_main_attr) { try {
    ifc_park();
} catch (exception_canceled, e); }

void lwt_once(lwt_once_t* once_ctrl, void (*init_fn)(void*), void* arg_ptr) {
    for (uint32_t n_spins = 0;;) {
        uint128_t once_ctrl_v;
        for (;;) {
            once_ctrl_v = once_ctrl->v;
            if (atomic_cas_uint128(&once_ctrl->v, once_ctrl_v, once_ctrl_v))
                break;
            sync_synchronize();
        }
        switch (once_ctrl_v) {{
        } case lwt_once_status_done: {
            return;
        } case lwt_once_status_init: {
            if (atomic_cas_uint128(&once_ctrl->v, lwt_once_status_init, lwt_once_status_spin)) {
                // We are chosen to call init_routine().
                sub_heap {
                    rcd_fid_t once_event_fid;
                    fmitosis {
                        once_event_fid = sfid(spawn_fiber(lwt_once_event_fiber("")));
                    }
                    bool cas_ok = atomic_cas_uint128(&once_ctrl->v, lwt_once_status_spin, once_event_fid);
                    assert(cas_ok);
                    try {
                        init_fn(arg_ptr);
                        // Init routine complete, let request through the once.
                        bool cas_ok = atomic_cas_uint128(&once_ctrl->v, once_event_fid, lwt_once_status_done);
                        assert(cas_ok);
                    } catch (exception_any, e) {
                        // Got some kind of cancellation, restore once control and forward exception.
                        // This behavior also makes us compatible with posix pthread_once:
                        // "However, if init_routine is a cancellation point and is canceled, the effect on once_control shall be as if pthread_once() was never called."
                        bool cas_ok = atomic_cas_uint128(&once_ctrl->v, once_event_fid, lwt_once_status_init);
                        assert(cas_ok);
                        lwt_throw_exception(e);
                    }
                }
            } else {
                case lwt_once_status_spin: {
                    // We where not chosen, spin yield while waiting to get a valid once event fid.
                    n_spins = atomic_spin_yield(n_spins);
                }
            }
            break;
        } default: {
            // Defer thread, waiting for the event fid to trigger by destroying itself.
            rcd_fid_t once_event_fid = once_ctrl_v;
            ifc_wait(once_event_fid);
            break;
        }}
    }
}

static void lwt_match_executor_count(uint32_t optimal_count) {
    lwt_start_cb_t executor_start_cb = {.start_fn = lwt_physical_executor_thread, .arg_ptr = 0};
    atomic_spinlock_lock(&lwt_executor_match_lock);
    uint32_t target_executor_thread_count = MIN(optimal_count, lwt_debug_max_worker_count);
    while (lwt_executor_thread_count < target_executor_thread_count) {
        lwt_physical_thread_t* phys_thread = lwt_start_physical_thread(executor_start_cb);
        global_heap {
            lwt_executor_thread_t* exec_thread = new(lwt_executor_thread_t);
            exec_thread->phys_thread = phys_thread;
            LL_PREPEND(lwt_executor_threads, exec_thread);
        }
        lwt_executor_thread_count++;
    }
    atomic_spinlock_unlock(&lwt_executor_match_lock);
}

uint32_t lwt_system_cpu_count() {
    fstr_t cpuinfo_path = "/proc/cpuinfo";
    uint32_t core_count = 0;
    sub_heap {
        fstr_t content = rio_read_virtual_file_contents(cpuinfo_path, fss(fstr_alloc(100 * PAGE_SIZE)));
        for (fstr_t scan_offs = content; scan_offs.len > 0;) {
#pragma re2c(scan_offs): (^|\n)processor\s*:\s+(\d+)\s*(\n|$) {@found_cpu}
            break;
            found_cpu:
            core_count++;
        }
    }
    return core_count;
}

void lwt_start_optimal_executor_count() {
    uint32_t optimal_count = lwt_system_cpu_count();
    lwt_match_executor_count(optimal_count > 4? optimal_count: 4);
}

void lwt_yield() {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_cancellation_point_raw(fiber);
    if (shared_fiber_mem.exec_block_queue.first != 0) {
        fiber->ctrl.done = false;
        lwt_scheduler_fiber_defer(true, 0, 0, -1);
        lwt_cancellation_point_raw(fiber);
    }
}

static void lwt_io_monitor_thread(void* arg_ptr) {
    // Rename the system fiber.
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    phys_thread->system_fiber.main_name = "[librcd I/O fiber]";
    const int epoll_events_count = PAGE_SIZE / sizeof(struct epoll_event);
    struct epoll_event epoll_events[epoll_events_count];
    for (;;) {
        // DBG("[io/*]: epoll_wait()");
        int epoll_r = epoll_wait(lwt_shared_epoll_fd, epoll_events, epoll_events_count, -1);
        if (epoll_r == -1) {
            if (errno == EINTR)
                continue;
            RCD_SYSCALL_EXCEPTION(epoll_wait, exception_fatal);
        }
        for (int i = 0; i < epoll_r; i++) {
            int32_t fd = epoll_events[i].data.fd;
            LWT_SYS_SPINLOCK_WLOCK(&shared_bfd_mem.rwlock); {
                // DBG("[io/", i2fs(fd), "]: epoll notified");
                hmap_bfd_lookup_t blu = hmap_bfd_lookup(&shared_bfd_mem.blocking_fd_map, fd, true);
                if (hmap_bfd_found(blu)) {
                    lwt_blocking_fd_t* blocking_fd = hmap_bfd_value(blu);
                    if (blocking_fd->is_epoll) {
                        // Due to how recursive epoll is implemented in the kernel the inner epoll must be waited on to consume the ready status.
                        // We must do it here to avoid race between the io-monitor thread and the corresponding fiber waking up.
                        struct epoll_event events[1];
                        int32_t epoll_wait_r = epoll_wait(fd, events, LENGTHOF(events), 0);
                        if (epoll_wait_r == -1 && errno != EINTR)
                            RCD_SYSCALL_EXCEPTION(epoll_wait, exception_fatal);
                    }
                    if ((epoll_events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) != 0) {
                        lwt_fiber_t* read_fiber = blocking_fd->read_fiber;
                        if (read_fiber != 0) {
                            // DBG("[io/", i2fs(fd), "]: waking read fiber");
                            lwt_scheduler_fiber_wake_done(read_fiber);
                            blocking_fd->read_fiber = 0;
                        } else {
                            // DBG("[io/", i2fs(fd), "]: pending read");
                            blocking_fd->read_ready = true;
                        }
                    }
                    if ((epoll_events[i].events & (EPOLLOUT | EPOLLERR | EPOLLHUP)) != 0) {
                        lwt_fiber_t* write_fiber = blocking_fd->write_fiber;
                        if (write_fiber != 0) {
                            // DBG("[io/", i2fs(fd), "]: waking write fiber");
                            lwt_scheduler_fiber_wake_done(write_fiber);
                            blocking_fd->write_fiber = 0;
                        } else {
                            // DBG("[io/", i2fs(fd), "]: pending write");
                            blocking_fd->write_ready = true;
                        }
                    }
                }
            } LWT_SYS_SPINLOCK_UNLOCK(&shared_bfd_mem.rwlock);
        }
    }
}

static void lwt_io_block(int fd, lwt_fd_event_t event, bool is_epoll) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_cancellation_point_raw(fiber);
    lwt_blocking_fd_t* blocking_fd;
    bool event_already_ready = false;
    bool epoll_ctrl_add_needed = false;
    const fstr_t* err_msg = 0;
    LWT_SYS_SPINLOCK_WLOCK(&shared_bfd_mem.rwlock); {
        hmap_bfd_lookup_t blu = hmap_bfd_lookup(&shared_bfd_mem.blocking_fd_map, fd, true);
        if (!hmap_bfd_found(blu)) {
            blocking_fd = lwt_blocking_fd_allocate();
            blocking_fd->read_ready = false;
            blocking_fd->read_fiber = (event == lwt_fd_event_read? fiber: 0);
            blocking_fd->write_ready = false;
            blocking_fd->write_fiber = (event == lwt_fd_event_write? fiber: 0);
            blocking_fd->is_epoll = is_epoll;
            hmap_bfd_insert(&shared_bfd_mem.blocking_fd_map, blu, fd, blocking_fd);
            epoll_ctrl_add_needed = true;
        } else {
            blocking_fd = hmap_bfd_value(blu);
            if (event == lwt_fd_event_read) {
                if (blocking_fd->read_ready) {
                    event_already_ready = true;
                    blocking_fd->read_ready = false;
                } else {
                    if (blocking_fd->read_fiber != 0) {
                        static const fstr_t err_cannot_read_block = "cannot read block on file descriptor: already read blocked by another fiber";
                        err_msg = &err_cannot_read_block;
                        break;
                    }
                    blocking_fd->read_fiber = fiber;
                }
            } else {
                if (blocking_fd->write_ready) {
                    event_already_ready = true;
                    blocking_fd->write_ready = false;
                } else {
                    if (blocking_fd->write_fiber != 0) {
                        static const fstr_t err_cannot_write_block = "cannot write block on file descriptor: already write blocked by another fiber";
                        err_msg = &err_cannot_write_block;
                        break;
                    }
                    blocking_fd->write_fiber = fiber;
                }
            }
        }
        // Prevents race (defer bounces if edge level event is triggered before it).
        fiber->ctrl.done = false;
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_bfd_mem.rwlock);
    if (err_msg != 0)
        throw(*err_msg, exception_io);
    if (epoll_ctrl_add_needed) {
        struct epoll_event event = {.events = EPOLLIN | EPOLLOUT | EPOLLET, .data.fd = fd};
        int epoll_ctl_r = epoll_ctl(lwt_shared_epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if (epoll_ctl_r == -1)
            RCD_SYSCALL_EXCEPTION(epoll_ctl, exception_io);
        // DBG("[io/", i2fs(fd), "]: EPOLL_CTL_ADD");
    }
    if (event_already_ready)
        return;
    // DBG("[io/", i2fs(fd), "]: fiber deferred");
    lwt_scheduler_fiber_defer(false, 0, 0, fd);
    // If we're still attached to the blocking fd we detach now. If we woke up
    // due to an I/O event the I/O thread should have detached us already but
    // we might also have woken up due to cancellation.
    LWT_SYS_SPINLOCK_WLOCK(&shared_bfd_mem.rwlock); {
        if (event == lwt_fd_event_read) {
            if (blocking_fd->read_fiber == fiber)
                blocking_fd->read_fiber = 0;
        } else {
            if (blocking_fd->write_fiber == fiber)
                blocking_fd->write_fiber = 0;
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_bfd_mem.rwlock);
    lwt_cancellation_point_raw(fiber);
}


void lwt_block_until_edge_level_io_event(int fd, lwt_fd_event_t event) {
    return lwt_io_block(fd, event, false);
}

void lwt_block_until_epoll_ready(int fd, lwt_fd_event_t event) {
    return lwt_io_block(fd, event, true);
}

void lwt_io_free_fd_tracking(int fd) {
    LWT_SYS_SPINLOCK_WLOCK(&shared_bfd_mem.rwlock); {
        hmap_bfd_lookup_t blu = hmap_bfd_lookup(&shared_bfd_mem.blocking_fd_map, fd, true);
        if (hmap_bfd_found(blu)) {
            lwt_blocking_fd_t* blocking_fd = hmap_bfd_value(blu);
            hmap_bfd_delete(&shared_bfd_mem.blocking_fd_map, blu);
            lwt_blocking_fd_free(blocking_fd);
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_bfd_mem.rwlock);
    // DBG("[io/", i2fs(fd), "]: tracking free'd");
}

fiber_main lwt_program_main(fiber_main_attr, list(fstr_t)* main_args, list(fstr_t)* main_env) {
    // Enable parallelism.
    lwt_start_optimal_executor_count();
#ifdef RCD_SELF_TEST
    // Run advanced rcd self tests.
    rcd_advanced_self_test(main_args, main_env);
#else
    // Run rcd main.
    rcd_main(main_args, main_env);
#endif
}

__attribute__((weak))
int32_t lwt_init_process(int argc, char** argv, char** env) {
    throw("this librcd program cannot run as an init process", exception_fatal);
}

static void lwt_init_program_cmdline_mem(int argc, char** argv, char** env) {
    // We know that the arguments and environments is allocated in one single contiguous chunk from arg 0 to the last environment variable.
    void* arg_mem_start = (void*) argv[0];
    for (size_t i = 0;; i++) {
        if (env[i] == 0) {
            void* last_str = (i != 0? env[i - 1]: (argc != 0? argv[argc - 1]: 0));
            lwt_program_cmdline_mem.str = arg_mem_start;
            lwt_program_cmdline_mem.len = (last_str != 0)? (last_str - arg_mem_start) + strlen(last_str) + 1: 0;
            return;
        }
    }
}

void __lwthreads_main(int argc, char** argv, char** env) {
    // Set up archaic physical thread struct while initializing vm.
    lwt_setup_archaic_physical_thread();
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    // Initialize the root thread pid.
    phys_thread->pid = getpid();
    // Initialize the global heap.
    lwt_global_heap.heap = vm_heap_create(0);
    // Register the main thread as just another physical thread with associated fiber.
    // This removes a corner case where the current phys thread does not have a fiber which would break many library functions.
    phys_thread->linux_tid = gettid();
    lwt_setup_physical_thread();
    phys_thread->system_rwspinlock = 0;
    // Initialize shared data structure trees.
    hmap_fid_init(&shared_fiber_mem.fiber_map);
    hmap_bfd_init(&shared_bfd_mem.blocking_fd_map);
    // Initialize global program path.
    global_heap {
        lwt_program_path = fstr_from_cstr(argv[0]);
    }
    // Initialize program arguments memory range.
    lwt_init_program_cmdline_mem(argc, argv, env);
    // Initialize standard signal handling.
    rsig_init();
    // We branch of from normal execution here if librcd is running as init.
    if (phys_thread->pid == 1)
        exit_group(lwt_init_process(argc, argv, env));
    // Creating main epoll file descriptor that schedules all asynchronous I/O.
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1)
        RCD_SYSCALL_EXCEPTION(epoll_create1, exception_fatal);
    lwt_shared_epoll_fd = epoll_fd;
    // Create janitor thread for vm.
    lwt_physical_thread_t* janitor_phys_thread;
    {
        lwt_start_cb_t janitor_start_cb = {.start_fn = vm_janitor_thread, .arg_ptr = 0};
        janitor_phys_thread = lwt_start_physical_thread(janitor_start_cb);
        vm_janitor_notify_ptid(janitor_phys_thread->pid);
    }
    // Initialize stdio compatibility.
    stdio_init();
#ifdef RCD_SELF_TEST
    // Run rcd self test which tests primitives and then returns
    // before testing advanced stuff so we can dedicate this thread to I/O.
    if (argc <= 1)
        rcd_primitive_self_test();
#endif
    // Create physical executor for main fiber.
    lwt_match_executor_count(1);
    // Spawn main fiber.
    fmitosis {
        list(fstr_t)* main_args = new_list(fstr_t);
        for (int i = 1; i < argc; i++)
            list_push_end(main_args, fstr_t, fss(fstr_from_cstr(argv[i])));
        list(fstr_t)* main_env = new_list(fstr_t);
        for (int i = 0; env[i] != 0; i++)
            list_push_end(main_env, fstr_t, fss(fstr_from_cstr(env[i])));
        spawn_static_fiber(lwt_program_main("[main]", main_args, main_env));
    }
    // We use the main thread as the i/o monitoring thread.
    return lwt_io_monitor_thread(0);
}

void lwt_cancellation_point() {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_cancellation_point_raw(fiber);
}

bool lwt_is_cancel_pending() {
    LWT_GET_LOCAL_FIBER(fiber);
    return fiber->ctrl.canceled != 0;
}

static bool lwt_has_live_join_race_from_join_event(lwt_edata_ifc_join_t* ifc_join_origin) {
    for (lwt_edata_ifc_join_t* ifc_join = ifc_join_origin;; ifc_join = ifc_join->prev) {
        if (ifc_join == 0)
            return false;
        if (ifc_join->ifc_client->live_join_race)
            return true;
    }
}

void lwt_test_live_join_race() {
    LWT_GET_LOCAL_FIBER(fiber);
    if (lwt_has_live_join_race_from_join_event(fiber->current_ifc_join_event))
        lwt_throw_join_race_exception(fiber);
}

void lwt_exit(int status) {
    exit_group(status);
    unreachable();
}

// Implements the external __errno_location specified in linux.h as a macro for errno here as it relies on thread local storage which only lwthreads.c can define.
int* __errno_location() {
    LWT_GET_LOCAL_FIBER(fiber);
    return &fiber->local_errno;
}

void __rcd_escape_sh_txn(lwt_heap_t** __rcd_txn_aheap) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_heap_t* heap = *__rcd_txn_aheap;
    vm_heap_import_all(fiber->current_heap, heap->vm_heap, true);
    lwt_alloc_free(heap);
}

void __rcd_escape_gh_txn(lwt_heap_t** __rcd_global_heap_tail) {
    if (vm_heap_has_allocs((*__rcd_global_heap_tail)->vm_heap)) {
        // Append the tail to the global heap.
        LWT_SYS_SPINLOCK_WLOCK(&lwt_global_heap.rwlock); {
            // We could specify a sub heap to explicitly require global_heap_tail to be a member of here, but since the call to this function is auto generated we don't really have to.
            vm_heap_import(0, lwt_global_heap.heap, *__rcd_global_heap_tail);
        } LWT_SYS_SPINLOCK_UNLOCK(&lwt_global_heap.rwlock);
    }
}

void __lwt_fiber_stack_push_sub_heap() {
    LWT_GET_LOCAL_FIBER(fiber);
    fiber->current_heap = vm_heap_create(fiber->current_heap);
    lwt_fiber_event_data_t edata;
    lwt_fiber_event_push(fiber, lwt_fiber_event_sub_heap, edata);
}

void __lwt_fiber_stack_pop_sub_heap(void* arg_ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_pop(fiber, lwt_fiber_event_sub_heap);
    fiber->current_heap = vm_heap_release(fiber->current_heap, 0, 0);
}

void __lwt_fiber_stack_push_switch_heap(lwt_heap_t* fiber_heap) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata;
    edata.switch_heap.prev_heap = fiber->current_heap;
    fiber->current_heap = fiber_heap->vm_heap;
    lwt_fiber_event_push(fiber, lwt_fiber_event_switch_heap, edata);
}

void __lwt_fiber_stack_pop_switch_heap(void* arg_ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata = lwt_fiber_event_pop(fiber, lwt_fiber_event_switch_heap);
    fiber->current_heap = edata.switch_heap.prev_heap;
}

rcd_fid_t __lwt_fiber_stack_push_mitosis() {
    LWT_GET_LOCAL_FIBER(fiber);
    // Create the new inactive fiber and index it.
    vm_heap_t* new_heap = vm_heap_create(0);
    lwt_fiber_t* new_fiber = lwt_fiber_allocate();
    register rcd_fid_t new_fiber_id;
    {
        static rcd_fid_t fiber_id_counter = LWT_INIT_FIBER_ID + 1;
        for (;;) {
            sync_synchronize();
            new_fiber_id = fiber_id_counter;
            register rcd_fid_t new_fiber_id_counter = new_fiber_id + 1;
            if (atomic_cas_uint128(&fiber_id_counter, new_fiber_id, new_fiber_id_counter))
                break;
        }
    }
    new_fiber->ctrl.id = new_fiber_id;
    new_fiber->ctrl.canceled = 0;
    new_fiber->ctrl.hidden_cancel = false;
    new_fiber->ctrl.done = false;
    new_fiber->ctrl.unintr = false;
    new_fiber->ctrl.join_race = false;
    new_fiber->ctrl.deferred = false;
    new_fiber->ctrl.started = false;
    new_fiber->main_name = "[librcd fiber in mitosis]";
    new_fiber->instance_name = "";
    new_fiber->stack_alloc_stack = 0;
    new_fiber->current_stacklet = 0;
    new_fiber->current_heap = new_heap;
    rbtree_init(&new_fiber->ifc_fn_queues, lwt_cmp_ifc_fn_queues);
    new_fiber->defer_wait_fid = 0;
    new_fiber->defer_wait_fd = -1;
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        hmap_fid_lookup_t lu = hmap_fid_lookup(&shared_fiber_mem.fiber_map, new_fiber_id, false);
        hmap_fid_insert(&shared_fiber_mem.fiber_map, lu, new_fiber_id, new_fiber);
        DL_APPEND(shared_fiber_mem.fiber_list, new_fiber);
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    // Push mitosis fiber event unto the event stack.
    lwt_fiber_event_data_t edata;
    edata.mitosis = lwt_edata_mitosis_allocate();
    edata.mitosis->post_mitosis_heap = fiber->current_heap;
    edata.mitosis->new_fiber = new_fiber;
    lwt_fiber_event_push(fiber, lwt_fiber_event_mitosis, edata);
    // Switch to the new heap before returning the new fiber id.
    fiber->current_heap = new_heap;
    return new_fiber_id;
}

rcd_fid_t __lwt_fiber_stack_pop_mitosis(void (*start_fn)(void *), void* arg_ptr, fstr_t fiber_name, const struct lwt_fiber_options* fiber_options) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata = lwt_fiber_event_pop(fiber, lwt_fiber_event_mitosis);
    // Prepare the fiber with the attributes it should start with.
    lwt_fiber_t* new_fiber = edata.mitosis->new_fiber;
    rcd_fid_t new_fiber_id = new_fiber->ctrl.id;
    new_fiber->main_name = fiber_options->name;
    new_fiber->est_stack_size = fiber_options->est_stack_size;
    new_fiber->instance_name = fiber_name;
    // While the fiber is not started we steal this fields to store the start_fn and arg_ptr.
    new_fiber->event_stack = (void*) start_fn;
    new_fiber->current_ifc_join_event = (void*) arg_ptr;
    // Enqueue the new fiber as execution blocking, so pending physical thread can start working on it asap.
    // DBG("[lwt] inserting fiber: #", DBG_PTR(new_fiber->ctrl->id));
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        lwt_scheduler_exec_block_enqueue(new_fiber);
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    // Pop the mitosis event from the event heap.
    fiber->current_heap = edata.mitosis->post_mitosis_heap;
    lwt_edata_mitosis_free(edata.mitosis);
    assert(fiber->current_heap != 0);
    return new_fiber_id;
}

/// Main functions for fibers that had their mitosis aborted.
static void lwt_zombie_fiber_main(void* arg_ptr) {}

void __lwt_fiber_stack_pop_mitosis_and_abort(uint8_t* _rcd_mitosis_used) {
    if (*_rcd_mitosis_used != 0)
        return;
    struct lwt_fiber_options fiber_options = {.name = ""};
    __lwt_fiber_stack_pop_mitosis(lwt_zombie_fiber_main, 0, "[librcd zombie fiber]", &fiber_options);
}

static lwt_ifc_fn_queue_t* ref_ifc_fn_queue(lwt_fiber_t* server_fiber, void* ifc_fn_ptr) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    lwt_ifc_fn_queue_t* ifc_fn_queue = RBTREE_LOOKUP_KEY(lwt_ifc_fn_queue_t, rb_node, &server_fiber->ifc_fn_queues, .fn_ptr = ifc_fn_ptr);
    if (ifc_fn_queue == 0) {
        ifc_fn_queue = lwt_ifc_fn_queue_allocate();
        ifc_fn_queue->fn_ptr = ifc_fn_ptr;
        ifc_fn_queue->ifc_server_ref_count = 0;
        ifc_fn_queue->freeish_servers = 0;
        ifc_fn_queue->sharedish_servers = 0;
        ifc_fn_queue->server_fiber = server_fiber;
        ifc_fn_queue->waiting_clients.first = 0;
        ifc_fn_queue->waiting_clients.last = 0;
        rbtree_insert(&ifc_fn_queue->rb_node, &server_fiber->ifc_fn_queues);
    }
    return ifc_fn_queue;
}

static void deref_ifc_fn_queue(lwt_ifc_fn_queue_t* ifc_fn_queue) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    if (ifc_fn_queue->ifc_server_ref_count == 0 && ifc_fn_queue->waiting_clients.first == 0) {
        if (ifc_fn_queue->server_fiber != 0)
            rbtree_remove(&ifc_fn_queue->rb_node, &ifc_fn_queue->server_fiber->ifc_fn_queues);
        lwt_ifc_fn_queue_free(ifc_fn_queue);
    }
}

static lwt_ifc_client_t* lwt_ifc_dequeue_waiting_client(lwt_ifc_fn_queue_t* ifc_fn_queue) {
    return QUEUE_DEQUEUE(&ifc_fn_queue->waiting_clients);
}

static lwt_ifc_fn_queue_t* lwt_ifc_get_next_waiting_client_queue(lwt_ifc_fn_queue_t** ifc_fn_queues, size_t n_ifc_fn_queues) {
    assert(ATOMIC_RWLOCK_IS_WLOCKED(shared_fiber_mem.rwlock));
    // Find the next waiting client (with the lowest call id = has waited the longest).
    lwt_ifc_client_t* next_waiting_client;
    lwt_ifc_fn_queue_t* next_ifc_fn_queue = 0;
    for (size_t i = 0; i < n_ifc_fn_queues; i++) {
        lwt_ifc_fn_queue_t* ifc_fn_queue = ifc_fn_queues[i];
        lwt_ifc_client_t* waiting_client = ifc_fn_queue->waiting_clients.first;
        if (waiting_client != 0 && (next_ifc_fn_queue == 0 || waiting_client->call_id < next_waiting_client->call_id)) {
            next_waiting_client = waiting_client;
            next_ifc_fn_queue = ifc_fn_queue;
        }
    }
    return next_ifc_fn_queue;
}

/// Defers the current fiber by accepting one or more requests for the specified ifc.
void __lwt_ifc_accept(void** fn_list, size_t n_fn_list, void* server_state, bool is_auto_reaccepting, bool is_server_side_accept) {
    // If not accepting on any functions we return immediately.
    if (n_fn_list == 0)
        return;
    assert(server_state != 0);
    LWT_GET_LOCAL_FIBER(fiber);
    // This function is a cancellation point so check here to save cycles.
    lwt_cancellation_point_raw(fiber);
    // Evaluate the accepting fiber which may be the server when accepting in a join.
    lwt_fiber_t* accepting_fiber;
    if (is_server_side_accept) {
        // We're accepting from the server side.
        // Essentially other fiber joins the servers fid instead of the clients fid and the server fid will end up having additional fiber queues for other functions.
        // When actually running there is no difference in practice as the server and client was already joined before the call, this is just a matter of indexing/referencing.
        if (fiber->current_ifc_join_event == 0)
            throw("attempted to server side accept without being joined first", exception_arg);
        accepting_fiber = fiber->current_ifc_join_event->ifc_client->ifc_server->fiber;
    } else {
        accepting_fiber = fiber;
    }
    // Ref all relevant ifc fn queues.
    lwt_ifc_fn_queue_t* ifc_fn_queues[n_fn_list];
    size_t n_final_fns = 0;
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        for (size_t i = 0; i < n_fn_list; i++) {
            void* fn_ptr = fn_list[i];
            if (fn_ptr == 0)
                continue;
            lwt_ifc_fn_queue_t* ifc_fn_queue = ref_ifc_fn_queue(accepting_fiber, fn_ptr);
            ifc_fn_queue->ifc_server_ref_count++;
            ifc_fn_queues[n_final_fns] = ifc_fn_queue;
            n_final_fns++;
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    // If not accepting on any functions we return immediately.
    if (n_final_fns == 0)
        return;
    // Initialize ifc server references.
    lwt_ifc_server_t ifc_server;
    ifc_server.fiber = fiber;
    ifc_server.heap = fiber->current_heap;
    ifc_server.running_clients = 0;
    lwt_ifc_server_ref_t ifc_server_refs[n_final_fns];
    for (size_t i = 0; i < n_final_fns; i++) {
        ifc_server_refs[i].server = &ifc_server;
        ifc_server_refs[i].head = 0;
    }
    // Accept clients. The server heap cannot be used beyond this point since
    // the client owns it until it's disconnected.
    do {
        LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
            lwt_ifc_fn_queue_t* waiting_client_queue = lwt_ifc_get_next_waiting_client_queue(ifc_fn_queues, n_final_fns);
            if (waiting_client_queue != 0) {
                // Found client, attach and wake it.
                lwt_ifc_client_t* waiting_client = lwt_ifc_dequeue_waiting_client(waiting_client_queue);
                waiting_client->server_state = server_state;
                waiting_client->ifc_server = &ifc_server;
                lwt_scheduler_fiber_wake_done_raw(waiting_client->fiber);
                DL_PREPEND(ifc_server.running_clients, waiting_client);
                if (waiting_client->call_type == RCD_IFC_CALL_SHARED) {
                    // If we accept a shared join we can join with all other shared clients that are next in line.
                    for (;;) {
                        lwt_ifc_fn_queue_t* waiting_client_queue = lwt_ifc_get_next_waiting_client_queue(ifc_fn_queues, n_final_fns);
                        if (waiting_client_queue == 0) {
                            // Listen for future shared client calls.
                            ifc_server.server_state = server_state;
                            ifc_server.read_locked_state = true;
                            goto reconnect_ifc_server_references;
                        }
                        lwt_ifc_client_t* waiting_client = waiting_client_queue->waiting_clients.first;
                        if (waiting_client->call_type != RCD_IFC_CALL_SHARED) {
                            // Throw away the server state to prevent the next locked join from being starved.
                            ifc_server.server_state = 0;
                            break;
                        }
                        lwt_ifc_dequeue_waiting_client(waiting_client_queue);
                        waiting_client->server_state = server_state;
                        waiting_client->ifc_server = &ifc_server;
                        lwt_scheduler_fiber_wake_done_raw(waiting_client->fiber);
                        DL_PREPEND(ifc_server.running_clients, waiting_client);
                    }
                } else {
                    // We're locked joining - don't share the server state.
                    ifc_server.server_state = 0;
                }
            } else {
                // Listen for any future client calls.
                ifc_server.server_state = server_state;
                ifc_server.read_locked_state = false;
                reconnect_ifc_server_references: {
                    // Re-connect all ifc server references so clients can find us.
                    for (size_t i = 0; i < n_final_fns; i++) {
                        lwt_ifc_fn_queue_t* ifc_fn_queue = ifc_fn_queues[i];
                        lwt_ifc_server_ref_t** new_head = (ifc_server.read_locked_state? &ifc_fn_queue->sharedish_servers: &ifc_fn_queue->freeish_servers);
                        lwt_ifc_server_ref_t* ifc_server_ref = &ifc_server_refs[i];
                        if (ifc_server_ref->head != 0) {
                            if (ifc_server_ref->head == new_head)
                                continue;
                            DL_DELETE(*ifc_server_ref->head, ifc_server_ref);
                        }
                        DL_PREPEND(*new_head, ifc_server_ref);
                        ifc_server_ref->head = new_head;
                    }
                }
            }
            // Either waiting for clients to connect or complete at this point.
            fiber->ctrl.done = false;
        } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
        // Wait for new clients to attach and complete and/or old clients to complete or cancellation.
        lwt_scheduler_fiber_defer(false, &ifc_server, accepting_fiber->ctrl.id, -1);
        if (!fiber->ctrl.unintr && (fiber->ctrl.canceled != 0 || fiber->ctrl.join_race)) {
            // Was canceled/join raced but can't really leave until all running clients are not running anymore so we'll remember that and wait for the clients.
            rcd_exception_t* pending_canceled = 0;
            bool pending_join_race = false;
            for (;;) {
                bool got_running_clients = false;
                LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                    // Consume a pending cancellation if we don't already have one.
                    // The hidden cancel flag ensures that we can get no more cancellations if one is already pending.
                    if (pending_canceled == 0) {
                        pending_canceled = fiber->ctrl.canceled;
                        if (pending_canceled != 0) {
                            fiber->ctrl.canceled = 0;
                            fiber->ctrl.hidden_cancel = true;
                        }
                    }
                    // Consume any pending join race. Unless we remove the flag we will be woken again.
                    pending_join_race = pending_join_race || fiber->ctrl.join_race;
                    fiber->ctrl.join_race = false;
                    // Stop any listening.
                    ifc_server.server_state = 0;
                    // Enable live join race on all running clients to make them abort request processing asap.
                    got_running_clients = (ifc_server.running_clients != 0);
                    if (got_running_clients) {
                        lwt_ifc_client_t* running_client;
                        DL_FOREACH(ifc_server.running_clients, running_client) {
                            running_client->live_join_race = true;
                            lwt_scheduler_fiber_wake_join_race_raw(running_client->fiber);
                        }
                        // We need to defer afterwards, waiting for all clients to exit.
                        fiber->ctrl.done = false;
                    }
                } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
                // No more running clients, we can leave.
                if (!got_running_clients)
                    break;
                // Wait for all running clients to detach.
                lwt_scheduler_fiber_defer(false, &ifc_server, accepting_fiber->ctrl.id, -1);
            }
            // Restore cancel/join race and leave accept.
            if (pending_canceled != 0 || pending_join_race) {
                LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                    if (pending_canceled != 0) {
                        fiber->ctrl.canceled = pending_canceled;
                        fiber->ctrl.hidden_cancel = false;
                    }
                    fiber->ctrl.join_race = pending_join_race;
                } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
            }
            break;
        }
    } while (is_auto_reaccepting);
    // Reference cleanup.
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        for (size_t i = 0; i < n_final_fns; i++) {
            // Disconnect ifc server reference if it's referenced.
            lwt_ifc_server_ref_t* ifc_server_ref = &ifc_server_refs[i];
            if (ifc_server_ref->head != 0)
                DL_DELETE(*ifc_server_ref->head, ifc_server_ref);
            // Deref from function queue.
            lwt_ifc_fn_queue_t* ifc_fn_queue = ifc_fn_queues[i];
            ifc_fn_queue->ifc_server_ref_count--;
            deref_ifc_fn_queue(ifc_fn_queue);
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    // Raise any pending cancel here.
    lwt_cancellation_point_raw(fiber);
}

/// If the ifc server is not read locked it is free. Free ifc servers are always re-indexed in the freeish servers set so we should never find free ifc servers in the sharedish set.
static void lwt_ifc_trigger_free_sharedish_error() {
    throw("lwthreads corrupt state: free state indexed as sharedish or shared state indexed as freeish (impossible condition)", exception_fatal);
}

VM_DEFINE_FREE_LIST_ALLOCATOR_FN(lwt_ifc_client_t, lwt_ifc_client_allocate, lwt_ifc_client_free, true);

void* __lwt_fiber_stack_push_ifc_call_join(void* ifc_fn_ptr, rcd_fid_t fiber_id, rcd_ifc_call_type_t call_type) {
    LWT_GET_LOCAL_FIBER(fiber);
    //ifc-dbg// DBG("client ", DBG_PTR(fiber), " is pushing join");
    // This function is a cancellation point so check here to save cycles.
    lwt_cancellation_point_raw(fiber);
    // Ref the ifc queue and check if we can attach to an existing server state immediately.
    lwt_ifc_fn_queue_t* ifc_fn_queue = 0;
    lwt_ifc_client_t* ifc_client = lwt_ifc_client_allocate();
    ifc_client->fiber = fiber;
    ifc_client->live_join_race = false;
    ifc_client->call_type = call_type;
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        hmap_fid_lookup_t lu = hmap_fid_lookup(&shared_fiber_mem.fiber_map, fiber_id, true);
        if (!hmap_fid_found(lu))
            break;
        lwt_fiber_t* server_fiber = hmap_fid_value(lu);
        ifc_fn_queue = ref_ifc_fn_queue(server_fiber, ifc_fn_ptr);
        if (call_type == RCD_IFC_CALL_SHARED) {
            for (;;) {
                lwt_ifc_server_ref_t* ifc_server_ref = ifc_fn_queue->sharedish_servers;
                if (ifc_server_ref == 0)
                    break;
                lwt_ifc_server_t* ifc_server = ifc_server_ref->server;
                void* server_state = ifc_server->server_state;
                if (server_state == 0) {
                    // Disconnect ifc server reference as it's invalid.
                    assert(ifc_server_ref->head == &ifc_fn_queue->sharedish_servers);
                    DL_DELETE(ifc_fn_queue->sharedish_servers, ifc_server_ref);
                    ifc_server_ref->head = 0;
                } else {
                    if (!ifc_server->read_locked_state)
                        lwt_ifc_trigger_free_sharedish_error();
                    // Use the server state.
                    ifc_client->server_state = server_state;
                    ifc_client->ifc_server = ifc_server;
                    DL_APPEND(ifc_server->running_clients, ifc_client);
                    goto client_aquired_server_state;
                }
            }
        }
        for (;;) {
            lwt_ifc_server_ref_t* ifc_server_ref = ifc_fn_queue->freeish_servers;
            if (ifc_server_ref == 0) {
                if (call_type == RCD_IFC_CALL_LOCK_SERVER) {
                    // Find one legitimate shared server and stop it from accepting more shared clients to prevent starvation.
                    for (bool starvation_possible = true; starvation_possible;) {
                        lwt_ifc_server_ref_t* ifc_server_ref = ifc_fn_queue->sharedish_servers;
                        if (ifc_server_ref == 0)
                            break;
                        lwt_ifc_server_t* ifc_server = ifc_server_ref->server;
                        if (ifc_server->server_state != 0) {
                            if (!ifc_server->read_locked_state)
                                lwt_ifc_trigger_free_sharedish_error();
                            // Prevent this shared ifc server from accepting more shared clients by clearing the server state.
                            ifc_server->server_state = 0;
                            starvation_possible = false;
                        }
                        // Disconnect ifc server reference as it's not valid (now).
                        assert(ifc_server_ref->head == &ifc_fn_queue->sharedish_servers);
                        DL_DELETE(ifc_fn_queue->sharedish_servers, ifc_server_ref);
                        ifc_server_ref->head = 0;
                    }
                }
                break;
            }
            lwt_ifc_server_t* ifc_server = ifc_server_ref->server;
            void* server_state = ifc_server->server_state;
            if (server_state == 0) {
                // Disconnect ifc server reference as it's no longer valid.
                assert(ifc_server_ref->head == &ifc_fn_queue->freeish_servers);
                DL_DELETE(ifc_fn_queue->freeish_servers, ifc_server_ref);
                ifc_server_ref->head = 0;
            } else {
                if (ifc_server->read_locked_state) {
                    // Reconnect the ifc server reference to the sharedish servers instead.
                    assert(ifc_server_ref->head == &ifc_fn_queue->freeish_servers);
                    DL_DELETE(ifc_fn_queue->freeish_servers, ifc_server_ref);
                    DL_APPEND(ifc_fn_queue->sharedish_servers, ifc_server_ref);
                    ifc_server_ref->head = &ifc_fn_queue->sharedish_servers;
                    // We can use this ifc server now if we are doing a shared join, otherwise continue search.
                    if (call_type != RCD_IFC_CALL_SHARED)
                        continue;
                } else if (call_type == RCD_IFC_CALL_SHARED) {
                    // We're shared joining - read lock the server state.
                    ifc_server->read_locked_state = true;
                } else {
                    // We're locked joining - don't share the server state.
                    ifc_server->server_state = 0;
                }
                // Use the server state.
                ifc_client->server_state = server_state;
                ifc_client->ifc_server = ifc_server;
                DL_APPEND(ifc_server->running_clients, ifc_client);
                goto client_aquired_server_state;
            }
        }
        // No server state was ready to be joined with.
        ifc_client->server_state = 0;
        // Assign unique call id of the join so we can be ordered across different join queues.
        ifc_client->call_id = shared_fiber_mem.ifc_call_id;
        shared_fiber_mem.ifc_call_id++;
        // Add ourself to waiting clients.
        QUEUE_ENQUEUE(&ifc_fn_queue->waiting_clients, ifc_client);
        fiber->ctrl.done = false;
        client_aquired_server_state:;
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    // If server fiber doesn't exist we throw a race exception or no such fiber exception depending on if the fiber is interruptible or not.
    if (ifc_fn_queue == 0) {
        // Free the ifc client and forward run time to exception handler.
        lwt_ifc_client_free(ifc_client);
        if (fiber->ctrl.unintr) {
            // Uninterruptable join failed - the target fiber did not exist.
            lwt_throw_no_such_fiber_exception();
        } else {
            // Join failed - the target fiber did not exist causing indistinguishable join race.
            lwt_throw_join_race_exception(fiber);
        }
        assert(false);
    }
    // If we didn't catch a server state we need to sleep until we're attached automatically (by server) or canceled.
    if (ifc_client->server_state == 0) {
        // Wait until server attaches us or until we're canceled, join raced, or if we're uninterruptable - simply done.
        lwt_scheduler_fiber_defer(false, 0, fiber_id, -1);
        // If we're attached we where dequeued and added to running clients automatically.
        if (ifc_client->server_state == 0) {
            if (!fiber->ctrl.unintr && fiber->ctrl.canceled == 0 && !fiber->ctrl.join_race)
                throw("scheduler error: client woke up without any satisfied preconditions", exception_fatal);
            LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
                // The outside check is just an optimization, we need to check server state again
                // with lock to ensure there's no race between checking and removing us from the waiting clients queue.
                if (ifc_client->server_state == 0) {
                    // Canceled - step out of waiting clients queue and deref ifc function queue.
                    QUEUE_STEP_OUT(&ifc_fn_queue->waiting_clients, ifc_client);
                    deref_ifc_fn_queue(ifc_fn_queue);
                }
            } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
            // Server state might be set now if we had a race between the unprotected (optimization check) and the inner check.
            // If then server already accepted us we're just another joined fiber and have to jump through the same hoops any other joined fibers have to jump through to pop the join,
            // so we don't do any special optimizations for popping joins here. They will simply be immediately unwound with the cancellation point below.
            if (ifc_client->server_state == 0) {
                // Free the ifc client and forward run time to exception handler.
                lwt_ifc_client_free(ifc_client);
                if (fiber->ctrl.unintr) {
                    // Uninterruptable join failed - the target fiber died.
                    lwt_throw_no_such_fiber_exception();
                } else {
                    // Throw cancellation or join race exception now.
                    lwt_cancellation_point_raw(fiber);
                }
                assert(false);
            }
        }
        assert(ifc_client->ifc_server != 0);
    }
    lwt_fiber_event_data_t edata;
    edata.ifc_call_join = lwt_edata_ifc_join_allocate();
    edata.ifc_call_join->ifc_client = ifc_client;
    edata.ifc_call_join->call_type = call_type;
    edata.ifc_call_join->prev = fiber->current_ifc_join_event;
    fiber->current_ifc_join_event = edata.ifc_call_join;
    lwt_fiber_event_push(fiber, lwt_fiber_event_ifc_join, edata);
    // Raise any pending cancel here.
    lwt_cancellation_point_raw(fiber);
    // Return server state.
    return ifc_client->server_state;
}

void __lwt_fiber_stack_pop_ifc_call_join() {
    LWT_GET_LOCAL_FIBER(fiber);
    //ifc-dbg// DBG("client ", DBG_PTR(fiber), ": joined complete, popping");
    lwt_fiber_event_data_t edata = lwt_fiber_event_pop(fiber, lwt_fiber_event_ifc_join);
    lwt_ifc_client_t* ifc_client = edata.ifc_call_join->ifc_client;
    lwt_ifc_server_t* ifc_server = ifc_client->ifc_server;
    fiber->current_ifc_join_event = edata.ifc_call_join->prev;
    lwt_edata_ifc_join_free(edata.ifc_call_join);
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        assert(ifc_server->running_clients != 0);
        DL_DELETE(ifc_server->running_clients, ifc_client);
        // Wake the server and throw away any shared server state if this was the last client.
        if (ifc_server->running_clients == 0) {
            ifc_server->server_state = 0;
            lwt_scheduler_fiber_wake_done_raw(ifc_server->fiber);
        }
        // If only had an inner join race that applied to this join we can defuse it here as we
        // successfully unjoined before ever noticing that we had a join race.
        if (fiber->ctrl.join_race && ifc_client->live_join_race) {
            if (!lwt_has_live_join_race_from_join_event(fiber->current_ifc_join_event))
                fiber->ctrl.join_race = false;
        }
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    lwt_ifc_client_free(ifc_client);
}

void __lwt_fiber_stack_push_flip_server_heap() {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_edata_ifc_join_t* edata_ifc_call_join = fiber->current_ifc_join_event;
    if (edata_ifc_call_join == 0)
        throw("cannot flip to server heap context without server context", exception_arg);
    if (edata_ifc_call_join->call_type == RCD_IFC_CALL_SHARED)
        throw("cannot flip to server heap context in atomic server context - this operation requires exclusive access to the server heap", exception_arg);
    // It is safe to access remote server fiber struct here as we're already joined with the server.
    FLIP(fiber->current_heap, edata_ifc_call_join->ifc_client->ifc_server->heap);
    lwt_fiber_event_data_t edata;
    lwt_fiber_event_push(fiber, lwt_fiber_event_flip_server_heap, edata);
}

void __lwt_fiber_stack_pop_flip_server_heap(void* arg_ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_pop(fiber, lwt_fiber_event_flip_server_heap);
    lwt_edata_ifc_join_t* edata_ifc_call_join = fiber->current_ifc_join_event;
    // It is safe to access remote server fiber struct here as we're already joined with the server.
   FLIP(fiber->current_heap, edata_ifc_call_join->ifc_client->ifc_server->heap);
}

void __lwt_fiber_stack_push_try_catch(jmp_buf* jbuf, rcd_exception_type_t exceptions_to_catch, rcd_exception_t* volatile* exception_caught_out) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata;
    edata.try_catch = lwt_edata_try_catch_allocate();
    edata.try_catch->jbuf = jbuf;
    // Reading fiber->stack_alloc_stack would be invalid if this frame did dynamic stack allocation so we ensure that this function never does.
    edata.try_catch->stack_alloc_top = fiber->stack_alloc_stack;
    edata.try_catch->exceptions_to_catch = exceptions_to_catch;
    edata.try_catch->exception_caught_out = exception_caught_out;
    lwt_fiber_event_push(fiber, lwt_fiber_event_try_catch, edata);
}

static void __lwt_fiber_stack_pop_try_catch() {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata = lwt_fiber_event_pop(fiber, lwt_fiber_event_try_catch);
    lwt_edata_try_catch_free(edata.try_catch);
}

void __rcd_escape_try(__rcd_try_prop_t** prop_ptr) {
    __rcd_try_prop_t* prop = *prop_ptr;
    if (prop != 0 && prop->has_finally)
        throw("indirect exit from try block with finally clause is not supported", exception_fatal);
    __lwt_fiber_stack_pop_try_catch();
}

void __lwt_fiber_stack_push_uninterruptible() {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata;
    edata.unintr.prev_unintr = fiber->ctrl.unintr;
    fiber->ctrl.unintr = true;
    lwt_fiber_event_push(fiber, lwt_fiber_event_unintr, edata);
}

void __lwt_fiber_stack_pop_uninterruptible(void* arg_ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_fiber_event_data_t edata = lwt_fiber_event_pop(fiber, lwt_fiber_event_unintr);
    fiber->ctrl.unintr = edata.unintr.prev_unintr;
}

/// Pthreads compatible interrupt control.
void __lwt_fiber_set_uninterruptible(bool new_state, bool* old_state) {
    LWT_GET_LOCAL_FIBER(fiber);
    *old_state = fiber->ctrl.unintr;
    fiber->ctrl.unintr = new_state;
}

static void lwt_sub_fiber_destructor(void* arg_ptr) {
    rcd_sub_fiber_t* sub_fiber = arg_ptr;
    lwt_cancel_fiber_id(sub_fiber->fiber_id);
    try uninterruptible {
        __lwt_fiber_stack_push_ifc_call_join(0, sub_fiber->fiber_id, RCD_IFC_CALL_LOCK_SERVER);
        throw("lwthreads corrupt state: placeholder join was successful while waiting for sub fiber exit (invalid condition)", exception_fatal);
    } catch (exception_inner_join_fail, e) {
        // Free the exception object explicitly. We cannot use sub heaps in a destructor.
        lwt_alloc_free(e->exception_heap);
    }
}

rcd_sub_fiber_t* lwt_wrap_sub_fiber(rcd_fid_t fiber_id) {
    rcd_sub_fiber_t* sub_fiber = lwt_alloc_destructable(sizeof(rcd_sub_fiber_t), lwt_sub_fiber_destructor);
    sub_fiber->fiber_id = fiber_id;
    return sub_fiber;
}

/// This is a heuristic backtrace implementation based on the GNU backtrace()
/// implementation which relies on call frames being laid out properly in the
/// stack which is the default for x86_64 compiled programs. It is simple
/// because it does not rely on debug sections and only enumerates return
/// addresses.
/// More specifically, frame pointer omission which is enabled under O3 (for
/// example) will make the rbp register into a general purpose register and
/// calculate all variable offets based on the current rsp position instead.
/// This makes walking stack frames impossible as the pushed rbp value is
/// used for something else.
static list(void*)* lwt_get_backtrace_for_frame_ptr(void* current_frame_ptr, uint32_t skip_n) {
    list(void*)* ret_addresses = new_list(void*);
    /* DBG("lwt_get_backtrace: called, printing current stack state");
    for (size_t i = 0; i < 64; i++) {
        void* ptr = (void*) vm_align_floor((uintptr_t) stack_low_end, 8) + i * sizeof(size_t);
        DBG(DBG_PTR(ptr), ": ", DBG_PTR(*(void**) ptr));
    }
    DBG("lwt_get_backtrace: starting on frame address [", DBG_PTR(current_stack_ptr), "]"); */
    // We need to be fiber aware so we can traverse the stacklets and read the actual return values that has been replaced with __releasestack return pointers.
    LWT_GET_LOCAL_FIBER(fiber);
    /*
    // Search to the stacklet related to the current frame pointer.
    lwt_stacklet_t* current_stacklet = fiber->current_stacklet;
    while (current_stacklet != 0) {
        void* current_stacklet_ptr = current_stacklet;
        if (current_frame_ptr > current_stacklet_ptr && current_frame_ptr < current_stacklet_ptr + current_stacklet->len)
            break;
        current_stacklet = current_stacklet->next;
    }*/
    if (current_frame_ptr == 0)
        current_frame_ptr = frame_address(0);
    while (current_frame_ptr != 0) {
        // Functions in librcd programs are required to _not_ omit the frame pointer.
        // If this is function x that was called from function y:
        // stack_items[x+1] = return address [call]
        // stack_items[x+0] = frame pointer [push %rbp] - pointer to &stack_items[y]
        // Note that due to segmented stacks we can't say what direction the next frame is in.
        // We must loop until we get to the bottom frame where the frame pointer should point to 0 as _start.s initializes %rbp to it (ABI enforced).
        // DBG("lwt_get_backtrace: pushing pointer onto backtrace: [", DBG_PTR(stack_items[1]), "] @ [", DBG_PTR(&stack_items[1]), "]");
        void** stack_items = current_frame_ptr;
        void* ret_address = stack_items[1];
        if (ret_address == 0)
            break;
        if (skip_n == 0)
            list_push_end(ret_addresses, void*, ret_address);
        else
            skip_n--;
        /*
        if (ret_address == __releasestack) {
            // Iterating to the next stacklet.
            if (current_stacklet == 0 || current_stacklet->older_rsp == 0)
                break;
            void* real_ret_address = (void*) current_stacklet->older_rsp[1];
            list_push_end(ret_addresses, void*, real_ret_address);
            current_stacklet = current_stacklet->next;
        }*/
        current_frame_ptr = stack_items[0];
    }
    // DBG("lwt_get_backtrace: done (", DBG_PTR(ret_addresses), ")");
    return ret_addresses;
}

static list(void*)* lwt_get_backtrace() {
    return lwt_get_backtrace_for_frame_ptr(0, 2);
}

static void get_bt_line_archaic(fstr_t* btail, void* backtrace_addr, size_t n) {
    fstr_t prefix = "#";
    fstr_t colon_oh_x = ": 0x";
    fstr_t space = " ";
    fstr_t in_space = " in ";
    fstr_t colon = ":";
    fstr_t suffix = "\n";
    fstr_t number_buffer;
    FSTR_STACK_DECL(number_buffer, 2 * sizeof(void*));
    fstr_cpy_over(*btail, prefix, btail, 0);
    fstr_t num = fstr_serial_uint(number_buffer, n, 10);
    fstr_cpy_over(*btail, num, btail, 0);
    fstr_cpy_over(*btail, colon_oh_x, btail, 0);
    fstr_t backtrace_addr_hex = fstr_serial_uint(number_buffer, (uint64_t) backtrace_addr, 16);
    fstr_cpy_over(*btail, backtrace_addr_hex, btail, 0);
    fstr_cpy_over(*btail, space, btail, 0);
    backtrace_addr--;
    fstr_t func;
    if (!rfl_addr_to_func(backtrace_addr, &func))
        func = "?";
    fstr_cpy_over(*btail, func, btail, 0);
    fstr_cpy_over(*btail, in_space, btail, 0);
    fstr_t file;
    if (!rfl_addr_to_file(backtrace_addr, &file))
        file = "?";
    fstr_cpy_over(*btail, file, btail, 0);
    fstr_cpy_over(*btail, colon, btail, 0);
    uint32_t line;
    fstr_t line_str;
    if (rfl_addr_to_line(backtrace_addr, &line)) {
        line_str = fstr_serial_uint(number_buffer, line, 10);
    } else {
        line_str = "?";
    }
    fstr_cpy_over(*btail, line_str, btail, 0);
    fstr_cpy_over(*btail, suffix, btail, 0);
}

/// Like lwt_get_backtrace() but prints the backtrace directly to a buffer
/// instead without doing any heap allocations so it's suitable to call from
/// any context.
fstr_t lwt_get_backtrace_archaic(fstr_t buf) {
    fstr_t header = "archaic librcd backtrace:\n";
    fstr_t btail = buf;
    fstr_cpy_over(btail, header, &btail, 0);
    for (size_t ctx = 0; ctx < 2; ctx++) {
        void* current_stack_ptr;
        if (ctx == 0) {
            // Display current stack first.
            current_stack_ptr = frame_address(0);
        } else {
            // Display "userspace" fiber context if there is one.
            lwt_physical_thread_t* pt = LWT_PHYS_THREAD;
            if (pt->end_of_stack != 0 || pt->current_fiber == 0 || pt->current_fiber == &pt->system_fiber)
                continue;
            // Get frame ptr for current fiber. We can safely assume stack_pinj_jmp_buf was used.
            current_stack_ptr = (void*) pt->stack_pinj_jmp_buf.rbp;
            // Write backtrace separator:
            fstr_t bt_sep = "--\n";
            fstr_cpy_over(btail, bt_sep, &btail, 0);
            // Write the rip line.
            get_bt_line_archaic(&btail, (void*) pt->stack_pinj_jmp_buf.rip, 0);
        }
        for (size_t i = ctx; current_stack_ptr != 0; i++) {
            // Check if current stack pointer is out of range (end of call stack).
            void** stack_items = current_stack_ptr;
            void* backtrace_addr = stack_items[1];
            if (backtrace_addr == 0)
                break;
            // Write backtrace line.
            get_bt_line_archaic(&btail, backtrace_addr, i);
            // Iterate to next stack pointer.
            current_stack_ptr = stack_items[0];
        }
    }
    return fstr_detail(buf, btail);
}

static rcd_exception_t* lwt_direct_copy_exception(rcd_exception_t* exception, lwt_heap_t* exception_heap) {
    // Copy over data to exception structure.
    rcd_exception_t* new_exception;
    switch_heap(exception_heap) {
        new_exception = new(rcd_exception_t);
        new_exception->type = exception->type;
        new_exception->eio_class = 0;
        new_exception->eio_data = 0;
        new_exception->message = fss(fstr_cpy(exception->message));
        new_exception->file = fss(fstr_cpy(exception->file));
        new_exception->line = exception->line;
        new_exception->backtrace_calls = new_list(void*);
        list_foreach(exception->backtrace_calls, void*, backtrace_call)
            list_push_end(new_exception->backtrace_calls, void*, backtrace_call);
        new_exception->fwd_exception = exception->fwd_exception != 0? lwt_direct_copy_exception(exception->fwd_exception, exception_heap): 0;
        new_exception->exception_heap = exception_heap;
    }
    return new_exception;
}

rcd_exception_t* lwt_copy_exception(rcd_exception_t* exception) { sub_heap {
    rcd_exception_t* new_exception = lwt_direct_copy_exception(exception, lwt_alloc_heap());
    (void) escape(new_exception->exception_heap);
    return new_exception;
}}

void lwt_throw_new_exception(fstr_t message, fstr_t file, uint64_t line, rcd_exception_type_t exception_type, void* eio_class, void* eio_data, lwt_heap_t* custom_heap, rcd_exception_t* fwd_exception) {
    LWT_GET_LOCAL_FIBER(fiber);
    {
        // Exceptions while system spinlocks are locked is automatically fatal.
        lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
        if (phys_thread->system_rwspinlock != 0)
            exception_type = exception_fatal;
    }
    switch (exception_type) {
    case exception_arg:
    case exception_io:
    case exception_join_race:
    case exception_no_such_fiber:
    case exception_canceled:
    case exception_fatal:
        break;
    default:
        sub_heap_e(throw(fss(conc("no such exception type [", fss(fstr_from_int(exception_type, 10)), "]")), exception_arg));
        break;
    }
    // Create heap for exception. If forwarding another exception, use that heap instead of a new one.
    lwt_heap_t* exception_heap;
    rcd_exception_t* exception;
    if (custom_heap != 0) {
        assert(eio_data != 0);
        if (fwd_exception != 0) {
            exception_heap = fwd_exception->exception_heap;
            switch_heap(exception_heap)
                lwt_alloc_import(custom_heap);
        } else {
            exception_heap = custom_heap;
        }
    } else {
        if (fwd_exception != 0) {
            exception_heap = fwd_exception->exception_heap;
        } else {
            exception_heap = lwt_alloc_heap();
        }
    }
    // Copy over data to exception structure.
    switch_heap(exception_heap) {
        exception = new(rcd_exception_t);
        exception->type = exception_type;
        exception->eio_class = eio_class;
        exception->eio_data = eio_data;
        exception->message = fss(fstr_cpy(message));
        exception->file = fss(fstr_cpy(file));
        exception->line = line;
        exception->backtrace_calls = lwt_get_backtrace_for_frame_ptr(0, 2);
        exception->fwd_exception = fwd_exception;
        exception->exception_heap = exception_heap;
    }
    // Throw the new exception now.
    lwt_throw_exception(exception);
}

void lwt_throw_exception(rcd_exception_t* exception) {
    LWT_GET_LOCAL_FIBER(fiber);
    // If the exception was fatal or if we have no event stack,
    // fail immediately without ever checking if we even have an event stack.
    // This allows system context that does not have event stacks to throw fatal exceptions.
    if (exception->type == exception_fatal || fiber->event_stack == 0)
        lwt_fatal_exception_handler(exception);
    // Deactivate the heap we created so any unwinds executed below doesn't touch it.
    lwt_heap_t* exception_heap = exception->exception_heap;
    vm_heap_t* existing_heap = exception_heap->vm_heap;
    lwt_fiber_heap_deactivate(exception_heap);
    vm_heap_free(fiber->current_heap, exception_heap);
    // Find the catching event. If there are no eventual exception handler we
    // fatal error immediately without touching anything.
    jmp_buf* try_jmp_buf;
    struct lwt_stack_alloc* try_stack_alloc_top;
    lwt_fiber_event_t* catching_event;
    {
        lwt_edata_ifc_join_t* ifc_join_event = fiber->current_ifc_join_event;
        for (lwt_fiber_event_t* event = fiber->event_stack;; event = event->next) {
            if (event == 0) {
                // No handler catches this exception, it is fatal.
                lwt_fatal_exception_handler(exception);
            }
            if (event->type == lwt_fiber_event_ifc_join) {
                ifc_join_event = ifc_join_event->prev;
            } else if (event->type == lwt_fiber_event_try_catch) {
                rcd_exception_type_t exceptions_to_catch = event->data.try_catch->exceptions_to_catch;
                if ((exceptions_to_catch & exception->type) != 0) {
                    // We shall catch the exception here now but we need to test for the child join race corner case first.
                    if (exception->type == exception_join_race && (exceptions_to_catch & exception_inner_join_fail) != 0) {
                        if (lwt_has_live_join_race_from_join_event(ifc_join_event))
                            continue;
                    }
                    // Catching exception, copy and use the try edata before it's free'd.
                    // We cannot copy directly into the thread handle because it would be unnecessary and
                    // also a data hazard as the stack event pop could throw and catch exceptions themselves.
                    try_jmp_buf =  event->data.try_catch->jbuf;
                    try_stack_alloc_top =  event->data.try_catch->stack_alloc_top;
                    *event->data.try_catch->exception_caught_out = exception;
                    catching_event = event;
                    break;
                }
            }
        }
    }
    // Unwind the stack until we arrive at a location which catches this exception.
    // This also serves to set the right heap context to allocate the exception in.
    for (;;) {
        lwt_fiber_event_t* event = fiber->event_stack;
        switch (event->type) {
        case lwt_fiber_event_sub_heap: {
            __lwt_fiber_stack_pop_sub_heap(0);
            break;
        } case lwt_fiber_event_switch_heap: {
            __lwt_fiber_stack_pop_switch_heap(0);
            break;
        } case lwt_fiber_event_mitosis: {
            uint8_t _rcd_mitosis_used = 0;
            __lwt_fiber_stack_pop_mitosis_and_abort(&_rcd_mitosis_used);
            break;
        } case lwt_fiber_event_ifc_join: {
            __lwt_fiber_stack_pop_ifc_call_join();
            break;
        } case lwt_fiber_event_flip_server_heap: {
            __lwt_fiber_stack_pop_flip_server_heap(0);
            break;
        } case lwt_fiber_event_try_catch: {
            __lwt_fiber_stack_pop_try_catch();
            if (event == catching_event)
                goto forward_exception;
            break;
        } case lwt_fiber_event_unintr: {
            __lwt_fiber_stack_pop_uninterruptible(0);
            break;
        } case lwt_fiber_event_deferred:
        default: {
            throw(fss(conc("found invalid stack event on stack while throwing exception [", fss(fstr_from_int(event->type, 10)), "]")), exception_fatal);
            break;
        }}
    }
    forward_exception: {
        // Activate the object heap again in the current context.
        exception_heap = lwt_construct_alternative_heap(fiber->current_heap, existing_heap);
        for (rcd_exception_t* e = exception; e != 0; e = e->fwd_exception)
            e->exception_heap = exception_heap;
        lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
        if (phys_thread->end_of_stack == 0) {
            // We don't have a managed stack as we're not running in a fiber so we don't have a scheduler and can instantly jump to the try block.
            longjmp(*try_jmp_buf, 1);
        } else {
            // We have a managed stack and a scheduler. Jump back to it now and let it unwind all the stack memory before long jumping to the try block.
            phys_thread->try_jmp_buf = try_jmp_buf;
            phys_thread->try_stack_alloc_top = try_stack_alloc_top;
            longjmp(phys_thread->physical_jmp_buf, LWT_LONGJMP_THROW_UNWIND_STACK);
        }
        unreachable();
    }
}

void* lwt_alloc_new(size_t size) {
    LWT_GET_LOCAL_FIBER(fiber);
    // Current heap might be zero in a really primitive context, we abort here asap instead of recursing until the stack blows.
    if (fiber->current_heap == 0)
        abort();
    return vm_heap_alloc(fiber->current_heap, size, 0);
}

void* lwt_alloc_zero(size_t size) {
    void* ptr = lwt_alloc_new(size);
    memset(ptr, 0, size);
    return ptr;
}

void* lwt_alloc_buffer(size_t min_size, size_t* size_out) {
    LWT_GET_LOCAL_FIBER(fiber);
    if (fiber->current_heap == 0)
        abort();
    return vm_heap_alloc(fiber->current_heap, min_size, size_out);
}

void lwt_alloc_escape(void* ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    if (fiber->current_heap == 0)
        abort();
    if (!vm_heap_escape(ptr))
        throw("attempted to escape allocation without a parent heap", exception_fatal);
}

void lwt_alloc_import(void* ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    if (fiber->current_heap == 0)
        abort();
    vm_heap_import(0, fiber->current_heap, ptr);
}

void lwt_consume_heap(lwt_heap_t* heap) {
    LWT_GET_LOCAL_FIBER(fiber);
    if (heap == 0)
        return;
    if (fiber->current_heap == 0)
        abort();
    vm_heap_import_all(fiber->current_heap, heap->vm_heap, false);
    lwt_alloc_free(heap);
}

size_t lwt_alloc_get_size(void* ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    if (fiber->current_heap == 0)
        abort();
    return vm_heap_get_size(fiber->current_heap, ptr);
}

void lwt_alloc_free(void* ptr) {
    LWT_GET_LOCAL_FIBER(fiber);
    // We check that we are not trying to free in a remote, unsynchronized heap.
    if (!vm_heap_free(fiber->current_heap, ptr))
        throw("attempting to free memory in remote heap, the program might work by accident", exception_fatal);
}

lwt_heap_t* lwt_alloc_heap() {
    LWT_GET_LOCAL_FIBER(fiber);
    vm_heap_t* vm_heap = vm_heap_create(0);
    return lwt_construct_alternative_heap(fiber->current_heap, vm_heap);
}

void* lwt_alloc_heaped_object(size_t size, lwt_heap_t** out_fiber_heap) {
    LWT_GET_LOCAL_FIBER(fiber);
    vm_heap_t* vm_heap = vm_heap_create(0);
    void* ptr = vm_heap_alloc(vm_heap, size, 0);
    lwt_heap_t* fiber_heap = lwt_construct_alternative_heap(fiber->current_heap, vm_heap);
    *out_fiber_heap = fiber_heap;
    return ptr;
}

void* lwt_alloc_destructable(size_t size, vm_destructor_t destructor_fn) {
    LWT_GET_LOCAL_FIBER(fiber);
    return vm_heap_alloc_destructable(fiber->current_heap, size, 0, destructor_fn);
}

void* lwt_alloc_buffer_destructable(size_t size, size_t* size_out, vm_destructor_t destructor_fn) {
    LWT_GET_LOCAL_FIBER(fiber);
    return vm_heap_alloc_destructable(fiber->current_heap, size, size_out, destructor_fn);
}

lwt_heap_t* lwt_import_heap(lwt_heap_t* fiber_heap) {
    LWT_GET_LOCAL_FIBER(fiber);
    lwt_heap_t* new_fiber_heap = lwt_construct_alternative_heap(fiber->current_heap, fiber_heap->vm_heap);
    lwt_fiber_heap_deactivate(fiber_heap);
    return new_fiber_heap;
}

void __assert_fail(fstr_t expr, fstr_t file, int line, fstr_t function) {
    throw(fss(conc("Assertion failed: ", expr, " (", file, ": ", fss(fstr_from_int(line, 10)), ": ", function, ")\n")), exception_fatal);
}

fstr_t lwt_get_exception_type_str(rcd_exception_type_t exception_type) {
    switch (exception_type) {
    case exception_arg:
        return "arg";
    case exception_io:
        return "io";
    case exception_join_race:
        return "join-race";
    case exception_no_such_fiber:
        return "no-such-fiber";
    case exception_canceled:
        return "cancel";
    case exception_fatal:
        return "fatal";
    case exception_cancel_source:
        return "cancellation-source";
    default:
        return "unknown";
    }
}

static bool lwt_unix_primitive_write(int32_t fd, fstr_t buffer) {
    while (buffer.len > 0) {
        int32_t write_r = write(fd, buffer.str, buffer.len);
        if (write_r == -1) {
            if (errno != EINTR)
                return false;
        } else if (write_r == 0) {
            return false;
        } else {
            buffer.len -= write_r;
        }
    }
    return true;
}

static fstr_mem_t* lwt_get_instruction_trace_dump(list(void*)* backtrace_calls, fstr_t indent_str) { sub_heap {
    if (backtrace_calls == 0)
        return escape(fstr_alloc(0));
    list(fstr_t)* chunks = new_list(fstr_t);
    size_t i = 0;
    bool released_stack = false;
    list_foreach(backtrace_calls, void*, backtrace_call) {
        if (backtrace_call == __releasestack || backtrace_call == __releasestack_fx) {
            released_stack = true;
            continue;
        }
        list_push_end(chunks, fstr_t, indent_str);
        list_push_end(chunks, fstr_t, "#");
        list_push_end(chunks, fstr_t, fss(fstr_from_int(i, 10)));
        list_push_end(chunks, fstr_t, ": ");
        list_push_end(chunks, fstr_t, fss(rfl_addr_to_location(backtrace_call - 1)));
        if (released_stack) {
            list_push_end(chunks, fstr_t, " [^s]");
            released_stack = false;
        }
        list_push_end(chunks, fstr_t, "\n");
        i++;
    }
    if (released_stack) {
        if (list_count(chunks, fstr_t) > 0) {
            (void) list_pop_end(chunks, fstr_t);
        }
        list_push_end(chunks, fstr_t, " [.s]\n");
    }
    return escape(fstr_implode(chunks, ""));
}}

static fstr_mem_t* lwt_get_external_instruction_trace_dump(void* rbp, void* rip) { sub_heap {
    list(void*)* trace = lwt_get_backtrace_for_frame_ptr(rbp, 0);
    list_push_start(trace, void*, rip);
    return escape(lwt_get_instruction_trace_dump(trace, ""));
}}

static fstr_mem_t* lwt_get_exception_dump_inner(rcd_exception_t* exception, int32_t indent) { sub_heap {
    fstr_t indent_str = fss(fstr_alloc(indent));
    fstr_fill(indent_str, ' ');
    fstr_t at = (exception->file.len == 0 && exception->line == 0)? "[n/a]": concs("[", exception->file, ":", fss(fstr_from_int(exception->line, 10)), "]");
    fstr_t extended_type_str = "";
    fstr_t type_str = "";
    if (exception->type == exception_io && exception->eio_class != 0) {
        fstr_mem_t* (*describe_fn)(void*) = exception->eio_class;
        fstr_t description = fss(describe_fn(exception->eio_data));
        fstr_t extended_type;
        if (fstr_divide(description, "; ", &extended_type, &description)) {
            type_str = concs(indent_str, description, "\n");
        } else {
            extended_type = description;
        }
        extended_type_str = concs(" (", extended_type, ")");
    }
    fstr_mem_t* dump = conc(
        indent_str, lwt_get_exception_type_str(exception->type), extended_type_str, " type exception at ", at, "\n",
        type_str,
        indent_str, "message: ", exception->message, "\n",
        indent_str, "backtrace: ", (list_count(exception->backtrace_calls, void*) > 0? "": "not available"), "\n"
    );
    if (list_count(exception->backtrace_calls, void*) > 0)
        dump = conc(fss(dump), fss(lwt_get_instruction_trace_dump(exception->backtrace_calls, indent_str)));
    if (exception->fwd_exception != 0) {
        dump = conc(
            fss(dump),
            "forwards inner exception:\n",
            fss(lwt_get_exception_dump_inner(exception->fwd_exception, MIN(indent + 2, 8)))
        );
    }
    return escape(dump);
}}

fstr_mem_t* lwt_get_exception_dump(rcd_exception_t* exception) {
    return lwt_get_exception_dump_inner(exception, 0);
}

static void lwt_fatal_exception_handler(rcd_exception_t* exception) {
    // Switch to a new heap in case we where called from a context where
    // the current heap is already in use (e.g. from inside a destructor).
    LWT_GET_LOCAL_FIBER(fiber);
    fiber->current_heap = vm_heap_create(0);
    // Format exception and print it before aborting.
    fstr_t message = concs("FATAL EXCEPTION CRASH:\n", fss(lwt_get_exception_dump(exception)));
    write(STDERR_FILENO, message.str, message.len);
    abort();
    unreachable();
}

noret void lwt_panic() {
    // Context switch to to system context before panicing to allow
    // expensive calls that don't count towards stacklet usage.
    lwt_physical_thread_t* phys_thread = LWT_PHYS_THREAD;
    if (phys_thread->end_of_stack != 0)
        longjmp(phys_thread->physical_jmp_buf, LWT_LONGJMP_PANIC);
    // Lock to throttle more than one panic and allow global buf.
    // Might as well exit the entire thread if this lock cannot be taken.
    static int8_t lock = 0;
    while (!atomic_spinlock_trylock(&lock))
        _exit(1);
    rio_debug("!!! LIBRCD PANIC !!!\n");
    static uint8_t buf[PAGE_SIZE * 2];
    fstr_t bt = lwt_get_backtrace_archaic(FSTR_PACK(buf));
    rio_debug(bt);
    abort();
}

rcd_fid_t lwt_get_fiber_id() {
    LWT_GET_LOCAL_FIBER(fiber);
    return fiber->ctrl.id;
}

rcd_fid_t lwt_get_sub_fiber_id(rcd_sub_fiber_t* sub_fiber) {
    if (sub_fiber == 0)
        return 0;
    return sub_fiber->fiber_id;
}

fstr_mem_t* lwt_get_program_path() {
    return fstr_cpy(fss(lwt_program_path));
}

fstr_t lwt_get_program_cmdline_mem() {
    return lwt_program_cmdline_mem;
}

void lwt_set_program_cmdline(fstr_t new_cmd_line) {
    // Make sure the last byte is zero for ensured null termination.
    fstr_t end_byte = fstr_sslice(lwt_program_cmdline_mem, -2, -1);
    fstr_fill(end_byte, 0);
    // Fill the rest with the new cmd line.
    fstr_t cmdline_mem = fstr_sslice(lwt_program_cmdline_mem, 0, -2);
    fstr_t tail_cmdline_mem;
    fstr_cpy_over(cmdline_mem, new_cmd_line, &tail_cmdline_mem, 0);
    // Fill the non written tail with zeroes.
    fstr_fill(tail_cmdline_mem, 0);
}

void lwt_debug_dump_backtrace(void* rbp, void* rip) { sub_heap {
    rio_debug(fss(lwt_get_external_instruction_trace_dump(rbp, rip)));
}}

static void lwt_inner_fiber_dump_fd(int32_t write_fd, lwt_fiber_t* fiber) { sub_heap {
    rio_direct_write(write_fd, concs(
        "\n[librcd] fiber [#", i2fs(fiber->ctrl.id), "], name: <", fiber->main_name
        , (fiber->instance_name.len > 0? concs(" \"", fiber->instance_name, "\""): "")
        , ">, status: [", (fiber->ctrl.deferred? "blocked": "running"), "]\n"
    ), 0);
    if (!fiber->ctrl.deferred)
        return;
    if (fiber->defer_ifc_server != 0) {
        fstr_t accept_state;
        if (fiber->defer_ifc_server->server_state != 0) {
            accept_state = (fiber->defer_ifc_server->read_locked_state)? "read-only": "read-write";
        } else {
            accept_state = "cooldown";
        }
        fstr_t no_clients_info = (fiber->defer_ifc_server->running_clients == 0)? " (no clients accepted)": "";
        rio_direct_write(write_fd, concs(" > accepting on fiber [#", ui2fs(fiber->defer_wait_fid), "]: [", accept_state, "]", no_clients_info, "\n"), 0);
        // List client fibers it is joined with.
        lwt_ifc_client_t* client;
        LL_FOREACH(fiber->defer_ifc_server->running_clients, client) {
            rcd_fid_t client_fid = client->fiber->ctrl.id;
            rio_direct_write(write_fd, concs(" <= joined with client fiber [#", ui2fs(client_fid), "]\n"), 0);
        }
    } else if (fiber->defer_wait_fid != 0) {
        rio_direct_write(write_fd, concs(" ... waiting for fiber [#", ui2fs(fiber->defer_wait_fid), "]\n"), 0);
    } else if (fiber->defer_wait_fd != -1) {
        fstr_t fd_str = i2fs(fiber->defer_wait_fd);
        fstr_t fd_target;
        try {
            fd_target = fss(rio_file_read_link(concs("/proc/self/fd/", fd_str)));
        } catch (exception_io, e) {
            fd_target = "?";
        }
        rio_direct_write(write_fd, concs(" ... waiting for file descriptor [", i2fs(fiber->defer_wait_fd), "] [", fd_target, "]\n"), 0);
    } else {
        rio_direct_write(write_fd, " ... waiting for [*]\n", 0);
    }
    // TODO: Also list all functions queues in the fiber and their state and function pointer dump (name/address).
    // List server fibers it is joined with.
    {
        lwt_fiber_event_t* event;
        LL_FOREACH(fiber->event_stack, event) {
            if (event->type == lwt_fiber_event_ifc_join) sub_heap {
                rcd_fid_t server_fid = event->data.ifc_call_join->ifc_client->ifc_server->fiber->ctrl.id;
                rio_direct_write(write_fd, concs(" => joined with server fiber [#", ui2fs(server_fid), "]\n"), 0);
            }
        }
    }
    // Print backtrace for fiber.
    {
        lwt_fiber_event_t* event = fiber->event_stack;
        if (event != 0 && event->type == lwt_fiber_event_deferred) {
            jmp_buf* jbuf = fiber->event_stack->data.deferred.jbuf;
            void* rbp = (void*) (*jbuf)->_[1]; // setjmp rbp is stored at 0x08
            void* rip = (void*) (*jbuf)->_[7]; // setjmp rip is stored at 0x38
            rio_direct_write(write_fd, fss(lwt_get_external_instruction_trace_dump(rbp, rip)), 0);
        } else {
            rio_direct_write(write_fd, "(failed to read defer context)\n", 0);
        }
    }
}}

void lwt_write_fiber_dump_fd(int32_t write_fd) { sub_heap {
    rio_direct_write(write_fd, concs("[librcd] fiber dump of [", i2fs(hmap_fid_count(&shared_fiber_mem.fiber_map)), "] fibers commencing ****"), 0);
    LWT_SYS_SPINLOCK_WLOCK(&shared_fiber_mem.rwlock); {
        lwt_fiber_t* fiber;
        DL_FOREACH(shared_fiber_mem.fiber_list, fiber)
            lwt_inner_fiber_dump_fd(write_fd, fiber);
    } LWT_SYS_SPINLOCK_UNLOCK(&shared_fiber_mem.rwlock);
    rio_direct_write(write_fd, "[librcd] fiber dump complete ****\n", 0);
}}

void lwt_write_fiber_dump_debug() {
    lwt_write_fiber_dump_fd(STDERR_FILENO);
}
