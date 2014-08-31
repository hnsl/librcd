/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef LIST_H
#define	LIST_H

#include "fstring.h"

typedef struct rcd_abstract_list_element {
    struct rcd_abstract_list_element* prev;
    struct rcd_abstract_list_element* next;
    uint8_t type_data[];
} rcd_abstract_list_element_t;

typedef struct rcd_abstract_list {
    size_t length;
    rcd_abstract_list_element_t* base;
} rcd_abstract_list_t;

typedef struct rcd_abstract_dict_element {
    // type_data[] is in immediate prefix - this allows the suffix data to be key and a compare function that is unaware of the type data size
    rbtree_node_t node;
    struct rcd_abstract_dict_element* prev;
    struct rcd_abstract_dict_element* next;
    fstr_mem_t key;
} rcd_abstract_dict_element_t;

typedef struct rcd_abstract_dict {
    rbtree_t tree;
    rcd_abstract_dict_element_t* seq;
    size_t length;
} rcd_abstract_dict_t;

#define list_count(set, type) ({ \
    list(type)* __typed_set = set; \
    ((rcd_abstract_list_t*) __typed_set)->length; \
})

#define dict_count(set, type) ({ \
    dict(type)* __typed_set = set; \
    ((rcd_abstract_dict_t*) __typed_set)->length; \
})

#define list_push_end(set, type, e) ({ \
    list(type)* __typed_set = set; \
    rcd_abstract_list_element_t* __elem = lwt_alloc_new(sizeof(rcd_abstract_list_element_t) + sizeof(type)); \
    *((type*) __elem->type_data) = e; \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    DL_APPEND(__abstract_list->base, __elem); \
    __abstract_list->length++;; \
})

#define list_push_end_n(set, type, ...) ({ \
    type elems[] = {__VA_ARGS__}; \
    for (int64_t __i = 0; __i < LENGTHOF(elems); __i++) \
        list_push_end(set, type, elems[__i]); \
})

#define list_push_start(set, type, e) ({ \
    list(type)* __typed_set = set; \
    rcd_abstract_list_element_t* __elem = lwt_alloc_new(sizeof(rcd_abstract_list_element_t) + sizeof(type)); \
    *((type*) __elem->type_data) = e; \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    DL_PREPEND(__abstract_list->base, __elem); \
    __abstract_list->length++; \
})

#define list_push_start_n(set, type, ...) ({ \
    type elems[] = {__VA_ARGS__}; \
    for (int64_t __i = LENGTHOF(elems) - 1; __i >= 0; __i--) \
        list_push_start(set, type, elems[__i]); \
})

#define __dict_elem_to_mem_ptr(__elem, type) ({ \
    size_t __type_size = vm_align_ceil(sizeof(type), 8); \
    ((void*) __elem) - __type_size; \
})

#define __dict_put(set, type, fstr_key, e) \
    dict(type)* __typed_set = set; \
    fstr_t __elem_key = fstr_key; \
    size_t __type_size = vm_align_ceil(sizeof(type), 8); \
    void* __mem_ptr = lwt_alloc_new(__type_size + sizeof(rcd_abstract_dict_element_t) + __elem_key.len); \
    *((type*) __mem_ptr) = e; \
    rcd_abstract_dict_element_t* __elem = __mem_ptr + __type_size; \
    __elem->key.len = __elem_key.len; \
    memcpy(__elem->key.str, __elem_key.str, __elem_key.len); \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \

#define __dict_xpend(set, type, fstr_key, e, xpend_code) \
    __dict_put(set, type, fstr_key, e); \
    rbtree_node_t* existing_node = rbtree_insert(&__elem->node, &_abstract_dict->tree); \
    if (existing_node == 0) { \
        _abstract_dict->length++; \
        xpend_code; \
    } else { \
        lwt_alloc_free(__mem_ptr); \
    } \
    existing_node == 0;

// TODO: dict pop start/end

#define dict_push_start(set, type, fstr_key, e) ({ \
    __dict_xpend(set, type, fstr_key, e, DL_PREPEND(_abstract_dict->seq, __elem)) \
})

#define dict_push_end(set, type, fstr_key, e) ({ \
    __dict_xpend(set, type, fstr_key, e, DL_APPEND(_abstract_dict->seq, __elem)) \
})

#define dict_insert(set, type, fstr_key, e) dict_push_end(set, type, fstr_key, e)

#define dict_replace(set, type, fstr_key, e) ({ \
    __dict_put(set, type, fstr_key, e); \
    rbtree_node_t* __existing_elem_node = rbtree_insert(&__elem->node, &_abstract_dict->tree); \
    if (__existing_elem_node == 0) { \
        _abstract_dict->length++; \
        DL_APPEND(_abstract_dict->seq, __elem); \
    } else { \
        rcd_abstract_dict_element_t* __existing_elem = (void*) (__existing_elem_node) - offsetof(rcd_abstract_dict_element_t, node); \
        rbtree_replace(__existing_elem_node, &__elem->node, &_abstract_dict->tree); \
        /* preserve the element at the same position in the sequence */ \
        __elem->prev = __existing_elem->prev; \
        if (__existing_elem->prev != 0 && __existing_elem->prev->next != 0) \
            __existing_elem->prev->next = __elem; \
        __elem->next = __existing_elem->next; \
        if (__existing_elem->next != 0) { \
            __existing_elem->next->prev = __elem; \
        } else { \
            if (_abstract_dict->seq->prev == __existing_elem) \
                _abstract_dict->seq->prev = __elem; \
        } \
        if (_abstract_dict->seq == __existing_elem) \
            _abstract_dict->seq = __elem; \
        /* deallocate the existing element */ \
        void* __old_mem_ptr = __dict_elem_to_mem_ptr(__existing_elem, type); \
        lwt_alloc_free(__old_mem_ptr); \
    } \
})

#define __dict_get(set, type, fstr_key) ({ \
    fstr_t __cmp_key = fstr_key; \
    char __cmp_mem[sizeof(rcd_abstract_dict_element_t) + __cmp_key.len]; \
    rcd_abstract_dict_element_t* __cmp_elem = (void*) __cmp_mem; \
    __cmp_elem->key.len = __cmp_key.len; \
    memcpy(__cmp_elem->key.str, __cmp_key.str, __cmp_key.len); \
    rbtree_node_t* __node_ptr = rbtree_lookup(&__cmp_elem->node, &_abstract_dict->tree); \
    RBTREE_NODE2ELEM(rcd_abstract_dict_element_t, node, __node_ptr); \
})

#define dict_read(set, type, fstr_key) ({ \
    dict(type)* __typed_set = set; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_dict_element_t* __elem = __dict_get(set, type, fstr_key); \
    (__elem == 0)? (type*) 0: (type*) __dict_elem_to_mem_ptr(__elem, type); \
})

#define dict_delete(set, type, fstr_key) ({ \
    dict(type)* __typed_set = set; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_dict_element_t* __elem = __dict_get(set, type, fstr_key); \
    if (__elem != 0) { \
        rbtree_remove(&__elem->node, &_abstract_dict->tree); \
        DL_DELETE(_abstract_dict->seq, __elem); \
        void* __mem_ptr = __dict_elem_to_mem_ptr(__elem, type); \
        lwt_alloc_free(__mem_ptr); \
        _abstract_dict->length--; \
    } \
    (__elem != 0); \
})

#define dict_first_key(set, type) ({ \
    dict(type)* __typed_set = set; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_dict_element_t* __elem = _abstract_dict->seq; \
    (__elem == 0)? (fstr_mem_t*) 0: (fstr_mem_t*) &__elem->key; \
})

#define dict_last_key(set, type) ({ \
    dict(type)* __typed_set = set; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_dict_element_t* __elem = (_abstract_dict->seq != 0? _abstract_dict->seq->prev: 0); \
    (__elem == 0)? (fstr_mem_t*) 0: (fstr_mem_t*) &__elem->key; \
})

#define list_peek_end(set, type) ({ \
    list(type)* _typed_set = set; \
    if (list_count(_typed_set, type) == 0) \
        throw(fstr("attempted to read last element in empty list"), exception_arg); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) _typed_set; \
    *((type*) &__abstract_list->base->prev->type_data); \
})

#define list_pop_end(set, type) ({ \
    list(type)* __typed_set = set; \
    type value = list_peek_end(__typed_set, type); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    rcd_abstract_list_element_t* __elem = __abstract_list->base->prev; \
    DL_DELETE(__abstract_list->base, __elem); \
    lwt_alloc_free(__elem); \
    __abstract_list->length--; \
    value; \
})

#define list_peek_start(set, type) ({ \
    list(type)* _typed_set = set; \
    if (list_count(_typed_set, type) == 0) \
        throw(fstr("attempted to read first element in empty list"), exception_arg); \
    *((type*) &((rcd_abstract_list_t*) _typed_set)->base->type_data); \
})

#define list_pop_start(set, type) ({ \
    list(type)* __typed_set = set; \
    type value = list_peek_start(__typed_set, type); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    rcd_abstract_list_element_t* __elem = __abstract_list->base; \
    DL_DELETE(__abstract_list->base, __elem); \
    lwt_alloc_free(__elem); \
    __abstract_list->length--; \
    value; \
})

#define list_foreach(set, type, key) \
    for (rcd_abstract_list_element_t *_cur = ((rcd_abstract_list_t *) set)->base, *_next = (_cur != 0? _cur->next: 0); _cur != 0; _cur = _next, _next = (_next != 0? _next->next: 0)) \
    for (int32_t _iter = 0; _iter == 0;) \
    for (; (_iter == 1 && (_next = 0), _iter == 0);) \
    for (type key = *((type*) &(_cur->type_data)); (_iter++) == 0; _iter = ({if (false) { list(type)* _type_check = set; } 2;}))

#define dict_foreach(set, type, each_key, each_value) \
    for (rcd_abstract_dict_element_t *_next = ((rcd_abstract_dict_t *) set)->seq, *_cur \
    ; _next != 0 && (_cur = _next, _next = _cur->next, true);) \
    for (int32_t _iter = 0; ({if (false) { dict(type)* _type_check = set; }}), _iter == 0;) \
    for (fstr_t each_key = fss(&_cur->key); _iter == 0;) \
    for (type __each_value_type; (_iter == 1 && (_next = 0), _iter == 0);) \
    for (type each_value; (_iter++) == 0? (each_value = *((type*) __dict_elem_to_mem_ptr(_cur, type)), true): false;)

#define list_foreach_delete_current(set, type) ({ \
    list(type)* __typed_set = set; \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    DL_DELETE(__abstract_list->base, _cur); \
    lwt_alloc_free(_cur); \
    __abstract_list->length--; \
})

#define dict_foreach_delete_current(set, type) ({ \
    dict(type)* __typed_set = set; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rbtree_remove(&_cur->node, &_abstract_dict->tree); \
    DL_DELETE(_abstract_dict->seq, _cur); \
    void* __mem_ptr = __dict_elem_to_mem_ptr(_cur, type); \
    lwt_alloc_free(__mem_ptr); \
    _abstract_dict->length--; \
    _cur = 0; \
})

#define dict_foreach_replace_current(type, val) ({ \
    if (false) { type* _type_check = &__each_value_type; } \
    *((type*) __dict_elem_to_mem_ptr(_cur, type)) = val; \
})

#define list_to_carray(set, type, carray_dest, carray_len_dest) ({ \
    list(type)* _typed_set = set; \
    if (list_count(_typed_set, type) == 0) { \
        carray_len_dest = 0; \
        carray_dest = 0; \
    } else { \
        carray_len_dest = list_count(_typed_set, type); \
        carray_dest = (type*) lwt_alloc_new(sizeof(type) * carray_len_dest); \
        size_t __i = 0; \
        list_foreach(_typed_set, type, key) { \
            carray_dest[__i] = key; \
            __i++; \
        } \
    } \
})

#define list_unpack(set, type, ...) ({ \
    list(type)* _typed_set = set; \
    type* elems[] = {__VA_ARGS__}; \
    bool __list_large_enough = list_count(_typed_set, type) >= LENGTHOF(elems); \
    if (__list_large_enough) { \
        size_t __i = 0; \
        list_foreach(_typed_set, type, key) { \
            *elems[__i] = key; \
            __i++; \
            if (__i >= LENGTHOF(elems)) \
            break; \
        } \
    } \
    __list_large_enough; \
})

#define new_list(type, ...) ({ \
    list(type)* _new_abstract_list = (list(type)*) new(rcd_abstract_list_t); \
    list_push_end_n(_new_abstract_list, type, __VA_ARGS__); \
    _new_abstract_list; \
})

int rcd_dict_cmp(const rbtree_node_t* node1, const rbtree_node_t* node2);

#define new_dict(type) ({ \
    rcd_abstract_dict_t* _abstract_dict = new(rcd_abstract_dict_t); \
    _abstract_dict->seq = 0; \
    _abstract_dict->length = 0; \
    rbtree_init(&_abstract_dict->tree, rcd_dict_cmp); \
    (dict(type)*) _abstract_dict; \
})

/// Allocates a new list containing the in-memory keys of the dict.
#define dict_keys(set, type) ({ \
    list(fstr_t)* keys = new_list(fstr_t); \
    dict_foreach(set, type, key, value) \
        list_push_end(keys, fstr_t, key); \
    keys; \
})

#define __QUEUE_ENQUEUE_ABSTRACT(queue, item, prev_code) ({ \
    typeof(queue) __queue = (queue); \
    typeof(__queue->first) __item = (item); \
    __item->next = 0; \
    prev_code \
    if (__queue->first == 0) { \
        __queue->first = __item; \
        __queue->last = __item; \
    } else { \
        __queue->last->next = __item; \
        __queue->last = __item; \
    }; \
})

#define QUEUE_ENQUEUE_SL(queue, item) \
    __QUEUE_ENQUEUE_ABSTRACT(queue, item,);

#define QUEUE_ENQUEUE(queue, item) \
    __QUEUE_ENQUEUE_ABSTRACT(queue, item, __item->prev = __queue->last;);

#define QUEUE_STEP_OUT(queue, item) ({ \
    typeof(queue) __queue = (queue); \
    typeof(__queue->first) __item = (item); \
    if (__item->prev != 0) \
        __item->prev->next = __item->next; \
    else \
        __queue->first = __item->next; \
    if (__item->next != 0) \
        __item->next->prev = __item->prev; \
    else \
        __queue->last = __item->prev; \
});


#define __QUEUE_DEQUEUE_ABSTRACT(queue, prev_code) ({ \
    typeof(queue) __queue = (queue); \
    typeof(__queue->first) _first = __queue->first; \
    if (_first != 0) { \
        typeof(_first) _next = _first->next; \
        __queue->first = _next; \
        if (_next == 0) { \
            __queue->last = 0; \
        } else { \
            prev_code \
        } \
    } \
    _first;\
});

#define QUEUE_DEQUEUE_SL(queue) \
    __QUEUE_DEQUEUE_ABSTRACT(queue,);

#define QUEUE_DEQUEUE(queue) \
    __QUEUE_DEQUEUE_ABSTRACT(queue, _next->prev = 0;);

#endif	/* LIST_H */
