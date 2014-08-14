/*
 * libtree.h - this file is part of Libtree.
 *
 * Copyright (C) 2010 Franck Bui-Huu <fbuihuu@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#ifndef RCD_AVL_H
#define RCD_AVL_H

#define AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr) \
((element_struct_type*)(node_ptr == 0? 0: ((void*) node_ptr) - offsetof(element_struct_type, node_field_name)))

#define AVLTREE_LOOKUP_KEY(element_struct_type, node_field_name, tree_ptr, ...) ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    avltree_node_t* node_ptr = avltree_lookup(&_key.node_field_name, tree_ptr); \
    AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define AVLTREE_LOOKUP_KEY_GTE(element_struct_type, node_field_name, tree_ptr, ...) ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    avltree_node_t* node_ptr = avltree_lookup_gte(&_key.node_field_name, tree_ptr); \
    AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define AVLTREE_LOOKUP_KEY_LTE(element_struct_type, node_field_name, tree_ptr, ...)  ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    avltree_node_t* node_ptr = avltree_lookup_lte(&_key.node_field_name, tree_ptr); \
    AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define AVLTREE_CMP_V(field1, field2, equal_expr) ({ \
    typeof(field1) field1_v = field1; \
    typeof(field2) field2_v = field2; \
    field1_v > field2_v? 1: (field1_v < field2_v? -1: (equal_expr)); \
})

#define AVLTREE_CMP_FN_DECL(name, element_struct_type, node_field_name, key_name) \
static int32_t name(const avltree_node_t* node1, const avltree_node_t* node2) { \
    element_struct_type* element1 = AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node1); \
    element_struct_type* element2 = AVLTREE_NODE2ELEM(element_struct_type, node_field_name, node2); \
    return AVLTREE_CMP_V(element1->key_name, element2->key_name, 0); \
}

/*
#ifndef RCD_H
#error "Cannot include RCD header directly. ONLY INCLUDE rcd.h"
#endif*/

typedef struct avltree_node {
    struct avltree_node *left, *right;
    uintptr_t parent; /* balance factor [0:4] */
    size_t count;
} __attribute__((aligned(8))) avltree_node_t;

typedef int (*avltree_cmp_fn_t)(const struct avltree_node *, const struct avltree_node *);

typedef struct avltree {
    struct avltree_node *root;
    avltree_cmp_fn_t cmp_fn;
    int height;
    struct avltree_node *first, *last;
    bool unique_index;
} avltree_t;

size_t avltree_count(const struct avltree *tree);
struct avltree_node *avltree_first(const struct avltree *tree);
struct avltree_node *avltree_last(const struct avltree *tree);
struct avltree_node *avltree_next(const struct avltree_node *node);
struct avltree_node *avltree_prev(const struct avltree_node *node);

struct avltree_node *avltree_lookup_index(size_t index, const struct avltree *tree);
struct avltree_node *avltree_lookup_gte(const struct avltree_node *key, const struct avltree *tree);
struct avltree_node *avltree_lookup_lte(const struct avltree_node *key, const struct avltree *tree);
struct avltree_node *avltree_lookup(const struct avltree_node *key, const struct avltree *tree);
struct avltree_node *avltree_insert(struct avltree_node *node, struct avltree *tree);
void avltree_remove(struct avltree_node *node, struct avltree *tree);
void avltree_replace(struct avltree_node *old_node, struct avltree_node * new_node, struct avltree *tree);
void avltree_init(struct avltree *tree, avltree_cmp_fn_t cmp, bool unique_index);

#endif /* RCD_AVL_H */
