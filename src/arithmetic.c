/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

bool arth_safe_add_int128(int128_t a, int128_t b, int128_t* out) {
    *out = 0;
    if (((b > 0) && (a > (INT128_MAX - b))) || ((b < 0) && (a < (INT128_MIN - b)))) {
        return false;
    }
    *out = a + b;
    return true;
}

bool arth_safe_add_uint128(uint128_t a, uint128_t b, uint128_t* out) {
    *out = 0;
    if (b > 0 && (a > (UINT128_MAX - b))) {
        return false;
    }
    *out = a + b;
    return true;
}

bool arth_safe_sub_int128(int128_t a, int128_t b, int128_t* out) {
    *out = 0;
    if ((b > 0 && a < INT128_MIN + b) || (b < 0 && a > INT128_MAX + b)) {
        return false;
    }
    *out = a - b;
    return true;
}

bool arth_safe_sub_uint128(uint128_t a, uint128_t b, uint128_t* out) {
    *out = 0;
    if (b > a) {
        return false;
    }
    *out = a - b;
    return true;
}

bool arth_safe_mul_int128(int128_t a, int128_t b, int128_t* out) {
    *out = 0;
    if (a > 0) {
        if (b > 0) {
            if (a > (INT128_MAX / b)) {
                return false;
            }
        } else {
            if (b < (INT128_MIN / a)) {
                return false;
            }
        }
    } else {
        if (b > 0) {
            if (a < (INT128_MIN / b)) {
                return false;
            }
        } else {
            if ((a != 0) && (b < (INT128_MAX / a))) {
                return false;
            }
        }
    }
    *out = a * b;
    return true;
}

bool arth_safe_mul_uint128(uint128_t a, uint128_t b, uint128_t* out) {
    *out = 0;
    if (b == 0)
        return false;
    if (a > (UINT128_MAX / b)) {
        return false;
    }
    *out = a * b;
    return true;
}
