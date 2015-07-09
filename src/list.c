/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

noret void _list_peek_end_zero_err() {
    throw("attempted to read last element in empty list", exception_arg);
}

noret void _list_peek_start_zero_err() {
    throw("attempted to read first element in empty list", exception_arg);
}

static int dict_cmp(const rbtree_node_t* node1, const rbtree_node_t* node2) {
    rcd_abstract_delem_t* dict_elem1 = RBTREE_NODE2ELEM(rcd_abstract_delem_t, node, node1);
    rcd_abstract_delem_t* dict_elem2 = RBTREE_NODE2ELEM(rcd_abstract_delem_t, node, node2);
    return fstr_cmp(fss(&dict_elem1->key), fss(&dict_elem2->key));
}

static inline size_t dict_elem_size(rcd_abstract_dict_t* dict, fstr_t key) {
    return sizeof(rcd_abstract_delem_t) + vm_align_ceil(key.len, 8) + dict->ent_size;
}

static void dict_destructor(void* arg_ptr) {
    rcd_abstract_dict_t* dict = arg_ptr;
    // Free all dict elements.
    rcd_abstract_delem_t *delem, *tmp;
    DL_FOREACH_SAFE(dict->seq, delem, tmp) {
        size_t delem_ex_size = dict_elem_size(dict, fss(&delem->key));
        vm_mmap_unreserve(delem, delem_ex_size);
    }
}

static rcd_abstract_delem_t* new_delem(rcd_abstract_dict_t* dict, fstr_t key, size_t* delem_size) {
    *delem_size = dict_elem_size(dict, key);
    rcd_abstract_delem_t* delem = vm_mmap_reserve(*delem_size, 0);
    delem->key.len = key.len;
    memcpy(delem->key.str, key.str, key.len);
    return delem;
}

static void* delem_value_ptr(rcd_abstract_delem_t* delem) {
    return ((void*) delem) + sizeof(rcd_abstract_delem_t) + vm_align_ceil(delem->key.len, 8);
}

static rcd_abstract_delem_t* dict_lookup(rcd_abstract_dict_t* dict, fstr_t key) {
    size_t cmp_elem_size;
    rcd_abstract_delem_t* cmp_delem = new_delem(dict, key, &cmp_elem_size);
    rbtree_node_t* delem_ex_node = rbtree_lookup(&cmp_delem->node, &dict->tree);
    vm_mmap_unreserve(cmp_delem, cmp_elem_size);
    return RBTREE_NODE2ELEM(rcd_abstract_delem_t, node, delem_ex_node);
}

void* _dict_get(rcd_abstract_dict_t* dict, fstr_t key) {
    // Lookup node.
    rcd_abstract_delem_t* delem = dict_lookup(dict, key);
    if (delem != 0) {
        // Return pointer to value.
        return delem_value_ptr(delem);
    } else {
        // Node not found.
        return 0;
    }
}

void* _dict_get_vptr(rcd_abstract_delem_t* delem) {
    return delem_value_ptr(delem);
}

bool _dict_insert(rcd_abstract_dict_t* dict, fstr_t key, fstr_t value, bool append, bool replace) {
    assert(value.len == dict->ent_size);
    // Allocate node and insert it into tree.
    size_t delem_size;
    rcd_abstract_delem_t* delem = new_delem(dict, key, &delem_size);
    rbtree_node_t* delem_ex_node = rbtree_insert(&delem->node, &dict->tree);
    rcd_abstract_delem_t* delem_ex = RBTREE_NODE2ELEM(rcd_abstract_delem_t, node, delem_ex_node);
    if (delem_ex != 0) {
        // Node already exists.
        if (replace) {
            // Replace node instead.
            rbtree_replace(&delem_ex->node, &delem->node, &dict->tree);
            // Inject into existing position.
            {
                // Fix "previous" pointer.
                delem->prev = delem_ex->prev != delem_ex? delem_ex->prev: delem;
                // Fix "next" pointer.
                assert(delem_ex->next != delem_ex);
                delem->next = delem_ex->next;
                // Fix "incoming previous" pointer.
                if (delem->prev != 0 && delem->prev->next != 0) {
                    assert(delem->prev->next == delem_ex);
                    delem->prev->next = delem;
                }
                // Fix "incoming next" pointer.
                if (delem->next != 0) {
                    assert(delem->next->prev == delem_ex);
                    delem->next->prev = delem;
                }
                // Fix head pointer.
                if (dict->seq == delem_ex) {
                    dict->seq = delem;
                }
            }
            // Delete existing node.
            size_t delem_ex_size = dict_elem_size(dict, fss(&delem_ex->key));
            vm_mmap_unreserve(delem_ex, delem_ex_size);
        } else {
            // Insert failed.
            vm_mmap_unreserve(delem, delem_size);
            return false;
        }
    } else {
        // Add node to double linked list.
        if (append) {
            DL_APPEND(dict->seq, delem);
        } else {
            DL_PREPEND(dict->seq, delem);
        }
        // Length increased by one.
        dict->length++;
    }
    // Write value to the new delem.
    memcpy(delem_value_ptr(delem), value.str, dict->ent_size);
    return true;
}

bool _dict_remove(rcd_abstract_dict_t* dict, fstr_t key) {
    // Lookup node.
    rcd_abstract_delem_t* delem_ex = dict_lookup(dict, key);
    if (delem_ex != 0) {
        // Remove node.
        rbtree_remove(&delem_ex->node, &dict->tree);
        DL_DELETE(dict->seq, delem_ex);
        size_t delem_ex_size = dict_elem_size(dict, fss(&delem_ex->key));
        vm_mmap_unreserve(delem_ex, delem_ex_size);
        // Length decreased by one.
        dict->length--;
        return true;
    } else {
        // Node did not exist.
        return false;
    }
}

rcd_abstract_dict_t* _dict_new(size_t ent_size) {
    rcd_abstract_dict_t* dict = lwt_alloc_destructable(sizeof(rcd_abstract_dict_t), dict_destructor);
    *dict = (rcd_abstract_dict_t) {0};
    rbtree_init(&dict->tree, dict_cmp);
    dict->ent_size = ent_size;
    return dict;
}

static void vec_destructor(void* arg_ptr) {
    rcd_abstract_vec_t* vec = arg_ptr;
    vm_mmap_unreserve(vec->mem, vec->size);
}

rcd_abstract_vec_t* _vec_new() {
    rcd_abstract_vec_t* vec = lwt_alloc_destructable(sizeof(rcd_abstract_vec_t), vec_destructor);
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
    vec->mem = vm_mmap_realloc(vec->mem, vec->size, cpy_size, min_size, &new_size);
    if ((vec->flags & VEC_F_NOINIT) == 0) {
        memset(vec->mem + cpy_size, 0, new_size - cpy_size);
    }
    vec->size = new_size;
    vec->cap = new_size / ent_size;
}

/// Reference an offset in a vector that is possible out of bounds, expanding the vector as necessary.
void* _vec_ref(rcd_abstract_vec_t* vec, size_t ent_size, size_t offs) {
    if ((vec->flags & VEC_F_LIMIT) != 0) {
        if (vec->limit <= offs) {
            throw(concs("invalid attempt to extend vector beyond limit: offset [",
                offs, "], with limit: [", vec->limit, "]"), exception_io);
        }
    }
    if (vec->cap <= offs) {
        // Increase capacity of vector to required capacity.
        size_t min_size = (offs + 1) * ent_size;
        void* old_mem = vec->mem;
        size_t old_size = vec->size;
        vec_realloc(vec, ent_size, min_size);
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
    if (vec->length > 0) {
        _vec_ref(new_vec, ent_size, vec->length - 1);
        memcpy(new_vec->mem, vec->mem, vec->length * ent_size);
    }
    return new_vec;
}

noret void _vec_throw_get_oob(size_t offs, size_t len) { sub_heap {
    throw(concs("invalid access in vector: offset [", offs, "], with length: [", len, "]"), exception_arg);
}}

fstr_t vstr_extend(vstr_t* vs, size_t len) {
    if (len == 0)
        return "";
    size_t offs = vec_count(vs, uint8_t);
    vec_resize(vs, uint8_t, offs + len);
    fstr_t mem = vstr_str(vs);
    return fstr_slice(mem, offs, -1);
}

void vstr_write(vstr_t* vs, fstr_t str) {
    fstr_t dst = vstr_extend(vs, str.len);
    fstr_cpy_over(dst, str, 0, 0);
}
