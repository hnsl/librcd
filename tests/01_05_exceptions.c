/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "test.h"

#pragma librcd

void rcd_self_test_exceptions_test0(volatile int32_t* i) {
    try {
        atest(*i == 0);
        (*i)++;
        return;
    } catch (exception_arg, e) {
        atest(false);
    }
    atest(false);
}

fiber_main test_exception_memory(fiber_main_attr) {
    TEST_MEM_LEAK {
        sub_heap {
            try {
                ifc_fiber_sync_recv();
                ifc_park();
                atest(false);
            } catch (exception_canceled, e) {
                rio_debug(fss(lwt_get_exception_dump(e)));
            }
        }
    }
}

void rcd_self_test_exceptions() {
    // Try catching a logical exception and dumping backtrace.
    try {
        throw("test exception", exception_arg);
    } catch (exception_arg, e) {
        fstr_mem_t* dump = lwt_get_exception_dump(e);
        rio_debug(fss(dump));
    }
    // Test that break works as intended in try/catch statements.
    {
        int32_t i = 0;
        try {
            atest(i == 0);
            i++;
            break;
        } catch (exception_any, e) {
            atest(false);
        }
    }{
        int32_t i = 0;
        try {
            throw("test exception", exception_arg);
        } catch (exception_any, e) {
            atest(i == 0);
            i++;
            break;
        }
    }
    // Test that naked finally block works.
    for (int32_t j = 0; j < 2; j++) {
        volatile int32_t i = 1;
        try {
            try {
                if (j == 1)
                    throw("test exception", exception_arg);
            } finally {
                atest(i == 1);
                i = 0;
            }
        } catch (exception_arg, e) {
            atest(i == 0);
            i = 2;
        }
        if (j == 1) {
            atest(i == 2);
        } else {
            atest(i == 0);
        }
    }
    // Test that finally block with catch works in both orders.
    {
        volatile int32_t i = 0;
        try {
            atest(i == 0);
            i++;
            throw("test exception", exception_arg);
        } finally {
            atest(i == 1);
            i = 2;
        } catch(exception_arg, e) {
            atest(i == 2);
            i = 3;
        }
        atest(i == 3);
    }{
        volatile int32_t i = 0;
        try {
            atest(i == 0);
            i++;
            throw("test exception", exception_arg);
        } catch(exception_arg, e) {
            atest(i == 2);
            i = 3;
        } finally {
            atest(i == 1);
            i = 2;
        }
        atest(i == 3);
    }
    // Test that finally block does not trigger catch block on wrong exception types.
    {
        volatile int32_t i = 0;
        try {
            try {
                atest(i == 0);
                i++;
                throw("test exception", exception_arg);
            } catch(exception_io, e) {
                atest(false);
            } finally {
                atest(i == 1);
                i = 2;
            }
        } catch(exception_arg, e) {
            atest(i == 2);
        }
        atest(i == 2);
    }
    // Test that goto, break and return works without a finally block.
    {
        volatile int32_t i = 0;
        try {
            atest(i == 0);
            i++;
            goto out0;
        } catch (exception_io, e) {
            atest(false);
        }
        out0:;
        atest(i == 1);
    }{
        volatile int32_t i = 0;
        try {
            atest(i == 0);
            i++;
            break;
        } catch (exception_io, e) {
            atest(false);
        }
        atest(i == 1);
    }{
        volatile int32_t i = 0;
        rcd_self_test_exceptions_test0(&i);
        atest(i == 1);
    }
    // Test that catching and printing a cancellation exception does not cause memory leaks.
    fmitosis {
        rcd_sub_fiber_t* sf = spawn_fiber(test_exception_memory(""));
        ifc_fiber_sync_send(sfid(sf));
        lwt_cancel_fiber_id(sfid(sf));
        ifc_wait(sfid(sf));
    }
    // TODO: Add more tests.
}
