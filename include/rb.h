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

#ifndef RB_H
#define	RB_H

#define RBTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr) \
    ((element_struct_type*)(node_ptr == 0? 0: ((void*) node_ptr) - offsetof(element_struct_type, node_field_name)))

#define RBTREE_LOOKUP_KEY(element_struct_type, node_field_name, tree_ptr, ...) ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    rbtree_node_t* node_ptr = rbtree_lookup(&_key.node_field_name, tree_ptr); \
    RBTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define RBTREE_LOOKUP_KEY_GTE(element_struct_type, node_field_name, tree_ptr, ...) ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    rbtree_node_t* node_ptr = rbtree_lookup_gte(&_key.node_field_name, tree_ptr); \
    RBTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define RBTREE_LOOKUP_KEY_LTE(element_struct_type, node_field_name, tree_ptr, ...)  ({ \
    element_struct_type _key = {__VA_ARGS__}; \
    rbtree_node_t* node_ptr = rbtree_lookup_lte(&_key.node_field_name, tree_ptr); \
    RBTREE_NODE2ELEM(element_struct_type, node_field_name, node_ptr); \
})

#define RBTREE_CMP_V(field1, field2, equal_expr) ({ \
    typeof(field1) field1_v = field1; \
    typeof(field2) field2_v = field2; \
    field1_v > field2_v? 1: (field1_v < field2_v? -1: (equal_expr)); \
})

#define RBTREE_CMP_FN_DECL(name, element_struct_type, node_field_name, key_name) \
static int32_t name(const rbtree_node_t* node1, const rbtree_node_t* node2) { \
    element_struct_type* element1 = RBTREE_NODE2ELEM(element_struct_type, node_field_name, node1); \
    element_struct_type* element2 = RBTREE_NODE2ELEM(element_struct_type, node_field_name, node2); \
    return RBTREE_CMP_V(element1->key_name, element2->key_name, 0); \
}

enum rb_color {
    RB_BLACK,
    RB_RED,
};

typedef struct rbtree_node {
    struct rbtree_node *left, *right;
    uintptr_t parent;
} __attribute__((aligned(2))) rbtree_node_t;

typedef int (*rbtree_cmp_fn_t)(const struct rbtree_node *, const struct rbtree_node *);

typedef struct rbtree {
    struct rbtree_node *root;
    rbtree_cmp_fn_t cmp_fn;
    struct rbtree_node *first, *last;
} rbtree_t;

struct rbtree_node *rbtree_first(const struct rbtree *tree);
struct rbtree_node *rbtree_last(const struct rbtree *tree);
struct rbtree_node *rbtree_next(const struct rbtree_node *node);
struct rbtree_node *rbtree_prev(const struct rbtree_node *node);

struct rbtree_node *rbtree_lookup_gte(const struct rbtree_node *key, const struct rbtree *tree);
struct rbtree_node *rbtree_lookup_lte(const struct rbtree_node *key, const struct rbtree *tree);
struct rbtree_node *rbtree_lookup(const struct rbtree_node *key, const struct rbtree *tree);
struct rbtree_node *rbtree_insert(struct rbtree_node *node, struct rbtree *tree);
void rbtree_remove(struct rbtree_node *node, struct rbtree *tree);
void rbtree_replace(struct rbtree_node *old, struct rbtree_node *node, struct rbtree *tree);
int rbtree_init(struct rbtree *tree, rbtree_cmp_fn_t cmp);

#endif	/* RB_H */
