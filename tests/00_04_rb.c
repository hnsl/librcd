#include "rcd.h"

typedef struct some_tree_node {
    int id;
    long data;
    rbtree_node_t node;
} some_tree_node_t;

static int cmp_nodes(const rbtree_node_t* node_addr1, const rbtree_node_t* node_addr2) {
    some_tree_node_t* some_tree_node1 = RBTREE_NODE2ELEM(some_tree_node_t, node, node_addr1);
    some_tree_node_t* some_tree_node2 = RBTREE_NODE2ELEM(some_tree_node_t, node, node_addr2);
    return some_tree_node1->id > some_tree_node2->id? 1: (some_tree_node1->id < some_tree_node2->id? -1: 0);
}

void rcd_self_test_rb() {
    // Test offset macros.
    {
        some_tree_node_t test;
        some_tree_node_t* test_addr = RBTREE_NODE2ELEM(some_tree_node_t, node, &test.node);
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
