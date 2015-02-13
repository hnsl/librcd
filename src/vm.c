/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/*
 * Configuration:
 *
 * DEBUG: Fills small allocated memory with a0 and free'd memory with fe to
 * prevent that code that assume otherwise work by accident and to
 * make allocated and free memory easy to spot in memory dumps.
 *
 * VM_DEBUG_GUARD_ZONE:  Also enables guard pages around all page sized allocations.
 * Useful to detect memory leaks but makes the allocation scale
 * poorly since linux generally only allow 2^16 vm mappings. Also makes
 * larger allocations much slower due to multiple syscall overhead.
 *
 * VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS: Extreme memory debugging that makes
 * allocation so slow and non-scalable that it is almost useless. Places
 * all allocations on their own page and never recycles virtual memory.
 * Require VM_DEBUG_GUARD_ZONE. Negates the effects of the above config.
 *
 * VM_DEBUG_LEAK: Tracks contexts that memory is allocated in.
 */

#include "rcd.h"
#include "hmap.h"
#include "atomic.h"
#include "vm-internal.h"
#include "linux.h"
#include "lwthreads-internal.h"
#include "cassert.h"

#pragma librcd

#define VM_ALLOC_FLAG_DESTRUCTOR (0x8000000000000000)

/// The smallest unit allocation possible in the VM. If this is smaller than
/// PAGE_SIZE it is possible to do allocations which the system cannot reclaim
/// memory for but if it's not the VM will not be compatible with making small
/// allocations quickly. 6 2e = 2^6 = 64 bytes
#define VM_LINE_SIZE_2E 6

/// Beyond this size virtual memory doesn't make sense and is invalid.
#define VM_MAX_SIZE_2E 64

/// We start off with a 4 MB allocation of virtual memory in the pool.
#define VM_POOL_MMAP_INIT_SIZE_2E 18

/// Delta janitor thread priority.
#define VM_JANITOR_NICENESS_DELTA (10)

/// How many seconds the janitor waits before returning memory to the system.
#ifdef DEBUG
# define VM_JANITOR_WAIT_SEC (2)
#else
# define VM_JANITOR_WAIT_SEC (30)
#endif

#define VM_STDERR_WRITE_LINE(stderr_line) { \
    fstr_t _str = stderr_line " \n"; \
    write(STDERR_FILENO, _str.str, _str.len); \
}

/// The vm can't use fatal exceptions as the fatal exception handler and all
/// other high level rcd features depends on the vm. So we use this macro instead.
#define VM_CORE_ERROR(stderr_line) { \
    VM_STDERR_WRITE_LINE(stderr_line); \
    lwt_panic(); \
}

/// An aligned chunk of memory. The bottom VM_ALLOC_ALIGN_BITS must be zero.
typedef struct vm_mchunk {
    void* ptr;
    size_t size;
} vm_mchunk_t;

typedef struct vm_dirty_mmap_index {
    struct vm_dirty_mmap_index* prev;
    struct vm_dirty_mmap_index* next;
} vm_dirty_mmap_index_t;

typedef struct vm_dirty_mmap {
    void* start_ptr;
    uint8_t size_2e;
    /// True if indexed in new queue, otherwise indexed in old queue.
    bool is_new;
    /// Work queue (dirty_mmap_queue)
    vm_dirty_mmap_index_t queue_index;
    /// Size index (dirty_mmap_sizes)
    vm_dirty_mmap_index_t size_index;
} vm_dirty_mmap_t;

typedef struct vm_mmap_index {
    void* start_ptr;
    struct vm_mmap_index* next;
} vm_mmap_index_t;

typedef struct vm_state {
    vm_mmap_index_t* free_map_ranges;
    int8_t free_map_ranges_lock;
    uint8_t pool_mmap_end_2e;
    vm_mmap_index_t* free_vm_list[64];
    int8_t free_vm_list_lock;
    vm_dirty_mmap_index_t* new_dirty_mmap_queue;
    vm_dirty_mmap_index_t* old_dirty_mmap_queue;
    vm_dirty_mmap_index_t* dirty_mmap_sizes[64];
    int8_t dirty_mmaps_lock;
    int32_t janitor_ptid;
    int32_t janitor_wait_state;
} vm_state_t;

typedef struct vm_heap_destructor_hdr {
    vm_destructor_t destructor_fn;
} vm_heap_destructor_hdr_t;

/// Checksummed heap pointer.
typedef struct vm_csheap {
    /// We use heuristic to validate the heap pointer.
    /// This value should be (heap_ptr ^ (heap_ptr >> 16) ^ (heap_ptr >> 32) ^ 0xd68a)
    /// which makes it improbable that invalid memory looks like reserved allocations.
    uint64_t csum:16;
    /// Only 48 bits is currently used on x86_64 for the address bus.
    uint64_t heap_ptr:48;
} vm_csheap_t;

CASSERT(sizeof(vm_csheap_t) == 8);

typedef struct vm_heap_alloc_hdr {
    /// Total size of the allocation.
    /// The top bit is reserved for destructor flag.
    uint64_t total_size;
    /// Checksum protected heap the allocation is member of.
    vm_csheap_t heap;
    /// Double linked list of allocations in heap.
    struct vm_heap_alloc_hdr* prev;
    struct vm_heap_alloc_hdr* next;
} vm_heap_alloc_hdr_t;

CASSERT(sizeof(vm_heap_alloc_hdr_t) == 32);

struct vm_heap {
    struct vm_heap* parent;
    vm_heap_alloc_hdr_t* alloc_headers;
};

#if defined(VM_DEBUG_LEAK)
typedef struct vm_alloc_ctx {
    // The hash id is a function of bt_ctx and alloc_size.
    uint64_t raw_hash_id;
    fstr_t bt_ctx;
    size_t alloc_size;
    uint64_t n_allocs;
    uint64_t n_frees;
    struct vm_alloc_ctx* prev;
    struct vm_alloc_ctx* next;
} vm_alloc_ctx_t;

static inline uint64_t acx_nohash(void* key, uint64_t salt) {
    uint64_t* hash_ptr = key;
    return *hash_ptr;
}

HMAP_DEFINE_ABSTRACT_TYPE(acx, uint64_t, acx_nohash, 0, vm_alloc_ctx_t*, false, HMAP_DFL_N_PEAK_EST, HMAP_DFL_MAX_FILL_R, false);

/// Hash map of all allocation contexts.
static hmap_acx_t acx_hmap;

/// True if acx_hmap has been initialized.
static bool acx_hmap_is_init = false;

/// Linked list of all allocation contexts.
static vm_alloc_ctx_t* acx_list = 0;

//// Lock for acx data structures.
static int8_t acx_lock = 0;
#endif

vm_state_t vm_state = {
    .pool_mmap_end_2e = VM_POOL_MMAP_INIT_SIZE_2E,
};

size_t vm_total_allocated_bytes = 0;

/// Increase this size when new x86_64 architecture allows additional bits of
/// virtual memory addressing in user space.
const uint8_t vm_linux_x86_64_virtual_memory_user_space_size_2e = 47;

void* __stacklet_mmap_reserve(size_t min_size, size_t* size_out);
void __stacklet_mmap_unreserve(void* ptr, size_t size);

/// Returns log2(x) rounded down to the nearest integer.
static uint8_t vm_log2(uint64_t value) {
    // First round down to one less than a power of 2.
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    // Lookup in table.
    const uint8_t de_bruijn_table[64] = {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };
    return de_bruijn_table[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2UL)) >> 58];
}

/// FIXME: Create a framework where we can unit test these functions.
static uint8_t vm_value_to_2e(uint64_t value, bool round_up) {
    if ((value & ~1UL) == 0)
        return 0;
    return vm_log2(round_up? (value - 1) << 1: value);
}

static uint8_t vm_bytes_to_lines_2e(size_t bytes, bool round_up) {
    if (bytes <= (1UL << VM_LINE_SIZE_2E))
        return 1;
    return vm_value_to_2e(bytes, round_up) + 1 - VM_LINE_SIZE_2E;
}

static size_t vm_lines_2e_to_bytes(uint8_t lines_2e) {
    return (1UL << (lines_2e - 1 + VM_LINE_SIZE_2E));
}

static uint8_t vm_page_size_lines_2e() {
    return vm_bytes_to_lines_2e(PAGE_SIZE, false);
}

static void vm_mmap_failure(size_t len) {
    int32_t err = errno;
    const char* errno_cstr = strerror(err);
    fstr_t errno_str = (errno_cstr != 0? fstr_fix_cstr(errno_cstr): "");
    DPRINT_RAW("librcd/vm: mmap(", DBG_INT(len), ") failed: [", DBG_INT(err), "] [", errno_str, "]");
    lwt_panic();
}

/// Used to change memory protection to provoke segmentation failures when writing past segment bounds.
static void vm_debug_mprotect(void* ptr, size_t len, int32_t flags) {
    int mprotect_r = mprotect(ptr, len, flags);
    if (mprotect_r == -1)
        VM_CORE_ERROR("librcd/vm_debug_mprotect: mprotect failed");
}

static void vm_free_list_push(void* start_ptr, uint8_t lines_2e, bool already_locked_free_vm_list) {
    assert(start_ptr != 0);
    if (lines_2e > VM_MAX_SIZE_2E || lines_2e == 0)
        VM_CORE_ERROR("librcd/vm_free_list_push: got invalid chunk size");
    size_t size_bytes = vm_lines_2e_to_bytes(lines_2e);
    uint8_t page_size_lines_2e = vm_page_size_lines_2e();
    vm_mmap_index_t* mmap_index;
    if (lines_2e < page_size_lines_2e) {
        // For small allocations we can use the memory itself as the mmap index.
        mmap_index = start_ptr;
    } else {
        // Aquire a mmap index, we must use a separate list of mmap indexes for this so the range can be reclaimed by the system.
        // DBG_RAW("[vm/vm_free_list_push] pushing 32*2e^", DBG_INT(lines_2e), " range [", DBG_PTR(start_ptr), "]-[", DBG_PTR(start_ptr + vm_lines_2e_to_bytes(lines_2e)), "]\n");
        atomic_spinlock_lock(&vm_state.free_map_ranges_lock); {
            mmap_index = vm_state.free_map_ranges;
            if (mmap_index != 0) {
                vm_state.free_map_ranges = mmap_index->next;
            } else {
                // We have ran out of mmap indexes, generate new ones from the lowest page in the range.
#if defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
                vm_debug_mprotect(start_ptr, PAGE_SIZE, PROT_READ | PROT_WRITE);
#endif
                void* page0_end_ptr = start_ptr + PAGE_SIZE;
                vm_mmap_index_t* cur_mmap_index = 0;
                for (void *new_mmap_index_ptr = start_ptr, *next_new_mmap_index_ptr
                ; next_new_mmap_index_ptr = new_mmap_index_ptr + sizeof(vm_mmap_index_t), next_new_mmap_index_ptr <= page0_end_ptr
                ; new_mmap_index_ptr = next_new_mmap_index_ptr) {
                    vm_mmap_index_t* new_mmap_index = new_mmap_index_ptr;
                    new_mmap_index->next = cur_mmap_index;
                    cur_mmap_index = new_mmap_index;
                }
                vm_state.free_map_ranges = cur_mmap_index;
            }
        } atomic_spinlock_unlock(&vm_state.free_map_ranges_lock);
    }
    if (mmap_index != 0) {
        // Add the mapping onto the related free vm list.
#if defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
        vm_debug_mprotect(start_ptr, size_bytes, PROT_NONE);
#endif
        mmap_index->start_ptr = start_ptr;
        if (!already_locked_free_vm_list)
            atomic_spinlock_lock(&vm_state.free_vm_list_lock);
        {
            mmap_index->next = vm_state.free_vm_list[lines_2e];
            vm_state.free_vm_list[lines_2e] = mmap_index;
        }
        // DBG_RAW("[vm/vm_free_list_push] the 32*2e^", DBG_INT(lines_2e), " range [", DBG_PTR(start_ptr), "]-[", DBG_PTR(start_ptr + vm_lines_2e_to_bytes(lines_2e)), "] added to free list\n");
        if (!already_locked_free_vm_list)
            atomic_spinlock_unlock(&vm_state.free_vm_list_lock);
    } else {
        // Split the given chunk until we reach size_2e = vm_page_size_lines_2e() (the initial page we used for mranges).
        void* cur_split_ptr = start_ptr + size_bytes;
        for (uint8_t split_lines_2e = lines_2e - 1; split_lines_2e >= page_size_lines_2e; split_lines_2e--) {
            cur_split_ptr -= vm_lines_2e_to_bytes(split_lines_2e);
            vm_free_list_push(cur_split_ptr, split_lines_2e, already_locked_free_vm_list);
        }
    }
}

static void vm_free_mmap_index(vm_mmap_index_t* mmap_index) {
    // We cannot free mmap indexes which are not separate.
    if (mmap_index->start_ptr == (void*) mmap_index)
        return;
    // Append the free mmap index to the linked list of free indexes.
    atomic_spinlock_lock(&vm_state.free_map_ranges_lock); {
        mmap_index->next = vm_state.free_map_ranges;
        vm_state.free_map_ranges = mmap_index;
    } atomic_spinlock_unlock(&vm_state.free_map_ranges_lock);
}

static void* vm_free_list_pop(uint8_t lines_2e) {
    if (lines_2e > VM_MAX_SIZE_2E || lines_2e == 0)
        VM_CORE_ERROR("librcd/vm_free_list_pop: got invalid chunk size");
    uint8_t page_size_lines_2e = vm_page_size_lines_2e();
    void* start_ptr = 0;
    if (lines_2e >= page_size_lines_2e) {
        // First priority is always to reuse an already dirty segment saving us from the wasted work of having to clean it up and remap it to physical memory.
        atomic_spinlock_lock(&vm_state.dirty_mmaps_lock); {
            vm_dirty_mmap_index_t* dirty_mmap_size_index = vm_state.dirty_mmap_sizes[lines_2e];
            if (dirty_mmap_size_index != 0) {
                // DBG_RAW("[vm/vm_free_list_pop] popping 32*2e^", DBG_INT(lines_2e), " range from dirty vm\n");
                vm_dirty_mmap_t* dirty_mmap = ((void*) dirty_mmap_size_index) - offsetof(vm_dirty_mmap_t, size_index);
                vm_dirty_mmap_index_t** dirty_mmap_queue = (dirty_mmap->is_new? &vm_state.new_dirty_mmap_queue: &vm_state.old_dirty_mmap_queue);
                DL_DELETE(*dirty_mmap_queue, &dirty_mmap->queue_index);
                DL_DELETE(vm_state.dirty_mmap_sizes[dirty_mmap->size_2e], &dirty_mmap->size_index);
                start_ptr = dirty_mmap->start_ptr;
            }
        } atomic_spinlock_unlock(&vm_state.dirty_mmaps_lock);
    }
    if (start_ptr == 0) {
        // If we can't reuse a dirty segment we aquire a clean one instead from the pure virtual memory.
        atomic_spinlock_lock(&vm_state.free_vm_list_lock); {
            // DBG_RAW("[vm/vm_free_list_pop] popping 32*2e^", DBG_INT(lines_2e), " range from from pure vm\n");
            vm_mmap_index_t* mmap_index = vm_state.free_vm_list[lines_2e];
            if (mmap_index != 0) {
                start_ptr = mmap_index->start_ptr;
                vm_state.free_vm_list[lines_2e] = mmap_index->next;
                vm_free_mmap_index(mmap_index);
            } else {
                // Find a larger segment and use instead.
                uint8_t larger_lines_2e = lines_2e + 1;
                for (;; larger_lines_2e++) {
                    if (larger_lines_2e == VM_MAX_SIZE_2E)
                        VM_CORE_ERROR("librcd/vm_free_list_pop: ran out virtual memory on system");
                    if (larger_lines_2e >= vm_state.pool_mmap_end_2e) {
                        // DBG_RAW("[vm/vm_free_list_pop] no available 32*2e^", DBG_INT(lines_2e), " chunk, querying system for larger 2e^", DBG_INT(larger_lines_2e), " chunk\n");
                        // We reached or passed the size class of the segment we initialized the pool with last time. This means that there
                        // is either not enough free memory in the pool or the memory is not contiguous enough. Query the system for more memory.
                        size_t next_pool_size = vm_lines_2e_to_bytes(larger_lines_2e);
                        void* mmap_r = mmap(0, next_pool_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                        if (mmap_r == MAP_FAILED)
                            vm_mmap_failure(next_pool_size);
                        start_ptr = mmap_r;
                        // We double the amount of memory we require each time this exhaustion happens as calling mmap() is a waste of time we'd like to avoid.
                        vm_state.pool_mmap_end_2e = larger_lines_2e + 1;
                        break;
                    }
                    // Query the next size class.
                    vm_mmap_index_t* mmap_index = vm_state.free_vm_list[larger_lines_2e];
                    if (mmap_index != 0) {
                        // DBG_RAW("[vm/vm_free_list_pop] no available 32*2e^", DBG_INT(lines_2e), " chunk, splitting larger 2e^", DBG_INT(larger_lines_2e), " chunk\n");
                        start_ptr = mmap_index->start_ptr;
                        vm_state.free_vm_list[larger_lines_2e] = mmap_index->next;
                        vm_free_mmap_index(mmap_index);
                        break;
                    }
                }
                // We need to index the smaller chunks immediately before unlocking to prevent parallel pop's from trying to smash chunks as well or return out of memory.
                void* cur_split_ptr = start_ptr + vm_lines_2e_to_bytes(larger_lines_2e);
                for (uint8_t split_lines_2e = larger_lines_2e - 1; split_lines_2e >= lines_2e; split_lines_2e--) {
                    cur_split_ptr -= vm_lines_2e_to_bytes(split_lines_2e);
                    vm_free_list_push(cur_split_ptr, split_lines_2e, true);
                }
            }
            // DBG_RAW("[vm/vm_free_list_pop] returning range [", DBG_PTR(start_ptr), "]-[", DBG_PTR(start_ptr + vm_lines_2e_to_bytes(lines_2e)), "]\n");
        } atomic_spinlock_unlock(&vm_state.free_vm_list_lock);
    }
    return start_ptr;
}

void vm_wait_for_janitor() {
    while (vm_state.janitor_wait_state != 1) {
        atomic_spinlock_lock(&vm_state.dirty_mmaps_lock); {
            if (vm_state.janitor_wait_state == 0) {
                // DBG_RAW("[vm/vm_wait_for_janitor] forcing janitor to stop waiting\n");
                vm_state.janitor_wait_state = -1;
                int32_t tkill_r = tkill(vm_state.janitor_ptid, SIGALRM);
                if (tkill_r == -1)
                    RCD_SYSCALL_EXCEPTION(tkill, exception_fatal);
            }
        } atomic_spinlock_unlock(&vm_state.dirty_mmaps_lock);
        sched_yield();
    }
}

void vm_janitor_notify_ptid(int32_t ptid) {
    vm_state.janitor_ptid = ptid;
}

void vm_janitor_thread(void* arg_ptr) {
    // The janitor thread should have lower priority as we don't want to spend time cleaning up memory when there is work to do and memory available.
    int32_t getpriority_r = getpriority(PRIO_PROCESS, 0);
    if (getpriority_r == -1)
        RCD_SYSCALL_EXCEPTION(getpriority, exception_fatal);
    // Apparently getpriority() returns the kernel priority scale and setpriority() expects the userland priority scale.
    int32_t setpriority_r = setpriority(PRIO_PROCESS, 0, MAX(1, MIN(40, getpriority_r + VM_JANITOR_NICENESS_DELTA)) - 20);
    if (setpriority_r == -1)
        RCD_SYSCALL_EXCEPTION(setpriority, exception_fatal);
    for (;;) {
        // Reclaim all mmaps that are still old to the system.
        for (;;) {
            vm_dirty_mmap_t* dirty_mmap = 0;
            // Fetch next old and dirty mmap segment from the work queue.
            atomic_spinlock_lock(&vm_state.dirty_mmaps_lock); {
                vm_dirty_mmap_index_t* dirty_mmap_index = vm_state.old_dirty_mmap_queue;
                if (dirty_mmap_index != 0) {
                    dirty_mmap = ((void*) dirty_mmap_index) - offsetof(vm_dirty_mmap_t, queue_index);
                    DL_DELETE(vm_state.old_dirty_mmap_queue, &dirty_mmap->queue_index);
                    DL_DELETE(vm_state.dirty_mmap_sizes[dirty_mmap->size_2e], &dirty_mmap->size_index);
                }
            } atomic_spinlock_unlock(&vm_state.dirty_mmaps_lock);
            if (dirty_mmap == 0)
                break;
            void* mmap_start_ptr = dirty_mmap->start_ptr;
            assert(((uintptr_t) mmap_start_ptr % PAGE_SIZE) == 0);
            uint8_t mmap_2e_size = dirty_mmap->size_2e;
            // DBG_RAW("[vm/vm_janitor_thread] shredding 2e^", DBG_INT(mmap_2e_size), " range @ [", DBG_PTR(mmap_start_ptr), "]\n");
            int madvise_r = madvise(mmap_start_ptr, vm_lines_2e_to_bytes(mmap_2e_size), MADV_DONTNEED);
            if (madvise_r == -1)
                RCD_SYSCALL_EXCEPTION(madvise, exception_fatal);
            vm_free_list_push(mmap_start_ptr, mmap_2e_size, false);
        }
        // Make all existing new dirty mmaps old.
        atomic_spinlock_lock(&vm_state.dirty_mmaps_lock); {
            vm_dirty_mmap_index_t* dirty_mmap_index;
            DL_FOREACH(vm_state.new_dirty_mmap_queue, dirty_mmap_index) {
                vm_dirty_mmap_t* dirty_mmap = ((void*) dirty_mmap_index) - offsetof(vm_dirty_mmap_t, queue_index);
                // DBG_RAW("[vm/vm_janitor_thread] aging 2e^", DBG_INT(dirty_mmap->size_2e), " range @ [", DBG_PTR(dirty_mmap->start_ptr), "]\n");
                assert(dirty_mmap->is_new);
                dirty_mmap->is_new = false;
            }
            assert(vm_state.old_dirty_mmap_queue == 0);
            if (vm_state.new_dirty_mmap_queue == 0) {
                // No new dirty mmaps, wait for work.
                vm_state.janitor_wait_state = 1;
            } else {
                // The queue of new dirty mmaps is now old.
                vm_state.old_dirty_mmap_queue = vm_state.new_dirty_mmap_queue;
                vm_state.new_dirty_mmap_queue = 0;
            }
        } atomic_spinlock_unlock(&vm_state.dirty_mmaps_lock);
        // DBG_RAW("[vm/vm_janitor_thread] has no work, waiting\n");
        // Wait until we actually get work to do.
        while (vm_state.janitor_wait_state == 1) {
            int32_t futex_r = futex((int32_t*) &vm_state.janitor_wait_state, FUTEX_WAIT, 1, 0, 0, 0);
            if (futex_r == -1 && errno != ETIMEDOUT && errno != EWOULDBLOCK && errno != EINTR)
                RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
        }
        // This wait prevents running the cleanup too often. There is no point
        // wasting time cleaning up memory and returning it to the system if
        // we still need to allocate it a moment later.
        if (vm_state.janitor_wait_state == 0) {
            // DBG_RAW("[vm/vm_janitor_thread] has low priority work, waiting\n");
            struct timespec ts = {.tv_sec = VM_JANITOR_WAIT_SEC, .tv_nsec = 0};
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGALRM);
            int32_t sigtimedwait_r = sigtimedwait(&set, 0, &ts);
            if (sigtimedwait_r == -1) {
                if (errno != EAGAIN)
                    RCD_SYSCALL_EXCEPTION(sigtimedwait, exception_fatal);
            } else {
                // DBG_RAW("[vm/vm_janitor_thread] low priority wait was interrupted\n");
            }
        }
        // DBG_RAW("[vm/vm_janitor_thread] getting to work\n");
    }
}

void* vm_mmap_reserve_sys(size_t min_size, size_t* size_out) {
    assert(LWT_READ_STACK_LIMIT == 0);
#if defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
    size_t user_size = min_size;
    user_size = vm_align_ceil(user_size, VM_ALLOC_ALIGN);
    size_t size_hdr_size = vm_align_ceil(sizeof(size_t), VM_ALLOC_ALIGN);
    user_size += size_hdr_size;
    min_size = MAX(PAGE_SIZE, user_size) + PAGE_SIZE * 2;
#elif defined(VM_DEBUG_GUARD_ZONE)
    size_t user_size = min_size;
    if (user_size >= PAGE_SIZE)
        min_size += PAGE_SIZE * 2;
#elif defined(VM_DEBUG_LEAK)
    size_t user_size = min_size;
    user_size = vm_align_ceil(user_size, VM_ALLOC_ALIGN);
    size_t actx_size = vm_align_ceil(sizeof(vm_alloc_ctx_t*), VM_ALLOC_ALIGN);
    min_size = user_size + actx_size;
#elif defined(DEBUG)
    size_t user_size = min_size;
#endif
    uint8_t size_lines_2e = vm_bytes_to_lines_2e(min_size, true);
    void* ptr = vm_free_list_pop(size_lines_2e);
    // "Lock free" update of memory usage statistics.
    size_t final_size = vm_lines_2e_to_bytes(size_lines_2e);
    for (;;) {
        uint64_t old_total = vm_total_allocated_bytes;
        uint64_t new_total = old_total + final_size;
        if (atomic_cas_uint64(&vm_total_allocated_bytes, old_total, new_total))
            break;
    }
    // Return the final size if caller required it.
    if (size_out != 0)
        *size_out = final_size;
#if defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
    ptr += PAGE_SIZE;
    size_t pg_aligned_user_size = vm_page_align_ceil(user_size);
    vm_debug_mprotect(ptr, pg_aligned_user_size, PROT_READ | PROT_WRITE);
    // Moving pointer up as much as possible to make it more likely that use of memory beyond end seg fail.
    ptr += (pg_aligned_user_size - user_size);
    *((size_t*) ptr) = user_size ^ 0x5555555555555555;
    ptr += size_hdr_size;
    if (size_out != 0)
        *size_out = (user_size - size_hdr_size);
#elif defined(VM_DEBUG_GUARD_ZONE)
    if (user_size >= PAGE_SIZE) {
        vm_debug_mprotect(ptr, PAGE_SIZE, PROT_NONE);
        ptr += PAGE_SIZE;
        size_t aligned_user_size = vm_page_align_ceil(user_size);
        vm_debug_mprotect(ptr + aligned_user_size, PAGE_SIZE, PROT_NONE);
        if (size_out != 0)
            *size_out = aligned_user_size;
    }
#elif defined(VM_DEBUG_LEAK)
    // Resolve acx, possibly allocating a new.
    vm_alloc_ctx_t** acx_ptr = ptr;
    int32_t my_tid = lwt_get_thread_pid();
    static int32_t lock_tid = -1;
    if (lock_tid != my_tid) {
        vm_alloc_ctx_t* acx;
        atomic_spinlock_lock(&acx_lock); {
            // Detect recursion.
            lock_tid = my_tid;
            // Init acx hmap if not done so.
            if (!acx_hmap_is_init) {
                hmap_acx_init(&acx_hmap);
                acx_hmap_is_init = true;
            }
            // Get hash id of allocation.
            fstr_t bt_ctx_buf;
            FSTR_STACK_DECL(bt_ctx_buf, PAGE_SIZE);
            fstr_t bt_ctx = lwt_get_backtrace_archaic(bt_ctx_buf);
            uint64_t raw_hash_id = 0;
            raw_hash_id = hmap_murmurhash_64a(bt_ctx.str, bt_ctx.len, raw_hash_id);
            raw_hash_id = hmap_murmurhash_64a(&user_size, sizeof(user_size), raw_hash_id);
            // Lookup hash id.
            hmap_acx_lookup_t lu = hmap_acx_lookup(&acx_hmap, raw_hash_id, true);
            if (hmap_acx_found(lu)) {
                acx = hmap_acx_value(lu);
                // Just increment total allocs.
                acx->n_allocs++;
            } else {
                acx = vm_mmap_reserve(sizeof(vm_alloc_ctx_t), 0);
                acx->raw_hash_id = raw_hash_id;
                // Heap allocate the bt_ctx.
                acx->bt_ctx.len = bt_ctx.len;
                acx->bt_ctx.str = vm_mmap_reserve(bt_ctx.len, 0);
                fstr_cpy_over(acx->bt_ctx, bt_ctx, 0, 0);
                acx->alloc_size = user_size;
                acx->n_allocs = 1;
                acx->n_frees = 0;
                // Insert the acx.
                DL_APPEND(acx_list, acx);
                hmap_acx_insert(&acx_hmap, lu, raw_hash_id, acx);
            }
            // Undetect recursion.
            lock_tid = -1;
        } atomic_spinlock_unlock(&acx_lock);
        // Reference the acx.
        *acx_ptr = acx;
    } else {
        // Recursive allocation, don't track.
        *acx_ptr = 0;
    }
    // Adjust pointer and size out to hide acx ptr.
    ptr += actx_size;
    if (size_out != 0)
        *size_out = *size_out - actx_size;
#endif
#if defined(DEBUG) && !defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
    if (user_size < PAGE_SIZE) {
        // Fill small chunks with 0xa0 in debug to reduce the chance that code that expects them to be initialized to 0 works by accident.
        memset(ptr, 0xa0, MIN(0x200, user_size));
    }
#endif
    return ptr;
}

void* vm_mmap_reserve(size_t min_size, size_t* size_out) {
    if (min_size == 0)
        return 0;
#if defined(DEBUG)
    // Check that allocation isn't larger than the total x86_64 address space.
    if (min_size > 281474976710656)
        throw("vm sanity check: invalid allocation size", exception_fatal);
#endif
    // We can't use the vm in userspace as it would cause deadlocks.
    if (LWT_READ_STACK_LIMIT != 0) {
        return __stacklet_mmap_reserve(min_size, size_out);
    } else {
        return vm_mmap_reserve_sys(min_size, size_out);
    }
}

void vm_mmap_unreserve_sys(void* ptr, size_t size) {
    assert(LWT_READ_STACK_LIMIT == 0);
#if defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
    size_t user_size = size;
    user_size = vm_align_ceil(user_size, VM_ALLOC_ALIGN);
    size_t size_hdr_size = vm_align_ceil(sizeof(size_t), VM_ALLOC_ALIGN);
    user_size += size_hdr_size;
    ptr -= size_hdr_size;
    size_t expect_user_size = *((size_t*) ptr) ^ 0x5555555555555555;
    if (user_size != expect_user_size)
        VM_CORE_ERROR("vm paranoid debug mode fatal assertation: the specified size was not exactly the size the chunk was allocated with OR the memory was overwritten by writing before start of segment");
    // Clean up memory, poison the virtual memory and throw it away.
    size_t pg_aligned_user_size = vm_page_align_ceil(user_size);
    ptr -= (pg_aligned_user_size - vm_align_ceil(user_size, VM_ALLOC_ALIGN));
    int madvise_r = madvise(ptr, pg_aligned_user_size, MADV_DONTNEED);
    if (madvise_r == -1)
        RCD_SYSCALL_EXCEPTION(madvise, exception_fatal);
    vm_debug_mprotect(ptr, pg_aligned_user_size, PROT_NONE);
    return;
#elif defined(VM_DEBUG_GUARD_ZONE)
    size_t user_size = size;
    if (user_size >= PAGE_SIZE) {
        size += PAGE_SIZE * 2;
        // Calculate real mapping without guard pages and remove them.
        vm_debug_mprotect(ptr + vm_page_align_ceil(user_size), PAGE_SIZE, PROT_READ | PROT_WRITE);
        ptr -= PAGE_SIZE;
        vm_debug_mprotect(ptr, PAGE_SIZE, PROT_READ | PROT_WRITE);
    }
#elif defined(VM_DEBUG_LEAK)
    size_t user_size = size;
    user_size = vm_align_ceil(user_size, VM_ALLOC_ALIGN);
    size_t actx_size = vm_align_ceil(sizeof(vm_alloc_ctx_t*), VM_ALLOC_ALIGN);
    user_size += actx_size;
    ptr -= actx_size;
    size = user_size;
    // Update acx statistics.
    vm_alloc_ctx_t* acx = *((void**) ptr);
    while (acx != 0) {
        uint64_t oldval = acx->n_frees;
        uint64_t newval = oldval + 1;
        if (atomic_cas_uint64(&acx->n_frees, oldval, newval))
            break;
    }
#elif defined(DEBUG)
    size_t user_size = size;
#endif
#if defined(DEBUG) && !defined(VM_DEBUG_PAGE_AND_NOREUSE_ALLOCS)
    if (user_size < PAGE_SIZE) {
        // Fill small chunks with 0xfe in debug to reduce the chance that code that uses them afterwards to work by accident.
        memset(ptr, 0xfe, MIN(0x200, user_size));
    }
#endif
    uint8_t lines_2e = vm_bytes_to_lines_2e(size, true);
    if (size >= PAGE_SIZE) {
        // Memory maps larger than PAGE_SIZE is passed to the janitor so it can let the system reclaim them.
        vm_dirty_mmap_t* dirty_mmap = ptr;
        dirty_mmap->start_ptr = ptr;
        dirty_mmap->size_2e = lines_2e;
        dirty_mmap->is_new = true;
        atomic_spinlock_lock(&vm_state.dirty_mmaps_lock); {
            DL_APPEND(vm_state.new_dirty_mmap_queue, &dirty_mmap->queue_index);
            DL_PREPEND(vm_state.dirty_mmap_sizes[lines_2e], &dirty_mmap->size_index);
            if (vm_state.janitor_wait_state == 1) {
                vm_state.janitor_wait_state = 0;
                int32_t futex_r = futex((int32_t*) &vm_state.janitor_wait_state, FUTEX_WAKE, 1, 0, 0, 0);
                if (futex_r == -1)
                    RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
            }
        } atomic_spinlock_unlock(&vm_state.dirty_mmaps_lock);
    } else {
        // Memory maps smaller than PAGE_SIZE cannot be reclaimed by the system and is simply returned to the vm free list.
        vm_free_list_push(ptr, lines_2e, false);
    }
    // "Lock free" update of memory usage statistics.
    size_t final_size = vm_lines_2e_to_bytes(lines_2e);
    for (;;) {
        uint64_t old_total = vm_total_allocated_bytes;
        uint64_t new_total = old_total - final_size;
        if (atomic_cas_uint64(&vm_total_allocated_bytes, old_total, new_total))
            break;
    }
}

void vm_mmap_unreserve(void* ptr, size_t size) {
    if (size == 0)
        return;
    // We can't use the vm in userspace as it would cause deadlocks.
    if (LWT_READ_STACK_LIMIT != 0) {
        return __stacklet_mmap_unreserve(ptr, size);
    } else {
        return vm_mmap_unreserve_sys(ptr, size);
    }
}

/// This function assumes that the pool has already been aligned.
static inline vm_mchunk_t vm_aligned_alloc_pool(vm_mchunk_t* pool, size_t size) {
    size_t aligned_size = vm_align_ceil(size, VM_ALLOC_ALIGN);
    vm_mchunk_t chunk = {0};
    if (pool->size >= aligned_size) {
        chunk.ptr = pool->ptr;
        chunk.size = aligned_size;
        pool->ptr += chunk.size;
        pool->size -= chunk.size;
    }
    return chunk;
}

void* vm_static_alloc(void** free_list_top, int8_t* sync_lock, size_t element_size) {
    if (element_size == 0)
        return 0;
    size_t aligned_element_size = vm_align_ceil(element_size, VM_ALLOC_ALIGN);
    if (sync_lock != 0)
        atomic_spinlock_lock(sync_lock);
    void* r_element = *free_list_top;
    if (r_element == 0) {
        // Out of elements, aquire more elements by allocating a suitable number of pages.
        // We want to burst allocate at least 32 elements per out of elements condition.
        size_t pool_size = ((32 * aligned_element_size) / PAGE_SIZE + 1) * PAGE_SIZE;
        void* start_ptr = vm_mmap_reserve(pool_size, 0);
        void* end_ptr = start_ptr + pool_size;
        void* cur_element_ptr = 0;
        for (void *new_element_ptr = start_ptr, *next_new_element_ptr
        ; next_new_element_ptr = new_element_ptr + aligned_element_size, next_new_element_ptr <= end_ptr
        ; new_element_ptr = next_new_element_ptr) {
            *((void**) new_element_ptr) = cur_element_ptr;
            cur_element_ptr = new_element_ptr;
        }
        r_element = cur_element_ptr;
    }
    *free_list_top = *((void**) r_element);
    if (sync_lock != 0)
        atomic_spinlock_unlock(sync_lock);
    return r_element;
}

void vm_static_free(void** free_list_top, int8_t* sync_lock, void* element) {
    if (sync_lock != 0)
        atomic_spinlock_lock(sync_lock);
    *((void**) element) = *free_list_top;
    *free_list_top = element;
    if (sync_lock != 0)
        atomic_spinlock_unlock(sync_lock);
}

static uint16_t vm_csheap_sum(uint64_t heap_ptr) {
    return (heap_ptr ^ (heap_ptr >> 16) ^ (heap_ptr >> 32) ^ 0xd68a);
}

static vm_heap_t* vm_csheap_read(vm_csheap_t csh) {
    return (csh.csum == vm_csheap_sum(csh.heap_ptr))? (void*) csh.heap_ptr: 0;
}

static vm_csheap_t vm_csheap_write(vm_heap_t* heap) {
    uint64_t heap_ptr = (uint64_t) heap;
    return (vm_csheap_t) {
        .csum = vm_csheap_sum(heap_ptr),
        .heap_ptr = heap_ptr,
    };
}

static vm_csheap_t vm_csheap_clear() {
    return (vm_csheap_t) {0};
}

void* vm_heap_alloc_destructable(vm_heap_t* heap, size_t min_size, size_t* size_out, vm_destructor_t destructor_fn) {
    assert(heap != 0);
    if (min_size == 0)
        return 0;
    if (min_size >= VM_ALLOC_FLAG_DESTRUCTOR)
        VM_CORE_ERROR("librcd/vm: allocation size is too large to be sensible, memory is corrupt");
    /*if (heap->in_use_by_child)
        VM_CORE_ERROR("librcd/vm: attempt to meddle with heap that is locked by an existing child");*/
    // Calculate sizes.
    bool use_destructor = (destructor_fn != 0);
    size_t destructor_size = (use_destructor? vm_align_ceil(sizeof(vm_heap_destructor_hdr_t), VM_ALLOC_ALIGN): 0);
    size_t header_size = vm_align_ceil(sizeof(vm_heap_alloc_hdr_t), VM_ALLOC_ALIGN);
    size_t prefix_size = destructor_size + header_size;
    size_t total_size = prefix_size + min_size;
    // Do allocation and populate allocation headers.
    void* ptr = vm_mmap_reserve(total_size, size_out);
    if (size_out != 0) {
        // Adjust the final total size, removing the space reserved for the prefix.
        *size_out -= prefix_size;
    }
    vm_heap_alloc_hdr_t* alloc_header;
    if (use_destructor) {
        vm_heap_destructor_hdr_t* destructor_header = ptr;
        destructor_header->destructor_fn = destructor_fn;
        alloc_header = ptr + destructor_size;
    } else {
        alloc_header = ptr;
    }
    alloc_header->total_size = total_size | (use_destructor? VM_ALLOC_FLAG_DESTRUCTOR: 0);
    alloc_header->heap = vm_csheap_write(heap);
    DL_APPEND(heap->alloc_headers, alloc_header);
    // Return aligned primary pointer.
    return ((void*) alloc_header) + header_size;
}

void* vm_heap_alloc(vm_heap_t* heap, size_t min_size, size_t* size_out) {
    return vm_heap_alloc_destructable(heap, min_size, size_out, 0);
}

static void* vm_heap_get_alloc_header_primary_ptr(vm_heap_alloc_hdr_t* alloc_header) {
    size_t header_size = vm_align_ceil(sizeof(vm_heap_alloc_hdr_t), VM_ALLOC_ALIGN);
    return ((void*) alloc_header) + header_size;
}

static inline vm_heap_t* vm_heap_ptr_resolve(void* primary_ptr, vm_mchunk_t* out_chunk, vm_heap_alloc_hdr_t** out_alloc_header, vm_heap_destructor_hdr_t** out_destructor_header) {
    size_t header_size = vm_align_ceil(sizeof(vm_heap_alloc_hdr_t), VM_ALLOC_ALIGN);
    vm_heap_alloc_hdr_t* alloc_header = primary_ptr - header_size;
    vm_heap_destructor_hdr_t* destructor_header;
    bool use_destructor = ((alloc_header->total_size & VM_ALLOC_FLAG_DESTRUCTOR) != 0);
    void* ptr;
    size_t total_size;
    if (use_destructor) {
        size_t destructor_size = vm_align_ceil(sizeof(vm_heap_destructor_hdr_t), VM_ALLOC_ALIGN);
        destructor_header = ((void*) alloc_header) - destructor_size;
        ptr = destructor_header;
        total_size = (alloc_header->total_size & ~VM_ALLOC_FLAG_DESTRUCTOR);
    } else {
        destructor_header = 0;
        ptr = alloc_header;
        total_size = alloc_header->total_size;
    }
    if (out_chunk != 0) {
        out_chunk->ptr = ptr;
        out_chunk->size = total_size;
    }
    if (out_alloc_header != 0)
        *out_alloc_header = alloc_header;
    if (out_destructor_header != 0)
        *out_destructor_header = destructor_header;
    vm_heap_t* heap = vm_csheap_read(alloc_header->heap);
    if (heap == 0)
        VM_CORE_ERROR("librcd/vm_heap_ptr_resolve: invalid primary pointer or allocation is already free");
    return heap;
}

static inline void vm_heap_free_raw(void* primary_ptr, vm_mchunk_t chunk, vm_heap_destructor_hdr_t* destructor_header) {
    if (destructor_header != 0)
        destructor_header->destructor_fn(primary_ptr);
    vm_mmap_unreserve(chunk.ptr, chunk.size);
}

static bool vm_require_heap(vm_heap_t* heap, vm_heap_t* require_parent_heap) {
    if (heap == 0)
        return true;
    for (vm_heap_t* cur_sub_heap = heap;; cur_sub_heap = (void*) (((uintptr_t) cur_sub_heap->parent) & ~0x1)) {
        if (cur_sub_heap == 0)
            return false;
        if (cur_sub_heap == require_parent_heap)
            return true;
    }
}

bool vm_heap_escape(void* primary_ptr) {
    if (primary_ptr == 0)
        return true;
    vm_heap_alloc_hdr_t* alloc_header;
    vm_heap_t* child_heap = vm_heap_ptr_resolve(primary_ptr, 0, &alloc_header, 0);
    vm_heap_t* parent_heap = child_heap->parent;
    if (parent_heap == 0)
        return false;
    DL_DELETE(child_heap->alloc_headers, alloc_header);
    DL_APPEND(parent_heap->alloc_headers, alloc_header);
    alloc_header->heap = vm_csheap_write(parent_heap);
    return true;
}

bool vm_heap_has_allocs(vm_heap_t* heap) {
    return (heap->alloc_headers != 0);
}

bool vm_heap_import(vm_heap_t* require_sub_heap, vm_heap_t* dst_heap, void* primary_ptr) {
    if (primary_ptr == 0)
        return true;
    vm_heap_alloc_hdr_t* alloc_header;
    vm_heap_t* src_heap = vm_heap_ptr_resolve(primary_ptr, 0, &alloc_header, 0);
    if (!vm_require_heap(require_sub_heap, src_heap))
        return false;
    DL_DELETE(src_heap->alloc_headers, alloc_header);
    DL_APPEND(dst_heap->alloc_headers, alloc_header);
    alloc_header->heap = vm_csheap_write(dst_heap);
    return true;
}

bool vm_heap_import_all(vm_heap_t* dst_heap, vm_heap_t* src_heap, bool escape) {
    if (escape) {
        dst_heap = dst_heap->parent;
        if (dst_heap == 0)
           return false;
    }
    vm_heap_alloc_hdr_t *alloc_header;
    DL_FOREACH(src_heap->alloc_headers, alloc_header) {
        alloc_header->heap = vm_csheap_write(dst_heap);
    }
    DL_CONCAT(dst_heap->alloc_headers, src_heap->alloc_headers);
    src_heap->alloc_headers = 0;
    return true;
}

bool vm_heap_free(vm_heap_t* require_sub_heap, void* primary_ptr) {
    if (primary_ptr == 0)
        return true;
    vm_mchunk_t chunk;
    vm_heap_alloc_hdr_t* alloc_header;
    vm_heap_destructor_hdr_t* destructor_header;
    vm_heap_t* alloc_heap = vm_heap_ptr_resolve(primary_ptr, &chunk, &alloc_header, &destructor_header);
    if (!vm_require_heap(require_sub_heap, alloc_heap))
        return false;
    DL_DELETE(alloc_heap->alloc_headers, alloc_header);
    alloc_header->heap = vm_csheap_clear();
    vm_heap_free_raw(primary_ptr, chunk, destructor_header);
    return true;
}

size_t vm_heap_get_size(vm_heap_t* require_sub_heap, void* primary_ptr) {
    if (primary_ptr == 0)
        return true;
    vm_mchunk_t chunk;
    vm_heap_alloc_hdr_t* alloc_header;
    vm_heap_t* alloc_heap = vm_heap_ptr_resolve(primary_ptr, &chunk, &alloc_header, 0);
    if (!vm_require_heap(require_sub_heap, alloc_heap))
        return false;
    return chunk.size - (primary_ptr - chunk.ptr);
}

VM_DEFINE_FREE_LIST_ALLOCATOR_FN(vm_heap_t, vm_heap_free_list_allocate, vm_heap_free_list_free, true);

static inline bool vm_heap_get_in_use(vm_heap_t* heap) {
    return (((uintptr_t) heap->parent) & 0x1) != 0;
}

static inline void vm_heap_toggle_in_use(vm_heap_t* heap, bool enable) {
    bool is_in_use = vm_heap_get_in_use(heap);
    if (enable) {
        if (is_in_use)
            VM_CORE_ERROR("librcd/vm: attempting to use heap context which was already in use");
        heap->parent = (void*) (((uintptr_t) heap->parent) | 0x1);
    } else {
        if (!is_in_use)
            VM_CORE_ERROR("librcd/vm: attempting to stop using the heap context failed (was not already in use)");
        heap->parent = (void*) (((uintptr_t) heap->parent) & ~0x1);
    }
}

vm_heap_t* vm_heap_create(vm_heap_t* parent_heap) {
    if (parent_heap != 0)
        vm_heap_toggle_in_use(parent_heap, true);
    vm_heap_t* heap = vm_heap_free_list_allocate();
    heap->alloc_headers = 0;
    heap->parent = parent_heap;
    return heap;
}

vm_heap_t* vm_heap_release(vm_heap_t* heap, size_t n_returned_allocs, void* returned_allocs[]) {
    // If the parent_heap 1 lsb is set vm_heap_toggle_in_use() will fail anyway.
    vm_heap_t* parent_heap = heap->parent;
    // Temporarily toggle in use by child to prevent the heap from been released while it's released.
    // This could happen when throwing an exception in a destructor and the exception leaks to the parent.
    // It also makes vm_heap_release() fail when attempting to release heaps that are already in use, an important constraint.
    vm_heap_toggle_in_use(heap, true);
    if (parent_heap != 0) {
        // Import surviving allocs into parent.
        for (size_t i = 0; i < n_returned_allocs; i++) {
            void* primary_ptr = returned_allocs[i];
            if (!vm_heap_import(heap, parent_heap, primary_ptr))
                VM_CORE_ERROR("librcd/vm_heap_release: trying to return allocation in sub heap that is not member of sub heap");
        }
    }
    // Free all remaining allocs in heap.
    // If a destructor uses the heap we might get more allocations.
    // We must be prepared to redo this until all memory has finally been freed.
    for (vm_heap_alloc_hdr_t* alloc_headers; alloc_headers = heap->alloc_headers, alloc_headers != 0;) {
        heap->alloc_headers = 0;
        vm_heap_alloc_hdr_t *alloc_header, *next;
        DL_FOREACH_SAFE(alloc_headers, alloc_header, next) {
            void* primary_ptr = vm_heap_get_alloc_header_primary_ptr(alloc_header);
            vm_mchunk_t chunk;
            vm_heap_destructor_hdr_t* destructor_header;
            vm_heap_ptr_resolve(primary_ptr, &chunk, 0, &destructor_header);
            alloc_header->heap = vm_csheap_clear();
            vm_heap_free_raw(primary_ptr, chunk, destructor_header);
        }
    }
    // Allow parent heap to be used again.
    if (parent_heap != 0)
        vm_heap_toggle_in_use(parent_heap, false);
    // Return the heap to the free list.
    vm_heap_free_list_free(heap);
    // Return the inner parent heap.
    return parent_heap;
}

#if defined(VM_DEBUG_LEAK)
typedef struct acx_acc {
    /// Bytes virtually allocated by this allocation context.
    size_t virt_byte;
    /// Allocation context.
    vm_alloc_ctx_t* acx;
} acx_acc_t;

static int32_t cmp_acx_acc(const void* a, const void* b) {
    const acx_acc_t *acx_a = a, *acx_b = b;
    return acx_b->virt_byte > acx_a->virt_byte? 1: (acx_b->virt_byte < acx_a->virt_byte? -1: 0);
}

void vm_debug_print_leak_info(int32_t fd, size_t top_n) {
    // Read acx count outside lock.
    size_t acx_count = acx_hmap.hm.count;
    // Allocate vector for acx accounting.
    size_t acx_vec_len = sizeof(acx_acc_t) * acx_count;
    acx_acc_t* acx_vec = vm_mmap_virterve(acx_vec_len, 0);
    // Go through the leak contexts and move them to the acx_vec.
    vm_alloc_ctx_t* acx = acx_list;
    for (size_t i = 0; i < acx_count; i++) {
        acx_vec[i].acx = acx;
        acx = acx->next;
    }
    // Do accounting on live statistics.
    for (size_t i = 0; i < acx_count; i++) {
        vm_alloc_ctx_t* acx = acx_vec[i].acx;
        acx_vec[i].virt_byte = (acx->n_allocs - acx->n_frees) * acx->alloc_size;
    }
    // Sort the allocation contexts after virt alloc.
    sort(acx_vec, acx_count, sizeof(acx_acc_t), cmp_acx_acc, 0);
    // Print the top_n.
    fstr_t nbuf;
    FSTR_STACK_DECL(nbuf, 0x20);
    for (ssize_t i = MIN(top_n, acx_count); i >= 0; i--) {
        fstr_t buf;
        FSTR_STACK_DECL(buf, PAGE_SIZE * 2);
        fstr_t btail = buf;
        vm_alloc_ctx_t* acx = acx_vec[i].acx;
        fstr_cpy_over(btail, "** VM ALLOC CTX #", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, i, 10), &btail, 0);
        fstr_cpy_over(btail, ", RES: [", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, acx_vec[i].virt_byte, 10), &btail, 0);
        fstr_cpy_over(btail, " b] , ALLOC_SIZE: [", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, acx->alloc_size, 10), &btail, 0);
        fstr_cpy_over(btail, " b], N_ALLOCS: [", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, acx->n_allocs, 10), &btail, 0);
        fstr_cpy_over(btail, "], N_FREES: ", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, acx->n_frees, 10), &btail, 0);
        fstr_cpy_over(btail, "], ID: [", &btail, 0);
        fstr_cpy_over(btail, fstr_serial_uint(nbuf, acx->raw_hash_id, 16), &btail, 0);
        fstr_cpy_over(btail, "]\n", &btail, 0);
        fstr_cpy_over(btail, acx->bt_ctx, &btail, 0);
        fstr_cpy_over(btail, "\n====================================================\n", &btail, 0);
        fstr_t out = fstr_detail(buf, btail);
        rio_direct_write(fd, out, 0);
    }
    // Deallocate the vector.
    vm_mmap_unreserve(acx_vec, acx_vec_len);
}
#else
void vm_debug_print_leak_info(int32_t fd, size_t top_n) {
    fstr_t out = "vm was not built with VM_DEBUG_LEAK";
    write(fd, out.str, out.len);
}
#endif
