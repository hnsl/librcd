/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "test.h"

#pragma librcd

void rcd_self_test_vec() {
    TEST_MEM_LEAK sub_heap {
        vec(int32_t)* test_vec = new_vec(int32_t);
        atest(vec_count(test_vec, int32_t) == 0);
        vec_append(test_vec, int32_t, 2);
        atest(vec_count(test_vec, int32_t) == 1);
        vec_append(test_vec, int32_t, 22);
        vec_append(test_vec, int32_t, 222);
        atest(vec_count(test_vec, int32_t) == 3);
        atest(vec_get(test_vec, int32_t, 0) == 2);
        atest(vec_get(test_vec, int32_t, 2) == 222);
        try {
            (void) vec_get(test_vec, int32_t, 3);
            atest(false);
        } catch (exception_arg, e) {}
    }
    TEST_MEM_LEAK sub_heap {
        vec(int32_t)* test_vec = new_vec(int32_t, 7777, 66666);
        atest(vec_count(test_vec, int32_t) == 2);
        vec_append_n(test_vec, int32_t, 99, 888);
        atest(vec_count(test_vec, int32_t) == 4);
        {
            size_t i2 = 0;
            vec_foreach(test_vec, int32_t, i, v) {
                atest(i <= 3);
                atest(i == i2);
                switch (i) {
                case 0:
                    atest(v == 7777);
                    break;
                case 1:
                    atest(v == 66666);
                    break;
                case 2:
                    atest(v == 99);
                    break;
                case 3:
                    atest(v == 888);
                    break;
                default:
                    atest(false);
                }
                i2++;
            }
            atest(i2 == 4);
        }
    }
    TEST_MEM_LEAK sub_heap {
        vec(int32_t*)* test_vec2;
        sub_heap {
            vec(int32_t*)* test_vec = new_vec(int32_t*);
            for (size_t i = 0; i < 100; i++) {
                int32_t* j = new(int32_t);
                *j = i;
                vec_append(test_vec, int32_t*, j);
                escape_list(j);
            }
            size_t i2 = 0;
            vec_foreach(test_vec, int32_t*, i, j) {
                atest(i <= 100);
                atest(i == i2);
                atest(*j == i);
                i2++;
            }
            atest(i2 == 100);
            test_vec2 = escape(vec_clone(test_vec, int32_t*));
        }
        size_t i2 = 0;
        vec_foreach(test_vec2, int32_t*, i, j) {
            atest(i <= 100);
            atest(i == i2);
            atest(*j == i);
            i2++;
        }
        atest(i2 == 100);
    }
    // Test some more raw vector stuff.
    TEST_MEM_LEAK sub_heap {
        vec(int32_t)* test_vec = new_vec(int32_t);
        atest(vec_count(test_vec, int32_t) == 0);
        (void) vec_ref(test_vec, int32_t, 999);
        atest(vec_count(test_vec, int32_t) == 1000);
        {
            size_t i2 = 0;
            vec_foreach(test_vec, int32_t, i, j) {
                atest(j == 0);
                i2++;
            }
            atest(i2 == 1000);
        }
        vec_set(test_vec, int32_t, 4999, 4999);
        atest(vec_count(test_vec, int32_t) == 5000);
        {
            size_t i2 = 0;
            vec_foreach(test_vec, int32_t, i, j) {
                atest(j == (i < 4999? 0: 4999));
                i2++;
            }
            atest(i2 == 5000);
        }
        for (size_t i = 0; i < 50000; i++) {
            vec_set(test_vec, int32_t, i, i);
            atest(vec_count(test_vec, int32_t) == MAX(i + 1, 5000));
        }
        atest(vec_count(test_vec, int32_t) == 50000);
        {
            size_t i2 = 0;
            vec_foreach(test_vec, int32_t, i, j) {
                atest(i == j);
                i2++;
            }
            atest(i2 == 50000);
        }
        {
            int32_t* arr = vec_array(test_vec, int32_t);
            for (size_t i = 0; i < 50000; i++) {
                atest(arr[i] == i);
            }
        }
    }
}
