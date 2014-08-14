#ifndef COMPILER_RT_H
#define COMPILER_RT_H

#include "rcd.h"
#include "musl.h"

/* ===-- int_lib.h - configuration header for compiler-rt  -----------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file is a configuration header for compiler-rt.
 * This file is not part of the interface of this library.
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef INT_LIB_H
#define INT_LIB_H

/* Assumption: Signed integral is 2's complement. */
/* Assumption: Right shift of signed negative is arithmetic shift. */
/* Assumption: Endianness is little or big (not mixed). */

/* ABI macro definitions */

#if __ARM_EABI__
# define ARM_EABI_FNALIAS(aeabi_name, name)         \
  void __aeabi_##aeabi_name() __attribute__((alias("__" #name)));
# define COMPILER_RT_ABI __attribute__((pcs("aapcs")))
#else
# define ARM_EABI_FNALIAS(aeabi_name, name)
# define COMPILER_RT_ABI
#endif

#endif /* INT_LIB_H */

/* ===-- int_endianness.h - configuration header for compiler-rt ------------===
 *
 *		       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file is a configuration header for compiler-rt.
 * This file is not part of the interface of this library.
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef INT_ENDIANNESS_H
#define INT_ENDIANNESS_H

/* librcd only supports x86_64 */
#define _YUGA_LITTLE_ENDIAN 1
#define _YUGA_BIG_ENDIAN    0

#endif /* INT_ENDIANNESS_H */

/* ===-- int_lib.h - configuration header for compiler-rt  -----------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file is not part of the interface of this library.
 *
 * This file defines various standard types, most importantly a number of unions
 * used to access parts of larger types.
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef INT_TYPES_H
#define INT_TYPES_H

typedef      int si_int;
typedef unsigned su_int;

typedef          long long di_int;
typedef unsigned long long du_int;

typedef union
{
    di_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        su_int low;
        si_int high;
#else
        si_int high;
        su_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} dwords;

typedef union
{
    du_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        su_int low;
        su_int high;
#else
        su_int high;
        su_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} udwords;

#if __x86_64

typedef int      ti_int __attribute__ ((mode (TI)));
typedef unsigned tu_int __attribute__ ((mode (TI)));

typedef union
{
    ti_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        du_int low;
        di_int high;
#else
        di_int high;
        du_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} twords;

typedef union
{
    tu_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        du_int low;
        du_int high;
#else
        du_int high;
        du_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} utwords;

static inline ti_int make_ti(di_int h, di_int l) {
    twords r;
    r.s.high = h;
    r.s.low = l;
    return r.all;
}

static inline tu_int make_tu(du_int h, du_int l) {
    utwords r;
    r.s.high = h;
    r.s.low = l;
    return r.all;
}

#endif /* __x86_64 */

typedef union
{
    su_int u;
    float f;
} float_bits;

typedef union
{
    udwords u;
    double  f;
} double_bits;

typedef struct
{
#if _YUGA_LITTLE_ENDIAN
    udwords low;
    udwords high;
#else
    udwords high;
    udwords low;
#endif /* _YUGA_LITTLE_ENDIAN */
} uqwords;

typedef union
{
    uqwords     u;
    long double f;
} long_double_bits;

#endif /* INT_TYPES_H */

/* ===-- int_util.h - internal utility functions ----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===-----------------------------------------------------------------------===
 *
 * This file is not part of the interface of this library.
 *
 * This file defines non-inline utilities which are available for use in the
 * library. The function definitions themselves are all contained in int_util.c
 * which will always be compiled into any compiler-rt library.
 *
 * ===-----------------------------------------------------------------------===
 */

#ifndef INT_UTIL_H
#define INT_UTIL_H

/** \brief Trigger a program abort (or panic for kernel code). */
#define compilerrt_abort() compilerrt_abort_impl(__FILE__, __LINE__, \
                                                 __FUNCTION__)

void compilerrt_abort_impl(const char *file, int line,
                           const char *function) __attribute__((noreturn));

#endif /* INT_UTIL_H */


/* ===-- int_math.h - internal math inlines ---------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===-----------------------------------------------------------------------===
 *
 * This file is not part of the interface of this library.
 *
 * This file defines substitutes for the libm functions used in some of the
 * compiler-rt implementations, defined in such a way that there is not a direct
 * dependency on libm or math.h. Instead, we use the compiler builtin versions
 * where available. This reduces our dependencies on the system SDK by foisting
 * the responsibility onto the compiler.
 *
 * ===-----------------------------------------------------------------------===
 */

#ifndef INT_MATH_H
#define INT_MATH_H

#ifndef __has_builtin
#  define  __has_builtin(x) 0
#endif

#define CRT_INFINITY __builtin_huge_valf()

#define crt_isinf(x) __builtin_isinf((x))
#define crt_isnan(x) __builtin_isnan((x))

/* Define crt_isfinite in terms of the builtin if available, otherwise provide
 * an alternate version in terms of our other functions. This supports some
 * versions of GCC which didn't have __builtin_isfinite.
 */
#if __has_builtin(__builtin_isfinite)
#  define crt_isfinite(x) __builtin_isfinite((x))
#else
#  define crt_isfinite(x) \
  __extension__(({ \
      __typeof((x)) x_ = (x); \
      !crt_isinf(x_) && !crt_isnan(x_); \
    }))
#endif

#define crt_copysign(x, y) __builtin_copysign((x), (y))
#define crt_copysignf(x, y) __builtin_copysignf((x), (y))
#define crt_copysignl(x, y) __builtin_copysignl((x), (y))

#define crt_fabs(x) __builtin_fabs((x))
#define crt_fabsf(x) __builtin_fabsf((x))
#define crt_fabsl(x) __builtin_fabsl((x))

#define crt_fmax(x, y) __builtin_fmax((x), (y))
#define crt_fmaxf(x, y) __builtin_fmaxf((x), (y))
#define crt_fmaxl(x, y) __builtin_fmaxl((x), (y))

#define crt_logb(x) __builtin_logb((x))
#define crt_logbf(x) __builtin_logbf((x))
#define crt_logbl(x) __builtin_logbl((x))

#define crt_scalbn(x, y) __builtin_scalbn((x), (y))
#define crt_scalbnf(x, y) __builtin_scalbnf((x), (y))
#define crt_scalbnl(x, y) __builtin_scalbnl((x), (y))

#endif /* INT_MATH_H */

#include "crt-fp-single.h"
#include "crt-fp.h"

#include "crt-fp-double.h"
#include "crt-fp.h"

#endif /* COMPILER_RT_H */
