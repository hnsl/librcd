/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "hmap.h"

#pragma librcd

/// Integer to integer (i2i) hash table definitions.
HMAP_DEFINE_TYPE(i2i, int32_t, int32_t, true, 10, 0.9, false)

// String to string (s2s) hash table definitions.
HMAP_DEFINE_STRK_TYPE(s2s, fstr_mem_t*, 10, 0.9, false)

fstr_t hmap_dump_addr2n(hmap_t* hm, const hmap_cfg_t cfg, void* slot_addr, size_t n_slots) {
    bool is_root = hmap_slot_ptr_is_root(slot_addr);
    slot_addr = hmap_slot_ptr_clear_root(slot_addr);
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    ssize_t offset = (slot_addr - ((void*) hm->mem.str));
    if (offset < 0)
        return concs(fss(fstr_from_uint((uint64_t) slot_addr, 16)), " -?");
    if ((offset % elem_len) != 0)
        return concs(fss(fstr_from_uint((uint64_t) slot_addr, 16)), " .?");
    size_t n_slot = offset / elem_len;
    if (n_slot >= n_slots)
        return concs(fss(fstr_from_uint((uint64_t) slot_addr, 16)), " >?");
    return concs("#", ui2fs(n_slot), (is_root? " [R]": ""));
}

void hmap_dump(hmap_t* hm, const hmap_cfg_t cfg) { sub_heap {
    size_t length = hm->end_slot_ptr - ((void*) hm->mem.str);
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    size_t n_slots = length / elem_len;
    DBG("** hash map [", DBG_PTR(hm->mem.str), "]-[", DBG_PTR(hm->end_slot_ptr), "] mlen:[", DBG_INT(hm->mem.len), "] elen:[", DBG_INT(elem_len), "] slots:[", DBG_INT(n_slots), "] **");
    DBG("** count:[", DBG_INT(hm->count), "] max count:[", DBG_INT(hm->max_count), "] salt:[", DBG_INT(hm->salt), "] **");
    for (size_t i = 0; i < n_slots; i++) {
        hmap_slot_t* slot = (((void*) hm->mem.str) + i * elem_len);
        if (slot->prev == 0) {
            DBG("slot [#", DBG_INT(i), "]: <empty>");
            continue;
        }
        fstr_t key_str = fss(fstr_hexencode((fstr_t) {.len = cfg.key_len, .str = slot->elem_data}));
        fstr_t value_str = fss(fstr_hexencode((fstr_t) {.len = cfg.value_len, .str = slot->elem_data + cfg.key_len}));
        DBG("slot [#", DBG_INT(i), "]: k:[", key_str, "] -> v:[", value_str, "], prev:[", hmap_dump_addr2n(hm, cfg, slot->prev, n_slots), "] next:[", hmap_dump_addr2n(hm, cfg, slot->next, n_slots), "]");
    }
}}

void rcd_self_test_hmap() {
    // Test integer to integer table.
    sub_heap {
        hmap_i2i_t hm;
        hmap_i2i_init(&hm);
        // Insert elements and check reachability.
        size_t test_n = 2000;
        atest(hmap_i2i_count(&hm) == 0);
        for (uint32_t i = 0; i < test_n; i++) {
            // hmap_dump(&hm.hm, hmap_i2i_cfg);
            {
                // Insert a mapping from i => test_n + i
                hmap_i2i_lookup_t lu = hmap_i2i_lookup(&hm, i, false);
                atest(!hmap_i2i_found(lu));
                hmap_i2i_insert(&hm, lu, i, test_n + i);
            }
            atest(hmap_i2i_count(&hm) == (i + 1));
            //hmap_dump(&hm.hm, hmap_i2i_cfg);
            for (uint32_t j = 0; j <= i; j++) {
                // Check that all previous mappings are still reachable and correct.
                hmap_i2i_lookup_t lu2 = hmap_i2i_lookup(&hm, j, true);
                atest(hmap_i2i_found(lu2));
                atest(hmap_i2i_key(lu2) == j);
                atest(hmap_i2i_value(lu2) == j + test_n);
            }
        }
        atest(hmap_i2i_count(&hm) == test_n);
        // Remove elements and check reachability.
        // hmap_dump(&hm.hm, hmap_i2i_cfg);
        for (uint32_t i = 0; i < test_n; i++) {
            for (uint32_t j = 0; j < 2; j++) {
                hmap_i2i_lookup_t lu = hmap_i2i_lookup(&hm, i + (j == 0? test_n: 0), true);
                if (j == 0) {
                    atest(!hmap_i2i_found(lu));
                } else {
                    atest(hmap_i2i_found(lu));
                    hmap_i2i_delete(&hm, lu);
                }
            }
            atest(hmap_i2i_count(&hm) == (test_n - (i + 1)));
            // hmap_dump(&hm.hm, hmap_i2i_cfg);
            for (uint32_t j = 0; j < test_n; j++) {
                // Check that all previous mappings are still reachable and correct.
                hmap_i2i_lookup_t lu2 = hmap_i2i_lookup(&hm, j, true);
                if (j > i) {
                    atest(hmap_i2i_found(lu2));
                    atest(hmap_i2i_key(lu2) == j);
                    atest(hmap_i2i_value(lu2) == j + test_n);
                } else {
                    atest(!hmap_i2i_found(lu2));
                }
            }
        }
    }
    // Test string to string table.
    sub_heap {
        hmap_s2s_t hm;
        hmap_s2s_init(&hm);
        // Insert elements and check reachability.
        size_t test_n = 1000;
        atest(hmap_s2s_count(&hm) == 0);
        for (uint32_t i = 0; i < test_n; i++) {
            // hmap_dump(&hm.hm, hmap_s2s_cfg);
            {
                // Insert a mapping from i => test_n + i
                fstr_mem_t* i_key = fstr_from_uint(i, 10);
                hmap_s2s_lookup_t lu = hmap_s2s_lookup(&hm, fss(i_key), false);
                atest(!hmap_s2s_found(lu));
                hmap_s2s_insert(&hm, lu, fss(i_key), fstr_cpy(((i % 2) == 0)? ui2fs(test_n + i): ""));
                if ((i % 2) != 0) {
                    hmap_s2s_lookup_t lu2 = hmap_s2s_lookup(&hm, fss(i_key), true);
                    atest(hmap_s2s_found(lu2));
                    if ((i % 3) == 0) {
                        // Test freeing the previous string key and allocating a new one with kv replace.
                        hmap_s2s_replace_kv(&hm, lu2, fsc(fss(i_key)), fstr_cpy(ui2fs(test_n + i)));
                        lwt_alloc_free(i_key);
                    } else {
                        // Test simple value replace
                        hmap_s2s_replace(&hm, lu2, fstr_cpy(ui2fs(test_n + i)));
                    }
                }
            }
            atest(hmap_s2s_count(&hm) == (i + 1));
            //hmap_dump(&hm.hm, hmap_s2s_cfg);
            for (uint32_t j = 0; j <= i; j++) sub_heap {
                // Check that all previous mappings are still reachable and correct.
                fstr_t j_key = ui2fs(j);
                hmap_s2s_lookup_t lu2 = hmap_s2s_lookup(&hm, j_key, true);
                atest(hmap_s2s_found(lu2));
                atest(fstr_equal(hmap_s2s_key(lu2), j_key));
                atest(fstr_equal(fss(hmap_s2s_value(lu2)), ui2fs(j + test_n)));
            }
        }
        atest(hmap_s2s_count(&hm) == test_n);
        // Remove elements and check reachability.
        // hmap_dump(&hm.hm, hmap_s2s_cfg);
        for (uint32_t i = 0; i < test_n; i++) {
            for (uint32_t j = 0; j < 2; j++) sub_heap {
                fstr_t key = ui2fs(i + (j == 0? test_n: 0));
                hmap_s2s_lookup_t lu = hmap_s2s_lookup(&hm, key, true);
                if (j == 0) {
                    atest(!hmap_s2s_found(lu));
                } else {
                    atest(hmap_s2s_found(lu));
                    hmap_s2s_delete(&hm, lu);
                }
            }
            atest(hmap_s2s_count(&hm) == (test_n - (i + 1)));
            // hmap_dump(&hm.hm, hmap_s2s_cfg);
            for (uint32_t j = 0; j < test_n; j++) sub_heap {
                // Check that all previous mappings are still reachable and correct.
                fstr_t j_key = ui2fs(j);
                hmap_s2s_lookup_t lu2 = hmap_s2s_lookup(&hm, j_key, true);
                if (j > i) {
                    atest(hmap_s2s_found(lu2));
                    atest(fstr_equal(hmap_s2s_key(lu2), j_key));
                    atest(fstr_equal(fss(hmap_s2s_value(lu2)), ui2fs(j + test_n)));
                } else {
                    atest(!hmap_s2s_found(lu2));
                }
            }
        }
    }
}
