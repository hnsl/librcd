/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

typedef struct some_tree_node {
    int id;
    long data;
    avltree_node_t node;
} some_tree_node_t;

static int cmp_nodes(const avltree_node_t* node_addr1, const avltree_node_t* node_addr2) {
    some_tree_node_t* some_tree_node1 = AVLTREE_NODE2ELEM(some_tree_node_t, node, node_addr1);
    some_tree_node_t* some_tree_node2 = AVLTREE_NODE2ELEM(some_tree_node_t, node, node_addr2);
    return some_tree_node1->id > some_tree_node2->id? 1: (some_tree_node1->id < some_tree_node2->id? -1: 0);
}

void rcd_self_test_avl() {
    // Test offset macros.
    {
        some_tree_node_t test;
        some_tree_node_t* test_addr = AVLTREE_NODE2ELEM(some_tree_node_t, node, &test.node);
        atest(test_addr == &test);
    }
    // Test adding nodes.
    // TODO
    // Test deleting nodes.
    // TODO
    // Test counting nodes.
    // TODO
    // Test nodes ordering.
    // TODO
}
