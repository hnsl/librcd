/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

int rcd_dict_cmp(const avltree_node_t* node1, const avltree_node_t* node2) {
    rcd_abstract_dict_element_t* dict_elem1 = AVLTREE_NODE2ELEM(rcd_abstract_dict_element_t, node, node1);
    rcd_abstract_dict_element_t* dict_elem2 = AVLTREE_NODE2ELEM(rcd_abstract_dict_element_t, node, node2);
    return fstr_cmp(fss(&dict_elem1->key), fss(&dict_elem2->key));
}
