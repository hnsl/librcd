/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/// Declares a global dummy symbol that flags to the compiler that another symbol
/// is segmented stack aware and require no auto generated mega stack wrapper.
.macro SEG_STACK_SYMBOL symbol_name
.global \symbol_name
.type \symbol_name,@function
\symbol_name:
.global __seg_stack_sym__\()\symbol_name\()__
__seg_stack_sym__\()\symbol_name\()__:
.endm

/// Declares segmented stack function symbols and prologue.
/// Use this macro when writing a segmented stack aware function frame
/// in assembler.
/// The upside to being segmented stack aware is that the function is allowed
/// to call other segmented stack aware code and will not require an auto
/// generated wrapper.
/// The downside is that the function will corrupt memory unless it specifies
/// the correct frame_stack_size, the number of bytes on the stack the function
/// need to allocate, and the overhead of the prologue.
.macro SEG_STACK_FN_PRLG fn_name frame_stack_size
SEG_STACK_SYMBOL \fn_name
        lea -\frame_stack_size(%rsp), %r11
        cmp %fs:0x8, %r11
        ja 1f
        mov $\frame_stack_size, %r10
        xor %r11, %r11
        call __morestack
        ret
1:
.endm

.macro BREAKPT
int $3
.endm
