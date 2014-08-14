/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef ARITHMETIC_H
#define	ARITHMETIC_H

bool arth_safe_add_int128(int128_t a, int128_t b, int128_t* out);
bool arth_safe_sub_int128(int128_t a, int128_t b, int128_t* out);
bool arth_safe_mul_int128(int128_t a, int128_t b, int128_t* out);

bool arth_safe_add_uint128(uint128_t a, uint128_t b, uint128_t* out);
bool arth_safe_sub_uint128(uint128_t a, uint128_t b, uint128_t* out);
bool arth_safe_mul_uint128(uint128_t a, uint128_t b, uint128_t* out);

#endif	/* ARITHMETIC_H */
