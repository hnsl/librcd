/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"
#include "test.h"

#pragma librcd

/// Tests dynamic stack allocation using alloca.
static void test_dsa_alloca(size_t count, bool test_throw_unwind) {
    if (count == 0) {
        if (test_throw_unwind)
            throw("test throw unwind", exception_arg);
        return;
    }
    uint8_t** data = alloca(sizeof(char*) * count);
    for (size_t size = 32, i = 0; i < count; size *= 2, i++) {
        data[i] = alloca(size);
        memset(data[i], i, size);
    }
    TEST_MEM_LEAK {
        test_dsa_alloca(count - 1, test_throw_unwind);
    }
    for (size_t size = 32, i = 0; i < count; size *= 2, i++) {
        for (size_t j = 0; j < size; j++) {
            atest(data[i][j] == i);
        }
    }
}

/// Tests dynamic stack allocation using vla.
static void test_dsa_vla(size_t count, bool test_throw_unwind) {
    if (count == 0) {
        if (test_throw_unwind)
            throw("test throw unwind", exception_arg);
        return;
    }
    const size_t row_size = 32;
    uint8_t* data[count];
    uint8_t mem[count * row_size];
    for (size_t i = 0; i < count; i++) {
        data[i] = &mem[i * row_size];
        memset(data[i], i, row_size);
    }
    TEST_MEM_LEAK {
        test_dsa_vla(count - 1, test_throw_unwind);
    }
    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < row_size; j++) {
            atest(data[i][j] == i);
        }
    }
}

static void test_va_fn2(va_list args, size_t n_args) {
    for (size_t i = 0; i < n_args; i++) {
        int32_t number = va_arg(args, int32_t);
        atest(number == 100 + i);
    }
}

static size_t test_va_fn1(bool do_some_alloca, size_t n_args, ...) {
    {
        // Make sure we blow the stack and get a new stacklet by doing a large allocation here and use it.
        size_t stacklet_forcer[0x402];
        if (do_some_alloca) {
            // Do some alloca to test branch where __releasestack return pointer is shadowed. We make it hard for the compiler to optimize the call away.
            stacklet_forcer[1] = (size_t) alloca(100 + n_args * 100);
        }
        // Do something with the allocation that is hard to optimize away so the compiler must keep it.
        memset(stacklet_forcer, 0, 8);
        atest(stacklet_forcer[0] == 0);
    }{
        // Test reading the va_list.
        va_list args, args2;
        va_start(args, n_args);
        va_copy(args2, args);
        test_va_fn2(args, n_args);
        va_end(args);
        test_va_fn2(args2, n_args);
        va_end(args2);
        return n_args;
     }
}

/// Tests that memory allocated inside a try block on the same frame on the root is free'd.
fiber_main test_va_unwind_save_inner_sameframe(fiber_main_attr, size_t size) {
    uint8_t* data;
    TEST_MEM_LEAK {
        try {
            data = alloca(size);
            memset(data, 0xaa, size);
            throw("test", exception_io);
        } catch (exception_io, e);
    }
}

void rcd_self_test_stacklets() {
    //  Test dynamic stack allocation and freeing.
    for (size_t throw_test = 0; throw_test < 2; throw_test++) {
        bool test_throw_unwind = (throw_test == 1);
        size_t top_dsa_size = 42 * throw_test;
        uint8_t* top_dsa = alloca(top_dsa_size);
        memset(top_dsa, 0xaa, top_dsa_size);
        for (size_t count = 1; count < 8; count++) {
            TEST_MEM_LEAK {
                try {
                    test_dsa_alloca(count, test_throw_unwind);
                } catch (exception_arg, e) {
                    atest(test_throw_unwind);
                }
            }
            for (size_t i = 0; i < top_dsa_size; i++)
                atest(top_dsa[i] == 0xaa);
        }
        for (size_t count = 1; count < 8; count++) {
            TEST_MEM_LEAK {
                try {
                    test_dsa_vla(count, test_throw_unwind);
                } catch (exception_arg, e) {
                    atest(test_throw_unwind);
                }
            }
            for (size_t i = 0; i < top_dsa_size; i++)
                atest(top_dsa[i] == 0xaa);
        }
    }
    // Test that variable number of arguments works.
    int32_t num[16];
    for (size_t i = 0; i < 16; i++)
        num[i] = 100 + i;
    for (size_t i = 0; i < 2; i++) {
        bool do_some_alloca = (i == 1);
        TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 0);
            atest(n == 0);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 1, num[0]);
            atest(n == 1);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 2, num[0], num[1]);
            atest(n == 2);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 3, num[0], num[1], num[2]);
            atest(n == 3);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 4, num[0], num[1], num[2], num[3]);
            atest(n == 4);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 5, num[0], num[1], num[2], num[3], num[4]);
            atest(n == 5);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 6, num[0], num[1], num[2], num[3], num[4], num[5]);
            atest(n == 6);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 7, num[0], num[1], num[2], num[3], num[4], num[5], num[6]);
            atest(n == 7);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 8, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7]);
            atest(n == 8);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 9, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8]);
            atest(n == 9);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 10, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9]);
            atest(n == 10);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 11, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10]);
            atest(n == 11);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 12, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10], num[11]);
            atest(n == 12);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 13, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10], num[11], num[12]);
            atest(n == 13);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 14, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10], num[11], num[12], num[13]);
            atest(n == 14);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 15, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10], num[11], num[12], num[13], num[14]);
            atest(n == 15);
        } TEST_MEM_LEAK {
            size_t n = test_va_fn1(do_some_alloca, 16, num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10], num[11], num[12], num[13], num[14], num[15]);
            atest(n == 16);
        }
    }
    // Test dynamic alloc and unwind to root.
    fmitosis {
        ifc_wait(spawn_static_fiber(test_va_unwind_save_inner_sameframe("", 42)));
    }
    // TODO: Test different calls to slowly incrementing function frame (stacklet) sizes to detect off by-n errors in minimum stack required calculations.
}
