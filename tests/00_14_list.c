/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

void rcd_self_test_list() {
    sub_heap {
        list(int32_t)* test_list = new_list(int32_t);
        atest(list_count(test_list, int32_t) == 0);
        list_push_end(test_list, int32_t, 2);
        atest(list_count(test_list, int32_t) == 1);
        list_push_end(test_list, int32_t, 22);
        list_push_end(test_list, int32_t, 222);
        atest(list_count(test_list, int32_t) == 3);
        atest(list_peek_start(test_list, int32_t) == 2);
        atest(list_peek_end(test_list, int32_t) == 222);
        atest(list_count(test_list, int32_t) == 3);
        atest(list_pop_start(test_list, int32_t) == 2);
        atest(list_pop_end(test_list, int32_t) == 222);
        atest(list_count(test_list, int32_t) == 1);
        atest(list_pop_start(test_list, int32_t) == 22);
        try {
            atest(list_pop_start(test_list, int32_t) == false);
        } catch (exception_arg, e) {}
        atest(list_count(test_list, int32_t) == 0);
        list_push_end_n(test_list, int32_t, 7777, 66666);
        atest(list_count(test_list, int32_t) == 2);
        list_push_start_n(test_list, int32_t, 99, 888);
        atest(list_count(test_list, int32_t) == 4);
        {
            int32_t i = 0;
            list_foreach(test_list, int32_t, v) {
                atest(i <= 1);
                switch (i) {
                case 0:
                    atest(v == 99);
                    break;
                case 1:
                    atest(v == 888);
                    break;
                default:
                    atest(false);
                }
                if (i == 1)
                    break;
                i++;
            }
        }
        for (size_t construct_test_i = 0; construct_test_i < 2; construct_test_i++) {
            {
                int32_t i = 0;
                list_foreach(test_list, int32_t, v) {
                    switch (i) {
                    case 0:
                        atest(v == 99);
                        break;
                    case 1:
                        atest(v == 888);
                        break;
                    case 2:
                        atest(v == 7777);
                        list_foreach_delete_current(test_list, int32_t);
                        atest(list_count(test_list, int32_t) == 3);
                        break;
                    case 3:
                        atest(v == 66666);
                        break;
                    default:
                        atest(false);
                    }
                    i++;
                }
            }
            {
                int32_t a, b, c, d;
                atest(list_unpack(test_list, int32_t, &a, &b, &d) == true);
                atest(a == 99);
                atest(b == 888);
                atest(d == 66666);
                atest(list_unpack(test_list, int32_t, &a, &b, &c, &d) == false);
            }
            test_list = new_list(int32_t, 99, 888, 7777, 66666);
        }
    }
    sub_heap {
        list(int32_t*)* test_list = new_list(int32_t*);
        for (size_t i = 0; i < 16; i++) {
            int32_t* j = new(int32_t);
            *j = i;
            list_push_end(test_list, int32_t*, j);
        }
        size_t i = 0;
        list_foreach(test_list, int32_t*, j) {
            atest(*j == i);
            i++;
        }
    }
}
