/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

int rcd_dict_cmp(const rbtree_node_t* node1, const rbtree_node_t* node2) {
    rcd_abstract_dict_element_t* dict_elem1 = RBTREE_NODE2ELEM(rcd_abstract_dict_element_t, node, node1);
    rcd_abstract_dict_element_t* dict_elem2 = RBTREE_NODE2ELEM(rcd_abstract_dict_element_t, node, node2);
    return fstr_cmp(fss(&dict_elem1->key), fss(&dict_elem2->key));
}

noret void _list_peek_end_zero_err() {
    throw("attempted to read last element in empty list", exception_arg);
}

noret void _list_peek_start_zero_err() {
    throw("attempted to read first element in empty list", exception_arg);
}


static void _vec_destructor(void* arg_ptr) {
    rcd_abstract_vec_t* vec = arg_ptr;
    vm_mmap_unreserve(vec->mem, vec->size);
}

rcd_abstract_vec_t* _vec_new() {
    rcd_abstract_vec_t* vec = lwt_alloc_destructable(sizeof(rcd_abstract_vec_t), _vec_destructor);
    *vec = (rcd_abstract_vec_t) {0};
    return vec;
}

/// Reallocates a vector, both preserving old content and null-initializing new content.
/// The amortized constant time guarantee is provided by the buffer doubling behavior
/// in the vm allocator. We are not doubing any buffers here.
static void vec_realloc(rcd_abstract_vec_t* vec, size_t ent_size, size_t req_size) {
    size_t cpy_size = vec->length * ent_size;
    size_t min_size = MAX(cpy_size, req_size);
    size_t new_size;
    void* new_mem = vm_mmap_reserve(min_size, &new_size);
    memcpy(new_mem, vec->mem, cpy_size);
    memset(new_mem + cpy_size, 0, new_size - cpy_size);
    vec->mem = new_mem;
    vec->size = new_size;
    vec->cap = new_size / ent_size;
}

/// Reference an offset in a vector that is possible out of bounds, expanding the vector as necessary.
void* _vec_ref(rcd_abstract_vec_t* vec, size_t ent_size, size_t offs) {
    if (vec->cap <= offs) {
        // Increase capacity of vector to required capacity.
        size_t min_size = (offs + 1) * ent_size;
        void* old_mem = vec->mem;
        size_t old_size = vec->size;
        vec_realloc(vec, ent_size, min_size);
        vm_mmap_unreserve(old_mem, old_size);
        assert(vec->cap > offs);
    }
    if (vec->length <= offs) {
        // Bump length of the vector.
        vec->length = offs + 1;
    }
    return vec->mem + offs * ent_size;
}

/// Clones a vector.
rcd_abstract_vec_t* _vec_clone(rcd_abstract_vec_t* vec, size_t ent_size) {
    rcd_abstract_vec_t* new_vec = _vec_new();
    *new_vec = *vec;
    vec_realloc(new_vec, ent_size, 0);
    return new_vec;
}

noret void _vec_throw_get_oob(size_t offs, size_t len) { sub_heap {
    throw(concs("invalid access in vector: offset [", offs, "], with length: [", len, "]"), exception_arg);
}}
