/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"

#pragma librcd

void rcd_self_test_seg_stacks();
void rcd_self_test_memory();
void rcd_self_test_libgcc();
void rcd_self_test_regex();
void rcd_self_test_util_macros();
void rcd_self_test_dict();
void rcd_self_test_queue();
void rcd_self_test_avl();
void rcd_self_test_hmap();
void rcd_self_test_vm();

/// Runs all primitive rcd self tests before returning so librcd can
/// initialize I/O and then runs the rest of the tests which requires advanced
/// librcd support. By running the more primitive tests in ascending order we
/// can more accurately pinpoint what is wrong. A dependency should always
/// be tested before the high level function that depends on it.
void rcd_primitive_self_test() {
    // TODO: Replace this with a preprocessor generated list of function calls instead.
    rio_debug("[rcd_primitive_self_test]: testing memory\n");
    rcd_self_test_memory();
    rio_debug("[rcd_primitive_self_test]: testing libgcc\n");
    rcd_self_test_libgcc();
    rio_debug("[rcd_primitive_self_test]: testing regex\n");
    rcd_self_test_regex();
    rio_debug("[rcd_primitive_self_test]: testing util macros\n");
    rcd_self_test_util_macros();
    rio_debug("[rcd_primitive_self_test]: testing dict\n");
    rcd_self_test_dict();
    rio_debug("[rcd_primitive_self_test]: testing queue\n");
    rcd_self_test_queue();
    rio_debug("[rcd_primitive_self_test]: testing avl\n");
    rcd_self_test_avl();
    rio_debug("[rcd_primitive_self_test]: testing hmap\n");
    rcd_self_test_hmap();
    rio_debug("[rcd_primitive_self_test]: testing vm\n");
    rcd_self_test_vm();
}

void rcd_self_test_exceptions();
void rcd_self_test_stacklets();
void rcd_self_test_fstring();
void rcd_self_test_list();
void rcd_self_test_json();
void rcd_self_test_multi_fiber();
void rcd_self_test_io();
void rcd_self_test_ifc();

void rcd_self_test_glibrcd();
void rcd_self_test_vsnprintf();
void rcd_self_test_vfscanf();
void rcd_self_test_pthread();
void rcd_self_test_tls();
void rcd_self_test_acid();

void ipc_test_post_execve();

/// Runs advanced self tests.
void rcd_advanced_self_test(list(fstr_t)* main_args, list(fstr_t)* main_env) {
    rio_debug("[rcd_advanced_self_test]: started\n");
    if (list_count(main_args, fstr_t) > 0) {
        if (fstr_equal(list_peek_start(main_args, fstr_t), "01_55_io/ipc_test_post_execve")) {
            ipc_test_post_execve();
            exit_group(0);
        }
    }
    // TODO: Replace this with a preprocessor generated list of function calls instead.
    rio_debug("[rcd_advanced_self_test]: testing exceptions\n");
    rcd_self_test_exceptions();
    rio_debug("[rcd_advanced_self_test]: testing stacklets\n");
    rcd_self_test_stacklets();
    rio_debug("[rcd_advanced_self_test]: testing fstring\n");
    rcd_self_test_fstring();
    rio_debug("[rcd_advanced_self_test]: testing list\n");
    rcd_self_test_list();
    rio_debug("[rcd_advanced_self_test]: testing json\n");
    rcd_self_test_json();
    rio_debug("[rcd_advanced_self_test]: testing multi fiber\n");
    rcd_self_test_multi_fiber();
    rio_debug("[rcd_advanced_self_test]: testing io\n");
    rcd_self_test_io();
    rio_debug("[rcd_advanced_self_test]: testing ifc\n");
    rcd_self_test_ifc();
    rio_debug("[rcd_advanced_self_test]: testing glibrcd\n");
    rcd_self_test_glibrcd();
    rio_debug("[rcd_advanced_self_test]: testing vsnprintf\n");
    rcd_self_test_vsnprintf();
    rio_debug("[rcd_advanced_self_test]: testing vfscanf\n");
    rcd_self_test_vfscanf();
    rio_debug("[rcd_advanced_self_test]: testing pthread\n");
    rcd_self_test_pthread();
    rio_debug("[rcd_advanced_self_test]: testing tls\n");
    rcd_self_test_tls();
    rio_debug("[rcd_advanced_self_test]: testing acid\n");
    rcd_self_test_acid();
    // All test completed.
    sub_heap_e(rio_debug("rcd self-test: all tests was successful\n"));
    lwt_exit(0);
}
