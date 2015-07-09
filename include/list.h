/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef LIST_H
#define	LIST_H

#include "fstring.h"

typedef struct rcd_abstract_lelem {
    struct rcd_abstract_lelem* prev;
    struct rcd_abstract_lelem* next;
    uint8_t type_data[];
} rcd_abstract_lelem_t;

typedef struct rcd_abstract_list {
    size_t length;
    rcd_abstract_lelem_t* base;
} rcd_abstract_list_t;

typedef struct rcd_abstract_delem {
    // type_data[] is in immediate prefix - this allows the suffix data to be
    // key and a compare function that is unaware of the type data size
    rbtree_node_t node;
    struct rcd_abstract_delem* prev;
    struct rcd_abstract_delem* next;
    fstr_mem_t key;
} rcd_abstract_delem_t;

typedef struct rcd_abstract_dict {
    rbtree_t tree;
    rcd_abstract_delem_t* seq;
    size_t length;
    size_t ent_size;
} rcd_abstract_dict_t;

typedef struct rcd_abstract_vec {
    /// Number of elements in the vector.
    size_t length;
    /// Limit of the vector length.
    size_t limit;
    /// Capacity, maximum number of elements in the vector.
    size_t cap;
    /// Raw size of the vector in bytes.
    size_t size;
    /// Pointer to vector memory.
    void* mem;
    /// Flags.
    uint64_t flags;
} rcd_abstract_vec_t;

/// This flag disables memory zero initialization in the vector leaving the
/// value of non-written entries undefined when the vector is expanded.
/// This is useful to improve performance in low-level usage scenarios.
#define VEC_F_NOINIT (1 << 0)

/// This flag enables the vector length limit.
#define VEC_F_LIMIT (1 << 1)

#define list_count(set, type) ({ \
    list(type)* __typed_set = set; \
    ((rcd_abstract_list_t*) __typed_set)->length; \
})

#define dict_count(set, type) ({ \
    dict(type)* __typed_set = set; \
    ((rcd_abstract_dict_t*) __typed_set)->length; \
})

#define vec_count(set, type) ({ \
    vec(type)* __typed_set = set; \
    ((rcd_abstract_vec_t*) __typed_set)->length; \
})

#define list_push_end(set, type, e) ({ \
    list(type)* __typed_set = set; \
    rcd_abstract_lelem_t* __elem = lwt_alloc_new(sizeof(rcd_abstract_lelem_t) + sizeof(type)); \
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
    rcd_abstract_lelem_t* __elem = lwt_alloc_new(sizeof(rcd_abstract_lelem_t) + sizeof(type)); \
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

#define _dict_insert_raw(SET, TYPE, FSTR_KEY, VALUE, APPEND, REPLACE) ({ \
    dict(TYPE)* __typed_set = SET; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    TYPE _value = (VALUE); \
    _dict_insert(_abstract_dict, (FSTR_KEY), FSTR_PACK(_value), APPEND, REPLACE); \
})

#define dict_push_start(SET, TYPE, FSTR_KEY, VALUE) \
    _dict_insert_raw(SET, TYPE, FSTR_KEY, VALUE, false, false)

#define dict_push_end(SET, TYPE, FSTR_KEY, VALUE) \
    _dict_insert_raw(SET, TYPE, FSTR_KEY, VALUE, true, false)

#define dict_insert(SET, TYPE, FSTR_KEY, VALUE) \
    dict_push_end(SET, TYPE, FSTR_KEY, VALUE)

#define dict_inserta(SET, TYPE, FSTR_KEY, VALUE) ({ \
    bool insert_ok = dict_push_end(SET, TYPE, FSTR_KEY, VALUE); \
    assert(insert_ok);; \
})

#define dict_replace(SET, TYPE, FSTR_KEY, VALUE) \
    _dict_insert_raw(SET, TYPE, FSTR_KEY, VALUE, true, true)

#define dict_replace_n(SET, TYPE, ...) ({ \
    struct { \
        fstr_t key; \
        TYPE value; \
    } _rkv[] = {__VA_ARGS__}; \
    for (size_t _i = 0;; _i++) { \
        if (_i == LENGTHOF(_rkv)) \
            break; \
        dict_replace(SET, TYPE, _rkv[_i].key, _rkv[_i].value); \
    } \
})

#define dict_read(SET, TYPE, FSTR_KEY) ({ \
    dict(TYPE)* __typed_set = SET; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    (TYPE*) _dict_get(_abstract_dict, FSTR_KEY); \
})

#define dict_delete(SET, TYPE, FSTR_KEY) ({ \
    dict(TYPE)* __typed_set = SET; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    _dict_remove(_abstract_dict, FSTR_KEY); \
})

#define dict_deletea(SET, TYPE, FSTR_KEY) ({ \
    bool delete_ok = dict_delete(SET, TYPE, FSTR_KEY); \
    assert(delete_ok);; \
})

#define dict_first_key(SET, TYPE) ({ \
    dict(TYPE)* __typed_set = SET; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_delem_t* __elem = _abstract_dict->seq; \
    (__elem == 0)? (fstr_mem_t*) 0: (fstr_mem_t*) &__elem->key; \
})

#define dict_last_key(SET, TYPE) ({ \
    dict(TYPE)* __typed_set = SET; \
    rcd_abstract_dict_t* _abstract_dict = (rcd_abstract_dict_t*) __typed_set; \
    rcd_abstract_delem_t* __elem = (_abstract_dict->seq != 0? _abstract_dict->seq->prev: 0); \
    (__elem == 0)? (fstr_mem_t*) 0: (fstr_mem_t*) &__elem->key; \
})

/// Returns a reference to a specific OFFSET in the vector.
/// The vector is expanded as needed to make the OFFSET valid.
#define vec_ref(SET, TYPE, OFFS) ({ \
    vec(TYPE)* ___typed_set = SET; \
    rcd_abstract_vec_t* ___vec = (rcd_abstract_vec_t*) ___typed_set; \
    (TYPE*) _vec_ref(___vec, sizeof(TYPE), (OFFS)); \
})

/// SETs a value at a specific OFFSET.
/// The vector is expanded as needed to make the OFFSET valid.
#define vec_set(SET, TYPE, OFFS, X) ({ \
    vec(TYPE)* __typed_set = (SET); \
    size_t __offs = (OFFS); \
    rcd_abstract_vec_t* __vec = (rcd_abstract_vec_t*) __typed_set; \
    *vec_ref(__typed_set, TYPE, __offs) = (X); \
    ; \
})

/// Appends a value to the vector.
/// The vector is expanded as needed to make the append possible.
#define vec_append(SET, TYPE, X) ({ \
    vec(TYPE)* __typed_set = SET; \
    rcd_abstract_vec_t* __vec = (rcd_abstract_vec_t*) __typed_set; \
    *vec_ref(__typed_set, TYPE, __vec->length) = (X); \
    ; \
})

/// Appends a series of values to the vector.
#define vec_append_n(SET, TYPE, ...) ({ \
    vec(TYPE)* _typed_set = (SET); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    TYPE _vv[] = {__VA_ARGS__}; \
    size_t _vv_n = LENGTHOF(_vv); \
    if (_vv_n > 0) { \
        /* preallocate last index */ \
        size_t offs = _vec->length; \
        (void) vec_ref(_typed_set, TYPE, offs + _vv_n - 1); \
        for (size_t _i = 0; _i < _vv_n; _i++) { \
            vec_set(_typed_set, TYPE, offs + _i, _vv[_i]); \
        } \
    } \
})

/// Gets an element pointer in the vector from a specific offset.
/// Throws an arg exception if the offset is invalid.
#define vec_get_ptr(SET, TYPE, OFFS) ({ \
    vec(TYPE)* _typed_set = (SET); \
    size_t _offs = (OFFS); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    if (_offs >= _vec->length) { \
        _vec_throw_get_oob(_offs, _vec->length); \
    } \
    (TYPE*) (_vec->mem + _offs * sizeof(TYPE)); \
})

/// Gets an element in the vector from a specific offset.
/// Throws an arg exception if the offset is invalid.
#define vec_get(SET, TYPE, OFFS) \
    (*vec_get_ptr(SET, TYPE, OFFS))

/// Resizes a vector to the specified length.
#define vec_resize(SET, TYPE, NEW_LENGTH) ({ \
    vec(TYPE)* _typed_set = SET; \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    size_t _new_length = (NEW_LENGTH); \
    if (_vec->length < _new_length) { \
        _vec_ref(_vec, sizeof(TYPE), (_new_length - 1)); \
    } else { \
        _vec->length = _new_length; \
    } \
})

/// Return the content of the vector as a plain c array.
/// This array is guaranteed to have vec_count() number of slots.
/// The memory is guaranteed to be valid until the vector is modified in some way.
#define vec_array(SET, TYPE) ({ \
    vec(TYPE)* _typed_set = (SET); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    (TYPE*) _vec->mem; \
})

/// Clones a vector.
#define vec_clone(SET, TYPE) ({ \
    vec(TYPE)* _typed_set = (SET); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    (vec(TYPE)*) _vec_clone(_vec, sizeof(TYPE)); \
})

/// Returns vector limit pointer for read/write.
/// When an operation would require extending the vector length beyond this
/// limit an io exception is thrown before any side effects can have effect.
#define vec_limit(SET, TYPE) ({ \
    vec(TYPE)* _typed_set = (SET); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    &_vec->limit; \
})

/// Returns vector flags pointer for read/write.
#define vec_flags(SET, TYPE) ({ \
    vec(TYPE)* _typed_set = (SET); \
    rcd_abstract_vec_t* _vec = (rcd_abstract_vec_t*) _typed_set; \
    &_vec->flags; \
})

#define list_peek_end(set, type) ({ \
    list(type)* _typed_set = set; \
    if (list_count(_typed_set, type) == 0) \
        _list_peek_end_zero_err(); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) _typed_set; \
    *((type*) &__abstract_list->base->prev->type_data); \
})

#define list_pop_end(set, type) ({ \
    list(type)* __typed_set = set; \
    type value = list_peek_end(__typed_set, type); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    rcd_abstract_lelem_t* __elem = __abstract_list->base->prev; \
    DL_DELETE(__abstract_list->base, __elem); \
    lwt_alloc_free(__elem); \
    __abstract_list->length--; \
    value; \
})

#define list_peek_start(set, type) ({ \
    list(type)* _typed_set = set; \
    if (list_count(_typed_set, type) == 0) \
        _list_peek_start_zero_err(); \
    *((type*) &((rcd_abstract_list_t*) _typed_set)->base->type_data); \
})

#define list_pop_start(set, type) ({ \
    list(type)* __typed_set = set; \
    type value = list_peek_start(__typed_set, type); \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    rcd_abstract_lelem_t* __elem = __abstract_list->base; \
    DL_DELETE(__abstract_list->base, __elem); \
    lwt_alloc_free(__elem); \
    __abstract_list->length--; \
    value; \
})

#define list_foreach(set, type, key) \
    for (rcd_abstract_lelem_t *_cur = ((rcd_abstract_list_t *) set)->base, *_next = (_cur != 0? _cur->next: 0); _cur != 0; _cur = _next, _next = (_next != 0? _next->next: 0)) \
    for (int32_t _iter = 0; _iter == 0;) \
    for (; (_iter == 1 && (_next = 0), _iter == 0);) \
    for (type key = *((type*) &(_cur->type_data)); (_iter++) == 0; _iter = ({if (false) { list(type)* _type_check = set; } 2;}))

#define dict_foreach(SET, TYPE, EACH_KEY, EACH_VALUE) \
    for (rcd_abstract_delem_t *_next = ((rcd_abstract_dict_t *) SET)->seq, *_cur \
    ; _next != 0 && (_cur = _next, _next = _cur->next, true);) \
    for (int32_t _iter = 0; ({if (false) { dict(TYPE)* _type_check = SET; }}), _iter == 0;) \
    for (fstr_t EACH_KEY = fss(&_cur->key); _iter == 0;) \
    for (TYPE __each_value_type; (_iter == 1 && (_next = 0), _iter == 0);) \
    for (TYPE EACH_VALUE; (_iter++) == 0? (EACH_VALUE = *((TYPE*) _dict_get_vptr(_cur)), true): false;)

#define vec_foreach(SET, TYPE, ITERATOR, KEY) \
    LET(vec(TYPE)* _vec = SET) \
    for (size_t ITERATOR = 0; ITERATOR < vec_count(_vec, TYPE); ITERATOR++) \
    LET(TYPE KEY = vec_get(_vec, TYPE, ITERATOR))

#define list_foreach_delete_current(set, type) ({ \
    list(type)* __typed_set = set; \
    rcd_abstract_list_t* __abstract_list = (rcd_abstract_list_t*) __typed_set; \
    DL_DELETE(__abstract_list->base, _cur); \
    lwt_alloc_free(_cur); \
    __abstract_list->length--; \
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

#define new_dict(TYPE, ...) ({ \
    rcd_abstract_dict_t* _new_dict = _dict_new(sizeof(TYPE)); \
    dict(TYPE)* _new_typed_set = (void*) _new_dict; \
    dict_replace_n(_new_typed_set, TYPE, __VA_ARGS__); \
    _new_typed_set; \
})

#define new_vec(TYPE, ...) ({ \
    rcd_abstract_vec_t* _new_vec = _vec_new(); \
    vec(TYPE)* _new_typed_set = (void*) _new_vec; \
    vec_append_n(_new_typed_set, TYPE, __VA_ARGS__); \
    _new_typed_set; \
})

/// Allocates a new list containing the in-memory keys of the dict.
#define dict_keys(SET, TYPE) ({ \
    list(fstr_t)* keys = new_list(fstr_t); \
    dict_foreach(SET, TYPE, key, value) \
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

noret void _list_peek_end_zero_err();
noret void _list_peek_start_zero_err();

void* _dict_get(rcd_abstract_dict_t* dict, fstr_t key);
void* _dict_get_vptr(rcd_abstract_delem_t* delem);
bool _dict_insert(rcd_abstract_dict_t* dict, fstr_t key, fstr_t value, bool append, bool replace);
bool _dict_remove(rcd_abstract_dict_t* dict, fstr_t key);
rcd_abstract_dict_t* _dict_new(size_t ent_size);

rcd_abstract_vec_t* _vec_new();
void* _vec_ref(rcd_abstract_vec_t* vec, size_t ent_size, size_t offs);
rcd_abstract_vec_t* _vec_clone(rcd_abstract_vec_t* vec, size_t ent_size);
noret void _vec_throw_get_oob(size_t offs, size_t len);

/// String vector, used for dynamic buffer logic.
typedef vec(uint8_t) vstr_t;

/// Creates a new string vector. Useful for dynamic string building.
static inline vstr_t* vstr_new() {
    vstr_t* vec = new_vec(uint8_t);
    *vec_flags(vec, uint8_t) = VEC_F_NOINIT;
    return vec;
}

/// Sets the length limit of the vector string.
static inline void vstr_limit_set(vstr_t* vs, bool enable, size_t new_limit) {
    uint64_t* flags = vec_flags(vs, uint8_t);
    *flags = enable? (*flags | VEC_F_LIMIT): (*flags & ~VEC_F_LIMIT);
    *vec_limit(vs, uint8_t) = new_limit;
}

/// Returns the internal memory of the string vector as a fixed string.
/// The fixed string becomes unsafe to use as soon as the vector is modified
/// (e.g. via vstr_write()).
static fstr_t vstr_str(vstr_t* vs) {
    return (fstr_t) {
        .str = vec_array(vs, uint8_t),
        .len = vec_count(vs, uint8_t)
    };
}

/// Extends the fixed string vector and returns the internal buffer.
fstr_t vstr_extend(vstr_t* vs, size_t len);

/// Writes a fixed string to a string vector by copying it to the end of the
/// string vector, expanding it as necessary. After this call any existing
/// string vector references become invalid.
void vstr_write(vstr_t* vs, fstr_t str);

#endif	/* LIST_H */
