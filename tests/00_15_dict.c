/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

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
                    dict_foreach_delete_current(word_book, fstr_t);
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
            atest(dict_count(word_book, fstr_t) == 2);
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "hello");
            atest(word == 0);
        }{
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(fstr_equal(*word, "ses"));
        }{
            atest(dict_delete(word_book, fstr_t, "bye"));
            fstr_t* word = dict_read(word_book, fstr_t, "bye");
            atest(word == 0);
            atest(dict_count(word_book, fstr_t) == 1);
        }
    }
    sub_heap {
        dict(int32_t*)* test_dict = new_dict(int32_t*);
        for (size_t i = 0; i < 16; i++) {
            int32_t* j = new(int32_t);
            *j = i;
            bool insert_ok = dict_push_end(test_dict, int32_t*, FSTR_PACK(i), j);
            atest(insert_ok);
        }{
            size_t i = 0;
            dict_foreach(test_dict, int32_t*, k, j) {
                atest(*j == i);
                int32_t* j2 = new(int32_t);
                *j2 = (i * 2);
                dict_foreach_replace_current(int32_t*, j2);
                i++;
            }
        }{
            size_t i = 0;
            dict_foreach(test_dict, int32_t*, k, j) {
                atest(*j == (i * 2));
                i++;
            }
        }
    }
    sub_heap {
        dict(int32_t)* test_dict = new_dict(int32_t, {"all", 1});
        dict_replace(test_dict, int32_t, "all", 2);
        dict_replace(test_dict, int32_t, "foo", 3);
        dict_replace(test_dict, int32_t, "bar", 4);
        dict_replace(test_dict, int32_t, "baz", 5);
        atest(dict_count(test_dict, int32_t) == 4);
        int32_t i = 0;
        dict_foreach(test_dict, int32_t, key, value) {
            switch (i) {
            case 0:
                atest(fstr_equal(key, "all"));
                atest(value == 2);
                break;
            case 1:
                atest(fstr_equal(key, "foo"));
                atest(value == 3);
                break;
            case 2:
                atest(fstr_equal(key, "bar"));
                atest(value == 4);
                break;
            case 3:
                atest(fstr_equal(key, "baz"));
                atest(value == 5);
                break;
            default:
                atest(false);
            }
            i++;
        }
        atest(i == 4);
    }
}
