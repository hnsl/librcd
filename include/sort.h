/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef SORT_H
#define	SORT_H

void sort(void* base, size_t num, size_t size, int32_t (*cmp_func)(const void*, const void*), void (*swap_func)(void*, void*, size_t size));

#endif	/* SORT_H */
