/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef	_SETJMP_H
#define	_SETJMP_H

/// See setjmp.s/longjump.s
typedef struct {
    uint64_t _[0x8];
} jmp_buf[1];

int setjmp(jmp_buf buf);

noret void longjmp(jmp_buf buf, int setjmp_r);

/// Librcd does not support a signal storing jmp buffer concept.
typedef jmp_buf sigjmp_buf;

static inline int sigsetjmp(sigjmp_buf env, int savemask) {
    return setjmp(env);
}

static inline void siglongjmp(sigjmp_buf env, int val) {
    longjmp(env, val);
}

#endif
