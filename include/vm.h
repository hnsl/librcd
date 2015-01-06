/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef VM_H
#define	VM_H

#include "rcd.h"

#define VM_ALLOC_ALIGN_BITS (4)
#define VM_ALLOC_ALIGN (1 << VM_ALLOC_ALIGN_BITS)

/// Defines a custom free list-based allocation arena for values of a certain
/// type. allocate_fn_name and free_fn_name will get declared as malloc and free
/// functions; 'synchronized' should be set to true if the allocator needs to
/// be thread safe.
#define VM_DEFINE_FREE_LIST_ALLOCATOR_FN(type, allocate_fn_name, free_fn_name, synchronized) \
static type* __##allocate_fn_name##_free_list(type* free_element) { \
    static void* free_list = 0; \
    static int8_t lock = 0; \
    if (free_element == 0) { \
        return vm_static_alloc(&free_list, synchronized? &lock: 0, sizeof(type)); \
    } else { \
        vm_static_free(&free_list, synchronized? &lock: 0, free_element); \
        return 0; \
    } \
} \
static type* allocate_fn_name() { \
    return __##allocate_fn_name##_free_list(0); \
} \
static void free_fn_name(type* element) { \
    __##allocate_fn_name##_free_list(element); \
}

/// The total number of allocated bytes.
/// Can be read for the purposes of statistics and detecting memory leaks.
extern size_t vm_total_allocated_bytes;

/// Maps a memory range. Similar to the linux mmap except that this function
/// guarantees O(1) run-time complexity and is able to chunks of arbitrary
/// size although special constraints apply:
/// If allocations are not a power of 2 the size is rounded up to the nearest
/// power of 2 so at maximum 50% of the requested size takes up unused virtual
/// memory for large allocations and physical memory for small allocations.
/// Allocations larger or equal to a size of PAGE_SIZE is guaranteed to be
/// returned to the system. Smaller allocations will never be reclaimed as
/// this is not technically possible without some sort of memory compacting
/// which is not possible with native c pointers.
/// This function will always return available memory from the virtual memory
/// pool that is at minimum 2 ^ 45 bytes (limited by x86_64). If the allocation
/// size cannot be satisfied the program will crash with a fatal memory error.
/// Failed allocation can happen even if the total sum of free bytes is larger
/// than the requested size due to fragmentation. Without memory compacting
/// it is not possible to prevent fragmentation although this algorithm
/// attempts to prevent it. The best way to prevent fragmentation though
/// is to have a much larger virtual memory space than the actual required
/// memory. If size_out is not 0 the actual size of the allocated segment will
/// be returned there.
void* vm_mmap_reserve(size_t min_size, size_t* size_out);

/// Frees memory previously allocated with vm_mmap_reserve. Similar to the
/// linux unmap feature. The pointer must be a pointer returned by
/// vm_mmap_reserve and the size must be the exact size passed to
/// vm_mmap_reserve. Other arguments will provide undefined behavior.
/// See vm_mmap_reserve() for more information.
void vm_mmap_unreserve(void* ptr, size_t size);

/// Allocates chunks of a static size and uses a custom free list for more
/// efficient and compact allocation.
/// It will also allocate elements in large batches that are at least one
/// page big each, providing better memory locality.
/// NOTICE: You should not use this function directly - rather use the
/// VM_DEFINE_FREE_LIST_ALLOCATOR_FN() macro.
void* vm_static_alloc(void** free_list_top, int8_t* sync_lock, size_t element_size);

/// See vm_static_alloc(). Element should be a pointer returned by
/// vm_static_alloc().
void vm_static_free(void** free_list_top, int8_t* sync_lock, void* element);

/// Prints debug memory leak information.
/// An error message is printed if vm was not built with VM_DEBUG_LEAK.
void vm_debug_print_leak_info(int32_t fd, size_t top_n);

/// A destructor callback. The destructors must be prepared to run in the context of:
/// - Exception unwinding.
/// - Freeing memory.
/// - Leaving a sub heap.
/// Using exceptions and allocating memory is allowed, however the destructor
/// must absolutely not leak exceptions.
/// All exceptions the destructor generates MUST be caught inside it or
/// undefined behavior will occur and probably trigger a fatal exception.
typedef void (*vm_destructor_t)(void*);

__attribute__((const, always_inline))
static inline bool vm_is_page_aligned(uintptr_t ptr) {
    return (ptr % PAGE_SIZE) == 0;
}

__attribute__((const, always_inline))
static inline uint64_t vm_align_floor(uint64_t value, uint64_t align) {
    return value - (value % align);
}

__attribute__((const, always_inline))
static inline uint64_t vm_align_ceil(uint64_t value, uint64_t align) {
    return vm_align_floor(value + align - 1, align);
}

__attribute__((const, always_inline))
static inline size_t vm_page_align_floor(size_t value) {
    return vm_align_floor(value, PAGE_SIZE);
}

__attribute__((const, always_inline))
static inline size_t vm_page_align_ceil(size_t value) {
    return vm_align_ceil(value, PAGE_SIZE);
}

__attribute__((const, always_inline))
static inline bool vm_mem_intersects(void* range1_start_ptr, size_t range1_len, void* range2_start_ptr, size_t range2_len) {
    uintptr_t range1_start = (uintptr_t) range1_start_ptr, range2_start = (uintptr_t) range2_start_ptr;
    return (range1_start <= range2_start && (range1_start + range1_len - 1) >= range2_start)
    || ((range2_start <= range1_start) && (range2_start + range2_len - 1) >= range1_start);
}

#endif	/* VM_H */
