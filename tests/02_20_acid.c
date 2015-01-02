/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

static void test_acid_test0(fstr_t data_file, fstr_t journal_file) {
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 100 * PAGE_SIZE);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 100 * PAGE_SIZE);
    // Read memory, ensure it's all zeroes.
    for (size_t i = 0; i < 100 * PAGE_SIZE; i++)
        atest(mem.str[i] == 0);
    // Write the last half of the first page and sync.
    for (size_t i = PAGE_SIZE / 2; i < PAGE_SIZE; i++)
        mem.str[i] = (i % 0x100);
    acid_fsync(ah);
    // Verify the last half of the page.
    for (size_t i = PAGE_SIZE / 2; i < PAGE_SIZE; i++)
        atest(mem.str[i] == (i % 0x100));
    // Close acid handle.
    acid_close(ah);
}

static void test_acid_test1(fstr_t data_file, fstr_t journal_file) {
    // Open the database again.
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 0);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 100 * PAGE_SIZE);
    // Read memory, assert first page is not corrupt.
    for (size_t i = 0; i < PAGE_SIZE / 2; i++)
        atest(mem.str[i] == 0);
    for (size_t i = PAGE_SIZE / 2; i < PAGE_SIZE; i++)
        atest(mem.str[i] == (i % 0x100));
    // Read memory, ensure it's all zeroes.
    for (size_t i = 1 * PAGE_SIZE; i < 100 * PAGE_SIZE; i++)
        atest(mem.str[i] == 0);
    // Close acid handle.
    acid_close(ah);
}

static void test_acid_test2(fstr_t data_file, fstr_t journal_file) {
    // Open the database again, shrinking it to 50 pages.
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 50 * PAGE_SIZE);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 50 * PAGE_SIZE);
    // Write to pages in pseudo random order.
    for (size_t i = 25; i < 50; i++) {
        size_t k = ((i % 2) == 1)? 49 - i + 25: i;
        mem.str[k * PAGE_SIZE + 3] = 0x20 + k;
        mem.str[k * PAGE_SIZE + PAGE_SIZE / 2] = 0x100 - k;
        mem.str[k * PAGE_SIZE + PAGE_SIZE - 1] = 0x60 + k;
    }
    // Check that all pages have the correct values.
    for (size_t i = 25; i < 50; i++) {
        size_t k = ((i % 2) == 1)? 49 - i + 25: i;
        atest(mem.str[k * PAGE_SIZE + 3] == 0x20 + k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE / 2] == 0x100 - k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE - 1] == 0x60 + k);
    }
    // Close acid handle.
    acid_close(ah);
}

static void test_acid_test3(fstr_t data_file, fstr_t journal_file) {
    // Open the database again.
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 0);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 50 * PAGE_SIZE);
    // Check that all pages have the correct values from previous test.
    for (size_t i = 25; i < 50; i++) {
        size_t k = ((i % 2) == 1)? 49 - i + 25: i;
        atest(mem.str[k * PAGE_SIZE + 3] == 0x20 + k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE / 2] == 0x100 - k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE - 1] == 0x60 + k);
    }
    // Write to pages in pseudo random order.
    for (size_t i = 25; i < 50; i++) {
        size_t k = ((i % 2) == 1)? 49 - i + 25: i;
        mem.str[k * PAGE_SIZE + 2] = 0x25 + k;
        mem.str[k * PAGE_SIZE + PAGE_SIZE / 2 + 1] = 0x90 - k;
        mem.str[k * PAGE_SIZE + PAGE_SIZE - 2] = 0x55 + k;
        // Try to take snapshot.
        bool snap_ok = acid_snapshot(ah);
        if (i == 25) {
            // First snapshot should always succeed since sync thread is idle.
            atest(snap_ok);
        } else {
            // Other snapshots are improbable.
            improb_atest(!snap_ok, "snapshot was successful even though sync thread should be busy waiting for disk");
        }
    }
    // Close acid handle.
    acid_close(ah);
}

static void test_acid_test4(fstr_t data_file, fstr_t journal_file) {
    // Open the database again.
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 0);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 50 * PAGE_SIZE);
    // Check that all pages have the correct values from previous test.
    for (size_t i = 25; i < 50; i++) {
        size_t k = ((i % 2) == 1)? 49 - i + 25: i;
        atest(mem.str[k * PAGE_SIZE + 3] == 0x20 + k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE / 2] == 0x100 - k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE - 1] == 0x60 + k);
        atest(mem.str[k * PAGE_SIZE + 2] == 0x25 + k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE / 2 + 1] == 0x90 - k);
        atest(mem.str[k * PAGE_SIZE + PAGE_SIZE - 2] == 0x55 + k);
    }
    // Ensure resize to smaller range should not change anything.
    acid_expand(ah, 25 * PAGE_SIZE);
    atest(acid_memory(ah).len == 50 * PAGE_SIZE);
    acid_expand(ah, 50 * PAGE_SIZE);
    atest(acid_memory(ah).len == 50 * PAGE_SIZE);
    // Test live expand.
    acid_expand(ah, 150 * PAGE_SIZE);
    mem = acid_memory(ah);
    atest(mem.len == 150 * PAGE_SIZE);
    // Read new memory, ensure it's all zeroes.
    for (size_t i = 50 * PAGE_SIZE; i < 150 * PAGE_SIZE; i++)
        atest(mem.str[i] == 0);
    // Write to new pages after forcing a snapshot to be taken.
    for (size_t i = 50 * PAGE_SIZE; i < 150 * PAGE_SIZE; i++)
        mem.str[i] = (i % 0x100);
    // Check and write to new pages again backwards after triggering snapshot, this should make COW very likely.
    atest(acid_snapshot(ah));
    for (size_t i = 150 * PAGE_SIZE - 1; i >= 50 * PAGE_SIZE; i--) {
        atest(mem.str[i] == (i % 0x100));
        mem.str[i] = ((i + 0x5e) % 0x100);
    }
    // Test just calling fsync and close acid handle.
    acid_fsync(ah);
    acid_close(ah);
}

static void test_acid_test5(fstr_t data_file, fstr_t journal_file) {
    // Open the database again.
    acid_h* ah = acid_open(data_file, journal_file, ACID_ADDR_0, 0);
    // Get memory range.
    fstr_t mem = acid_memory(ah);
    atest(mem.len == 150 * PAGE_SIZE);
    // Check memory.
    for (size_t i = 50 * PAGE_SIZE; i < 150 * PAGE_SIZE; i++)
        atest(mem.str[i] == ((i + 0x5e) % 0x100));

    // Close acid handle.
    acid_close(ah);
}

void rcd_self_test_acid() { sub_heap {
    fstr_t data_file = concs("/var/tmp/.librcd-acid-test.", lwt_rdrand64(), ".data");
    fstr_t journal_file = concs("/var/tmp/.librcd-acid-test.", lwt_rdrand64(), ".jrnl");
    test_acid_test0(data_file, journal_file);
    test_acid_test1(data_file, journal_file);
    test_acid_test2(data_file, journal_file);
    test_acid_test3(data_file, journal_file);
    test_acid_test4(data_file, journal_file);
    test_acid_test5(data_file, journal_file);


    // Unit test complete. Delete the data base.
    rio_file_unlink(data_file);
    rio_file_unlink(journal_file);
}}

