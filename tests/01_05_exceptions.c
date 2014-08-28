/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "test.h"

#pragma librcd

typedef struct {
    fstr_t hello;
} test2_eio_t;
define_eio_complex(test2, hello);
define_eio(test1);

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

static int test_no_warnings1() {
    try {
        for (;;);
    } finally {}
}

static int test_no_warnings2() {
    try {
        return 1;
    } catch(exception_io, e) {
        return 2;
    }
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
    // Test that multiple catch blocks work.
    {
        bool reached = false;
        try {
            throw("test exception", exception_arg);
        } catch(exception_io, e) {
            atest(false);
        } catch(exception_arg, e) {
            reached = true;
        } catch(exception_arg, e) {
            atest(false);
        }
        atest(reached);
    }
    // Tests for exceptions with eio class.
    {
        bool reached = false;
        try {
            throw_eio("test exception", test1);
        } catch_eio(test1, e, _) {
            reached = true;
        } catch(exception_io, e) {
            atest(false);
        }
        atest(reached);
    }{
        bool reached = false;
        try {
            throw_eio("test exception", test1);
        } catch(exception_io, e) {
            reached = true;
            atest(e->eio_class == test1_eio);
            atest(e->eio_data == 0);
        } catch_eio(test1, e) {
            atest(false);
        }
        atest(reached);
    }{
        bool reached = false;
        try {
            throw("test exception", exception_io);
        } catch_eio(test1, e) {
            atest(false);
        } catch(exception_io, e) {
            reached = true;
            atest(e->eio_class == 0);
            atest(e->eio_data == 0);
        }
        atest(reached);
    }{
        bool reached = false;
        try {
            try {
                throw_eio("test exception", test1);
            } catch_eio(test2, e) {
                atest(false);
            }
            atest(false);
        } catch_eio(test1, e) {
            reached = true;
        }
        atest(reached);
    }{
        volatile int32_t i = 0;
        try {
            try {
                throw_eio("test exception", test1);
            } finally {
                atest(i == 0);
                i = 1;
            } catch_eio(test2, e) {
                atest(false);
            }
            atest(false);
        } finally {
            atest(i == 1);
            i = 2;
        } catch_eio(test1, e) {
            atest(i == 2);
            i = 3;
        }
        atest(i == 3);
    }{
        try {
            emitosis(test2, data) {
                atest(data.hello.str == 0);
                data.hello = fsc("hello");
                throw_em("test exception", data);
            }
            atest(false);
        } catch_eio(test2, e, data) {
            atest(e->eio_class == test2_eio);
            test2_eio_t* real_data = (test2_eio_t*)e->eio_data;
            atest(real_data->hello.str == data.hello.str);
            atest(fstr_equal(data.hello, "hello"));
        }
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
