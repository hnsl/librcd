/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"
#include "test.h"

#pragma librcd

/*
void test_fsbuf() {
    fstr_t no10 = "1234567890";
    fsbuf_t* buf = fsbuf_open(3000);
    for (int i = 0; i < 200; i++) {
        atest(fsbuf_write(buf, no10));
    }
    fstr_t cur = fsbuf_get_current(buf);
    atest(cur.len == no10.len * 200)
    for (int i = 0; i < cur.len; i++) {
        atest(cur.str[i] == no10.str[i % no10.len]);
    }
    fsbuf_free(buf);
    const int test_max_len = 500000;
    fsbuf_t* buf = fsbuf_open(test_max_len);
    int max_no10s = test_max_len / no10.len;
    for (int i = 0; i < max_no10s; i++) {
        atest(fsbuf_write(buf, no10));
    }
    atest(!fsbuf_is_overflowed(buf));
    for (int i = 0; i < 10; i++) {
        // Try and write more to the buffer - should fail after a while as we're on the edge of overflow.
        for (int j = 0; j < max_no10s; j++) {
            if (!fsbuf_write(buf, no10))
                goto ok_buffer_full;
        }
        CU_FAIL_FATAL("buffer failed to become full");
        ok_buffer_full:
        size_t current_len = fsbuf_get_current(buf).len;
        atest(current_len > test_max_len);
        // Remove the first 10000 * i bytes in the buffer and fill it up again.
        fsbuf_discard_front(buf, 10000 * i);
        atest(fsbuf_get_current(buf).len, current_len - 10000 * i);
        fstr_t cur = fsbuf_get_current(buf);
        for (int i = 0; i < cur.len; i++) {
            atest(cur.str[i], no10.str[i % no10.len]);
        }
    }
    atest(fsbuf_is_overflowed(buf));
    fsbuf_free(buf);
}*/

/*
void test_fstrscan() {
    // Test finding in a large file.
    fstr_t* alice = sysif_read_file_contents(malloc_fng, "alice.txt", SYSIF_STD1M_MAX_BUF);
    {
        fstr_t find = FSC("four times seven is");
        int64_t offset = fstrscan(*alice, find);
        CU_ASSERT_NOT_EQUAL_FATAL(offset, -1);
        fstr_t after_expected = FSC("--oh dear!");
        fstr_t after = {.str = alice->str + offset + find.len, .len = after_expected.len};
        CU_ASSERT_NSTRING_EQUAL_FATAL(after.str, after_expected.str, after.len);
    }
    {
        fstr_t find = FSC("five times seven is");
        int64_t offset = fstrscan(*alice, find);
        atest(offset, -1);
    }
}*/

fstr_t test_hash_str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed odio nibh, malesuada ut dapibus sit amet, interdum fringilla elit. Sed sed lobortis ante. Cras varius diam id diam fringilla id varius libero aliquet. Etiam id tortor a felis aliquet placerat. Nunc ac velit velit";

void rcd_self_test_fstring() {
    // Test fstr case character check and compare fns.
    for (size_t i = 0; i < 'A'; i++) {
        atest(!fstr_cisupper(i));
        atest(!fstr_cislower(i));
        atest(fstr_ctoupper(i) == i);
        atest(fstr_ctolower(i) == i);
    }
    for (size_t i = 'A'; i < 'Z' + 1; i++) {
        atest(fstr_cisupper(i));
        atest(!fstr_cislower(i));
        atest(fstr_ctoupper(i) == i);
        atest(fstr_ctolower(i) == (i - 'A' + 'a'));
    }
    for (size_t i = 'Z' + 1; i < 'a'; i++) {
        atest(!fstr_cisupper(i));
        atest(!fstr_cislower(i));
        atest(fstr_ctoupper(i) == i);
        atest(fstr_ctolower(i) == i);
    }
    for (size_t i = 'a'; i < 'z' + 1; i++) {
        atest(!fstr_cisupper(i));
        atest(fstr_cislower(i));
        atest(fstr_ctoupper(i) == (i - 'a' + 'A'));
        atest(fstr_ctolower(i) == i);
    }
    for (size_t i = 'z' + 1; i < 0x100; i++) {
        atest(!fstr_cisupper(i));
        atest(!fstr_cislower(i));
        atest(fstr_ctoupper(i) == i);
        atest(fstr_ctolower(i) == i);
    }
    atest(fstr_equal_case("a", "a"));
    atest(fstr_equal_case("a", "A"));
    atest(fstr_equal_case("A", "a"));
    atest(fstr_equal_case("A", "A"));
    atest(fstr_equal_case("aa", "aa"));
    atest(fstr_equal_case("aa", "aA"));
    atest(fstr_equal_case("aA", "aa"));
    atest(fstr_equal_case("AA", "AA"));
    atest(!fstr_equal_case("aa", "ab"));
    atest(!fstr_equal_case("ba", "aA"));
    atest(!fstr_equal_case("aB", "aa"));
    atest(!fstr_equal_case("AA", "AB"));
    // Test ftsr alloc.
    sub_heap {
        fstr_mem_t* str = fstr_alloc(100);
        atest(str->len == 100);
    }
    // Test fstr compare.
    atest(fstr_cmp("abc", "def") < 0);
    atest(fstr_cmp("zzz", "eee") > 0);
    atest(fstr_cmp("qwerty", "qwerty") == 0);
    atest(!fstr_equal("qwerty", "qwert."));
    atest(fstr_equal("qwerty", "qwerty"));
    atest(fstr_equal("", ""));
    atest(!fstr_equal("", "e"));
    // Test fstr_cmp_natural.
    atest(fstr_cmp_natural("abc", "def") < 0);
    atest(fstr_cmp_natural("zzz", "eee") > 0);
    atest(fstr_cmp_natural("qwerty", "qwerty") == 0);
    atest(fstr_cmp_natural("qwerty", "qwert.") > 0);
    atest(fstr_cmp_natural("", "") == 0);
    atest(fstr_cmp_natural("", "e") < 0);
    atest(fstr_cmp_natural("", "5") < 0);
    atest(fstr_cmp_natural("5", "5") == 0);
    atest(fstr_cmp_natural("5", "") > 0);
    atest(fstr_cmp_natural("3", "5") < 0);
    atest(fstr_cmp_natural("5", "5") == 0);
    atest(fstr_cmp_natural("7", "5") > 0);
    atest(fstr_cmp_natural("abc3", "abc5") < 0);
    atest(fstr_cmp_natural("abc5", "abc5") == 0);
    atest(fstr_cmp_natural("abc7", "abc5") > 0);
    atest(fstr_cmp_natural("abc5", "abc33") < 0);
    atest(fstr_cmp_natural("abc55", "abc5") > 0);
    atest(fstr_cmp_natural("abc75", "abc5xxx") > 0);
    atest(fstr_cmp_natural("abc123efg", "abc123efg") == 0);
    atest(fstr_cmp_natural("abc123efghi", "abc1234efg") < 0);
    atest(fstr_cmp_natural("abc123efg456hij789", "abc123efg456hij78") > 0);
    atest(fstr_cmp_natural("cool9.png", "cool10.png") < 0);
    atest(fstr_cmp_natural("cool1.png", "cool100.png") < 0);
    // Test fstr prefixes.
    atest(fstr_prefixes("", ""));
    atest(!fstr_prefixes("", "a"));
    atest(!fstr_prefixes("", "ab"));
    atest(fstr_prefixes("a", ""));
    atest(fstr_prefixes("a", "a"));
    atest(!fstr_prefixes("a", "ab"));
    atest(!fstr_prefixes("a", "abc"));
    atest(!fstr_prefixes("a", "ba"));
    atest(!fstr_prefixes("a", "b"));
    atest(fstr_prefixes("ab", ""));
    atest(fstr_prefixes("ab", "a"));
    atest(!fstr_prefixes("ab", "b"));
    atest(fstr_prefixes("ab", "ab"));
    atest(!fstr_prefixes("ab", "abc"));
    atest(!fstr_prefixes("ab", "cab"));
    atest(!fstr_prefixes("ab", "cba"));
    atest(fstr_prefixes(test_hash_str, "Lorem ipsum dolor sit amet"));
    atest(!fstr_prefixes(test_hash_str, "Lorem ipsum dolor sit ameq"));
    atest(!fstr_prefixes(test_hash_str, "orem ipsum dolor sit ameq"));
    atest(!fstr_prefixes(test_hash_str, "rat. Nunc ac velit velit"));
    atest(!fstr_prefixes(test_hash_str, "rat. Nunc ac velit veli"));
    atest(!fstr_prefixes(test_hash_str, "oat. Nunc ac velit velit"));
    // Test fstr suffixes.
    atest(fstr_suffixes("", ""));
    atest(!fstr_suffixes("", "a"));
    atest(!fstr_suffixes("", "ab"));
    atest(fstr_suffixes("a", ""));
    atest(fstr_suffixes("a", "a"));
    atest(!fstr_suffixes("a", "ab"));
    atest(!fstr_suffixes("a", "abc"));
    atest(!fstr_suffixes("a", "ba"));
    atest(!fstr_suffixes("a", "b"));
    atest(fstr_suffixes("ab", ""));
    atest(!fstr_suffixes("ab", "a"));
    atest(fstr_suffixes("ab", "b"));
    atest(fstr_suffixes("ab", "ab"));
    atest(!fstr_suffixes("ab", "abc"));
    atest(!fstr_suffixes("ab", "cab"));
    atest(!fstr_suffixes("ab", "cba"));
    atest(!fstr_suffixes(test_hash_str, "Lorem ipsum dolor sit amet"));
    atest(!fstr_suffixes(test_hash_str, "Lorem ipsum dolor sit ameq"));
    atest(!fstr_suffixes(test_hash_str, "orem ipsum dolor sit ameq"));
    atest(fstr_suffixes(test_hash_str, "rat. Nunc ac velit velit"));
    atest(!fstr_suffixes(test_hash_str, "rat. Nunc ac velit veli"));
    atest(!fstr_suffixes(test_hash_str, "oat. Nunc ac velit velit"));
    // Test fstr number serialization/deserialization.
    sub_heap {
        {
            uint128_t out;
            atest(fstr_unserial_uint("22...", 10, &out) == true);
            atest(out == 22);
        }
        {
            uint128_t out;
            atest(fstr_unserial_uint("2qwerty3af", 10, &out) == true);
            atest(out == 23);
        }
        {
            int128_t out;
            atest(fstr_unserial_int(" 24  ", 10, &out) == true);
            atest(out == 24);
        }
        {
            int128_t out;
            atest(fstr_unserial_int("- 25 ", 10, &out) == true);
            atest(out == -25);
        }{
            fstr_t buf;
            FSTR_STACK_DECL(buf, 32);
            fstr_t out = fstr_serial_uint(buf, 12345, 10);
            atest(fstr_equal(out, "12345"));
        }{
            fstr_t buf;
            FSTR_STACK_DECL(buf, 32);
            fstr_t out = fstr_serial_int(buf, 12345, 10);
            atest(fstr_equal(out, "12345"));
        }{
            fstr_t buf;
            FSTR_STACK_DECL(buf, 32);
            fstr_t out = fstr_serial_int(buf, -12345, 10);
            atest(fstr_equal(out, "-12345"));
        }
        atest(fstr_to_uint("10", 10) == 10);
        atest(fstr_to_int("10", 10) == 10);
        atest(fstr_to_int("-10", 10) == -10);
        atest(fstr_to_uint("a", 16) == 10);
        atest(fstr_to_int("a", 16) == 10);
        atest(fstr_to_int("A", 16) == 10);
        atest(fstr_to_int("-a", 16) == -10);
        atest(fstr_to_int("-216476413347656565245", 8) == -2569251597888187045);
        atest(fstr_equal(fss(fstr_from_uint(10, 10)), "10"));
        atest(fstr_equal(fss(fstr_from_int(10, 10)), "10"));
        atest(fstr_equal(fss(fstr_from_int(-10, 10)), "-10"));
        // Test all the corner cases.
        for (uint8_t base = 2; base <= 36; base++)
            atest(fstr_to_int("0", base) == 0);
        atest(fstr_to_int("7FFFFFFFFFFFFFFF", 16) == LONG_MAX);
        atest(fstr_to_int("-8000000000000000", 16) == LONG_MIN);
        atest(fstr_to_uint("FFFFFFFFFFFFFFFF", 16) == ULONG_MAX);
        atest(fstr_to_uint("0000000000000000", 16) == 0);
        atest(fstr_to_int("111111111111111111111111111111111111111111111111111111111111111", 2) == LONG_MAX);
        atest(fstr_to_int("-170141183460469231731687303715884105727", 10) == (INT128_MIN + 1));
        atest(fstr_to_int("-170141183460469231731687303715884105728", 10) == INT128_MIN);
        atest(fstr_to_int("-170141183460469231731687303715884105729", 10) == 0);
        atest(fstr_to_int("-7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16) == (INT128_MIN + 1));
        atest(fstr_to_int("-80000000000000000000000000000000", 16) == INT128_MIN);
        atest(fstr_to_int("-80000000000000000000000000000001", 16) == 0);
        atest(fstr_to_int("170141183460469231731687303715884105727", 10) == INT128_MAX);
        atest(fstr_to_int("170141183460469231731687303715884105728", 10) == 0);
        atest(fstr_to_uint("340282366920938463463374607431768211455", 10) == UINT128_MAX);
        atest(fstr_to_uint("7vvvvvvvvvvvvvvvvvvvvvvvvv", 32) == UINT128_MAX);
        atest(fstr_to_uint("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16) == UINT128_MAX);
        atest(fstr_to_uint("11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", 2) == UINT128_MAX);
        // Try some errornous calls.
        {
            uint128_t out;
            atest(fstr_unserial_uint("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0", 16, &out) == false);
        }
        {
            int128_t out;
            atest(fstr_unserial_int("8FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16, &out) == false);
        }
        try {
            atest(fstr_to_int("23", 1) == 0);
        } catch (exception_any, e) {
            atest(false);
        }
        try {
            atest(fstr_to_int("23", 37) == 0);
        } catch (exception_any, e) {
            atest(false);
        }
        // Test double unserialization.
        {
            double d = fstr_to_double("");
            atest(d == 0.0);
        }{
            double d = fstr_to_double("0");
            atest(d == 0.0);
        }{
            double d = fstr_to_double("0.5");
            atest(d == 0.5);
        }{
            double d = fstr_to_double("0.25");
            atest(d == 0.25);
        }{
            double d = fstr_to_double("12345");
            atest(d == 12345);
        }{
            double d = fstr_to_double("1e100");
            atest(d == 1e100);
        }{
            double d = fstr_to_double("-1e100");
            atest(d == -1e100);
        }{
            double d = fstr_to_double("1e-100");
            atest(d == 1e-100);
        }{
            double d = fstr_to_double("-1e-100");
            atest(d == -1e-100);
        }{
            double d = fstr_to_double("1.23455432101234564470360055566E4");
            atest(d == 1.23455432101234564470360055566E4);
        }{
            double d = fstr_to_double("3.33333333333333314829616256247E-1");
            atest(d == 3.33333333333333314829616256247E-1);
        }{
            double d = fstr_to_double("inf");
            atest(isinf(d));
        }{
            double d = fstr_to_double("INFINITY");
            atest(isinf(d));
        }{
            double d = fstr_to_double("-inf");
            atest(isinf(d));
        }{
            double d = fstr_to_double("nan");
            atest(isnan(d));
        }{
            double d = fstr_to_double("NaN");
            atest(isnan(d));
        }
        // Test double serialization.
        TEST_MEM_LEAK {
            sub_heap {
                fstr_mem_t* ds = fstr_from_double(0);
                atest(fstr_equal(fss(ds), "0"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(0.5);
                atest(fstr_equal(fss(ds), "0.5"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(12345);
                atest(fstr_equal(fss(ds), "12345"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(100);
                atest(fstr_equal(fss(ds), "100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(9e9);
                atest(fstr_equal(fss(ds), "9000000000"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1.23e-5);
                atest(fstr_equal(fss(ds), "0.0000123"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1.23e-5);
                atest(fstr_equal(fss(ds), "-0.0000123"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1e-6);
                atest(fstr_equal(fss(ds), "1e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1e-6);
                atest(fstr_equal(fss(ds), "-1e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(9.98e-6);
                atest(fstr_equal(fss(ds), "9.98e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-9.98e-6);
                atest(fstr_equal(fss(ds), "-9.98e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(4e11);
                atest(fstr_equal(fss(ds), "400000000000"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-4e11);
                atest(fstr_equal(fss(ds), "-400000000000"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(4.567e11);
                atest(fstr_equal(fss(ds), "456700000000"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-4.567e11);
                atest(fstr_equal(fss(ds), "-456700000000"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(4e12);
                atest(fstr_equal(fss(ds), "4e12"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-4e12);
                atest(fstr_equal(fss(ds), "-4e12"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(4.567e12);
                atest(fstr_equal(fss(ds), "4.567e12"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-4.567e12);
                atest(fstr_equal(fss(ds), "-4.567e12"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1e100);
                atest(fstr_equal(fss(ds), "1e100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1e100);
                atest(fstr_equal(fss(ds), "-1e100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1e-100);
                atest(fstr_equal(fss(ds), "1e-100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1e-100);
                atest(fstr_equal(fss(ds), "-1e-100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1.23456789e100);
                atest(fstr_equal(fss(ds), "1.23456789e100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1.23456789e100);
                atest(fstr_equal(fss(ds), "-1.23456789e100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1.23456789e-100);
                atest(fstr_equal(fss(ds), "1.23456789e-100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-1.23456789e-100);
                atest(fstr_equal(fss(ds), "-1.23456789e-100"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(1.23455432101234564470360055566E4);
                atest(fstr_equal(fss(ds), "12345.543210123456"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333314829616256247E-1);
                atest(fstr_equal(fss(ds), "0.3333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333328707404064062E-2);
                atest(fstr_equal(fss(ds), "0.03333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333354728256203714E-3);
                atest(fstr_equal(fss(ds), "0.0033333333333333335"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333322202191029149E-4);
                atest(fstr_equal(fss(ds), "0.0003333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333349307245341286E-5);
                atest(fstr_equal(fss(ds), "0.000033333333333333335"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.333333333333333323665863962E-6);
                atest(fstr_equal(fss(ds), "3.3333333333333333e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(3.33333333333333353542410077558E-7);
                atest(fstr_equal(fss(ds), "3.3333333333333335e-7"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333314829616256247E-1);
                atest(fstr_equal(fss(ds), "-0.3333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333328707404064062E-2);
                atest(fstr_equal(fss(ds), "-0.03333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333354728256203714E-3);
                atest(fstr_equal(fss(ds), "-0.0033333333333333335"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333322202191029149E-4);
                atest(fstr_equal(fss(ds), "-0.0003333333333333333"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333349307245341286E-5);
                atest(fstr_equal(fss(ds), "-0.000033333333333333335"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.333333333333333323665863962E-6);
                atest(fstr_equal(fss(ds), "-3.3333333333333333e-6"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(-3.33333333333333353542410077558E-7);
                atest(fstr_equal(fss(ds), "-3.3333333333333335e-7"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(INFINITY);
                atest(fstr_equal(fss(ds), "Infinity"));
            } sub_heap {
                fstr_mem_t* ds = fstr_from_double(NAN);
                atest(fstr_equal(fss(ds), "NaN"));
            }
        }
    }
    // Test fstr slice.
    sub_heap {
        for (size_t mlen = 0; mlen < 5; mlen++) {
            fstr_t data = fss(fstr_alloc(mlen));
            for (size_t n = 0; n < mlen; n++)
                data.str[n] = (n + 1);
            for (size_t i = 0; i < mlen; i++)
            for (size_t j = 0; j < mlen; j++) {
                fstr_t slice = fstr_slice(data, i, j);
                atest(slice.len == MIN(data.len, (i > j? 0: j - i)));
                for (size_t n = 0; n < slice.len; n++)
                    atest(slice.str[n] == (i + n + 1));
            }
        }
    }
    // Test fstr slice.
    {
        fstr_t str = {.len = 10, .str = (void*) 0};
        fstr_t str2 = fstr_slice(str, 5, 9);
        atest(str2.len == 4);
        atest(str2.str == (void*) 5);
    }{
        fstr_t str = {.len = 0x90000000, .str = (void*) 0x9000000000000000};
        fstr_t str2 = fstr_sslice(str, 0x89900000, -1);
        atest(str2.len == 0x6700000);
        atest(str2.str == (void*) 0x9000000089900000);
    }
    // Test fstr scan.
    {
        atest(fstr_scan("", "") == 0);
        atest(fstr_scan("a", "") == 0);
        atest(fstr_scan("a", "a") == 0);
        atest(fstr_scan("a", "b") == -1);
        atest(fstr_scan("aa", "a") == 0);
        atest(fstr_scan("aa", "b") == -1);
        atest(fstr_scan("ba", "a") == 1);
        atest(fstr_scan(test_hash_str, "lobortis") == 136);
        atest(fstr_scan("lobortis", test_hash_str) == -1);
        atest(fstr_scan("", "a") == -1);
        atest(fstr_scan("qqaaxvvaaxzz", "aax") == 2);
    }
    // Test fstr reverse scan.
    {
        atest(fstr_rscan("", "") == 0);
        atest(fstr_rscan("a", "") == 1);
        atest(fstr_rscan("a", "a") == 0);
        atest(fstr_rscan("a", "b") == -1);
        atest(fstr_rscan("aa", "a") == 1);
        atest(fstr_rscan("aa", "b") == -1);
        atest(fstr_rscan("ba", "a") == 1);
        atest(fstr_rscan(test_hash_str, "lobortis") == 136);
        atest(fstr_rscan("lobortis", test_hash_str) == -1);
        atest(fstr_rscan("", "a") == -1);
        atest(fstr_rscan("qqaaxvvaaxzz", "aax") == 7);
    }
    // Test fstr explode.
    sub_heap {
        {
            list(fstr_t)* test = fstr_explode("abcbd", "b");
            atest(list_count(test, fstr_t) == 3);
            fstr_t element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, "a"));
            element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, "c"));
            element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, "d"));
            atest(list_count(test, fstr_t) == 0);
        }{
            list(fstr_t)* test = fstr_explode("qq", "q");
            atest(list_count(test, fstr_t) == 3);
            fstr_t element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, ""));
            element = list_pop_start(test, fstr_t);
            atest(list_count(test, fstr_t) == 1);
            atest(fstr_equal(element, ""));
            element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, ""));
        }{
            list(fstr_t)* test = fstr_explode("eeeeeeeeeeeeeeeeeeee", "\x00");
            atest(list_count(test, fstr_t) == 1);
            fstr_t element = list_pop_start(test, fstr_t);
            atest(fstr_equal(element, "eeeeeeeeeeeeeeeeeeee"));
        }{
            list(fstr_t)* test = fstr_explode("foo!@:bar!@:baz!@:biz0!@!@:!!!!@:", "!@:");
            fstr_t e0, e1, e2, e3, e4, e5;
            atest(list_unpack(test, fstr_t, &e0, &e1, &e2, &e3, &e4, &e5));
            atest(fstr_equal(e0, "foo"));
            atest(fstr_equal(e1, "bar"));
            atest(fstr_equal(e2, "baz"));
            atest(fstr_equal(e3, "biz0!@"));
            atest(fstr_equal(e4, "!!!"));
            atest(fstr_equal(e5, ""));
        }{
            /// Detects trying to match invalid memory.
            list(fstr_t)* test = fstr_explode(fss(fstr_cpy("1234")), "34\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");
            atest(list_count(test, fstr_t) == 1);
        }
    }

    // TODO: TEST FSTR IMPLODE/CONC, SPECIFICALLY TEST IT WITH AN EMPTY STRING @ THE END.

    // Test fstr_replace.
    sub_heap {
        fstr_t test = fss(fstr_replace("alpha beta delta", "beta", "omega"));
        atest(fstr_equal(test, "alpha omega delta"));
        fstr_t test2 = fss(fstr_replace("the best the best the best the best the best the best the best the best the best of you", "best", "worst"));
        atest(fstr_equal(test2, "the worst the worst the worst the worst the worst the worst the worst the worst the worst of you"));
        fstr_t test3 = fss(fstr_replace("foo", "bar", "baz"));
        atest(fstr_equal(test3, "foo"));
    }
    // Test fstr_divide.
    TEST_MEM_LEAK {
        fstr_t before, after;
        {
            atest(!fstr_divide("", "e", &before, &after));
        }{
            atest(fstr_divide("", "", &before, &after));
            atest(fstr_equal(before, ""));
            atest(fstr_equal(after, ""));
        }{
            atest(!fstr_divide("foo", ":", &before, &after));
        }{
            atest(fstr_divide("foo", "", &before, &after));
            atest(fstr_equal(before, ""));
            atest(fstr_equal(after, "foo"));
        }{
            atest(fstr_divide("foo:bar:baz", ":", &before, &after));
            atest(fstr_equal(before, "foo"));
            atest(fstr_equal(after, "bar:baz"));
        }{
            atest(fstr_divide("foo:bar:baz", ":", 0, &after));
            atest(fstr_equal(before, "foo"));
        }{
            atest(fstr_divide("foo:bar:baz", ":", &before, 0));
            atest(fstr_equal(after, "bar:baz"));
        }{
            atest(fstr_divide("foo:bar:baz", ":", 0, 0));
        }{
            atest(fstr_divide("foo:bar:baz", "foo", &before, &after));
            atest(fstr_equal(before, ""));
            atest(fstr_equal(after, ":bar:baz"));
        }{
            atest(fstr_divide("foo:bar:baz", "baz", &before, &after));
            atest(fstr_equal(before, "foo:bar:"));
            atest(fstr_equal(after, ""));
        }{
            atest(fstr_divide("foo:bar:baz", "foo:bar:baz", &before, &after));
            atest(fstr_equal(before, ""));
            atest(fstr_equal(after, ""));
        }{
            atest(fstr_divide(" foo: bar :\tbaz ", ":", &before, &after));
            atest(fstr_equal(before, " foo"));
            atest(fstr_equal(after, " bar :\tbaz "));
            atest(fstr_divide(after, ":", &before, &after));
            atest(fstr_equal(before, " bar "));
            atest(fstr_equal(after, "\tbaz "));
            atest(!fstr_divide(after, ":", &before, &after));
        }
    }
    // Test fstr_iterate.
    for (size_t pass = 0; pass < 2; pass++) TEST_MEM_LEAK {
        fstr_t src, chunk;
        fstr_t* chunk_ptr = (pass == 0? &chunk: 0);
        {
            src = "";
            atest(!fstr_iterate(&src, "", chunk_ptr));
        }{
            src = "";
            atest(!fstr_iterate(&src, ":", chunk_ptr));
        }{
            src = ":";
            atest(!fstr_iterate(&src, "", chunk_ptr));
        }{
            src = ":";
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(!fstr_iterate(&src, ":", chunk_ptr));
        }{
            src = "a:b";
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(!fstr_iterate(&src, ":", chunk_ptr));
        }{
            src = "a::b";
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(!fstr_iterate(&src, ":", chunk_ptr));
        }{
            src = "a::b:";
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(!fstr_iterate(&src, ":", chunk_ptr));
        }{
            src = "foo!bar!foo!bar";
            atest(fstr_iterate(&src, "foo", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "!"));
            atest(fstr_iterate(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "!foo!"));
            atest(!fstr_iterate(&src, "bar", chunk_ptr));
        }
    }
    // Test fstr_iterate_trim.
    for (size_t pass = 0; pass < 2; pass++) TEST_MEM_LEAK {
        fstr_t src, chunk;
        fstr_t* chunk_ptr = (pass == 0? &chunk: 0);
        {
            src = "";
            atest(!fstr_iterate_trim(&src, "", chunk_ptr));
        }{
            src = "";
            atest(!fstr_iterate_trim(&src, "\t", chunk_ptr));
        }{
            src = " :\t";
            atest(!fstr_iterate_trim(&src, "", chunk_ptr));
        }{
            src = " :\t";
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(!fstr_iterate_trim(&src, ":", chunk_ptr));
        }{
            src = " a \t\nb \t c\t d ";
            atest(fstr_iterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_iterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "c"));
            atest(fstr_iterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "d"));
            atest(!fstr_iterate_trim(&src, " ", chunk_ptr));
        }{
            src = "\t \na\n\t :  \n :b \t ";
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(!fstr_iterate_trim(&src, ":", chunk_ptr));
        }{
            src = " \n   \na\t\n  :  \n :\nb\n:";
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(!fstr_iterate_trim(&src, ":", chunk_ptr));
        }{
            src = "  foo bar\tfoo  \n bar\n\n";
            atest(fstr_iterate_trim(&src, "foo", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate_trim(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_iterate_trim(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "foo"));
            atest(!fstr_iterate_trim(&src, "bar", chunk_ptr));
        }
    }
    // Test fstr_riterate.
    for (size_t pass = 0; pass < 2; pass++) TEST_MEM_LEAK {
        fstr_t src, chunk;
        fstr_t* chunk_ptr = (pass == 0? &chunk: 0);
        {
            src = "";
            atest(!fstr_riterate(&src, "", chunk_ptr));
        }{
            src = "";
            atest(!fstr_riterate(&src, ":", chunk_ptr));
        }{
            src = ":";
            atest(!fstr_riterate(&src, "", chunk_ptr));
        }{
            src = ":";
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(!fstr_riterate(&src, ":", chunk_ptr));
        }{
            src = "a:b";
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate(&src, ":", chunk_ptr));
        }{
            src = "a::b";
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate(&src, ":", chunk_ptr));
        }{
            src = ":a::b";
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate(&src, ":", chunk_ptr));
        }{
            src = "foo!bar!foo!bar";
            atest(fstr_riterate(&src, "foo", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "!bar"));
            atest(fstr_riterate(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "!"));
            atest(fstr_riterate(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "foo!"));
            atest(!fstr_riterate(&src, "bar", chunk_ptr));
        }
    }
    // Test fstr_riterate_trim.
    for (size_t pass = 0; pass < 2; pass++) TEST_MEM_LEAK {
        fstr_t src, chunk;
        fstr_t* chunk_ptr = (pass == 0? &chunk: 0);
        {
            src = "";
            atest(!fstr_riterate_trim(&src, "", chunk_ptr));
        }{
            src = "";
            atest(!fstr_riterate_trim(&src, "\t", chunk_ptr));
        }{
            src = " :\t";
            atest(!fstr_riterate_trim(&src, "", chunk_ptr));
        }{
            src = " :\t";
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(!fstr_riterate_trim(&src, ":", chunk_ptr));
        }{
            src = " a \t\nb \t c\t d ";
            atest(fstr_riterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "d"));
            atest(fstr_riterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "c"));
            atest(fstr_riterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate_trim(&src, " ", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate_trim(&src, " ", chunk_ptr));
        }{
            src = "\t \na\n\t :  \n :b \t ";
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate_trim(&src, ":", chunk_ptr));
        }{
            src = ": \n   \na\t\n  :  \n :\nb\n";
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "b"));
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate_trim(&src, ":", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "a"));
            atest(!fstr_riterate_trim(&src, ":", chunk_ptr));
        }{
            src = "  foo bar\tfoo  \n bar\n\n";
            atest(fstr_riterate_trim(&src, "bar", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate_trim(&src, "foo", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, ""));
            atest(fstr_riterate_trim(&src, "foo", chunk_ptr));
            atest(pass == 1 || fstr_equal(chunk, "bar"));
            atest(!fstr_riterate_trim(&src, "foo", chunk_ptr));
        }
    }
    // Test fstr trim.
    atest(fstr_equal(fstr_trim("foo"), "foo"));
    atest(fstr_equal(fstr_trim("foo bar"), "foo bar"));
    atest(fstr_equal(fstr_trim(" foo \tbar\t\t\t"), "foo \tbar"));
    atest(fstr_equal(fstr_trim("\0 foo\0 \tbar\t\t\0"), "foo\0 \tbar"));
    atest(fstr_equal(fstr_trim("foo baz     "), "foo baz"));
    atest(fstr_equal(fstr_trim("      foo     baz\t\tbiz  \t"), "foo     baz\t\tbiz"));
    atest(fstr_equal(fstr_trim(" \0   \t\t \t"), ""));
    // Test fstr fstr_order_next/fstr_order_prev.
    sub_heap {
        atest(fstr_equal(fss(fstr_order_next("")), "\x00"));
        atest(fstr_equal(fss(fstr_order_next("\x00")), "\x01"));
        atest(fstr_equal(fss(fstr_order_next("a")), "b"));
        atest(fstr_equal(fss(fstr_order_next("\xff")), "\x00\x00"));
        atest(fstr_equal(fss(fstr_order_next("\x00\x00")), "\x00\x01"));
        atest(fstr_equal(fss(fstr_order_next("\x00\x7f")), "\x00\x80"));
        atest(fstr_equal(fss(fstr_order_next("\x00\xff")), "\x01\x00"));
        atest(fstr_equal(fss(fstr_order_next("\x01\x20")), "\x01\x21"));
        atest(fstr_equal(fss(fstr_order_next("\x01\xff")), "\x02\x00"));
        atest(fstr_equal(fss(fstr_order_next("\xff\xff")), "\x00\x00\x00"));
        atest(fstr_equal(fss(fstr_order_prev("\x00\x00\x00")), "\xff\xff"));
        atest(fstr_equal(fss(fstr_order_prev("\x02\x00")), "\x01\xff"));
        atest(fstr_equal(fss(fstr_order_prev("\x01\x21")), "\x01\x20"));
        atest(fstr_equal(fss(fstr_order_prev("\x01\x00")), "\x00\xff"));
        atest(fstr_equal(fss(fstr_order_prev("\x00\x80")), "\x00\x7f"));
        atest(fstr_equal(fss(fstr_order_prev("\x00\x01")), "\x00\x00"));
        atest(fstr_equal(fss(fstr_order_prev("\x00\x00")), "\xff"));
        atest(fstr_equal(fss(fstr_order_prev("b")), "a"));
        atest(fstr_equal(fss(fstr_order_prev("\x01")), "\x00"));
        atest(fstr_equal(fss(fstr_order_prev("\x00")), ""));
        try {
            fstr_order_prev("");
            atest(false);
        } catch (exception_arg, e) {}
    }
    // Test reversion of strings
    sub_heap {
        // in place tests
        sub_heap {
            fstr_t ip = fss(fstr_cpy("hello, world"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, "dlrow ,olleh"));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy(""));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, ""));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy("a"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, "a"));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy("abc"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, "cba"));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy("\n\tasdf asdf\n"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, "\nfdsa fdsa\t\n"));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy(",.;"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, ";.,"));
        }
        sub_heap {
            fstr_t ip = fss(fstr_cpy("the worst the worst the worst the worst the worst the worst the worst the worst the worst of you"));
            fstr_reverse_buffer(ip);
            atest(fstr_equal(ip, "uoy fo tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht"));
        }{
            // test reversal in newly created strings
            fstr_t str = "hello, world";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "dlrow ,olleh"));
        }{
            fstr_t str = "";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), ""));
        }{
            fstr_t str = "a";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "a"));
        }{
            fstr_t str = "abc";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "cba"));
        }{
            fstr_t str = "\n\tasdf asdf\n";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "\nfdsa fdsa\t\n"));
        }{
            fstr_t str = ",.;";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), ";.,"));
        }{
            fstr_t str = "hello, world";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "dlrow ,olleh"));
        }{
            fstr_t str = "";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), ""));
        }{
            fstr_t str = "a";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "a"));
        }{
            fstr_t str = "abc";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "cba"));
        }{
            fstr_t str = "\n\tasdf asdf\n";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "\nfdsa fdsa\t\n"));
        }{
            fstr_t str = ",.;";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), ";.,"));
        }{
            fstr_t str = "the worst the worst the worst the worst the worst the worst the worst the worst the worst of you";
            fstr_mem_t* rev = fstr_reverse(str);
            atest(fstr_equal(fss(rev), "uoy fo tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht tsrow eht"));
        }
    }
    // Test hexencode/hexdecode.
    sub_heap {
        atest(fstr_equal(fss(fstr_hexencode("")), ""));
        atest(fstr_equal(fss(fstr_hexencode("\xaa\xbb\xcc")), "aabbcc"));
        atest(fstr_equal(fss(fstr_hexencode("\x09\x3b\xf0\x5a\x20\x24\xd8\xa6\x11\xb5\x5a\x58\x3b\x7f\x32\x19\x74\x45\x31\x10")), "093bf05a2024d8a611b55a583b7f321974453110"));
        atest(fstr_equal(fss(fstr_hexdecode("")), ""));
        atest(fstr_equal(fss(fstr_hexdecode("aabbcc")), "\xaa\xbb\xcc"));
        atest(fstr_equal(fss(fstr_hexdecode("093bf05a2024d8a611b55a583b7f321974453110")), "\x09\x3b\xf0\x5a\x20\x24\xd8\xa6\x11\xb5\x5a\x58\x3b\x7f\x32\x19\x74\x45\x31\x10"));
        atest(fstr_equal(fss(fstr_hexdecode("\x00 ???093/bf0..5a2024d8a6&11b55a583b7f321974\x00 45311-=0")), "\x09\x3b\xf0\x5a\x20\x24\xd8\xa6\x11\xb5\x5a\x58\x3b\x7f\x32\x19\x74\x45\x31\x10"));
    }
    // Test fstr_hex_from_ptr.
    sub_heap {
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0x0)), "0x0000000000000000"));
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0x1)), "0x0000000000000001"));
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0x11)), "0x0000000000000011"));
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0x1000000000000001)), "0x1000000000000001"));
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0xfffffffffffffffe)), "0xfffffffffffffffe"));
        atest(fstr_equal(fss(fstr_hex_from_ptr((void*) 0xffffffffffffffff)), "0xffffffffffffffff"));
    }
    // Test sha1.
    sub_heap {
        atest(fstr_equal(flstr_to_fstr(fstr_sha1(""), 20), "\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09"));
        atest(fstr_equal(flstr_to_fstr(fstr_sha1("test"), 20), "\xa9\x4a\x8f\xe5\xcc\xb1\x9b\xa6\x1c\x4c\x08\x73\xd3\x91\xe9\x87\x98\x2f\xbb\xd3"));
        atest(fstr_equal(flstr_to_fstr(fstr_sha1(test_hash_str), 20), "\x09\x3b\xf0\x5a\x20\x24\xd8\xa6\x11\xb5\x5a\x58\x3b\x7f\x32\x19\x74\x45\x31\x10"));
    }
    // Test sha256.
    sub_heap {
        atest(fstr_equal(flstr_to_fstr(fstr_sha256(""), 32), "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55"));
        atest(fstr_equal(flstr_to_fstr(fstr_sha256("test"), 32), "\x9f\x86\xd0\x81\x88\x4c\x7d\x65\x9a\x2f\xea\xa0\xc5\x5a\xd0\x15\xa3\xbf\x4f\x1b\x2b\x0b\x82\x2c\xd1\x5d\x6c\x15\xb0\xf0\x0a\x08"));
        atest(fstr_equal(flstr_to_fstr(fstr_sha256(test_hash_str), 32), "\x06\x19\x10\x5d\x38\x86\x35\x5f\x0b\xb4\x49\x33\x35\x9d\x48\xb9\xab\x40\xd5\x1b\xc6\x54\x95\x2e\xd3\xb5\xa1\x6c\xb2\xe0\xed\x93"));
    }
    // Test md5.
    sub_heap {
        atest(fstr_equal(flstr_to_fstr(fstr_md5(""), 16), "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e"));
        atest(fstr_equal(flstr_to_fstr(fstr_md5("test"), 16), "\x09\x8f\x6b\xcd\x46\x21\xd3\x73\xca\xde\x4e\x83\x26\x27\xb4\xf6"));
        atest(fstr_equal(flstr_to_fstr(fstr_md5(test_hash_str), 16), "\xd0\xc3\x2c\xdd\xcf\x05\x4c\x4a\x15\xed\xe5\x21\x42\xb0\x5a\x24"));
    }
    // Test base64.
    sub_heap {
        atest(fstr_equal(fss(fstr_base64_encode("")), ""));
        atest(fstr_equal(fss(fstr_base64_encode("a")), "YQ=="));
        atest(fstr_equal(fss(fstr_base64_encode("ab")), "YWI="));
        atest(fstr_equal(fss(fstr_base64_encode("abc")), "YWJj"));
        atest(fstr_equal(fss(fstr_base64_encode("abcd")), "YWJjZA=="));
        atest(fstr_equal(fss(fstr_base64_encode("qwertyuiopasdfghjklzxcvbnm\x00\x00")), "cXdlcnR5dWlvcGFzZGZnaGprbHp4Y3Zibm0AAA=="));
    }
    sub_heap {
        atest(fstr_equal(fss(fstr_base64_decode("")), ""));
        atest(fstr_equal(fss(fstr_base64_decode("YQ==")), "a"));
        atest(fstr_equal(fss(fstr_base64_decode("YWI=")), "ab"));
        atest(fstr_equal(fss(fstr_base64_decode("YWJj")), "abc"));
        atest(fstr_equal(fss(fstr_base64_decode("YWJjZA==")), "abcd"));
        atest(fstr_equal(fss(fstr_base64_decode("cXdlcnR5dWlvcGFzZGZnaGprbHp4Y3Zibm0AAA==")), "qwertyuiopasdfghjklzxcvbnm\x00\x00"));
    }
    sub_heap {
        atest(fstr_equal(fss(fstr_base64_decode("\x00")), ""));
        atest(fstr_equal(fss(fstr_base64_decode("YQ=\x00=")), "a"));
        atest(fstr_equal(fss(fstr_base64_decode("YW%%%%%%I=")), "ab"));
        atest(fstr_equal(fss(fstr_base64_decode("YWI")), "ab"));
        atest(fstr_equal(fss(fstr_base64_decode("\x00\x00\x00\x00YW===Jj===")), "abc"));
        atest(fstr_equal(fss(fstr_base64_decode("YWJjZA")), "abcd"));
        atest(fstr_equal(fss(fstr_base64_decode("    cXd\r\nlcnR5dWlvcGFzZGZ   naGprbHp4 Y 3 Z ibm0AA\r\n    \r\n   \nA")), "qwertyuiopasdfghjklzxcvbnm\x00\x00"));
    }
    sub_heap {
        fstr_t buf = fss(fstr_alloc(0x100));
        fstr_t buf_tail;
        fstr_cpy_over(buf, "cXdlcnR5dWlvcGFzZGZnaGprbHp4Y3Zibm0A", &buf_tail, 0);
        for (size_t i = 0x7f;; i++) {
            if (i == 0x100) {
                buf_tail.str[i - 0x7f] = 'A';
                i++;
                buf_tail.str[i - 0x7f] = 'A';
                i++;
                buf.len = (&buf_tail.str[i - 0x7f] - buf.str);
                break;
            }
            buf_tail.str[i - 0x7f] = i;
        }
        fstr_t decoded_buf = fss(fstr_base64_decode(buf));
        atest(fstr_equal("qwertyuiopasdfghjklzxcvbnm\x00\x00", decoded_buf));
    }
    for (size_t extra = 0x100 - 8; extra <= 0x100; extra++) {
        sub_heap {
            fstr_t in = fss(fstr_alloc(extra));
            for (size_t i = 0; i < extra; i++)
                in.str[i] = i;
            fstr_t out = fss(fstr_base64_decode(fss(fstr_base64_encode(in))));
            atest(in.len == out.len);
            for (size_t i = 0; i < extra; i++)
                atest(out.str[i] == i);
        }
    }
    // Test base32.
#define TEST_BASE32(from, to) \
    sub_heap { \
        fstr_t enc = fss(fstr_base32_encode(from)); \
        atest(fstr_equal(enc, to)); \
        fstr_t dec = fss(fstr_base32_decode(to)); \
        atest(fstr_equal(dec, from)); \
    }
    TEST_BASE32("", "");
    TEST_BASE32("a", "ME======");
    TEST_BASE32("ab", "MFRA====");
    TEST_BASE32("abc", "MFRGG===");
    TEST_BASE32("abcd", "MFRGGZA=");
    TEST_BASE32("abcde", "MFRGGZDF");
    TEST_BASE32("abcdef", "MFRGGZDFMY======");
    TEST_BASE32("test", "ORSXG5A=");
    TEST_BASE32("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris placerat est quis magna vulputate, nec pharetra odio ullamcorper.",
        "JRXXEZLNEBUXA43VNUQGI33MN5ZCA43JOQQGC3LFOQWCAY3PNZZWKY3UMV2HK4RAMFSGS4DJONRWS3THEBSWY2LUFYQE2YLVOJUXGIDQNRQWGZLSMF2CAZLTOQQHC5LJOMQG2YLHNZQSA5TVNRYHK5DBORSSYIDOMVRSA4DIMFZGK5DSMEQG6ZDJN4QHK3DMMFWWG33SOBSXELQ=");
    TEST_BASE32(
        "The Base 32 encoding is designed to represent arbitrary sequences of octets in a form that needs to be case insensitive but that need not be human readable. A 33-character subset of US-ASCII is used, enabling 5 bits to be represented per printable character. (The extra 33rd character, \"=\", is used to signify a special processing function.)",
        "KRUGKICCMFZWKIBTGIQGK3TDN5SGS3THEBUXGIDEMVZWSZ3OMVSCA5DPEBZGK4DSMVZWK3TUEBQXEYTJORZGC4TZEBZWK4LVMVXGGZLTEBXWMIDPMN2GK5DTEBUW4IDBEBTG64TNEB2GQYLUEBXGKZLEOMQHI3ZAMJSSAY3BONSSA2LOONSW443JORUXMZJAMJ2XIIDUNBQXIIDOMVSWIIDON52CAYTFEBUHK3LBNYQHEZLBMRQWE3DFFYQECIBTGMWWG2DBOJQWG5DFOIQHG5LCONSXIIDPMYQFKUZNIFJUGSKJEBUXGIDVONSWILBAMVXGCYTMNFXGOIBVEBRGS5DTEB2G6IDCMUQHEZLQOJSXGZLOORSWIIDQMVZCA4DSNFXHIYLCNRSSAY3IMFZGCY3UMVZC4IBIKRUGKIDFPB2HEYJAGMZXEZBAMNUGC4TBMN2GK4RMEARD2IRMEBUXGIDVONSWIIDUN4QHG2LHNZUWM6JAMEQHG4DFMNUWC3BAOBZG6Y3FONZWS3THEBTHK3TDORUW63ROFE======");
#undef TEST_BASE32
    // Test circular fifo.
    sub_heap {
        fstr_cfifo_t cfifo = fstr_cfifo_init(fss(fstr_alloc(1)));
        atest(fstr_cfifo_len_data(&cfifo) == 0);
        atest(fstr_cfifo_len_space(&cfifo) == 1);
        fstr_t rbuf = fss(fstr_alloc(1));
        atest(fstr_cfifo_read(&cfifo, rbuf, false).len == 0);
        atest(fstr_cfifo_read(&cfifo, rbuf, true).len == 0);
        atest(fstr_cfifo_write(&cfifo, "!", false).len == 0);
        atest(fstr_equal(fstr_cfifo_write(&cfifo, "@", false), "@"));
        atest(fstr_cfifo_len_data(&cfifo) == 1);
        atest(fstr_cfifo_len_space(&cfifo) == 0);
        atest(fstr_equal(fstr_cfifo_read(&cfifo, rbuf, true), "!"));
        atest(fstr_cfifo_len_data(&cfifo) == 1);
        atest(fstr_cfifo_len_space(&cfifo) == 0);
        atest(fstr_equal(fstr_cfifo_read(&cfifo, rbuf, false), "!"));
        atest(fstr_cfifo_len_data(&cfifo) == 0);
        atest(fstr_cfifo_len_space(&cfifo) == 1);
        atest(fstr_equal(fstr_cfifo_read(&cfifo, rbuf, false), ""));
    }
    sub_heap {
        for (size_t mlen = 1; mlen < 5; mlen++) {
            fstr_t rwbuf = fss(fstr_alloc(mlen));
            fstr_cfifo_t cfifo = fstr_cfifo_init(fss(fstr_alloc(mlen)));
            for (size_t i = 0; i < mlen; i++)
            for (size_t j = 0; j < mlen; j++)
            for (size_t k = 0; k < 3; k++)
            for (size_t m = 0; m < mlen; m++) {
                for (size_t n = 0; n < mlen; n++)
                    cfifo.buffer.str[n] = 0;
                cfifo.data_offs = i;
                cfifo.data_len = j;
                atest(fstr_cfifo_len_data(&cfifo) == j);
                atest(fstr_cfifo_len_space(&cfifo) == (mlen - j));
                if (k == 0) {
                    // Try read.
                    for (size_t n = 0; n < cfifo.data_len; n++)
                        cfifo.buffer.str[(cfifo.data_offs + n) % mlen] = (1 + n);
                    rwbuf.len = m;
                    fstr_t rp = fstr_cfifo_read(&cfifo, rwbuf, true);
                    atest(fstr_cfifo_len_data(&cfifo) == j);
                    atest(fstr_cfifo_len_space(&cfifo) == (mlen - j));
                    fstr_t r_data = fstr_cfifo_read(&cfifo, rwbuf, false);
                    atest(fstr_equal(r_data, rp));
                    for (size_t n = 0; n < cfifo.data_len; n++)
                        atest(r_data.str[n] == (1 + n));
                    atest(fstr_cfifo_len_data(&cfifo) == j - r_data.len);
                    atest(fstr_cfifo_len_space(&cfifo) == (mlen - j + r_data.len));
                    atest(cfifo.data_offs == ((i + r_data.len) % mlen));
                    atest(cfifo.data_len == j - r_data.len);
                } else if (k == 1) {
                    // Try write.
                    rwbuf.len = m;
                    for (size_t n = 0; n < m; n++)
                        rwbuf.str[n] = (1 + n);
                    fstr_t w_tail = fstr_cfifo_write(&cfifo, rwbuf, false);
                    fstr_t w_head = fstr_detail(rwbuf, w_tail);
                    atest(w_head.len == MIN(mlen - j, rwbuf.len));
                    for (size_t n = 0; n < w_tail.len; n++)
                        atest(w_tail.str[n] == (rwbuf.len - w_tail.len + n + 1));
                    for (size_t n = 0; n < w_head.len; n++)
                        atest(cfifo.buffer.str[(i + j + n) % mlen] = (1 + n));
                    atest(fstr_cfifo_len_data(&cfifo) == j + w_head.len);
                    atest(fstr_cfifo_len_space(&cfifo) == (mlen - j - w_head.len));
                } else if (k == 2) {
                    // Try replace.
                    rwbuf.len = m;
                    for (size_t n = 0; n < m; n++)
                        rwbuf.str[n] = (1 + n);
                    fstr_t w_tail = fstr_cfifo_write(&cfifo, rwbuf, true);
                    atest(w_tail.len == 0);
                    for (size_t n = 0; n < rwbuf.len; n++)
                        atest(cfifo.buffer.str[(i + j + n) % mlen] = (1 + n));
                    atest(fstr_cfifo_len_data(&cfifo) == MIN(mlen, j + rwbuf.len));
                    atest(fstr_cfifo_len_space(&cfifo) == MAX(0, (ssize_t) (mlen - j - rwbuf.len)));
                }
            }
        }
    }
    // Test ace encode/decode.
    sub_heap {
        // The hash test string should be the exact same in ace.
        fstr_t ace0 = fss(fstr_ace_encode(test_hash_str));
        atest(fstr_equal(ace0, test_hash_str));
        fstr_t dec0 = fss(fstr_ace_decode(ace0));
        atest(fstr_equal(dec0, test_hash_str));
        // Test encoding all characters.
        fstr_t all = fss(fstr_alloc(0x100));
        for (size_t i = 0; i < 0x100; i++) {
            all.str[i] = i;
        }
        fstr_t ace1 = fss(fstr_ace_encode(all));
        fstr_t dec1 = fss(fstr_ace_decode(ace1));
        atest(fstr_equal(dec1, all));
        // Test cornercase.
        fstr_t test = "\0\\\0\\\0\\\0\\\0\\";
        fstr_t ace3 = fss(fstr_ace_encode(test));
        atest(fstr_equal(ace3, "\\x00\\\\\\x00\\\\\\x00\\\\\\x00\\\\\\x00\\\\"));
        fstr_t dec3 = fss(fstr_ace_decode(ace1));
        // Test cornercase all null.
        fstr_fill(all, 0);
        fstr_t ace2 = fss(fstr_ace_encode(all));
        atest(ace2.len == all.len * 4);
        fstr_t dec2 = fss(fstr_ace_decode(ace2));
        atest(fstr_equal(dec2, all));
    }
    // Test fstr_path_base.
    TEST_MEM_LEAK {
        {
            fstr_t base = fstr_path_base("");
            atest(fstr_equal(base, ""));
        }{
            fstr_t base = fstr_path_base("/");
            atest(fstr_equal(base, ""));
        }{
            fstr_t base = fstr_path_base("a");
            atest(fstr_equal(base, "a"));
        }{
            fstr_t base = fstr_path_base("/a");
            atest(fstr_equal(base, "a"));
        }{
            fstr_t base = fstr_path_base("//a");
            atest(fstr_equal(base, "a"));
        }{
            fstr_t base = fstr_path_base("/a/b");
            atest(fstr_equal(base, "b"));
        }{
            fstr_t base = fstr_path_base("/a/b/c");
            atest(fstr_equal(base, "c"));
        }{
            fstr_t base = fstr_path_base("a/b/c/dd");
            atest(fstr_equal(base, "dd"));
        }{
            fstr_t base = fstr_path_base("a/b/c/ee/");
            atest(fstr_equal(base, "ee"));
        }{
            fstr_t base = fstr_path_base("a/b/c/ff//");
            atest(fstr_equal(base, "ff"));
        }{
            fstr_t base = fstr_path_base("//a////b//////c//gg///////");
            atest(fstr_equal(base, "gg"));
        }
    }

    //Test UTF-8 clean up.
    // Escape character for invalid binary data.
    #define esc "\xEF\xBF\xBD"
    // Invalid UTF-8 after 1-4 bytes parsed.
    #define inv1 "\x80"
    #define inv2 "\xC2\xC0"
    #define inv3 "\xE1\x80\xC0"
    #define inv4 "\xF1\x80\x80\xC0"
    // Modified UTF-8 null encoding, which is invalid.
    #define inv_null "\xC0\x80"
    atest(fstr_equal(esc "a", fss(fstr_clean_utf8(inv1"a"))));
    atest(fstr_equal("a" esc "a", fss(fstr_clean_utf8("a" inv1 "a"))));
    atest(fstr_equal(esc, fss(fstr_clean_utf8(inv1))));
    atest(fstr_equal(esc esc, fss(fstr_clean_utf8(inv1 inv1))));
    atest(fstr_equal(esc esc, fss(fstr_clean_utf8(inv2))));
    atest(fstr_equal(esc esc "a" esc, fss(fstr_clean_utf8(inv2 "a" inv1))));
    atest(fstr_equal(esc esc esc, fss(fstr_clean_utf8(inv3))));
    atest(fstr_equal(esc esc esc esc, fss(fstr_clean_utf8(inv4))));
    atest(fstr_equal(esc esc esc esc esc, fss(fstr_clean_utf8(inv1 inv4))));
    atest(fstr_equal(esc esc, fss(fstr_clean_utf8(inv_null))));
    #undef esc
    #undef inv1
    #undef inv2
    #undef inv3
    #undef inv4
    #undef inv_null
}
