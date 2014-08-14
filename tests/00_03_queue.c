/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

typedef struct test_element {
    int32_t test_no;
    struct test_element* prev;
    struct test_element* next;
} test_element_t;

int32_t adjust_i(int32_t i, bool up) {
    return ((i % 7) == 3)? i + (up? 1: -1): i;
}

void rcd_self_test_queue() {
    struct {
        struct test_element* first;
        struct test_element* last;
    } test_queue = {0};
    test_element_t elements[100];
    for (int32_t i = 0; i < LENGTHOF(elements); i++) {
        elements[i].test_no = i;
        QUEUE_ENQUEUE(&test_queue, &elements[i]);
    }
    atest(test_queue.first != 0);
    {
        int32_t i = 0;
        for (test_element_t* elem = test_queue.first; elem != 0; elem = elem->next) {
            atest(elem->test_no == i);
            if (elem->prev == 0) {
                atest(i == 0);
            } else {
                atest(elem->prev->test_no == i - 1);
            }
            if (elem->next == 0) {
                atest(i == 99);
                elem = test_queue.last;
            } else {
                atest(elem->next->test_no == i + 1);
            }
            i++;
        }
    }
    for (int32_t i = 0; i < 20; i++) {
        test_element_t* elem = QUEUE_DEQUEUE(&test_queue);
        atest(elem != 0);
        atest(elem->test_no == i);
    }
    {
        int32_t i = 20;
        for (test_element_t* elem = test_queue.first; elem != 0; elem = elem->next) {
            atest(elem->test_no == i);
            if (elem->prev == 0) {
                atest(i == 20);
            } else {
                atest(elem->prev->test_no == i - 1);
            }
            if (elem->next == 0) {
                atest(i == 99);
                elem = test_queue.last;
            } else {
                atest(elem->next->test_no == i + 1);
            }
            i++;
        }
    }
    {
        for (test_element_t* elem = test_queue.first; elem != 0; elem = elem->next) {
            if ((elem->test_no % 7) == 3) {
                QUEUE_STEP_OUT(&test_queue, elem);
            }
        }
    }
    {
        int32_t i = 20;
        for (test_element_t* elem = test_queue.first; elem != 0; elem = elem->next) {
            atest(elem->test_no == i);
            if (elem->prev == 0) {
                atest(i == 20);
            } else {
                atest(elem->prev->test_no == adjust_i(i - 1, false));
            }
            if (elem->next == 0) {
                atest(i == 99);
                elem = test_queue.last;
            } else {
                atest(elem->next->test_no == adjust_i(i + 1, true));
            }
            i = adjust_i(i + 1, true);
        }
    }
    {
        for (int32_t i = 20;; i = adjust_i(i + 1, true)) {
            test_element_t* elem = QUEUE_DEQUEUE(&test_queue);
            if (elem == 0) {
                atest(i == 100);
                atest(test_queue.first == 0);
                atest(test_queue.last == 0);
                break;
            } else {
                atest(elem->test_no == i);
                if (i < 99) {
                    atest(test_queue.first->prev == 0);
                    atest(test_queue.last->test_no == 99);
                    atest(test_queue.last->next == 0);
                }
            }
        }
    }
}
