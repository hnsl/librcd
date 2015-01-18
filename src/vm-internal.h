/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/* Contains VM declarations that are internal in librcd and should not be used
 * outside librcd. These functions are subject to change. */

#ifndef VM_INTERNAL_H
#define	VM_INTERNAL_H

#include "atomic.h"

typedef struct vm_heap vm_heap_t;

void* vm_mmap_reserve_sys(size_t min_size, size_t* size_out);
void vm_mmap_unreserve_sys(void* ptr, size_t size);

void vm_wait_for_janitor();
void vm_janitor_notify_ptid(int32_t ptid);
void vm_janitor_thread(void* arg_ptr);

void* vm_heap_alloc_destructable(vm_heap_t* heap, size_t min_size, size_t* size_out, vm_destructor_t destructor_fn);
void* vm_heap_alloc(vm_heap_t* heap, size_t min_size, size_t* size_out);
bool vm_heap_escape(void* primary_ptr);
bool vm_heap_import(vm_heap_t* require_sub_heap, vm_heap_t* dst_heap, void* primary_ptr);
bool vm_heap_has_allocs(vm_heap_t* heap);
bool vm_heap_free(vm_heap_t* require_sub_heap, void* primary_ptr);
size_t vm_heap_get_size(vm_heap_t* require_sub_heap, void* primary_ptr);

vm_heap_t* vm_heap_release(vm_heap_t* heap, size_t n_returned_allocs, void* returned_allocs[]);
vm_heap_t* vm_heap_create(vm_heap_t* parent_heap);

#endif	/* VM_INTERNAL_H */
