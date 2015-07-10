/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "test.h"

#pragma librcd

void rcd_self_test_dict() {
    for (size_t i = 0; i < 2; i++) sub_heap {
        dict(fstr_t)* word_book;
        if (i == 0) {
            word_book = new_dict(fstr_t);
            atest(dict_count(word_book, fstr_t) == 0);
            atest(dict_insert(word_book, fstr_t, "hello", "hej"));
            dict_replace(word_book, fstr_t, "bye", "vi ses");
            atest(!dict_insert(word_book, fstr_t, "bye", "conflict"));
            atest(dict_count(word_book, fstr_t) == 2);
            atest(dict_insert(word_book, fstr_t, "stuff", "saker"));
            atest(dict_count(word_book, fstr_t) == 3);
        } else {
            word_book = new_dict(fstr_t,
                {"hello", "hej"},
                {"bye", "vi ses"},
                {"stuff", "saker"},
            );
        }
        {
            fstr_t* word = dict_read(word_book, fstr_t, "hello");
            atest(fstr_equal(*word, "hej"));
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(fstr_equal(*word, "vi ses"));
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "bai");
            atest(word == 0);
        }{
            dict_replace(word_book, fstr_t, "bye", "ses");
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(fstr_equal(*word, "ses"));
        }{
            size_t i = 0;
            // Test that the dict is ordered correctly for the two first elements and that breaking works.
            dict_foreach(word_book, fstr_t, english_word, swedish_word) {
                switch (i) {
                case 0:
                    atest(fstr_equal(english_word, "hello"));
                    atest(fstr_equal(swedish_word, "hej"));
                    break;
                case 1:
                    atest(fstr_equal(english_word, "bye"));
                    atest(fstr_equal(swedish_word, "ses"));
                    break;
                default:
                    atest(false);
                }
                if (i == 1)
                    break;
                i++;
            }
            atest(i == 1);
        }{
            size_t i = 0;
            dict_foreach(word_book, fstr_t, english_word, swedish_word) {
                switch (i) {
                case 0:
                    atest(fstr_equal(english_word, "hello"));
                    atest(fstr_equal(swedish_word, "hej"));
                    break;
                case 1:
                    atest(fstr_equal(english_word, "bye"));
                    atest(fstr_equal(swedish_word, "ses"));
                    break;
                case 2:
                    atest(fstr_equal(english_word, "stuff"));
                    atest(fstr_equal(swedish_word, "saker"));
                    break;
                default:
                    atest(false);
                }
                i++;
            }
            atest(i == 3);
            atest(dict_count(word_book, fstr_t) == 3);
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "hello");
            atest(fstr_equal(*word, "hej"));
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(fstr_equal(*word, "ses"));
        }{
            atest(dict_delete(word_book, fstr_t, "bye"));
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(word == 0);
            atest(dict_count(word_book, fstr_t) == 2);
        }
    }
    sub_heap {
        dict(int32_t*)* test_dict = new_dict(int32_t*);
        for (size_t i = 0; i < 16; i++) {
            int32_t* j = new(int32_t);
            *j = i;
            bool insert_ok = dict_push_end(test_dict, int32_t*, FSTR_PACK(i), j);
            atest(insert_ok);
        }
    }
    // Test replace linking.
    for (int32_t i = 0; i < 6; i++) {
        for (int32_t j = 0; j <= i; j++) TEST_MEM_LEAK {
            DBGFN("[", i, "/", j, "] starting next test ***");
            dict(int32_t)* d = new_dict(int32_t);
            sub_heap {
                // Build the initial dict.
                for (size_t k = 0; k <= i; k++) {
                    atest(dict_insert(d, int32_t, STR(k), k));
                    DBGFN("[", i, "/", j, "] insert [", k, "] => [", k, "]");
                }
                // Make the test replace.
                atest(dict_replace(d, int32_t, STR(j), j + 1000));
                DBGFN("[", i, "/", j, "] replace [", j, "] => [", j + 1000, "]");
            }
            for (int32_t i_e = 0; i_e <= 6; i_e++) {
                // Manipulate sequence further.
                sub_heap {
                    if (i_e > 3) {
                        dict_push_start(d, int32_t, STR(i + i_e), i + i_e);
                        DBGFN("[", i, "/", j, "] prepend [", i + i_e, "] => [", i + i_e, "]");
                    } else if (i_e > 0) {
                        dict_push_end(d, int32_t, STR(i + i_e), i + i_e);
                        DBGFN("[", i, "/", j, "] append [", i + i_e, "] => [", i + i_e, "]");
                    }
                }
                // Verify basic integrity.
                for (size_t k = 0; k <= i + i_e; k++) sub_heap {
                    int32_t* i = dict_read(d, int32_t, STR(k));
                    atest(*i == (k == j? k + 1000: k));
                }
                int32_t k = 0;
                int32_t n_pfx = MAX(i_e - 3, 0);
                dict_foreach(d, int32_t, key, value) sub_heap {
                    int32_t e_key, e_value;
                    if (n_pfx > 0) {
                        e_key = i + 3 + n_pfx;
                        e_value = e_key;
                        n_pfx--;
                    } else {
                        e_key = k;
                        e_value = (k == j? k + 1000: k);
                        k++;
                    }
                    atest(fstr_equal(key, STR(e_key)));
                    atest(value == e_value);
                }
                atest(n_pfx == 0);
                atest(k == (i + 1 + MIN(i_e, 3)));
                atest(dict_count(d, int32_t) == (i + 1 + i_e));
            }
            lwt_alloc_free(d);
        }
    }
}
