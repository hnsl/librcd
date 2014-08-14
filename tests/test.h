/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef TEST_H
#define	TEST_H

#define TEST_MEM_LEAK for (size_t mem_usage = vm_total_allocated_bytes; mem_usage != UINT64_MAX; ({ mem_usage = UINT64_MAX; }))

static void test_memory_leak(size_t original_mem_usage, size_t current_mem_usage) {
    atest(current_mem_usage == original_mem_usage);
}

#endif	/* TEST_H */
