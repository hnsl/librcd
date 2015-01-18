// Automatically generated from musl (http://www.musl-libc.org/) by the
// tools/musl-generator script. Do not edit.
//
// Copyright © 2005-2014 Rich Felker, et al.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "rcd.h"
#include "musl.h"

#pragma clang diagnostic ignored "-Wdangling-else"
#pragma clang diagnostic ignored "-Wshift-op-parentheses"
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wparentheses"

#define weak_alias(old, new) \
    extern __typeof(old) new __attribute__((weak, alias(#old)))

#define LFS64(...)
#define LFS64_2(...)
#define __pthread_self_def(...)

#define __mmap mmap

/// START musl code

/// >>> START src/complex/__cexp.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_exp.c */
/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/// >>> START src/internal/libm.h
/* origin: FreeBSD /usr/src/lib/msun/src/math_private.h */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#ifndef _LIBM_H
#undef _LIBM_H
#define _LIBM_H

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape {
    long double f;
    struct {
        uint64_t m;
        uint16_t se;
    } i;
};
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape {
    long double f;
    struct {
        uint64_t lo;
        uint32_t mid;
        uint16_t top;
        uint16_t se;
    } i;
    struct {
        uint64_t lo;
        uint64_t hi;
    } i2;
};
#else
#error Unsupported long double representation
#endif

#undef FORCE_EVAL
#define FORCE_EVAL(x) do {                        \
    if (sizeof(x) == sizeof(float)) {         \
        volatile float __x;               \
        __x = (x);                        \
    } else if (sizeof(x) == sizeof(double)) { \
        volatile double __x;              \
        __x = (x);                        \
    } else {                                  \
        volatile long double __x;         \
        __x = (x);                        \
    }                                         \
} while(0)

/* Get two 32 bit ints from a double.  */
#undef EXTRACT_WORDS
#define EXTRACT_WORDS(hi,lo,d)                    \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (hi) = __u.i >> 32;                             \
  (lo) = (uint32_t)__u.i;                         \
} while (0)

/* Get the more significant 32 bit int from a double.  */
#undef GET_HIGH_WORD
#define GET_HIGH_WORD(hi,d)                       \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (hi) = __u.i >> 32;                             \
} while (0)

/* Get the less significant 32 bit int from a double.  */
#undef GET_LOW_WORD
#define GET_LOW_WORD(lo,d)                        \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (lo) = (uint32_t)__u.i;                         \
} while (0)

/* Set a double from two 32 bit ints.  */
#undef INSERT_WORDS
#define INSERT_WORDS(d,hi,lo)                     \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.i = ((uint64_t)(hi)<<32) | (uint32_t)(lo);  \
  (d) = __u.f;                                    \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */
#undef SET_HIGH_WORD
#define SET_HIGH_WORD(d,hi)                       \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  __u.i &= 0xffffffff;                            \
  __u.i |= (uint64_t)(hi) << 32;                  \
  (d) = __u.f;                                    \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */
#undef SET_LOW_WORD
#define SET_LOW_WORD(d,lo)                        \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  __u.i &= 0xffffffff00000000ull;                 \
  __u.i |= (uint32_t)(lo);                        \
  (d) = __u.f;                                    \
} while (0)

/* Get a 32 bit int from a float.  */
#undef GET_FLOAT_WORD
#define GET_FLOAT_WORD(w,d)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.f = (d);                                    \
  (w) = __u.i;                                    \
} while (0)

/* Set a float from a 32 bit int.  */
#undef SET_FLOAT_WORD
#define SET_FLOAT_WORD(d,w)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.i = (w);                                    \
  (d) = __u.f;                                    \
} while (0)

/* fdlibm kernel functions */

int    __rem_pio2_large(double*,double*,int,int,int);

int    __rem_pio2(double,double*);
double __sin(double,double,int);
double __cos(double,double);
double __tan(double,double,int);
double __expo2(double);
double complex __ldexp_cexp(double complex,int);

int    __rem_pio2f(float,double*);
float  __sindf(double);
float  __cosdf(double);
float  __tandf(double,int);
float  __expo2f(float);
float complex __ldexp_cexpf(float complex,int);

int __rem_pio2l(long double, long double *);
long double __sinl(long double, long double, int);
long double __cosl(long double, long double);
long double __tanl(long double, long double, int);

/* polynomial evaluation */
long double __polevll(long double, const long double *, int);
long double __p1evll(long double, const long double *, int);

#endif

/// >>> CONTINUE src/complex/__cexp.c

static const uint32_t __cexp_c__k = 1799; /* constant for reduction */
static const double __cexp_c__kln2 = 1246.97177782734161156; /* __cexp_c__k * ln2 */

/*
 * Compute exp(x), scaled to avoid spurious overflow.  An exponent is
 * returned separately in 'expt'.
 *
 * Input:  ln(DBL_MAX) <= x < ln(2 * DBL_MAX / DBL_MIN_DENORM) ~= 1454.91
 * Output: 2**1023 <= y < 2**1024
 */
static double __frexp_exp(double x, int *expt) {
    double exp_x;
    uint32_t hx;

    /*
     * We use exp(x) = exp(x - __cexp_c__kln2) * 2**__cexp_c__k, carefully chosen to
     * minimize |exp(__cexp_c__kln2) - 2**__cexp_c__k|.  We also scale the exponent of
     * exp_x to MAX_EXP so that the result can be multiplied by
     * a __cexp_c__tiny number without losing accuracy due to denormalization.
     */
    exp_x = exp(x - __cexp_c__kln2);
    GET_HIGH_WORD(hx, exp_x);
    *expt = (hx >> 20) - (0x3ff + 1023) + __cexp_c__k;
    SET_HIGH_WORD(exp_x, (hx & 0xfffff) | ((0x3ff + 1023) << 20));
    return exp_x;
}

/*
 * __ldexp_cexp(x, expt) compute exp(x) * 2**expt.
 * It is intended for large arguments (real part >= ln(DBL_MAX))
 * where care is needed to avoid overflow.
 *
 * The present implementation is narrowly tailored for our hyperbolic and
 * exponential functions.  We assume expt is small (0 or -1), and the caller
 * has filtered out very large x, for which overflow would be inevitable.
 */
double complex __ldexp_cexp(double complex z, int expt) {
    double x, y, exp_x, scale1, scale2;
    int ex_expt, half_expt;

    x = creal(z);
    y = cimag(z);
    exp_x = __frexp_exp(x, &ex_expt);
    expt += ex_expt;

    /*
     * Arrange so that scale1 * scale2 == 2**expt.  We use this to
     * compensate for scalbn being horrendously slow.
     */
    half_expt = expt / 2;
    INSERT_WORDS(scale1, (0x3ff + half_expt) << 20, 0);
    half_expt = expt - half_expt;
    INSERT_WORDS(scale2, (0x3ff + half_expt) << 20, 0);

    return CMPLX(cos(y) * exp_x * scale1 * scale2, sin(y) * exp_x * scale1 * scale2);
}

/// >>> START src/complex/__cexpf.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_expf.c */
/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const uint32_t __cexpf_c__k = 235; /* constant for reduction */
static const float __cexpf_c__kln2 = 162.88958740F; /* __cexpf_c__k * ln2 */

/*
 * See __cexp.c for details.
 *
 * Input:  ln(FLT_MAX) <= x < ln(2 * FLT_MAX / FLT_MIN_DENORM) ~= 192.7
 * Output: 2**127 <= y < 2**128
 */
static float __frexp_expf(float x, int *expt) {
    float exp_x;
    uint32_t hx;

    exp_x = expf(x - __cexpf_c__kln2);
    GET_FLOAT_WORD(hx, exp_x);
    *expt = (hx >> 23) - (0x7f + 127) + __cexpf_c__k;
    SET_FLOAT_WORD(exp_x, (hx & 0x7fffff) | ((0x7f + 127) << 23));
    return exp_x;
}

float complex __ldexp_cexpf(float complex z, int expt) {
    float x, y, exp_x, scale1, scale2;
    int ex_expt, half_expt;

    x = crealf(z);
    y = cimagf(z);
    exp_x = __frexp_expf(x, &ex_expt);
    expt += ex_expt;

    half_expt = expt / 2;
    SET_FLOAT_WORD(scale1, (0x7f + half_expt) << 23);
    half_expt = expt - half_expt;
    SET_FLOAT_WORD(scale2, (0x7f + half_expt) << 23);

    return CMPLXF(cosf(y) * exp_x * scale1 * scale2,
      sinf(y) * exp_x * scale1 * scale2);
}

/// >>> START src/complex/cabs.c

double cabs(double complex z) {
    return hypot(creal(z), cimag(z));
}

/// >>> START src/complex/cabsf.c

float cabsf(float complex z) {
    return hypotf(crealf(z), cimagf(z));
}

/// >>> START src/complex/cabsl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double cabsl(long double complex z) {
    return cabs(z);
}
#else
long double cabsl(long double complex z) {
    return hypotl(creall(z), cimagl(z));
}
#endif

/// >>> START src/complex/cacos.c

// FIXME: Hull et al. "Implementing the complex arcsine and arccosine functions using exception handling" 1997

/* acos(z) = cacos_c__pi/2 - asin(z) */

double complex cacos(double complex z) {
    z = casin(z);
    return CMPLX(M_PI_2 - creal(z), -cimag(z));
}

/// >>> START src/complex/cacosf.c

// FIXME

float complex cacosf(float complex z) {
    z = casinf(z);
    return CMPLXF((float)M_PI_2 - crealf(z), -cimagf(z));
}

/// >>> START src/complex/cacosh.c

/* acosh(z) = i acos(z) */

double complex cacosh(double complex z) {
    z = cacos(z);
    return CMPLX(-cimag(z), creal(z));
}

/// >>> START src/complex/cacoshf.c

float complex cacoshf(float complex z) {
    z = cacosf(z);
    return CMPLXF(-cimagf(z), crealf(z));
}

/// >>> START src/complex/cacoshl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex cacoshl(long double complex z) {
    return cacosh(z);
}
#else
long double complex cacoshl(long double complex z) {
    z = cacosl(z);
    return CMPLXL(-cimagl(z), creall(z));
}
#endif

/// >>> START src/complex/cacosl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex cacosl(long double complex z) {
    return cacos(z);
}
#else
// FIXME
#undef PI_2
#define PI_2 1.57079632679489661923132169163975144L
long double complex cacosl(long double complex z) {
    z = casinl(z);
    return CMPLXL(PI_2 - creall(z), -cimagl(z));
}
#endif

/// >>> START src/complex/carg.c

double carg(double complex z) {
    return atan2(cimag(z), creal(z));
}

/// >>> START src/complex/cargf.c

float cargf(float complex z) {
    return atan2f(cimagf(z), crealf(z));
}

/// >>> START src/complex/cargl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double cargl(long double complex z) {
    return carg(z);
}
#else
long double cargl(long double complex z) {
    return atan2l(cimagl(z), creall(z));
}
#endif

/// >>> START src/complex/casin.c

// FIXME

/* asin(z) = -i log(i z + sqrt(1 - z*z)) */

double complex casin(double complex z) {
    double complex w;
    double x, y;

    x = creal(z);
    y = cimag(z);
    w = CMPLX(1.0 - (x - y)*(x + y), -2.0*x*y);
    return clog(CMPLX(-y, x) + csqrt(w));
}

/// >>> START src/complex/casinf.c

// FIXME

float complex casinf(float complex z) {
    float complex w;
    float x, y;

    x = crealf(z);
    y = cimagf(z);
    w = CMPLXF(1.0 - (x - y)*(x + y), -2.0*x*y);
    return clogf(CMPLXF(-y, x) + csqrtf(w));
}

/// >>> START src/complex/casinh.c

/* asinh(z) = -i asin(i z) */

double complex casinh(double complex z) {
    z = casin(CMPLX(-cimag(z), creal(z)));
    return CMPLX(cimag(z), -creal(z));
}

/// >>> START src/complex/casinhf.c

float complex casinhf(float complex z) {
    z = casinf(CMPLXF(-cimagf(z), crealf(z)));
    return CMPLXF(cimagf(z), -crealf(z));
}

/// >>> START src/complex/casinhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex casinhl(long double complex z) {
    return casinh(z);
}
#else
long double complex casinhl(long double complex z) {
    z = casinl(CMPLXL(-cimagl(z), creall(z)));
    return CMPLXL(cimagl(z), -creall(z));
}
#endif

/// >>> START src/complex/casinl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex casinl(long double complex z) {
    return casin(z);
}
#else
// FIXME
long double complex casinl(long double complex z) {
    long double complex w;
    long double x, y;

    x = creall(z);
    y = cimagl(z);
    w = CMPLXL(1.0 - (x - y)*(x + y), -2.0*x*y);
    return clogl(CMPLXL(-y, x) + csqrtl(w));
}
#endif

/// >>> START src/complex/catan.c
/* origin: OpenBSD /usr/src/lib/libm/src/s_catan.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Complex circular arc tangent
 *
 *
 * SYNOPSIS:
 *
 * double complex catan();
 * double complex z, w;
 *
 * w = catan (z);
 *
 *
 * DESCRIPTION:
 *
 * If
 *     z = x + iy,
 *
 * then
 *          1       (    2x     )
 * Re w  =  - arctan(-----------)  +  catan_c__k PI
 *          2       (     2    2)
 *                  (1 - x  - y )
 *
 *               ( 2         2)
 *          1    (x  +  (y+1) )
 * Im w  =  - log(------------)
 *          4    ( 2         2)
 *               (x  +  (y-1) )
 *
 * Where catan_c__k is an arbitrary integer.
 *
 * catan(z) = -i catanh(iz).
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC       -10,+10      5900       1.3e-16     7.8e-18
 *    IEEE      -10,+10     30000       2.3e-15     8.5e-17
 * The check catan( ctan(z) )  =  z, with |x| and |y| < PI/2,
 * had peak relative error 1.5e-16, rms relative error
 * 2.9e-17.  See also clog().
 */

#undef MAXNUM
#define MAXNUM 1.0e308

static const double catan_c__DP1 = 3.14159265160560607910E0;
static const double catan_c__DP2 = 1.98418714791870343106E-9;
static const double catan_c__DP3 = 1.14423774522196636802E-17;

static double _redupi(double x) {
    double t;
    long i;

    t = x/M_PI;
    if (t >= 0.0)
        t += 0.5;
    else
        t -= 0.5;

    i = t;  /* the multiple */
    t = i;
    t = ((x - t * catan_c__DP1) - t * catan_c__DP2) - t * catan_c__DP3;
    return t;
}

double complex catan(double complex z) {
    double complex w;
    double a, t, x, x2, y;

    x = creal(z);
    y = cimag(z);

    if (x == 0.0 && y > 1.0)
        goto ovrf;

    x2 = x * x;
    a = 1.0 - x2 - (y * y);
    if (a == 0.0)
        goto ovrf;

    t = 0.5 * atan2(2.0 * x, a);
    w = _redupi(t);

    t = y - 1.0;
    a = x2 + (t * t);
    if (a == 0.0)
        goto ovrf;

    t = y + 1.0;
    a = (x2 + t * t)/a;
    w = w + (0.25 * log(a)) * I;
    return w;

ovrf:
    // FIXME
    w = MAXNUM + MAXNUM * I;
    return w;
}

/// >>> START src/complex/catanf.c
/* origin: OpenBSD /usr/src/lib/libm/src/s_catanf.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Complex circular arc tangent
 *
 *
 * SYNOPSIS:
 *
 * float complex catanf();
 * float complex z, w;
 *
 * w = catanf( z );
 *
 *
 * DESCRIPTION:
 *
 * If
 *     z = x + iy,
 *
 * then
 *          1       (    2x     )
 * Re w  =  - arctan(-----------)  +  catanf_c__k PI
 *          2       (     2    2)
 *                  (1 - x  - y )
 *
 *               ( 2         2)
 *          1    (x  +  (y+1) )
 * Im w  =  - log(------------)
 *          4    ( 2         2)
 *               (x  +  (y-1) )
 *
 * Where catanf_c__k is an arbitrary integer.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10,+10     30000        2.3e-6      5.2e-8
 */

#undef MAXNUMF
#define MAXNUMF 1.0e38F

static const double catanf_c__DP1 = 3.140625;
static const double catanf_c__DP2 = 9.67502593994140625E-4;
static const double catanf_c__DP3 = 1.509957990978376432E-7;

static float _redupif(float xx) {
    float x, t;
    long i;

    x = xx;
    t = x/(float)M_PI;
    if (t >= 0.0f)
        t += 0.5f;
    else
        t -= 0.5f;

    i = t;  /* the multiple */
    t = i;
    t = ((x - t * catanf_c__DP1) - t * catanf_c__DP2) - t * catanf_c__DP3;
    return t;
}

float complex catanf(float complex z) {
    float complex w;
    float a, t, x, x2, y;

    x = crealf(z);
    y = cimagf(z);

    if ((x == 0.0f) && (y > 1.0f))
        goto ovrf;

    x2 = x * x;
    a = 1.0f - x2 - (y * y);
    if (a == 0.0f)
        goto ovrf;

    t = 0.5f * atan2f(2.0f * x, a);
    w = _redupif(t);

    t = y - 1.0f;
    a = x2 + (t * t);
    if (a == 0.0f)
        goto ovrf;

    t = y + 1.0f;
    a = (x2 + (t * t))/a;
    w = w + (0.25f * logf (a)) * I;
    return w;

ovrf:
    // FIXME
    w = MAXNUMF + MAXNUMF * I;
    return w;
}

/// >>> START src/complex/catanh.c

/* atanh = -i atan(i z) */

double complex catanh(double complex z) {
    z = catan(CMPLX(-cimag(z), creal(z)));
    return CMPLX(cimag(z), -creal(z));
}

/// >>> START src/complex/catanhf.c

float complex catanhf(float complex z) {
    z = catanf(CMPLXF(-cimagf(z), crealf(z)));
    return CMPLXF(cimagf(z), -crealf(z));
}

/// >>> START src/complex/catanhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex catanhl(long double complex z) {
    return catanh(z);
}
#else
long double complex catanhl(long double complex z) {
    z = catanl(CMPLXL(-cimagl(z), creall(z)));
    return CMPLXL(cimagl(z), -creall(z));
}
#endif

/// >>> START src/complex/catanl.c
/* origin: OpenBSD /usr/src/lib/libm/src/s_catanl.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Complex circular arc tangent
 *
 *
 * SYNOPSIS:
 *
 * long double complex catanl();
 * long double complex z, w;
 *
 * w = catanl( z );
 *
 *
 * DESCRIPTION:
 *
 * If
 *     z = x + iy,
 *
 * then
 *          1       (    2x     )
 * Re w  =  - arctan(-----------)  +  catanl_c__k PI
 *          2       (     2    2)
 *                  (1 - x  - y )
 *
 *               ( 2         2)
 *          1    (x  +  (y+1) )
 * Im w  =  - log(------------)
 *          4    ( 2         2)
 *               (x  +  (y-1) )
 *
 * Where catanl_c__k is an arbitrary integer.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC       -10,+10      5900       1.3e-16     7.8e-18
 *    IEEE      -10,+10     30000       2.3e-15     8.5e-17
 * The check catan( ctan(z) )  =  z, with |x| and |y| < PI/2,
 * had peak relative error 1.5e-16, rms relative error
 * 2.9e-17.  See also clog().
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex catanl(long double complex z) {
    return catan(z);
}
#else
static const long double catanl_c__PIL = 3.141592653589793238462643383279502884197169L;
static const long double catanl_c__DP1 = 3.14159265358979323829596852490908531763125L;
static const long double catanl_c__DP2 = 1.6667485837041756656403424829301998703007e-19L;
static const long double catanl_c__DP3 = 1.8830410776607851167459095484560349402753e-39L;

static long double redupil(long double x) {
    long double t;
    long i;

    t = x / catanl_c__PIL;
    if (t >= 0.0L)
        t += 0.5L;
    else
        t -= 0.5L;

    i = t;  /* the multiple */
    t = i;
    t = ((x - t * catanl_c__DP1) - t * catanl_c__DP2) - t * catanl_c__DP3;
    return t;
}

long double complex catanl(long double complex z) {
    long double complex w;
    long double a, t, x, x2, y;

    x = creall(z);
    y = cimagl(z);

    if ((x == 0.0L) && (y > 1.0L))
        goto ovrf;

    x2 = x * x;
    a = 1.0L - x2 - (y * y);
    if (a == 0.0L)
        goto ovrf;

    t = atan2l(2.0L * x, a) * 0.5L;
    w = redupil(t);

    t = y - 1.0L;
    a = x2 + (t * t);
    if (a == 0.0L)
        goto ovrf;

    t = y + 1.0L;
    a = (x2 + (t * t)) / a;
    w = w + (0.25L * logl(a)) * I;
    return w;

ovrf:
    // FIXME
    w = LDBL_MAX + LDBL_MAX * I;
    return w;
}
#endif

/// >>> START src/complex/ccos.c

/* cos(z) = cosh(i z) */

double complex ccos(double complex z) {
    return ccosh(CMPLX(-cimag(z), creal(z)));
}

/// >>> START src/complex/ccosf.c

float complex ccosf(float complex z) {
    return ccoshf(CMPLXF(-cimagf(z), crealf(z)));
}

/// >>> START src/complex/ccosh.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_ccosh.c */
/*-
 * Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic cosine of a complex argument z = x + i y.
 *
 * cosh(z) = cosh(x+iy)
 *         = cosh(x) cos(y) + i sinh(x) sin(y).
 *
 * Exceptional values are noted in the comments within the source code.
 * These values and the return value were taken from n1124.pdf.
 */

static const double ccosh_c__huge = 0x1p1023;

double complex ccosh(double complex z) {
    double x, y, h;
    int32_t hx, hy, ix, iy, lx, ly;

    x = creal(z);
    y = cimag(z);

    EXTRACT_WORDS(hx, lx, x);
    EXTRACT_WORDS(hy, ly, y);

    ix = 0x7fffffff & hx;
    iy = 0x7fffffff & hy;

    /* Handle the nearly-non-exceptional cases where x and y are finite. */
    if (ix < 0x7ff00000 && iy < 0x7ff00000) {
        if ((iy | ly) == 0)
            return CMPLX(cosh(x), x * y);
        if (ix < 0x40360000)    /* small x: normal case */
            return CMPLX(cosh(x) * cos(y), sinh(x) * sin(y));

        /* |x| >= 22, so cosh(x) ~= exp(|x|) */
        if (ix < 0x40862e42) {
            /* x < 710: exp(|x|) won't overflow */
            h = exp(fabs(x)) * 0.5;
            return CMPLX(h * cos(y), copysign(h, x) * sin(y));
        } else if (ix < 0x4096bbaa) {
            /* x < 1455: scale to avoid overflow */
            z = __ldexp_cexp(CMPLX(fabs(x), y), -1);
            return CMPLX(creal(z), cimag(z) * copysign(1, x));
        } else {
            /* x >= 1455: the result always overflows */
            h = ccosh_c__huge * x;
            return CMPLX(h * h * cos(y), h * sin(y));
        }
    }

    /*
     * cosh(+-0 +- I Inf) = dNaN + I sign(d(+-0, dNaN))0.
     * The sign of 0 in the result is unspecified.  Choice = normally
     * the same as dNaN.  Raise the invalid floating-point exception.
     *
     * cosh(+-0 +- I NaN) = d(NaN) + I sign(d(+-0, NaN))0.
     * The sign of 0 in the result is unspecified.  Choice = normally
     * the same as d(NaN).
     */
    if ((ix | lx) == 0 && iy >= 0x7ff00000)
        return CMPLX(y - y, copysign(0, x * (y - y)));

    /*
     * cosh(+-Inf +- I 0) = +Inf + I (+-)(+-)0.
     *
     * cosh(NaN +- I 0)   = d(NaN) + I sign(d(NaN, +-0))0.
     * The sign of 0 in the result is unspecified.
     */
    if ((iy | ly) == 0 && ix >= 0x7ff00000) {
        if (((hx & 0xfffff) | lx) == 0)
            return CMPLX(x * x, copysign(0, x) * y);
        return CMPLX(x * x, copysign(0, (x + x) * y));
    }

    /*
     * cosh(x +- I Inf) = dNaN + I dNaN.
     * Raise the invalid floating-point exception for finite nonzero x.
     *
     * cosh(x + I NaN) = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception for finite
     * nonzero x.  Choice = don't raise (except for signaling NaNs).
     */
    if (ix < 0x7ff00000 && iy >= 0x7ff00000)
        return CMPLX(y - y, x * (y - y));

    /*
     * cosh(+-Inf + I NaN)  = +Inf + I d(NaN).
     *
     * cosh(+-Inf +- I Inf) = +Inf + I dNaN.
     * The sign of Inf in the result is unspecified.  Choice = always +.
     * Raise the invalid floating-point exception.
     *
     * cosh(+-Inf + I y)   = +Inf cos(y) +- I Inf sin(y)
     */
    if (ix >= 0x7ff00000 && ((hx & 0xfffff) | lx) == 0) {
        if (iy >= 0x7ff00000)
            return CMPLX(x * x, x * (y - y));
        return CMPLX((x * x) * cos(y), x * sin(y));
    }

    /*
     * cosh(NaN + I NaN)  = d(NaN) + I d(NaN).
     *
     * cosh(NaN +- I Inf) = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception.
     * Choice = raise.
     *
     * cosh(NaN + I y)    = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception for finite
     * nonzero y.  Choice = don't raise (except for signaling NaNs).
     */
    return CMPLX((x * x) * (y - y), (x + x) * (y - y));
}

/// >>> START src/complex/ccoshf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_ccoshf.c */
/*-
 * Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic cosine of a complex argument.  See s_ccosh.c for details.
 */

static const float ccoshf_c__huge = 0x1p127;

float complex ccoshf(float complex z) {
    float x, y, h;
    int32_t hx, hy, ix, iy;

    x = crealf(z);
    y = cimagf(z);

    GET_FLOAT_WORD(hx, x);
    GET_FLOAT_WORD(hy, y);

    ix = 0x7fffffff & hx;
    iy = 0x7fffffff & hy;

    if (ix < 0x7f800000 && iy < 0x7f800000) {
        if (iy == 0)
            return CMPLXF(coshf(x), x * y);
        if (ix < 0x41100000)    /* small x: normal case */
            return CMPLXF(coshf(x) * cosf(y), sinhf(x) * sinf(y));

        /* |x| >= 9, so cosh(x) ~= exp(|x|) */
        if (ix < 0x42b17218) {
            /* x < 88.7: expf(|x|) won't overflow */
            h = expf(fabsf(x)) * 0.5f;
            return CMPLXF(h * cosf(y), copysignf(h, x) * sinf(y));
        } else if (ix < 0x4340b1e7) {
            /* x < 192.7: scale to avoid overflow */
            z = __ldexp_cexpf(CMPLXF(fabsf(x), y), -1);
            return CMPLXF(crealf(z), cimagf(z) * copysignf(1, x));
        } else {
            /* x >= 192.7: the result always overflows */
            h = ccoshf_c__huge * x;
            return CMPLXF(h * h * cosf(y), h * sinf(y));
        }
    }

    if (ix == 0 && iy >= 0x7f800000)
        return CMPLXF(y - y, copysignf(0, x * (y - y)));

    if (iy == 0 && ix >= 0x7f800000) {
        if ((hx & 0x7fffff) == 0)
            return CMPLXF(x * x, copysignf(0, x) * y);
        return CMPLXF(x * x, copysignf(0, (x + x) * y));
    }

    if (ix < 0x7f800000 && iy >= 0x7f800000)
        return CMPLXF(y - y, x * (y - y));

    if (ix >= 0x7f800000 && (hx & 0x7fffff) == 0) {
        if (iy >= 0x7f800000)
            return CMPLXF(x * x, x * (y - y));
        return CMPLXF((x * x) * cosf(y), x * sinf(y));
    }

    return CMPLXF((x * x) * (y - y), (x + x) * (y - y));
}

/// >>> START src/complex/ccoshl.c

//FIXME
long double complex ccoshl(long double complex z) {
    return ccosh(z);
}

/// >>> START src/complex/ccosl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex ccosl(long double complex z) {
    return ccos(z);
}
#else
long double complex ccosl(long double complex z) {
    return ccoshl(CMPLXL(-cimagl(z), creall(z)));
}
#endif

/// >>> START src/complex/cexp.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cexp.c */
/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const uint32_t
cexp_c__exp_ovfl  = 0x40862e42,  /* high bits of MAX_EXP * ln2 ~= 710 */
cexp_c__cexp_ovfl = 0x4096b8e4;  /* (MAX_EXP - MIN_DENORM_EXP) * ln2 */

double complex cexp(double complex z) {
    double x, y, exp_x;
    uint32_t hx, hy, lx, ly;

    x = creal(z);
    y = cimag(z);

    EXTRACT_WORDS(hy, ly, y);
    hy &= 0x7fffffff;

    /* cexp(x + I 0) = exp(x) + I 0 */
    if ((hy | ly) == 0)
        return CMPLX(exp(x), y);
    EXTRACT_WORDS(hx, lx, x);
    /* cexp(0 + I y) = cos(y) + I sin(y) */
    if (((hx & 0x7fffffff) | lx) == 0)
        return CMPLX(cos(y), sin(y));

    if (hy >= 0x7ff00000) {
        if (lx != 0 || (hx & 0x7fffffff) != 0x7ff00000) {
            /* cexp(finite|NaN +- I Inf|NaN) = NaN + I NaN */
            return CMPLX(y - y, y - y);
        } else if (hx & 0x80000000) {
            /* cexp(-Inf +- I Inf|NaN) = 0 + I 0 */
            return CMPLX(0.0, 0.0);
        } else {
            /* cexp(+Inf +- I Inf|NaN) = Inf + I NaN */
            return CMPLX(x, y - y);
        }
    }

    if (hx >= cexp_c__exp_ovfl && hx <= cexp_c__cexp_ovfl) {
        /*
         * x is between 709.7 and 1454.3, so we must scale to avoid
         * overflow in exp(x).
         */
        return __ldexp_cexp(z, 0);
    } else {
        /*
         * Cases covered here:
         *  -  x < cexp_c__exp_ovfl and exp(x) won't overflow (cexp_c__common case)
         *  -  x > cexp_c__cexp_ovfl, so exp(x) * s overflows for all s > 0
         *  -  x = +-Inf (generated by exp())
         *  -  x = NaN (spurious inexact exception from y)
         */
        exp_x = exp(x);
        return CMPLX(exp_x * cos(y), exp_x * sin(y));
    }
}

/// >>> START src/complex/cexpf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cexpf.c */
/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const uint32_t
cexpf_c__exp_ovfl  = 0x42b17218,  /* MAX_EXP * ln2 ~= 88.722839355 */
cexpf_c__cexp_ovfl = 0x43400074;  /* (MAX_EXP - MIN_DENORM_EXP) * ln2 */

float complex cexpf(float complex z) {
    float x, y, exp_x;
    uint32_t hx, hy;

    x = crealf(z);
    y = cimagf(z);

    GET_FLOAT_WORD(hy, y);
    hy &= 0x7fffffff;

    /* cexp(x + I 0) = exp(x) + I 0 */
    if (hy == 0)
        return CMPLXF(expf(x), y);
    GET_FLOAT_WORD(hx, x);
    /* cexp(0 + I y) = cos(y) + I sin(y) */
    if ((hx & 0x7fffffff) == 0)
        return CMPLXF(cosf(y), sinf(y));

    if (hy >= 0x7f800000) {
        if ((hx & 0x7fffffff) != 0x7f800000) {
            /* cexp(finite|NaN +- I Inf|NaN) = NaN + I NaN */
            return CMPLXF(y - y, y - y);
        } else if (hx & 0x80000000) {
            /* cexp(-Inf +- I Inf|NaN) = 0 + I 0 */
            return CMPLXF(0.0, 0.0);
        } else {
            /* cexp(+Inf +- I Inf|NaN) = Inf + I NaN */
            return CMPLXF(x, y - y);
        }
    }

    if (hx >= cexpf_c__exp_ovfl && hx <= cexpf_c__cexp_ovfl) {
        /*
         * x is between 88.7 and 192, so we must scale to avoid
         * overflow in expf(x).
         */
        return __ldexp_cexpf(z, 0);
    } else {
        /*
         * Cases covered here:
         *  -  x < cexpf_c__exp_ovfl and exp(x) won't overflow (cexpf_c__common case)
         *  -  x > cexpf_c__cexp_ovfl, so exp(x) * s overflows for all s > 0
         *  -  x = +-Inf (generated by exp())
         *  -  x = NaN (spurious inexact exception from y)
         */
        exp_x = expf(x);
        return CMPLXF(exp_x * cosf(y), exp_x * sinf(y));
    }
}

/// >>> START src/complex/cexpl.c

//FIXME
long double complex cexpl(long double complex z) {
    return cexp(z);
}

/// >>> START src/complex/cimag.c

double (cimag)(double complex z) {
    return cimag(z);
}

/// >>> START src/complex/cimagf.c

float (cimagf)(float complex z) {
    return cimagf(z);
}

/// >>> START src/complex/cimagl.c

long double (cimagl)(long double complex z) {
    return cimagl(z);
}

/// >>> START src/complex/clog.c

// FIXME

/* log(z) = log(|z|) + i arg(z) */

double complex clog(double complex z) {
    double r, phi;

    r = cabs(z);
    phi = carg(z);
    return CMPLX(log(r), phi);
}

/// >>> START src/complex/clogf.c

// FIXME

float complex clogf(float complex z) {
    float r, phi;

    r = cabsf(z);
    phi = cargf(z);
    return CMPLXF(logf(r), phi);
}

/// >>> START src/complex/clogl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex clogl(long double complex z) {
    return clog(z);
}
#else
// FIXME
long double complex clogl(long double complex z) {
    long double r, phi;

    r = cabsl(z);
    phi = cargl(z);
    return CMPLXL(logl(r), phi);
}
#endif

/// >>> START src/complex/conj.c

double complex conj(double complex z) {
    return CMPLX(creal(z), -cimag(z));
}

/// >>> START src/complex/conjf.c

float complex conjf(float complex z) {
    return CMPLXF(crealf(z), -cimagf(z));
}

/// >>> START src/complex/conjl.c

long double complex conjl(long double complex z) {
    return CMPLXL(creall(z), -cimagl(z));
}

/// >>> START src/complex/cpow.c

/* pow(z, c) = exp(c log(z)), See C99 G.6.4.1 */

double complex cpow(double complex z, double complex c) {
    return cexp(c * clog(z));
}

/// >>> START src/complex/cpowf.c

float complex cpowf(float complex z, float complex c) {
    return cexpf(c * clogf(z));
}

/// >>> START src/complex/cpowl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex cpowl(long double complex z, long double complex c) {
    return cpow(z, c);
}
#else
long double complex cpowl(long double complex z, long double complex c) {
    return cexpl(c * clogl(z));
}
#endif

/// >>> START src/complex/cproj.c

double complex cproj(double complex z) {
    if (isinf(creal(z)) || isinf(cimag(z)))
        return CMPLX(INFINITY, copysign(0.0, creal(z)));
    return z;
}

/// >>> START src/complex/cprojf.c

float complex cprojf(float complex z) {
    if (isinf(crealf(z)) || isinf(cimagf(z)))
        return CMPLXF(INFINITY, copysignf(0.0, crealf(z)));
    return z;
}

/// >>> START src/complex/cprojl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex cprojl(long double complex z) {
    return cproj(z);
}
#else
long double complex cprojl(long double complex z) {
    if (isinf(creall(z)) || isinf(cimagl(z)))
        return CMPLXL(INFINITY, copysignl(0.0, creall(z)));
    return z;
}
#endif

/// >>> START src/complex/creal.c

double (creal)(double complex z) {
    return creal(z);
}

/// >>> START src/complex/crealf.c

float (crealf)(float complex z) {
    return crealf(z);
}

/// >>> START src/complex/creall.c

long double (creall)(long double complex z) {
    return creall(z);
}

/// >>> START src/complex/csin.c

/* sin(z) = -i sinh(i z) */

double complex csin(double complex z) {
    z = csinh(CMPLX(-cimag(z), creal(z)));
    return CMPLX(cimag(z), -creal(z));
}

/// >>> START src/complex/csinf.c

float complex csinf(float complex z) {
    z = csinhf(CMPLXF(-cimagf(z), crealf(z)));
    return CMPLXF(cimagf(z), -crealf(z));
}

/// >>> START src/complex/csinh.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_csinh.c */
/*-
 * Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic sine of a complex argument z = x + i y.
 *
 * sinh(z) = sinh(x+iy)
 *         = sinh(x) cos(y) + i cosh(x) sin(y).
 *
 * Exceptional values are noted in the comments within the source code.
 * These values and the return value were taken from n1124.pdf.
 */

static const double csinh_c__huge = 0x1p1023;

double complex csinh(double complex z) {
    double x, y, h;
    int32_t hx, hy, ix, iy, lx, ly;

    x = creal(z);
    y = cimag(z);

    EXTRACT_WORDS(hx, lx, x);
    EXTRACT_WORDS(hy, ly, y);

    ix = 0x7fffffff & hx;
    iy = 0x7fffffff & hy;

    /* Handle the nearly-non-exceptional cases where x and y are finite. */
    if (ix < 0x7ff00000 && iy < 0x7ff00000) {
        if ((iy | ly) == 0)
            return CMPLX(sinh(x), y);
        if (ix < 0x40360000)    /* small x: normal case */
            return CMPLX(sinh(x) * cos(y), cosh(x) * sin(y));

        /* |x| >= 22, so cosh(x) ~= exp(|x|) */
        if (ix < 0x40862e42) {
            /* x < 710: exp(|x|) won't overflow */
            h = exp(fabs(x)) * 0.5;
            return CMPLX(copysign(h, x) * cos(y), h * sin(y));
        } else if (ix < 0x4096bbaa) {
            /* x < 1455: scale to avoid overflow */
            z = __ldexp_cexp(CMPLX(fabs(x), y), -1);
            return CMPLX(creal(z) * copysign(1, x), cimag(z));
        } else {
            /* x >= 1455: the result always overflows */
            h = csinh_c__huge * x;
            return CMPLX(h * cos(y), h * h * sin(y));
        }
    }

    /*
     * sinh(+-0 +- I Inf) = sign(d(+-0, dNaN))0 + I dNaN.
     * The sign of 0 in the result is unspecified.  Choice = normally
     * the same as dNaN.  Raise the invalid floating-point exception.
     *
     * sinh(+-0 +- I NaN) = sign(d(+-0, NaN))0 + I d(NaN).
     * The sign of 0 in the result is unspecified.  Choice = normally
     * the same as d(NaN).
     */
    if ((ix | lx) == 0 && iy >= 0x7ff00000)
        return CMPLX(copysign(0, x * (y - y)), y - y);

    /*
     * sinh(+-Inf +- I 0) = +-Inf + I +-0.
     *
     * sinh(NaN +- I 0)   = d(NaN) + I +-0.
     */
    if ((iy | ly) == 0 && ix >= 0x7ff00000) {
        if (((hx & 0xfffff) | lx) == 0)
            return CMPLX(x, y);
        return CMPLX(x, copysign(0, y));
    }

    /*
     * sinh(x +- I Inf) = dNaN + I dNaN.
     * Raise the invalid floating-point exception for finite nonzero x.
     *
     * sinh(x + I NaN) = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception for finite
     * nonzero x.  Choice = don't raise (except for signaling NaNs).
     */
    if (ix < 0x7ff00000 && iy >= 0x7ff00000)
        return CMPLX(y - y, x * (y - y));

    /*
     * sinh(+-Inf + I NaN)  = +-Inf + I d(NaN).
     * The sign of Inf in the result is unspecified.  Choice = normally
     * the same as d(NaN).
     *
     * sinh(+-Inf +- I Inf) = +Inf + I dNaN.
     * The sign of Inf in the result is unspecified.  Choice = always +.
     * Raise the invalid floating-point exception.
     *
     * sinh(+-Inf + I y)   = +-Inf cos(y) + I Inf sin(y)
     */
    if (ix >= 0x7ff00000 && ((hx & 0xfffff) | lx) == 0) {
        if (iy >= 0x7ff00000)
            return CMPLX(x * x, x * (y - y));
        return CMPLX(x * cos(y), INFINITY * sin(y));
    }

    /*
     * sinh(NaN + I NaN)  = d(NaN) + I d(NaN).
     *
     * sinh(NaN +- I Inf) = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception.
     * Choice = raise.
     *
     * sinh(NaN + I y)    = d(NaN) + I d(NaN).
     * Optionally raises the invalid floating-point exception for finite
     * nonzero y.  Choice = don't raise (except for signaling NaNs).
     */
    return CMPLX((x * x) * (y - y), (x + x) * (y - y));
}

/// >>> START src/complex/csinhf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_csinhf.c */
/*-
 * Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic sine of a complex argument z.  See s_csinh.c for details.
 */

static const float csinhf_c__huge = 0x1p127;

float complex csinhf(float complex z) {
    float x, y, h;
    int32_t hx, hy, ix, iy;

    x = crealf(z);
    y = cimagf(z);

    GET_FLOAT_WORD(hx, x);
    GET_FLOAT_WORD(hy, y);

    ix = 0x7fffffff & hx;
    iy = 0x7fffffff & hy;

    if (ix < 0x7f800000 && iy < 0x7f800000) {
        if (iy == 0)
            return CMPLXF(sinhf(x), y);
        if (ix < 0x41100000)    /* small x: normal case */
            return CMPLXF(sinhf(x) * cosf(y), coshf(x) * sinf(y));

        /* |x| >= 9, so cosh(x) ~= exp(|x|) */
        if (ix < 0x42b17218) {
            /* x < 88.7: expf(|x|) won't overflow */
            h = expf(fabsf(x)) * 0.5f;
            return CMPLXF(copysignf(h, x) * cosf(y), h * sinf(y));
        } else if (ix < 0x4340b1e7) {
            /* x < 192.7: scale to avoid overflow */
            z = __ldexp_cexpf(CMPLXF(fabsf(x), y), -1);
            return CMPLXF(crealf(z) * copysignf(1, x), cimagf(z));
        } else {
            /* x >= 192.7: the result always overflows */
            h = csinhf_c__huge * x;
            return CMPLXF(h * cosf(y), h * h * sinf(y));
        }
    }

    if (ix == 0 && iy >= 0x7f800000)
        return CMPLXF(copysignf(0, x * (y - y)), y - y);

    if (iy == 0 && ix >= 0x7f800000) {
        if ((hx & 0x7fffff) == 0)
            return CMPLXF(x, y);
        return CMPLXF(x, copysignf(0, y));
    }

    if (ix < 0x7f800000 && iy >= 0x7f800000)
        return CMPLXF(y - y, x * (y - y));

    if (ix >= 0x7f800000 && (hx & 0x7fffff) == 0) {
        if (iy >= 0x7f800000)
            return CMPLXF(x * x, x * (y - y));
        return CMPLXF(x * cosf(y), INFINITY * sinf(y));
    }

    return CMPLXF((x * x) * (y - y), (x + x) * (y - y));
}

/// >>> START src/complex/csinhl.c

//FIXME
long double complex csinhl(long double complex z) {
    return csinh(z);
}

/// >>> START src/complex/csinl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex csinl(long double complex z) {
    return csin(z);
}
#else
long double complex csinl(long double complex z) {
    z = csinhl(CMPLXL(-cimagl(z), creall(z)));
    return CMPLXL(cimagl(z), -creall(z));
}
#endif

/// >>> START src/complex/csqrt.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_csqrt.c */
/*-
 * Copyright (c) 2007 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * gcc doesn't implement complex multiplication or division correctly,
 * so we need to handle infinities specially. We turn on this pragma to
 * notify conforming c99 compilers that the fast-but-incorrect code that
 * gcc generates is acceptable, since the special cases have already been
 * handled.
 */
#pragma STDC CX_LIMITED_RANGE ON

/* We risk spurious overflow for components >= DBL_MAX / (1 + sqrt(2)). */
#undef THRESH
#define THRESH  0x1.a827999fcef32p+1022

double complex csqrt(double complex z) {
    double complex result;
    double a, b;
    double t;
    int scale;

    a = creal(z);
    b = cimag(z);

    /* Handle special cases. */
    if (z == 0)
        return CMPLX(0, b);
    if (isinf(b))
        return CMPLX(INFINITY, b);
    if (isnan(a)) {
        t = (b - b) / (b - b);  /* raise invalid if b is not a NaN */
        return CMPLX(a, t);   /* return NaN + NaN i */
    }
    if (isinf(a)) {
        /*
         * csqrt(inf + NaN i)  = inf +  NaN i
         * csqrt(inf + y i)    = inf +  0 i
         * csqrt(-inf + NaN i) = NaN +- inf i
         * csqrt(-inf + y i)   = 0   +  inf i
         */
        if (signbit(a))
            return CMPLX(fabs(b - b), copysign(a, b));
        else
            return CMPLX(a, copysign(b - b, b));
    }
    /*
     * The remaining special case (b is NaN) is handled just fine by
     * the normal code path below.
     */

    /* Scale to avoid overflow. */
    if (fabs(a) >= THRESH || fabs(b) >= THRESH) {
        a *= 0.25;
        b *= 0.25;
        scale = 1;
    } else {
        scale = 0;
    }

    /* Algorithm 312, CACM vol 10, Oct 1967. */
    if (a >= 0) {
        t = sqrt((a + hypot(a, b)) * 0.5);
        result = CMPLX(t, b / (2 * t));
    } else {
        t = sqrt((-a + hypot(a, b)) * 0.5);
        result = CMPLX(fabs(b) / (2 * t), copysign(t, b));
    }

    /* Rescale. */
    if (scale)
        result *= 2;
    return result;
}

/// >>> START src/complex/csqrtf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_csqrtf.c */
/*-
 * Copyright (c) 2007 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * gcc doesn't implement complex multiplication or division correctly,
 * so we need to handle infinities specially. We turn on this pragma to
 * notify conforming c99 compilers that the fast-but-incorrect code that
 * gcc generates is acceptable, since the special cases have already been
 * handled.
 */
#pragma STDC CX_LIMITED_RANGE ON

float complex csqrtf(float complex z) {
    float a = crealf(z), b = cimagf(z);
    double t;

    /* Handle special cases. */
    if (z == 0)
        return CMPLXF(0, b);
    if (isinf(b))
        return CMPLXF(INFINITY, b);
    if (isnan(a)) {
        t = (b - b) / (b - b);  /* raise invalid if b is not a NaN */
        return CMPLXF(a, t);  /* return NaN + NaN i */
    }
    if (isinf(a)) {
        /*
         * csqrtf(inf + NaN i)  = inf +  NaN i
         * csqrtf(inf + y i)    = inf +  0 i
         * csqrtf(-inf + NaN i) = NaN +- inf i
         * csqrtf(-inf + y i)   = 0   +  inf i
         */
        if (signbit(a))
            return CMPLXF(fabsf(b - b), copysignf(a, b));
        else
            return CMPLXF(a, copysignf(b - b, b));
    }
    /*
     * The remaining special case (b is NaN) is handled just fine by
     * the normal code path below.
     */

    /*
     * We compute t in double precision to avoid overflow and to
     * provide correct rounding in nearly all cases.
     * This is Algorithm 312, CACM vol 10, Oct 1967.
     */
    if (a >= 0) {
        t = sqrt((a + hypot(a, b)) * 0.5);
        return CMPLXF(t, b / (2.0 * t));
    } else {
        t = sqrt((-a + hypot(a, b)) * 0.5);
        return CMPLXF(fabsf(b) / (2.0 * t), copysignf(t, b));
    }
}

/// >>> START src/complex/csqrtl.c

//FIXME
long double complex csqrtl(long double complex z) {
    return csqrt(z);
}

/// >>> START src/complex/ctan.c

/* tan(z) = -i tanh(i z) */

double complex ctan(double complex z) {
    z = ctanh(CMPLX(-cimag(z), creal(z)));
    return CMPLX(cimag(z), -creal(z));
}

/// >>> START src/complex/ctanf.c

float complex ctanf(float complex z) {
    z = ctanhf(CMPLXF(-cimagf(z), crealf(z)));
    return CMPLXF(cimagf(z), -crealf(z));
}

/// >>> START src/complex/ctanh.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_ctanh.c */
/*-
 * Copyright (c) 2011 David Schultz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic tangent of a complex argument z = x + i y.
 *
 * The algorithm is from:
 *
 *   W. Kahan.  Branch Cuts for Complex Elementary Functions or Much
 *   Ado About Nothing's Sign Bit.  In The State of the Art in
 *   Numerical Analysis, pp. 165 ff.  Iserles and Powell, eds., 1987.
 *
 * Method:
 *
 *   Let t    = tan(x)
 *       beta = 1/cos^2(y)
 *       s    = sinh(x)
 *       rho  = cosh(x)
 *
 *   We have:
 *
 *   tanh(z) = sinh(z) / cosh(z)
 *
 *             sinh(x) cos(y) + i cosh(x) sin(y)
 *           = ---------------------------------
 *             cosh(x) cos(y) + i sinh(x) sin(y)
 *
 *             cosh(x) sinh(x) / cos^2(y) + i tan(y)
 *           = -------------------------------------
 *                    1 + sinh^2(x) / cos^2(y)
 *
 *             beta rho s + i t
 *           = ----------------
 *               1 + beta s^2
 *
 * Modifications:
 *
 *   I omitted the original algorithm's handling of overflow in tan(x) after
 *   verifying with nearpi.c that this can't happen in IEEE single or double
 *   precision.  I also handle large x differently.
 */

double complex ctanh(double complex z) {
    double x, y;
    double t, beta, s, rho, denom;
    uint32_t hx, ix, lx;

    x = creal(z);
    y = cimag(z);

    EXTRACT_WORDS(hx, lx, x);
    ix = hx & 0x7fffffff;

    /*
     * ctanh(NaN + i 0) = NaN + i 0
     *
     * ctanh(NaN + i y) = NaN + i NaN               for y != 0
     *
     * The imaginary part has the sign of x*sin(2*y), but there's no
     * special effort to get this right.
     *
     * ctanh(+-Inf +- i Inf) = +-1 +- 0
     *
     * ctanh(+-Inf + i y) = +-1 + 0 sin(2y)         for y finite
     *
     * The imaginary part of the sign is unspecified.  This special
     * case is only needed to avoid a spurious invalid exception when
     * y is infinite.
     */
    if (ix >= 0x7ff00000) {
        if ((ix & 0xfffff) | lx)        /* x is NaN */
            return CMPLX(x, (y == 0 ? y : x * y));
        SET_HIGH_WORD(x, hx - 0x40000000);      /* x = copysign(1, x) */
        return CMPLX(x, copysign(0, isinf(y) ? y : sin(y) * cos(y)));
    }

    /*
     * ctanh(x + i NAN) = NaN + i NaN
     * ctanh(x +- i Inf) = NaN + i NaN
     */
    if (!isfinite(y))
        return CMPLX(y - y, y - y);

    /*
     * ctanh(+-ctanh_c__huge + i +-y) ~= +-1 +- i 2sin(2y)/exp(2x), using the
     * approximation sinh^2(ctanh_c__huge) ~= exp(2*ctanh_c__huge) / 4.
     * We use a modified formula to avoid spurious overflow.
     */
    if (ix >= 0x40360000) { /* x >= 22 */
        double exp_mx = exp(-fabs(x));
        return CMPLX(copysign(1, x), 4 * sin(y) * cos(y) * exp_mx * exp_mx);
    }

    /* Kahan's algorithm */
    t = tan(y);
    beta = 1.0 + t * t;     /* = 1 / cos^2(y) */
    s = sinh(x);
    rho = sqrt(1 + s * s);  /* = cosh(x) */
    denom = 1 + beta * s * s;
    return CMPLX((beta * rho * s) / denom, t / denom);
}

/// >>> START src/complex/ctanhf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_ctanhf.c */
/*-
 * Copyright (c) 2011 David Schultz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Hyperbolic tangent of a complex argument z.  See s_ctanh.c for details.
 */

float complex ctanhf(float complex z) {
    float x, y;
    float t, beta, s, rho, denom;
    uint32_t hx, ix;

    x = crealf(z);
    y = cimagf(z);

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;

    if (ix >= 0x7f800000) {
        if (ix & 0x7fffff)
            return CMPLXF(x, (y == 0 ? y : x * y));
        SET_FLOAT_WORD(x, hx - 0x40000000);
        return CMPLXF(x, copysignf(0, isinf(y) ? y : sinf(y) * cosf(y)));
    }

    if (!isfinite(y))
        return CMPLXF(y - y, y - y);

    if (ix >= 0x41300000) { /* x >= 11 */
        float exp_mx = expf(-fabsf(x));
        return CMPLXF(copysignf(1, x), 4 * sinf(y) * cosf(y) * exp_mx * exp_mx);
    }

    t = tanf(y);
    beta = 1.0 + t * t;
    s = sinhf(x);
    rho = sqrtf(1 + s * s);
    denom = 1 + beta * s * s;
    return CMPLXF((beta * rho * s) / denom, t / denom);
}

/// >>> START src/complex/ctanhl.c

//FIXME
long double complex ctanhl(long double complex z) {
    return ctanh(z);
}

/// >>> START src/complex/ctanl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double complex ctanl(long double complex z) {
    return ctan(z);
}
#else
long double complex ctanl(long double complex z) {
    z = ctanhl(CMPLXL(-cimagl(z), creall(z)));
    return CMPLXL(cimagl(z), -creall(z));
}
#endif

/// >>> START src/conf/confstr.c

size_t confstr(int name, char *buf, size_t len) {
    const char *s = "";
    if (!name) {
        s = "/bin:/usr/bin";
    } else if ((name&~4U)!=1 && name-_CS_POSIX_V6_ILP32_OFF32_CFLAGS>31U) {
        errno = EINVAL;
        return 0;
    }
    // snprintf is overkill but avoid wasting code size to implement
    // this completely useless function and its truncation semantics
    return snprintf(buf, len, "%s", s);
}

/// >>> START src/conf/fpathconf.c

long fpathconf(int fd, int name) {
    static const short values[] = {
        [_PC_LINK_MAX] = _POSIX_LINK_MAX,
        [_PC_MAX_CANON] = _POSIX_MAX_CANON,
        [_PC_MAX_INPUT] = _POSIX_MAX_INPUT,
        [_PC_NAME_MAX] = NAME_MAX,
        [_PC_PATH_MAX] = PATH_MAX,
        [_PC_PIPE_BUF] = PIPE_BUF,
        [_PC_CHOWN_RESTRICTED] = 1,
        [_PC_NO_TRUNC] = 1,
        [_PC_VDISABLE] = 0,
        [_PC_SYNC_IO] = 1,
        [_PC_ASYNC_IO] = -1,
        [_PC_PRIO_IO] = -1,
        [_PC_SOCK_MAXBUF] = -1,
        [_PC_FILESIZEBITS] = FILESIZEBITS,
        [_PC_REC_INCR_XFER_SIZE] = 4096,
        [_PC_REC_MAX_XFER_SIZE] = 4096,
        [_PC_REC_MIN_XFER_SIZE] = 4096,
        [_PC_REC_XFER_ALIGN] = 4096,
        [_PC_ALLOC_SIZE_MIN] = 4096,
        [_PC_SYMLINK_MAX] = SYMLINK_MAX,
        [_PC_2_SYMLINKS] = 1
    };
    if (name > sizeof(values)/sizeof(values[0])) {
        errno = EINVAL;
        return -1;
    }
    return values[name];
}

/// >>> START src/conf/pathconf.c

long pathconf(const char *path, int name) {
    return fpathconf(-1, name);
}

/// >>> START src/conf/sysconf.c
/// >>> START src/internal/libc.h
#ifndef LIBC_H
#undef LIBC_H
#define LIBC_H

struct __libc {
    void *main_thread;
    int threaded;
    int secure;
    size_t *auxv;
    volatile int threads_minus_1;
    int canceldisable;
    FILE *ofl_head;
    int ofl_lock[2];
    size_t tls_size;
    size_t page_size;
};

extern size_t __hwcap;

#ifndef PAGE_SIZE
#undef PAGE_SIZE
#define PAGE_SIZE libc.page_size
#endif

#if !defined(__PIC__) || (100*__GNUC__+__GNUC_MINOR__ >= 303 && !defined(__PCC__))

#ifdef __PIC__
#if __GNUC__ < 4
#undef BROKEN_VISIBILITY
#define BROKEN_VISIBILITY 1
#endif
#undef ATTR_LIBC_VISIBILITY
#define ATTR_LIBC_VISIBILITY __attribute__((visibility("hidden")))
#else
#undef ATTR_LIBC_VISIBILITY
#define ATTR_LIBC_VISIBILITY
#endif

extern struct __libc __libc ATTR_LIBC_VISIBILITY;
#undef libc
#define libc __libc

#else

#undef USE_LIBC_ACCESSOR
#define USE_LIBC_ACCESSOR
#undef ATTR_LIBC_VISIBILITY
#define ATTR_LIBC_VISIBILITY
extern struct __libc *__libc_loc(void) __attribute__((const));
#undef libc
#define libc (*__libc_loc())

#endif

/* Designed to avoid any overhead in non-threaded processes */
void __lock(volatile int *) ATTR_LIBC_VISIBILITY;
void __unlock(volatile int *) ATTR_LIBC_VISIBILITY;
int __lockfile(FILE *) ATTR_LIBC_VISIBILITY;
void __unlockfile(FILE *) ATTR_LIBC_VISIBILITY;

void __synccall(void (*)(void *), void *);
int __setxid(int, int, int, int);

extern char **__environ;

#undef weak_alias
#undef weak_alias
#define weak_alias(old, new) \
    extern __typeof(old) new __attribute__((weak, alias(#old)))

#undef LFS64_2
#undef LFS64_2
#define LFS64_2(x, y) weak_alias(x, y)

#undef LFS64
#undef LFS64
#define LFS64(x) LFS64_2(x, x##64)

#endif

/// >>> CONTINUE src/conf/sysconf.c

#undef VER
#define VER (-2)
#undef OFLOW
#define OFLOW (-3)
#undef CPUCNT
#define CPUCNT (-4)
#undef RLIM
#define RLIM(x) (-32768|(RLIMIT_ ## x))

long sysconf(int name) {
    static const short values[] = {
        [_SC_ARG_MAX] = OFLOW,
        [_SC_CHILD_MAX] = RLIM(NPROC),
        [_SC_CLK_TCK] = 100,
        [_SC_NGROUPS_MAX] = 32,
        [_SC_OPEN_MAX] = RLIM(NOFILE),
        [_SC_STREAM_MAX] = -1,
        [_SC_TZNAME_MAX] = TZNAME_MAX,
        [_SC_JOB_CONTROL] = 1,
        [_SC_SAVED_IDS] = 1,
        [_SC_REALTIME_SIGNALS] = 1,
        [_SC_PRIORITY_SCHEDULING] = -1,
        [_SC_TIMERS] = VER,
        [_SC_ASYNCHRONOUS_IO] = VER,
        [_SC_PRIORITIZED_IO] = -1,
        [_SC_SYNCHRONIZED_IO] = -1,
        [_SC_FSYNC] = VER,
        [_SC_MAPPED_FILES] = VER,
        [_SC_MEMLOCK] = VER,
        [_SC_MEMLOCK_RANGE] = VER,
        [_SC_MEMORY_PROTECTION] = VER,
        [_SC_MESSAGE_PASSING] = VER,
        [_SC_SEMAPHORES] = VER,
        [_SC_SHARED_MEMORY_OBJECTS] = VER,
        [_SC_AIO_LISTIO_MAX] = -1,
        [_SC_AIO_MAX] = -1,
        [_SC_AIO_PRIO_DELTA_MAX] = 0, /* ?? */
        [_SC_DELAYTIMER_MAX] = _POSIX_DELAYTIMER_MAX,
        [_SC_MQ_OPEN_MAX] = -1,
        [_SC_MQ_PRIO_MAX] = OFLOW,
        [_SC_VERSION] = VER,
        [_SC_PAGE_SIZE] = OFLOW,
        [_SC_RTSIG_MAX] = _NSIG - 1 - 31 - 3,
        [_SC_SEM_NSEMS_MAX] = SEM_NSEMS_MAX,
        [_SC_SEM_VALUE_MAX] = OFLOW,
        [_SC_SIGQUEUE_MAX] = -1,
        [_SC_TIMER_MAX] = -1,
        [_SC_BC_BASE_MAX] = _POSIX2_BC_BASE_MAX,
        [_SC_BC_DIM_MAX] = _POSIX2_BC_DIM_MAX,
        [_SC_BC_SCALE_MAX] = _POSIX2_BC_SCALE_MAX,
        [_SC_BC_STRING_MAX] = _POSIX2_BC_STRING_MAX,
        [_SC_COLL_WEIGHTS_MAX] = COLL_WEIGHTS_MAX,
        [_SC_EQUIV_CLASS_MAX] = -1, /* ?? */
        [_SC_EXPR_NEST_MAX] = -1,
        [_SC_LINE_MAX] = -1,
        [_SC_RE_DUP_MAX] = RE_DUP_MAX,
        [_SC_CHARCLASS_NAME_MAX] = -1, /* ?? */
        [_SC_2_VERSION] = VER,
        [_SC_2_C_BIND] = VER,
        [_SC_2_C_DEV] = -1,
        [_SC_2_FORT_DEV] = -1,
        [_SC_2_FORT_RUN] = -1,
        [_SC_2_SW_DEV] = -1,
        [_SC_2_LOCALEDEF] = -1,
        [_SC_PII] = -1, /* ????????? */
        [_SC_PII_XTI] = -1,
        [_SC_PII_SOCKET] = -1,
        [_SC_PII_INTERNET] = -1,
        [_SC_PII_OSI] = -1,
        [_SC_POLL] = 1,
        [_SC_SELECT] = 1,
        [_SC_IOV_MAX] = IOV_MAX,
        [_SC_PII_INTERNET_STREAM] = -1,
        [_SC_PII_INTERNET_DGRAM] = -1,
        [_SC_PII_OSI_COTS] = -1,
        [_SC_PII_OSI_CLTS] = -1,
        [_SC_PII_OSI_M] = -1,
        [_SC_T_IOV_MAX] = -1,
        [_SC_THREADS] = VER,
        [_SC_THREAD_SAFE_FUNCTIONS] = VER,
        [_SC_GETGR_R_SIZE_MAX] = -1,
        [_SC_GETPW_R_SIZE_MAX] = -1,
        [_SC_LOGIN_NAME_MAX] = 256,
        [_SC_TTY_NAME_MAX] = TTY_NAME_MAX,
        [_SC_THREAD_DESTRUCTOR_ITERATIONS] = PTHREAD_DESTRUCTOR_ITERATIONS,
        [_SC_THREAD_KEYS_MAX] = PTHREAD_KEYS_MAX,
        [_SC_THREAD_STACK_MIN] = PTHREAD_STACK_MIN,
        [_SC_THREAD_THREADS_MAX] = -1,
        [_SC_THREAD_ATTR_STACKADDR] = VER,
        [_SC_THREAD_ATTR_STACKSIZE] = VER,
        [_SC_THREAD_PRIORITY_SCHEDULING] = VER,
        [_SC_THREAD_PRIO_INHERIT] = -1,
        [_SC_THREAD_PRIO_PROTECT] = -1,
        [_SC_THREAD_PROCESS_SHARED] = VER,
        [_SC_NPROCESSORS_CONF] = CPUCNT,
        [_SC_NPROCESSORS_ONLN] = CPUCNT,
        [_SC_PHYS_PAGES] = -1,
        [_SC_AVPHYS_PAGES] = -1,
        [_SC_ATEXIT_MAX] = -1,
        [_SC_PASS_MAX] = -1,
        [_SC_XOPEN_VERSION] = _XOPEN_VERSION,
        [_SC_XOPEN_XCU_VERSION] = _XOPEN_VERSION,
        [_SC_XOPEN_UNIX] = 1,
        [_SC_XOPEN_CRYPT] = -1,
        [_SC_XOPEN_ENH_I18N] = 1,
        [_SC_XOPEN_SHM] = 1,
        [_SC_2_CHAR_TERM] = -1,
        [_SC_2_C_VERSION] = -1,
        [_SC_2_UPE] = -1,
        [_SC_XOPEN_XPG2] = -1,
        [_SC_XOPEN_XPG3] = -1,
        [_SC_XOPEN_XPG4] = -1,
        [_SC_CHAR_BIT] = -1,
        [_SC_CHAR_MAX] = -1,
        [_SC_CHAR_MIN] = -1,
        [_SC_INT_MAX] = -1,
        [_SC_INT_MIN] = -1,
        [_SC_LONG_BIT] = -1,
        [_SC_WORD_BIT] = -1,
        [_SC_MB_LEN_MAX] = -1,
        [_SC_NZERO] = NZERO,
        [_SC_SSIZE_MAX] = -1,
        [_SC_SCHAR_MAX] = -1,
        [_SC_SCHAR_MIN] = -1,
        [_SC_SHRT_MAX] = -1,
        [_SC_SHRT_MIN] = -1,
        [_SC_UCHAR_MAX] = -1,
        [_SC_UINT_MAX] = -1,
        [_SC_ULONG_MAX] = -1,
        [_SC_USHRT_MAX] = -1,
        [_SC_NL_ARGMAX] = -1,
        [_SC_NL_LANGMAX] = -1,
        [_SC_NL_MSGMAX] = -1,
        [_SC_NL_NMAX] = -1,
        [_SC_NL_SETMAX] = -1,
        [_SC_NL_TEXTMAX] = -1,
        [_SC_XBS5_ILP32_OFF32] = -1,
        [_SC_XBS5_ILP32_OFFBIG] = 2*(sizeof(long)==4)-1,
        [_SC_XBS5_LP64_OFF64] = 2*(sizeof(long)==8)-1,
        [_SC_XBS5_LPBIG_OFFBIG] = -1,
        [_SC_XOPEN_LEGACY] = -1,
        [_SC_XOPEN_REALTIME] = -1,
        [_SC_XOPEN_REALTIME_THREADS] = -1,
        [_SC_ADVISORY_INFO] = VER,
        [_SC_BARRIERS] = VER,
        [_SC_BASE] = -1,
        [_SC_C_LANG_SUPPORT] = -1,
        [_SC_C_LANG_SUPPORT_R] = -1,
        [_SC_CLOCK_SELECTION] = VER,
        [_SC_CPUTIME] = VER,
        [_SC_THREAD_CPUTIME] = VER,
        [_SC_DEVICE_IO] = -1,
        [_SC_DEVICE_SPECIFIC] = -1,
        [_SC_DEVICE_SPECIFIC_R] = -1,
        [_SC_FD_MGMT] = -1,
        [_SC_FIFO] = -1,
        [_SC_PIPE] = -1,
        [_SC_FILE_ATTRIBUTES] = -1,
        [_SC_FILE_LOCKING] = -1,
        [_SC_FILE_SYSTEM] = -1,
        [_SC_MONOTONIC_CLOCK] = VER,
        [_SC_MULTI_PROCESS] = -1,
        [_SC_SINGLE_PROCESS] = -1,
        [_SC_NETWORKING] = -1,
        [_SC_READER_WRITER_LOCKS] = VER,
        [_SC_SPIN_LOCKS] = VER,
        [_SC_REGEXP] = 1,
        [_SC_REGEX_VERSION] = -1,
        [_SC_SHELL] = 1,
        [_SC_SIGNALS] = -1,
        [_SC_SPAWN] = VER,
        [_SC_SPORADIC_SERVER] = -1,
        [_SC_THREAD_SPORADIC_SERVER] = -1,
        [_SC_SYSTEM_DATABASE] = -1,
        [_SC_SYSTEM_DATABASE_R] = -1,
        [_SC_TIMEOUTS] = VER,
        [_SC_TYPED_MEMORY_OBJECTS] = -1,
        [_SC_USER_GROUPS] = -1,
        [_SC_USER_GROUPS_R] = -1,
        [_SC_2_PBS] = -1,
        [_SC_2_PBS_ACCOUNTING] = -1,
        [_SC_2_PBS_LOCATE] = -1,
        [_SC_2_PBS_MESSAGE] = -1,
        [_SC_2_PBS_TRACK] = -1,
        [_SC_SYMLOOP_MAX] = SYMLOOP_MAX,
        [_SC_STREAMS] = 0,
        [_SC_2_PBS_CHECKPOINT] = -1,
        [_SC_V6_ILP32_OFF32] = -1,
        [_SC_V6_ILP32_OFFBIG] = 2*(sizeof(long)==4)-1,
        [_SC_V6_LP64_OFF64] = 2*(sizeof(long)==8)-1,
        [_SC_V6_LPBIG_OFFBIG] = -1,
        [_SC_HOST_NAME_MAX] = HOST_NAME_MAX,
        [_SC_TRACE] = -1,
        [_SC_TRACE_EVENT_FILTER] = -1,
        [_SC_TRACE_INHERIT] = -1,
        [_SC_TRACE_LOG] = -1,

        [_SC_IPV6] = VER,
        [_SC_RAW_SOCKETS] = VER,
        [_SC_V7_ILP32_OFF32] = -1,
        [_SC_V7_ILP32_OFFBIG] = 2*(sizeof(long)==4)-1,
        [_SC_V7_LP64_OFF64] = 2*(sizeof(long)==8)-1,
        [_SC_V7_LPBIG_OFFBIG] = -1,
        [_SC_SS_REPL_MAX] = -1,
        [_SC_TRACE_EVENT_NAME_MAX] = -1,
        [_SC_TRACE_NAME_MAX] = -1,
        [_SC_TRACE_SYS_MAX] = -1,
        [_SC_TRACE_USER_EVENT_MAX] = -1,
        [_SC_XOPEN_STREAMS] = 0,
        [_SC_THREAD_ROBUST_PRIO_INHERIT] = -1,
        [_SC_THREAD_ROBUST_PRIO_PROTECT] = -1,
    };
    if (name > sizeof(values)/sizeof(values[0])) {
        errno = EINVAL;
        return -1;
    } else if (values[name] == VER) {
        return _POSIX_VERSION;
    } else if (values[name] == OFLOW) {
        if (name == _SC_ARG_MAX) return ARG_MAX;
        if (name == _SC_SEM_VALUE_MAX) return SEM_VALUE_MAX;
        if (name == _SC_MQ_PRIO_MAX) return MQ_PRIO_MAX;
        /* name == _SC_PAGE_SIZE */
        return PAGE_SIZE;
    } else if (values[name] == CPUCNT) {
        unsigned char set[128] = {1};
        int i, cnt;
        __syscall(SYS_sched_getaffinity, 0, sizeof set, set);
        for (i=cnt=0; i<sizeof set; i++)
            for (; set[i]; set[i]&=set[i]-1, cnt++);
        return cnt;
    } else if (values[name] < OFLOW) {
        long lim[2];
        __syscall(SYS_getrlimit, values[name]&16383, lim);
        return lim[0] < 0 ? LONG_MAX : lim[0];
    }
    return values[name];
}

/// >>> START src/ctype/__ctype_b_loc.c

#if __BYTE_ORDER == __BIG_ENDIAN
#undef X
#define X(x) x
#else
#undef X
#define X(x) (((x)/256 | (x)*256) % 65536)
#endif

static const unsigned short __ctype_b_loc_c__table[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x200),X(0x320),X(0x220),X(0x220),X(0x220),X(0x220),X(0x200),X(0x200),
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x160),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),
X(0x8d8),X(0x8d8),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x200),
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const unsigned short *const __ctype_b_loc_c__ptable = __ctype_b_loc_c__table+128;

const unsigned short **__ctype_b_loc(void) {
    return (void *)&__ctype_b_loc_c__ptable;
}

/// >>> START src/ctype/__ctype_get_mb_cur_max.c

size_t __ctype_get_mb_cur_max() {
    return 4;
}

/// >>> START src/ctype/__ctype_tolower_loc.c

static const int32_t __ctype_tolower_loc_c__table[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,
'a','b','c','d','e','f','g','h','i','j','k','l','m',
'n','o','p','q','r','s','t','u','v','w','x','y','z',
91,92,93,94,95,96,
'a','b','c','d','e','f','g','h','i','j','k','l','m',
'n','o','p','q','r','s','t','u','v','w','x','y','z',
123,124,125,126,127,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const int32_t *const __ctype_tolower_loc_c__ptable = __ctype_tolower_loc_c__table+128;

const int32_t **__ctype_tolower_loc(void) {
    return (void *)&__ctype_tolower_loc_c__ptable;
}

/// >>> START src/ctype/__ctype_toupper_loc.c

static const int32_t __ctype_toupper_loc_c__table[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,
'A','B','C','D','E','F','G','H','I','J','K','L','M',
'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
91,92,93,94,95,96,
'A','B','C','D','E','F','G','H','I','J','K','L','M',
'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
123,124,125,126,127,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const int32_t *const __ctype_toupper_loc_c__ptable = __ctype_toupper_loc_c__table+128;

const int32_t **__ctype_toupper_loc(void) {
    return (void *)&__ctype_toupper_loc_c__ptable;
}

/// >>> START src/ctype/isalnum.c

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

/// >>> START src/ctype/isalpha.c
#undef isalpha

int isalpha(int c) {
    return ((unsigned)c|32)-'a' < 26;
}

/// >>> START src/ctype/isascii.c

int isascii(int c) {
    return !(c&~0x7f);
}

/// >>> START src/ctype/isblank.c

int isblank(int c) {
    return (c == ' ' || c == '\t');
}

/// >>> START src/ctype/iscntrl.c

int iscntrl(int c) {
    return (unsigned)c < 0x20 || c == 0x7f;
}

/// >>> START src/ctype/isdigit.c
#undef isdigit

int isdigit(int c) {
    return (unsigned)c-'0' < 10;
}

/// >>> START src/ctype/isgraph.c
int isgraph(int c) {
    return (unsigned)c-0x21 < 0x5e;
}

/// >>> START src/ctype/islower.c
#undef islower

int islower(int c) {
    return (unsigned)c-'a' < 26;
}

/// >>> START src/ctype/isprint.c
int isprint(int c) {
    return (unsigned)c-0x20 < 0x5f;
}

/// >>> START src/ctype/ispunct.c

int ispunct(int c) {
    return isgraph(c) && !isalnum(c);
}

/// >>> START src/ctype/isspace.c

int isspace(int c) {
    return c == ' ' || (unsigned)c-'\t' < 5;
}

/// >>> START src/ctype/isupper.c
#undef isupper

int isupper(int c) {
    return (unsigned)c-'A' < 26;
}

/// >>> START src/ctype/iswalnum.c

int iswalnum(wint_t iswalnum_c__wc) {
    return iswdigit(iswalnum_c__wc) || iswalpha(iswalnum_c__wc);
}

/// >>> START src/ctype/iswalpha.c

static const unsigned char iswalpha_c__table[] = {
/// >>> START src/ctype/alpha.h
18,17,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,17,34,35,36,17,37,38,39,40,
41,42,43,44,17,45,46,47,16,16,48,16,16,16,16,16,16,16,49,50,51,16,52,53,16,16,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,54,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,55,17,17,17,17,56,17,57,58,59,60,61,62,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,63,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,64,65,17,66,67,68,69,70,71,72,
73,16,16,16,74,75,76,77,78,16,16,16,79,80,16,16,16,16,81,16,16,16,16,16,16,16,
16,16,17,17,17,82,83,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,84,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,85,16,
16,16,16,86,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,87,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
88,89,90,91,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
92,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,0,0,0,0,0,0,0,0,254,255,255,7,254,255,255,7,0,0,0,0,0,4,32,4,
255,255,127,255,255,255,127,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,195,255,3,0,31,80,0,0,0,0,
0,0,0,0,0,0,32,0,0,0,0,0,223,60,64,215,255,255,251,255,255,255,255,255,255,
255,255,255,191,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,3,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,0,254,255,255,255,127,2,254,255,255,255,255,0,0,0,0,0,255,191,182,
0,255,255,255,7,7,0,0,0,255,7,255,255,255,255,255,255,255,254,255,195,255,255,
255,255,255,255,255,255,255,255,255,255,239,31,254,225,255,159,0,0,255,255,
255,255,255,255,0,224,255,255,255,255,255,255,255,255,255,255,255,255,3,0,255,
255,255,255,255,7,48,4,255,255,255,252,255,31,0,0,255,255,255,1,0,0,0,0,0,0,0,
0,253,31,0,0,0,0,0,0,240,3,255,127,255,255,255,255,255,255,255,239,255,223,
225,255,207,255,254,254,238,159,249,255,255,253,197,227,159,89,128,176,207,
255,3,0,238,135,249,255,255,253,109,195,135,25,2,94,192,255,63,0,238,191,251,
255,255,253,237,227,191,27,1,0,207,255,0,0,238,159,249,255,255,253,237,227,
159,25,192,176,207,255,2,0,236,199,61,214,24,199,255,195,199,29,129,0,192,255,
0,0,238,223,253,255,255,253,239,227,223,29,96,3,207,255,0,0,236,223,253,255,
255,253,239,227,223,29,96,64,207,255,6,0,236,223,253,255,255,255,255,231,223,
93,128,0,207,255,0,252,236,255,127,252,255,255,251,47,127,128,95,255,0,0,12,0,
254,255,255,255,255,127,255,7,63,32,255,3,0,0,0,0,150,37,240,254,174,236,255,
59,95,32,255,243,0,0,0,0,1,0,0,0,255,3,0,0,255,254,255,255,255,31,254,255,3,
255,255,254,255,255,255,31,0,0,0,0,0,0,0,0,255,255,255,255,255,255,127,249,
255,3,255,255,231,193,255,255,127,64,255,51,255,255,255,255,191,32,255,255,
255,255,255,247,255,255,255,255,255,255,255,255,255,61,127,61,255,255,255,255,
255,61,255,255,255,255,61,127,61,255,127,255,255,255,255,255,255,255,61,255,
255,255,255,255,255,255,255,135,0,0,0,0,255,255,0,0,255,255,255,255,255,255,
255,255,255,255,31,0,254,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,159,255,255,254,255,255,7,255,
255,255,255,255,255,255,255,255,199,1,0,255,223,15,0,255,255,15,0,255,255,15,
0,255,223,13,0,255,255,255,255,255,255,207,255,255,1,128,16,255,3,0,0,0,0,255,
3,255,255,255,255,255,255,255,255,255,255,255,0,255,255,255,255,255,7,255,255,
255,255,255,255,255,255,63,0,255,255,255,31,255,15,255,1,192,255,255,255,255,
63,31,0,255,255,255,255,255,15,255,255,255,3,255,3,0,0,0,0,255,255,255,15,255,
255,255,255,255,255,255,127,254,255,31,0,255,3,255,3,128,0,0,0,0,0,0,0,0,0,0,
0,255,255,255,255,255,255,239,255,239,15,255,3,0,0,0,0,255,255,255,255,255,
243,255,255,255,255,255,255,191,255,3,0,255,255,255,255,255,255,63,0,255,227,
255,255,255,255,255,63,0,0,0,0,0,0,0,0,0,0,0,0,0,222,111,0,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,0,0,0,0,0,0,0,0,255,255,63,63,255,255,255,255,63,63,255,170,255,255,255,
63,255,255,255,255,255,255,223,95,220,31,207,15,255,31,220,31,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,2,128,0,0,255,31,0,0,0,0,0,0,0,0,0,0,0,0,132,252,47,62,80,189,255,
243,224,67,0,0,255,255,255,255,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,255,255,255,255,255,255,3,0,0,255,255,255,
255,255,127,255,255,255,255,255,127,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,31,120,12,0,255,255,255,255,191,32,255,255,255,255,
255,255,255,128,0,0,255,255,127,0,127,127,127,127,127,127,127,127,255,255,255,
255,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,0,0,
0,254,3,62,31,254,255,255,255,255,255,255,255,255,255,127,224,254,255,255,255,
255,255,255,255,255,255,255,247,224,255,255,255,255,63,254,255,255,255,255,
255,255,255,255,255,255,127,0,0,255,255,255,7,0,0,0,0,0,0,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
63,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,31,0,0,0,0,0,0,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,31,0,0,0,0,0,0,0,0,255,
255,255,255,255,63,255,31,255,255,255,15,0,0,255,255,255,255,255,127,240,143,
255,255,255,128,255,255,255,255,255,255,255,255,255,255,0,0,0,0,128,255,252,
255,255,255,255,255,255,255,255,255,255,255,255,121,15,0,255,7,0,0,0,0,0,0,0,
0,0,255,187,247,255,255,255,0,0,0,255,255,255,255,255,255,15,0,255,255,255,
255,255,255,255,255,15,0,255,3,0,0,252,8,255,255,255,255,255,7,255,255,255,
255,7,0,255,255,255,31,255,255,255,255,255,255,247,255,0,128,255,3,0,0,0,0,
255,255,255,255,255,255,127,0,255,63,255,3,255,255,127,4,255,255,255,255,255,
255,255,127,5,0,0,56,255,255,60,0,126,126,126,0,127,127,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,255,255,255,255,255,7,255,3,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,15,0,255,255,127,248,255,255,255,
255,255,15,255,255,255,255,255,255,255,255,255,255,255,255,255,63,255,255,255,
255,255,255,255,255,255,255,255,255,255,3,0,0,0,0,127,0,248,224,255,253,127,
95,219,255,255,255,255,255,255,255,255,255,255,255,255,255,3,0,0,0,248,255,
255,255,255,255,255,255,255,255,255,255,255,63,0,0,255,255,255,255,255,255,
255,255,252,255,255,255,255,255,255,0,0,0,0,0,255,15,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,223,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,31,0,
0,255,3,254,255,255,7,254,255,255,7,192,255,255,255,255,255,255,255,255,255,
255,127,252,252,252,28,0,0,0,0,255,239,255,255,127,255,255,183,255,63,255,63,
0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,7,0,0,0,0,
0,0,0,0,255,255,255,255,255,255,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,31,255,255,255,255,255,255,1,0,0,0,0,0,
255,255,255,127,0,0,255,255,255,7,0,0,0,0,0,0,255,255,255,63,255,255,255,255,
15,255,62,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,63,255,3,0,0,0,0,0,0,0,0,0,0,63,253,255,255,255,255,191,
145,255,255,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,63,0,255,255,
255,3,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,192,0,0,0,0,0,0,0,0,111,240,
239,254,255,255,15,0,0,0,0,0,255,255,255,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,63,0,255,255,63,0,255,255,7,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,255,255,255,255,255,255,255,255,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,255,255,255,255,255,255,255,255,63,0,0,0,192,255,0,0,252,255,255,
255,255,255,255,1,0,0,255,255,255,1,255,3,255,255,255,255,255,255,199,255,0,0,
0,0,0,0,0,0,255,255,255,255,255,255,255,255,30,0,255,3,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,63,0,255,3,0,0,0,0,0,0,255,255,255,
255,255,255,255,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,255,255,255,255,255,255,255,255,255,255,255,7,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,255,255,255,255,255,255,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,255,255,255,255,255,255,255,255,31,0,255,255,255,255,255,127,0,0,
248,255,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,223,255,255,255,255,
255,255,255,255,223,100,222,255,235,239,255,255,255,255,255,255,255,191,231,
223,223,255,255,255,123,95,252,253,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,63,255,255,255,253,255,255,
247,255,255,255,247,255,255,223,255,255,255,223,255,255,127,255,255,255,127,
255,255,255,253,255,255,255,253,255,255,247,207,255,255,255,255,255,255,239,
255,255,255,150,254,247,10,132,234,150,170,150,247,247,94,255,251,255,15,238,
251,255,15,0,0,0,0,0,0,0,0,

/// >>> CONTINUE src/ctype/iswalpha.c
};

int iswalpha(wint_t iswalpha_c__wc) {
    if (iswalpha_c__wc<0x20000U)
        return (iswalpha_c__table[iswalpha_c__table[iswalpha_c__wc>>8]*32+((iswalpha_c__wc&255)>>3)]>>(iswalpha_c__wc&7))&1;
    if (iswalpha_c__wc<0x2fffeU)
        return 1;
    return 0;
}

/// >>> START src/ctype/iswblank.c

int iswblank(wint_t iswblank_c__wc) {
    return isblank(iswblank_c__wc);
}

/// >>> START src/ctype/iswcntrl.c

int iswcntrl(wint_t iswcntrl_c__wc) {
    return (unsigned)iswcntrl_c__wc < 32
        || (unsigned)(iswcntrl_c__wc-0x7f) < 33
        || (unsigned)(iswcntrl_c__wc-0x2028) < 2
        || (unsigned)(iswcntrl_c__wc-0xfff9) < 3;
}

/// >>> START src/ctype/iswctype.c

#undef WCTYPE_ALNUM
#define WCTYPE_ALNUM  1
#undef WCTYPE_ALPHA
#define WCTYPE_ALPHA  2
#undef WCTYPE_BLANK
#define WCTYPE_BLANK  3
#undef WCTYPE_CNTRL
#define WCTYPE_CNTRL  4
#undef WCTYPE_DIGIT
#define WCTYPE_DIGIT  5
#undef WCTYPE_GRAPH
#define WCTYPE_GRAPH  6
#undef WCTYPE_LOWER
#define WCTYPE_LOWER  7
#undef WCTYPE_PRINT
#define WCTYPE_PRINT  8
#undef WCTYPE_PUNCT
#define WCTYPE_PUNCT  9
#undef WCTYPE_SPACE
#define WCTYPE_SPACE  10
#undef WCTYPE_UPPER
#define WCTYPE_UPPER  11
#undef WCTYPE_XDIGIT
#define WCTYPE_XDIGIT 12

int iswctype(wint_t iswctype_c__wc, wctype_t type) {
    switch (type) {
    case WCTYPE_ALNUM:
        return iswalnum(iswctype_c__wc);
    case WCTYPE_ALPHA:
        return iswalpha(iswctype_c__wc);
    case WCTYPE_BLANK:
        return iswblank(iswctype_c__wc);
    case WCTYPE_CNTRL:
        return iswcntrl(iswctype_c__wc);
    case WCTYPE_DIGIT:
        return iswdigit(iswctype_c__wc);
    case WCTYPE_GRAPH:
        return iswgraph(iswctype_c__wc);
    case WCTYPE_LOWER:
        return iswlower(iswctype_c__wc);
    case WCTYPE_PRINT:
        return iswprint(iswctype_c__wc);
    case WCTYPE_PUNCT:
        return iswpunct(iswctype_c__wc);
    case WCTYPE_SPACE:
        return iswspace(iswctype_c__wc);
    case WCTYPE_UPPER:
        return iswupper(iswctype_c__wc);
    case WCTYPE_XDIGIT:
        return iswxdigit(iswctype_c__wc);
    }
    return 0;
}

wctype_t wctype(const char *s) {
    int i;
    const char *p;
    /* order must match! */
    static const char names[] =
        "alnum\0" "alpha\0" "blank\0"
        "cntrl\0" "digit\0" "graph\0"
        "lower\0" "print\0" "punct\0"
        "space\0" "upper\0" "xdigit";
    for (i=1, p=names; *p; i++, p+=6)
        if (*s == *p && !strcmp(s, p))
            return i;
    return 0;
}

/// >>> START src/ctype/iswdigit.c

#undef iswdigit

int iswdigit(wint_t iswdigit_c__wc) {
    return (unsigned)iswdigit_c__wc-'0' < 10;
}

/// >>> START src/ctype/iswgraph.c

int iswgraph(wint_t iswgraph_c__wc) {
    /* ISO C defines this function as: */
    return !iswspace(iswgraph_c__wc) && iswprint(iswgraph_c__wc);
}

/// >>> START src/ctype/iswlower.c

int iswlower(wint_t iswlower_c__wc) {
    return towupper(iswlower_c__wc) != iswlower_c__wc || iswlower_c__wc == 0xdf;
}

/// >>> START src/ctype/iswprint.c

/* Consider all legal codepoints as printable except for:
 * - iswprint_c__C0 and iswprint_c__C1 control characters
 * - U+2028 and U+2029 (line/para break)
 * - U+FFF9 through U+FFFB (interlinear annotation controls)
 * The following code is optimized heavily to make hot paths for the
 * expected printable characters. */

int iswprint(wint_t iswprint_c__wc) {
    if (iswprint_c__wc < 0xffU)
        return (iswprint_c__wc+1 & 0x7f) >= 0x21;
    if (iswprint_c__wc < 0x2028U || iswprint_c__wc-0x202aU < 0xd800-0x202a || iswprint_c__wc-0xe000U < 0xfff9-0xe000)
        return 1;
    if (iswprint_c__wc-0xfffcU > 0x10ffff-0xfffc || (iswprint_c__wc&0xfffe)==0xfffe)
        return 0;
    return 1;
}

/// >>> START src/ctype/iswpunct.c

static const unsigned char iswpunct_c__table[] = {
/// >>> START src/ctype/punct.h
18,16,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,16,16,34,35,16,36,37,38,39,
40,41,42,43,16,44,45,46,17,47,48,17,17,49,17,17,17,50,51,52,53,54,55,56,57,17,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,58,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,59,16,60,61,62,63,64,65,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,66,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,67,16,16,68,16,69,70,71,16,72,16,73,
16,16,16,16,74,75,76,77,16,16,78,16,79,80,16,16,16,16,81,16,16,16,16,16,16,16,
16,16,16,16,16,16,82,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,83,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,84,85,86,87,
16,16,88,89,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
90,16,91,92,93,94,95,96,97,98,16,16,16,16,16,16,16,16,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,0,0,0,0,254,255,0,252,1,0,0,248,1,0,0,120,0,0,0,0,255,251,223,
251,0,0,128,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,
252,255,224,175,255,255,255,255,255,255,255,255,255,255,223,255,255,255,255,
255,32,64,176,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
252,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,0,0,0,0,0,134,254,
255,255,255,0,64,73,0,0,0,0,0,24,0,223,255,0,200,0,0,0,0,0,0,0,1,0,60,0,0,0,0,
0,0,0,0,0,0,0,0,16,224,1,30,0,96,255,191,0,0,0,0,0,0,255,7,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,248,207,3,0,0,0,3,0,32,255,127,0,0,0,78,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,252,0,0,0,0,0,0,0,0,0,16,0,32,30,0,48,0,1,0,0,0,0,0,0,0,0,16,
0,32,0,0,0,0,252,15,0,0,0,0,0,0,0,16,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,32,0,
0,0,0,3,0,0,0,0,0,0,0,0,16,0,32,0,0,0,0,253,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,
255,7,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,255,0,0,0,0,0,0,0,16,0,32,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,32,0,0,0,0,63,2,0,0,0,0,0,0,0,0,0,4,0,0,0,0,16,0,0,0,0,0,0,
128,0,128,192,223,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,31,0,0,0,0,0,0,254,255,255,
255,0,252,255,255,0,0,0,0,0,0,0,0,252,0,0,0,0,0,0,192,255,223,255,7,0,0,0,0,0,
0,0,0,0,0,128,6,0,252,0,0,24,62,0,0,128,191,0,204,0,0,0,0,0,0,0,0,0,0,0,8,0,0,
0,0,0,0,0,0,0,0,0,96,255,255,255,31,0,0,255,3,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,96,0,0,1,0,0,24,0,0,0,0,0,0,0,0,0,56,0,0,0,0,16,0,0,0,112,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,48,0,0,254,127,47,0,0,255,3,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,49,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,196,255,255,255,255,0,0,0,192,0,0,0,0,0,0,0,0,1,0,224,159,0,0,0,0,
127,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,16,0,0,252,255,255,255,31,0,0,0,0,
0,12,0,0,0,0,0,0,64,0,12,240,0,0,0,0,0,0,192,248,0,0,0,0,0,0,0,192,0,0,0,0,0,
0,0,0,255,0,255,255,255,33,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,255,255,255,127,0,0,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
160,3,224,0,224,0,224,0,96,128,248,255,255,255,252,255,255,255,255,255,127,31,
252,241,127,255,127,0,0,255,255,255,3,0,0,255,255,255,255,1,0,123,3,208,193,
175,66,0,12,31,188,255,255,0,0,0,0,0,2,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,15,0,255,255,
255,255,127,0,0,0,255,7,0,0,255,255,255,255,255,255,255,255,255,255,63,0,0,0,
0,0,0,252,255,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,31,255,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,135,3,254,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,1,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,
127,255,15,0,0,0,0,0,0,0,0,255,255,255,251,255,255,255,255,255,255,255,255,
255,255,15,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,63,0,0,0,255,15,30,255,255,255,1,252,
193,224,0,0,0,0,0,0,0,0,0,0,0,30,1,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,255,255,0,0,0,0,255,255,255,255,15,0,0,0,255,255,255,127,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
255,255,255,255,127,0,0,0,0,0,0,192,0,224,0,0,0,0,0,0,0,0,0,0,0,128,15,112,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,255,255,127,0,3,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,68,8,0,0,0,15,255,3,0,0,0,0,0,0,240,0,0,0,0,0,0,0,0,0,
16,192,0,0,255,255,3,7,0,0,0,0,0,248,0,0,0,0,8,128,0,0,0,0,0,0,0,0,0,0,8,0,
255,63,0,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,240,0,0,128,11,0,0,0,0,0,0,0,128,2,
0,0,192,0,0,67,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,
0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,2,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,252,255,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,255,255,255,3,127,0,255,255,255,255,247,
255,127,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,254,255,0,252,1,0,0,248,1,0,
0,248,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,127,0,48,135,255,255,255,255,255,
143,255,0,0,0,0,0,0,224,255,255,7,255,15,0,0,0,0,0,0,255,255,255,255,255,63,0,
0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,128,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,143,0,0,0,128,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,135,255,0,255,1,
0,0,0,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,255,0,0,0,
255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,127,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,63,252,255,63,0,0,0,3,0,0,0,
0,0,0,254,3,0,0,0,0,0,0,0,0,0,0,0,0,0,24,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
225,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,7,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,63,0,255,255,255,255,127,254,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,63,0,0,0,0,
255,255,255,255,255,255,255,255,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,255,255,255,255,255,255,255,255,255,255,127,0,255,255,3,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,8,0,0,0,
8,0,0,32,0,0,0,32,0,0,128,0,0,0,128,0,0,0,2,0,0,0,2,0,0,8,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,255,255,255,255,255,
15,255,255,255,255,255,255,255,255,255,255,255,255,15,0,255,127,254,127,254,
255,254,255,0,0,0,0,255,7,255,255,255,127,255,255,255,255,255,255,255,15,255,
255,255,255,255,7,0,0,0,0,0,0,0,0,192,255,255,255,7,0,255,255,255,255,255,7,
255,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,1,0,191,255,
255,255,255,255,255,255,255,31,255,255,15,0,255,255,255,255,223,7,0,0,255,255,
1,0,255,255,255,255,255,255,255,127,253,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,30,255,255,255,255,255,
255,255,63,15,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,255,255,
255,255,255,255,255,255,225,255,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
63,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,15,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

/// >>> CONTINUE src/ctype/iswpunct.c
};

int iswpunct(wint_t iswpunct_c__wc) {
    if (iswpunct_c__wc<0x20000U)
        return (iswpunct_c__table[iswpunct_c__table[iswpunct_c__wc>>8]*32+((iswpunct_c__wc&255)>>3)]>>(iswpunct_c__wc&7))&1;
    return 0;
}

/// >>> START src/ctype/iswspace.c

/* Our definition of whitespace is the Unicode White_Space property,
 * minus non-breaking spaces (U+00A0, U+2007, and U+202F) and script-
 * specific characters with non-blank glyphs (U+1680 and U+180E). */

int iswspace(wint_t iswspace_c__wc) {
    static const wchar_t spaces[] = {
        ' ', '\t', '\n', '\r', 11, 12,  0x0085,
        0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
        0x2006, 0x2008, 0x2009, 0x200a,
        0x2028, 0x2029, 0x205f, 0x3000, 0
    };
    return iswspace_c__wc && wcschr(spaces, iswspace_c__wc);
}

/// >>> START src/ctype/iswupper.c

int iswupper(wint_t iswupper_c__wc) {
    return towlower(iswupper_c__wc) != iswupper_c__wc;
}

/// >>> START src/ctype/iswxdigit.c

int iswxdigit(wint_t iswxdigit_c__wc) {
    return (unsigned)(iswxdigit_c__wc-'0') < 10 || (unsigned)((iswxdigit_c__wc|32)-'a') < 6;
}

/// >>> START src/ctype/isxdigit.c

int isxdigit(int c) {
    return isdigit(c) || ((unsigned)c|32)-'a' < 6;
}

/// >>> START src/ctype/toascii.c

/* nonsense function that should NEVER be used! */
int toascii(int c) {
    return c & 0x7f;
}

/// >>> START src/ctype/tolower.c

int tolower(int c) {
    if (isupper(c)) return c | 32;
    return c;
}

/// >>> START src/ctype/toupper.c

int toupper(int c) {
    if (islower(c)) return c & 0x5f;
    return c;
}

/// >>> START src/ctype/towctrans.c

#undef CASEMAP
#define CASEMAP(towctrans_c__u1,towctrans_c__u2,l) { (towctrans_c__u1), (l)-(towctrans_c__u1), (towctrans_c__u2)-(towctrans_c__u1)+1 }
#undef CASELACE
#define CASELACE(towctrans_c__u1,towctrans_c__u2) CASEMAP((towctrans_c__u1),(towctrans_c__u2),(towctrans_c__u1)+1)

static const struct {
    unsigned short upper;
    signed char lower;
    unsigned char len;
} casemaps[] = {
    CASEMAP('A','Z','a'),
    CASEMAP(0xc0,0xde,0xe0),

    CASELACE(0x0100,0x012e),
    CASELACE(0x0132,0x0136),
    CASELACE(0x0139,0x0147),
    CASELACE(0x014a,0x0176),
    CASELACE(0x0179,0x017d),

    CASELACE(0x370,0x372),
    CASEMAP(0x391,0x3a1,0x3b1),
    CASEMAP(0x3a3,0x3ab,0x3c3),
    CASEMAP(0x400,0x40f,0x450),
    CASEMAP(0x410,0x42f,0x430),

    CASELACE(0x460,0x480),
    CASELACE(0x48a,0x4be),
    CASELACE(0x4c1,0x4cd),
    CASELACE(0x4d0,0x50e),

    CASELACE(0x514,0x526),
    CASEMAP(0x531,0x556,0x561),

    CASELACE(0x01a0,0x01a4),
    CASELACE(0x01b3,0x01b5),
    CASELACE(0x01cd,0x01db),
    CASELACE(0x01de,0x01ee),
    CASELACE(0x01f8,0x021e),
    CASELACE(0x0222,0x0232),
    CASELACE(0x03d8,0x03ee),

    CASELACE(0x1e00,0x1e94),
    CASELACE(0x1ea0,0x1efe),

    CASEMAP(0x1f08,0x1f0f,0x1f00),
    CASEMAP(0x1f18,0x1f1d,0x1f10),
    CASEMAP(0x1f28,0x1f2f,0x1f20),
    CASEMAP(0x1f38,0x1f3f,0x1f30),
    CASEMAP(0x1f48,0x1f4d,0x1f40),

    CASEMAP(0x1f68,0x1f6f,0x1f60),
    CASEMAP(0x1f88,0x1f8f,0x1f80),
    CASEMAP(0x1f98,0x1f9f,0x1f90),
    CASEMAP(0x1fa8,0x1faf,0x1fa0),
    CASEMAP(0x1fb8,0x1fb9,0x1fb0),
    CASEMAP(0x1fba,0x1fbb,0x1f70),
    CASEMAP(0x1fc8,0x1fcb,0x1f72),
    CASEMAP(0x1fd8,0x1fd9,0x1fd0),
    CASEMAP(0x1fda,0x1fdb,0x1f76),
    CASEMAP(0x1fe8,0x1fe9,0x1fe0),
    CASEMAP(0x1fea,0x1feb,0x1f7a),
    CASEMAP(0x1ff8,0x1ff9,0x1f78),
    CASEMAP(0x1ffa,0x1ffb,0x1f7c),

    CASELACE(0x246,0x24e),
    CASELACE(0x510,0x512),
    CASEMAP(0x2160,0x216f,0x2170),
    CASEMAP(0x2c00,0x2c2e,0x2c30),
    CASELACE(0x2c67,0x2c6b),
    CASELACE(0x2c80,0x2ce2),
    CASELACE(0x2ceb,0x2ced),

    CASELACE(0xa640,0xa66c),
    CASELACE(0xa680,0xa696),

    CASELACE(0xa722,0xa72e),
    CASELACE(0xa732,0xa76e),
    CASELACE(0xa779,0xa77b),
    CASELACE(0xa77e,0xa786),

    CASELACE(0xa790,0xa792),
    CASELACE(0xa7a0,0xa7a8),

    CASEMAP(0xff21,0xff3a,0xff41),
    { 0,0,0 }
};

static const unsigned short pairs[][2] = {
    { 'I',    0x0131 },
    { 'S',    0x017f },
    { 0x0130, 'i'    },
    { 0x0178, 0x00ff },
    { 0x0181, 0x0253 },
    { 0x0182, 0x0183 },
    { 0x0184, 0x0185 },
    { 0x0186, 0x0254 },
    { 0x0187, 0x0188 },
    { 0x0189, 0x0256 },
    { 0x018a, 0x0257 },
    { 0x018b, 0x018c },
    { 0x018e, 0x01dd },
    { 0x018f, 0x0259 },
    { 0x0190, 0x025b },
    { 0x0191, 0x0192 },
    { 0x0193, 0x0260 },
    { 0x0194, 0x0263 },
    { 0x0196, 0x0269 },
    { 0x0197, 0x0268 },
    { 0x0198, 0x0199 },
    { 0x019c, 0x026f },
    { 0x019d, 0x0272 },
    { 0x019f, 0x0275 },
    { 0x01a6, 0x0280 },
    { 0x01a7, 0x01a8 },
    { 0x01a9, 0x0283 },
    { 0x01ac, 0x01ad },
    { 0x01ae, 0x0288 },
    { 0x01af, 0x01b0 },
    { 0x01b1, 0x028a },
    { 0x01b2, 0x028b },
    { 0x01b7, 0x0292 },
    { 0x01b8, 0x01b9 },
    { 0x01bc, 0x01bd },
    { 0x01c4, 0x01c6 },
    { 0x01c4, 0x01c5 },
    { 0x01c5, 0x01c6 },
    { 0x01c7, 0x01c9 },
    { 0x01c7, 0x01c8 },
    { 0x01c8, 0x01c9 },
    { 0x01ca, 0x01cc },
    { 0x01ca, 0x01cb },
    { 0x01cb, 0x01cc },
    { 0x01f1, 0x01f3 },
    { 0x01f1, 0x01f2 },
    { 0x01f2, 0x01f3 },
    { 0x01f4, 0x01f5 },
    { 0x01f6, 0x0195 },
    { 0x01f7, 0x01bf },
    { 0x0220, 0x019e },
    { 0x0386, 0x03ac },
    { 0x0388, 0x03ad },
    { 0x0389, 0x03ae },
    { 0x038a, 0x03af },
    { 0x038c, 0x03cc },
    { 0x038e, 0x03cd },
    { 0x038f, 0x03ce },
    { 0x0399, 0x0345 },
    { 0x0399, 0x1fbe },
    { 0x03a3, 0x03c2 },
    { 0x03f7, 0x03f8 },
    { 0x03fa, 0x03fb },
    { 0x1e60, 0x1e9b },
    { 0xdf, 0xdf },
    { 0x1e9e, 0xdf },

    { 0x1f59, 0x1f51 },
    { 0x1f5b, 0x1f53 },
    { 0x1f5d, 0x1f55 },
    { 0x1f5f, 0x1f57 },
    { 0x1fbc, 0x1fb3 },
    { 0x1fcc, 0x1fc3 },
    { 0x1fec, 0x1fe5 },
    { 0x1ffc, 0x1ff3 },

    { 0x23a, 0x2c65 },
    { 0x23b, 0x23c },
    { 0x23d, 0x19a },
    { 0x23e, 0x2c66 },
    { 0x241, 0x242 },
    { 0x243, 0x180 },
    { 0x244, 0x289 },
    { 0x245, 0x28c },
    { 0x3f4, 0x3b8 },
    { 0x3f9, 0x3f2 },
    { 0x3fd, 0x37b },
    { 0x3fe, 0x37c },
    { 0x3ff, 0x37d },
    { 0x4c0, 0x4cf },

    { 0x2126, 0x3c9 },
    { 0x212a, 'k' },
    { 0x212b, 0xe5 },
    { 0x2132, 0x214e },
    { 0x2183, 0x2184 },
    { 0x2c60, 0x2c61 },
    { 0x2c62, 0x26b },
    { 0x2c63, 0x1d7d },
    { 0x2c64, 0x27d },
    { 0x2c6d, 0x251 },
    { 0x2c6e, 0x271 },
    { 0x2c6f, 0x250 },
    { 0x2c70, 0x252 },
    { 0x2c72, 0x2c73 },
    { 0x2c75, 0x2c76 },
    { 0x2c7e, 0x23f },
    { 0x2c7f, 0x240 },
    { 0x2cf2, 0x2cf3 },

    { 0xa77d, 0x1d79 },
    { 0xa78b, 0xa78c },
    { 0xa78d, 0x265 },
    { 0xa7aa, 0x266 },

    { 0x10c7, 0x2d27 },
    { 0x10cd, 0x2d2d },

    /* bogus greek 'symbol' letters */
    { 0x376, 0x377 },
    { 0x39c, 0xb5 },
    { 0x392, 0x3d0 },
    { 0x398, 0x3d1 },
    { 0x3a6, 0x3d5 },
    { 0x3a0, 0x3d6 },
    { 0x39a, 0x3f0 },
    { 0x3a1, 0x3f1 },
    { 0x395, 0x3f5 },
    { 0x3cf, 0x3d7 },

    { 0,0 }
};

static wchar_t __towcase(wchar_t towctrans_c__wc, int lower) {
    int i;
    int lmul = 2*lower-1;
    int lmask = lower-1;
    /* no letters with case in these large ranges */
    if (!iswalpha(towctrans_c__wc)
     || (unsigned)towctrans_c__wc - 0x0600 <= 0x0fff-0x0600
     || (unsigned)towctrans_c__wc - 0x2e00 <= 0xa63f-0x2e00
     || (unsigned)towctrans_c__wc - 0xa800 <= 0xfeff-0xa800)
        return towctrans_c__wc;
    /* special case because the diff between upper/lower is too big */
    if (lower && (unsigned)towctrans_c__wc - 0x10a0 < 0x2e)
        if (towctrans_c__wc>0x10c5 && towctrans_c__wc != 0x10c7 && towctrans_c__wc != 0x10cd) return towctrans_c__wc;
        else return towctrans_c__wc + 0x2d00 - 0x10a0;
    if (!lower && (unsigned)towctrans_c__wc - 0x2d00 < 0x26)
        if (towctrans_c__wc>0x2d25 && towctrans_c__wc != 0x2d27 && towctrans_c__wc != 0x2d2d) return towctrans_c__wc;
        else return towctrans_c__wc + 0x10a0 - 0x2d00;
    for (i=0; casemaps[i].len; i++) {
        int base = casemaps[i].upper + (lmask & casemaps[i].lower);
        if ((unsigned)towctrans_c__wc-base < casemaps[i].len) {
            if (casemaps[i].lower == 1)
                return towctrans_c__wc + lower - ((towctrans_c__wc-casemaps[i].upper)&1);
            return towctrans_c__wc + lmul*casemaps[i].lower;
        }
    }
    for (i=0; pairs[i][1-lower]; i++) {
        if (pairs[i][1-lower] == towctrans_c__wc)
            return pairs[i][lower];
    }
    if ((unsigned)towctrans_c__wc - (0x10428 - 0x28*lower) < 0x28)
        return towctrans_c__wc - 0x28 + 0x50*lower;
    return towctrans_c__wc;
}

wint_t towupper(wint_t towctrans_c__wc) {
    return __towcase(towctrans_c__wc, 0);
}

wint_t towlower(wint_t towctrans_c__wc) {
    return __towcase(towctrans_c__wc, 1);
}

/// >>> START src/ctype/wcswidth.c

int wcswidth(const wchar_t *wcs, size_t n) {
    int l=0, wcswidth_c__k=0;
    for (; n-- && *wcs && (wcswidth_c__k = wcwidth(*wcs)) >= 0; l+=wcswidth_c__k, wcs++);
    return (wcswidth_c__k < 0) ? wcswidth_c__k : l;
}

/// >>> START src/ctype/wctrans.c

wctrans_t wctrans(const char *class) {
    if (!strcmp(class, "toupper")) return (wctrans_t)1;
    if (!strcmp(class, "tolower")) return (wctrans_t)2;
    return 0;
}

wint_t towctrans(wint_t wctrans_c__wc, wctrans_t trans) {
    if (trans == (wctrans_t)1) return towupper(wctrans_c__wc);
    if (trans == (wctrans_t)2) return towlower(wctrans_c__wc);
    return wctrans_c__wc;
}

/// >>> START src/ctype/wcwidth.c

static const unsigned char wcwidth_c__table[] = {
/// >>> START src/ctype/nonspacing.h
16,16,16,18,19,20,21,22,23,24,25,26,27,28,29,30,31,16,16,32,16,16,16,33,34,35,
36,37,38,39,16,16,40,16,16,16,16,16,16,16,16,16,16,16,41,42,16,16,43,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,44,16,45,46,47,48,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,49,16,16,50,51,16,52,16,16,
16,16,16,16,16,16,53,16,16,16,16,16,54,55,16,16,16,16,56,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,57,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,58,59,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,3,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,255,255,255,255,191,
182,0,0,0,0,0,0,0,31,0,255,7,0,0,0,0,0,248,255,255,0,0,1,0,0,0,0,0,0,0,0,0,0,
0,192,191,159,61,0,0,0,128,2,0,0,0,255,255,255,7,0,0,0,0,0,0,0,0,0,0,192,255,
1,0,0,0,0,0,0,248,15,0,0,0,192,251,239,62,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,240,255,255,127,7,0,0,0,0,0,0,20,254,33,254,0,12,0,0,0,2,0,0,0,0,0,
0,16,30,32,0,0,12,0,0,0,6,0,0,0,0,0,0,16,134,57,2,0,0,0,35,0,6,0,0,0,0,0,0,16,
190,33,0,0,12,0,0,0,2,0,0,0,0,0,0,144,30,32,64,0,12,0,0,0,4,0,0,0,0,0,0,0,1,
32,0,0,0,0,0,0,0,0,0,0,0,0,0,192,193,61,96,0,12,0,0,0,0,0,0,0,0,0,0,144,64,48,
0,0,12,0,0,0,0,0,0,0,0,0,0,0,30,32,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,4,92,0,0,0,
0,0,0,0,0,0,0,0,242,7,128,127,0,0,0,0,0,0,0,0,0,0,0,0,242,27,0,63,0,0,0,0,0,0,
0,0,0,3,0,0,160,2,0,0,0,0,0,0,254,127,223,224,255,254,255,255,255,31,64,0,0,0,
0,0,0,0,0,0,0,0,0,224,253,102,0,0,0,195,1,0,30,0,100,32,0,32,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
28,0,0,0,28,0,0,0,12,0,0,0,12,0,0,0,0,0,0,0,176,63,64,254,15,32,0,0,0,0,0,56,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,135,1,4,
14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,1,0,0,0,0,0,0,64,
127,229,31,248,159,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,208,23,4,0,0,
0,0,248,15,0,3,0,0,0,60,11,0,0,0,0,0,0,64,163,3,0,0,0,0,0,0,240,207,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,255,253,33,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,255,255,255,255,127,0,0,240,0,248,0,0,0,124,0,0,0,0,0,0,31,
252,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,
0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,247,63,0,0,0,128,0,0,0,0,0,
0,0,0,0,0,3,0,68,8,0,0,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,255,
255,3,0,0,0,0,0,192,63,0,0,128,255,3,0,0,0,0,0,7,0,0,0,0,0,200,19,0,0,0,0,0,0,
0,0,0,0,0,0,0,126,102,0,8,16,0,0,0,0,0,0,0,0,0,0,0,0,157,193,2,0,0,0,0,48,64,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,0,0,0,0,0,64,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,127,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,32,110,240,0,0,0,0,0,135,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,255,127,0,0,0,0,0,0,0,3,0,0,0,0,0,120,38,0,0,
0,0,0,0,0,0,7,0,0,0,128,239,31,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,192,127,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,191,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,128,3,248,255,231,15,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

/// >>> CONTINUE src/ctype/wcwidth.c
};

static const unsigned char wtable[] = {
/// >>> START src/ctype/wide.h
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,18,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,19,16,16,16,16,16,16,16,16,16,16,20,21,22,23,24,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,25,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,26,16,16,16,16,27,16,16,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,28,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,16,16,16,29,30,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,31,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
16,16,16,16,32,16,16,16,16,16,16,16,16,16,16,16,16,16,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,
0,248,0,0,0,0,0,0,0,0,0,0,252,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,251,255,255,255,
255,255,255,255,255,255,255,15,0,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,63,0,0,0,255,15,
255,255,255,255,255,255,255,127,254,255,255,255,255,255,255,255,255,255,127,
254,255,255,255,255,255,255,255,255,255,255,255,255,224,255,255,255,255,63,
254,255,255,255,255,255,255,255,255,255,255,127,255,255,255,255,255,7,255,255,
255,255,15,0,255,255,255,255,255,127,255,255,255,255,255,0,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,127,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,31,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,255,255,255,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,15,0,255,
255,127,248,255,255,255,255,255,15,0,0,255,3,0,0,255,255,255,255,247,255,127,
15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,255,255,255,255,255,255,255,255,
255,255,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,255,255,255,255,255,7,255,1,3,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

/// >>> CONTINUE src/ctype/wcwidth.c
};

int wcwidth(wchar_t wcwidth_c__wc) {
    if (wcwidth_c__wc < 0xffU)
        return (wcwidth_c__wc+1 & 0x7f) >= 0x21 ? 1 : wcwidth_c__wc ? -1 : 0;
    if ((wcwidth_c__wc & 0xfffeffffU) < 0xfffe) {
        if ((wcwidth_c__table[wcwidth_c__table[wcwidth_c__wc>>8]*32+((wcwidth_c__wc&255)>>3)]>>(wcwidth_c__wc&7))&1)
            return 0;
        if ((wtable[wtable[wcwidth_c__wc>>8]*32+((wcwidth_c__wc&255)>>3)]>>(wcwidth_c__wc&7))&1)
            return 2;
        return 1;
    }
    if ((wcwidth_c__wc & 0xfffe) == 0xfffe)
        return -1;
    if (wcwidth_c__wc-0x20000U < 0x20000)
        return 2;
    if (wcwidth_c__wc == 0xe0001 || wcwidth_c__wc-0xe0020U < 0x5f || wcwidth_c__wc-0xe0100 < 0xef)
        return 0;
    return 1;
}

/// >>> START src/dirent/__getdents.c

int __getdents(int fd, struct dirent *buf, size_t len) {
    return syscall(SYS_getdents, fd, buf, len);
}

/// >>> START src/dirent/alphasort.c

int alphasort(const struct dirent **a, const struct dirent **b) {
    return strcoll((*a)->d_name, (*b)->d_name);
}

/// >>> START src/dirent/closedir.c
/// >>> START src/dirent/__dirent.h
struct __dirstream {
    int fd;
    off_t tell;
    int buf_pos;
    int buf_end;
    int lock[2];
    char buf[2048];
};

/// >>> CONTINUE src/dirent/closedir.c

int closedir(DIR *dir) {
    int ret = close(dir->fd);
    free(dir);
    return ret;
}

/// >>> START src/dirent/dirfd.c

int dirfd(DIR *d) {
    return d->fd;
}

/// >>> START src/dirent/fdopendir.c

DIR *fdopendir(int fd) {
    DIR *dir;
    struct stat fdopendir_c__st;

    if (fstat(fd, &fdopendir_c__st) < 0) {
        return 0;
    }
    if (!S_ISDIR(fdopendir_c__st.st_mode)) {
        errno = ENOTDIR;
        return 0;
    }
    if (!(dir = calloc(1, sizeof *dir))) {
        return 0;
    }

    fcntl(fd, F_SETFD, FD_CLOEXEC);
    dir->fd = fd;
    return dir;
}

/// >>> START src/dirent/opendir.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

DIR *opendir(const char *name) {
    int fd;
    DIR *dir;

    if ((fd = open(name, O_RDONLY|O_DIRECTORY|O_CLOEXEC)) < 0)
        return 0;
    if (!(dir = calloc(1, sizeof *dir))) {
        __syscall(SYS_close, fd);
        return 0;
    }
    dir->fd = fd;
    return dir;
}

/// >>> START src/dirent/readdir.c

int __getdents(int, struct dirent *, size_t);

struct dirent *readdir(DIR *dir) {
    struct dirent *de;
    
    if (dir->buf_pos >= dir->buf_end) {
        int len = __getdents(dir->fd, (void *)dir->buf, sizeof dir->buf);
        if (len <= 0) return 0;
        dir->buf_end = len;
        dir->buf_pos = 0;
    }
    de = (void *)(dir->buf + dir->buf_pos);
    dir->buf_pos += de->d_reclen;
    dir->tell = de->d_off;
    return de;
}

/// >>> START src/dirent/readdir_r.c

int readdir_r(DIR *restrict dir, struct dirent *restrict buf, struct dirent **restrict result) {
    struct dirent *de;
    int errno_save = errno;
    int ret;
    
    LOCK(dir->lock);
    errno = 0;
    de = readdir(dir);
    if ((ret = errno)) {
        UNLOCK(dir->lock);
        return ret;
    }
    errno = errno_save;
    if (de) memcpy(buf, de, de->d_reclen);
    else buf = NULL;

    UNLOCK(dir->lock);
    *result = buf;
    return 0;
}

/// >>> START src/dirent/rewinddir.c

void rewinddir(DIR *dir) {
    LOCK(dir->lock);
    lseek(dir->fd, 0, SEEK_SET);
    dir->buf_pos = dir->buf_end = 0;
    dir->tell = 0;
    UNLOCK(dir->lock);
}

/// >>> START src/dirent/scandir.c

int scandir(const char *path, struct dirent ***res,
    int (*sel)(const struct dirent *),
    int (*cmp)(const struct dirent **, const struct dirent **)) {
    DIR *d = opendir(path);
    struct dirent *de, **names=0, **tmp;
    size_t cnt=0, len=0;
    int old_errno = errno;

    if (!d) return -1;

    while ((errno=0), (de = readdir(d))) {
        if (sel && !sel(de)) continue;
        if (cnt >= len) {
            len = 2*len+1;
            if (len > SIZE_MAX/sizeof *names) break;
            tmp = realloc(names, len * sizeof *names);
            if (!tmp) break;
            names = tmp;
        }
        names[cnt] = malloc(de->d_reclen);
        if (!names[cnt]) break;
        memcpy(names[cnt++], de, de->d_reclen);
    }

    closedir(d);

    if (errno) {
        if (names) while (cnt-->0) free(names[cnt]);
        free(names);
        return -1;
    }
    errno = old_errno;

    if (cmp) qsort(names, cnt, sizeof *names, (int (*)(const void *, const void *))cmp);
    *res = names;
    return cnt;
}

/// >>> START src/dirent/seekdir.c

void seekdir(DIR *dir, long off) {
    LOCK(dir->lock);
    dir->tell = lseek(dir->fd, off, SEEK_SET);
    dir->buf_pos = dir->buf_end = 0;
    UNLOCK(dir->lock);
}

/// >>> START src/dirent/telldir.c

long telldir(DIR *dir) {
    return dir->tell;
}

/// >>> START src/dirent/versionsort.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int versionsort(const struct dirent **a, const struct dirent **b) {
    return strverscmp((*a)->d_name, (*b)->d_name);
}

/// >>> START src/fcntl/creat.c

int creat(const char *filename, mode_t mode) {
    return open(filename, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

/// >>> START src/fcntl/fcntl.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int fcntl(int fd, int cmd, ...) {
    long arg;
    va_list ap;
    va_start(ap, cmd);
    arg = va_arg(ap, long);
    va_end(ap);
    if (cmd == F_SETFL) arg |= O_LARGEFILE;
    if (cmd == F_SETLKW) return syscall_cp(SYS_fcntl, fd, cmd, (void *)arg);
    if (cmd == F_GETOWN) {
        struct f_owner_ex ex;
        int ret = __syscall(SYS_fcntl, fd, F_GETOWN_EX, &ex);
        if (ret == -EINVAL) return __syscall(SYS_fcntl, fd, cmd, (void *)arg);
        if (ret) return __syscall_ret(ret);
        return ex.type == F_OWNER_PGRP ? -ex.pid : ex.pid;
    }
    if (cmd == F_DUPFD_CLOEXEC) {
        int ret = __syscall(SYS_fcntl, fd, F_DUPFD_CLOEXEC, arg);
        if (ret != -EINVAL) {
            if (ret >= 0)
                __syscall(SYS_fcntl, ret, F_SETFD, FD_CLOEXEC);
            return __syscall_ret(ret);
        }
        ret = __syscall(SYS_fcntl, fd, F_DUPFD_CLOEXEC, 0);
        if (ret != -EINVAL) {
            if (ret >= 0) __syscall(SYS_close, ret);
            return __syscall_ret(-EINVAL);
        }
        ret = __syscall(SYS_fcntl, fd, F_DUPFD, arg);
        if (ret >= 0) __syscall(SYS_fcntl, ret, F_SETFD, FD_CLOEXEC);
        return __syscall_ret(ret);
    }
    switch (cmd) {
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK:
    case F_GETOWN_EX:
    case F_SETOWN_EX:
        return syscall(SYS_fcntl, fd, cmd, (void *)arg);
    default:
        return syscall(SYS_fcntl, fd, cmd, arg);
    }
}

/// >>> START src/fcntl/open.c

int open(const char *filename, int flags, ...) {
    mode_t mode;
    va_list ap;
    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);
    return syscall_cp(SYS_open, filename, flags|O_LARGEFILE|O_CLOEXEC, mode);
}

/// >>> START src/fcntl/openat.c

int openat(int fd, const char *filename, int flags, ...) {
    mode_t mode;
    va_list ap;
    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);
    return syscall_cp(SYS_openat, fd, filename, flags|O_LARGEFILE|O_CLOEXEC, mode);
}

/// >>> START src/fcntl/posix_fadvise.c

int posix_fadvise(int fd, off_t base, off_t len, int advice) {
    return -(__syscall)(SYS_fadvise, fd, __SYSCALL_LL_O(base),
        __SYSCALL_LL_E(len), advice);
}

/// >>> START src/fcntl/posix_fallocate.c

int posix_fallocate(int fd, off_t base, off_t len) {
    return -__syscall(SYS_fallocate, fd, 0, __SYSCALL_LL_E(base),
        __SYSCALL_LL_E(len));
}

/// >>> START src/fenv/fegetexceptflag.c

int fegetexceptflag(fexcept_t *fp, int mask) {
    *fp = fetestexcept(mask);
    return 0;
}

/// >>> START src/fenv/feholdexcept.c

int feholdexcept(fenv_t *envp) {
    fegetenv(envp);
    feclearexcept(FE_ALL_EXCEPT);
    return 0;
}

/// >>> START src/fenv/fesetexceptflag.c

int fesetexceptflag(const fexcept_t *fp, int mask) {
    feclearexcept(~*fp & mask);
    feraiseexcept(*fp & mask);
    return 0;
}

/// >>> START src/fenv/fesetround.c

/* __fesetround wrapper for arch independent argument check */

int __fesetround(int);

int fesetround(int r) {
    if (r & ~(
        FE_TONEAREST
#ifdef FE_DOWNWARD
        |FE_DOWNWARD
#endif
#ifdef FE_UPWARD
        |FE_UPWARD
#endif
#ifdef FE_TOWARDZERO
        |FE_TOWARDZERO
#endif
        ))
        return -1;
    return __fesetround(r);
}

/// >>> START src/fenv/feupdateenv.c

int feupdateenv(const fenv_t *envp) {
    int ex = fetestexcept(FE_ALL_EXCEPT);
    fesetenv(envp);
    feraiseexcept(ex);
    return 0;
}

/// >>> START src/locale/catclose.c

int catclose (nl_catd catd) {
    return 0;
}

/// >>> START src/locale/catgets.c

char *catgets (nl_catd catd, int set_id, int msg_id, const char *s) {
    return (char *)s;
}

/// >>> START src/locale/catopen.c

nl_catd catopen (const char *name, int oflag) {
    return (nl_catd)-1;
}

/// >>> START src/locale/intl.c

char *gettext(const char *msgid) {
    return (char *) msgid;
}

char *dgettext(const char *domainname, const char *msgid) {
    return (char *) msgid;
}

char *dcgettext(const char *domainname, const char *msgid, int category) {
    return (char *) msgid;
}

char *ngettext(const char *msgid1, const char *msgid2, unsigned long int n) {
    return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *dngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long int n) {
    return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *dcngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long int n, int category) {
    return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *textdomain(const char *domainname) {
    static const char default_str[] = "messages";

    if (domainname && *domainname && strcmp(domainname, default_str)) {
        errno = EINVAL;
        return NULL;
    }
    return (char *) default_str;
}

char *bindtextdomain(const char *domainname, const char *dirname) {
    static const char dir[] = "/";

    if (!domainname || !*domainname
        || (dirname && ((dirname[0] != '/') || dirname[1]))
        ) {
        errno = EINVAL;
        return NULL;
    }

    return (char *) dir;
}

char *bind_textdomain_codeset(const char *domainname, const char *codeset) {
    if (!domainname || !*domainname || (codeset && strcasecmp(codeset, "UTF-8"))) {
        errno = EINVAL;
    }
    return NULL;
}

/// >>> START src/locale/isalnum_l.c

int isalnum_l(int c, locale_t l) {
    return isalnum(c);
}

/// >>> START src/locale/isalpha_l.c

int isalpha_l(int c, locale_t l) {
    return isalpha(c);
}

/// >>> START src/locale/isblank_l.c

int isblank_l(int c, locale_t l) {
    return isblank(c);
}

/// >>> START src/locale/iscntrl_l.c

int iscntrl_l(int c, locale_t l) {
    return iscntrl(c);
}

/// >>> START src/locale/isdigit_l.c

int isdigit_l(int c, locale_t l) {
    return isdigit(c);
}

/// >>> START src/locale/isgraph_l.c

int isgraph_l(int c, locale_t l) {
    return isgraph(c);
}

/// >>> START src/locale/islower_l.c

int islower_l(int c, locale_t l) {
    return islower(c);
}

/// >>> START src/locale/isprint_l.c

int isprint_l(int c, locale_t l) {
    return isprint(c);
}

/// >>> START src/locale/ispunct_l.c

int ispunct_l(int c, locale_t l) {
    return ispunct(c);
}

/// >>> START src/locale/isspace_l.c

int isspace_l(int c, locale_t l) {
    return isspace(c);
}

/// >>> START src/locale/isupper_l.c

int isupper_l(int c, locale_t l) {
    return isupper(c);
}

/// >>> START src/locale/iswalnum_l.c

int iswalnum_l(wint_t c, locale_t l) {
    return iswalnum(c);
}

/// >>> START src/locale/iswalpha_l.c

int iswalpha_l(wint_t c, locale_t l) {
    return iswalpha(c);
}

/// >>> START src/locale/iswblank_l.c

int iswblank_l(wint_t c, locale_t l) {
    return iswblank(c);
}

/// >>> START src/locale/iswcntrl_l.c

int iswcntrl_l(wint_t c, locale_t l) {
    return iswcntrl(c);
}

/// >>> START src/locale/iswctype_l.c

int iswctype_l(wint_t c, wctype_t t, locale_t l) {
    return iswctype(c, t);
}

weak_alias(iswctype_l, __iswctype_l);

/// >>> START src/locale/iswdigit_l.c

int iswdigit_l(wint_t c, locale_t l) {
    return iswdigit(c);
}

/// >>> START src/locale/iswgraph_l.c

int iswgraph_l(wint_t c, locale_t l) {
    return iswgraph(c);
}

/// >>> START src/locale/iswlower_l.c

int iswlower_l(wint_t c, locale_t l) {
    return iswlower(c);
}

/// >>> START src/locale/iswprint_l.c

int iswprint_l(wint_t c, locale_t l) {
    return iswprint(c);
}

/// >>> START src/locale/iswpunct_l.c

int iswpunct_l(wint_t c, locale_t l) {
    return iswpunct(c);
}

/// >>> START src/locale/iswspace_l.c

int iswspace_l(wint_t c, locale_t l) {
    return iswspace(c);
}

/// >>> START src/locale/iswupper_l.c

int iswupper_l(wint_t c, locale_t l) {
    return iswupper(c);
}

/// >>> START src/locale/iswxdigit_l.c

int iswxdigit_l(wint_t c, locale_t l) {
    return iswxdigit(c);
}

/// >>> START src/locale/isxdigit_l.c

int isxdigit_l(int c, locale_t l) {
    return isxdigit(c);
}

/// >>> START src/locale/langinfo.c

static const char c_time[] =
    "Sun\0" "Mon\0" "Tue\0" "Wed\0" "Thu\0" "Fri\0" "Sat\0"
    "Sunday\0" "Monday\0" "Tuesday\0" "Wednesday\0"
    "Thursday\0" "Friday\0" "Saturday\0"
    "Jan\0" "Feb\0" "Mar\0" "Apr\0" "May\0" "Jun\0"
    "Jul\0" "Aug\0" "Sep\0" "Oct\0" "Nov\0" "Dec\0"
    "January\0"   "February\0" "March\0"    "April\0"
    "May\0"       "June\0"     "July\0"     "August\0"
    "September\0" "October\0"  "November\0" "December\0"
    "AM\0" "PM\0"
    "%a %b %e %langinfo_c__T %Y\0"
    "%m/%d/%y\0"
    "%H:%M:%langinfo_c__S\0"
    "%I:%M:%langinfo_c__S %p\0"
    "\0"
    "%m/%d/%y\0"
    "0123456789"
    "%a %b %e %langinfo_c__T %Y\0"
    "%H:%M:%langinfo_c__S";

static const char c_messages[] = "^[yY]\0" "^[nN]";
static const char c_numeric[] = ".\0" "";

char *__nl_langinfo_l(nl_item item, locale_t loc) {
    int cat = item >> 16;
    int idx = item & 65535;
    const char *str;

    if (item == CODESET) return "UTF-8";
    
    switch (cat) {
    case LC_NUMERIC:
        if (idx > 1) return NULL;
        str = c_numeric;
        break;
    case LC_TIME:
        if (idx > 0x31) return NULL;
        str = c_time;
        break;
    case LC_MONETARY:
        if (idx > 0) return NULL;
        str = "";
        break;
    case LC_MESSAGES:
        if (idx > 1) return NULL;
        str = c_messages;
        break;
    default:
        return NULL;
    }

    for (; idx; idx--, str++) for (; *str; str++);
    return (char *)str;
}

char *__nl_langinfo(nl_item item) {
    return __nl_langinfo_l(item, 0);
}

weak_alias(__nl_langinfo, nl_langinfo);
weak_alias(__nl_langinfo_l, nl_langinfo_l);

/// >>> START src/locale/localeconv.c

static const struct lconv posix_lconv = {
    .decimal_point = ".",
    .thousands_sep = "",
    .grouping = "",
    .int_curr_symbol = "",
    .currency_symbol = "",
    .mon_decimal_point = "",
    .mon_thousands_sep = "",
    .mon_grouping = "",
    .positive_sign = "",
    .negative_sign = "",
    .int_frac_digits = -1,
    .frac_digits = -1,
    .p_cs_precedes = -1,
    .p_sep_by_space = -1,
    .n_cs_precedes = -1,
    .n_sep_by_space = -1,
    .p_sign_posn = -1,
    .n_sign_posn = -1,
    .int_p_cs_precedes = -1,
    .int_p_sep_by_space = -1,
    .int_n_cs_precedes = -1,
    .int_n_sep_by_space = -1,
    .int_p_sign_posn = -1,
    .int_n_sign_posn = -1,
};

struct lconv *localeconv(void) {
    return (void *)&posix_lconv;
}

/// >>> START src/locale/setlocale.c

char *setlocale(int category, const char *locale) {
    /* Note: plain "C" would be better, but puts some broken
     * software into legacy 8-bit-codepage mode, ignoring
     * the standard library's multibyte encoding */
    return "C.UTF-8";
}

/// >>> START src/locale/strcasecmp_l.c

int strcasecmp_l(const char *l, const char *r, locale_t loc) {
    return strcasecmp(l, r);
}

/// >>> START src/locale/strcoll.c

int __strcoll_l(const char *l, const char *r, locale_t loc) {
    return strcmp(l, r);
}

int strcoll(const char *l, const char *r) {
    return __strcoll_l(l, r, 0);
}

weak_alias(__strcoll_l, strcoll_l);

/// >>> START src/locale/strerror_l.c

char *strerror_l(int err, locale_t l) {
    return strerror(err);
}

/// >>> START src/locale/strfmon.c

static ssize_t vstrfmon_l(char *s, size_t n, locale_t loc, const char *fmt, va_list ap) {
    size_t l;
    double x;
    int fill, nogrp, negpar, nosym, left, intl;
    int lp, rp, w, fw;
    char *s0=s;
    for (; n && *fmt; ) {
        if (*fmt != '%') {
        literal:
            *s++ = *fmt++;
            n--;
            continue;
        }
        fmt++;
        if (*fmt == '%') goto literal;

        fill = ' ';
        nogrp = 0;
        negpar = 0;
        nosym = 0;
        left = 0;
        for (; ; fmt++) {
            switch (*fmt) {
            case '=':
                fill = *++fmt;
                continue;
            case '^':
                nogrp = 1;
                continue;
            case '(':
                negpar = 1;
            case '+':
                continue;
            case '!':
                nosym = 1;
                continue;
            case '-':
                left = 1;
                continue;
            }
            break;
        }

        for (fw=0; isdigit(*fmt); fmt++)
            fw = 10*fw + (*fmt-'0');
        lp = 0;
        rp = 2;
        if (*fmt=='#') for (lp=0, fmt++; isdigit(*fmt); fmt++)
            lp = 10*lp + (*fmt-'0');
        if (*fmt=='.') for (rp=0, fmt++; isdigit(*fmt); fmt++)
            rp = 10*rp + (*fmt-'0');

        intl = *fmt++ == 'i';

        w = lp + 1 + rp;
        if (!left && fw>w) w = fw;

        x = va_arg(ap, double);
        l = snprintf(s, n, "%*.*f", w, rp, x);
        if (l >= n) {
            errno = E2BIG;
            return -1;
        }
        s += l;
        n -= l;
    }
    return s-s0;
}

ssize_t strfmon_l(char *restrict s, size_t n, locale_t loc, const char *restrict fmt, ...) {
    va_list ap;
    ssize_t ret;

    va_start(ap, fmt);
    ret = vstrfmon_l(s, n, loc, fmt, ap);
    va_end(ap);

    return ret;
}

ssize_t strfmon(char *restrict s, size_t n, const char *restrict fmt, ...) {
    va_list ap;
    ssize_t ret;

    va_start(ap, fmt);
    ret = vstrfmon_l(s, n, 0, fmt, ap);
    va_end(ap);

    return ret;
}

/// >>> START src/locale/strncasecmp_l.c

int strncasecmp_l(const char *l, const char *r, size_t n, locale_t loc) {
    return strncasecmp(l, r, n);
}

/// >>> START src/locale/strxfrm.c

/* collate only by code points */
size_t __strxfrm_l(char *restrict dest, const char *restrict src, size_t n, locale_t loc) {
    size_t l = strlen(src);
    if (n > l) strcpy(dest, src);
    return l;
}

size_t strxfrm(char *restrict dest, const char *restrict src, size_t n) {
    return __strxfrm_l(dest, src, n, 0);
}

weak_alias(__strxfrm_l, strxfrm_l);

/// >>> START src/locale/tolower_l.c

int tolower_l(int c, locale_t l) {
    return tolower(c);
}

/// >>> START src/locale/toupper_l.c

int toupper_l(int c, locale_t l) {
    return toupper(c);
}

/// >>> START src/locale/towctrans_l.c

wint_t towctrans_l(wint_t c, wctrans_t t, locale_t l) {
    return towctrans(c, t);
}

/// >>> START src/locale/towlower_l.c

wint_t towlower_l(wint_t c, locale_t l) {
    return towlower(c);
}

weak_alias(towlower_l, __towlower_l);

/// >>> START src/locale/towupper_l.c

wint_t towupper_l(wint_t c, locale_t l) {
    return towupper(c);
}

weak_alias(towupper_l, __towupper_l);

/// >>> START src/locale/wcscoll.c

/* FIXME: stub */
int __wcscoll_l(const wchar_t *l, const wchar_t *r, locale_t locale) {
    return wcscmp(l, r);
}

int wcscoll(const wchar_t *l, const wchar_t *r) {
    return __wcscoll_l(l, r, 0);
}

weak_alias(__wcscoll_l, wcscoll_l);

/// >>> START src/locale/wcsxfrm.c

/* collate only by code points */
size_t __wcsxfrm_l(wchar_t *restrict dest, const wchar_t *restrict src, size_t n, locale_t loc) {
    size_t l = wcslen(src);
    if (l >= n) {
        wmemcpy(dest, src, n-1);
        dest[n-1] = 0;
    } else wcscpy(dest, src);
    return l;
}

size_t wcsxfrm(wchar_t *restrict dest, const wchar_t *restrict src, size_t n) {
    return __wcsxfrm_l(dest, src, n, 0);
}

weak_alias(__wcsxfrm_l, wcsxfrm_l);

/// >>> START src/locale/wctrans_l.c

wctrans_t wctrans_l(const char *s, locale_t l) {
    return wctrans(s);
}

/// >>> START src/locale/wctype_l.c

wctype_t wctype_l(const char *s, locale_t l) {
    return wctype(s);
}

weak_alias(wctype_l, __wctype_l);

/// >>> START src/math/__cos.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_cos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * __cos( x,  y )
 * kernel cos function on [-__cos_c__pi/4, __cos_c__pi/4], __cos_c__pi/4 ~ 0.785398164
 * Input x is assumed to be bounded by ~__cos_c__pi/4 in magnitude.
 * Input y is the tail of x.
 *
 * Algorithm
 *      1. Since cos(-x) = cos(x), we need only to consider positive x.
 *      2. if x < 2^-27 (hx<0x3e400000 0), return 1 with inexact if x!=0.
 *      3. cos(x) is approximated by a polynomial of degree 14 on
 *         [0,__cos_c__pi/4]
 *                                       4            14
 *              cos(x) ~ 1 - x*x/2 + __cos_c__C1*x + ... + __cos_c__C6*x
 *         where the remez error is
 *
 *      |              2     4     6     8     10    12     14 |     -58
 *      |cos(x)-(1-.5*x +__cos_c__C1*x +__cos_c__C2*x +__cos_c__C3*x +__cos_c__C4*x +__cos_c__C5*x  +__cos_c__C6*x  )| <= 2
 *      |                                                      |
 *
 *                     4     6     8     10    12     14
 *      4. let r = __cos_c__C1*x +__cos_c__C2*x +__cos_c__C3*x +__cos_c__C4*x +__cos_c__C5*x  +__cos_c__C6*x  , then
 *             cos(x) ~ 1 - x*x/2 + r
 *         since cos(x+y) ~ cos(x) - sin(x)*y
 *                        ~ cos(x) - x*y,
 *         a correction term is necessary in cos(x) and hence
 *              cos(x+y) = 1 - (x*x/2 - (r - x*y))
 *         For better accuracy, rearrange to
 *              cos(x+y) ~ w + (tmp + (r-x*y))
 *         where w = 1 - x*x/2 and tmp is a __cos_c__tiny correction term
 *         (1 - x*x/2 == w + tmp exactly in infinite precision).
 *         The exactness of w + tmp in infinite precision depends on w
 *         and tmp having the same precision as x.  If they have extra
 *         precision due to compiler bugs, then the extra precision is
 *         only good provided it is retained in all terms of the final
 *         expression for cos().  Retention happens in all cases tested
 *         under FreeBSD, so don't pessimize things by forcibly clipping
 *         any extra precision in w.
 */

static const double
__cos_c__C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
__cos_c__C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
__cos_c__C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
__cos_c__C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
__cos_c__C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
__cos_c__C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

double __cos(double x, double y) {
    double_t hz,z,r,w;

    z  = x*x;
    w  = z*z;
    r  = z*(__cos_c__C1+z*(__cos_c__C2+z*__cos_c__C3)) + w*w*(__cos_c__C4+z*(__cos_c__C5+z*__cos_c__C6));
    hz = 0.5*z;
    w  = 1.0-hz;
    return w + (((1.0-w)-hz) + (z*r-x*y));
}

/// >>> START src/math/__cosdf.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_cosf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Debugged and optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* |cos(x) - c(x)| < 2**-34.1 (~[-5.37e-11, 5.295e-11]). */
static const double
__cosdf_c__C0  = -0x1ffffffd0c5e81.0p-54, /* -0.499999997251031003120 */
__cosdf_c__C1  =  0x155553e1053a42.0p-57, /*  0.0416666233237390631894 */
__cosdf_c__C2  = -0x16c087e80f1e27.0p-62, /* -0.00138867637746099294692 */
__cosdf_c__C3  =  0x199342e0ee5069.0p-68; /*  0.0000243904487962774090654 */

float __cosdf(double x) {
    double_t r, w, z;

    /* Try to optimize for parallel evaluation as in __tandf.c. */
    z = x*x;
    w = z*z;
    r = __cosdf_c__C2+z*__cosdf_c__C3;
    return ((1.0+z*__cosdf_c__C0) + w*__cosdf_c__C1) + (w*z)*r;
}

/// >>> START src/math/__cosl.c
/* origin: FreeBSD /usr/src/lib/msun/ld80/k_cosl.c */
/* origin: FreeBSD /usr/src/lib/msun/ld128/k_cosl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
/*
 * ld80 version of __cos.c.  See __cos.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-2.43e-23, 2.425e-23]:
 * |cos(x) - c(x)| < 2**-75.1
 *
 * The coefficients of c(x) were generated by a pari-gp script using
 * a Remez algorithm that searches for the best higher coefficients
 * after rounding leading coefficients to a specified precision.
 *
 * Simpler methods like Chebyshev or basic Remez barely suffice for
 * cos() in 64-bit precision, because we want the coefficient of x^2
 * to be precisely -0.5 so that multiplying by it is exact, and plain
 * rounding of the coefficients of a good polynomial approximation only
 * gives this up to about 64-bit precision.  Plain rounding also gives
 * a mediocre approximation for the coefficient of x^4, but a rounding
 * error of 0.5 ulps for this coefficient would only contribute ~0.01
 * ulps to the final error, so this is unimportant.  Rounding errors in
 * higher coefficients are even less important.
 *
 * In fact, coefficients above the x^4 one only need to have 53-bit
 * precision, and this is more efficient.  We get this optimization
 * almost for free from the complications needed to search for the best
 * higher coefficients.
 */
static const long double
__cosl_c__C1 =  0.0416666666666666666136L;        /*  0xaaaaaaaaaaaaaa9b.0p-68 */
static const double
__cosl_c__C2 = -0.0013888888888888874,            /* -0x16c16c16c16c10.0p-62 */
__cosl_c__C3 =  0.000024801587301571716,          /*  0x1a01a01a018e22.0p-68 */
__cosl_c__C4 = -0.00000027557319215507120,        /* -0x127e4fb7602f22.0p-74 */
__cosl_c__C5 =  0.0000000020876754400407278,      /*  0x11eed8caaeccf1.0p-81 */
__cosl_c__C6 = -1.1470297442401303e-11,           /* -0x19393412bd1529.0p-89 */
__cosl_c__C7 =  4.7383039476436467e-14;           /*  0x1aac9d9af5c43e.0p-97 */
#undef POLY
#define POLY(z) (z*(__cosl_c__C1+z*(__cosl_c__C2+z*(__cosl_c__C3+z*(__cosl_c__C4+z*(__cosl_c__C5+z*(__cosl_c__C6+z*__cosl_c__C7)))))))
#elif LDBL_MANT_DIG == 113
/*
 * ld128 version of __cos.c.  See __cos.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-1.80e-37, 1.79e-37]:
 * |cos(x) - c(x))| < 2**-122.0
 *
 * 113-bit precision requires more care than 64-bit precision, since
 * simple methods give a minimax polynomial with coefficient for x^2
 * that is 1 ulp below 0.5, but we want it to be precisely 0.5.  See
 * above for more details.
 */
static const long double
__cosl_c__C1 =  0.04166666666666666666666666666666658424671L,
__cosl_c__C2 = -0.001388888888888888888888888888863490893732L,
__cosl_c__C3 =  0.00002480158730158730158730158600795304914210L,
__cosl_c__C4 = -0.2755731922398589065255474947078934284324e-6L,
__cosl_c__C5 =  0.2087675698786809897659225313136400793948e-8L,
__cosl_c__C6 = -0.1147074559772972315817149986812031204775e-10L,
__cosl_c__C7 =  0.4779477332386808976875457937252120293400e-13L;
static const double
__cosl_c__C8 = -0.1561920696721507929516718307820958119868e-15,
__cosl_c__C9 =  0.4110317413744594971475941557607804508039e-18,
C10 = -0.8896592467191938803288521958313920156409e-21,
C11 =  0.1601061435794535138244346256065192782581e-23;
#undef POLY
#define POLY(z) (z*(__cosl_c__C1+z*(__cosl_c__C2+z*(__cosl_c__C3+z*(__cosl_c__C4+z*(__cosl_c__C5+z*(__cosl_c__C6+z*(__cosl_c__C7+ \
    z*(__cosl_c__C8+z*(__cosl_c__C9+z*(C10+z*C11)))))))))))
#endif

long double __cosl(long double x, long double y) {
    long double hz,z,r,w;

    z  = x*x;
    r  = POLY(z);
    hz = 0.5*z;
    w  = 1.0-hz;
    return w + (((1.0-w)-hz) + (z*r-x*y));
}
#endif

/// >>> START src/math/__expo2.c

/* __expo2_c__k is such that __expo2_c__k*ln2 has minimal relative error and x - __expo2_c__kln2 > log(DBL_MIN) */
static const int __expo2_c__k = 2043;
static const double __expo2_c__kln2 = 0x1.62066151add8bp+10;

/* exp(x)/2 for x >= log(DBL_MAX), slightly better than 0.5*exp(x/2)*exp(x/2) */
double __expo2(double x) {
    double scale;

    /* note that __expo2_c__k is odd and scale*scale overflows */
    INSERT_WORDS(scale, (uint32_t)(0x3ff + __expo2_c__k/2) << 20, 0);
    /* exp(x - __expo2_c__k ln2) * 2**(__expo2_c__k-1) */
    return exp(x - __expo2_c__kln2) * scale * scale;
}

/// >>> START src/math/__expo2f.c

/* __expo2f_c__k is such that __expo2f_c__k*ln2 has minimal relative error and x - __expo2f_c__kln2 > log(FLT_MIN) */
static const int __expo2f_c__k = 235;
static const float __expo2f_c__kln2 = 0x1.45c778p+7f;

/* expf(x)/2 for x >= log(FLT_MAX), slightly better than 0.5f*expf(x/2)*expf(x/2) */
float __expo2f(float x) {
    float scale;

    /* note that __expo2f_c__k is odd and scale*scale overflows */
    SET_FLOAT_WORD(scale, (uint32_t)(0x7f + __expo2f_c__k/2) << 23);
    /* exp(x - __expo2f_c__k ln2) * 2**(__expo2f_c__k-1) */
    return expf(x - __expo2f_c__kln2) * scale * scale;
}

/// >>> START src/math/__fpclassify.c

int __fpclassify(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = u.i>>52 & 0x7ff;
    if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
    if (e==0x7ff) return u.i<<12 ? FP_NAN : FP_INFINITE;
    return FP_NORMAL;
}

/// >>> START src/math/__fpclassifyf.c

int __fpclassifyf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = u.i>>23 & 0xff;
    if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
    if (e==0xff) return u.i<<9 ? FP_NAN : FP_INFINITE;
    return FP_NORMAL;
}

/// >>> START src/math/__fpclassifyl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024

#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
int __fpclassifyl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    int msb = u.i.m>>63;
    if (!e && !msb)
        return u.i.m ? FP_SUBNORMAL : FP_ZERO;
    if (!msb)
        return FP_NAN;
    if (e == 0x7fff)
        return u.i.m << 1 ? FP_NAN : FP_INFINITE;
    return FP_NORMAL;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
int __fpclassifyl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    if (!e)
        return u.i2.lo | u.i2.hi ? FP_SUBNORMAL : FP_ZERO;
    if (e == 0x7fff) {
        u.i.se = 0;
        return u.i2.lo | u.i2.hi ? FP_NAN : FP_INFINITE;
    }
    return FP_NORMAL;
}
#endif

/// >>> START src/math/__invtrigl.c
/// >>> START src/math/__invtrigl.h
/* shared by acosl, asinl and atan2l */
#undef acst2l__pio2_hi
#define acst2l__pio2_hi __pio2_hi
#undef acst2l__pio2_lo
#define acst2l__pio2_lo __pio2_lo
extern const long double acst2l__pio2_hi, acst2l__pio2_lo;

long double __invtrigl_R(long double z);

/// >>> CONTINUE src/math/__invtrigl.c

#if LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
static const long double
__invtrigl_c__pS0 =  1.66666666666666666631e-01L,
__invtrigl_c__pS1 = -4.16313987993683104320e-01L,
__invtrigl_c__pS2 =  3.69068046323246813704e-01L,
__invtrigl_c__pS3 = -1.36213932016738603108e-01L,
__invtrigl_c__pS4 =  1.78324189708471965733e-02L,
__invtrigl_c__pS5 = -2.19216428382605211588e-04L,
__invtrigl_c__pS6 = -7.10526623669075243183e-06L,
__invtrigl_c__qS1 = -2.94788392796209867269e+00L,
__invtrigl_c__qS2 =  3.27309890266528636716e+00L,
__invtrigl_c__qS3 = -1.68285799854822427013e+00L,
__invtrigl_c__qS4 =  3.90699412641738801874e-01L,
__invtrigl_c__qS5 = -3.14365703596053263322e-02L;

const long double acst2l__pio2_hi = 1.57079632679489661926L;
const long double acst2l__pio2_lo = -2.50827880633416601173e-20L;

/* used in asinl() and acosl() */
/* __invtrigl_c__R(x^2) is a rational approximation of (asin(x)-x)/x^3 with Remez algorithm */
long double __invtrigl_R(long double z) {
    long double p, q;
    p = z*(__invtrigl_c__pS0+z*(__invtrigl_c__pS1+z*(__invtrigl_c__pS2+z*(__invtrigl_c__pS3+z*(__invtrigl_c__pS4+z*(__invtrigl_c__pS5+z*__invtrigl_c__pS6))))));
    q = 1.0+z*(__invtrigl_c__qS1+z*(__invtrigl_c__qS2+z*(__invtrigl_c__qS3+z*(__invtrigl_c__qS4+z*__invtrigl_c__qS5))));
    return p/q;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
static const long double
__invtrigl_c__pS0 =  1.66666666666666666666666666666700314e-01L,
__invtrigl_c__pS1 = -7.32816946414566252574527475428622708e-01L,
__invtrigl_c__pS2 =  1.34215708714992334609030036562143589e+00L,
__invtrigl_c__pS3 = -1.32483151677116409805070261790752040e+00L,
__invtrigl_c__pS4 =  7.61206183613632558824485341162121989e-01L,
__invtrigl_c__pS5 = -2.56165783329023486777386833928147375e-01L,
__invtrigl_c__pS6 =  4.80718586374448793411019434585413855e-02L,
__invtrigl_c__pS7 = -4.42523267167024279410230886239774718e-03L,
__invtrigl_c__pS8 =  1.44551535183911458253205638280410064e-04L,
__invtrigl_c__pS9 = -2.10558957916600254061591040482706179e-07L,
__invtrigl_c__qS1 = -4.84690167848739751544716485245697428e+00L,
__invtrigl_c__qS2 =  9.96619113536172610135016921140206980e+00L,
__invtrigl_c__qS3 = -1.13177895428973036660836798461641458e+01L,
__invtrigl_c__qS4 =  7.74004374389488266169304117714658761e+00L,
__invtrigl_c__qS5 = -3.25871986053534084709023539900339905e+00L,
__invtrigl_c__qS6 =  8.27830318881232209752469022352928864e-01L,
__invtrigl_c__qS7 = -1.18768052702942805423330715206348004e-01L,
__invtrigl_c__qS8 =  8.32600764660522313269101537926539470e-03L,
__invtrigl_c__qS9 = -1.99407384882605586705979504567947007e-04L;

const long double acst2l__pio2_hi = 1.57079632679489661923132169163975140L;
const long double acst2l__pio2_lo = 4.33590506506189051239852201302167613e-35L;

long double __invtrigl_R(long double z) {
    long double p, q;
    p = z*(__invtrigl_c__pS0+z*(__invtrigl_c__pS1+z*(__invtrigl_c__pS2+z*(__invtrigl_c__pS3+z*(__invtrigl_c__pS4+z*(__invtrigl_c__pS5+z*(__invtrigl_c__pS6+z*(__invtrigl_c__pS7+z*(__invtrigl_c__pS8+z*__invtrigl_c__pS9)))))))));
    q = 1.0+z*(__invtrigl_c__qS1+z*(__invtrigl_c__qS2+z*(__invtrigl_c__qS3+z*(__invtrigl_c__qS4+z*(__invtrigl_c__qS5+z*(__invtrigl_c__pS6+z*(__invtrigl_c__pS7+z*(__invtrigl_c__pS8+z*__invtrigl_c__pS9))))))));
    return p/q;
}
#endif

/// >>> START src/math/__polevll.c
/* origin: OpenBSD /usr/src/lib/libm/src/polevll.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Evaluate polynomial
 *
 *
 * SYNOPSIS:
 *
 * int N;
 * long double x, y, coef[N+1], polevl[];
 *
 * y = polevll( x, coef, N );
 *
 *
 * DESCRIPTION:
 *
 * Evaluates polynomial of degree N:
 *
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 *
 *  The function p1evll() assumes that coef[N] = 1.0 and is
 * omitted from the array.  Its calling arguments are
 * otherwise the same as polevll().
 *
 *
 * SPEED:
 *
 * In the interest of speed, there are no checks for out
 * of bounds arithmetic.  This routine is used by most of
 * the functions in the library.  Depending on available
 * equipment features, the user may wish to rewrite the
 * program in microcode or assembly language.
 *
 */

/*
 * Polynomial evaluator:
 *  __polevll_c__P[0] x^n  +  __polevll_c__P[1] x^(n-1)  +  ...  +  __polevll_c__P[n]
 */
long double __polevll(long double x, const long double *__polevll_c__P, int n) {
    long double y;

    y = *__polevll_c__P++;
    do {
        y = y * x + *__polevll_c__P++;
    } while (--n);

    return y;
}

/*
 * Polynomial evaluator:
 *  x^n  +  __polevll_c__P[0] x^(n-1)  +  __polevll_c__P[1] x^(n-2)  +  ...  +  __polevll_c__P[n]
 */
long double __p1evll(long double x, const long double *__polevll_c__P, int n) {
    long double y;

    n -= 1;
    y = x + *__polevll_c__P++;
    do {
        y = y * x + *__polevll_c__P++;
    } while (--n);

    return y;
}

/// >>> START src/math/__rem_pio2.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_rem_pio2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * Optimized by Bruce D. Evans.
 */
/* __rem_pio2(x,y)
 *
 * return the remainder of x rem __rem_pio2_c__pi/2 in y[0]+y[1]
 * use __rem_pio2_large() for large x
 */

/*
 * __rem_pio2_c__invpio2:  53 bits of 2/__rem_pio2_c__pi
 * __rem_pio2_c__pio2_1:   first  33 bit of __rem_pio2_c__pi/2
 * __rem_pio2_c__pio2_1t:  __rem_pio2_c__pi/2 - __rem_pio2_c__pio2_1
 * __rem_pio2_c__pio2_2:   second 33 bit of __rem_pio2_c__pi/2
 * __rem_pio2_c__pio2_2t:  __rem_pio2_c__pi/2 - (__rem_pio2_c__pio2_1+__rem_pio2_c__pio2_2)
 * __rem_pio2_c__pio2_3:   third  33 bit of __rem_pio2_c__pi/2
 * __rem_pio2_c__pio2_3t:  __rem_pio2_c__pi/2 - (__rem_pio2_c__pio2_1+__rem_pio2_c__pio2_2+__rem_pio2_c__pio2_3)
 */
static const double
__rem_pio2_c__invpio2 = 6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
__rem_pio2_c__pio2_1  = 1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
__rem_pio2_c__pio2_1t = 6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
__rem_pio2_c__pio2_2  = 6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
__rem_pio2_c__pio2_2t = 2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
__rem_pio2_c__pio2_3  = 2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
__rem_pio2_c__pio2_3t = 8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

/* caller must handle the case when reduction is not needed: |x| ~<= __rem_pio2_c__pi/4 */
int __rem_pio2(double x, double *y) {
    union {double f; uint64_t i;} u = {x};
    double_t z,w,t,r;
    double tx[3],ty[2],fn;
    uint32_t ix;
    int sign, n, ex, ey, i;

    sign = u.i>>63;
    ix = u.i>>32 & 0x7fffffff;
    if (ix <= 0x400f6a7a) {  /* |x| ~<= 5pi/4 */
        if ((ix & 0xfffff) == 0x921fb)  /* |x| ~= __rem_pio2_c__pi/2 or 2pi/2 */
            goto medium;  /* cancellation -- use medium case */
        if (ix <= 0x4002d97c) {  /* |x| ~<= 3pi/4 */
            if (!sign) {
                z = x - __rem_pio2_c__pio2_1;  /* one round good to 85 bits */
                y[0] = z - __rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) - __rem_pio2_c__pio2_1t;
                return 1;
            } else {
                z = x + __rem_pio2_c__pio2_1;
                y[0] = z + __rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) + __rem_pio2_c__pio2_1t;
                return -1;
            }
        } else {
            if (!sign) {
                z = x - 2*__rem_pio2_c__pio2_1;
                y[0] = z - 2*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) - 2*__rem_pio2_c__pio2_1t;
                return 2;
            } else {
                z = x + 2*__rem_pio2_c__pio2_1;
                y[0] = z + 2*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) + 2*__rem_pio2_c__pio2_1t;
                return -2;
            }
        }
    }
    if (ix <= 0x401c463b) {  /* |x| ~<= 9pi/4 */
        if (ix <= 0x4015fdbc) {  /* |x| ~<= 7pi/4 */
            if (ix == 0x4012d97c)  /* |x| ~= 3pi/2 */
                goto medium;
            if (!sign) {
                z = x - 3*__rem_pio2_c__pio2_1;
                y[0] = z - 3*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) - 3*__rem_pio2_c__pio2_1t;
                return 3;
            } else {
                z = x + 3*__rem_pio2_c__pio2_1;
                y[0] = z + 3*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) + 3*__rem_pio2_c__pio2_1t;
                return -3;
            }
        } else {
            if (ix == 0x401921fb)  /* |x| ~= 4pi/2 */
                goto medium;
            if (!sign) {
                z = x - 4*__rem_pio2_c__pio2_1;
                y[0] = z - 4*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) - 4*__rem_pio2_c__pio2_1t;
                return 4;
            } else {
                z = x + 4*__rem_pio2_c__pio2_1;
                y[0] = z + 4*__rem_pio2_c__pio2_1t;
                y[1] = (z-y[0]) + 4*__rem_pio2_c__pio2_1t;
                return -4;
            }
        }
    }
    if (ix < 0x413921fb) {  /* |x| ~< 2^20*(__rem_pio2_c__pi/2), medium size */
medium:
        /* rint(x/(__rem_pio2_c__pi/2)), Assume round-to-nearest. */
        fn = x*__rem_pio2_c__invpio2 + 0x1.8p52;
        fn = fn - 0x1.8p52;
        n = (int32_t)fn;
        r = x - fn*__rem_pio2_c__pio2_1;
        w = fn*__rem_pio2_c__pio2_1t;  /* 1st round, good to 85 bits */
        y[0] = r - w;
        u.f = y[0];
        ey = u.i>>52 & 0x7ff;
        ex = ix>>20;
        if (ex - ey > 16) { /* 2nd round, good to 118 bits */
            t = r;
            w = fn*__rem_pio2_c__pio2_2;
            r = t - w;
            w = fn*__rem_pio2_c__pio2_2t - ((t-r)-w);
            y[0] = r - w;
            u.f = y[0];
            ey = u.i>>52 & 0x7ff;
            if (ex - ey > 49) {  /* 3rd round, good to 151 bits, covers all cases */
                t = r;
                w = fn*__rem_pio2_c__pio2_3;
                r = t - w;
                w = fn*__rem_pio2_c__pio2_3t - ((t-r)-w);
                y[0] = r - w;
            }
        }
        y[1] = (r - y[0]) - w;
        return n;
    }
    /*
     * all other (large) arguments
     */
    if (ix >= 0x7ff00000) {  /* x is inf or NaN */
        y[0] = y[1] = x - x;
        return 0;
    }
    /* set z = scalbn(|x|,-ilogb(x)+23) */
    u.f = x;
    u.i &= (uint64_t)-1>>12;
    u.i |= (uint64_t)(0x3ff + 23)<<52;
    z = u.f;
    for (i=0; i < 2; i++) {
        tx[i] = (double)(int32_t)z;
        z     = (z-tx[i])*0x1p24;
    }
    tx[i] = z;
    /* skip __rem_pio2_c__zero terms, first term is non-__rem_pio2_c__zero */
    while (tx[i] == 0.0)
        i--;
    n = __rem_pio2_large(tx,ty,(int)(ix>>20)-(0x3ff+23),i+1,1);
    if (sign) {
        y[0] = -ty[0];
        y[1] = -ty[1];
        return -n;
    }
    y[0] = ty[0];
    y[1] = ty[1];
    return n;
}

/// >>> START src/math/__rem_pio2_large.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_rem_pio2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * __rem_pio2_large(x,y,e0,nx,prec)
 * double x[],y[]; int e0,nx,prec;
 *
 * __rem_pio2_large return the last three digits of N with
 *              y = x - N*__rem_pio2_large_c__pi/2
 * so that |y| < __rem_pio2_large_c__pi/2.
 *
 * The method is to compute the integer (mod 8) and fraction parts of
 * (2/__rem_pio2_large_c__pi)*x without doing the full multiplication. In general we
 * skip the part of the product that are known to be a __rem_pio2_large_c__huge integer (
 * more accurately, = 0 mod 8 ). Thus the number of operations are
 * independent of the exponent of the input.
 *
 * (2/__rem_pio2_large_c__pi) is represented by an array of 24-bit integers in ipio2[].
 *
 * Input parameters:
 *      x[]     The input value (must be positive) is broken into nx
 *              pieces of 24-bit integers in double precision format.
 *              x[i] will be the i-th 24 bit of x. The scaled exponent
 *              of x[0] is given in input parameter e0 (i.e., x[0]*2^e0
 *              match x's up to 24 bits.
 *
 *              Example of breaking a double positive z into x[0]+x[1]+x[2]:
 *                      e0 = ilogb(z)-23
 *                      z  = scalbn(z,-e0)
 *              for i = 0,1,2
 *                      x[i] = floor(z)
 *                      z    = (z-x[i])*2**24
 *
 *
 *      y[]     ouput result in an array of double precision numbers.
 *              The dimension of y[] is:
 *                      24-bit  precision       1
 *                      53-bit  precision       2
 *                      64-bit  precision       2
 *                      113-bit precision       3
 *              The actual value is the sum of them. Thus for 113-bit
 *              precison, one may have to do something like:
 *
 *              long double t,w,r_head, r_tail;
 *              t = (long double)y[2] + (long double)y[1];
 *              w = (long double)y[0];
 *              r_head = t+w;
 *              r_tail = w - (r_head - t);
 *
 *      e0      The exponent of x[0]. Must be <= 16360 or you need to
 *              expand the ipio2 __rem_pio2_large_c__table.
 *
 *      nx      dimension of x[]
 *
 *      prec    an integer indicating the precision:
 *                      0       24  bits (single)
 *                      1       53  bits (double)
 *                      2       64  bits (extended)
 *                      3       113 bits (quad)
 *
 * External function:
 *      double scalbn(), floor();
 *
 *
 * Here is the description of some local variables:
 *
 *      jk      jk+1 is the initial number of terms of ipio2[] needed
 *              in the computation. The minimum and recommended value
 *              for jk is 3,4,4,6 for single, double, extended, and quad.
 *              jk+1 must be 2 larger than you might expect so that our
 *              recomputation test works. (Up to 24 bits in the integer
 *              part (the 24 bits of it that we compute) and 23 bits in
 *              the fraction part may be lost to cancelation before we
 *              recompute.)
 *
 *      jz      local integer variable indicating the number of
 *              terms of ipio2[] used.
 *
 *      jx      nx - 1
 *
 *      jv      index for pointing to the suitable ipio2[] for the
 *              computation. In general, we want
 *                      ( 2^e0*x[0] * ipio2[jv-1]*2^(-24jv) )/8
 *              is an integer. Thus
 *                      e0-3-24*jv >= 0 or (e0-3)/24 >= jv
 *              Hence jv = max(0,(e0-3)/24).
 *
 *      jp      jp+1 is the number of terms in PIo2[] needed, jp = jk.
 *
 *      q[]     double array with integral value, representing the
 *              24-bits chunk of the product of x and 2/__rem_pio2_large_c__pi.
 *
 *      q0      the corresponding exponent of q[0]. Note that the
 *              exponent for q[i] would be q0-24*i.
 *
 *      PIo2[]  double precision array, obtained by cutting __rem_pio2_large_c__pi/2
 *              into 24 bits chunks.
 *
 *      f[]     ipio2[] in floating point
 *
 *      iq[]    integer array by breaking up q[] in 24-bits chunk.
 *
 *      fq[]    final product of x*(2/__rem_pio2_large_c__pi) in fq[0],..,fq[jk]
 *
 *      ih      integer. If >0 it indicates q[] is >= 0.5, hence
 *              it also indicates the *sign* of the result.
 *
 */
/*
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const int init_jk[] = {3,4,4,6}; /* initial value for jk */

/*
 * Table of constants for 2/__rem_pio2_large_c__pi, 396 Hex digits (476 decimal) of 2/__rem_pio2_large_c__pi
 *
 *              integer array, contains the (24*i)-th to (24*i+23)-th
 *              bit of 2/__rem_pio2_large_c__pi after binary point. The corresponding
 *              floating value is
 *
 *                      ipio2[i] * 2^(-24(i+1)).
 *
 * NB: This __rem_pio2_large_c__table must have at least (e0-3)/24 + jk terms.
 *     For quad precision (e0 <= 16360, jk = 6), this is 686.
 */
static const int32_t ipio2[] = {
0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62,
0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A,
0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41,
0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8,
0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08,
0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880,
0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,

#if LDBL_MAX_EXP > 1024
0x47C419, 0xC367CD, 0xDCE809, 0x2A8359, 0xC4768B, 0x961CA6,
0xDDAF44, 0xD15719, 0x053EA5, 0xFF0705, 0x3F7E33, 0xE832C2,
0xDE4F98, 0x327DBB, 0xC33D26, 0xEF6B1E, 0x5EF89F, 0x3A1F35,
0xCAF27F, 0x1D87F1, 0x21907C, 0x7C246A, 0xFA6ED5, 0x772D30,
0x433B15, 0xC614B5, 0x9D19C3, 0xC2C4AD, 0x414D2C, 0x5D000C,
0x467D86, 0x2D71E3, 0x9AC69B, 0x006233, 0x7CD2B4, 0x97A7B4,
0xD55537, 0xF63ED7, 0x1810A3, 0xFC764D, 0x2A9D64, 0xABD770,
0xF87C63, 0x57B07A, 0xE71517, 0x5649C0, 0xD9D63B, 0x3884A7,
0xCB2324, 0x778AD6, 0x23545A, 0xB91F00, 0x1B0AF1, 0xDFCE19,
0xFF319F, 0x6A1E66, 0x615799, 0x47FBAC, 0xD87F7E, 0xB76522,
0x89E832, 0x60BFE6, 0xCDC4EF, 0x09366C, 0xD43F5D, 0xD7DE16,
0xDE3B58, 0x929BDE, 0x2822D2, 0xE88628, 0x4D58E2, 0x32CAC6,
0x16E308, 0xCB7DE0, 0x50C017, 0xA71DF3, 0x5BE018, 0x34132E,
0x621283, 0x014883, 0x5B8EF5, 0x7FB0AD, 0xF2E91E, 0x434A48,
0xD36710, 0xD8DDAA, 0x425FAE, 0xCE616A, 0xA4280A, 0xB499D3,
0xF2A606, 0x7F775C, 0x83C2A3, 0x883C61, 0x78738A, 0x5A8CAF,
0xBDD76F, 0x63A62D, 0xCBBFF4, 0xEF818D, 0x67C126, 0x45CA55,
0x36D9CA, 0xD2A828, 0x8D61C2, 0x77C912, 0x142604, 0x9B4612,
0xC459C4, 0x44C5C8, 0x91B24D, 0xF31700, 0xAD43D4, 0xE54929,
0x10D5FD, 0xFCBE00, 0xCC941E, 0xEECE70, 0xF53E13, 0x80F1EC,
0xC3E7B3, 0x28F8C7, 0x940593, 0x3E71C1, 0xB3092E, 0xF3450B,
0x9C1288, 0x7B20AB, 0x9FB52E, 0xC29247, 0x2F327B, 0x6D550C,
0x90A772, 0x1FE76B, 0x96CB31, 0x4A1679, 0xE27941, 0x89DFF4,
0x9794E8, 0x84E6E2, 0x973199, 0x6BED88, 0x365F5F, 0x0EFDBB,
0xB49A48, 0x6CA467, 0x427271, 0x325D8D, 0xB8159F, 0x09E5BC,
0x25318D, 0x3974F7, 0x1C0530, 0x010C0D, 0x68084B, 0x58EE2C,
0x90AA47, 0x02E774, 0x24D6BD, 0xA67DF7, 0x72486E, 0xEF169F,
0xA6948E, 0xF691B4, 0x5153D1, 0xF20ACF, 0x339820, 0x7E4BF5,
0x6863B2, 0x5F3EDD, 0x035D40, 0x7F8985, 0x295255, 0xC06437,
0x10D86D, 0x324832, 0x754C5B, 0xD4714E, 0x6E5445, 0xC1090B,
0x69F52A, 0xD56614, 0x9D0727, 0x50045D, 0xDB3BB4, 0xC576EA,
0x17F987, 0x7D6B49, 0xBA271D, 0x296996, 0xACCCC6, 0x5414AD,
0x6AE290, 0x89D988, 0x50722C, 0xBEA404, 0x940777, 0x7030F3,
0x27FC00, 0xA871EA, 0x49C266, 0x3DE064, 0x83DD97, 0x973FA3,
0xFD9443, 0x8C860D, 0xDE4131, 0x9D3992, 0x8C70DD, 0xE7B717,
0x3BDF08, 0x2B3715, 0xA0805C, 0x93805A, 0x921110, 0xD8E80F,
0xAF806C, 0x4BFFDB, 0x0F9038, 0x761859, 0x15A562, 0xBBCB61,
0xB989C7, 0xBD4010, 0x04F2D2, 0x277549, 0xF6B6EB, 0xBB22DB,
0xAA140A, 0x2F2689, 0x768364, 0x333B09, 0x1A940E, 0xAA3A51,
0xC2A31D, 0xAEEDAF, 0x12265C, 0x4DC26D, 0x9C7A2D, 0x9756C0,
0x833F03, 0xF6F009, 0x8C402B, 0x99316D, 0x07B439, 0x15200C,
0x5BC3D8, 0xC492F5, 0x4BADC6, 0xA5CA4E, 0xCD37A7, 0x36A9E6,
0x9492AB, 0x6842DD, 0xDE6319, 0xEF8C76, 0x528B68, 0x37DBFC,
0xABA1AE, 0x3115DF, 0xA1AE00, 0xDAFB0C, 0x664D64, 0xB705ED,
0x306529, 0xBF5657, 0x3AFF47, 0xB9F96A, 0xF3BE75, 0xDF9328,
0x3080AB, 0xF68C66, 0x15CB04, 0x0622FA, 0x1DE4D9, 0xA4B33D,
0x8F1B57, 0x09CD36, 0xE9424E, 0xA4BE13, 0xB52333, 0x1AAAF0,
0xA8654F, 0xA5C1D2, 0x0F3F0B, 0xCD785B, 0x76F923, 0x048B7B,
0x721789, 0x53A6C6, 0xE26E6F, 0x00EBEF, 0x584A9B, 0xB7DAC4,
0xBA66AA, 0xCFCF76, 0x1D02D1, 0x2DF1B1, 0xC1998C, 0x77ADC3,
0xDA4886, 0xA05DF7, 0xF480C6, 0x2FF0AC, 0x9AECDD, 0xBC5C3F,
0x6DDED0, 0x1FC790, 0xB6DB2A, 0x3A25A3, 0x9AAF00, 0x9353AD,
0x0457B6, 0xB42D29, 0x7E804B, 0xA707DA, 0x0EAA76, 0xA1597B,
0x2A1216, 0x2DB7DC, 0xFDE5FA, 0xFEDB89, 0xFDBE89, 0x6C76E4,
0xFCA906, 0x70803E, 0x156E85, 0xFF87FD, 0x073E28, 0x336761,
0x86182A, 0xEABD4D, 0xAFE7B3, 0x6E6D8F, 0x396795, 0x5BBF31,
0x48D784, 0x16DF30, 0x432DC7, 0x356125, 0xCE70C9, 0xB8CB30,
0xFD6CBF, 0xA200A4, 0xE46C05, 0xA0DD5A, 0x476F21, 0xD21262,
0x845CB9, 0x496170, 0xE0566B, 0x015299, 0x375550, 0xB7D51E,
0xC4F133, 0x5F6E13, 0xE4305D, 0xA92E85, 0xC3B21D, 0x3632A1,
0xA4B708, 0xD4B1EA, 0x21F716, 0xE4698F, 0x77FF27, 0x80030C,
0x2D408D, 0xA0CD4F, 0x99A520, 0xD3A2B3, 0x0A5D2F, 0x42F9B4,
0xCBDA11, 0xD0BE7D, 0xC1DB9B, 0xBD17AB, 0x81A2CA, 0x5C6A08,
0x17552E, 0x550027, 0xF0147F, 0x8607E1, 0x640B14, 0x8D4196,
0xDEBE87, 0x2AFDDA, 0xB6256B, 0x34897B, 0xFEF305, 0x9EBFB9,
0x4F6A68, 0xA82A4A, 0x5AC44F, 0xBCF82D, 0x985AD7, 0x95C7F4,
0x8D4D0D, 0xA63A20, 0x5F57A4, 0xB13F14, 0x953880, 0x0120CC,
0x86DD71, 0xB6DEC9, 0xF560BF, 0x11654D, 0x6B0701, 0xACB08C,
0xD0C0B2, 0x485551, 0x0EFB1E, 0xC37295, 0x3B06A3, 0x3540C0,
0x7BDC06, 0xCC45E0, 0xFA294E, 0xC8CAD6, 0x41F3E8, 0xDE647C,
0xD8649B, 0x31BED9, 0xC397A4, 0xD45877, 0xC5E369, 0x13DAF0,
0x3C3ABA, 0x461846, 0x5F7555, 0xF5BDD2, 0xC6926E, 0x5D2EAC,
0xED440E, 0x423E1C, 0x87C461, 0xE9FD29, 0xF3D6E7, 0xCA7C22,
0x35916F, 0xC5E008, 0x8DD7FF, 0xE26A6E, 0xC6FDB0, 0xC10893,
0x745D7C, 0xB2AD6B, 0x9D6ECD, 0x7B723E, 0x6A11C6, 0xA9CFF7,
0xDF7329, 0xBAC9B5, 0x5100B7, 0x0DB2E2, 0x24BA74, 0x607DE5,
0x8AD874, 0x2C150D, 0x0C1881, 0x94667E, 0x162901, 0x767A9F,
0xBEFDFD, 0xEF4556, 0x367ED9, 0x13D9EC, 0xB9BA8B, 0xFC97C4,
0x27A831, 0xC36EF1, 0x36C594, 0x56A8D8, 0xB5A8B4, 0x0ECCCF,
0x2D8912, 0x34576F, 0x89562C, 0xE3CE99, 0xB920D6, 0xAA5E6B,
0x9C2A3E, 0xCC5F11, 0x4A0BFD, 0xFBF4E1, 0x6D3B8E, 0x2C86E2,
0x84D4E9, 0xA9B4FC, 0xD1EEEF, 0xC9352E, 0x61392F, 0x442138,
0xC8D91B, 0x0AFC81, 0x6A4AFB, 0xD81C2F, 0x84B453, 0x8C994E,
0xCC2254, 0xDC552A, 0xD6C6C0, 0x96190B, 0xB8701A, 0x649569,
0x605A26, 0xEE523F, 0x0F117F, 0x11B5F4, 0xF5CBFC, 0x2DBC34,
0xEEBC34, 0xCC5DE8, 0x605EDD, 0x9B8E67, 0xEF3392, 0xB817C9,
0x9B5861, 0xBC57E1, 0xC68351, 0x103ED8, 0x4871DD, 0xDD1C2D,
0xA118AF, 0x462C21, 0xD7F359, 0x987AD9, 0xC0549E, 0xFA864F,
0xFC0656, 0xAE79E5, 0x362289, 0x22AD38, 0xDC9367, 0xAAE855,
0x382682, 0x9BE7CA, 0xA40D51, 0xB13399, 0x0ED7A9, 0x480569,
0xF0B265, 0xA7887F, 0x974C88, 0x36D1F9, 0xB39221, 0x4A827B,
0x21CF98, 0xDC9F40, 0x5547DC, 0x3A74E1, 0x42EB67, 0xDF9DFE,
0x5FD45E, 0xA4677B, 0x7AACBA, 0xA2F655, 0x23882B, 0x55BA41,
0x086E59, 0x862A21, 0x834739, 0xE6E389, 0xD49EE5, 0x40FB49,
0xE956FF, 0xCA0F1C, 0x8A59C5, 0x2BFA94, 0xC5C1D3, 0xCFC50F,
0xAE5ADB, 0x86C547, 0x624385, 0x3B8621, 0x94792C, 0x876110,
0x7B4C2A, 0x1A2C80, 0x12BF43, 0x902688, 0x893C78, 0xE4C4A8,
0x7BDBE5, 0xC23AC4, 0xEAF426, 0x8A67F7, 0xBF920D, 0x2BA365,
0xB1933D, 0x0B7CBD, 0xDC51A4, 0x63DD27, 0xDDE169, 0x19949A,
0x9529A8, 0x28CE68, 0xB4ED09, 0x209F44, 0xCA984E, 0x638270,
0x237C7E, 0x32B90F, 0x8EF5A7, 0xE75614, 0x08F121, 0x2A9DB5,
0x4D7E6F, 0x5119A5, 0xABF9B5, 0xD6DF82, 0x61DD96, 0x023616,
0x9F3AC4, 0xA1A283, 0x6DED72, 0x7A8D39, 0xA9B882, 0x5C326B,
0x5B2746, 0xED3400, 0x7700D2, 0x55F4FC, 0x4D5901, 0x8071E0,
#endif
};

static const double PIo2[] = {
  1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
  7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
  5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
  3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
  1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
  1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
  2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
  2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
};

int __rem_pio2_large(double *x, double *y, int e0, int nx, int prec) {
    int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,__rem_pio2_large_c__k,m,q0,ih;
    double z,fw,f[20],fq[20],q[20];

    /* initialize jk*/
    jk = init_jk[prec];
    jp = jk;

    /* determine jx,jv,q0, note that 3>q0 */
    jx = nx-1;
    jv = (e0-3)/24;  if(jv<0) jv=0;
    q0 = e0-24*(jv+1);

    /* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
    j = jv-jx; m = jx+jk;
    for (i=0; i<=m; i++,j++)
        f[i] = j<0 ? 0.0 : (double)ipio2[j];

    /* compute q[0],q[1],...q[jk] */
    for (i=0; i<=jk; i++) {
        for (j=0,fw=0.0; j<=jx; j++)
            fw += x[j]*f[jx+i-j];
        q[i] = fw;
    }

    jz = jk;
recompute:
    /* distill q[] into iq[] reversingly */
    for (i=0,j=jz,z=q[jz]; j>0; i++,j--) {
        fw    = (double)(int32_t)(0x1p-24*z);
        iq[i] = (int32_t)(z - 0x1p24*fw);
        z     = q[j-1]+fw;
    }

    /* compute n */
    z  = scalbn(z,q0);       /* actual value of z */
    z -= 8.0*floor(z*0.125); /* trim off integer >= 8 */
    n  = (int32_t)z;
    z -= (double)n;
    ih = 0;
    if (q0 > 0) {  /* need iq[jz-1] to determine n */
        i  = iq[jz-1]>>(24-q0); n += i;
        iq[jz-1] -= i<<(24-q0);
        ih = iq[jz-1]>>(23-q0);
    }
    else if (q0 == 0) ih = iq[jz-1]>>23;
    else if (z >= 0.5) ih = 2;

    if (ih > 0) {  /* q > 0.5 */
        n += 1; carry = 0;
        for (i=0; i<jz; i++) {  /* compute 1-q */
            j = iq[i];
            if (carry == 0) {
                if (j != 0) {
                    carry = 1;
                    iq[i] = 0x1000000 - j;
                }
            } else
                iq[i] = 0xffffff - j;
        }
        if (q0 > 0) {  /* rare case: chance is 1 in 12 */
            switch(q0) {
            case 1:
                iq[jz-1] &= 0x7fffff; break;
            case 2:
                iq[jz-1] &= 0x3fffff; break;
            }
        }
        if (ih == 2) {
            z = 1.0 - z;
            if (carry != 0)
                z -= scalbn(1.0,q0);
        }
    }

    /* check if recomputation is needed */
    if (z == 0.0) {
        j = 0;
        for (i=jz-1; i>=jk; i--) j |= iq[i];
        if (j == 0) {  /* need recomputation */
            for (__rem_pio2_large_c__k=1; iq[jk-__rem_pio2_large_c__k]==0; __rem_pio2_large_c__k++);  /* __rem_pio2_large_c__k = no. of terms needed */

            for (i=jz+1; i<=jz+__rem_pio2_large_c__k; i++) {  /* add q[jz+1] to q[jz+__rem_pio2_large_c__k] */
                f[jx+i] = (double)ipio2[jv+i];
                for (j=0,fw=0.0; j<=jx; j++)
                    fw += x[j]*f[jx+i-j];
                q[i] = fw;
            }
            jz += __rem_pio2_large_c__k;
            goto recompute;
        }
    }

    /* chop off __rem_pio2_large_c__zero terms */
    if (z == 0.0) {
        jz -= 1;
        q0 -= 24;
        while (iq[jz] == 0) {
            jz--;
            q0 -= 24;
        }
    } else { /* break z into 24-bit if necessary */
        z = scalbn(z,-q0);
        if (z >= 0x1p24) {
            fw = (double)(int32_t)(0x1p-24*z);
            iq[jz] = (int32_t)(z - 0x1p24*fw);
            jz += 1;
            q0 += 24;
            iq[jz] = (int32_t)fw;
        } else
            iq[jz] = (int32_t)z;
    }

    /* convert integer "bit" chunk to floating-point value */
    fw = scalbn(1.0,q0);
    for (i=jz; i>=0; i--) {
        q[i] = fw*(double)iq[i];
        fw *= 0x1p-24;
    }

    /* compute PIo2[0,...,jp]*q[jz,...,0] */
    for(i=jz; i>=0; i--) {
        for (fw=0.0,__rem_pio2_large_c__k=0; __rem_pio2_large_c__k<=jp && __rem_pio2_large_c__k<=jz-i; __rem_pio2_large_c__k++)
            fw += PIo2[__rem_pio2_large_c__k]*q[i+__rem_pio2_large_c__k];
        fq[jz-i] = fw;
    }

    /* compress fq[] into y[] */
    switch(prec) {
    case 0:
        fw = 0.0;
        for (i=jz; i>=0; i--)
            fw += fq[i];
        y[0] = ih==0 ? fw : -fw;
        break;
    case 1:
    case 2:
        fw = 0.0;
        for (i=jz; i>=0; i--)
            fw += fq[i];
        // TODO: drop excess precision here once double_t is used
        fw = (double)fw;
        y[0] = ih==0 ? fw : -fw;
        fw = fq[0]-fw;
        for (i=1; i<=jz; i++)
            fw += fq[i];
        y[1] = ih==0 ? fw : -fw;
        break;
    case 3:  /* painful */
        for (i=jz; i>0; i--) {
            fw      = fq[i-1]+fq[i];
            fq[i]  += fq[i-1]-fw;
            fq[i-1] = fw;
        }
        for (i=jz; i>1; i--) {
            fw      = fq[i-1]+fq[i];
            fq[i]  += fq[i-1]-fw;
            fq[i-1] = fw;
        }
        for (fw=0.0,i=jz; i>=2; i--)
            fw += fq[i];
        if (ih==0) {
            y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
        } else {
            y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
        }
    }
    return n&7;
}

/// >>> START src/math/__rem_pio2f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_rem_pio2f.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Debugged and optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* __rem_pio2f(x,y)
 *
 * return the remainder of x rem __rem_pio2f_c__pi/2 in *y
 * use double precision for everything except passing x
 * use __rem_pio2_large() for large x
 */

/*
 * __rem_pio2f_c__invpio2:  53 bits of 2/__rem_pio2f_c__pi
 * __rem_pio2f_c__pio2_1:   first 25 bits of __rem_pio2f_c__pi/2
 * __rem_pio2f_c__pio2_1t:  __rem_pio2f_c__pi/2 - __rem_pio2f_c__pio2_1
 */
static const double
__rem_pio2f_c__invpio2 = 6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
__rem_pio2f_c__pio2_1  = 1.57079631090164184570e+00, /* 0x3FF921FB, 0x50000000 */
__rem_pio2f_c__pio2_1t = 1.58932547735281966916e-08; /* 0x3E5110b4, 0x611A6263 */

int __rem_pio2f(float x, double *y) {
    union {float f; uint32_t i;} u = {x};
    double tx[1],ty[1],fn;
    uint32_t ix;
    int n, sign, e0;

    ix = u.i & 0x7fffffff;
    /* 25+53 bit __rem_pio2f_c__pi is good enough for medium size */
    if (ix < 0x4dc90fdb) {  /* |x| ~< 2^28*(__rem_pio2f_c__pi/2), medium size */
        /* Use a specialized rint() to get fn.  Assume round-to-nearest. */
        fn = x*__rem_pio2f_c__invpio2 + 0x1.8p52;
        fn = fn - 0x1.8p52;
        n  = (int32_t)fn;
        *y = x - fn*__rem_pio2f_c__pio2_1 - fn*__rem_pio2f_c__pio2_1t;
        return n;
    }
    if(ix>=0x7f800000) {  /* x is inf or NaN */
        *y = x-x;
        return 0;
    }
    /* scale x into [2^23, 2^24-1] */
    sign = u.i>>31;
    e0 = (ix>>23) - (0x7f+23);  /* e0 = ilogb(|x|)-23, positive */
    u.i = ix - (e0<<23);
    tx[0] = u.f;
    n  =  __rem_pio2_large(tx,ty,e0,1,0);
    if (sign) {
        *y = -ty[0];
        return -n;
    }
    *y = ty[0];
    return n;
}

/// >>> START src/math/__rem_pio2l.c
/* origin: FreeBSD /usr/src/lib/msun/ld80/e_rem_pio2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * Optimized by Bruce D. Evans.
 */
#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
/* ld80 and ld128 version of __rem_pio2(x,y)
 *
 * return the remainder of x rem __rem_pio2l_c__pi/2 in y[0]+y[1]
 * use __rem_pio2_large() for large x
 */

#if LDBL_MANT_DIG == 64
/* u ~< 0x1p25*__rem_pio2l_c__pi/2 */
#undef SMALL
#define SMALL(u) (((u.i.se & 0x7fffU)<<16 | u.i.m>>48) < ((0x3fff + 25)<<16 | 0x921f>>1 | 0x8000))
#undef TOINT
#define TOINT 0x1.8p63
#undef QUOBITS
#define QUOBITS(x) ((uint32_t)(int32_t)x & 0x7fffffff)
#undef ROUND1
#define ROUND1 22
#undef ROUND2
#define ROUND2 61
#undef NX
#define NX 3
#undef NY
#define NY 2
/*
 * __rem_pio2l_c__invpio2:  64 bits of 2/__rem_pio2l_c__pi
 * __rem_pio2l_c__pio2_1:   first  39 bits of __rem_pio2l_c__pi/2
 * __rem_pio2l_c__pio2_1t:  __rem_pio2l_c__pi/2 - __rem_pio2l_c__pio2_1
 * __rem_pio2l_c__pio2_2:   second 39 bits of __rem_pio2l_c__pi/2
 * __rem_pio2l_c__pio2_2t:  __rem_pio2l_c__pi/2 - (__rem_pio2l_c__pio2_1+__rem_pio2l_c__pio2_2)
 * __rem_pio2l_c__pio2_3:   third  39 bits of __rem_pio2l_c__pi/2
 * __rem_pio2l_c__pio2_3t:  __rem_pio2l_c__pi/2 - (__rem_pio2l_c__pio2_1+__rem_pio2l_c__pio2_2+__rem_pio2l_c__pio2_3)
 */
static const double
__rem_pio2l_c__pio2_1 =  1.57079632679597125389e+00, /* 0x3FF921FB, 0x54444000 */
__rem_pio2l_c__pio2_2 = -1.07463465549783099519e-12, /* -0x12e7b967674000.0p-92 */
__rem_pio2l_c__pio2_3 =  6.36831716351370313614e-25; /*  0x18a2e037074000.0p-133 */
static const long double
__rem_pio2l_c__invpio2 =  6.36619772367581343076e-01L, /*  0xa2f9836e4e44152a.0p-64 */
__rem_pio2l_c__pio2_1t = -1.07463465549719416346e-12L, /* -0x973dcb3b399d747f.0p-103 */
__rem_pio2l_c__pio2_2t =  6.36831716351095013979e-25L, /*  0xc51701b839a25205.0p-144 */
__rem_pio2l_c__pio2_3t = -2.75299651904407171810e-37L; /* -0xbb5bf6c7ddd660ce.0p-185 */
#elif LDBL_MANT_DIG == 113
/* u ~< 0x1p45*__rem_pio2l_c__pi/2 */
#undef SMALL
#define SMALL(u) (((u.i.se & 0x7fffU)<<16 | u.i.top) < ((0x3fff + 45)<<16 | 0x921f))
#undef TOINT
#define TOINT 0x1.8p112
#undef QUOBITS
#define QUOBITS(x) ((uint32_t)(int64_t)x & 0x7fffffff)
#undef ROUND1
#define ROUND1 51
#undef ROUND2
#define ROUND2 119
#undef NX
#define NX 5
#undef NY
#define NY 3
static const long double
__rem_pio2l_c__invpio2 =  6.3661977236758134307553505349005747e-01L,    /*  0x145f306dc9c882a53f84eafa3ea6a.0p-113 */
__rem_pio2l_c__pio2_1  =  1.5707963267948966192292994253909555e+00L,    /*  0x1921fb54442d18469800000000000.0p-112 */
__rem_pio2l_c__pio2_1t =  2.0222662487959507323996846200947577e-21L,    /*  0x13198a2e03707344a4093822299f3.0p-181 */
__rem_pio2l_c__pio2_2  =  2.0222662487959507323994779168837751e-21L,    /*  0x13198a2e03707344a400000000000.0p-181 */
__rem_pio2l_c__pio2_2t =  2.0670321098263988236496903051604844e-43L,    /*  0x127044533e63a0105df531d89cd91.0p-254 */
__rem_pio2l_c__pio2_3  =  2.0670321098263988236499468110329591e-43L,    /*  0x127044533e63a0105e00000000000.0p-254 */
__rem_pio2l_c__pio2_3t = -2.5650587247459238361625433492959285e-65L;    /* -0x159c4ec64ddaeb5f78671cbfb2210.0p-327 */
#endif

int __rem_pio2l(long double x, long double *y) {
    union ldshape u,uz;
    long double z,w,t,r,fn;
    double tx[NX],ty[NY];
    int ex,ey,n,i;

    u.f = x;
    ex = u.i.se & 0x7fff;
    if (SMALL(u)) {
        /* rint(x/(__rem_pio2l_c__pi/2)), Assume round-to-nearest. */
        fn = x*__rem_pio2l_c__invpio2 + TOINT - TOINT;
        n = QUOBITS(fn);
        r = x-fn*__rem_pio2l_c__pio2_1;
        w = fn*__rem_pio2l_c__pio2_1t;  /* 1st round good to 102/180 bits (ld80/ld128) */
        y[0] = r-w;
        u.f = y[0];
        ey = u.i.se & 0x7fff;
        if (ex - ey > ROUND1) {  /* 2nd iteration needed, good to 141/248 (ld80/ld128) */
            t = r;
            w = fn*__rem_pio2l_c__pio2_2;
            r = t-w;
            w = fn*__rem_pio2l_c__pio2_2t-((t-r)-w);
            y[0] = r-w;
            u.f = y[0];
            ey = u.i.se & 0x7fff;
            if (ex - ey > ROUND2) {  /* 3rd iteration, good to 180/316 bits */
                t = r; /* will cover all possible cases (not verified for ld128) */
                w = fn*__rem_pio2l_c__pio2_3;
                r = t-w;
                w = fn*__rem_pio2l_c__pio2_3t-((t-r)-w);
                y[0] = r-w;
            }
        }
        y[1] = (r - y[0]) - w;
        return n;
    }
    /*
     * all other (large) arguments
     */
    if (ex == 0x7fff) {                /* x is inf or NaN */
        y[0] = y[1] = x - x;
        return 0;
    }
    /* set z = scalbn(|x|,-ilogb(x)+23) */
    uz.f = x;
    uz.i.se = 0x3fff + 23;
    z = uz.f;
    for (i=0; i < NX - 1; i++) {
        tx[i] = (double)(int32_t)z;
        z     = (z-tx[i])*0x1p24;
    }
    tx[i] = z;
    while (tx[i] == 0)
        i--;
    n = __rem_pio2_large(tx, ty, ex-0x3fff-23, i+1, NY);
    w = ty[1];
    if (NY == 3)
        w += ty[2];
    r = ty[0] + w;
    /* TODO: for ld128 this does not follow the recommendation of the
    comments of __rem_pio2_large which seem wrong if |ty[0]| > |ty[1]+ty[2]| */
    w -= r - ty[0];
    if (u.i.se >> 15) {
        y[0] = -r;
        y[1] = -w;
        return -n;
    }
    y[0] = r;
    y[1] = w;
    return n;
}
#endif

/// >>> START src/math/__signbit.c

// FIXME: macro in math.h
int __signbit(double x) {
    union {
        double d;
        uint64_t i;
    } y = { x };
    return y.i>>63;
}

/// >>> START src/math/__signbitf.c

// FIXME: macro in math.h
int __signbitf(float x) {
    union {
        float f;
        uint32_t i;
    } y = { x };
    return y.i>>31;
}

/// >>> START src/math/__signbitl.c

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
int __signbitl(long double x) {
    union ldshape u = {x};
    return u.i.se >> 15;
}
#endif

/// >>> START src/math/__sin.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_sin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* __sin( x, y, iy)
 * kernel sin function on ~[-__sin_c__pi/4, __sin_c__pi/4] (except on -0), __sin_c__pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~__sin_c__pi/4 in magnitude.
 * Input y is the tail of x.
 * Input iy indicates whether y is 0. (if iy=0, y assume to be 0).
 *
 * Algorithm
 *      1. Since sin(-x) = -sin(x), we need only to consider positive x.
 *      2. Callers must return sin(-0) = -0 without calling here since our
 *         odd polynomial is not evaluated in a way that preserves -0.
 *         Callers may do the optimization sin(x) ~ x for __sin_c__tiny x.
 *      3. sin(x) is approximated by a polynomial of degree 13 on
 *         [0,__sin_c__pi/4]
 *                               3            13
 *              sin(x) ~ x + __sin_c__S1*x + ... + __sin_c__S6*x
 *         where
 *
 *      |sin(x)         2     4     6     8     10     12  |     -58
 *      |----- - (1+__sin_c__S1*x +__sin_c__S2*x +__sin_c__S3*x +__sin_c__S4*x +__sin_c__S5*x  +__sin_c__S6*x   )| <= 2
 *      |  x                                               |
 *
 *      4. sin(x+y) = sin(x) + sin'(x')*y
 *                  ~ sin(x) + (1-x*x/2)*y
 *         For better accuracy, let
 *                   3      2      2      2      2
 *              r = x *(__sin_c__S2+x *(__sin_c__S3+x *(__sin_c__S4+x *(__sin_c__S5+x *__sin_c__S6))))
 *         then                   3    2
 *              sin(x) = x + (__sin_c__S1*x + (x *(r-y/2)+y))
 */

static const double
__sin_c__S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
__sin_c__S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
__sin_c__S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
__sin_c__S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
__sin_c__S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
__sin_c__S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

double __sin(double x, double y, int iy) {
    double_t z,r,v,w;

    z = x*x;
    w = z*z;
    r = __sin_c__S2 + z*(__sin_c__S3 + z*__sin_c__S4) + z*w*(__sin_c__S5 + z*__sin_c__S6);
    v = z*x;
    if (iy == 0)
        return x + v*(__sin_c__S1 + z*r);
    else
        return x - ((z*(0.5*y - v*r) - y) - v*__sin_c__S1);
}

/// >>> START src/math/__sindf.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_sinf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* |sin(x)/x - s(x)| < 2**-37.5 (~[-4.89e-12, 4.824e-12]). */
static const double
__sindf_c__S1 = -0x15555554cbac77.0p-55, /* -0.166666666416265235595 */
__sindf_c__S2 =  0x111110896efbb2.0p-59, /*  0.0083333293858894631756 */
__sindf_c__S3 = -0x1a00f9e2cae774.0p-65, /* -0.000198393348360966317347 */
__sindf_c__S4 =  0x16cd878c3b46a7.0p-71; /*  0.0000027183114939898219064 */

float __sindf(double x) {
    double_t r, s, w, z;

    /* Try to optimize for parallel evaluation as in __tandf.c. */
    z = x*x;
    w = z*z;
    r = __sindf_c__S3 + z*__sindf_c__S4;
    s = z*x;
    return (x + s*(__sindf_c__S1 + z*__sindf_c__S2)) + s*w*r;
}

/// >>> START src/math/__sinl.c
/* origin: FreeBSD /usr/src/lib/msun/ld80/k_sinl.c */
/* origin: FreeBSD /usr/src/lib/msun/ld128/k_sinl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
/*
 * ld80 version of __sin.c.  See __sin.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-1.89e-22, 1.915e-22]
 * |sin(x)/x - s(x)| < 2**-72.1
 *
 * See __cosl.c for more details about the polynomial.
 */
static const long double
__sinl_c__S1 = -0.166666666666666666671L;   /* -0xaaaaaaaaaaaaaaab.0p-66 */
static const double
__sinl_c__S2 =  0.0083333333333333332,      /*  0x11111111111111.0p-59 */
__sinl_c__S3 = -0.00019841269841269427,     /* -0x1a01a01a019f81.0p-65 */
__sinl_c__S4 =  0.0000027557319223597490,   /*  0x171de3a55560f7.0p-71 */
__sinl_c__S5 = -0.000000025052108218074604, /* -0x1ae64564f16cad.0p-78 */
__sinl_c__S6 =  1.6059006598854211e-10,     /*  0x161242b90243b5.0p-85 */
__sinl_c__S7 = -7.6429779983024564e-13,     /* -0x1ae42ebd1b2e00.0p-93 */
__sinl_c__S8 =  2.6174587166648325e-15;     /*  0x179372ea0b3f64.0p-101 */
#undef POLY
#define POLY(z) (__sinl_c__S2+z*(__sinl_c__S3+z*(__sinl_c__S4+z*(__sinl_c__S5+z*(__sinl_c__S6+z*(__sinl_c__S7+z*__sinl_c__S8))))))
#elif LDBL_MANT_DIG == 113
/*
 * ld128 version of __sin.c.  See __sin.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-1.53e-37, 1.659e-37]
 * |sin(x)/x - s(x)| < 2**-122.1
 *
 * See __cosl.c for more details about the polynomial.
 */
static const long double
__sinl_c__S1 = -0.16666666666666666666666666666666666606732416116558L,
__sinl_c__S2 =  0.0083333333333333333333333333333331135404851288270047L,
__sinl_c__S3 = -0.00019841269841269841269841269839935785325638310428717L,
__sinl_c__S4 =  0.27557319223985890652557316053039946268333231205686e-5L,
__sinl_c__S5 = -0.25052108385441718775048214826384312253862930064745e-7L,
__sinl_c__S6 =  0.16059043836821614596571832194524392581082444805729e-9L,
__sinl_c__S7 = -0.76471637318198151807063387954939213287488216303768e-12L,
__sinl_c__S8 =  0.28114572543451292625024967174638477283187397621303e-14L;
static const double
__sinl_c__S9  = -0.82206352458348947812512122163446202498005154296863e-17,
S10 =  0.19572940011906109418080609928334380560135358385256e-19,
S11 = -0.38680813379701966970673724299207480965452616911420e-22,
S12 =  0.64038150078671872796678569586315881020659912139412e-25;
#undef POLY
#define POLY(z) (__sinl_c__S2+z*(__sinl_c__S3+z*(__sinl_c__S4+z*(__sinl_c__S5+z*(__sinl_c__S6+z*(__sinl_c__S7+z*(__sinl_c__S8+ \
    z*(__sinl_c__S9+z*(S10+z*(S11+z*S12))))))))))
#endif

long double __sinl(long double x, long double y, int iy) {
    long double z,r,v;

    z = x*x;
    v = z*x;
    r = POLY(z);
    if (iy == 0)
        return x+v*(__sinl_c__S1+z*r);
    return x-((z*(0.5*y-v*r)-y)-v*__sinl_c__S1);
}
#endif

/// >>> START src/math/__tan.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_tan.c */
/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* __tan( x, y, __tan_c__k )
 * kernel tan function on ~[-__tan_c__pi/4, __tan_c__pi/4] (except on -0), __tan_c__pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~__tan_c__pi/4 in magnitude.
 * Input y is the tail of x.
 * Input odd indicates whether tan (if odd = 0) or -1/tan (if odd = 1) is returned.
 *
 * Algorithm
 *      1. Since tan(-x) = -tan(x), we need only to consider positive x.
 *      2. Callers must return tan(-0) = -0 without calling here since our
 *         odd polynomial is not evaluated in a way that preserves -0.
 *         Callers may do the optimization tan(x) ~ x for __tan_c__tiny x.
 *      3. tan(x) is approximated by a odd polynomial of degree 27 on
 *         [0,0.67434]
 *                               3             27
 *              tan(x) ~ x + T1*x + ... + T13*x
 *         where
 *
 *              |tan(x)         2     4            26   |     -59.2
 *              |----- - (1+T1*x +T2*x +.... +T13*x    )| <= 2
 *              |  x                                    |
 *
 *         Note: tan(x+y) = tan(x) + tan'(x)*y
 *                        ~ tan(x) + (1+x*x)*y
 *         Therefore, for better accuracy in computing tan(x+y), let
 *                   3      2      2       2       2
 *              r = x *(T2+x *(T3+x *(...+x *(T12+x *T13))))
 *         then
 *                                  3    2
 *              tan(x+y) = x + (T1*x + (x *(r+y)+y))
 *
 *      4. For x in [0.67434,__tan_c__pi/4],  let y = __tan_c__pi/4 - x, then
 *              tan(x) = tan(__tan_c__pi/4-y) = (1-tan(y))/(1+tan(y))
 *                     = 1 - 2*(tan(y) - (tan(y)^2)/(1+tan(y)))
 */

static const double __tan_c__T[] = {
             3.33333333333334091986e-01, /* 3FD55555, 55555563 */
             1.33333333333201242699e-01, /* 3FC11111, 1110FE7A */
             5.39682539762260521377e-02, /* 3FABA1BA, 1BB341FE */
             2.18694882948595424599e-02, /* 3F9664F4, 8406D637 */
             8.86323982359930005737e-03, /* 3F8226E3, E96E8493 */
             3.59207910759131235356e-03, /* 3F6D6D22, C9560328 */
             1.45620945432529025516e-03, /* 3F57DBC8, FEE08315 */
             5.88041240820264096874e-04, /* 3F4344D8, F2F26501 */
             2.46463134818469906812e-04, /* 3F3026F7, 1A8D1068 */
             7.81794442939557092300e-05, /* 3F147E88, A03792A6 */
             7.14072491382608190305e-05, /* 3F12B80F, 32F0A7E9 */
            -1.85586374855275456654e-05, /* BEF375CB, DB605373 */
             2.59073051863633712884e-05, /* 3EFB2A70, 74BF7AD4 */
},
__tan_c__pio4 =       7.85398163397448278999e-01, /* 3FE921FB, 54442D18 */
__tan_c__pio4lo =     3.06161699786838301793e-17; /* 3C81A626, 33145C07 */

double __tan(double x, double y, int odd) {
    double_t z, r, v, w, s, a;
    double __tan_c__w0, __tan_c__a0;
    uint32_t hx;
    int big, sign;

    GET_HIGH_WORD(hx,x);
    big = (hx&0x7fffffff) >= 0x3FE59428; /* |x| >= 0.6744 */
    if (big) {
        sign = hx>>31;
        if (sign) {
            x = -x;
            y = -y;
        }
        x = (__tan_c__pio4 - x) + (__tan_c__pio4lo - y);
        y = 0.0;
    }
    z = x * x;
    w = z * z;
    /*
     * Break x^5*(__tan_c__T[1]+x^2*__tan_c__T[2]+...) into
     * x^5(__tan_c__T[1]+x^4*__tan_c__T[3]+...+x^20*__tan_c__T[11]) +
     * x^5(x^2*(__tan_c__T[2]+x^4*__tan_c__T[4]+...+x^22*[T12]))
     */
    r = __tan_c__T[1] + w*(__tan_c__T[3] + w*(__tan_c__T[5] + w*(__tan_c__T[7] + w*(__tan_c__T[9] + w*__tan_c__T[11]))));
    v = z*(__tan_c__T[2] + w*(__tan_c__T[4] + w*(__tan_c__T[6] + w*(__tan_c__T[8] + w*(__tan_c__T[10] + w*__tan_c__T[12])))));
    s = z * x;
    r = y + z*(s*(r + v) + y) + s*__tan_c__T[0];
    w = x + r;
    if (big) {
        s = 1 - 2*odd;
        v = s - 2.0 * (x + (r - w*w/(w + s)));
        return sign ? -v : v;
    }
    if (!odd)
        return w;
    /* -1.0/(x+r) has up to 2ulp error, so compute it accurately */
    __tan_c__w0 = w;
    SET_LOW_WORD(__tan_c__w0, 0);
    v = r - (__tan_c__w0 - x);       /* __tan_c__w0+v = r+x */
    __tan_c__a0 = a = -1.0 / w;
    SET_LOW_WORD(__tan_c__a0, 0);
    return __tan_c__a0 + a*(1.0 + __tan_c__a0*__tan_c__w0 + __tan_c__a0*v);
}

/// >>> START src/math/__tandf.c
/* origin: FreeBSD /usr/src/lib/msun/src/k_tanf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* |tan(x)/x - t(x)| < 2**-25.5 (~[-2e-08, 2e-08]). */
static const double __tandf_c__T[] = {
  0x15554d3418c99f.0p-54, /* 0.333331395030791399758 */
  0x1112fd38999f72.0p-55, /* 0.133392002712976742718 */
  0x1b54c91d865afe.0p-57, /* 0.0533812378445670393523 */
  0x191df3908c33ce.0p-58, /* 0.0245283181166547278873 */
  0x185dadfcecf44e.0p-61, /* 0.00297435743359967304927 */
  0x1362b9bf971bcd.0p-59, /* 0.00946564784943673166728 */
};

float __tandf(double x, int odd) {
    double_t z,r,w,s,t,u;

    z = x*x;
    /*
     * Split up the polynomial into small independent terms to give
     * opportunities for parallel evaluation.  The chosen splitting is
     * micro-optimized for Athlons (XP, X64).  It costs 2 multiplications
     * relative to Horner's method on sequential machines.
     *
     * We add the small terms from lowest degree up for efficiency on
     * non-sequential machines (the lowest degree terms tend to be ready
     * earlier).  Apart from this, we don't care about order of
     * operations, and don't need to to care since we have precision to
     * spare.  However, the chosen splitting is good for accuracy too,
     * and would give results as accurate as Horner's method if the
     * small terms were added from highest degree down.
     */
    r = __tandf_c__T[4] + z*__tandf_c__T[5];
    t = __tandf_c__T[2] + z*__tandf_c__T[3];
    w = z*z;
    s = z*x;
    u = __tandf_c__T[0] + z*__tandf_c__T[1];
    r = (x + s*u) + (s*w)*(t + w*r);
    return odd ? -1.0/r : r;
}

/// >>> START src/math/__tanl.c
/* origin: FreeBSD /usr/src/lib/msun/ld80/k_tanl.c */
/* origin: FreeBSD /usr/src/lib/msun/ld128/k_tanl.c */
/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
/*
 * ld80 version of __tan.c.  See __tan.c for most comments.
 */
/*
 * Domain [-0.67434, 0.67434], range ~[-2.25e-22, 1.921e-22]
 * |tan(x)/x - t(x)| < 2**-71.9
 *
 * See __cosl.c for more details about the polynomial.
 */
static const long double
T3 =  0.333333333333333333180L,         /*  0xaaaaaaaaaaaaaaa5.0p-65 */
T5 =  0.133333333333333372290L,         /*  0x88888888888893c3.0p-66 */
T7 =  0.0539682539682504975744L,        /*  0xdd0dd0dd0dc13ba2.0p-68 */
__tanl_c__pio4   =  0.785398163397448309628L,     /*  0xc90fdaa22168c235.0p-64 */
__tanl_c__pio4lo = -1.25413940316708300586e-20L;  /* -0xece675d1fc8f8cbb.0p-130 */
static const double
T9  =  0.021869488536312216,            /*  0x1664f4882cc1c2.0p-58 */
T11 =  0.0088632355256619590,           /*  0x1226e355c17612.0p-59 */
T13 =  0.0035921281113786528,           /*  0x1d6d3d185d7ff8.0p-61 */
T15 =  0.0014558334756312418,           /*  0x17da354aa3f96b.0p-62 */
T17 =  0.00059003538700862256,          /*  0x13559358685b83.0p-63 */
T19 =  0.00023907843576635544,          /*  0x1f56242026b5be.0p-65 */
T21 =  0.000097154625656538905,         /*  0x1977efc26806f4.0p-66 */
T23 =  0.000038440165747303162,         /*  0x14275a09b3ceac.0p-67 */
T25 =  0.000018082171885432524,         /*  0x12f5e563e5487e.0p-68 */
T27 =  0.0000024196006108814377,        /*  0x144c0d80cc6896.0p-71 */
T29 =  0.0000078293456938132840,        /*  0x106b59141a6cb3.0p-69 */
T31 = -0.0000032609076735050182,        /* -0x1b5abef3ba4b59.0p-71 */
T33 =  0.0000023261313142559411;        /*  0x13835436c0c87f.0p-71 */
#undef RPOLY
#define RPOLY(w) (T5 + w * (T9 + w * (T13 + w * (T17 + w * (T21 + \
    w * (T25 + w * (T29 + w * T33)))))))
#undef VPOLY
#define VPOLY(w) (T7 + w * (T11 + w * (T15 + w * (T19 + w * (T23 + \
    w * (T27 + w * T31))))))
#elif LDBL_MANT_DIG == 113
/*
 * ld128 version of __tan.c.  See __tan.c for most comments.
 */
/*
 * Domain [-0.67434, 0.67434], range ~[-3.37e-36, 1.982e-37]
 * |tan(x)/x - t(x)| < 2**-117.8 (XXX should be ~1e-37)
 *
 * See __cosl.c for more details about the polynomial.
 */
static const long double
T3 = 0x1.5555555555555555555555555553p-2L,
T5 = 0x1.1111111111111111111111111eb5p-3L,
T7 = 0x1.ba1ba1ba1ba1ba1ba1ba1b694cd6p-5L,
T9 = 0x1.664f4882c10f9f32d6bbe09d8bcdp-6L,
T11 = 0x1.226e355e6c23c8f5b4f5762322eep-7L,
T13 = 0x1.d6d3d0e157ddfb5fed8e84e27b37p-9L,
T15 = 0x1.7da36452b75e2b5fce9ee7c2c92ep-10L,
T17 = 0x1.355824803674477dfcf726649efep-11L,
T19 = 0x1.f57d7734d1656e0aceb716f614c2p-13L,
T21 = 0x1.967e18afcb180ed942dfdc518d6cp-14L,
T23 = 0x1.497d8eea21e95bc7e2aa79b9f2cdp-15L,
T25 = 0x1.0b132d39f055c81be49eff7afd50p-16L,
T27 = 0x1.b0f72d33eff7bfa2fbc1059d90b6p-18L,
T29 = 0x1.5ef2daf21d1113df38d0fbc00267p-19L,
T31 = 0x1.1c77d6eac0234988cdaa04c96626p-20L,
T33 = 0x1.cd2a5a292b180e0bdd701057dfe3p-22L,
T35 = 0x1.75c7357d0298c01a31d0a6f7d518p-23L,
T37 = 0x1.2f3190f4718a9a520f98f50081fcp-24L,
__tanl_c__pio4 = 0x1.921fb54442d18469898cc51701b8p-1L,
__tanl_c__pio4lo = 0x1.cd129024e088a67cc74020bbea60p-116L;
static const double
T39 =  0.000000028443389121318352,    /*  0x1e8a7592977938.0p-78 */
T41 =  0.000000011981013102001973,    /*  0x19baa1b1223219.0p-79 */
T43 =  0.0000000038303578044958070,    /*  0x107385dfb24529.0p-80 */
T45 =  0.0000000034664378216909893,    /*  0x1dc6c702a05262.0p-81 */
T47 = -0.0000000015090641701997785,    /* -0x19ecef3569ebb6.0p-82 */
T49 =  0.0000000029449552300483952,    /*  0x194c0668da786a.0p-81 */
T51 = -0.0000000022006995706097711,    /* -0x12e763b8845268.0p-81 */
T53 =  0.0000000015468200913196612,    /*  0x1a92fc98c29554.0p-82 */
T55 = -0.00000000061311613386849674,    /* -0x151106cbc779a9.0p-83 */
T57 =  1.4912469681508012e-10;        /*  0x147edbdba6f43a.0p-85 */
#undef RPOLY
#define RPOLY(w) (T5 + w * (T9 + w * (T13 + w * (T17 + w * (T21 + \
    w * (T25 + w * (T29 + w * (T33 + w * (T37 + w * (T41 + \
    w * (T45 + w * (T49 + w * (T53 + w * T57)))))))))))))
#undef VPOLY
#define VPOLY(w) (T7 + w * (T11 + w * (T15 + w * (T19 + w * (T23 + \
    w * (T27 + w * (T31 + w * (T35 + w * (T39 + w * (T43 + \
    w * (T47 + w * (T51 + w * T55))))))))))))
#endif

long double __tanl(long double x, long double y, int odd) {
    long double z, r, v, w, s, a, t;
    int big, sign;

    big = fabsl(x) >= 0.67434;
    if (big) {
        sign = 0;
        if (x < 0) {
            sign = 1;
            x = -x;
            y = -y;
        }
        x = (__tanl_c__pio4 - x) + (__tanl_c__pio4lo - y);
        y = 0.0;
    }
    z = x * x;
    w = z * z;
    r = RPOLY(w);
    v = z * VPOLY(w);
    s = z * x;
    r = y + z * (s * (r + v) + y) + T3 * s;
    w = x + r;
    if (big) {
        s = 1 - 2*odd;
        v = s - 2.0 * (x + (r - w * w / (w + s)));
        return sign ? -v : v;
    }
    if (!odd)
        return w;
    /*
     * if allow error up to 2 ulp, simply return
     * -1.0 / (x+r) here
     */
    /* compute -1.0 / (x+r) accurately */
    z = w;
    z = z + 0x1p32 - 0x1p32;
    v = r - (z - x);        /* z+v = r+x */
    t = a = -1.0 / w;       /* a = -1.0/w */
    t = t + 0x1p32 - 0x1p32;
    s = 1.0 + t * z;
    return t + a * (s + t * v);
}
#endif

/// >>> START src/math/acos.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_acos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* acos(x)
 * Method :
 *      acos(x)  = acos_c__pi/2 - asin(x)
 *      acos(-x) = acos_c__pi/2 + asin(x)
 * For |x|<=0.5
 *      acos(x) = acos_c__pi/2 - (x + x*x^2*acos_c__R(x^2))     (see asin.c)
 * For x>0.5
 *      acos(x) = acos_c__pi/2 - (acos_c__pi/2 - 2asin(sqrt((1-x)/2)))
 *              = 2asin(sqrt((1-x)/2))
 *              = 2s + 2s*z*acos_c__R(z)        ...z=(1-x)/2, s=sqrt(z)
 *              = 2f + (2c + 2s*z*acos_c__R(z))
 *     where f=hi part of s, and c = (z-f*f)/(s+f) is the correction term
 *     for f so that f+c ~ sqrt(z).
 * For x<-0.5
 *      acos(x) = acos_c__pi - 2asin(sqrt((1-|x|)/2))
 *              = acos_c__pi - 0.5*(s+s*z*acos_c__R(z)), where z=(1-|x|)/2,s=sqrt(z)
 *
 * Special cases:
 *      if x is NaN, return x itself;
 *      if |x|>1, return NaN with invalid signal.
 *
 * Function needed: sqrt
 */

static const double
acos_c__pio2_hi = 1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
acos_c__pio2_lo = 6.12323399573676603587e-17, /* 0x3C91A626, 0x33145C07 */
acos_c__pS0 =  1.66666666666666657415e-01, /* 0x3FC55555, 0x55555555 */
acos_c__pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
acos_c__pS2 =  2.01212532134862925881e-01, /* 0x3FC9C155, 0x0E884455 */
acos_c__pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
acos_c__pS4 =  7.91534994289814532176e-04, /* 0x3F49EFE0, 0x7501B288 */
acos_c__pS5 =  3.47933107596021167570e-05, /* 0x3F023DE1, 0x0DFDF709 */
acos_c__qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
acos_c__qS2 =  2.02094576023350569471e+00, /* 0x40002AE5, 0x9C598AC8 */
acos_c__qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
acos_c__qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

static double acos_c__R(double z) {
    double_t p, q;
    p = z*(acos_c__pS0+z*(acos_c__pS1+z*(acos_c__pS2+z*(acos_c__pS3+z*(acos_c__pS4+z*acos_c__pS5)))));
    q = 1.0+z*(acos_c__qS1+z*(acos_c__qS2+z*(acos_c__qS3+z*acos_c__qS4)));
    return p/q;
}

double acos(double x) {
    double z,w,s,c,df;
    uint32_t hx,ix;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    /* |x| >= 1 or nan */
    if (ix >= 0x3ff00000) {
        uint32_t lx;

        GET_LOW_WORD(lx,x);
        if ((ix-0x3ff00000 | lx) == 0) {
            /* acos(1)=0, acos(-1)=acos_c__pi */
            if (hx >> 31)
                return 2*acos_c__pio2_hi + 0x1p-120f;
            return 0;
        }
        return 0/(x-x);
    }
    /* |x| < 0.5 */
    if (ix < 0x3fe00000) {
        if (ix <= 0x3c600000)  /* |x| < 2**-57 */
            return acos_c__pio2_hi + 0x1p-120f;
        return acos_c__pio2_hi - (x - (acos_c__pio2_lo-x*acos_c__R(x*x)));
    }
    /* x < -0.5 */
    if (hx >> 31) {
        z = (1.0+x)*0.5;
        s = sqrt(z);
        w = acos_c__R(z)*s-acos_c__pio2_lo;
        return 2*(acos_c__pio2_hi - (s+w));
    }
    /* x > 0.5 */
    z = (1.0-x)*0.5;
    s = sqrt(z);
    df = s;
    SET_LOW_WORD(df,0);
    c = (z-df*df)/(s+df);
    w = acos_c__R(z)*s+c;
    return 2*(df+w);
}

/// >>> START src/math/acosf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_acosf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
acosf_c__pio2_hi = 1.5707962513e+00, /* 0x3fc90fda */
acosf_c__pio2_lo = 7.5497894159e-08, /* 0x33a22168 */
acosf_c__pS0 =  1.6666586697e-01,
acosf_c__pS1 = -4.2743422091e-02,
acosf_c__pS2 = -8.6563630030e-03,
acosf_c__qS1 = -7.0662963390e-01;

static float acosf_c__R(float z) {
    float_t p, q;
    p = z*(acosf_c__pS0+z*(acosf_c__pS1+z*acosf_c__pS2));
    q = 1.0f+z*acosf_c__qS1;
    return p/q;
}

float acosf(float x) {
    float z,w,s,c,df;
    uint32_t hx,ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    /* |x| >= 1 or nan */
    if (ix >= 0x3f800000) {
        if (ix == 0x3f800000) {
            if (hx >> 31)
                return 2*acosf_c__pio2_hi + 0x1p-120f;
            return 0;
        }
        return 0/(x-x);
    }
    /* |x| < 0.5 */
    if (ix < 0x3f000000) {
        if (ix <= 0x32800000) /* |x| < 2**-26 */
            return acosf_c__pio2_hi + 0x1p-120f;
        return acosf_c__pio2_hi - (x - (acosf_c__pio2_lo-x*acosf_c__R(x*x)));
    }
    /* x < -0.5 */
    if (hx >> 31) {
        z = (1+x)*0.5f;
        s = sqrtf(z);
        w = acosf_c__R(z)*s-acosf_c__pio2_lo;
        return 2*(acosf_c__pio2_hi - (s+w));
    }
    /* x > 0.5 */
    z = (1-x)*0.5f;
    s = sqrtf(z);
    GET_FLOAT_WORD(hx,s);
    SET_FLOAT_WORD(df,hx&0xfffff000);
    c = (z-df*df)/(s+df);
    w = acosf_c__R(z)*s+c;
    return 2*(df+w);
}

/// >>> START src/math/acosh.c

#if FLT_EVAL_METHOD==2
#undef sqrt
#undef sqrt
#define sqrt sqrtl
#endif

/* acosh(x) = log(x + sqrt(x*x-1)) */
double acosh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    unsigned e = u.i >> 52 & 0x7ff;

    /* x < 1 domain error is handled in the called functions */

    if (e < 0x3ff + 1)
        /* |x| < 2, up to 2ulp error in [1,1.125] */
        return log1p(x-1 + sqrt((x-1)*(x-1)+2*(x-1)));
    if (e < 0x3ff + 26)
        /* |x| < 0x1p26 */
        return log(2*x - 1/(x+sqrt(x*x-1)));
    /* |x| >= 0x1p26 or nan */
    return log(x) + 0.693147180559945309417232121458176568;
}

/// >>> START src/math/acoshf.c

#if FLT_EVAL_METHOD==2
#undef sqrtf
#undef sqrtf
#define sqrtf sqrtl
#elif FLT_EVAL_METHOD==1
#undef sqrtf
#undef sqrtf
#define sqrtf sqrt
#endif

/* acosh(x) = log(x + sqrt(x*x-1)) */
float acoshf(float x) {
    union {float f; uint32_t i;} u = {x};
    uint32_t a = u.i & 0x7fffffff;

    if (a < 0x3f800000+(1<<23))
        /* |x| < 2, invalid if x < 1 or nan */
        /* up to 2ulp error in [1,1.125] */
        return log1pf(x-1 + sqrtf((x-1)*(x-1)+2*(x-1)));
    if (a < 0x3f800000+(12<<23))
        /* |x| < 0x1p12 */
        return logf(2*x - 1/(x+sqrtf(x*x-1)));
    /* x >= 0x1p12 */
    return logf(x) + 0.693147180559945309417232121458176568f;
}

/// >>> START src/math/acoshl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double acoshl(long double x) {
    return acosh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* acosh(x) = log(x + sqrt(x*x-1)) */
long double acoshl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;

    if (e < 0x3fff + 1)
        /* |x| < 2, invalid if x < 1 or nan */
        return log1pl(x-1 + sqrtl((x-1)*(x-1)+2*(x-1)));
    if (e < 0x3fff + 32)
        /* |x| < 0x1p32 */
        return logl(2*x - 1/(x+sqrtl(x*x-1)));
    return logl(x) + 0.693147180559945309417232121458176568L;
}
#endif

/// >>> START src/math/acosl.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_acosl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in acos.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double acosl(long double x) {
    return acos(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef CLEARBOTTOM
#define CLEARBOTTOM(u) (u.i.m &= -1ULL << 32)
#elif LDBL_MANT_DIG == 113
#undef CLEARBOTTOM
#define CLEARBOTTOM(u) (u.i.lo = 0)
#endif

long double acosl(long double x) {
    union ldshape u = {x};
    long double z, s, c, f;
    uint16_t e = u.i.se & 0x7fff;

    /* |x| >= 1 or nan */
    if (e >= 0x3fff) {
        if (x == 1)
            return 0;
        if (x == -1)
            return 2*acst2l__pio2_hi + 0x1p-120f;
        return 0/(x-x);
    }
    /* |x| < 0.5 */
    if (e < 0x3fff - 1) {
        if (e < 0x3fff - LDBL_MANT_DIG - 1)
            return acst2l__pio2_hi + 0x1p-120f;
        return acst2l__pio2_hi - (__invtrigl_R(x*x)*x - acst2l__pio2_lo + x);
    }
    /* x < -0.5 */
    if (u.i.se >> 15) {
        z = (1 + x)*0.5;
        s = sqrtl(z);
        return 2*(acst2l__pio2_hi - (__invtrigl_R(z)*s - acst2l__pio2_lo + s));
    }
    /* x > 0.5 */
    z = (1 - x)*0.5;
    s = sqrtl(z);
    u.f = s;
    CLEARBOTTOM(u);
    f = u.f;
    c = (z - f*f)/(s + f);
    return 2*(__invtrigl_R(z)*s + c + f);
}
#endif

/// >>> START src/math/asin.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_asin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* asin(x)
 * Method :
 *      Since  asin(x) = x + x^3/6 + x^5*3/40 + x^7*15/336 + ...
 *      we approximate asin(x) on [0,0.5] by
 *              asin(x) = x + x*x^2*asin_c__R(x^2)
 *      where
 *              asin_c__R(x^2) is a rational approximation of (asin(x)-x)/x^3
 *      and its remez error is bounded by
 *              |(asin(x)-x)/x^3 - asin_c__R(x^2)| < 2^(-58.75)
 *
 *      For x in [0.5,1]
 *              asin(x) = asin_c__pi/2-2*asin(sqrt((1-x)/2))
 *      Let y = (1-x), z = y/2, s := sqrt(z), and asin_c__pio2_hi+asin_c__pio2_lo=asin_c__pi/2;
 *      then for x>0.98
 *              asin(x) = asin_c__pi/2 - 2*(s+s*z*asin_c__R(z))
 *                      = asin_c__pio2_hi - (2*(s+s*z*asin_c__R(z)) - asin_c__pio2_lo)
 *      For x<=0.98, let pio4_hi = asin_c__pio2_hi/2, then
 *              f = hi part of s;
 *              c = sqrt(z) - f = (z-f*f)/(s+f)         ...f+c=sqrt(z)
 *      and
 *              asin(x) = asin_c__pi/2 - 2*(s+s*z*asin_c__R(z))
 *                      = pio4_hi+(asin_c__pio4-2s)-(2s*z*asin_c__R(z)-asin_c__pio2_lo)
 *                      = pio4_hi+(asin_c__pio4-2f)-(2s*z*asin_c__R(z)-(asin_c__pio2_lo+2c))
 *
 * Special cases:
 *      if x is NaN, return x itself;
 *      if |x|>1, return NaN with invalid signal.
 *
 */

static const double
asin_c__pio2_hi = 1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
asin_c__pio2_lo = 6.12323399573676603587e-17, /* 0x3C91A626, 0x33145C07 */
/* coefficients for asin_c__R(x^2) */
asin_c__pS0 =  1.66666666666666657415e-01, /* 0x3FC55555, 0x55555555 */
asin_c__pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
asin_c__pS2 =  2.01212532134862925881e-01, /* 0x3FC9C155, 0x0E884455 */
asin_c__pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
asin_c__pS4 =  7.91534994289814532176e-04, /* 0x3F49EFE0, 0x7501B288 */
asin_c__pS5 =  3.47933107596021167570e-05, /* 0x3F023DE1, 0x0DFDF709 */
asin_c__qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
asin_c__qS2 =  2.02094576023350569471e+00, /* 0x40002AE5, 0x9C598AC8 */
asin_c__qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
asin_c__qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

static double asin_c__R(double z) {
    double_t p, q;
    p = z*(asin_c__pS0+z*(asin_c__pS1+z*(asin_c__pS2+z*(asin_c__pS3+z*(asin_c__pS4+z*asin_c__pS5)))));
    q = 1.0+z*(asin_c__qS1+z*(asin_c__qS2+z*(asin_c__qS3+z*asin_c__qS4)));
    return p/q;
}

double asin(double x) {
    double z,r,s;
    uint32_t hx,ix;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    /* |x| >= 1 or nan */
    if (ix >= 0x3ff00000) {
        uint32_t lx;
        GET_LOW_WORD(lx, x);
        if ((ix-0x3ff00000 | lx) == 0)
            /* asin(1) = +-asin_c__pi/2 with inexact */
            return x*asin_c__pio2_hi + 0x1p-120f;
        return 0/(x-x);
    }
    /* |x| < 0.5 */
    if (ix < 0x3fe00000) {
        /* if 0x1p-1022 <= |x| < 0x1p-26, avoid raising underflow */
        if (ix < 0x3e500000 && ix >= 0x00100000)
            return x;
        return x + x*asin_c__R(x*x);
    }
    /* 1 > |x| >= 0.5 */
    z = (1 - fabs(x))*0.5;
    s = sqrt(z);
    r = asin_c__R(z);
    if (ix >= 0x3fef3333) {  /* if |x| > 0.975 */
        x = asin_c__pio2_hi-(2*(s+s*r)-asin_c__pio2_lo);
    } else {
        double f,c;
        /* f+c = sqrt(z) */
        f = s;
        SET_LOW_WORD(f,0);
        c = (z-f*f)/(s+f);
        x = 0.5*asin_c__pio2_hi - (2*s*r - (asin_c__pio2_lo-2*c) - (0.5*asin_c__pio2_hi-2*f));
    }
    if (hx >> 31)
        return -x;
    return x;
}

/// >>> START src/math/asinf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_asinf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const double
asinf_c__pio2 = 1.570796326794896558e+00;

static const float
/* coefficients for asinf_c__R(x^2) */
asinf_c__pS0 =  1.6666586697e-01,
asinf_c__pS1 = -4.2743422091e-02,
asinf_c__pS2 = -8.6563630030e-03,
asinf_c__qS1 = -7.0662963390e-01;

static float asinf_c__R(float z) {
    float_t p, q;
    p = z*(asinf_c__pS0+z*(asinf_c__pS1+z*asinf_c__pS2));
    q = 1.0f+z*asinf_c__qS1;
    return p/q;
}

float asinf(float x) {
    double s;
    float z;
    uint32_t hx,ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x3f800000) {  /* |x| >= 1 */
        if (ix == 0x3f800000)  /* |x| == 1 */
            return x*asinf_c__pio2 + 0x1p-120f;  /* asin(+-1) = +-asinf_c__pi/2 with inexact */
        return 0/(x-x);  /* asin(|x|>1) is NaN */
    }
    if (ix < 0x3f000000) {  /* |x| < 0.5 */
        /* if 0x1p-126 <= |x| < 0x1p-12, avoid raising underflow */
        if (ix < 0x39800000 && ix >= 0x00800000)
            return x;
        return x + x*asinf_c__R(x*x);
    }
    /* 1 > |x| >= 0.5 */
    z = (1 - fabsf(x))*0.5f;
    s = sqrt(z);
    x = asinf_c__pio2 - 2*(s+s*asinf_c__R(z));
    if (hx >> 31)
        return -x;
    return x;
}

/// >>> START src/math/asinh.c

/* asinh(x) = sign(x)*log(|x|+sqrt(x*x+1)) ~= x - x^3/6 + o(x^5) */
double asinh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    unsigned e = u.i >> 52 & 0x7ff;
    unsigned s = u.i >> 63;

    /* |x| */
    u.i &= (uint64_t)-1/2;
    x = u.f;

    if (e >= 0x3ff + 26) {
        /* |x| >= 0x1p26 or inf or nan */
        x = log(x) + 0.693147180559945309417232121458176568;
    } else if (e >= 0x3ff + 1) {
        /* |x| >= 2 */
        x = log(2*x + 1/(sqrt(x*x+1)+x));
    } else if (e >= 0x3ff - 26) {
        /* |x| >= 0x1p-26, up to 1.6ulp error in [0.125,0.5] */
        x = log1p(x + x*x/(sqrt(x*x+1)+1));
    } else {
        /* |x| < 0x1p-26, raise inexact if x != 0 */
        FORCE_EVAL(x + 0x1p120f);
    }
    return s ? -x : x;
}

/// >>> START src/math/asinhf.c

/* asinh(x) = sign(x)*log(|x|+sqrt(x*x+1)) ~= x - x^3/6 + o(x^5) */
float asinhf(float x) {
    union {float f; uint32_t i;} u = {.f = x};
    uint32_t i = u.i & 0x7fffffff;
    unsigned s = u.i >> 31;

    /* |x| */
    u.i = i;
    x = u.f;

    if (i >= 0x3f800000 + (12<<23)) {
        /* |x| >= 0x1p12 or inf or nan */
        x = logf(x) + 0.693147180559945309417232121458176568f;
    } else if (i >= 0x3f800000 + (1<<23)) {
        /* |x| >= 2 */
        x = logf(2*x + 1/(sqrtf(x*x+1)+x));
    } else if (i >= 0x3f800000 - (12<<23)) {
        /* |x| >= 0x1p-12, up to 1.6ulp error in [0.125,0.5] */
        x = log1pf(x + x*x/(sqrtf(x*x+1)+1));
    } else {
        /* |x| < 0x1p-12, raise inexact if x!=0 */
        FORCE_EVAL(x + 0x1p120f);
    }
    return s ? -x : x;
}

/// >>> START src/math/asinhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double asinhl(long double x) {
    return asinh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* asinh(x) = sign(x)*log(|x|+sqrt(x*x+1)) ~= x - x^3/6 + o(x^5) */
long double asinhl(long double x) {
    union ldshape u = {x};
    unsigned e = u.i.se & 0x7fff;
    unsigned s = u.i.se >> 15;

    /* |x| */
    u.i.se = e;
    x = u.f;

    if (e >= 0x3fff + 32) {
        /* |x| >= 0x1p32 or inf or nan */
        x = logl(x) + 0.693147180559945309417232121458176568L;
    } else if (e >= 0x3fff + 1) {
        /* |x| >= 2 */
        x = logl(2*x + 1/(sqrtl(x*x+1)+x));
    } else if (e >= 0x3fff - 32) {
        /* |x| >= 0x1p-32 */
        x = log1pl(x + x*x/(sqrtl(x*x+1)+1));
    } else {
        /* |x| < 0x1p-32, raise inexact if x!=0 */
        FORCE_EVAL(x + 0x1p120f);
    }
    return s ? -x : x;
}
#endif

/// >>> START src/math/asinl.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_asinl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in asin.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double asinl(long double x) {
    return asin(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef CLOSETO1
#define CLOSETO1(u) (u.i.m>>56 >= 0xf7)
#undef CLEARBOTTOM
#define CLEARBOTTOM(u) (u.i.m &= -1ULL << 32)
#elif LDBL_MANT_DIG == 113
#undef CLOSETO1
#define CLOSETO1(u) (u.i.top >= 0xee00)
#undef CLEARBOTTOM
#define CLEARBOTTOM(u) (u.i.lo = 0)
#endif

long double asinl(long double x) {
    union ldshape u = {x};
    long double z, r, s;
    uint16_t e = u.i.se & 0x7fff;
    int sign = u.i.se >> 15;

    if (e >= 0x3fff) {   /* |x| >= 1 or nan */
        /* asin(+-1)=+-asinl_c__pi/2 with inexact */
        if (x == 1 || x == -1)
            return x*acst2l__pio2_hi + 0x1p-120f;
        return 0/(x-x);
    }
    if (e < 0x3fff - 1) {  /* |x| < 0.5 */
        if (e < 0x3fff - (LDBL_MANT_DIG+1)/2) {
            /* return x with inexact if x!=0 */
            FORCE_EVAL(x + 0x1p120f);
            return x;
        }
        return x + x*__invtrigl_R(x*x);
    }
    /* 1 > |x| >= 0.5 */
    z = (1.0 - fabsl(x))*0.5;
    s = sqrtl(z);
    r = __invtrigl_R(z);
    if (CLOSETO1(u)) {
        x = acst2l__pio2_hi - (2*(s+s*r)-acst2l__pio2_lo);
    } else {
        long double f, c;
        u.f = s;
        CLEARBOTTOM(u);
        f = u.f;
        c = (z - f*f)/(s + f);
        x = 0.5*acst2l__pio2_hi-(2*s*r - (acst2l__pio2_lo-2*c) - (0.5*acst2l__pio2_hi-2*f));
    }
    return sign ? -x : x;
}
#endif

/// >>> START src/math/atan.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_atan.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* atan(x)
 * Method
 *   1. Reduce x to positive by atan(x) = -atan(-x).
 *   2. According to the integer atan_c__k=4t+0.25 chopped, t=x, the argument
 *      is further reduced to one of the following intervals and the
 *      arctangent of t is evaluated by the corresponding formula:
 *
 *      [0,7/16]      atan(x) = t-t^3*(atan_c__a1+t^2*(atan_c__a2+...(atan_c__a10+t^2*atan_c__a11)...)
 *      [7/16,11/16]  atan(x) = atan(1/2) + atan( (t-0.5)/(1+t/2) )
 *      [11/16.19/16] atan(x) = atan( 1 ) + atan( (t-1)/(1+t) )
 *      [19/16,39/16] atan(x) = atan(3/2) + atan( (t-1.5)/(1+1.5t) )
 *      [39/16,INF]   atan(x) = atan(INF) + atan( -1/t )
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double atan_c__atanhi[] = {
  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double atan_c__atanlo[] = {
  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const double atan_c__aT[] = {
  3.33333333333329318027e-01, /* 0x3FD55555, 0x5555550D */
 -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
  1.42857142725034663711e-01, /* 0x3FC24924, 0x920083FF */
 -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
  9.09088713343650656196e-02, /* 0x3FB745CD, 0xC54C206E */
 -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
  6.66107313738753120669e-02, /* 0x3FB10D66, 0xA0D03D51 */
 -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
  4.97687799461593236017e-02, /* 0x3FA97B4B, 0x24760DEB */
 -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
  1.62858201153657823623e-02, /* 0x3F90AD3A, 0xE322DA11 */
};

double atan(double x) {
    double_t w,atan_c__s1,atan_c__s2,z;
    uint32_t ix,sign;
    int id;

    GET_HIGH_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;
    if (ix >= 0x44100000) {   /* if |x| >= 2^66 */
        if (isnan(x))
            return x;
        z = atan_c__atanhi[3] + 0x1p-120f;
        return sign ? -z : z;
    }
    if (ix < 0x3fdc0000) {    /* |x| < 0.4375 */
        if (ix < 0x3e400000) {  /* |x| < 2^-27 */
            if (ix < 0x00100000)
                /* raise underflow for subnormal x */
                FORCE_EVAL((float)x);
            return x;
        }
        id = -1;
    } else {
        x = fabs(x);
        if (ix < 0x3ff30000) {  /* |x| < 1.1875 */
            if (ix < 0x3fe60000) {  /*  7/16 <= |x| < 11/16 */
                id = 0;
                x = (2.0*x-1.0)/(2.0+x);
            } else {                /* 11/16 <= |x| < 19/16 */
                id = 1;
                x = (x-1.0)/(x+1.0);
            }
        } else {
            if (ix < 0x40038000) {  /* |x| < 2.4375 */
                id = 2;
                x = (x-1.5)/(1.0+1.5*x);
            } else {                /* 2.4375 <= |x| < 2^66 */
                id = 3;
                x = -1.0/x;
            }
        }
    }
    /* end of argument reduction */
    z = x*x;
    w = z*z;
    /* break sum from i=0 to 10 atan_c__aT[i]z**(i+1) into odd and even poly */
    atan_c__s1 = z*(atan_c__aT[0]+w*(atan_c__aT[2]+w*(atan_c__aT[4]+w*(atan_c__aT[6]+w*(atan_c__aT[8]+w*atan_c__aT[10])))));
    atan_c__s2 = w*(atan_c__aT[1]+w*(atan_c__aT[3]+w*(atan_c__aT[5]+w*(atan_c__aT[7]+w*atan_c__aT[9]))));
    if (id < 0)
        return x - x*(atan_c__s1+atan_c__s2);
    z = atan_c__atanhi[id] - (x*(atan_c__s1+atan_c__s2) - atan_c__atanlo[id] - x);
    return sign ? -z : z;
}

/// >>> START src/math/atan2.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_atan2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/* atan2(y,x)
 * Method :
 *      1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *      2. Reduce x to positive by (if x and y are unexceptional):
 *              ARG (x+iy) = arctan(y/x)           ... if x > 0,
 *              ARG (x+iy) = atan2_c__pi - arctan[y/(-x)]   ... if x < 0,
 *
 * Special cases:
 *
 *      ATAN2((anything), NaN ) is NaN;
 *      ATAN2(NAN , (anything) ) is NaN;
 *      ATAN2(+-0, +(anything but NaN)) is +-0  ;
 *      ATAN2(+-0, -(anything but NaN)) is +-atan2_c__pi ;
 *      ATAN2(+-(anything but 0 and NaN), 0) is +-atan2_c__pi/2;
 *      ATAN2(+-(anything but INF and NaN), +INF) is +-0 ;
 *      ATAN2(+-(anything but INF and NaN), -INF) is +-atan2_c__pi;
 *      ATAN2(+-INF,+INF ) is +-atan2_c__pi/4 ;
 *      ATAN2(+-INF,-INF ) is +-3pi/4;
 *      ATAN2(+-INF, (anything but,0,NaN, and INF)) is +-atan2_c__pi/2;
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
atan2_c__pi     = 3.1415926535897931160E+00, /* 0x400921FB, 0x54442D18 */
atan2_c__pi_lo  = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

double atan2(double y, double x) {
    double z;
    uint32_t m,lx,ly,ix,iy;

    if (isnan(x) || isnan(y))
        return x+y;
    EXTRACT_WORDS(ix, lx, x);
    EXTRACT_WORDS(iy, ly, y);
    if ((ix-0x3ff00000 | lx) == 0)  /* x = 1.0 */
        return atan(y);
    m = ((iy>>31)&1) | ((ix>>30)&2);  /* 2*sign(x)+sign(y) */
    ix = ix & 0x7fffffff;
    iy = iy & 0x7fffffff;

    /* when y = 0 */
    if ((iy|ly) == 0) {
        switch(m) {
        case 0:
        case 1: return y;   /* atan(+-0,+anything)=+-0 */
        case 2: return  atan2_c__pi; /* atan(+0,-anything) = atan2_c__pi */
        case 3: return -atan2_c__pi; /* atan(-0,-anything) =-atan2_c__pi */
        }
    }
    /* when x = 0 */
    if ((ix|lx) == 0)
        return m&1 ? -atan2_c__pi/2 : atan2_c__pi/2;
    /* when x is INF */
    if (ix == 0x7ff00000) {
        if (iy == 0x7ff00000) {
            switch(m) {
            case 0: return  atan2_c__pi/4;   /* atan(+INF,+INF) */
            case 1: return -atan2_c__pi/4;   /* atan(-INF,+INF) */
            case 2: return  3*atan2_c__pi/4; /* atan(+INF,-INF) */
            case 3: return -3*atan2_c__pi/4; /* atan(-INF,-INF) */
            }
        } else {
            switch(m) {
            case 0: return  0.0; /* atan(+...,+INF) */
            case 1: return -0.0; /* atan(-...,+INF) */
            case 2: return  atan2_c__pi;  /* atan(+...,-INF) */
            case 3: return -atan2_c__pi;  /* atan(-...,-INF) */
            }
        }
    }
    /* |y/x| > 0x1p64 */
    if (ix+(64<<20) < iy || iy == 0x7ff00000)
        return m&1 ? -atan2_c__pi/2 : atan2_c__pi/2;

    /* z = atan(|y/x|) without spurious underflow */
    if ((m&2) && iy+(64<<20) < ix)  /* |y/x| < 0x1p-64, x<0 */
        z = 0;
    else
        z = atan(fabs(y/x));
    switch (m) {
    case 0: return z;              /* atan(+,+) */
    case 1: return -z;             /* atan(-,+) */
    case 2: return atan2_c__pi - (z-atan2_c__pi_lo); /* atan(+,-) */
    default: /* case 3 */
        return (z-atan2_c__pi_lo) - atan2_c__pi; /* atan(-,-) */
    }
}

/// >>> START src/math/atan2f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_atan2f.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
atan2f_c__pi     = 3.1415927410e+00, /* 0x40490fdb */
atan2f_c__pi_lo  = -8.7422776573e-08; /* 0xb3bbbd2e */

float atan2f(float y, float x) {
    float z;
    uint32_t m,ix,iy;

    if (isnan(x) || isnan(y))
        return x+y;
    GET_FLOAT_WORD(ix, x);
    GET_FLOAT_WORD(iy, y);
    if (ix == 0x3f800000)  /* x=1.0 */
        return atanf(y);
    m = ((iy>>31)&1) | ((ix>>30)&2);  /* 2*sign(x)+sign(y) */
    ix &= 0x7fffffff;
    iy &= 0x7fffffff;

    /* when y = 0 */
    if (iy == 0) {
        switch (m) {
        case 0:
        case 1: return y;   /* atan(+-0,+anything)=+-0 */
        case 2: return  atan2f_c__pi; /* atan(+0,-anything) = atan2f_c__pi */
        case 3: return -atan2f_c__pi; /* atan(-0,-anything) =-atan2f_c__pi */
        }
    }
    /* when x = 0 */
    if (ix == 0)
        return m&1 ? -atan2f_c__pi/2 : atan2f_c__pi/2;
    /* when x is INF */
    if (ix == 0x7f800000) {
        if (iy == 0x7f800000) {
            switch (m) {
            case 0: return  atan2f_c__pi/4; /* atan(+INF,+INF) */
            case 1: return -atan2f_c__pi/4; /* atan(-INF,+INF) */
            case 2: return 3*atan2f_c__pi/4;  /*atan(+INF,-INF)*/
            case 3: return -3*atan2f_c__pi/4; /*atan(-INF,-INF)*/
            }
        } else {
            switch (m) {
            case 0: return  0.0f;    /* atan(+...,+INF) */
            case 1: return -0.0f;    /* atan(-...,+INF) */
            case 2: return  atan2f_c__pi; /* atan(+...,-INF) */
            case 3: return -atan2f_c__pi; /* atan(-...,-INF) */
            }
        }
    }
    /* |y/x| > 0x1p26 */
    if (ix+(26<<23) < iy || iy == 0x7f800000)
        return m&1 ? -atan2f_c__pi/2 : atan2f_c__pi/2;

    /* z = atan(|y/x|) with correct underflow */
    if ((m&2) && iy+(26<<23) < ix)  /*|y/x| < 0x1p-26, x < 0 */
        z = 0.0;
    else
        z = atanf(fabsf(y/x));
    switch (m) {
    case 0: return z;              /* atan(+,+) */
    case 1: return -z;             /* atan(-,+) */
    case 2: return atan2f_c__pi - (z-atan2f_c__pi_lo); /* atan(+,-) */
    default: /* case 3 */
        return (z-atan2f_c__pi_lo) - atan2f_c__pi; /* atan(-,-) */
    }
}

/// >>> START src/math/atan2l.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_atan2l.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/*
 * See comments in atan2.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double atan2l(long double y, long double x) {
    return atan2(y, x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384

long double atan2l(long double y, long double x) {
    union ldshape ux, uy;
    long double z;
    int m, ex, ey;

    if (isnan(x) || isnan(y))
        return x+y;
    if (x == 1)
        return atanl(y);
    ux.f = x;
    uy.f = y;
    ex = ux.i.se & 0x7fff;
    ey = uy.i.se & 0x7fff;
    m = 2*(ux.i.se>>15) | uy.i.se>>15;
    if (y == 0) {
        switch(m) {
        case 0:
        case 1: return y;           /* atan(+-0,+anything)=+-0 */
        case 2: return  2*acst2l__pio2_hi;  /* atan(+0,-anything) = atan2l_c__pi */
        case 3: return -2*acst2l__pio2_hi;  /* atan(-0,-anything) =-atan2l_c__pi */
        }
    }
    if (x == 0)
        return m&1 ? -acst2l__pio2_hi : acst2l__pio2_hi;
    if (ex == 0x7fff) {
        if (ey == 0x7fff) {
            switch(m) {
            case 0: return  acst2l__pio2_hi/2;   /* atan(+INF,+INF) */
            case 1: return -acst2l__pio2_hi/2;   /* atan(-INF,+INF) */
            case 2: return  1.5*acst2l__pio2_hi; /* atan(+INF,-INF) */
            case 3: return -1.5*acst2l__pio2_hi; /* atan(-INF,-INF) */
            }
        } else {
            switch(m) {
            case 0: return  0.0;        /* atan(+...,+INF) */
            case 1: return -0.0;        /* atan(-...,+INF) */
            case 2: return  2*acst2l__pio2_hi;  /* atan(+...,-INF) */
            case 3: return -2*acst2l__pio2_hi;  /* atan(-...,-INF) */
            }
        }
    }
    if (ex+120 < ey || ey == 0x7fff)
        return m&1 ? -acst2l__pio2_hi : acst2l__pio2_hi;
    /* z = atan(|y/x|) without spurious underflow */
    if ((m&2) && ey+120 < ex)  /* |y/x| < 0x1p-120, x<0 */
        z = 0.0;
    else
        z = atanl(fabsl(y/x));
    switch (m) {
    case 0: return z;               /* atan(+,+) */
    case 1: return -z;              /* atan(-,+) */
    case 2: return 2*acst2l__pio2_hi-(z-2*acst2l__pio2_lo); /* atan(+,-) */
    default: /* case 3 */
        return (z-2*acst2l__pio2_lo)-2*acst2l__pio2_hi; /* atan(-,-) */
    }
}
#endif

/// >>> START src/math/atanf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_atanf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float atanf_c__atanhi[] = {
  4.6364760399e-01, /* atan(0.5)hi 0x3eed6338 */
  7.8539812565e-01, /* atan(1.0)hi 0x3f490fda */
  9.8279368877e-01, /* atan(1.5)hi 0x3f7b985e */
  1.5707962513e+00, /* atan(inf)hi 0x3fc90fda */
};

static const float atanf_c__atanlo[] = {
  5.0121582440e-09, /* atan(0.5)lo 0x31ac3769 */
  3.7748947079e-08, /* atan(1.0)lo 0x33222168 */
  3.4473217170e-08, /* atan(1.5)lo 0x33140fb4 */
  7.5497894159e-08, /* atan(inf)lo 0x33a22168 */
};

static const float atanf_c__aT[] = {
  3.3333328366e-01,
 -1.9999158382e-01,
  1.4253635705e-01,
 -1.0648017377e-01,
  6.1687607318e-02,
};

float atanf(float x) {
    float_t w,atanf_c__s1,atanf_c__s2,z;
    uint32_t ix,sign;
    int id;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x4c800000) {  /* if |x| >= 2**26 */
        if (isnan(x))
            return x;
        z = atanf_c__atanhi[3] + 0x1p-120f;
        return sign ? -z : z;
    }
    if (ix < 0x3ee00000) {   /* |x| < 0.4375 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            if (ix < 0x00800000)
                /* raise underflow for subnormal x */
                FORCE_EVAL(x*x);
            return x;
        }
        id = -1;
    } else {
        x = fabsf(x);
        if (ix < 0x3f980000) {  /* |x| < 1.1875 */
            if (ix < 0x3f300000) {  /*  7/16 <= |x| < 11/16 */
                id = 0;
                x = (2.0f*x - 1.0f)/(2.0f + x);
            } else {                /* 11/16 <= |x| < 19/16 */
                id = 1;
                x = (x - 1.0f)/(x + 1.0f);
            }
        } else {
            if (ix < 0x401c0000) {  /* |x| < 2.4375 */
                id = 2;
                x = (x - 1.5f)/(1.0f + 1.5f*x);
            } else {                /* 2.4375 <= |x| < 2**26 */
                id = 3;
                x = -1.0f/x;
            }
        }
    }
    /* end of argument reduction */
    z = x*x;
    w = z*z;
    /* break sum from i=0 to 10 atanf_c__aT[i]z**(i+1) into odd and even poly */
    atanf_c__s1 = z*(atanf_c__aT[0]+w*(atanf_c__aT[2]+w*atanf_c__aT[4]));
    atanf_c__s2 = w*(atanf_c__aT[1]+w*atanf_c__aT[3]);
    if (id < 0)
        return x - x*(atanf_c__s1+atanf_c__s2);
    z = atanf_c__atanhi[id] - ((x*(atanf_c__s1+atanf_c__s2) - atanf_c__atanlo[id]) - x);
    return sign ? -z : z;
}

/// >>> START src/math/atanh.c

/* atanh(x) = log((1+x)/(1-x))/2 = log1p(2x/(1-x))/2 ~= x + x^3/3 + o(x^5) */
double atanh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    unsigned e = u.i >> 52 & 0x7ff;
    unsigned s = u.i >> 63;
    double_t y;

    /* |x| */
    u.i &= (uint64_t)-1/2;
    y = u.f;

    if (e < 0x3ff - 1) {
        if (e < 0x3ff - 32) {
            /* handle underflow */
            if (e == 0)
                FORCE_EVAL((float)y);
        } else {
            /* |x| < 0.5, up to 1.7ulp error */
            y = 0.5*log1p(2*y + 2*y*y/(1-y));
        }
    } else {
        /* avoid overflow */
        y = 0.5*log1p(2*(y/(1-y)));
    }
    return s ? -y : y;
}

/// >>> START src/math/atanhf.c

/* atanh(x) = log((1+x)/(1-x))/2 = log1p(2x/(1-x))/2 ~= x + x^3/3 + o(x^5) */
float atanhf(float x) {
    union {float f; uint32_t i;} u = {.f = x};
    unsigned s = u.i >> 31;
    float_t y;

    /* |x| */
    u.i &= 0x7fffffff;
    y = u.f;

    if (u.i < 0x3f800000 - (1<<23)) {
        if (u.i < 0x3f800000 - (32<<23)) {
            /* handle underflow */
            if (u.i < (1<<23))
                FORCE_EVAL((float)(y*y));
        } else {
            /* |x| < 0.5, up to 1.7ulp error */
            y = 0.5f*log1pf(2*y + 2*y*y/(1-y));
        }
    } else {
        /* avoid overflow */
        y = 0.5f*log1pf(2*(y/(1-y)));
    }
    return s ? -y : y;
}

/// >>> START src/math/atanhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double atanhl(long double x) {
    return atanh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* atanh(x) = log((1+x)/(1-x))/2 = log1p(2x/(1-x))/2 ~= x + x^3/3 + o(x^5) */
long double atanhl(long double x) {
    union ldshape u = {x};
    unsigned e = u.i.se & 0x7fff;
    unsigned s = u.i.se >> 15;

    /* |x| */
    u.i.se = e;
    x = u.f;

    if (e < 0x3ff - 1) {
        if (e < 0x3ff - LDBL_MANT_DIG/2) {
            /* handle underflow */
            if (e == 0)
                FORCE_EVAL((float)x);
        } else {
            /* |x| < 0.5, up to 1.7ulp error */
            x = 0.5*log1pl(2*x + 2*x*x/(1-x));
        }
    } else {
        /* avoid overflow */
        x = 0.5*log1pl(2*(x/(1-x)));
    }
    return s ? -x : x;
}
#endif

/// >>> START src/math/atanl.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_atanl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in atan.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double atanl(long double x) {
    return atan(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384

#if LDBL_MANT_DIG == 64
#undef EXPMAN
#define EXPMAN(u) ((u.i.se & 0x7fff)<<8 | (u.i.m>>55 & 0xff))

static const long double atanl_c__atanhi[] = {
     4.63647609000806116202e-01L,
     7.85398163397448309628e-01L,
     9.82793723247329067960e-01L,
     1.57079632679489661926e+00L,
};

static const long double atanl_c__atanlo[] = {
     1.18469937025062860669e-20L,
    -1.25413940316708300586e-20L,
     2.55232234165405176172e-20L,
    -2.50827880633416601173e-20L,
};

static const long double atanl_c__aT[] = {
     3.33333333333333333017e-01L,
    -1.99999999999999632011e-01L,
     1.42857142857046531280e-01L,
    -1.11111111100562372733e-01L,
     9.09090902935647302252e-02L,
    -7.69230552476207730353e-02L,
     6.66661718042406260546e-02L,
    -5.88158892835030888692e-02L,
     5.25499891539726639379e-02L,
    -4.70119845393155721494e-02L,
     4.03539201366454414072e-02L,
    -2.91303858419364158725e-02L,
     1.24822046299269234080e-02L,
};

static long double T_even(long double x) {
    return atanl_c__aT[0] + x * (atanl_c__aT[2] + x * (atanl_c__aT[4] + x * (atanl_c__aT[6] +
        x * (atanl_c__aT[8] + x * (atanl_c__aT[10] + x * atanl_c__aT[12])))));
}

static long double T_odd(long double x) {
    return atanl_c__aT[1] + x * (atanl_c__aT[3] + x * (atanl_c__aT[5] + x * (atanl_c__aT[7] +
        x * (atanl_c__aT[9] + x * atanl_c__aT[11]))));
}
#elif LDBL_MANT_DIG == 113
#undef EXPMAN
#define EXPMAN(u) ((u.i.se & 0x7fff)<<8 | u.i.top>>8)

const long double atanl_c__atanhi[] = {
     4.63647609000806116214256231461214397e-01L,
     7.85398163397448309615660845819875699e-01L,
     9.82793723247329067985710611014666038e-01L,
     1.57079632679489661923132169163975140e+00L,
};

const long double atanl_c__atanlo[] = {
     4.89509642257333492668618435220297706e-36L,
     2.16795253253094525619926100651083806e-35L,
    -2.31288434538183565909319952098066272e-35L,
     4.33590506506189051239852201302167613e-35L,
};

const long double atanl_c__aT[] = {
     3.33333333333333333333333333333333125e-01L,
    -1.99999999999999999999999999999180430e-01L,
     1.42857142857142857142857142125269827e-01L,
    -1.11111111111111111111110834490810169e-01L,
     9.09090909090909090908522355708623681e-02L,
    -7.69230769230769230696553844935357021e-02L,
     6.66666666666666660390096773046256096e-02L,
    -5.88235294117646671706582985209643694e-02L,
     5.26315789473666478515847092020327506e-02L,
    -4.76190476189855517021024424991436144e-02L,
     4.34782608678695085948531993458097026e-02L,
    -3.99999999632663469330634215991142368e-02L,
     3.70370363987423702891250829918659723e-02L,
    -3.44827496515048090726669907612335954e-02L,
     3.22579620681420149871973710852268528e-02L,
    -3.03020767654269261041647570626778067e-02L,
     2.85641979882534783223403715930946138e-02L,
    -2.69824879726738568189929461383741323e-02L,
     2.54194698498808542954187110873675769e-02L,
    -2.35083879708189059926183138130183215e-02L,
     2.04832358998165364349957325067131428e-02L,
    -1.54489555488544397858507248612362957e-02L,
     8.64492360989278761493037861575248038e-03L,
    -2.58521121597609872727919154569765469e-03L,
};

static long double T_even(long double x) {
    return (atanl_c__aT[0] + x * (atanl_c__aT[2] + x * (atanl_c__aT[4] + x * (atanl_c__aT[6] + x * (atanl_c__aT[8] +
        x * (atanl_c__aT[10] + x * (atanl_c__aT[12] + x * (atanl_c__aT[14] + x * (atanl_c__aT[16] +
        x * (atanl_c__aT[18] + x * (atanl_c__aT[20] + x * atanl_c__aT[22])))))))))));
}

static long double T_odd(long double x) {
    return (atanl_c__aT[1] + x * (atanl_c__aT[3] + x * (atanl_c__aT[5] + x * (atanl_c__aT[7] + x * (atanl_c__aT[9] +
        x * (atanl_c__aT[11] + x * (atanl_c__aT[13] + x * (atanl_c__aT[15] + x * (atanl_c__aT[17] +
        x * (atanl_c__aT[19] + x * (atanl_c__aT[21] + x * atanl_c__aT[23])))))))))));
}
#endif

long double atanl(long double x) {
    union ldshape u = {x};
    long double w, atanl_c__s1, atanl_c__s2, z;
    int id;
    unsigned e = u.i.se & 0x7fff;
    unsigned sign = u.i.se >> 15;
    unsigned expman;

    if (e >= 0x3fff + LDBL_MANT_DIG + 1) { /* if |x| is large, atan(x)~=atanl_c__pi/2 */
        if (isnan(x))
            return x;
        return sign ? -atanl_c__atanhi[3] : atanl_c__atanhi[3];
    }
    /* Extract the exponent and the first few bits of the mantissa. */
    expman = EXPMAN(u);
    if (expman < ((0x3fff - 2) << 8) + 0xc0) {  /* |x| < 0.4375 */
        if (e < 0x3fff - (LDBL_MANT_DIG+1)/2) {   /* if |x| is small, atanl(x)~=x */
            /* raise underflow if subnormal */
            if (e == 0)
                FORCE_EVAL((float)x);
            return x;
        }
        id = -1;
    } else {
        x = fabsl(x);
        if (expman < (0x3fff << 8) + 0x30) {  /* |x| < 1.1875 */
            if (expman < ((0x3fff - 1) << 8) + 0x60) { /*  7/16 <= |x| < 11/16 */
                id = 0;
                x = (2.0*x-1.0)/(2.0+x);
            } else {                                 /* 11/16 <= |x| < 19/16 */
                id = 1;
                x = (x-1.0)/(x+1.0);
            }
        } else {
            if (expman < ((0x3fff + 1) << 8) + 0x38) { /* |x| < 2.4375 */
                id = 2;
                x = (x-1.5)/(1.0+1.5*x);
            } else {                                 /* 2.4375 <= |x| */
                id = 3;
                x = -1.0/x;
            }
        }
    }
    /* end of argument reduction */
    z = x*x;
    w = z*z;
    /* break sum atanl_c__aT[i]z**(i+1) into odd and even poly */
    atanl_c__s1 = z*T_even(w);
    atanl_c__s2 = w*T_odd(w);
    if (id < 0)
        return x - x*(atanl_c__s1+atanl_c__s2);
    z = atanl_c__atanhi[id] - ((x*(atanl_c__s1+atanl_c__s2) - atanl_c__atanlo[id]) - x);
    return sign ? -z : z;
}
#endif

/// >>> START src/math/cbrt.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cbrt.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * Optimized by Bruce D. Evans.
 */
/* cbrt(x)
 * Return cube root of x
 */

static const uint32_t
cbrt_c__B1 = 715094163, /* cbrt_c__B1 = (1023-1023/3-0.03306235651)*2**20 */
cbrt_c__B2 = 696219795; /* cbrt_c__B2 = (1023-1023/3-54/3-0.03306235651)*2**20 */

/* |1/cbrt(x) - p(x)| < 2**-23.5 (~[-7.93e-8, 7.929e-8]). */
static const double
cbrt_c__P0 =  1.87595182427177009643,  /* 0x3ffe03e6, 0x0f61e692 */
cbrt_c__P1 = -1.88497979543377169875,  /* 0xbffe28e0, 0x92f02420 */
cbrt_c__P2 =  1.621429720105354466140, /* 0x3ff9f160, 0x4a49d6c2 */
cbrt_c__P3 = -0.758397934778766047437, /* 0xbfe844cb, 0xbee751d9 */
cbrt_c__P4 =  0.145996192886612446982; /* 0x3fc2b000, 0xd4e4edd7 */

double cbrt(double x) {
    union {double f; uint64_t i;} u = {x};
    double_t r,s,t,w;
    uint32_t hx = u.i>>32 & 0x7fffffff;

    if (hx >= 0x7ff00000)  /* cbrt(NaN,INF) is itself */
        return x+x;

    /*
     * Rough cbrt to 5 bits:
     *    cbrt(2**e*(1+m) ~= 2**(e/3)*(1+(e%3+m)/3)
     * where e is integral and >= 0, m is real and in [0, 1), and "/" and
     * "%" are integer division and modulus with rounding towards minus
     * infinity.  The RHS is always >= the LHS and has a maximum relative
     * error of about 1 in 16.  Adding a bias of -0.03306235651 to the
     * (e%3+m)/3 term reduces the error to about 1 in 32. With the IEEE
     * floating point representation, for finite positive normal values,
     * ordinary integer divison of the value in bits magically gives
     * almost exactly the RHS of the above provided we first subtract the
     * exponent bias (1023 for doubles) and later add it back.  We do the
     * subtraction virtually to keep e >= 0 so that ordinary integer
     * division rounds towards minus infinity; this is also efficient.
     */
    if (hx < 0x00100000) { /* cbrt_c__zero or subnormal? */
        u.f = x*0x1p54;
        hx = u.i>>32 & 0x7fffffff;
        if (hx == 0)
            return x;  /* cbrt(0) is itself */
        hx = hx/3 + cbrt_c__B2;
    } else
        hx = hx/3 + cbrt_c__B1;
    u.i &= 1ULL<<63;
    u.i |= (uint64_t)hx << 32;
    t = u.f;

    /*
     * New cbrt to 23 bits:
     *    cbrt(x) = t*cbrt(x/t**3) ~= t*cbrt_c__P(t**3/x)
     * where cbrt_c__P(r) is a polynomial of degree 4 that approximates 1/cbrt(r)
     * to within 2**-23.5 when |r - 1| < 1/10.  The rough approximation
     * has produced t such than |t/cbrt(x) - 1| ~< 1/32, and cubing this
     * gives us bounds for r = t**3/x.
     *
     * Try to optimize for parallel evaluation as in __tanf.c.
     */
    r = (t*t)*(t/x);
    t = t*((cbrt_c__P0+r*(cbrt_c__P1+r*cbrt_c__P2))+((r*r)*r)*(cbrt_c__P3+r*cbrt_c__P4));

    /*
     * Round t away from cbrt_c__zero to 23 bits (sloppily except for ensuring that
     * the result is larger in magnitude than cbrt(x) but not much more than
     * 2 23-bit ulps larger).  With rounding towards cbrt_c__zero, the error bound
     * would be ~5/6 instead of ~4/6.  With a maximum error of 2 23-bit ulps
     * in the rounded t, the infinite-precision error in the Newton
     * approximation barely affects third digit in the final error
     * 0.667; the error in the rounded t can be up to about 3 23-bit ulps
     * before the final error is larger than 0.667 ulps.
     */
    u.f = t;
    u.i = (u.i + 0x80000000) & 0xffffffffc0000000ULL;
    t = u.f;

    /* one step Newton iteration to 53 bits with error < 0.667 ulps */
    s = t*t;         /* t*t is exact */
    r = x/s;         /* error <= 0.5 ulps; |r| < |t| */
    w = t+t;         /* t+t is exact */
    r = (r-t)/(w+r); /* r-t is exact; w+r ~= 3*t */
    t = t+t*r;       /* error <= 0.5 + 0.5/3 + epsilon */
    return t;
}

/// >>> START src/math/cbrtf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cbrtf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Debugged and optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* cbrtf(x)
 * Return cube root of x
 */

static const unsigned
cbrtf_c__B1 = 709958130, /* cbrtf_c__B1 = (127-127.0/3-0.03306235651)*2**23 */
cbrtf_c__B2 = 642849266; /* cbrtf_c__B2 = (127-127.0/3-24/3-0.03306235651)*2**23 */

float cbrtf(float x) {
    double_t r,cbrtf_c__T;
    union {float f; uint32_t i;} u = {x};
    uint32_t hx = u.i & 0x7fffffff;

    if (hx >= 0x7f800000)  /* cbrt(NaN,INF) is itself */
        return x + x;

    /* rough cbrt to 5 bits */
    if (hx < 0x00800000) {  /* cbrtf_c__zero or subnormal? */
        if (hx == 0)
            return x;  /* cbrt(+-0) is itself */
        u.f = x*0x1p24f;
        hx = u.i & 0x7fffffff;
        hx = hx/3 + cbrtf_c__B2;
    } else
        hx = hx/3 + cbrtf_c__B1;
    u.i &= 0x80000000;
    u.i |= hx;

    /*
     * First step Newton iteration (solving t*t-x/t == 0) to 16 bits.  In
     * double precision so that its terms can be arranged for efficiency
     * without causing overflow or underflow.
     */
    cbrtf_c__T = u.f;
    r = cbrtf_c__T*cbrtf_c__T*cbrtf_c__T;
    cbrtf_c__T = cbrtf_c__T*((double_t)x+x+r)/(x+r+r);

    /*
     * Second step Newton iteration to 47 bits.  In double precision for
     * efficiency and accuracy.
     */
    r = cbrtf_c__T*cbrtf_c__T*cbrtf_c__T;
    cbrtf_c__T = cbrtf_c__T*((double_t)x+x+r)/(x+r+r);

    /* rounding to 24 bits is perfect in round-to-nearest mode */
    return cbrtf_c__T;
}

/// >>> START src/math/cbrtl.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cbrtl.c */
/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2009-2011, Bruce D. Evans, Steven G. Kargl, David Schultz.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * The argument reduction and testing for exceptional cases was
 * written by Steven G. Kargl with input from Bruce D. Evans
 * and David A. Schultz.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double cbrtl(long double x) {
    return cbrt(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
static const unsigned cbrtl_c__B1 = 709958130; /* cbrtl_c__B1 = (127-127.0/3-0.03306235651)*2**23 */

long double cbrtl(long double x) {
    union ldshape u = {x}, v;
    union {float f; uint32_t i;} uft;
    long double r, s, t, w;
    double_t dr, dt, dx;
    float_t ft;
    int e = u.i.se & 0x7fff;
    int sign = u.i.se & 0x8000;

    /*
     * If x = +-Inf, then cbrt(x) = +-Inf.
     * If x = NaN, then cbrt(x) = NaN.
     */
    if (e == 0x7fff)
        return x + x;
    if (e == 0) {
        /* Adjust subnormal numbers. */
        u.f *= 0x1p120;
        e = u.i.se & 0x7fff;
        /* If x = +-0, then cbrt(x) = +-0. */
        if (e == 0)
            return x;
        e -= 120;
    }
    e -= 0x3fff;
    u.i.se = 0x3fff;
    x = u.f;
    switch (e % 3) {
    case 1:
    case -2:
        x *= 2;
        e--;
        break;
    case 2:
    case -1:
        x *= 4;
        e -= 2;
        break;
    }
    v.f = 1.0;
    v.i.se = sign | (0x3fff + e/3);

    /*
     * The following is the guts of s_cbrtf, with the handling of
     * special values removed and extra care for accuracy not taken,
     * but with most of the extra accuracy not discarded.
     */

    /* ~5-bit estimate: */
    uft.f = x;
    uft.i = (uft.i & 0x7fffffff)/3 + cbrtl_c__B1;
    ft = uft.f;

    /* ~16-bit estimate: */
    dx = x;
    dt = ft;
    dr = dt * dt * dt;
    dt = dt * (dx + dx + dr) / (dx + dr + dr);

    /* ~47-bit estimate: */
    dr = dt * dt * dt;
    dt = dt * (dx + dx + dr) / (dx + dr + dr);

#if LDBL_MANT_DIG == 64
    /*
     * dt is cbrtl(x) to ~47 bits (after x has been reduced to 1 <= x < 8).
     * Round it away from cbrtl_c__zero to 32 bits (32 so that t*t is exact, and
     * away from cbrtl_c__zero for technical reasons).
     */
    t = dt + (0x1.0p32L + 0x1.0p-31L) - 0x1.0p32;
#elif LDBL_MANT_DIG == 113
    /*
     * Round dt away from cbrtl_c__zero to 47 bits.  Since we don't trust the 47,
     * add 2 47-bit ulps instead of 1 to round up.  Rounding is slow and
     * might be avoidable in this case, since on most machines dt will
     * have been evaluated in 53-bit precision and the technical reasons
     * for rounding up might not apply to either case in cbrtl() since
     * dt is much more accurate than needed.
     */
    t = dt + 0x2.0p-46 + 0x1.0p60L - 0x1.0p60;
#endif

    /*
     * Final step Newton iteration to 64 or 113 bits with
     * error < 0.667 ulps
     */
    s = t*t;         /* t*t is exact */
    r = x/s;         /* error <= 0.5 ulps; |r| < |t| */
    w = t+t;         /* t+t is exact */
    r = (r-t)/(w+r); /* r-t is exact; w+r ~= 3*t */
    t = t+t*r;       /* error <= 0.5 + 0.5/3 + epsilon */

    t *= v.f;
    return t;
}
#endif

/// >>> START src/math/ceil.c

double ceil(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = u.i >> 52 & 0x7ff;
    double_t y;

    if (e >= 0x3ff+52 || x == 0)
        return x;
    /* y = int(x) - x, where int(x) is an integer neighbor of x */
    if (u.i >> 63)
        y = (double)(x - 0x1p52) + 0x1p52 - x;
    else
        y = (double)(x + 0x1p52) - 0x1p52 - x;
    /* special case because of non-nearest rounding modes */
    if (e <= 0x3ff-1) {
        FORCE_EVAL(y);
        return u.i >> 63 ? -0.0 : 1;
    }
    if (y < 0)
        return x + y + 1;
    return x + y;
}

/// >>> START src/math/ceilf.c

float ceilf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = (int)(u.i >> 23 & 0xff) - 0x7f;
    uint32_t m;

    if (e >= 23)
        return x;
    if (e >= 0) {
        m = 0x007fffff >> e;
        if ((u.i & m) == 0)
            return x;
        FORCE_EVAL(x + 0x1p120f);
        if (u.i >> 31 == 0)
            u.i += m;
        u.i &= ~m;
    } else {
        FORCE_EVAL(x + 0x1p120f);
        if (u.i >> 31)
            u.f = -0.0;
        else if (u.i << 1)
            u.f = 1.0;
    }
    return u.f;
}

/// >>> START src/math/ceill.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double ceill(long double x) {
    return ceil(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double ceill(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    long double y;

    if (e >= 0x3fff+LDBL_MANT_DIG-1 || x == 0)
        return x;
    /* y = int(x) - x, where int(x) is an integer neighbor of x */
    if (u.i.se >> 15)
        y = x - TOINT + TOINT - x;
    else
        y = x + TOINT - TOINT - x;
    /* special case because of non-nearest rounding modes */
    if (e <= 0x3fff-1) {
        FORCE_EVAL(y);
        return u.i.se >> 15 ? -0.0 : 1;
    }
    if (y < 0)
        return x + y + 1;
    return x + y;
}
#endif

/// >>> START src/math/copysign.c

double copysign(double x, double y) {
    union {double f; uint64_t i;} ux={x}, uy={y};
    ux.i &= -1ULL/2;
    ux.i |= uy.i & 1ULL<<63;
    return ux.f;
}

/// >>> START src/math/copysignf.c

float copysignf(float x, float y) {
    union {float f; uint32_t i;} ux={x}, uy={y};
    ux.i &= 0x7fffffff;
    ux.i |= uy.i & 0x80000000;
    return ux.f;
}

/// >>> START src/math/copysignl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double copysignl(long double x, long double y) {
    return copysign(x, y);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double copysignl(long double x, long double y) {
    union ldshape ux = {x}, uy = {y};
    ux.i.se &= 0x7fff;
    ux.i.se |= uy.i.se & 0x8000;
    return ux.f;
}
#endif

/// >>> START src/math/cos.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* cos(x)
 * Return cosine function of x.
 *
 * kernel function:
 *      __sin           ... sine function on [-cos_c__pi/4,cos_c__pi/4]
 *      __cos           ... cosine function on [-cos_c__pi/4,cos_c__pi/4]
 *      __rem_pio2      ... argument reduction routine
 *
 * Method.
 *      Let cos_c__S,C and cos_c__T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-cos_c__k*cos_c__pi/2
 *      in [-cos_c__pi/4 , +cos_c__pi/4], and let n = cos_c__k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          cos_c__S           C             cos_c__T
 *          1          C          -cos_c__S            -1/cos_c__T
 *          2         -cos_c__S          -C             cos_c__T
 *          3         -C           cos_c__S            -1/cos_c__T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double cos(double x) {
    double y[2];
    uint32_t ix;
    unsigned n;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    /* |x| ~< cos_c__pi/4 */
    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e46a09e) {  /* |x| < 2**-27 * sqrt(2) */
            /* raise inexact if x!=0 */
            FORCE_EVAL(x + 0x1p120f);
            return 1.0;
        }
        return __cos(x, 0);
    }

    /* cos(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000)
        return x-x;

    /* argument reduction */
    n = __rem_pio2(x, y);
    switch (n&3) {
    case 0: return  __cos(y[0], y[1]);
    case 1: return -__sin(y[0], y[1], 1);
    case 2: return -__cos(y[0], y[1]);
    default:
        return  __sin(y[0], y[1], 1);
    }
}

/// >>> START src/math/cosf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_cosf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* Small multiples of cosf_c__pi/2 rounded to double precision. */
static const double
c1pio2 = 1*M_PI_2, /* 0x3FF921FB, 0x54442D18 */
c2pio2 = 2*M_PI_2, /* 0x400921FB, 0x54442D18 */
c3pio2 = 3*M_PI_2, /* 0x4012D97C, 0x7F3321D2 */
c4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

float cosf(float x) {
    double y;
    uint32_t ix;
    unsigned n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= cosf_c__pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x != 0 */
            FORCE_EVAL(x + 0x1p120f);
            return 1.0f;
        }
        return __cosdf(x);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*cosf_c__pi/4 */
        if (ix > 0x4016cbe3)  /* |x|  ~> 3*cosf_c__pi/4 */
            return -__cosdf(sign ? x+c2pio2 : x-c2pio2);
        else {
            if (sign)
                return __sindf(x + c1pio2);
            else
                return __sindf(c1pio2 - x);
        }
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*cosf_c__pi/4 */
        if (ix > 0x40afeddf)  /* |x| ~> 7*cosf_c__pi/4 */
            return __cosdf(sign ? x+c4pio2 : x-c4pio2);
        else {
            if (sign)
                return __sindf(-x - c3pio2);
            else
                return __sindf(x - c3pio2);
        }
    }

    /* cos(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x-x;

    /* general argument reduction needed */
    n = __rem_pio2f(x,&y);
    switch (n&3) {
    case 0: return  __cosdf(y);
    case 1: return  __sindf(-y);
    case 2: return -__cosdf(y);
    default:
        return  __sindf(y);
    }
}

/// >>> START src/math/cosh.c

/* cosh(x) = (exp(x) + 1/exp(x))/2
 *         = 1 + 0.5*(exp(x)-1)*(exp(x)-1)/exp(x)
 *         = 1 + x*x/2 + o(x^4)
 */
double cosh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    uint32_t w;
    double t;

    /* |x| */
    u.i &= (uint64_t)-1/2;
    x = u.f;
    w = u.i >> 32;

    /* |x| < log(2) */
    if (w < 0x3fe62e42) {
        if (w < 0x3ff00000 - (26<<20)) {
            /* raise inexact if x!=0 */
            FORCE_EVAL(x + 0x1p120f);
            return 1;
        }
        t = expm1(x);
        return 1 + t*t/(2*(1+t));
    }

    /* |x| < log(DBL_MAX) */
    if (w < 0x40862e42) {
        t = exp(x);
        /* note: if x>log(0x1p26) then the 1/t is not needed */
        return 0.5*(t + 1/t);
    }

    /* |x| > log(DBL_MAX) or nan */
    /* note: the result is stored to handle overflow */
    t = __expo2(x);
    return t;
}

/// >>> START src/math/coshf.c

float coshf(float x) {
    union {float f; uint32_t i;} u = {.f = x};
    uint32_t w;
    float t;

    /* |x| */
    u.i &= 0x7fffffff;
    x = u.f;
    w = u.i;

    /* |x| < log(2) */
    if (w < 0x3f317217) {
        if (w < 0x3f800000 - (12<<23)) {
            FORCE_EVAL(x + 0x1p120f);
            return 1;
        }
        t = expm1f(x);
        return 1 + t*t/(2*(1+t));
    }

    /* |x| < log(FLT_MAX) */
    if (w < 0x42b17217) {
        t = expf(x);
        return 0.5f*(t + 1/t);
    }

    /* |x| > log(FLT_MAX) or nan */
    t = __expo2f(x);
    return t;
}

/// >>> START src/math/coshl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double coshl(long double x) {
    return cosh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
long double coshl(long double x) {
    union ldshape u = {x};
    unsigned ex = u.i.se & 0x7fff;
    uint32_t w;
    long double t;

    /* |x| */
    u.i.se = ex;
    x = u.f;
    w = u.i.m >> 32;

    /* |x| < log(2) */
    if (ex < 0x3fff-1 || (ex == 0x3fff-1 && w < 0xb17217f7)) {
        if (ex < 0x3fff-32) {
            FORCE_EVAL(x + 0x1p120f);
            return 1;
        }
        t = expm1l(x);
        return 1 + t*t/(2*(1+t));
    }

    /* |x| < log(LDBL_MAX) */
    if (ex < 0x3fff+13 || (ex == 0x3fff+13 && w < 0xb17217f7)) {
        t = expl(x);
        return 0.5*(t + 1/t);
    }

    /* |x| > log(LDBL_MAX) or nan */
    t = expl(0.5*x);
    return 0.5*t*t;
}
#endif

/// >>> START src/math/cosl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double cosl(long double x) {
    return cos(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double cosl(long double x) {
    union ldshape u = {x};
    unsigned n;
    long double y[2], hi, lo;

    u.i.se &= 0x7fff;
    if (u.i.se == 0x7fff)
        return x - x;
    x = u.f;
    if (x < M_PI_4) {
        if (u.i.se < 0x3fff - LDBL_MANT_DIG)
            /* raise inexact if x!=0 */
            return 1.0 + x;
        return __cosl(x, 0);
    }
    n = __rem_pio2l(x, y);
    hi = y[0];
    lo = y[1];
    switch (n & 3) {
    case 0:
        return __cosl(hi, lo);
    case 1:
        return -__sinl(hi, lo, 1);
    case 2:
        return -__cosl(hi, lo);
    case 3:
    default:
        return __sinl(hi, lo, 1);
    }
}
#endif

/// >>> START src/math/erf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_erf.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* double erf(double x)
 * double erfc(double x)
 *                           x
 *                    2      |\
 *     erf(x)  =  ---------  | exp(-t*t)dt
 *                 sqrt(erf_c__pi) \|
 *                           0
 *
 *     erfc(x) =  1-erf(x)
 *  Note that
 *              erf(-x) = -erf(x)
 *              erfc(-x) = 2 - erfc(x)
 *
 * Method:
 *      1. For |x| in [0, 0.84375]
 *          erf(x)  = x + x*erf_c__R(x^2)
 *          erfc(x) = 1 - erf(x)           if x in [-.84375,0.25]
 *                  = 0.5 + ((0.5-x)-x*erf_c__R)  if x in [0.25,0.84375]
 *         where erf_c__R = erf_c__P/erf_c__Q where erf_c__P is an odd poly of degree 8 and
 *         erf_c__Q is an odd poly of degree 10.
 *                                               -57.90
 *                      | erf_c__R - (erf(x)-x)/x | <= 2
 *
 *
 *         Remark. The formula is derived by noting
 *          erf(x) = (2/sqrt(erf_c__pi))*(x - x^3/3 + x^5/10 - x^7/42 + ....)
 *         and that
 *          2/sqrt(erf_c__pi) = 1.128379167095512573896158903121545171688
 *         is close to one. The interval is chosen because the fix
 *         point of erf(x) is near 0.6174 (i.e., erf(x)=x when x is
 *         near 0.6174), and by some experiment, 0.84375 is chosen to
 *         guarantee the error is less than one ulp for erf.
 *
 *      2. For |x| in [0.84375,1.25], let s = |x| - 1, and
 *         c = 0.84506291151 rounded to single (24 bits)
 *              erf(x)  = sign(x) * (c  + erf_c__P1(s)/erf_c__Q1(s))
 *              erfc(x) = (1-c)  - erf_c__P1(s)/erf_c__Q1(s) if x > 0
 *                        1+(c+erf_c__P1(s)/erf_c__Q1(s))    if x < 0
 *              |erf_c__P1/erf_c__Q1 - (erf(|x|)-c)| <= 2**-59.06
 *         Remark: here we use the taylor series expansion at x=1.
 *              erf(1+s) = erf(1) + s*Poly(s)
 *                       = 0.845.. + erf_c__P1(s)/erf_c__Q1(s)
 *         That is, we use rational approximation to approximate
 *                      erf(1+s) - (c = (single)0.84506291151)
 *         Note that |erf_c__P1/erf_c__Q1|< 0.078 for x in [0.84375,1.25]
 *         where
 *              erf_c__P1(s) = degree 6 poly in s
 *              erf_c__Q1(s) = degree 6 poly in s
 *
 *      3. For x in [1.25,1/0.35(~2.857143)],
 *              erfc(x) = (1/x)*exp(-x*x-0.5625+R1/erf_c__S1)
 *              erf(x)  = 1 - erfc(x)
 *         where
 *              R1(z) = degree 7 poly in z, (z=1/x^2)
 *              erf_c__S1(z) = degree 8 poly in z
 *
 *      4. For x in [1/0.35,28]
 *              erfc(x) = (1/x)*exp(-x*x-0.5625+R2/erf_c__S2) if x > 0
 *                      = 2.0 - (1/x)*exp(-x*x-0.5625+R2/erf_c__S2) if -6<x<0
 *                      = 2.0 - erf_c__tiny            (if x <= -6)
 *              erf(x)  = sign(x)*(1.0 - erfc(x)) if x < 6, else
 *              erf(x)  = sign(x)*(1.0 - erf_c__tiny)
 *         where
 *              R2(z) = degree 6 poly in z, (z=1/x^2)
 *              erf_c__S2(z) = degree 7 poly in z
 *
 *      Note1:
 *         To compute exp(-x*x-0.5625+erf_c__R/erf_c__S), let s be a single
 *         precision number and s := x; then
 *              -x*x = -s*s + (s-x)*(s+x)
 *              exp(-x*x-0.5626+erf_c__R/erf_c__S) =
 *                      exp(-s*s-0.5625)*exp((s-x)*(s+x)+erf_c__R/erf_c__S);
 *      Note2:
 *         Here 4 and 5 make use of the asymptotic series
 *                        exp(-x*x)
 *              erfc(x) ~ ---------- * ( 1 + Poly(1/x^2) )
 *                        x*sqrt(erf_c__pi)
 *         We use rational approximation to approximate
 *              g(s)=f(1/x^2) = log(erfc(x)*x) - x*x + 0.5625
 *         Here is the error bound for R1/erf_c__S1 and R2/erf_c__S2
 *              |R1/erf_c__S1 - f(x)|  < 2**(-62.57)
 *              |R2/erf_c__S2 - f(x)|  < 2**(-61.52)
 *
 *      5. For inf > x >= 28
 *              erf(x)  = sign(x) *(1 - erf_c__tiny)  (raise inexact)
 *              erfc(x) = erf_c__tiny*erf_c__tiny (raise underflow) if x > 0
 *                      = 2 - erf_c__tiny if x<0
 *
 *      7. Special case:
 *              erf(0)  = 0, erf(inf)  = 1, erf(-inf) = -1,
 *              erfc(0) = 1, erfc(inf) = 0, erfc(-inf) = 2,
 *              erfc/erf(NaN) is NaN
 */

static const double
erf_c__erx  = 8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
erf_c__efx8 =  1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
erf_c__pp0  =  1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
erf_c__pp1  = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
erf_c__pp2  = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
erf_c__pp3  = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
erf_c__pp4  = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
erf_c__qq1  =  3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
erf_c__qq2  =  6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
erf_c__qq3  =  5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
erf_c__qq4  =  1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
erf_c__qq5  = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
erf_c__pa0  = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
erf_c__pa1  =  4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
erf_c__pa2  = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
erf_c__pa3  =  3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
erf_c__pa4  = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
erf_c__pa5  =  3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
erf_c__pa6  = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
erf_c__qa1  =  1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
erf_c__qa2  =  5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
erf_c__qa3  =  7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
erf_c__qa4  =  1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
erf_c__qa5  =  1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
erf_c__qa6  =  1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
/*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */
erf_c__ra0  = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
erf_c__ra1  = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
erf_c__ra2  = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
erf_c__ra3  = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
erf_c__ra4  = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
erf_c__ra5  = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
erf_c__ra6  = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
erf_c__ra7  = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
erf_c__sa1  =  1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
erf_c__sa2  =  1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
erf_c__sa3  =  4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
erf_c__sa4  =  6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
erf_c__sa5  =  4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
erf_c__sa6  =  1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
erf_c__sa7  =  6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
erf_c__sa8  = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
/*
 * Coefficients for approximation to  erfc in [1/.35,28]
 */
erf_c__rb0  = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
erf_c__rb1  = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
erf_c__rb2  = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
erf_c__rb3  = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
erf_c__rb4  = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
erf_c__rb5  = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
erf_c__rb6  = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
erf_c__sb1  =  3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
erf_c__sb2  =  3.25792512996573918826e+02, /* 0x40745CAE, 0x221B9F0A */
erf_c__sb3  =  1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
erf_c__sb4  =  3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
erf_c__sb5  =  2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
erf_c__sb6  =  4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
erf_c__sb7  = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

static double erf_c__erfc1(double x) {
    double_t s,erf_c__P,erf_c__Q;

    s = fabs(x) - 1;
    erf_c__P = erf_c__pa0+s*(erf_c__pa1+s*(erf_c__pa2+s*(erf_c__pa3+s*(erf_c__pa4+s*(erf_c__pa5+s*erf_c__pa6)))));
    erf_c__Q = 1+s*(erf_c__qa1+s*(erf_c__qa2+s*(erf_c__qa3+s*(erf_c__qa4+s*(erf_c__qa5+s*erf_c__qa6)))));
    return 1 - erf_c__erx - erf_c__P/erf_c__Q;
}

static double erf_c__erfc2(uint32_t ix, double x) {
    double_t s,erf_c__R,erf_c__S;
    double z;

    if (ix < 0x3ff40000)  /* |x| < 1.25 */
        return erf_c__erfc1(x);

    x = fabs(x);
    s = 1/(x*x);
    if (ix < 0x4006db6d) {  /* |x| < 1/.35 ~ 2.85714 */
        erf_c__R = erf_c__ra0+s*(erf_c__ra1+s*(erf_c__ra2+s*(erf_c__ra3+s*(erf_c__ra4+s*(
             erf_c__ra5+s*(erf_c__ra6+s*erf_c__ra7))))));
        erf_c__S = 1.0+s*(erf_c__sa1+s*(erf_c__sa2+s*(erf_c__sa3+s*(erf_c__sa4+s*(
             erf_c__sa5+s*(erf_c__sa6+s*(erf_c__sa7+s*erf_c__sa8)))))));
    } else {                /* |x| > 1/.35 */
        erf_c__R = erf_c__rb0+s*(erf_c__rb1+s*(erf_c__rb2+s*(erf_c__rb3+s*(erf_c__rb4+s*(
             erf_c__rb5+s*erf_c__rb6)))));
        erf_c__S = 1.0+s*(erf_c__sb1+s*(erf_c__sb2+s*(erf_c__sb3+s*(erf_c__sb4+s*(
             erf_c__sb5+s*(erf_c__sb6+s*erf_c__sb7))))));
    }
    z = x;
    SET_LOW_WORD(z,0);
    return exp(-z*z-0.5625)*exp((z-x)*(z+x)+erf_c__R/erf_c__S)/x;
}

double erf(double x) {
    double r,s,z,y;
    uint32_t ix;
    int sign;

    GET_HIGH_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7ff00000) {
        /* erf(nan)=nan, erf(+-inf)=+-1 */
        return 1-2*sign + 1/x;
    }
    if (ix < 0x3feb0000) {  /* |x| < 0.84375 */
        if (ix < 0x3e300000) {  /* |x| < 2**-28 */
            /* avoid underflow */
            return 0.125*(8*x + erf_c__efx8*x);
        }
        z = x*x;
        r = erf_c__pp0+z*(erf_c__pp1+z*(erf_c__pp2+z*(erf_c__pp3+z*erf_c__pp4)));
        s = 1.0+z*(erf_c__qq1+z*(erf_c__qq2+z*(erf_c__qq3+z*(erf_c__qq4+z*erf_c__qq5))));
        y = r/s;
        return x + x*y;
    }
    if (ix < 0x40180000)  /* 0.84375 <= |x| < 6 */
        y = 1 - erf_c__erfc2(ix,x);
    else
        y = 1 - 0x1p-1022;
    return sign ? -y : y;
}

double erfc(double x) {
    double r,s,z,y;
    uint32_t ix;
    int sign;

    GET_HIGH_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7ff00000) {
        /* erfc(nan)=nan, erfc(+-inf)=0,2 */
        return 2*sign + 1/x;
    }
    if (ix < 0x3feb0000) {  /* |x| < 0.84375 */
        if (ix < 0x3c700000)  /* |x| < 2**-56 */
            return 1.0 - x;
        z = x*x;
        r = erf_c__pp0+z*(erf_c__pp1+z*(erf_c__pp2+z*(erf_c__pp3+z*erf_c__pp4)));
        s = 1.0+z*(erf_c__qq1+z*(erf_c__qq2+z*(erf_c__qq3+z*(erf_c__qq4+z*erf_c__qq5))));
        y = r/s;
        if (sign || ix < 0x3fd00000) {  /* x < 1/4 */
            return 1.0 - (x+x*y);
        }
        return 0.5 - (x - 0.5 + x*y);
    }
    if (ix < 0x403c0000) {  /* 0.84375 <= |x| < 28 */
        return sign ? 2 - erf_c__erfc2(ix,x) : erf_c__erfc2(ix,x);
    }
    return sign ? 2 - 0x1p-1022 : 0x1p-1022*0x1p-1022;
}

/// >>> START src/math/erff.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_erff.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
erff_c__erx  =  8.4506291151e-01, /* 0x3f58560b */
/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
erff_c__efx8 =  1.0270333290e+00, /* 0x3f8375d4 */
erff_c__pp0  =  1.2837916613e-01, /* 0x3e0375d4 */
erff_c__pp1  = -3.2504209876e-01, /* 0xbea66beb */
erff_c__pp2  = -2.8481749818e-02, /* 0xbce9528f */
erff_c__pp3  = -5.7702702470e-03, /* 0xbbbd1489 */
erff_c__pp4  = -2.3763017452e-05, /* 0xb7c756b1 */
erff_c__qq1  =  3.9791721106e-01, /* 0x3ecbbbce */
erff_c__qq2  =  6.5022252500e-02, /* 0x3d852a63 */
erff_c__qq3  =  5.0813062117e-03, /* 0x3ba68116 */
erff_c__qq4  =  1.3249473704e-04, /* 0x390aee49 */
erff_c__qq5  = -3.9602282413e-06, /* 0xb684e21a */
/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
erff_c__pa0  = -2.3621185683e-03, /* 0xbb1acdc6 */
erff_c__pa1  =  4.1485610604e-01, /* 0x3ed46805 */
erff_c__pa2  = -3.7220788002e-01, /* 0xbebe9208 */
erff_c__pa3  =  3.1834661961e-01, /* 0x3ea2fe54 */
erff_c__pa4  = -1.1089469492e-01, /* 0xbde31cc2 */
erff_c__pa5  =  3.5478305072e-02, /* 0x3d1151b3 */
erff_c__pa6  = -2.1663755178e-03, /* 0xbb0df9c0 */
erff_c__qa1  =  1.0642088205e-01, /* 0x3dd9f331 */
erff_c__qa2  =  5.4039794207e-01, /* 0x3f0a5785 */
erff_c__qa3  =  7.1828655899e-02, /* 0x3d931ae7 */
erff_c__qa4  =  1.2617121637e-01, /* 0x3e013307 */
erff_c__qa5  =  1.3637083583e-02, /* 0x3c5f6e13 */
erff_c__qa6  =  1.1984500103e-02, /* 0x3c445aa3 */
/*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */
erff_c__ra0  = -9.8649440333e-03, /* 0xbc21a093 */
erff_c__ra1  = -6.9385856390e-01, /* 0xbf31a0b7 */
erff_c__ra2  = -1.0558626175e+01, /* 0xc128f022 */
erff_c__ra3  = -6.2375331879e+01, /* 0xc2798057 */
erff_c__ra4  = -1.6239666748e+02, /* 0xc322658c */
erff_c__ra5  = -1.8460508728e+02, /* 0xc3389ae7 */
erff_c__ra6  = -8.1287437439e+01, /* 0xc2a2932b */
erff_c__ra7  = -9.8143291473e+00, /* 0xc11d077e */
erff_c__sa1  =  1.9651271820e+01, /* 0x419d35ce */
erff_c__sa2  =  1.3765776062e+02, /* 0x4309a863 */
erff_c__sa3  =  4.3456588745e+02, /* 0x43d9486f */
erff_c__sa4  =  6.4538726807e+02, /* 0x442158c9 */
erff_c__sa5  =  4.2900814819e+02, /* 0x43d6810b */
erff_c__sa6  =  1.0863500214e+02, /* 0x42d9451f */
erff_c__sa7  =  6.5702495575e+00, /* 0x40d23f7c */
erff_c__sa8  = -6.0424413532e-02, /* 0xbd777f97 */
/*
 * Coefficients for approximation to  erfc in [1/.35,28]
 */
erff_c__rb0  = -9.8649431020e-03, /* 0xbc21a092 */
erff_c__rb1  = -7.9928326607e-01, /* 0xbf4c9dd4 */
erff_c__rb2  = -1.7757955551e+01, /* 0xc18e104b */
erff_c__rb3  = -1.6063638306e+02, /* 0xc320a2ea */
erff_c__rb4  = -6.3756646729e+02, /* 0xc41f6441 */
erff_c__rb5  = -1.0250950928e+03, /* 0xc480230b */
erff_c__rb6  = -4.8351919556e+02, /* 0xc3f1c275 */
erff_c__sb1  =  3.0338060379e+01, /* 0x41f2b459 */
erff_c__sb2  =  3.2579251099e+02, /* 0x43a2e571 */
erff_c__sb3  =  1.5367296143e+03, /* 0x44c01759 */
erff_c__sb4  =  3.1998581543e+03, /* 0x4547fdbb */
erff_c__sb5  =  2.5530502930e+03, /* 0x451f90ce */
erff_c__sb6  =  4.7452853394e+02, /* 0x43ed43a7 */
erff_c__sb7  = -2.2440952301e+01; /* 0xc1b38712 */

static float erff_c__erfc1(float x) {
    float_t s,erff_c__P,erff_c__Q;

    s = fabsf(x) - 1;
    erff_c__P = erff_c__pa0+s*(erff_c__pa1+s*(erff_c__pa2+s*(erff_c__pa3+s*(erff_c__pa4+s*(erff_c__pa5+s*erff_c__pa6)))));
    erff_c__Q = 1+s*(erff_c__qa1+s*(erff_c__qa2+s*(erff_c__qa3+s*(erff_c__qa4+s*(erff_c__qa5+s*erff_c__qa6)))));
    return 1 - erff_c__erx - erff_c__P/erff_c__Q;
}

static float erff_c__erfc2(uint32_t ix, float x) {
    float_t s,erff_c__R,erff_c__S;
    float z;

    if (ix < 0x3fa00000)  /* |x| < 1.25 */
        return erff_c__erfc1(x);

    x = fabsf(x);
    s = 1/(x*x);
    if (ix < 0x4036db6d) {   /* |x| < 1/0.35 */
        erff_c__R = erff_c__ra0+s*(erff_c__ra1+s*(erff_c__ra2+s*(erff_c__ra3+s*(erff_c__ra4+s*(
             erff_c__ra5+s*(erff_c__ra6+s*erff_c__ra7))))));
        erff_c__S = 1.0f+s*(erff_c__sa1+s*(erff_c__sa2+s*(erff_c__sa3+s*(erff_c__sa4+s*(
             erff_c__sa5+s*(erff_c__sa6+s*(erff_c__sa7+s*erff_c__sa8)))))));
    } else {                 /* |x| >= 1/0.35 */
        erff_c__R = erff_c__rb0+s*(erff_c__rb1+s*(erff_c__rb2+s*(erff_c__rb3+s*(erff_c__rb4+s*(
             erff_c__rb5+s*erff_c__rb6)))));
        erff_c__S = 1.0f+s*(erff_c__sb1+s*(erff_c__sb2+s*(erff_c__sb3+s*(erff_c__sb4+s*(
             erff_c__sb5+s*(erff_c__sb6+s*erff_c__sb7))))));
    }
    GET_FLOAT_WORD(ix, x);
    SET_FLOAT_WORD(z, ix&0xffffe000);
    return expf(-z*z - 0.5625f) * expf((z-x)*(z+x) + erff_c__R/erff_c__S)/x;
}

float erff(float x) {
    float r,s,z,y;
    uint32_t ix;
    int sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7f800000) {
        /* erf(nan)=nan, erf(+-inf)=+-1 */
        return 1-2*sign + 1/x;
    }
    if (ix < 0x3f580000) {  /* |x| < 0.84375 */
        if (ix < 0x31800000) {  /* |x| < 2**-28 */
            /*avoid underflow */
            return 0.125f*(8*x + erff_c__efx8*x);
        }
        z = x*x;
        r = erff_c__pp0+z*(erff_c__pp1+z*(erff_c__pp2+z*(erff_c__pp3+z*erff_c__pp4)));
        s = 1+z*(erff_c__qq1+z*(erff_c__qq2+z*(erff_c__qq3+z*(erff_c__qq4+z*erff_c__qq5))));
        y = r/s;
        return x + x*y;
    }
    if (ix < 0x40c00000)  /* |x| < 6 */
        y = 1 - erff_c__erfc2(ix,x);
    else
        y = 1 - 0x1p-120f;
    return sign ? -y : y;
}

float erfcf(float x) {
    float r,s,z,y;
    uint32_t ix;
    int sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7f800000) {
        /* erfc(nan)=nan, erfc(+-inf)=0,2 */
        return 2*sign + 1/x;
    }

    if (ix < 0x3f580000) {  /* |x| < 0.84375 */
        if (ix < 0x23800000)  /* |x| < 2**-56 */
            return 1.0f - x;
        z = x*x;
        r = erff_c__pp0+z*(erff_c__pp1+z*(erff_c__pp2+z*(erff_c__pp3+z*erff_c__pp4)));
        s = 1.0f+z*(erff_c__qq1+z*(erff_c__qq2+z*(erff_c__qq3+z*(erff_c__qq4+z*erff_c__qq5))));
        y = r/s;
        if (sign || ix < 0x3e800000)  /* x < 1/4 */
            return 1.0f - (x+x*y);
        return 0.5f - (x - 0.5f + x*y);
    }
    if (ix < 0x41e00000) {  /* |x| < 28 */
        return sign ? 2 - erff_c__erfc2(ix,x) : erff_c__erfc2(ix,x);
    }
    return sign ? 2 - 0x1p-120f : 0x1p-120f*0x1p-120f;
}

/// >>> START src/math/erfl.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_erfl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* double erf(double x)
 * double erfc(double x)
 *                           x
 *                    2      |\
 *     erf(x)  =  ---------  | exp(-t*t)dt
 *                 sqrt(erfl_c__pi) \|
 *                           0
 *
 *     erfc(x) =  1-erf(x)
 *  Note that
 *              erf(-x) = -erf(x)
 *              erfc(-x) = 2 - erfc(x)
 *
 * Method:
 *      1. For |x| in [0, 0.84375]
 *          erf(x)  = x + x*erfl_c__R(x^2)
 *          erfc(x) = 1 - erf(x)           if x in [-.84375,0.25]
 *                  = 0.5 + ((0.5-x)-x*erfl_c__R)  if x in [0.25,0.84375]
 *         Remark. The formula is derived by noting
 *          erf(x) = (2/sqrt(erfl_c__pi))*(x - x^3/3 + x^5/10 - x^7/42 + ....)
 *         and that
 *          2/sqrt(erfl_c__pi) = 1.128379167095512573896158903121545171688
 *         is close to one. The interval is chosen because the fix
 *         point of erf(x) is near 0.6174 (i.e., erf(x)=x when x is
 *         near 0.6174), and by some experiment, 0.84375 is chosen to
 *         guarantee the error is less than one ulp for erf.
 *
 *      2. For |x| in [0.84375,1.25], let s = |x| - 1, and
 *         c = 0.84506291151 rounded to single (24 bits)
 *      erf(x)  = sign(x) * (c  + erfl_c__P1(s)/erfl_c__Q1(s))
 *      erfc(x) = (1-c)  - erfl_c__P1(s)/erfl_c__Q1(s) if x > 0
 *                        1+(c+erfl_c__P1(s)/erfl_c__Q1(s))    if x < 0
 *         Remark: here we use the taylor series expansion at x=1.
 *              erf(1+s) = erf(1) + s*Poly(s)
 *                       = 0.845.. + erfl_c__P1(s)/erfl_c__Q1(s)
 *         Note that |erfl_c__P1/erfl_c__Q1|< 0.078 for x in [0.84375,1.25]
 *
 *      3. For x in [1.25,1/0.35(~2.857143)],
 *      erfc(x) = (1/x)*exp(-x*x-0.5625+R1(z)/erfl_c__S1(z))
 *              z=1/x^2
 *      erf(x)  = 1 - erfc(x)
 *
 *      4. For x in [1/0.35,107]
 *      erfc(x) = (1/x)*exp(-x*x-0.5625+R2/erfl_c__S2) if x > 0
 *                      = 2.0 - (1/x)*exp(-x*x-0.5625+R2(z)/erfl_c__S2(z))
 *                             if -6.666<x<0
 *                      = 2.0 - erfl_c__tiny            (if x <= -6.666)
 *              z=1/x^2
 *      erf(x)  = sign(x)*(1.0 - erfc(x)) if x < 6.666, else
 *      erf(x)  = sign(x)*(1.0 - erfl_c__tiny)
 *      Note1:
 *         To compute exp(-x*x-0.5625+erfl_c__R/erfl_c__S), let s be a single
 *         precision number and s := x; then
 *              -x*x = -s*s + (s-x)*(s+x)
 *              exp(-x*x-0.5626+erfl_c__R/erfl_c__S) =
 *                      exp(-s*s-0.5625)*exp((s-x)*(s+x)+erfl_c__R/erfl_c__S);
 *      Note2:
 *         Here 4 and 5 make use of the asymptotic series
 *                        exp(-x*x)
 *              erfc(x) ~ ---------- * ( 1 + Poly(1/x^2) )
 *                        x*sqrt(erfl_c__pi)
 *
 *      5. For inf > x >= 107
 *      erf(x)  = sign(x) *(1 - erfl_c__tiny)  (raise inexact)
 *      erfc(x) = erfl_c__tiny*erfl_c__tiny (raise underflow) if x > 0
 *                      = 2 - erfl_c__tiny if x<0
 *
 *      7. Special case:
 *      erf(0)  = 0, erf(inf)  = 1, erf(-inf) = -1,
 *      erfc(0) = 1, erfc(inf) = 0, erfc(-inf) = 2,
 *              erfc/erf(NaN) is NaN
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double erfl(long double x) {
    return erf(x);
}
long double erfcl(long double x) {
    return erfc(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
static const long double
erfl_c__erx = 0.845062911510467529296875L,

/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
/* 8 * (2/sqrt(erfl_c__pi) - 1) */
erfl_c__efx8 = 1.0270333367641005911692712249723613735048E0L,
pp[6] = {
    1.122751350964552113068262337278335028553E6L,
    -2.808533301997696164408397079650699163276E6L,
    -3.314325479115357458197119660818768924100E5L,
    -6.848684465326256109712135497895525446398E4L,
    -2.657817695110739185591505062971929859314E3L,
    -1.655310302737837556654146291646499062882E2L,
},
qq[6] = {
    8.745588372054466262548908189000448124232E6L,
    3.746038264792471129367533128637019611485E6L,
    7.066358783162407559861156173539693900031E5L,
    7.448928604824620999413120955705448117056E4L,
    4.511583986730994111992253980546131408924E3L,
    1.368902937933296323345610240009071254014E2L,
    /* 1.000000000000000000000000000000000000000E0 */
},

/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
/* erf(x+1) = 0.845062911510467529296875 + pa(x)/qa(x)
   -0.15625 <= x <= +.25
   Peak relative error 8.5e-22  */
pa[8] = {
    -1.076952146179812072156734957705102256059E0L,
     1.884814957770385593365179835059971587220E2L,
    -5.339153975012804282890066622962070115606E1L,
     4.435910679869176625928504532109635632618E1L,
     1.683219516032328828278557309642929135179E1L,
    -2.360236618396952560064259585299045804293E0L,
     1.852230047861891953244413872297940938041E0L,
     9.394994446747752308256773044667843200719E-2L,
},
qa[7] =  {
    4.559263722294508998149925774781887811255E2L,
    3.289248982200800575749795055149780689738E2L,
    2.846070965875643009598627918383314457912E2L,
    1.398715859064535039433275722017479994465E2L,
    6.060190733759793706299079050985358190726E1L,
    2.078695677795422351040502569964299664233E1L,
    4.641271134150895940966798357442234498546E0L,
    /* 1.000000000000000000000000000000000000000E0 */
},

/*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */
/* erfc(1/x) = x exp (-1/x^2 - 0.5625 + ra(x^2)/sa(x^2))
   1/2.85711669921875 < 1/x < 1/1.25
   Peak relative error 3.1e-21  */
ra[] = {
    1.363566591833846324191000679620738857234E-1L,
    1.018203167219873573808450274314658434507E1L,
    1.862359362334248675526472871224778045594E2L,
    1.411622588180721285284945138667933330348E3L,
    5.088538459741511988784440103218342840478E3L,
    8.928251553922176506858267311750789273656E3L,
    7.264436000148052545243018622742770549982E3L,
    2.387492459664548651671894725748959751119E3L,
    2.220916652813908085449221282808458466556E2L,
},
sa[] = {
    -1.382234625202480685182526402169222331847E1L,
    -3.315638835627950255832519203687435946482E2L,
    -2.949124863912936259747237164260785326692E3L,
    -1.246622099070875940506391433635999693661E4L,
    -2.673079795851665428695842853070996219632E4L,
    -2.880269786660559337358397106518918220991E4L,
    -1.450600228493968044773354186390390823713E4L,
    -2.874539731125893533960680525192064277816E3L,
    -1.402241261419067750237395034116942296027E2L,
    /* 1.000000000000000000000000000000000000000E0 */
},

/*
 * Coefficients for approximation to  erfc in [1/.35,107]
 */
/* erfc(1/x) = x exp (-1/x^2 - 0.5625 + rb(x^2)/sb(x^2))
   1/6.6666259765625 < 1/x < 1/2.85711669921875
   Peak relative error 4.2e-22  */
rb[] = {
    -4.869587348270494309550558460786501252369E-5L,
    -4.030199390527997378549161722412466959403E-3L,
    -9.434425866377037610206443566288917589122E-2L,
    -9.319032754357658601200655161585539404155E-1L,
    -4.273788174307459947350256581445442062291E0L,
    -8.842289940696150508373541814064198259278E0L,
    -7.069215249419887403187988144752613025255E0L,
    -1.401228723639514787920274427443330704764E0L,
},
sb[] = {
    4.936254964107175160157544545879293019085E-3L,
    1.583457624037795744377163924895349412015E-1L,
    1.850647991850328356622940552450636420484E0L,
    9.927611557279019463768050710008450625415E0L,
    2.531667257649436709617165336779212114570E1L,
    2.869752886406743386458304052862814690045E1L,
    1.182059497870819562441683560749192539345E1L,
    /* 1.000000000000000000000000000000000000000E0 */
},
/* erfc(1/x) = x exp (-1/x^2 - 0.5625 + erfl_c__rc(x^2)/erfl_c__sc(x^2))
   1/107 <= 1/x <= 1/6.6666259765625
   Peak relative error 1.1e-21  */
erfl_c__rc[] = {
    -8.299617545269701963973537248996670806850E-5L,
    -6.243845685115818513578933902532056244108E-3L,
    -1.141667210620380223113693474478394397230E-1L,
    -7.521343797212024245375240432734425789409E-1L,
    -1.765321928311155824664963633786967602934E0L,
    -1.029403473103215800456761180695263439188E0L,
},
erfl_c__sc[] = {
    8.413244363014929493035952542677768808601E-3L,
    2.065114333816877479753334599639158060979E-1L,
    1.639064941530797583766364412782135680148E0L,
    4.936788463787115555582319302981666347450E0L,
    5.005177727208955487404729933261347679090E0L,
    /* 1.000000000000000000000000000000000000000E0 */
};

static long double erfl_c__erfc1(long double x) {
    long double s,erfl_c__P,erfl_c__Q;

    s = fabsl(x) - 1;
    erfl_c__P = pa[0] + s * (pa[1] + s * (pa[2] +
         s * (pa[3] + s * (pa[4] + s * (pa[5] + s * (pa[6] + s * pa[7]))))));
    erfl_c__Q = qa[0] + s * (qa[1] + s * (qa[2] +
         s * (qa[3] + s * (qa[4] + s * (qa[5] + s * (qa[6] + s))))));
    return 1 - erfl_c__erx - erfl_c__P / erfl_c__Q;
}

static long double erfl_c__erfc2(uint32_t ix, long double x) {
    union ldshape u;
    long double s,z,erfl_c__R,erfl_c__S;

    if (ix < 0x3fffa000)  /* 0.84375 <= |x| < 1.25 */
        return erfl_c__erfc1(x);

    x = fabsl(x);
    s = 1 / (x * x);
    if (ix < 0x4000b6db) {  /* 1.25 <= |x| < 2.857 ~ 1/.35 */
        erfl_c__R = ra[0] + s * (ra[1] + s * (ra[2] + s * (ra[3] + s * (ra[4] +
             s * (ra[5] + s * (ra[6] + s * (ra[7] + s * ra[8])))))));
        erfl_c__S = sa[0] + s * (sa[1] + s * (sa[2] + s * (sa[3] + s * (sa[4] +
             s * (sa[5] + s * (sa[6] + s * (sa[7] + s * (sa[8] + s))))))));
    } else if (ix < 0x4001d555) {  /* 2.857 <= |x| < 6.6666259765625 */
        erfl_c__R = rb[0] + s * (rb[1] + s * (rb[2] + s * (rb[3] + s * (rb[4] +
             s * (rb[5] + s * (rb[6] + s * rb[7]))))));
        erfl_c__S = sb[0] + s * (sb[1] + s * (sb[2] + s * (sb[3] + s * (sb[4] +
             s * (sb[5] + s * (sb[6] + s))))));
    } else { /* 6.666 <= |x| < 107 (erfc only) */
        erfl_c__R = erfl_c__rc[0] + s * (erfl_c__rc[1] + s * (erfl_c__rc[2] + s * (erfl_c__rc[3] +
             s * (erfl_c__rc[4] + s * erfl_c__rc[5]))));
        erfl_c__S = erfl_c__sc[0] + s * (erfl_c__sc[1] + s * (erfl_c__sc[2] + s * (erfl_c__sc[3] +
             s * (erfl_c__sc[4] + s))));
    }
    u.f = x;
    u.i.m &= -1ULL << 40;
    z = u.f;
    return expl(-z*z - 0.5625) * expl((z - x) * (z + x) + erfl_c__R / erfl_c__S) / x;
}

long double erfl(long double x) {
    long double r, s, z, y;
    union ldshape u = {x};
    uint32_t ix = (u.i.se & 0x7fffU)<<16 | u.i.m>>48;
    int sign = u.i.se >> 15;

    if (ix >= 0x7fff0000)
        /* erf(nan)=nan, erf(+-inf)=+-1 */
        return 1 - 2*sign + 1/x;
    if (ix < 0x3ffed800) {  /* |x| < 0.84375 */
        if (ix < 0x3fde8000) {  /* |x| < 2**-33 */
            return 0.125 * (8 * x + erfl_c__efx8 * x);  /* avoid underflow */
        }
        z = x * x;
        r = pp[0] + z * (pp[1] +
             z * (pp[2] + z * (pp[3] + z * (pp[4] + z * pp[5]))));
        s = qq[0] + z * (qq[1] +
             z * (qq[2] + z * (qq[3] + z * (qq[4] + z * (qq[5] + z)))));
        y = r / s;
        return x + x * y;
    }
    if (ix < 0x4001d555)  /* |x| < 6.6666259765625 */
        y = 1 - erfl_c__erfc2(ix,x);
    else
        y = 1 - 0x1p-16382L;
    return sign ? -y : y;
}

long double erfcl(long double x) {
    long double r, s, z, y;
    union ldshape u = {x};
    uint32_t ix = (u.i.se & 0x7fffU)<<16 | u.i.m>>48;
    int sign = u.i.se >> 15;

    if (ix >= 0x7fff0000)
        /* erfc(nan) = nan, erfc(+-inf) = 0,2 */
        return 2*sign + 1/x;
    if (ix < 0x3ffed800) {  /* |x| < 0.84375 */
        if (ix < 0x3fbe0000)  /* |x| < 2**-65 */
            return 1.0 - x;
        z = x * x;
        r = pp[0] + z * (pp[1] +
             z * (pp[2] + z * (pp[3] + z * (pp[4] + z * pp[5]))));
        s = qq[0] + z * (qq[1] +
             z * (qq[2] + z * (qq[3] + z * (qq[4] + z * (qq[5] + z)))));
        y = r / s;
        if (ix < 0x3ffd8000) /* x < 1/4 */
            return 1.0 - (x + x * y);
        return 0.5 - (x - 0.5 + x * y);
    }
    if (ix < 0x4005d600)  /* |x| < 107 */
        return sign ? 2 - erfl_c__erfc2(ix,x) : erfl_c__erfc2(ix,x);
    y = 0x1p-16382L;
    return sign ? 2 - y : y*y;
}
#endif

/// >>> START src/math/exp.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_exp.c */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* exp(x)
 * Returns the exponential of x.
 *
 * Method
 *   1. Argument reduction:
 *      Reduce x to an r so that |r| <= 0.5*ln2 ~ 0.34658.
 *      Given x, find r and integer exp_c__k such that
 *
 *               x = exp_c__k*ln2 + r,  |r| <= 0.5*ln2.
 *
 *      Here r will be represented as r = hi-lo for better
 *      accuracy.
 *
 *   2. Approximation of exp(r) by a special rational function on
 *      the interval [0,0.34658]:
 *      Write
 *          exp_c__R(r**2) = r*(exp(r)+1)/(exp(r)-1) = 2 + r*r/6 - r**4/360 + ...
 *      We use a special Remez algorithm on [0,0.34658] to generate
 *      a polynomial of degree 5 to approximate exp_c__R. The maximum error
 *      of this polynomial approximation is bounded by 2**-59. In
 *      other words,
 *          exp_c__R(z) ~ 2.0 + exp_c__P1*z + exp_c__P2*z**2 + exp_c__P3*z**3 + exp_c__P4*z**4 + exp_c__P5*z**5
 *      (where z=r*r, and the values of exp_c__P1 to exp_c__P5 are listed below)
 *      and
 *          |                  5          |     -59
 *          | 2.0+exp_c__P1*z+...+exp_c__P5*z   -  exp_c__R(z) | <= 2
 *          |                             |
 *      The computation of exp(r) thus becomes
 *                              2*r
 *              exp(r) = 1 + ----------
 *                            exp_c__R(r) - r
 *                                 r*c(r)
 *                     = 1 + r + ----------- (for better accuracy)
 *                                2 - c(r)
 *      where
 *                              2       4             10
 *              c(r) = r - (exp_c__P1*r  + exp_c__P2*r  + ... + exp_c__P5*r   ).
 *
 *   3. Scale back to obtain exp(x):
 *      From step 1, we have
 *         exp(x) = 2^exp_c__k * exp(r)
 *
 * Special cases:
 *      exp(INF) is INF, exp(NaN) is NaN;
 *      exp(-INF) is 0, and
 *      for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Misc. info.
 *      For IEEE double
 *          if x >  709.782712893383973096 then exp(x) overflows
 *          if x < -745.133219101941108420 then exp(x) underflows
 */

static const double
exp_c__half[2] = {0.5,-0.5},
exp_c__ln2hi = 6.93147180369123816490e-01, /* 0x3fe62e42, 0xfee00000 */
exp_c__ln2lo = 1.90821492927058770002e-10, /* 0x3dea39ef, 0x35793c76 */
exp_c__invln2 = 1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
exp_c__P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
exp_c__P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
exp_c__P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
exp_c__P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
exp_c__P5   =  4.13813679705723846039e-08; /* 0x3E663769, 0x72BEA4D0 */

double exp(double x) {
    double_t hi, lo, c, xx, y;
    int exp_c__k, sign;
    uint32_t hx;

    GET_HIGH_WORD(hx, x);
    sign = hx>>31;
    hx &= 0x7fffffff;  /* high word of |x| */

    /* special cases */
    if (hx >= 0x4086232b) {  /* if |x| >= 708.39... */
        if (isnan(x))
            return x;
        if (x > 709.782712893383973096) {
            /* overflow if x!=inf */
            x *= 0x1p1023;
            return x;
        }
        if (x < -708.39641853226410622) {
            /* underflow if x!=-inf */
            FORCE_EVAL((float)(-0x1p-149/x));
            if (x < -745.13321910194110842)
                return 0;
        }
    }

    /* argument reduction */
    if (hx > 0x3fd62e42) {  /* if |x| > 0.5 ln2 */
        if (hx >= 0x3ff0a2b2)  /* if |x| >= 1.5 ln2 */
            exp_c__k = (int)(exp_c__invln2*x + exp_c__half[sign]);
        else
            exp_c__k = 1 - sign - sign;
        hi = x - exp_c__k*exp_c__ln2hi;  /* exp_c__k*exp_c__ln2hi is exact here */
        lo = exp_c__k*exp_c__ln2lo;
        x = hi - lo;
    } else if (hx > 0x3e300000)  {  /* if |x| > 2**-28 */
        exp_c__k = 0;
        hi = x;
        lo = 0;
    } else {
        /* inexact if x!=0 */
        FORCE_EVAL(0x1p1023 + x);
        return 1 + x;
    }

    /* x is now in primary range */
    xx = x*x;
    c = x - xx*(exp_c__P1+xx*(exp_c__P2+xx*(exp_c__P3+xx*(exp_c__P4+xx*exp_c__P5))));
    y = 1 + (x*c/(2-c) - lo + hi);
    if (exp_c__k == 0)
        return y;
    return scalbn(y, exp_c__k);
}

/// >>> START src/math/exp10.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

double exp10(double x) {
    static const double p10[] = {
        1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
        1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
        1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15
    };
    double n, y = modf(x, &n);
    if (fabs(n) < 16) {
        if (!y) return p10[(int)n+15];
        y = exp2(3.32192809488736234787031942948939 * y);
        return y * p10[(int)n+15];
    }
    return pow(10.0, x);
}

weak_alias(exp10, pow10);

/// >>> START src/math/exp10f.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

float exp10f(float x) {
    static const float p10[] = {
        1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
        1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7
    };
    float n, y = modff(x, &n);
    if (fabsf(n) < 8) {
        if (!y) return p10[(int)n+7];
        y = exp2f(3.32192809488736234787031942948939f * y);
        return y * p10[(int)n+7];
    }
    return exp2(3.32192809488736234787031942948939 * x);
}

weak_alias(exp10f, pow10f);

/// >>> START src/math/exp10l.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

long double exp10l(long double x) {
    static const long double p10[] = {
        1e-15L, 1e-14L, 1e-13L, 1e-12L, 1e-11L, 1e-10L,
        1e-9L, 1e-8L, 1e-7L, 1e-6L, 1e-5L, 1e-4L, 1e-3L, 1e-2L, 1e-1L,
        1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15
    };
    long double n, y = modfl(x, &n);
    if (fabsl(n) < 16) {
        if (!y) return p10[(int)n+15];
        y = exp2l(3.32192809488736234787031942948939L * y);
        return y * p10[(int)n+15];
    }
    return powl(10.0, x);
}

weak_alias(exp10l, pow10l);

/// >>> START src/math/exp2.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_exp2.c */
/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#undef exp2_c__TBLSIZE
#define exp2_c__TBLSIZE 256

static const double
exp2_c__redux = 0x1.8p52 / exp2_c__TBLSIZE,
exp2_c__P1    = 0x1.62e42fefa39efp-1,
exp2_c__P2    = 0x1.ebfbdff82c575p-3,
exp2_c__P3    = 0x1.c6b08d704a0a6p-5,
exp2_c__P4    = 0x1.3b2ab88f70400p-7,
exp2_c__P5    = 0x1.5d88003875c74p-10;

static const double exp2_c__tbl[exp2_c__TBLSIZE * 2] = {
/*  exp2(z + eps)          eps     */
  0x1.6a09e667f3d5dp-1,  0x1.9880p-44,
  0x1.6b052fa751744p-1,  0x1.8000p-50,
  0x1.6c012750bd9fep-1, -0x1.8780p-45,
  0x1.6cfdcddd476bfp-1,  0x1.ec00p-46,
  0x1.6dfb23c651a29p-1, -0x1.8000p-50,
  0x1.6ef9298593ae3p-1, -0x1.c000p-52,
  0x1.6ff7df9519386p-1, -0x1.fd80p-45,
  0x1.70f7466f42da3p-1, -0x1.c880p-45,
  0x1.71f75e8ec5fc3p-1,  0x1.3c00p-46,
  0x1.72f8286eacf05p-1, -0x1.8300p-44,
  0x1.73f9a48a58152p-1, -0x1.0c00p-47,
  0x1.74fbd35d7ccfcp-1,  0x1.f880p-45,
  0x1.75feb564267f1p-1,  0x1.3e00p-47,
  0x1.77024b1ab6d48p-1, -0x1.7d00p-45,
  0x1.780694fde5d38p-1, -0x1.d000p-50,
  0x1.790b938ac1d00p-1,  0x1.3000p-49,
  0x1.7a11473eb0178p-1, -0x1.d000p-49,
  0x1.7b17b0976d060p-1,  0x1.0400p-45,
  0x1.7c1ed0130c133p-1,  0x1.0000p-53,
  0x1.7d26a62ff8636p-1, -0x1.6900p-45,
  0x1.7e2f336cf4e3bp-1, -0x1.2e00p-47,
  0x1.7f3878491c3e8p-1, -0x1.4580p-45,
  0x1.80427543e1b4ep-1,  0x1.3000p-44,
  0x1.814d2add1071ap-1,  0x1.f000p-47,
  0x1.82589994ccd7ep-1, -0x1.1c00p-45,
  0x1.8364c1eb942d0p-1,  0x1.9d00p-45,
  0x1.8471a4623cab5p-1,  0x1.7100p-43,
  0x1.857f4179f5bbcp-1,  0x1.2600p-45,
  0x1.868d99b4491afp-1, -0x1.2c40p-44,
  0x1.879cad931a395p-1, -0x1.3000p-45,
  0x1.88ac7d98a65b8p-1, -0x1.a800p-45,
  0x1.89bd0a4785800p-1, -0x1.d000p-49,
  0x1.8ace5422aa223p-1,  0x1.3280p-44,
  0x1.8be05bad619fap-1,  0x1.2b40p-43,
  0x1.8cf3216b54383p-1, -0x1.ed00p-45,
  0x1.8e06a5e08664cp-1, -0x1.0500p-45,
  0x1.8f1ae99157807p-1,  0x1.8280p-45,
  0x1.902fed0282c0ep-1, -0x1.cb00p-46,
  0x1.9145b0b91ff96p-1, -0x1.5e00p-47,
  0x1.925c353aa2ff9p-1,  0x1.5400p-48,
  0x1.93737b0cdc64ap-1,  0x1.7200p-46,
  0x1.948b82b5f98aep-1, -0x1.9000p-47,
  0x1.95a44cbc852cbp-1,  0x1.5680p-45,
  0x1.96bdd9a766f21p-1, -0x1.6d00p-44,
  0x1.97d829fde4e2ap-1, -0x1.1000p-47,
  0x1.98f33e47a23a3p-1,  0x1.d000p-45,
  0x1.9a0f170ca0604p-1, -0x1.8a40p-44,
  0x1.9b2bb4d53ff89p-1,  0x1.55c0p-44,
  0x1.9c49182a3f15bp-1,  0x1.6b80p-45,
  0x1.9d674194bb8c5p-1, -0x1.c000p-49,
  0x1.9e86319e3238ep-1,  0x1.7d00p-46,
  0x1.9fa5e8d07f302p-1,  0x1.6400p-46,
  0x1.a0c667b5de54dp-1, -0x1.5000p-48,
  0x1.a1e7aed8eb8f6p-1,  0x1.9e00p-47,
  0x1.a309bec4a2e27p-1,  0x1.ad80p-45,
  0x1.a42c980460a5dp-1, -0x1.af00p-46,
  0x1.a5503b23e259bp-1,  0x1.b600p-47,
  0x1.a674a8af46213p-1,  0x1.8880p-44,
  0x1.a799e1330b3a7p-1,  0x1.1200p-46,
  0x1.a8bfe53c12e8dp-1,  0x1.6c00p-47,
  0x1.a9e6b5579fcd2p-1, -0x1.9b80p-45,
  0x1.ab0e521356fb8p-1,  0x1.b700p-45,
  0x1.ac36bbfd3f381p-1,  0x1.9000p-50,
  0x1.ad5ff3a3c2780p-1,  0x1.4000p-49,
  0x1.ae89f995ad2a3p-1, -0x1.c900p-45,
  0x1.afb4ce622f367p-1,  0x1.6500p-46,
  0x1.b0e07298db790p-1,  0x1.fd40p-45,
  0x1.b20ce6c9a89a9p-1,  0x1.2700p-46,
  0x1.b33a2b84f1a4bp-1,  0x1.d470p-43,
  0x1.b468415b747e7p-1, -0x1.8380p-44,
  0x1.b59728de5593ap-1,  0x1.8000p-54,
  0x1.b6c6e29f1c56ap-1,  0x1.ad00p-47,
  0x1.b7f76f2fb5e50p-1,  0x1.e800p-50,
  0x1.b928cf22749b2p-1, -0x1.4c00p-47,
  0x1.ba5b030a10603p-1, -0x1.d700p-47,
  0x1.bb8e0b79a6f66p-1,  0x1.d900p-47,
  0x1.bcc1e904bc1ffp-1,  0x1.2a00p-47,
  0x1.bdf69c3f3a16fp-1, -0x1.f780p-46,
  0x1.bf2c25bd71db8p-1, -0x1.0a00p-46,
  0x1.c06286141b2e9p-1, -0x1.1400p-46,
  0x1.c199bdd8552e0p-1,  0x1.be00p-47,
  0x1.c2d1cd9fa64eep-1, -0x1.9400p-47,
  0x1.c40ab5fffd02fp-1, -0x1.ed00p-47,
  0x1.c544778fafd15p-1,  0x1.9660p-44,
  0x1.c67f12e57d0cbp-1, -0x1.a100p-46,
  0x1.c7ba88988c1b6p-1, -0x1.8458p-42,
  0x1.c8f6d9406e733p-1, -0x1.a480p-46,
  0x1.ca3405751c4dfp-1,  0x1.b000p-51,
  0x1.cb720dcef9094p-1,  0x1.1400p-47,
  0x1.ccb0f2e6d1689p-1,  0x1.0200p-48,
  0x1.cdf0b555dc412p-1,  0x1.3600p-48,
  0x1.cf3155b5bab3bp-1, -0x1.6900p-47,
  0x1.d072d4a0789bcp-1,  0x1.9a00p-47,
  0x1.d1b532b08c8fap-1, -0x1.5e00p-46,
  0x1.d2f87080d8a85p-1,  0x1.d280p-46,
  0x1.d43c8eacaa203p-1,  0x1.1a00p-47,
  0x1.d5818dcfba491p-1,  0x1.f000p-50,
  0x1.d6c76e862e6a1p-1, -0x1.3a00p-47,
  0x1.d80e316c9834ep-1, -0x1.cd80p-47,
  0x1.d955d71ff6090p-1,  0x1.4c00p-48,
  0x1.da9e603db32aep-1,  0x1.f900p-48,
  0x1.dbe7cd63a8325p-1,  0x1.9800p-49,
  0x1.dd321f301b445p-1, -0x1.5200p-48,
  0x1.de7d5641c05bfp-1, -0x1.d700p-46,
  0x1.dfc97337b9aecp-1, -0x1.6140p-46,
  0x1.e11676b197d5ep-1,  0x1.b480p-47,
  0x1.e264614f5a3e7p-1,  0x1.0ce0p-43,
  0x1.e3b333b16ee5cp-1,  0x1.c680p-47,
  0x1.e502ee78b3fb4p-1, -0x1.9300p-47,
  0x1.e653924676d68p-1, -0x1.5000p-49,
  0x1.e7a51fbc74c44p-1, -0x1.7f80p-47,
  0x1.e8f7977cdb726p-1, -0x1.3700p-48,
  0x1.ea4afa2a490e8p-1,  0x1.5d00p-49,
  0x1.eb9f4867ccae4p-1,  0x1.61a0p-46,
  0x1.ecf482d8e680dp-1,  0x1.5500p-48,
  0x1.ee4aaa2188514p-1,  0x1.6400p-51,
  0x1.efa1bee615a13p-1, -0x1.e800p-49,
  0x1.f0f9c1cb64106p-1, -0x1.a880p-48,
  0x1.f252b376bb963p-1, -0x1.c900p-45,
  0x1.f3ac948dd7275p-1,  0x1.a000p-53,
  0x1.f50765b6e4524p-1, -0x1.4f00p-48,
  0x1.f6632798844fdp-1,  0x1.a800p-51,
  0x1.f7bfdad9cbe38p-1,  0x1.abc0p-48,
  0x1.f91d802243c82p-1, -0x1.4600p-50,
  0x1.fa7c1819e908ep-1, -0x1.b0c0p-47,
  0x1.fbdba3692d511p-1, -0x1.0e00p-51,
  0x1.fd3c22b8f7194p-1, -0x1.0de8p-46,
  0x1.fe9d96b2a23eep-1,  0x1.e430p-49,
  0x1.0000000000000p+0,  0x0.0000p+0,
  0x1.00b1afa5abcbep+0, -0x1.3400p-52,
  0x1.0163da9fb3303p+0, -0x1.2170p-46,
  0x1.02168143b0282p+0,  0x1.a400p-52,
  0x1.02c9a3e77806cp+0,  0x1.f980p-49,
  0x1.037d42e11bbcap+0, -0x1.7400p-51,
  0x1.04315e86e7f89p+0,  0x1.8300p-50,
  0x1.04e5f72f65467p+0, -0x1.a3f0p-46,
  0x1.059b0d315855ap+0, -0x1.2840p-47,
  0x1.0650a0e3c1f95p+0,  0x1.1600p-48,
  0x1.0706b29ddf71ap+0,  0x1.5240p-46,
  0x1.07bd42b72a82dp+0, -0x1.9a00p-49,
  0x1.0874518759bd0p+0,  0x1.6400p-49,
  0x1.092bdf66607c8p+0, -0x1.0780p-47,
  0x1.09e3ecac6f383p+0, -0x1.8000p-54,
  0x1.0a9c79b1f3930p+0,  0x1.fa00p-48,
  0x1.0b5586cf988fcp+0, -0x1.ac80p-48,
  0x1.0c0f145e46c8ap+0,  0x1.9c00p-50,
  0x1.0cc922b724816p+0,  0x1.5200p-47,
  0x1.0d83b23395dd8p+0, -0x1.ad00p-48,
  0x1.0e3ec32d3d1f3p+0,  0x1.bac0p-46,
  0x1.0efa55fdfa9a6p+0, -0x1.4e80p-47,
  0x1.0fb66affed2f0p+0, -0x1.d300p-47,
  0x1.1073028d7234bp+0,  0x1.1500p-48,
  0x1.11301d0125b5bp+0,  0x1.c000p-49,
  0x1.11edbab5e2af9p+0,  0x1.6bc0p-46,
  0x1.12abdc06c31d5p+0,  0x1.8400p-49,
  0x1.136a814f2047dp+0, -0x1.ed00p-47,
  0x1.1429aaea92de9p+0,  0x1.8e00p-49,
  0x1.14e95934f3138p+0,  0x1.b400p-49,
  0x1.15a98c8a58e71p+0,  0x1.5300p-47,
  0x1.166a45471c3dfp+0,  0x1.3380p-47,
  0x1.172b83c7d5211p+0,  0x1.8d40p-45,
  0x1.17ed48695bb9fp+0, -0x1.5d00p-47,
  0x1.18af9388c8d93p+0, -0x1.c880p-46,
  0x1.1972658375d66p+0,  0x1.1f00p-46,
  0x1.1a35beb6fcba7p+0,  0x1.0480p-46,
  0x1.1af99f81387e3p+0, -0x1.7390p-43,
  0x1.1bbe084045d54p+0,  0x1.4e40p-45,
  0x1.1c82f95281c43p+0, -0x1.a200p-47,
  0x1.1d4873168b9b2p+0,  0x1.3800p-49,
  0x1.1e0e75eb44031p+0,  0x1.ac00p-49,
  0x1.1ed5022fcd938p+0,  0x1.1900p-47,
  0x1.1f9c18438cdf7p+0, -0x1.b780p-46,
  0x1.2063b88628d8fp+0,  0x1.d940p-45,
  0x1.212be3578a81ep+0,  0x1.8000p-50,
  0x1.21f49917ddd41p+0,  0x1.b340p-45,
  0x1.22bdda2791323p+0,  0x1.9f80p-46,
  0x1.2387a6e7561e7p+0, -0x1.9c80p-46,
  0x1.2451ffb821427p+0,  0x1.2300p-47,
  0x1.251ce4fb2a602p+0, -0x1.3480p-46,
  0x1.25e85711eceb0p+0,  0x1.2700p-46,
  0x1.26b4565e27d16p+0,  0x1.1d00p-46,
  0x1.2780e341de00fp+0,  0x1.1ee0p-44,
  0x1.284dfe1f5633ep+0, -0x1.4c00p-46,
  0x1.291ba7591bb30p+0, -0x1.3d80p-46,
  0x1.29e9df51fdf09p+0,  0x1.8b00p-47,
  0x1.2ab8a66d10e9bp+0, -0x1.27c0p-45,
  0x1.2b87fd0dada3ap+0,  0x1.a340p-45,
  0x1.2c57e39771af9p+0, -0x1.0800p-46,
  0x1.2d285a6e402d9p+0, -0x1.ed00p-47,
  0x1.2df961f641579p+0, -0x1.4200p-48,
  0x1.2ecafa93e2ecfp+0, -0x1.4980p-45,
  0x1.2f9d24abd8822p+0, -0x1.6300p-46,
  0x1.306fe0a31b625p+0, -0x1.2360p-44,
  0x1.31432edeea50bp+0, -0x1.0df8p-40,
  0x1.32170fc4cd7b8p+0, -0x1.2480p-45,
  0x1.32eb83ba8e9a2p+0, -0x1.5980p-45,
  0x1.33c08b2641766p+0,  0x1.ed00p-46,
  0x1.3496266e3fa27p+0, -0x1.c000p-50,
  0x1.356c55f929f0fp+0, -0x1.0d80p-44,
  0x1.36431a2de88b9p+0,  0x1.2c80p-45,
  0x1.371a7373aaa39p+0,  0x1.0600p-45,
  0x1.37f26231e74fep+0, -0x1.6600p-46,
  0x1.38cae6d05d838p+0, -0x1.ae00p-47,
  0x1.39a401b713ec3p+0, -0x1.4720p-43,
  0x1.3a7db34e5a020p+0,  0x1.8200p-47,
  0x1.3b57fbfec6e95p+0,  0x1.e800p-44,
  0x1.3c32dc313a8f2p+0,  0x1.f800p-49,
  0x1.3d0e544ede122p+0, -0x1.7a00p-46,
  0x1.3dea64c1234bbp+0,  0x1.6300p-45,
  0x1.3ec70df1c4eccp+0, -0x1.8a60p-43,
  0x1.3fa4504ac7e8cp+0, -0x1.cdc0p-44,
  0x1.40822c367a0bbp+0,  0x1.5b80p-45,
  0x1.4160a21f72e95p+0,  0x1.ec00p-46,
  0x1.423fb27094646p+0, -0x1.3600p-46,
  0x1.431f5d950a920p+0,  0x1.3980p-45,
  0x1.43ffa3f84b9ebp+0,  0x1.a000p-48,
  0x1.44e0860618919p+0, -0x1.6c00p-48,
  0x1.45c2042a7d201p+0, -0x1.bc00p-47,
  0x1.46a41ed1d0016p+0, -0x1.2800p-46,
  0x1.4786d668b3326p+0,  0x1.0e00p-44,
  0x1.486a2b5c13c00p+0, -0x1.d400p-45,
  0x1.494e1e192af04p+0,  0x1.c200p-47,
  0x1.4a32af0d7d372p+0, -0x1.e500p-46,
  0x1.4b17dea6db801p+0,  0x1.7800p-47,
  0x1.4bfdad53629e1p+0, -0x1.3800p-46,
  0x1.4ce41b817c132p+0,  0x1.0800p-47,
  0x1.4dcb299fddddbp+0,  0x1.c700p-45,
  0x1.4eb2d81d8ab96p+0, -0x1.ce00p-46,
  0x1.4f9b2769d2d02p+0,  0x1.9200p-46,
  0x1.508417f4531c1p+0, -0x1.8c00p-47,
  0x1.516daa2cf662ap+0, -0x1.a000p-48,
  0x1.5257de83f51eap+0,  0x1.a080p-43,
  0x1.5342b569d4edap+0, -0x1.6d80p-45,
  0x1.542e2f4f6ac1ap+0, -0x1.2440p-44,
  0x1.551a4ca5d94dbp+0,  0x1.83c0p-43,
  0x1.56070dde9116bp+0,  0x1.4b00p-45,
  0x1.56f4736b529dep+0,  0x1.15a0p-43,
  0x1.57e27dbe2c40ep+0, -0x1.9e00p-45,
  0x1.58d12d497c76fp+0, -0x1.3080p-45,
  0x1.59c0827ff0b4cp+0,  0x1.dec0p-43,
  0x1.5ab07dd485427p+0, -0x1.4000p-51,
  0x1.5ba11fba87af4p+0,  0x1.0080p-44,
  0x1.5c9268a59460bp+0, -0x1.6c80p-45,
  0x1.5d84590998e3fp+0,  0x1.69a0p-43,
  0x1.5e76f15ad20e1p+0, -0x1.b400p-46,
  0x1.5f6a320dcebcap+0,  0x1.7700p-46,
  0x1.605e1b976dcb8p+0,  0x1.6f80p-45,
  0x1.6152ae6cdf715p+0,  0x1.1000p-47,
  0x1.6247eb03a5531p+0, -0x1.5d00p-46,
  0x1.633dd1d1929b5p+0, -0x1.2d00p-46,
  0x1.6434634ccc313p+0, -0x1.a800p-49,
  0x1.652b9febc8efap+0, -0x1.8600p-45,
  0x1.6623882553397p+0,  0x1.1fe0p-40,
  0x1.671c1c708328ep+0, -0x1.7200p-44,
  0x1.68155d44ca97ep+0,  0x1.6800p-49,
  0x1.690f4b19e9471p+0, -0x1.9780p-45,
};

/*
 * exp2(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.503 ulp for normalized results.
 *
 * Method: (accurate tables)
 *
 *   Reduce x:
 *     x = exp2_c__k + y, for integer exp2_c__k and |y| <= 1/2.
 *     Thus we have exp2(x) = 2**exp2_c__k * exp2(y).
 *
 *   Reduce y:
 *     y = i/exp2_c__TBLSIZE + z - eps[i] for integer i near y * exp2_c__TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/exp2_c__TBLSIZE) * exp2(z - eps[i]),
 *     with |z - eps[i]| <= 2**-9 + 2**-39 for the exp2_c__table used.
 *
 *   We compute exp2(i/exp2_c__TBLSIZE) via exp2_c__table lookup and exp2(z - eps[i]) via
 *   a degree-5 minimax polynomial with maximum error under 1.3 * 2**-61.
 *   The values in exp2t[] and eps[] are chosen such that
 *   exp2t[i] = exp2(i/exp2_c__TBLSIZE + eps[i]), and eps[i] is a small offset such
 *   that exp2t[i] is accurate to 2**-64.
 *
 *   Note that the range of i is +-exp2_c__TBLSIZE/2, so we actually index the tables
 *   by i0 = i + exp2_c__TBLSIZE/2.  For cache efficiency, exp2t[] and eps[] are
 *   virtual tables, interleaved in the real exp2_c__table exp2_c__tbl[].
 *
 *   This method is due to Gal, with many details due to Gal and Bachelis:
 *
 *      Gal, exp2_c__S. and Bachelis, B.  An Accurate Elementary Mathematical Library
 *      for the IEEE Floating Point Standard.  TOMS 17(1), 26-46 (1991).
 */
double exp2(double x) {
    double_t r, t, z;
    uint32_t ix, i0;
    union {double f; uint64_t i;} u = {x};
    union {uint32_t u; int32_t i;} exp2_c__k;

    /* Filter out exceptional cases. */
    ix = u.i>>32 & 0x7fffffff;
    if (ix >= 0x408ff000) {  /* |x| >= 1022 or nan */
        if (ix >= 0x40900000 && u.i>>63 == 0) {  /* x >= 1024 or nan */
            /* overflow */
            x *= 0x1p1023;
            return x;
        }
        if (ix >= 0x7ff00000)  /* -inf or -nan */
            return -1/x;
        if (u.i>>63) {  /* x <= -1022 */
            /* underflow */
            if (x <= -1075 || x - 0x1p52 + 0x1p52 != x)
                FORCE_EVAL((float)(-0x1p-149/x));
            if (x <= -1075)
                return 0;
        }
    } else if (ix < 0x3c900000) {  /* |x| < 0x1p-54 */
        return 1.0 + x;
    }

    /* Reduce x, computing z, i0, and exp2_c__k. */
    u.f = x + exp2_c__redux;
    i0 = u.i;
    i0 += exp2_c__TBLSIZE / 2;
    exp2_c__k.u = i0 / exp2_c__TBLSIZE * exp2_c__TBLSIZE;
    exp2_c__k.i /= exp2_c__TBLSIZE;
    i0 %= exp2_c__TBLSIZE;
    u.f -= exp2_c__redux;
    z = x - u.f;

    /* Compute r = exp2(y) = exp2t[i0] * p(z - eps[i]). */
    t = exp2_c__tbl[2*i0];       /* exp2t[i0] */
    z -= exp2_c__tbl[2*i0 + 1];  /* eps[i0]   */
    r = t + t * z * (exp2_c__P1 + z * (exp2_c__P2 + z * (exp2_c__P3 + z * (exp2_c__P4 + z * exp2_c__P5))));

    return scalbn(r, exp2_c__k.i);
}

/// >>> START src/math/exp2f.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_exp2f.c */
/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#undef exp2f_c__TBLSIZE
#define exp2f_c__TBLSIZE 16

static const float
exp2f_c__redux = 0x1.8p23f / exp2f_c__TBLSIZE,
exp2f_c__P1    = 0x1.62e430p-1f,
exp2f_c__P2    = 0x1.ebfbe0p-3f,
exp2f_c__P3    = 0x1.c6b348p-5f,
exp2f_c__P4    = 0x1.3b2c9cp-7f;

static const double exp2ft[exp2f_c__TBLSIZE] = {
  0x1.6a09e667f3bcdp-1,
  0x1.7a11473eb0187p-1,
  0x1.8ace5422aa0dbp-1,
  0x1.9c49182a3f090p-1,
  0x1.ae89f995ad3adp-1,
  0x1.c199bdd85529cp-1,
  0x1.d5818dcfba487p-1,
  0x1.ea4afa2a490dap-1,
  0x1.0000000000000p+0,
  0x1.0b5586cf9890fp+0,
  0x1.172b83c7d517bp+0,
  0x1.2387a6e756238p+0,
  0x1.306fe0a31b715p+0,
  0x1.3dea64c123422p+0,
  0x1.4bfdad5362a27p+0,
  0x1.5ab07dd485429p+0,
};

/*
 * exp2f(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.501 ulp; location of peak: -0.030110927.
 *
 * Method: (equally-spaced tables)
 *
 *   Reduce x:
 *     x = exp2f_c__k + y, for integer exp2f_c__k and |y| <= 1/2.
 *     Thus we have exp2f(x) = 2**exp2f_c__k * exp2(y).
 *
 *   Reduce y:
 *     y = i/exp2f_c__TBLSIZE + z for integer i near y * exp2f_c__TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/exp2f_c__TBLSIZE) * exp2(z),
 *     with |z| <= 2**-(exp2f_c__TBLSIZE+1).
 *
 *   We compute exp2(i/exp2f_c__TBLSIZE) via exp2f_c__table lookup and exp2(z) via a
 *   degree-4 minimax polynomial with maximum error under 1.4 * 2**-33.
 *   Using double precision for everything except the reduction makes
 *   roundoff error insignificant and simplifies the scaling step.
 *
 *   This method is due to Tang, but I do not use his suggested parameters:
 *
 *      Tang, exp2f_c__P.  Table-driven Implementation of the Exponential Function
 *      in IEEE Floating-Point Arithmetic.  TOMS 15(2), 144-157 (1989).
 */
float exp2f(float x) {
    double_t t, r, z;
    union {float f; uint32_t i;} u = {x};
    union {double f; uint64_t i;} uk;
    uint32_t ix, i0, exp2f_c__k;

    /* Filter out exceptional cases. */
    ix = u.i & 0x7fffffff;
    if (ix > 0x42fc0000) {  /* |x| > 126 */
        if (u.i >= 0x43000000 && u.i < 0x80000000) {  /* x >= 128 */
            x *= 0x1p127f;
            return x;
        }
        if (u.i >= 0x80000000) {  /* x < -126 */
            if (u.i >= 0xc3160000 || (u.i & 0x0000ffff))
                FORCE_EVAL(-0x1p-149f/x);
            if (u.i >= 0xc3160000)  /* x <= -150 */
                return 0;
        }
    } else if (ix <= 0x33000000) {  /* |x| <= 0x1p-25 */
        return 1.0f + x;
    }

    /* Reduce x, computing z, i0, and exp2f_c__k. */
    u.f = x + exp2f_c__redux;
    i0 = u.i;
    i0 += exp2f_c__TBLSIZE / 2;
    exp2f_c__k = i0 / exp2f_c__TBLSIZE;
    uk.i = (uint64_t)(0x3ff + exp2f_c__k)<<52;
    i0 &= exp2f_c__TBLSIZE - 1;
    u.f -= exp2f_c__redux;
    z = x - u.f;
    /* Compute r = exp2(y) = exp2ft[i0] * p(z). */
    r = exp2ft[i0];
    t = r * z;
    r = r + t * (exp2f_c__P1 + z * exp2f_c__P2) + t * (z * z) * (exp2f_c__P3 + z * exp2f_c__P4);

    /* Scale by 2**exp2f_c__k */
    return r * uk.f;
}

/// >>> START src/math/exp2l.c
/* origin: FreeBSD /usr/src/lib/msun/ld80/s_exp2l.c */
/*-
 * Copyright (c) 2005-2008 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double exp2l(long double x) {
    return exp2(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
#undef TBLBITS
#define TBLBITS 7
#undef exp2l_c__TBLSIZE
#define exp2l_c__TBLSIZE (1 << TBLBITS)

static const double
exp2l_c__redux = 0x1.8p63 / exp2l_c__TBLSIZE,
exp2l_c__P1    = 0x1.62e42fefa39efp-1,
exp2l_c__P2    = 0x1.ebfbdff82c58fp-3,
exp2l_c__P3    = 0x1.c6b08d7049fap-5,
exp2l_c__P4    = 0x1.3b2ab6fba4da5p-7,
exp2l_c__P5    = 0x1.5d8804780a736p-10,
exp2l_c__P6    = 0x1.430918835e33dp-13;

static const double exp2l_c__tbl[exp2l_c__TBLSIZE * 2] = {
    0x1.6a09e667f3bcdp-1,   -0x1.bdd3413b2648p-55,
    0x1.6c012750bdabfp-1,   -0x1.2895667ff0cp-57,
    0x1.6dfb23c651a2fp-1,   -0x1.bbe3a683c88p-58,
    0x1.6ff7df9519484p-1,   -0x1.83c0f25860fp-56,
    0x1.71f75e8ec5f74p-1,   -0x1.16e4786887bp-56,
    0x1.73f9a48a58174p-1,   -0x1.0a8d96c65d5p-55,
    0x1.75feb564267c9p-1,   -0x1.0245957316ep-55,
    0x1.780694fde5d3fp-1,    0x1.866b80a0216p-55,
    0x1.7a11473eb0187p-1,   -0x1.41577ee0499p-56,
    0x1.7c1ed0130c132p-1,    0x1.f124cd1164ep-55,
    0x1.7e2f336cf4e62p-1,    0x1.05d02ba157ap-57,
    0x1.80427543e1a12p-1,   -0x1.27c86626d97p-55,
    0x1.82589994cce13p-1,   -0x1.d4c1dd41533p-55,
    0x1.8471a4623c7adp-1,   -0x1.8d684a341cep-56,
    0x1.868d99b4492edp-1,   -0x1.fc6f89bd4f68p-55,
    0x1.88ac7d98a6699p-1,    0x1.994c2f37cb5p-55,
    0x1.8ace5422aa0dbp-1,    0x1.6e9f156864bp-55,
    0x1.8cf3216b5448cp-1,   -0x1.0d55e32e9e4p-57,
    0x1.8f1ae99157736p-1,    0x1.5cc13a2e397p-56,
    0x1.9145b0b91ffc6p-1,   -0x1.dd6792e5825p-55,
    0x1.93737b0cdc5e5p-1,   -0x1.75fc781b58p-58,
    0x1.95a44cbc8520fp-1,   -0x1.64b7c96a5fp-57,
    0x1.97d829fde4e5p-1,    -0x1.d185b7c1b86p-55,
    0x1.9a0f170ca07bap-1,   -0x1.173bd91cee6p-55,
    0x1.9c49182a3f09p-1,     0x1.c7c46b071f2p-57,
    0x1.9e86319e32323p-1,    0x1.824ca78e64cp-57,
    0x1.a0c667b5de565p-1,   -0x1.359495d1cd5p-55,
    0x1.a309bec4a2d33p-1,    0x1.6305c7ddc368p-55,
    0x1.a5503b23e255dp-1,   -0x1.d2f6edb8d42p-55,
    0x1.a799e1330b358p-1,    0x1.bcb7ecac564p-55,
    0x1.a9e6b5579fdbfp-1,    0x1.0fac90ef7fdp-55,
    0x1.ac36bbfd3f37ap-1,   -0x1.f9234cae76dp-56,
    0x1.ae89f995ad3adp-1,    0x1.7a1cd345dcc8p-55,
    0x1.b0e07298db666p-1,   -0x1.bdef54c80e4p-55,
    0x1.b33a2b84f15fbp-1,   -0x1.2805e3084d8p-58,
    0x1.b59728de5593ap-1,   -0x1.c71dfbbba6ep-55,
    0x1.b7f76f2fb5e47p-1,   -0x1.5584f7e54acp-57,
    0x1.ba5b030a1064ap-1,   -0x1.efcd30e5429p-55,
    0x1.bcc1e904bc1d2p-1,    0x1.23dd07a2d9fp-56,
    0x1.bf2c25bd71e09p-1,   -0x1.efdca3f6b9c8p-55,
    0x1.c199bdd85529cp-1,    0x1.11065895049p-56,
    0x1.c40ab5fffd07ap-1,    0x1.b4537e083c6p-55,
    0x1.c67f12e57d14bp-1,    0x1.2884dff483c8p-55,
    0x1.c8f6d9406e7b5p-1,    0x1.1acbc48805cp-57,
    0x1.cb720dcef9069p-1,    0x1.503cbd1e94ap-57,
    0x1.cdf0b555dc3fap-1,   -0x1.dd83b53829dp-56,
    0x1.d072d4a07897cp-1,   -0x1.cbc3743797a8p-55,
    0x1.d2f87080d89f2p-1,   -0x1.d487b719d858p-55,
    0x1.d5818dcfba487p-1,    0x1.2ed02d75b37p-56,
    0x1.d80e316c98398p-1,   -0x1.11ec18bedep-55,
    0x1.da9e603db3285p-1,    0x1.c2300696db5p-55,
    0x1.dd321f301b46p-1,     0x1.2da5778f019p-55,
    0x1.dfc97337b9b5fp-1,   -0x1.1a5cd4f184b8p-55,
    0x1.e264614f5a129p-1,   -0x1.7b627817a148p-55,
    0x1.e502ee78b3ff6p-1,    0x1.39e8980a9cdp-56,
    0x1.e7a51fbc74c83p-1,    0x1.2d522ca0c8ep-55,
    0x1.ea4afa2a490dap-1,   -0x1.e9c23179c288p-55,
    0x1.ecf482d8e67f1p-1,   -0x1.c93f3b411ad8p-55,
    0x1.efa1bee615a27p-1,    0x1.dc7f486a4b68p-55,
    0x1.f252b376bba97p-1,    0x1.3a1a5bf0d8e8p-55,
    0x1.f50765b6e454p-1,     0x1.9d3e12dd8a18p-55,
    0x1.f7bfdad9cbe14p-1,   -0x1.dbb12d00635p-55,
    0x1.fa7c1819e90d8p-1,    0x1.74853f3a593p-56,
    0x1.fd3c22b8f71f1p-1,    0x1.2eb74966578p-58,
    0x1p+0,                  0x0p+0,
    0x1.0163da9fb3335p+0,    0x1.b61299ab8cd8p-54,
    0x1.02c9a3e778061p+0,   -0x1.19083535b08p-56,
    0x1.04315e86e7f85p+0,   -0x1.0a31c1977c98p-54,
    0x1.059b0d3158574p+0,    0x1.d73e2a475b4p-55,
    0x1.0706b29ddf6dep+0,   -0x1.c91dfe2b13cp-55,
    0x1.0874518759bc8p+0,    0x1.186be4bb284p-57,
    0x1.09e3ecac6f383p+0,    0x1.14878183161p-54,
    0x1.0b5586cf9890fp+0,    0x1.8a62e4adc61p-54,
    0x1.0cc922b7247f7p+0,    0x1.01edc16e24f8p-54,
    0x1.0e3ec32d3d1a2p+0,    0x1.03a1727c58p-59,
    0x1.0fb66affed31bp+0,   -0x1.b9bedc44ebcp-57,
    0x1.11301d0125b51p+0,   -0x1.6c51039449bp-54,
    0x1.12abdc06c31ccp+0,   -0x1.1b514b36ca8p-58,
    0x1.1429aaea92dep+0,    -0x1.32fbf9af1368p-54,
    0x1.15a98c8a58e51p+0,    0x1.2406ab9eeabp-55,
    0x1.172b83c7d517bp+0,   -0x1.19041b9d78ap-55,
    0x1.18af9388c8deap+0,   -0x1.11023d1970f8p-54,
    0x1.1a35beb6fcb75p+0,    0x1.e5b4c7b4969p-55,
    0x1.1bbe084045cd4p+0,   -0x1.95386352ef6p-54,
    0x1.1d4873168b9aap+0,    0x1.e016e00a264p-54,
    0x1.1ed5022fcd91dp+0,   -0x1.1df98027bb78p-54,
    0x1.2063b88628cd6p+0,    0x1.dc775814a85p-55,
    0x1.21f49917ddc96p+0,    0x1.2a97e9494a6p-55,
    0x1.2387a6e756238p+0,    0x1.9b07eb6c7058p-54,
    0x1.251ce4fb2a63fp+0,    0x1.ac155bef4f5p-55,
    0x1.26b4565e27cddp+0,    0x1.2bd339940eap-55,
    0x1.284dfe1f56381p+0,   -0x1.a4c3a8c3f0d8p-54,
    0x1.29e9df51fdee1p+0,    0x1.612e8afad12p-55,
    0x1.2b87fd0dad99p+0,    -0x1.10adcd6382p-59,
    0x1.2d285a6e4030bp+0,    0x1.0024754db42p-54,
    0x1.2ecafa93e2f56p+0,    0x1.1ca0f45d524p-56,
    0x1.306fe0a31b715p+0,    0x1.6f46ad23183p-55,
    0x1.32170fc4cd831p+0,    0x1.a9ce78e1804p-55,
    0x1.33c08b26416ffp+0,    0x1.327218436598p-54,
    0x1.356c55f929ff1p+0,   -0x1.b5cee5c4e46p-55,
    0x1.371a7373aa9cbp+0,   -0x1.63aeabf42ebp-54,
    0x1.38cae6d05d866p+0,   -0x1.e958d3c99048p-54,
    0x1.3a7db34e59ff7p+0,   -0x1.5e436d661f6p-56,
    0x1.3c32dc313a8e5p+0,   -0x1.efff8375d2ap-54,
    0x1.3dea64c123422p+0,    0x1.ada0911f09fp-55,
    0x1.3fa4504ac801cp+0,   -0x1.7d023f956fap-54,
    0x1.4160a21f72e2ap+0,   -0x1.ef3691c309p-58,
    0x1.431f5d950a897p+0,   -0x1.1c7dde35f7ap-55,
    0x1.44e086061892dp+0,    0x1.89b7a04ef8p-59,
    0x1.46a41ed1d0057p+0,    0x1.c944bd1648a8p-54,
    0x1.486a2b5c13cdp+0,     0x1.3c1a3b69062p-56,
    0x1.4a32af0d7d3dep+0,    0x1.9cb62f3d1be8p-54,
    0x1.4bfdad5362a27p+0,    0x1.d4397afec42p-56,
    0x1.4dcb299fddd0dp+0,    0x1.8ecdbbc6a78p-54,
    0x1.4f9b2769d2ca7p+0,   -0x1.4b309d25958p-54,
    0x1.516daa2cf6642p+0,   -0x1.f768569bd94p-55,
    0x1.5342b569d4f82p+0,   -0x1.07abe1db13dp-55,
    0x1.551a4ca5d920fp+0,   -0x1.d689cefede6p-55,
    0x1.56f4736b527dap+0,    0x1.9bb2c011d938p-54,
    0x1.58d12d497c7fdp+0,    0x1.295e15b9a1ep-55,
    0x1.5ab07dd485429p+0,    0x1.6324c0546478p-54,
    0x1.5c9268a5946b7p+0,    0x1.c4b1b81698p-60,
    0x1.5e76f15ad2148p+0,    0x1.ba6f93080e68p-54,
    0x1.605e1b976dc09p+0,   -0x1.3e2429b56de8p-54,
    0x1.6247eb03a5585p+0,   -0x1.383c17e40b48p-54,
    0x1.6434634ccc32p+0,    -0x1.c483c759d89p-55,
    0x1.6623882552225p+0,   -0x1.bb60987591cp-54,
    0x1.68155d44ca973p+0,    0x1.038ae44f74p-57,
};

/*
 * exp2l(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.511 ulp.
 *
 * Method: (equally-spaced tables)
 *
 *   Reduce x:
 *     x = 2**exp2l_c__k + y, for integer exp2l_c__k and |y| <= 1/2.
 *     Thus we have exp2l(x) = 2**exp2l_c__k * exp2(y).
 *
 *   Reduce y:
 *     y = i/exp2l_c__TBLSIZE + z for integer i near y * exp2l_c__TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/exp2l_c__TBLSIZE) * exp2(z),
 *     with |z| <= 2**-(TBLBITS+1).
 *
 *   We compute exp2(i/exp2l_c__TBLSIZE) via exp2l_c__table lookup and exp2(z) via a
 *   degree-6 minimax polynomial with maximum error under 2**-69.
 *   The exp2l_c__table entries each have 104 bits of accuracy, encoded as
 *   a pair of double precision values.
 */
long double exp2l(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    long double r, z;
    uint32_t i0;
    union {uint32_t u; int32_t i;} exp2l_c__k;

    /* Filter out exceptional cases. */
    if (e >= 0x3fff + 13) {  /* |x| >= 8192 or x is NaN */
        if (u.i.se >= 0x3fff + 14 && u.i.se >> 15 == 0)
            /* overflow */
            return x * 0x1p16383L;
        if (e == 0x7fff)  /* -inf or -nan */
            return -1/x;
        if (x < -16382) {
            if (x <= -16446 || x - 0x1p63 + 0x1p63 != x)
                /* underflow */
                FORCE_EVAL((float)(-0x1p-149/x));
            if (x <= -16446)
                return 0;
        }
    } else if (e < 0x3fff - 64) {
        return 1 + x;
    }

    /*
     * Reduce x, computing z, i0, and exp2l_c__k. The low bits of x + exp2l_c__redux
     * contain the 16-bit integer part of the exponent (exp2l_c__k) followed by
     * TBLBITS fractional bits (i0). We use bit tricks to extract these
     * as integers, then set z to the remainder.
     *
     * Example: Suppose x is 0xabc.123456p0 and TBLBITS is 8.
     * Then the low-order word of x + exp2l_c__redux is 0x000abc12,
     * We split this into exp2l_c__k = 0xabc and i0 = 0x12 (adjusted to
     * index into the exp2l_c__table), then we compute z = 0x0.003456p0.
     */
    u.f = x + exp2l_c__redux;
    i0 = u.i.m + exp2l_c__TBLSIZE / 2;
    exp2l_c__k.u = i0 / exp2l_c__TBLSIZE * exp2l_c__TBLSIZE;
    exp2l_c__k.i /= exp2l_c__TBLSIZE;
    i0 %= exp2l_c__TBLSIZE;
    u.f -= exp2l_c__redux;
    z = x - u.f;

    /* Compute r = exp2l(y) = exp2lt[i0] * p(z). */
    long double t_hi = exp2l_c__tbl[2*i0];
    long double t_lo = exp2l_c__tbl[2*i0 + 1];
    /* XXX This gives > 1 ulp errors outside of FE_TONEAREST mode */
    r = t_lo + (t_hi + t_lo) * z * (exp2l_c__P1 + z * (exp2l_c__P2 + z * (exp2l_c__P3 + z * (exp2l_c__P4
         + z * (exp2l_c__P5 + z * exp2l_c__P6))))) + t_hi;

    return scalbnl(r, exp2l_c__k.i);
}
#endif

/// >>> START src/math/expf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_expf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
expf_c__half[2] = {0.5,-0.5},
expf_c__ln2hi   = 6.9314575195e-1f,  /* 0x3f317200 */
expf_c__ln2lo   = 1.4286067653e-6f,  /* 0x35bfbe8e */
expf_c__invln2  = 1.4426950216e+0f,  /* 0x3fb8aa3b */
/*
 * Domain [-0.34568, 0.34568], range ~[-4.278e-9, 4.447e-9]:
 * |x*(exp(x)+1)/(exp(x)-1) - p(x)| < 2**-27.74
 */
expf_c__P1 =  1.6666625440e-1f, /*  0xaaaa8f.0p-26 */
expf_c__P2 = -2.7667332906e-3f; /* -0xb55215.0p-32 */

float expf(float x) {
    float_t hi, lo, c, xx, y;
    int expf_c__k, sign;
    uint32_t hx;

    GET_FLOAT_WORD(hx, x);
    sign = hx >> 31;   /* sign bit of x */
    hx &= 0x7fffffff;  /* high word of |x| */

    /* special cases */
    if (hx >= 0x42aeac50) {  /* if |x| >= -87.33655f or NaN */
        if (hx >= 0x42b17218 && !sign) {  /* x >= 88.722839f */
            /* overflow */
            x *= 0x1p127f;
            return x;
        }
        if (sign) {
            /* underflow */
            FORCE_EVAL(-0x1p-149f/x);
            if (hx >= 0x42cff1b5)  /* x <= -103.972084f */
                return 0;
        }
    }

    /* argument reduction */
    if (hx > 0x3eb17218) {  /* if |x| > 0.5 ln2 */
        if (hx > 0x3f851592)  /* if |x| > 1.5 ln2 */
            expf_c__k = expf_c__invln2*x + expf_c__half[sign];
        else
            expf_c__k = 1 - sign - sign;
        hi = x - expf_c__k*expf_c__ln2hi;  /* expf_c__k*expf_c__ln2hi is exact here */
        lo = expf_c__k*expf_c__ln2lo;
        x = hi - lo;
    } else if (hx > 0x39000000) {  /* |x| > 2**-14 */
        expf_c__k = 0;
        hi = x;
        lo = 0;
    } else {
        /* raise inexact */
        FORCE_EVAL(0x1p127f + x);
        return 1 + x;
    }

    /* x is now in primary range */
    xx = x*x;
    c = x - xx*(expf_c__P1+xx*expf_c__P2);
    y = 1 + (x*c/(2-c) - lo + hi);
    if (expf_c__k == 0)
        return y;
    return scalbnf(y, expf_c__k);
}

/// >>> START src/math/expl.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_expl.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Exponential function, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, expl();
 *
 * y = expl( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns e (2.71828...) raised to the x power.
 *
 * Range reduction is accomplished by separating the argument
 * into an integer expl_c__k and fraction f such that
 *
 *     x    expl_c__k  f
 *    e  = 2  e.
 *
 * A Pade' form of degree 5/6 is used to approximate exp(f) - 1
 * in the basic range [-0.5 ln 2, 0.5 ln 2].
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      +-10000     50000       1.12e-19    2.81e-20
 *
 *
 * Error amplification in the exponential function can be
 * a serious matter.  The error propagation involves
 * exp( X(1+delta) ) = exp(X) ( 1 + X*delta + ... ),
 * which shows that a 1 lsb error in representing X produces
 * a relative error of X times 1 lsb in the function.
 * While the routine gives an accurate result for arguments
 * that are exactly represented by a long double precision
 * computer number, the result contains amplified roundoff
 * error for large arguments not exactly represented.
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * exp underflow    x < MINLOG         0.0
 * exp overflow     x > MAXLOG         MAXNUM
 *
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double expl(long double x) {
    return exp(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384

static const long double expl_c__P[3] = {
 1.2617719307481059087798E-4L,
 3.0299440770744196129956E-2L,
 9.9999999999999999991025E-1L,
};
static const long double expl_c__Q[4] = {
 3.0019850513866445504159E-6L,
 2.5244834034968410419224E-3L,
 2.2726554820815502876593E-1L,
 2.0000000000000000000897E0L,
};
static const long double
LN2HI = 6.9314575195312500000000E-1L,
LN2LO = 1.4286068203094172321215E-6L,
LOG2E = 1.4426950408889634073599E0L;

long double expl(long double x) {
    long double px, xx;
    int expl_c__k;

    if (isnan(x))
        return x;
    if (x > 11356.5234062941439488L) /* x > ln(2^16384 - 0.5) */
        return x * 0x1p16383L;
    if (x < -11399.4985314888605581L) /* x < ln(2^-16446) */
        return -0x1p-16445L/x;

    /* Express e**x = e**f 2**expl_c__k
     *   = e**(f + expl_c__k ln(2))
     */
    px = floorl(LOG2E * x + 0.5);
    expl_c__k = px;
    x -= px * LN2HI;
    x -= px * LN2LO;

    /* rational approximation of the fractional part:
     * e**x =  1 + 2x expl_c__P(x**2)/(expl_c__Q(x**2) - x expl_c__P(x**2))
     */
    xx = x * x;
    px = x * __polevll(xx, expl_c__P, 2);
    x = px/(__polevll(xx, expl_c__Q, 3) - px);
    x = 1.0 + 2.0 * x;
    return scalbnl(x, expl_c__k);
}
#endif

/// >>> START src/math/expm1.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_expm1.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* expm1(x)
 * Returns exp(x)-1, the exponential of x minus 1.
 *
 * Method
 *   1. Argument reduction:
 *      Given x, find r and integer expm1_c__k such that
 *
 *               x = expm1_c__k*ln2 + r,  |r| <= 0.5*ln2 ~ 0.34658
 *
 *      Here a correction term c will be computed to compensate
 *      the error in r when rounded to a floating-point number.
 *
 *   2. Approximating expm1(r) by a special rational function on
 *      the interval [0,0.34658]:
 *      Since
 *          r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 - r^4/360 + ...
 *      we define R1(r*r) by
 *          r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 * R1(r*r)
 *      That is,
 *          R1(r**2) = 6/r *((exp(r)+1)/(exp(r)-1) - 2/r)
 *                   = 6/r * ( 1 + 2.0*(1/(exp(r)-1) - 1/r))
 *                   = 1 - r^2/60 + r^4/2520 - r^6/100800 + ...
 *      We use a special Remez algorithm on [0,0.347] to generate
 *      a polynomial of degree 5 in r*r to approximate R1. The
 *      maximum error of this polynomial approximation is bounded
 *      by 2**-61. In other words,
 *          R1(z) ~ 1.0 + expm1_c__Q1*z + expm1_c__Q2*z**2 + expm1_c__Q3*z**3 + expm1_c__Q4*z**4 + expm1_c__Q5*z**5
 *      where   expm1_c__Q1  =  -1.6666666666666567384E-2,
 *              expm1_c__Q2  =   3.9682539681370365873E-4,
 *              expm1_c__Q3  =  -9.9206344733435987357E-6,
 *              expm1_c__Q4  =   2.5051361420808517002E-7,
 *              expm1_c__Q5  =  -6.2843505682382617102E-9;
 *              z   =  r*r,
 *      with error bounded by
 *          |                  5           |     -61
 *          | 1.0+expm1_c__Q1*z+...+expm1_c__Q5*z   -  R1(z) | <= 2
 *          |                              |
 *
 *      expm1(r) = exp(r)-1 is then computed by the following
 *      specific way which minimize the accumulation rounding error:
 *                             2     3
 *                            r     r    [ 3 - (R1 + R1*r/2)  ]
 *            expm1(r) = r + --- + --- * [--------------------]
 *                            2     2    [ 6 - r*(3 - R1*r/2) ]
 *
 *      To compensate the error in the argument reduction, we use
 *              expm1(r+c) = expm1(r) + c + expm1(r)*c
 *                         ~ expm1(r) + c + r*c
 *      Thus c+r*c will be added in as the correction terms for
 *      expm1(r+c). Now rearrange the term to avoid optimization
 *      screw up:
 *                      (      2                                    2 )
 *                      ({  ( r    [ R1 -  (3 - R1*r/2) ]  )  }    r  )
 *       expm1(r+c)~r - ({r*(--- * [--------------------]-c)-c} - --- )
 *                      ({  ( 2    [ 6 - r*(3 - R1*r/2) ]  )  }    2  )
 *                      (                                             )
 *
 *                 = r - E
 *   3. Scale back to obtain expm1(x):
 *      From step 1, we have
 *         expm1(x) = either 2^expm1_c__k*[expm1(r)+1] - 1
 *                  = or     2^expm1_c__k*[expm1(r) + (1-2^-expm1_c__k)]
 *   4. Implementation notes:
 *      (A). To save one multiplication, we scale the coefficient Qi
 *           to Qi*2^i, and replace z by (x^2)/2.
 *      (B). To achieve maximum accuracy, we compute expm1(x) by
 *        (i)   if x < -56*ln2, return -1.0, (raise inexact if x!=inf)
 *        (ii)  if expm1_c__k=0, return r-E
 *        (iii) if expm1_c__k=-1, return 0.5*(r-E)-0.5
 *        (iv)  if expm1_c__k=1 if r < -0.25, return 2*((r+0.5)- E)
 *                     else          return  1.0+2.0*(r-E);
 *        (v)   if (expm1_c__k<-2||expm1_c__k>56) return 2^expm1_c__k(1-(E-r)) - 1 (or exp(x)-1)
 *        (vi)  if expm1_c__k <= 20, return 2^expm1_c__k((1-2^-expm1_c__k)-(E-r)), else
 *        (vii) return 2^expm1_c__k(1-((E+2^-expm1_c__k)-r))
 *
 * Special cases:
 *      expm1(INF) is INF, expm1(NaN) is NaN;
 *      expm1(-INF) is -1, and
 *      for finite argument, only expm1(0)=0 is exact.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Misc. info.
 *      For IEEE double
 *          if x >  7.09782712893383973096e+02 then expm1(x) overflow
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
expm1_c__o_threshold = 7.09782712893383973096e+02, /* 0x40862E42, 0xFEFA39EF */
expm1_c__ln2_hi      = 6.93147180369123816490e-01, /* 0x3fe62e42, 0xfee00000 */
expm1_c__ln2_lo      = 1.90821492927058770002e-10, /* 0x3dea39ef, 0x35793c76 */
expm1_c__invln2      = 1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
/* Scaled Q's: Qn_here = 2**n * Qn_above, for expm1_c__R(2*z) where z = hxs = x*x/2: */
expm1_c__Q1 = -3.33333333333331316428e-02, /* BFA11111 111110F4 */
expm1_c__Q2 =  1.58730158725481460165e-03, /* 3F5A01A0 19FE5585 */
expm1_c__Q3 = -7.93650757867487942473e-05, /* BF14CE19 9EAADBB7 */
expm1_c__Q4 =  4.00821782732936239552e-06, /* 3ED0CFCA 86E65239 */
expm1_c__Q5 = -2.01099218183624371326e-07; /* BE8AFDB7 6E09C32D */

double expm1(double x) {
    double_t y,hi,lo,c,t,e,hxs,hfx,expm1_c__r1,twopk;
    union {double f; uint64_t i;} u = {x};
    uint32_t hx = u.i>>32 & 0x7fffffff;
    int expm1_c__k, sign = u.i>>63;

    /* filter out expm1_c__huge and non-finite argument */
    if (hx >= 0x4043687A) {  /* if |x|>=56*ln2 */
        if (isnan(x))
            return x;
        if (sign)
            return -1;
        if (x > expm1_c__o_threshold) {
            x *= 0x1p1023;
            return x;
        }
    }

    /* argument reduction */
    if (hx > 0x3fd62e42) {  /* if  |x| > 0.5 ln2 */
        if (hx < 0x3FF0A2B2) {  /* and |x| < 1.5 ln2 */
            if (!sign) {
                hi = x - expm1_c__ln2_hi;
                lo = expm1_c__ln2_lo;
                expm1_c__k =  1;
            } else {
                hi = x + expm1_c__ln2_hi;
                lo = -expm1_c__ln2_lo;
                expm1_c__k = -1;
            }
        } else {
            expm1_c__k  = expm1_c__invln2*x + (sign ? -0.5 : 0.5);
            t  = expm1_c__k;
            hi = x - t*expm1_c__ln2_hi;  /* t*expm1_c__ln2_hi is exact here */
            lo = t*expm1_c__ln2_lo;
        }
        x = hi-lo;
        c = (hi-x)-lo;
    } else if (hx < 0x3c900000) {  /* |x| < 2**-54, return x */
        if (hx < 0x00100000)
            FORCE_EVAL((float)x);
        return x;
    } else
        expm1_c__k = 0;

    /* x is now in primary range */
    hfx = 0.5*x;
    hxs = x*hfx;
    expm1_c__r1 = 1.0+hxs*(expm1_c__Q1+hxs*(expm1_c__Q2+hxs*(expm1_c__Q3+hxs*(expm1_c__Q4+hxs*expm1_c__Q5))));
    t  = 3.0-expm1_c__r1*hfx;
    e  = hxs*((expm1_c__r1-t)/(6.0 - x*t));
    if (expm1_c__k == 0)   /* c is 0 */
        return x - (x*e-hxs);
    e  = x*(e-c) - c;
    e -= hxs;
    /* exp(x) ~ 2^expm1_c__k (x_reduced - e + 1) */
    if (expm1_c__k == -1)
        return 0.5*(x-e) - 0.5;
    if (expm1_c__k == 1) {
        if (x < -0.25)
            return -2.0*(e-(x+0.5));
        return 1.0+2.0*(x-e);
    }
    u.i = (uint64_t)(0x3ff + expm1_c__k)<<52;  /* 2^expm1_c__k */
    twopk = u.f;
    if (expm1_c__k < 0 || expm1_c__k > 56) {  /* suffice to return exp(x)-1 */
        y = x - e + 1.0;
        if (expm1_c__k == 1024)
            y = y*2.0*0x1p1023;
        else
            y = y*twopk;
        return y - 1.0;
    }
    u.i = (uint64_t)(0x3ff - expm1_c__k)<<52;  /* 2^-expm1_c__k */
    if (expm1_c__k < 20)
        y = (x-e+(1-u.f))*twopk;
    else
        y = (x-(e+u.f)+1)*twopk;
    return y;
}

/// >>> START src/math/expm1f.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_expm1f.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
expm1f_c__o_threshold = 8.8721679688e+01, /* 0x42b17180 */
expm1f_c__ln2_hi      = 6.9313812256e-01, /* 0x3f317180 */
expm1f_c__ln2_lo      = 9.0580006145e-06, /* 0x3717f7d1 */
expm1f_c__invln2      = 1.4426950216e+00, /* 0x3fb8aa3b */
/*
 * Domain [-0.34568, 0.34568], range ~[-6.694e-10, 6.696e-10]:
 * |6 / x * (1 + 2 * (1 / (exp(x) - 1) - 1 / x)) - q(x)| < 2**-30.04
 * Scaled coefficients: Qn_here = 2**n * Qn_for_q (see s_expm1.c):
 */
expm1f_c__Q1 = -3.3333212137e-2, /* -0x888868.0p-28 */
expm1f_c__Q2 =  1.5807170421e-3; /*  0xcf3010.0p-33 */

float expm1f(float x) {
    float_t y,hi,lo,c,t,e,hxs,hfx,expm1f_c__r1,twopk;
    union {float f; uint32_t i;} u = {x};
    uint32_t hx = u.i & 0x7fffffff;
    int expm1f_c__k, sign = u.i >> 31;

    /* filter out expm1f_c__huge and non-finite argument */
    if (hx >= 0x4195b844) {  /* if |x|>=27*ln2 */
        if (hx > 0x7f800000)  /* NaN */
            return x;
        if (sign)
            return -1;
        if (x > expm1f_c__o_threshold) {
            x *= 0x1p127f;
            return x;
        }
    }

    /* argument reduction */
    if (hx > 0x3eb17218) {           /* if  |x| > 0.5 ln2 */
        if (hx < 0x3F851592) {       /* and |x| < 1.5 ln2 */
            if (!sign) {
                hi = x - expm1f_c__ln2_hi;
                lo = expm1f_c__ln2_lo;
                expm1f_c__k =  1;
            } else {
                hi = x + expm1f_c__ln2_hi;
                lo = -expm1f_c__ln2_lo;
                expm1f_c__k = -1;
            }
        } else {
            expm1f_c__k  = expm1f_c__invln2*x + (sign ? -0.5f : 0.5f);
            t  = expm1f_c__k;
            hi = x - t*expm1f_c__ln2_hi;      /* t*expm1f_c__ln2_hi is exact here */
            lo = t*expm1f_c__ln2_lo;
        }
        x = hi-lo;
        c = (hi-x)-lo;
    } else if (hx < 0x33000000) {  /* when |x|<2**-25, return x */
        if (hx < 0x00800000)
            FORCE_EVAL(x*x);
        return x;
    } else
        expm1f_c__k = 0;

    /* x is now in primary range */
    hfx = 0.5f*x;
    hxs = x*hfx;
    expm1f_c__r1 = 1.0f+hxs*(expm1f_c__Q1+hxs*expm1f_c__Q2);
    t  = 3.0f - expm1f_c__r1*hfx;
    e  = hxs*((expm1f_c__r1-t)/(6.0f - x*t));
    if (expm1f_c__k == 0)  /* c is 0 */
        return x - (x*e-hxs);
    e  = x*(e-c) - c;
    e -= hxs;
    /* exp(x) ~ 2^expm1f_c__k (x_reduced - e + 1) */
    if (expm1f_c__k == -1)
        return 0.5f*(x-e) - 0.5f;
    if (expm1f_c__k == 1) {
        if (x < -0.25f)
            return -2.0f*(e-(x+0.5f));
        return 1.0f + 2.0f*(x-e);
    }
    u.i = (0x7f+expm1f_c__k)<<23;  /* 2^expm1f_c__k */
    twopk = u.f;
    if (expm1f_c__k < 0 || expm1f_c__k > 56) {   /* suffice to return exp(x)-1 */
        y = x - e + 1.0f;
        if (expm1f_c__k == 128)
            y = y*2.0f*0x1p127f;
        else
            y = y*twopk;
        return y - 1.0f;
    }
    u.i = (0x7f-expm1f_c__k)<<23;  /* 2^-expm1f_c__k */
    if (expm1f_c__k < 23)
        y = (x-e+(1-u.f))*twopk;
    else
        y = (x-(e+u.f)+1)*twopk;
    return y;
}

/// >>> START src/math/expm1l.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_expm1l.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Exponential function, minus 1
 *      Long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, expm1l();
 *
 * y = expm1l( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns e (2.71828...) raised to the x power, minus 1.
 *
 * Range reduction is accomplished by separating the argument
 * into an integer expm1l_c__k and fraction f such that
 *
 *     x    expm1l_c__k  f
 *    e  = 2  e.
 *
 * An expansion x + .5 x^2 + x^3 expm1l_c__R(x) approximates exp(f) - 1
 * in the basic range [-0.5 ln 2, 0.5 ln 2].
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE    -45,+maxarg   200,000     1.2e-19     2.5e-20
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double expm1l(long double x) {
    return expm1(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384

/* exp(x) - 1 = x + 0.5 x^2 + x^3 expm1l_c__P(x)/expm1l_c__Q(x)
   -.5 ln 2  <  x  <  .5 ln 2
   Theoretical peak relative error = 3.4e-22  */
static const long double
expm1l_c__P0 = -1.586135578666346600772998894928250240826E4L,
expm1l_c__P1 =  2.642771505685952966904660652518429479531E3L,
expm1l_c__P2 = -3.423199068835684263987132888286791620673E2L,
expm1l_c__P3 =  1.800826371455042224581246202420972737840E1L,
expm1l_c__P4 = -5.238523121205561042771939008061958820811E-1L,
expm1l_c__Q0 = -9.516813471998079611319047060563358064497E4L,
expm1l_c__Q1 =  3.964866271411091674556850458227710004570E4L,
expm1l_c__Q2 = -7.207678383830091850230366618190187434796E3L,
expm1l_c__Q3 =  7.206038318724600171970199625081491823079E2L,
expm1l_c__Q4 = -4.002027679107076077238836622982900945173E1L,
/* expm1l_c__Q5 = 1.000000000000000000000000000000000000000E0 */
/* expm1l_c__C1 + expm1l_c__C2 = ln 2 */
expm1l_c__C1 = 6.93145751953125E-1L,
expm1l_c__C2 = 1.428606820309417232121458176568075500134E-6L,
/* ln 2^-65 */
minarg = -4.5054566736396445112120088E1L,
/* ln 2^16384 */
maxarg = 1.1356523406294143949492E4L;

long double expm1l(long double x) {
    long double px, qx, xx;
    int expm1l_c__k;

    if (isnan(x))
        return x;
    if (x > maxarg)
        return x*0x1p16383L; /* overflow, unless x==inf */
    if (x == 0.0)
        return x;
    if (x < minarg)
        return -1.0;

    xx = expm1l_c__C1 + expm1l_c__C2;
    /* Express x = ln 2 (expm1l_c__k + remainder), remainder not exceeding 1/2. */
    px = floorl(0.5 + x / xx);
    expm1l_c__k = px;
    /* remainder times ln 2 */
    x -= px * expm1l_c__C1;
    x -= px * expm1l_c__C2;

    /* Approximate exp(remainder ln 2).*/
    px = (((( expm1l_c__P4 * x + expm1l_c__P3) * x + expm1l_c__P2) * x + expm1l_c__P1) * x + expm1l_c__P0) * x;
    qx = (((( x + expm1l_c__Q4) * x + expm1l_c__Q3) * x + expm1l_c__Q2) * x + expm1l_c__Q1) * x + expm1l_c__Q0;
    xx = x * x;
    qx = x + (0.5 * xx + xx * px / qx);

    /* exp(x) = exp(expm1l_c__k ln 2) exp(remainder ln 2) = 2^expm1l_c__k exp(remainder ln 2).
     We have qx = exp(remainder ln 2) - 1, so
     exp(x) - 1  =  2^expm1l_c__k (qx + 1) - 1  =  2^expm1l_c__k qx + 2^expm1l_c__k - 1.  */
    px = scalbnl(1.0, expm1l_c__k);
    x = px * qx + (px - 1.0);
    return x;
}
#endif

/// >>> START src/math/fabs.c

double fabs(double x) {
    union {double f; uint64_t i;} u = {x};
    u.i &= -1ULL/2;
    return u.f;
}

/// >>> START src/math/fabsf.c

float fabsf(float x) {
    union {float f; uint32_t i;} u = {x};
    u.i &= 0x7fffffff;
    return u.f;
}

/// >>> START src/math/fabsl.c
#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fabsl(long double x) {
    return fabs(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double fabsl(long double x) {
    union ldshape u = {x};

    u.i.se &= 0x7fff;
    return u.f;
}
#endif

/// >>> START src/math/fdim.c

double fdim(double x, double y) {
    if (isnan(x))
        return x;
    if (isnan(y))
        return y;
    return x > y ? x - y : 0;
}

/// >>> START src/math/fdimf.c

float fdimf(float x, float y) {
    if (isnan(x))
        return x;
    if (isnan(y))
        return y;
    return x > y ? x - y : 0;
}

/// >>> START src/math/fdiml.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fdiml(long double x, long double y) {
    return fdim(x, y);
}
#else
long double fdiml(long double x, long double y) {
    if (isnan(x))
        return x;
    if (isnan(y))
        return y;
    return x > y ? x - y : 0;
}
#endif

/// >>> START src/math/finite.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int finite(double x) {
    return isfinite(x);
}

/// >>> START src/math/finitef.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int finitef(float x) {
    return isfinite(x);
}

/// >>> START src/math/floor.c

double floor(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = u.i >> 52 & 0x7ff;
    double_t y;

    if (e >= 0x3ff+52 || x == 0)
        return x;
    /* y = int(x) - x, where int(x) is an integer neighbor of x */
    if (u.i >> 63)
        y = (double)(x - 0x1p52) + 0x1p52 - x;
    else
        y = (double)(x + 0x1p52) - 0x1p52 - x;
    /* special case because of non-nearest rounding modes */
    if (e <= 0x3ff-1) {
        FORCE_EVAL(y);
        return u.i >> 63 ? -1 : 0;
    }
    if (y > 0)
        return x + y - 1;
    return x + y;
}

/// >>> START src/math/floorf.c

float floorf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = (int)(u.i >> 23 & 0xff) - 0x7f;
    uint32_t m;

    if (e >= 23)
        return x;
    if (e >= 0) {
        m = 0x007fffff >> e;
        if ((u.i & m) == 0)
            return x;
        FORCE_EVAL(x + 0x1p120f);
        if (u.i >> 31)
            u.i += m;
        u.i &= ~m;
    } else {
        FORCE_EVAL(x + 0x1p120f);
        if (u.i >> 31 == 0)
            u.i = 0;
        else if (u.i << 1)
            u.f = -1.0;
    }
    return u.f;
}

/// >>> START src/math/floorl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double floorl(long double x) {
    return floor(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double floorl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    long double y;

    if (e >= 0x3fff+LDBL_MANT_DIG-1 || x == 0)
        return x;
    /* y = int(x) - x, where int(x) is an integer neighbor of x */
    if (u.i.se >> 15)
        y = x - TOINT + TOINT - x;
    else
        y = x + TOINT - TOINT - x;
    /* special case because of non-nearest rounding modes */
    if (e <= 0x3fff-1) {
        FORCE_EVAL(y);
        return u.i.se >> 15 ? -1 : 0;
    }
    if (y > 0)
        return x + y - 1;
    return x + y;
}
#endif

/// >>> START src/math/fma.c

#if LDBL_MANT_DIG==64 && LDBL_MAX_EXP==16384
/* exact add, assumes exponent_x >= exponent_y */
static void add(long double *hi, long double *lo, long double x, long double y) {
    long double r;

    r = x + y;
    *hi = r;
    r -= x;
    *lo = y - r;
}

/* exact mul, assumes no over/underflow */
static void mul(long double *hi, long double *lo, long double x, long double y) {
    static const long double c = 1.0 + 0x1p32L;
    long double cx, xh, xl, cy, yh, yl;

    cx = c*x;
    xh = (x - cx) + cx;
    xl = x - xh;
    cy = c*y;
    yh = (y - cy) + cy;
    yl = y - yh;
    *hi = x*y;
    *lo = (xh*yh - *hi) + xh*yl + xl*yh + xl*yl;
}

/*
assume (long double)(hi+lo) == hi
return an adjusted hi so that rounding it to double (or less) precision is correct
*/
static long double adjust(long double hi, long double lo) {
    union ldshape uhi, ulo;

    if (lo == 0)
        return hi;
    uhi.f = hi;
    if (uhi.i.m & 0x3ff)
        return hi;
    ulo.f = lo;
    if ((uhi.i.se & 0x8000) == (ulo.i.se & 0x8000))
        uhi.i.m++;
    else {
        /* handle underflow and take care of ld80 implicit msb */
        if (uhi.i.m << 1 == 0) {
            uhi.i.m = 0;
            uhi.i.se--;
        }
        uhi.i.m--;
    }
    return uhi.f;
}

/* adjusted add so the result is correct when rounded to double (or less) precision */
static long double dadd(long double x, long double y) {
    add(&x, &y, x, y);
    return adjust(x, y);
}

/* adjusted mul so the result is correct when rounded to double (or less) precision */
static long double dmul(long double x, long double y) {
    mul(&x, &y, x, y);
    return adjust(x, y);
}

static int getexp(long double x) {
    union ldshape u;
    u.f = x;
    return u.i.se & 0x7fff;
}

double fma(double x, double y, double z) {
    long double hi, lo1, lo2, xy;
    int round, ez, exy;

    /* handle +-inf,nan */
    if (!isfinite(x) || !isfinite(y))
        return x*y + z;
    if (!isfinite(z))
        return z;
    /* handle +-0 */
    if (x == 0.0 || y == 0.0)
        return x*y + z;
    round = fegetround();
    if (z == 0.0) {
        if (round == FE_TONEAREST)
            return dmul(x, y);
        return x*y;
    }

    /* exact mul and add require nearest rounding */
    /* spurious inexact exceptions may be raised */
    fesetround(FE_TONEAREST);
    mul(&xy, &lo1, x, y);
    exy = getexp(xy);
    ez = getexp(z);
    if (ez > exy) {
        add(&hi, &lo2, z, xy);
    } else if (ez > exy - 12) {
        add(&hi, &lo2, xy, z);
        if (hi == 0) {
            /*
            xy + z is 0, but it should be calculated with the
            original rounding mode so the sign is correct, if the
            compiler does not support FENV_ACCESS ON it does not
            know about the changed rounding mode and eliminates
            the xy + z below without the volatile memory access
            */
            volatile double z_;
            fesetround(round);
            z_ = z;
            return (xy + z_) + lo1;
        }
    } else {
        /*
        ez <= exy - 12
        the 12 extra bits (1guard, 11round+sticky) are needed so with
            lo = dadd(lo1, lo2)
        elo <= ehi - 11, and we use the last 10 bits in adjust so
            dadd(hi, lo)
        gives correct result when rounded to double
        */
        hi = xy;
        lo2 = z;
    }
    /*
    the result is stored before return for correct precision and exceptions

    one corner case is when the underflow flag should be raised because
    the precise result is an inexact subnormal double, but the calculated
    long double result is an exact subnormal double
    (so rounding to double does not raise exceptions)

    in nearest rounding mode dadd takes care of this: the last bit of the
    result is adjusted so rounding sees an inexact value when it should

    in non-nearest rounding mode fenv is used for the workaround
    */
    fesetround(round);
    if (round == FE_TONEAREST)
        z = dadd(hi, dadd(lo1, lo2));
    else {
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        int e = fetestexcept(FE_INEXACT);
        feclearexcept(FE_INEXACT);
#endif
        z = hi + (lo1 + lo2);
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        if (getexp(z) < 0x3fff-1022 && fetestexcept(FE_INEXACT))
            feraiseexcept(FE_UNDERFLOW);
        else if (e)
            feraiseexcept(FE_INEXACT);
#endif
    }
    return z;
}
#else
/* origin: FreeBSD /usr/src/lib/msun/src/s_fma.c */
/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * A struct dd represents a floating-point number with twice the precision
 * of a double.  We maintain the invariant that "hi" stores the 53 high-order
 * bits of the result.
 */
struct dd {
    double hi;
    double lo;
};

/*
 * Compute a+b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are finite, but make no assumptions about their relative
 * magnitudes.
 */
static inline struct dd dd_add(double a, double b) {
    struct dd ret;
    double s;

    ret.hi = a + b;
    s = ret.hi - a;
    ret.lo = (a - (ret.hi - s)) + (b - s);
    return (ret);
}

/*
 * Compute a+b, with a small tweak:  The least significant bit of the
 * result is adjusted into a sticky bit summarizing all the bits that
 * were lost to rounding.  This adjustment negates the effects of double
 * rounding when the result is added to another number with a higher
 * exponent.  For an explanation of round and sticky bits, see any reference
 * on FPU design, e.g.,
 *
 *     J. Coonen.  An Implementation Guide to a Proposed Standard for
 *     Floating-Point Arithmetic.  Computer, vol. 13, no. 1, Jan 1980.
 */
static inline double add_adjusted(double a, double b) {
    struct dd sum;
    union {double f; uint64_t i;} uhi, ulo;

    sum = dd_add(a, b);
    if (sum.lo != 0) {
        uhi.f = sum.hi;
        if ((uhi.i & 1) == 0) {
            /* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
            ulo.f = sum.lo;
            uhi.i += 1 - ((uhi.i ^ ulo.i) >> 62);
            sum.hi = uhi.f;
        }
    }
    return (sum.hi);
}

/*
 * Compute ldexp(a+b, scale) with a single rounding error. It is assumed
 * that the result will be subnormal, and care is taken to ensure that
 * double rounding does not occur.
 */
static inline double add_and_denormalize(double a, double b, int scale) {
    struct dd sum;
    union {double f; uint64_t i;} uhi, ulo;
    int bits_lost;

    sum = dd_add(a, b);

    /*
     * If we are losing at least two bits of accuracy to denormalization,
     * then the first lost bit becomes a round bit, and we adjust the
     * lowest bit of sum.hi to make it a sticky bit summarizing all the
     * bits in sum.lo. With the sticky bit adjusted, the hardware will
     * break any ties in the correct direction.
     *
     * If we are losing only one bit to denormalization, however, we must
     * break the ties manually.
     */
    if (sum.lo != 0) {
        uhi.f = sum.hi;
        bits_lost = -((int)(uhi.i >> 52) & 0x7ff) - scale + 1;
        if (bits_lost != 1 ^ (int)(uhi.i & 1)) {
            /* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
            ulo.f = sum.lo;
            uhi.i += 1 - (((uhi.i ^ ulo.i) >> 62) & 2);
            sum.hi = uhi.f;
        }
    }
    return scalbn(sum.hi, scale);
}

/*
 * Compute a*b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are normalized, so no underflow or overflow will occur.
 * The current rounding mode must be round-to-nearest.
 */
static inline struct dd dd_mul(double a, double b) {
    static const double split = 0x1p27 + 1.0;
    struct dd ret;
    double ha, hb, la, lb, p, q;

    p = a * split;
    ha = a - p;
    ha += p;
    la = a - ha;

    p = b * split;
    hb = b - p;
    hb += p;
    lb = b - hb;

    p = ha * hb;
    q = ha * lb + la * hb;

    ret.hi = p + q;
    ret.lo = p - ret.hi + q + la * lb;
    return (ret);
}

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * We use scaling to avoid overflow/underflow, along with the
 * canonical precision-doubling technique adapted from:
 *
 *      Dekker, fma_c__T.  A Floating-Point Technique for Extending the
 *      Available Precision.  Numer. Math. 18, 224-242 (1971).
 *
 * This algorithm is sensitive to the rounding precision.  FPUs such
 * as the i387 must be set in double-precision mode if variables are
 * to be stored in FP registers in order to avoid incorrect results.
 * This is the default on FreeBSD, but not on many other systems.
 *
 * Hardware instructions should be used on architectures that support it,
 * since this implementation will likely be several times slower.
 */
double fma(double x, double y, double z) {
    double xs, ys, zs, adj;
    struct dd xy, r;
    int oround;
    int ex, ey, ez;
    int spread;

    /*
     * Handle special cases. The order of operations and the particular
     * return values here are crucial in handling special cases involving
     * infinities, NaNs, overflows, and signed zeroes correctly.
     */
    if (!isfinite(x) || !isfinite(y))
        return (x * y + z);
    if (!isfinite(z))
        return (z);
    if (x == 0.0 || y == 0.0)
        return (x * y + z);
    if (z == 0.0)
        return (x * y);

    xs = frexp(x, &ex);
    ys = frexp(y, &ey);
    zs = frexp(z, &ez);
    oround = fegetround();
    spread = ex + ey - ez;

    /*
     * If x * y and z are many orders of magnitude apart, the scaling
     * will overflow, so we handle these cases specially.  Rounding
     * modes other than FE_TONEAREST are painful.
     */
    if (spread < -DBL_MANT_DIG) {
#ifdef FE_INEXACT
        feraiseexcept(FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
        if (!isnormal(z))
            feraiseexcept(FE_UNDERFLOW);
#endif
        switch (oround) {
        default: /* FE_TONEAREST */
            return (z);
#ifdef FE_TOWARDZERO
        case FE_TOWARDZERO:
            if (x > 0.0 ^ y < 0.0 ^ z < 0.0)
                return (z);
            else
                return (nextafter(z, 0));
#endif
#ifdef FE_DOWNWARD
        case FE_DOWNWARD:
            if (x > 0.0 ^ y < 0.0)
                return (z);
            else
                return (nextafter(z, -INFINITY));
#endif
#ifdef FE_UPWARD
        case FE_UPWARD:
            if (x > 0.0 ^ y < 0.0)
                return (nextafter(z, INFINITY));
            else
                return (z);
#endif
        }
    }
    if (spread <= DBL_MANT_DIG * 2)
        zs = scalbn(zs, -spread);
    else
        zs = copysign(DBL_MIN, zs);

    fesetround(FE_TONEAREST);

    /*
     * Basic approach for round-to-nearest:
     *
     *     (xy.hi, xy.lo) = x * y           (exact)
     *     (r.hi, r.lo)   = xy.hi + z       (exact)
     *     adj = xy.lo + r.lo               (inexact; low bit is sticky)
     *     result = r.hi + adj              (correctly rounded)
     */
    xy = dd_mul(xs, ys);
    r = dd_add(xy.hi, zs);

    spread = ex + ey;

    if (r.hi == 0.0) {
        /*
         * When the addends cancel to 0, ensure that the result has
         * the correct sign.
         */
        fesetround(oround);
        volatile double vzs = zs; /* XXX gcc CSE bug workaround */
        return xy.hi + vzs + scalbn(xy.lo, spread);
    }

    if (oround != FE_TONEAREST) {
        /*
         * There is no need to worry about double rounding in directed
         * rounding modes.
         * But underflow may not be raised properly, example in downward rounding:
         * fma(0x1.000000001p-1000, 0x1.000000001p-30, -0x1p-1066)
         */
        double ret;
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        int e = fetestexcept(FE_INEXACT);
        feclearexcept(FE_INEXACT);
#endif
        fesetround(oround);
        adj = r.lo + xy.lo;
        ret = scalbn(r.hi + adj, spread);
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        if (ilogb(ret) < -1022 && fetestexcept(FE_INEXACT))
            feraiseexcept(FE_UNDERFLOW);
        else if (e)
            feraiseexcept(FE_INEXACT);
#endif
        return ret;
    }

    adj = add_adjusted(r.lo, xy.lo);
    if (spread + ilogb(r.hi) > -1023)
        return scalbn(r.hi + adj, spread);
    else
        return add_and_denormalize(r.hi, adj, spread);
}
#endif

/// >>> START src/math/fmaf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_fmaf.c */
/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * A double has more than twice as much precision than a float, so
 * direct double-precision arithmetic suffices, except where double
 * rounding occurs.
 */
float fmaf(float x, float y, float z) {
    double xy, result;
    union {double f; uint64_t i;} u;
    int e;

    xy = (double)x * y;
    result = xy + z;
    u.f = result;
    e = u.i>>52 & 0x7ff;
    /* Common case: The double precision result is fine. */
    if ((u.i & 0x1fffffff) != 0x10000000 || /* not a halfway case */
        e == 0x7ff ||                   /* NaN */
        result - xy == z ||                 /* exact */
        fegetround() != FE_TONEAREST)       /* not round-to-nearest */ {
        /*
        underflow may not be raised correctly, example:
        fmaf(0x1p-120f, 0x1p-120f, 0x1p-149f)
        */
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        if (e < 0x3ff-126 && e >= 0x3ff-149 && fetestexcept(FE_INEXACT)) {
            feclearexcept(FE_INEXACT);
            /* TODO: gcc and clang bug workaround */
            volatile float vz = z;
            result = xy + vz;
            if (fetestexcept(FE_INEXACT))
                feraiseexcept(FE_UNDERFLOW);
            else
                feraiseexcept(FE_INEXACT);
        }
#endif
        z = result;
        return z;
    }

    /*
     * If result is inexact, and exactly halfway between two float values,
     * we need to adjust the low-order bit in the direction of the error.
     */
#ifdef FE_TOWARDZERO
    fesetround(FE_TOWARDZERO);
#endif
    volatile double vxy = xy;  /* XXX work around gcc CSE bug */
    double adjusted_result = vxy + z;
    fesetround(FE_TONEAREST);
    if (result == adjusted_result) {
        u.f = adjusted_result;
        u.i++;
        adjusted_result = u.f;
    }
    z = adjusted_result;
    return z;
}

/// >>> START src/math/fmal.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_fmal.c */
/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fmal(long double x, long double y, long double z) {
    return fma(x, y, z);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef LASTBIT
#define LASTBIT(u) (u.i.m & 1)
#undef SPLIT
#define SPLIT (0x1p32L + 1)
#elif LDBL_MANT_DIG == 113
#undef LASTBIT
#define LASTBIT(u) (u.i.lo & 1)
#undef SPLIT
#define SPLIT (0x1p57L + 1)
#endif

/*
 * A struct dd represents a floating-point number with twice the precision
 * of a long double.  We maintain the invariant that "hi" stores the high-order
 * bits of the result.
 */
struct dd {
    long double hi;
    long double lo;
};

/*
 * Compute a+b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are finite, but make no assumptions about their relative
 * magnitudes.
 */
static inline struct dd dd_add(long double a, long double b) {
    struct dd ret;
    long double s;

    ret.hi = a + b;
    s = ret.hi - a;
    ret.lo = (a - (ret.hi - s)) + (b - s);
    return (ret);
}

/*
 * Compute a+b, with a small tweak:  The least significant bit of the
 * result is adjusted into a sticky bit summarizing all the bits that
 * were lost to rounding.  This adjustment negates the effects of double
 * rounding when the result is added to another number with a higher
 * exponent.  For an explanation of round and sticky bits, see any reference
 * on FPU design, e.g.,
 *
 *     J. Coonen.  An Implementation Guide to a Proposed Standard for
 *     Floating-Point Arithmetic.  Computer, vol. 13, no. 1, Jan 1980.
 */
static inline long double add_adjusted(long double a, long double b) {
    struct dd sum;
    union ldshape u;

    sum = dd_add(a, b);
    if (sum.lo != 0) {
        u.f = sum.hi;
        if (!LASTBIT(u))
            sum.hi = nextafterl(sum.hi, INFINITY * sum.lo);
    }
    return (sum.hi);
}

/*
 * Compute ldexp(a+b, scale) with a single rounding error. It is assumed
 * that the result will be subnormal, and care is taken to ensure that
 * double rounding does not occur.
 */
static inline long double add_and_denormalize(long double a, long double b, int scale) {
    struct dd sum;
    int bits_lost;
    union ldshape u;

    sum = dd_add(a, b);

    /*
     * If we are losing at least two bits of accuracy to denormalization,
     * then the first lost bit becomes a round bit, and we adjust the
     * lowest bit of sum.hi to make it a sticky bit summarizing all the
     * bits in sum.lo. With the sticky bit adjusted, the hardware will
     * break any ties in the correct direction.
     *
     * If we are losing only one bit to denormalization, however, we must
     * break the ties manually.
     */
    if (sum.lo != 0) {
        u.f = sum.hi;
        bits_lost = -u.i.se - scale + 1;
        if ((bits_lost != 1) ^ LASTBIT(u))
            sum.hi = nextafterl(sum.hi, INFINITY * sum.lo);
    }
    return scalbnl(sum.hi, scale);
}

/*
 * Compute a*b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are normalized, so no underflow or overflow will occur.
 * The current rounding mode must be round-to-nearest.
 */
static inline struct dd dd_mul(long double a, long double b) {
    struct dd ret;
    long double ha, hb, la, lb, p, q;

    p = a * SPLIT;
    ha = a - p;
    ha += p;
    la = a - ha;

    p = b * SPLIT;
    hb = b - p;
    hb += p;
    lb = b - hb;

    p = ha * hb;
    q = ha * lb + la * hb;

    ret.hi = p + q;
    ret.lo = p - ret.hi + q + la * lb;
    return (ret);
}

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * We use scaling to avoid overflow/underflow, along with the
 * canonical precision-doubling technique adapted from:
 *
 *      Dekker, fmal_c__T.  A Floating-Point Technique for Extending the
 *      Available Precision.  Numer. Math. 18, 224-242 (1971).
 */
long double fmal(long double x, long double y, long double z) {
    long double xs, ys, zs, adj;
    struct dd xy, r;
    int oround;
    int ex, ey, ez;
    int spread;

    /*
     * Handle special cases. The order of operations and the particular
     * return values here are crucial in handling special cases involving
     * infinities, NaNs, overflows, and signed zeroes correctly.
     */
    if (!isfinite(x) || !isfinite(y))
        return (x * y + z);
    if (!isfinite(z))
        return (z);
    if (x == 0.0 || y == 0.0)
        return (x * y + z);
    if (z == 0.0)
        return (x * y);

    xs = frexpl(x, &ex);
    ys = frexpl(y, &ey);
    zs = frexpl(z, &ez);
    oround = fegetround();
    spread = ex + ey - ez;

    /*
     * If x * y and z are many orders of magnitude apart, the scaling
     * will overflow, so we handle these cases specially.  Rounding
     * modes other than FE_TONEAREST are painful.
     */
    if (spread < -LDBL_MANT_DIG) {
#ifdef FE_INEXACT
        feraiseexcept(FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
        if (!isnormal(z))
            feraiseexcept(FE_UNDERFLOW);
#endif
        switch (oround) {
        default: /* FE_TONEAREST */
            return (z);
#ifdef FE_TOWARDZERO
        case FE_TOWARDZERO:
            if (x > 0.0 ^ y < 0.0 ^ z < 0.0)
                return (z);
            else
                return (nextafterl(z, 0));
#endif
#ifdef FE_DOWNWARD
        case FE_DOWNWARD:
            if (x > 0.0 ^ y < 0.0)
                return (z);
            else
                return (nextafterl(z, -INFINITY));
#endif
#ifdef FE_UPWARD
        case FE_UPWARD:
            if (x > 0.0 ^ y < 0.0)
                return (nextafterl(z, INFINITY));
            else
                return (z);
#endif
        }
    }
    if (spread <= LDBL_MANT_DIG * 2)
        zs = scalbnl(zs, -spread);
    else
        zs = copysignl(LDBL_MIN, zs);

    fesetround(FE_TONEAREST);

    /*
     * Basic approach for round-to-nearest:
     *
     *     (xy.hi, xy.lo) = x * y           (exact)
     *     (r.hi, r.lo)   = xy.hi + z       (exact)
     *     adj = xy.lo + r.lo               (inexact; low bit is sticky)
     *     result = r.hi + adj              (correctly rounded)
     */
    xy = dd_mul(xs, ys);
    r = dd_add(xy.hi, zs);

    spread = ex + ey;

    if (r.hi == 0.0) {
        /*
         * When the addends cancel to 0, ensure that the result has
         * the correct sign.
         */
        fesetround(oround);
        volatile long double vzs = zs; /* XXX gcc CSE bug workaround */
        return xy.hi + vzs + scalbnl(xy.lo, spread);
    }

    if (oround != FE_TONEAREST) {
        /*
         * There is no need to worry about double rounding in directed
         * rounding modes.
         * But underflow may not be raised correctly, example in downward rounding:
         * fmal(0x1.0000000001p-16000L, 0x1.0000000001p-400L, -0x1p-16440L)
         */
        long double ret;
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        int e = fetestexcept(FE_INEXACT);
        feclearexcept(FE_INEXACT);
#endif
        fesetround(oround);
        adj = r.lo + xy.lo;
        ret = scalbnl(r.hi + adj, spread);
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
        if (ilogbl(ret) < -16382 && fetestexcept(FE_INEXACT))
            feraiseexcept(FE_UNDERFLOW);
        else if (e)
            feraiseexcept(FE_INEXACT);
#endif
        return ret;
    }

    adj = add_adjusted(r.lo, xy.lo);
    if (spread + ilogbl(r.hi) > -16383)
        return scalbnl(r.hi + adj, spread);
    else
        return add_and_denormalize(r.hi, adj, spread);
}
#endif

/// >>> START src/math/fmax.c

double fmax(double x, double y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeros, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? y : x;
    return x < y ? y : x;
}

/// >>> START src/math/fmaxf.c

float fmaxf(float x, float y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeroes, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? y : x;
    return x < y ? y : x;
}

/// >>> START src/math/fmaxl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fmaxl(long double x, long double y) {
    return fmax(x, y);
}
#else
long double fmaxl(long double x, long double y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeros, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? y : x;
    return x < y ? y : x;
}
#endif

/// >>> START src/math/fmin.c

double fmin(double x, double y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeros, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? x : y;
    return x < y ? x : y;
}

/// >>> START src/math/fminf.c

float fminf(float x, float y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeros, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? x : y;
    return x < y ? x : y;
}

/// >>> START src/math/fminl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fminl(long double x, long double y) {
    return fmin(x, y);
}
#else
long double fminl(long double x, long double y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    /* handle signed zeros, see C99 Annex F.9.9.2 */
    if (signbit(x) != signbit(y))
        return signbit(x) ? x : y;
    return x < y ? x : y;
}
#endif

/// >>> START src/math/fmod.c

double fmod(double x, double y) {
    union {double f; uint64_t i;} ux = {x}, uy = {y};
    int ex = ux.i>>52 & 0x7ff;
    int ey = uy.i>>52 & 0x7ff;
    int sx = ux.i>>63;
    uint64_t i;

    /* in the followings uxi should be ux.i, but then gcc wrongly adds */
    /* float load/store to inner loops ruining performance and code size */
    uint64_t uxi = ux.i;

    if (uy.i<<1 == 0 || isnan(y) || ex == 0x7ff)
        return (x*y)/(x*y);
    if (uxi<<1 <= uy.i<<1) {
        if (uxi<<1 == uy.i<<1)
            return 0*x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        for (i = uxi<<12; i>>63 == 0; ex--, i <<= 1);
        uxi <<= -ex + 1;
    } else {
        uxi &= -1ULL >> 12;
        uxi |= 1ULL << 52;
    }
    if (!ey) {
        for (i = uy.i<<12; i>>63 == 0; ey--, i <<= 1);
        uy.i <<= -ey + 1;
    } else {
        uy.i &= -1ULL >> 12;
        uy.i |= 1ULL << 52;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 63 == 0) {
            if (i == 0)
                return 0*x;
            uxi = i;
        }
        uxi <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 63 == 0) {
        if (i == 0)
            return 0*x;
        uxi = i;
    }
    for (; uxi>>52 == 0; uxi <<= 1, ex--);

    /* scale result */
    if (ex > 0) {
        uxi -= 1ULL << 52;
        uxi |= (uint64_t)ex << 52;
    } else {
        uxi >>= -ex + 1;
    }
    uxi |= (uint64_t)sx << 63;
    ux.i = uxi;
    return ux.f;
}

/// >>> START src/math/fmodf.c

float fmodf(float x, float y) {
    union {float f; uint32_t i;} ux = {x}, uy = {y};
    int ex = ux.i>>23 & 0xff;
    int ey = uy.i>>23 & 0xff;
    uint32_t sx = ux.i & 0x80000000;
    uint32_t i;
    uint32_t uxi = ux.i;

    if (uy.i<<1 == 0 || isnan(y) || ex == 0xff)
        return (x*y)/(x*y);
    if (uxi<<1 <= uy.i<<1) {
        if (uxi<<1 == uy.i<<1)
            return 0*x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        for (i = uxi<<9; i>>31 == 0; ex--, i <<= 1);
        uxi <<= -ex + 1;
    } else {
        uxi &= -1U >> 9;
        uxi |= 1U << 23;
    }
    if (!ey) {
        for (i = uy.i<<9; i>>31 == 0; ey--, i <<= 1);
        uy.i <<= -ey + 1;
    } else {
        uy.i &= -1U >> 9;
        uy.i |= 1U << 23;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 31 == 0) {
            if (i == 0)
                return 0*x;
            uxi = i;
        }
        uxi <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 31 == 0) {
        if (i == 0)
            return 0*x;
        uxi = i;
    }
    for (; uxi>>23 == 0; uxi <<= 1, ex--);

    /* scale result up */
    if (ex > 0) {
        uxi -= 1U << 23;
        uxi |= (uint32_t)ex << 23;
    } else {
        uxi >>= -ex + 1;
    }
    uxi |= sx;
    ux.i = uxi;
    return ux.f;
}

/// >>> START src/math/fmodl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fmodl(long double x, long double y) {
    return fmod(x, y);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double fmodl(long double x, long double y) {
    union ldshape ux = {x}, uy = {y};
    int ex = ux.i.se & 0x7fff;
    int ey = uy.i.se & 0x7fff;
    int sx = ux.i.se & 0x8000;

    if (y == 0 || isnan(y) || ex == 0x7fff)
        return (x*y)/(x*y);
    ux.i.se = ex;
    uy.i.se = ey;
    if (ux.f <= uy.f) {
        if (ux.f == uy.f)
            return 0*x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        ux.f *= 0x1p120f;
        ex = ux.i.se - 120;
    }
    if (!ey) {
        uy.f *= 0x1p120f;
        ey = uy.i.se - 120;
    }

    /* x mod y */
#if LDBL_MANT_DIG == 64
    uint64_t i, mx, my;
    mx = ux.i.m;
    my = uy.i.m;
    for (; ex > ey; ex--) {
        i = mx - my;
        if (mx >= my) {
            if (i == 0)
                return 0*x;
            mx = 2*i;
        } else if (2*mx < mx) {
            mx = 2*mx - my;
        } else {
            mx = 2*mx;
        }
    }
    i = mx - my;
    if (mx >= my) {
        if (i == 0)
            return 0*x;
        mx = i;
    }
    for (; mx >> 63 == 0; mx *= 2, ex--);
    ux.i.m = mx;
#elif LDBL_MANT_DIG == 113
    uint64_t hi, lo, xhi, xlo, yhi, ylo;
    xhi = (ux.i2.hi & -1ULL>>16) | 1ULL<<48;
    yhi = (uy.i2.hi & -1ULL>>16) | 1ULL<<48;
    xlo = ux.i2.lo;
    ylo = ux.i2.lo;
    for (; ex > ey; ex--) {
        hi = xhi - yhi;
        lo = xlo - ylo;
        if (xlo < ylo)
            hi -= 1;
        if (hi >> 63 == 0) {
            if ((hi|lo) == 0)
                return 0*x;
            xhi = 2*hi + (lo>>63);
            xlo = 2*lo;
        } else {
            xhi = 2*xhi + (xlo>>63);
            xlo = 2*xlo;
        }
    }
    hi = xhi - yhi;
    lo = xlo - ylo;
    if (xlo < ylo)
        hi -= 1;
    if (hi >> 63 == 0) {
        if ((hi|lo) == 0)
            return 0*x;
        xhi = hi;
        xlo = lo;
    }
    for (; xhi >> 48 == 0; xhi = 2*xhi + (xlo>>63), xlo = 2*xlo, ex--);
    ux.i2.hi = xhi;
    ux.i2.lo = xlo;
#endif

    /* scale result */
    if (ex <= 0) {
        ux.i.se = (ex+120)|sx;
        ux.f *= 0x1p-120f;
    } else
        ux.i.se = ex|sx;
    return ux.f;
}
#endif

/// >>> START src/math/frexp.c

double frexp(double x, int *e) {
    union { double d; uint64_t i; } y = { x };
    int ee = y.i>>52 & 0x7ff;

    if (!ee) {
        if (x) {
            x = frexp(x*0x1p64, e);
            *e -= 64;
        } else *e = 0;
        return x;
    } else if (ee == 0x7ff) {
        return x;
    }

    *e = ee - 0x3fe;
    y.i &= 0x800fffffffffffffull;
    y.i |= 0x3fe0000000000000ull;
    return y.d;
}

/// >>> START src/math/frexpf.c

float frexpf(float x, int *e) {
    union { float f; uint32_t i; } y = { x };
    int ee = y.i>>23 & 0xff;

    if (!ee) {
        if (x) {
            x = frexpf(x*0x1p64, e);
            *e -= 64;
        } else *e = 0;
        return x;
    } else if (ee == 0xff) {
        return x;
    }

    *e = ee - 0x7e;
    y.i &= 0x807ffffful;
    y.i |= 0x3f000000ul;
    return y.f;
}

/// >>> START src/math/frexpl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double frexpl(long double x, int *e) {
    return frexp(x, e);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double frexpl(long double x, int *e) {
    union ldshape u = {x};
    int ee = u.i.se & 0x7fff;

    if (!ee) {
        if (x) {
            x = frexpl(x*0x1p120, e);
            *e -= 120;
        } else *e = 0;
        return x;
    } else if (ee == 0x7fff) {
        return x;
    }

    *e = ee - 0x3ffe;
    u.i.se &= 0x8000;
    u.i.se |= 0x3ffe;
    return u.f;
}
#endif

/// >>> START src/math/hypot.c

#if FLT_EVAL_METHOD > 1U && LDBL_MANT_DIG == 64
#undef SPLIT
#define SPLIT (0x1p32 + 1)
#else
#undef SPLIT
#define SPLIT (0x1p27 + 1)
#endif

static void hypot_c__sq(double_t *hi, double_t *lo, double x) {
    double_t xh, xl, xc;

    xc = x*SPLIT;
    xh = x - xc + xc;
    xl = x - xh;
    *hi = x*x;
    *lo = xh*xh - *hi + 2*xh*xl + xl*xl;
}

double hypot(double x, double y) {
    union {double f; uint64_t i;} ux = {x}, uy = {y}, hypot_c__ut;
    int ex, ey;
    double_t hx, lx, hy, ly, z;

    /* arrange |x| >= |y| */
    ux.i &= -1ULL>>1;
    uy.i &= -1ULL>>1;
    if (ux.i < uy.i) {
        hypot_c__ut = ux;
        ux = uy;
        uy = hypot_c__ut;
    }

    /* special cases */
    ex = ux.i>>52;
    ey = uy.i>>52;
    x = ux.f;
    y = uy.f;
    /* note: hypot(inf,nan) == inf */
    if (ey == 0x7ff)
        return y;
    if (ex == 0x7ff || uy.i == 0)
        return x;
    /* note: hypot(x,y) ~= x + y*y/x/2 with inexact for small y/x */
    /* 64 difference is enough for ld80 double_t */
    if (ex - ey > 64)
        return x + y;

    /* precise sqrt argument in nearest rounding mode without overflow */
    /* xh*xh must not overflow and xl*xl must not underflow in hypot_c__sq */
    z = 1;
    if (ex > 0x3ff+510) {
        z = 0x1p700;
        x *= 0x1p-700;
        y *= 0x1p-700;
    } else if (ey < 0x3ff-450) {
        z = 0x1p-700;
        x *= 0x1p700;
        y *= 0x1p700;
    }
    hypot_c__sq(&hx, &lx, x);
    hypot_c__sq(&hy, &ly, y);
    return z*sqrt(ly+lx+hy+hx);
}

/// >>> START src/math/hypotf.c

float hypotf(float x, float y) {
    union {float f; uint32_t i;} ux = {x}, uy = {y}, hypotf_c__ut;
    float_t z;

    ux.i &= -1U>>1;
    uy.i &= -1U>>1;
    if (ux.i < uy.i) {
        hypotf_c__ut = ux;
        ux = uy;
        uy = hypotf_c__ut;
    }

    x = ux.f;
    y = uy.f;
    if (uy.i == 0xff<<23)
        return y;
    if (ux.i >= 0xff<<23 || uy.i == 0 || ux.i - uy.i >= 25<<23)
        return x + y;

    z = 1;
    if (ux.i >= (0x7f+60)<<23) {
        z = 0x1p90f;
        x *= 0x1p-90f;
        y *= 0x1p-90f;
    } else if (uy.i < (0x7f-60)<<23) {
        z = 0x1p-90f;
        x *= 0x1p90f;
        y *= 0x1p90f;
    }
    return z*sqrtf((double)x*x + (double)y*y);
}

/// >>> START src/math/hypotl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double hypotl(long double x, long double y) {
    return hypot(x, y);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef SPLIT
#define SPLIT (0x1p32L+1)
#elif LDBL_MANT_DIG == 113
#undef SPLIT
#define SPLIT (0x1p57L+1)
#endif

static void hypotl_c__sq(long double *hi, long double *lo, long double x) {
    long double xh, xl, xc;
    xc = x*SPLIT;
    xh = x - xc + xc;
    xl = x - xh;
    *hi = x*x;
    *lo = xh*xh - *hi + 2*xh*xl + xl*xl;
}

long double hypotl(long double x, long double y) {
    union ldshape ux = {x}, uy = {y};
    int ex, ey;
    long double hx, lx, hy, ly, z;

    ux.i.se &= 0x7fff;
    uy.i.se &= 0x7fff;
    if (ux.i.se < uy.i.se) {
        ex = uy.i.se;
        ey = ux.i.se;
        x = uy.f;
        y = ux.f;
    } else {
        ex = ux.i.se;
        ey = uy.i.se;
        x = ux.f;
        y = uy.f;
    }

    if (ex == 0x7fff && isinf(y))
        return y;
    if (ex == 0x7fff || y == 0)
        return x;
    if (ex - ey > LDBL_MANT_DIG)
        return x + y;

    z = 1;
    if (ex > 0x3fff+8000) {
        z = 0x1p10000L;
        x *= 0x1p-10000L;
        y *= 0x1p-10000L;
    } else if (ey < 0x3fff-8000) {
        z = 0x1p-10000L;
        x *= 0x1p10000L;
        y *= 0x1p10000L;
    }
    hypotl_c__sq(&hx, &lx, x);
    hypotl_c__sq(&hy, &ly, y);
    return z*sqrtl(ly+lx+hy+hx);
}
#endif

/// >>> START src/math/ilogb.c

int ilogb(double x) {
    union {double f; uint64_t i;} u = {x};
    uint64_t i = u.i;
    int e = i>>52 & 0x7ff;

    if (!e) {
        i <<= 12;
        if (i == 0) {
            FORCE_EVAL(0/0.0f);
            return FP_ILOGB0;
        }
        /* subnormal x */
        for (e = -0x3ff; i>>63 == 0; e--, i<<=1);
        return e;
    }
    if (e == 0x7ff) {
        FORCE_EVAL(0/0.0f);
        return i<<12 ? FP_ILOGBNAN : INT_MAX;
    }
    return e - 0x3ff;
}

/// >>> START src/math/ilogbf.c

int ilogbf(float x) {
    union {float f; uint32_t i;} u = {x};
    uint32_t i = u.i;
    int e = i>>23 & 0xff;

    if (!e) {
        i <<= 9;
        if (i == 0) {
            FORCE_EVAL(0/0.0f);
            return FP_ILOGB0;
        }
        /* subnormal x */
        for (e = -0x7f; i>>31 == 0; e--, i<<=1);
        return e;
    }
    if (e == 0xff) {
        FORCE_EVAL(0/0.0f);
        return i<<9 ? FP_ILOGBNAN : INT_MAX;
    }
    return e - 0x7f;
}

/// >>> START src/math/ilogbl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
int ilogbl(long double x) {
    return ilogb(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
int ilogbl(long double x) {
    union ldshape u = {x};
    uint64_t m = u.i.m;
    int e = u.i.se & 0x7fff;

    if (!e) {
        if (m == 0) {
            FORCE_EVAL(0/0.0f);
            return FP_ILOGB0;
        }
        /* subnormal x */
        for (e = -0x3fff+1; m>>63 == 0; e--, m<<=1);
        return e;
    }
    if (e == 0x7fff) {
        FORCE_EVAL(0/0.0f);
        return m<<1 ? FP_ILOGBNAN : INT_MAX;
    }
    return e - 0x3fff;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
int ilogbl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;

    if (!e) {
        if (x == 0) {
            FORCE_EVAL(0/0.0f);
            return FP_ILOGB0;
        }
        /* subnormal x */
        x *= 0x1p120;
        return ilogbl(x) - 120;
    }
    if (e == 0x7fff) {
        FORCE_EVAL(0/0.0f);
        u.i.se = 0;
        return u.f ? FP_ILOGBNAN : INT_MAX;
    }
    return e - 0x3fff;
}
#endif

/// >>> START src/math/j0.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_j0.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* j0(x), y0(x)
 * Bessel function of the first and second kinds of order j0_c__zero.
 * Method -- j0(x):
 *      1. For j0_c__tiny x, we use j0(x) = 1 - x^2/4 + x^4/64 - ...
 *      2. Reduce x to |x| since j0(x)=j0(-x),  and
 *         for x in (0,2)
 *              j0(x) = 1-z/4+ z^2*R0/j0_c__S0,  where z = x*x;
 *         (precision:  |j0-1+z/4-z^2R0/j0_c__S0 |<2**-63.67 )
 *         for x in (2,inf)
 *              j0(x) = sqrt(2/(j0_c__pi*x))*(p0(x)*cos(x0)-q0(x)*sin(x0))
 *         where x0 = x-j0_c__pi/4. It is better to compute sin(x0),cos(x0)
 *         as follow:
 *              cos(x0) = cos(x)cos(j0_c__pi/4)+sin(x)sin(j0_c__pi/4)
 *                      = 1/sqrt(2) * (cos(x) + sin(x))
 *              sin(x0) = sin(x)cos(j0_c__pi/4)-cos(x)sin(j0_c__pi/4)
 *                      = 1/sqrt(2) * (sin(x) - cos(x))
 *         (To avoid cancellation, use
 *              sin(x) +- cos(x) = -cos(2x)/(sin(x) -+ cos(x))
 *          to compute the worse one.)
 *
 *      3 Special cases
 *              j0(nan)= nan
 *              j0(0) = 1
 *              j0(inf) = 0
 *
 * Method -- y0(x):
 *      1. For x<2.
 *         Since
 *              y0(x) = 2/j0_c__pi*(j0(x)*(ln(x/2)+Euler) + x^2/4 - ...)
 *         therefore y0(x)-2/j0_c__pi*j0(x)*ln(x) is an even function.
 *         We use the following function to approximate y0,
 *              y0(x) = U(z)/V(z) + (2/j0_c__pi)*(j0(x)*ln(x)), z= x^2
 *         where
 *              U(z) = j0_c__u00 + j0_c__u01*z + ... + j0_c__u06*z^6
 *              V(z) = 1  + j0_c__v01*z + ... + j0_c__v04*z^4
 *         with absolute approximation error bounded by 2**-72.
 *         Note: For j0_c__tiny x, U/V = j0_c__u0 and j0(x)~1, hence
 *              y0(j0_c__tiny) = j0_c__u0 + (2/j0_c__pi)*ln(j0_c__tiny), (choose j0_c__tiny<2**-27)
 *      2. For x>=2.
 *              y0(x) = sqrt(2/(j0_c__pi*x))*(p0(x)*cos(x0)+q0(x)*sin(x0))
 *         where x0 = x-j0_c__pi/4. It is better to compute sin(x0),cos(x0)
 *         by the method mentioned above.
 *      3. Special cases: y0(0)=-inf, y0(x<0)=NaN, y0(inf)=0.
 */

static double pzero(double), qzero(double);

static const double
j0_c__invsqrtpi = 5.64189583547756279280e-01, /* 0x3FE20DD7, 0x50429B6D */
j0_c__tpi       = 6.36619772367581382433e-01; /* 0x3FE45F30, 0x6DC9C883 */

/* j0_c__common method when |x|>=2 */
static double j0_c__common(uint32_t ix, double x, int y0) {
    double s,c,ss,cc,z;

    /*
     * j0(x) = sqrt(2/(j0_c__pi*x))*(p0(x)*cos(x-j0_c__pi/4)-q0(x)*sin(x-j0_c__pi/4))
     * y0(x) = sqrt(2/(j0_c__pi*x))*(p0(x)*sin(x-j0_c__pi/4)+q0(x)*cos(x-j0_c__pi/4))
     *
     * sin(x-j0_c__pi/4) = (sin(x) - cos(x))/sqrt(2)
     * cos(x-j0_c__pi/4) = (sin(x) + cos(x))/sqrt(2)
     * sin(x) +- cos(x) = -cos(2x)/(sin(x) -+ cos(x))
     */
    s = sin(x);
    c = cos(x);
    if (y0)
        c = -c;
    cc = s+c;
    /* avoid overflow in 2*x, big ulp error when x>=0x1p1023 */
    if (ix < 0x7fe00000) {
        ss = s-c;
        z = -cos(2*x);
        if (s*c < 0)
            cc = z/ss;
        else
            ss = z/cc;
        if (ix < 0x48000000) {
            if (y0)
                ss = -ss;
            cc = pzero(x)*cc-qzero(x)*ss;
        }
    }
    return j0_c__invsqrtpi*cc/sqrt(x);
}

/* R0/j0_c__S0 on [0, 2.00] */
static const double
j0_c__R02 =  1.56249999999999947958e-02, /* 0x3F8FFFFF, 0xFFFFFFFD */
j0_c__R03 = -1.89979294238854721751e-04, /* 0xBF28E6A5, 0xB61AC6E9 */
j0_c__R04 =  1.82954049532700665670e-06, /* 0x3EBEB1D1, 0x0C503919 */
j0_c__R05 = -4.61832688532103189199e-09, /* 0xBE33D5E7, 0x73D63FCE */
j0_c__S01 =  1.56191029464890010492e-02, /* 0x3F8FFCE8, 0x82C8C2A4 */
j0_c__S02 =  1.16926784663337450260e-04, /* 0x3F1EA6D2, 0xDD57DBF4 */
j0_c__S03 =  5.13546550207318111446e-07, /* 0x3EA13B54, 0xCE84D5A9 */
j0_c__S04 =  1.16614003333790000205e-09; /* 0x3E1408BC, 0xF4745D8F */

double j0(double x) {
    double z,r,s;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    /* j0(+-inf)=0, j0(nan)=nan */
    if (ix >= 0x7ff00000)
        return 1/(x*x);
    x = fabs(x);

    if (ix >= 0x40000000) {  /* |x| >= 2 */
        /* large ulp error near zeros: 2.4, 5.52, 8.6537,.. */
        return j0_c__common(ix,x,0);
    }

    /* 1 - x*x/4 + x*x*j0_c__R(x^2)/j0_c__S(x^2) */
    if (ix >= 0x3f200000) {  /* |x| >= 2**-13 */
        /* up to 4ulp error close to 2 */
        z = x*x;
        r = z*(j0_c__R02+z*(j0_c__R03+z*(j0_c__R04+z*j0_c__R05)));
        s = 1+z*(j0_c__S01+z*(j0_c__S02+z*(j0_c__S03+z*j0_c__S04)));
        return (1+x/2)*(1-x/2) + z*(r/s);
    }

    /* 1 - x*x/4 */
    /* prevent underflow */
    /* inexact should be raised when x!=0, this is not done correctly */
    if (ix >= 0x38000000)  /* |x| >= 2**-127 */
        x = 0.25*x*x;
    return 1 - x;
}

static const double
j0_c__u00  = -7.38042951086872317523e-02, /* 0xBFB2E4D6, 0x99CBD01F */
j0_c__u01  =  1.76666452509181115538e-01, /* 0x3FC69D01, 0x9DE9E3FC */
j0_c__u02  = -1.38185671945596898896e-02, /* 0xBF8C4CE8, 0xB16CFA97 */
j0_c__u03  =  3.47453432093683650238e-04, /* 0x3F36C54D, 0x20B29B6B */
j0_c__u04  = -3.81407053724364161125e-06, /* 0xBECFFEA7, 0x73D25CAD */
j0_c__u05  =  1.95590137035022920206e-08, /* 0x3E550057, 0x3B4EABD4 */
j0_c__u06  = -3.98205194132103398453e-11, /* 0xBDC5E43D, 0x693FB3C8 */
j0_c__v01  =  1.27304834834123699328e-02, /* 0x3F8A1270, 0x91C9C71A */
j0_c__v02  =  7.60068627350353253702e-05, /* 0x3F13ECBB, 0xF578C6C1 */
j0_c__v03  =  2.59150851840457805467e-07, /* 0x3E91642D, 0x7FF202FD */
j0_c__v04  =  4.41110311332675467403e-10; /* 0x3DFE5018, 0x3BD6D9EF */

double y0(double x) {
    double z,u,v;
    uint32_t ix,lx;

    EXTRACT_WORDS(ix, lx, x);

    /* y0(nan)=nan, y0(<0)=nan, y0(0)=-inf, y0(inf)=0 */
    if ((ix<<1 | lx) == 0)
        return -1/0.0;
    if (ix>>31)
        return 0/0.0;
    if (ix >= 0x7ff00000)
        return 1/x;

    if (ix >= 0x40000000) {  /* x >= 2 */
        /* large ulp errors near zeros: 3.958, 7.086,.. */
        return j0_c__common(ix,x,1);
    }

    /* U(x^2)/V(x^2) + (2/j0_c__pi)*j0(x)*log(x) */
    if (ix >= 0x3e400000) {  /* x >= 2**-27 */
        /* large ulp error near the first j0_c__zero, x ~= 0.89 */
        z = x*x;
        u = j0_c__u00+z*(j0_c__u01+z*(j0_c__u02+z*(j0_c__u03+z*(j0_c__u04+z*(j0_c__u05+z*j0_c__u06)))));
        v = 1.0+z*(j0_c__v01+z*(j0_c__v02+z*(j0_c__v03+z*j0_c__v04)));
        return u/v + j0_c__tpi*(j0(x)*log(x));
    }
    return j0_c__u00 + j0_c__tpi*log(x);
}

/* The asymptotic expansions of pzero is
 *      1 - 9/128 s^2 + 11025/98304 s^4 - ...,  where s = 1/x.
 * For x >= 2, We approximate pzero by
 *      pzero(x) = 1 + (j0_c__R/j0_c__S)
 * where  j0_c__R = j0_c__pR0 + j0_c__pR1*s^2 + j0_c__pR2*s^4 + ... + j0_c__pR5*s^10
 *        j0_c__S = 1 + j0_c__pS0*s^2 + ... + j0_c__pS4*s^10
 * and
 *      | pzero(x)-1-j0_c__R/j0_c__S | <= 2  ** ( -60.26)
 */
static const double j0_c__pR8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
 -7.03124999999900357484e-02, /* 0xBFB1FFFF, 0xFFFFFD32 */
 -8.08167041275349795626e+00, /* 0xC02029D0, 0xB44FA779 */
 -2.57063105679704847262e+02, /* 0xC0701102, 0x7B19E863 */
 -2.48521641009428822144e+03, /* 0xC0A36A6E, 0xCD4DCAFC */
 -5.25304380490729545272e+03, /* 0xC0B4850B, 0x36CC643D */
};
static const double j0_c__pS8[5] = {
  1.16534364619668181717e+02, /* 0x405D2233, 0x07A96751 */
  3.83374475364121826715e+03, /* 0x40ADF37D, 0x50596938 */
  4.05978572648472545552e+04, /* 0x40E3D2BB, 0x6EB6B05F */
  1.16752972564375915681e+05, /* 0x40FC810F, 0x8F9FA9BD */
  4.76277284146730962675e+04, /* 0x40E74177, 0x4F2C49DC */
};

static const double j0_c__pR5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
 -1.14125464691894502584e-11, /* 0xBDA918B1, 0x47E495CC */
 -7.03124940873599280078e-02, /* 0xBFB1FFFF, 0xE69AFBC6 */
 -4.15961064470587782438e+00, /* 0xC010A370, 0xF90C6BBF */
 -6.76747652265167261021e+01, /* 0xC050EB2F, 0x5A7D1783 */
 -3.31231299649172967747e+02, /* 0xC074B3B3, 0x6742CC63 */
 -3.46433388365604912451e+02, /* 0xC075A6EF, 0x28A38BD7 */
};
static const double j0_c__pS5[5] = {
  6.07539382692300335975e+01, /* 0x404E6081, 0x0C98C5DE */
  1.05125230595704579173e+03, /* 0x40906D02, 0x5C7E2864 */
  5.97897094333855784498e+03, /* 0x40B75AF8, 0x8FBE1D60 */
  9.62544514357774460223e+03, /* 0x40C2CCB8, 0xFA76FA38 */
  2.40605815922939109441e+03, /* 0x40A2CC1D, 0xC70BE864 */
};

static const double j0_c__pR3[6] = {/* for x in [4.547,2.8571]=1/[0.2199,0.35001] */
 -2.54704601771951915620e-09, /* 0xBE25E103, 0x6FE1AA86 */
 -7.03119616381481654654e-02, /* 0xBFB1FFF6, 0xF7C0E24B */
 -2.40903221549529611423e+00, /* 0xC00345B2, 0xAEA48074 */
 -2.19659774734883086467e+01, /* 0xC035F74A, 0x4CB94E14 */
 -5.80791704701737572236e+01, /* 0xC04D0A22, 0x420A1A45 */
 -3.14479470594888503854e+01, /* 0xC03F72AC, 0xA892D80F */
};
static const double j0_c__pS3[5] = {
  3.58560338055209726349e+01, /* 0x4041ED92, 0x84077DD3 */
  3.61513983050303863820e+02, /* 0x40769839, 0x464A7C0E */
  1.19360783792111533330e+03, /* 0x4092A66E, 0x6D1061D6 */
  1.12799679856907414432e+03, /* 0x40919FFC, 0xB8C39B7E */
  1.73580930813335754692e+02, /* 0x4065B296, 0xFC379081 */
};

static const double j0_c__pR2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
 -8.87534333032526411254e-08, /* 0xBE77D316, 0xE927026D */
 -7.03030995483624743247e-02, /* 0xBFB1FF62, 0x495E1E42 */
 -1.45073846780952986357e+00, /* 0xBFF73639, 0x8A24A843 */
 -7.63569613823527770791e+00, /* 0xC01E8AF3, 0xEDAFA7F3 */
 -1.11931668860356747786e+01, /* 0xC02662E6, 0xC5246303 */
 -3.23364579351335335033e+00, /* 0xC009DE81, 0xAF8FE70F */
};
static const double j0_c__pS2[5] = {
  2.22202997532088808441e+01, /* 0x40363865, 0x908B5959 */
  1.36206794218215208048e+02, /* 0x4061069E, 0x0EE8878F */
  2.70470278658083486789e+02, /* 0x4070E786, 0x42EA079B */
  1.53875394208320329881e+02, /* 0x40633C03, 0x3AB6FAFF */
  1.46576176948256193810e+01, /* 0x402D50B3, 0x44391809 */
};

static double pzero(double x) {
    const double *p,*q;
    double_t z,r,s;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x40200000){p = j0_c__pR8; q = j0_c__pS8;}
    else if (ix >= 0x40122E8B){p = j0_c__pR5; q = j0_c__pS5;}
    else if (ix >= 0x4006DB6D){p = j0_c__pR3; q = j0_c__pS3;}
    else /*ix >= 0x40000000*/ {p = j0_c__pR2; q = j0_c__pS2;}
    z = 1.0/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*q[4]))));
    return 1.0 + r/s;
}

/* For x >= 8, the asymptotic expansions of qzero is
 *      -1/8 s + 75/1024 s^3 - ..., where s = 1/x.
 * We approximate pzero by
 *      qzero(x) = s*(-1.25 + (j0_c__R/j0_c__S))
 * where  j0_c__R = j0_c__qR0 + j0_c__qR1*s^2 + j0_c__qR2*s^4 + ... + j0_c__qR5*s^10
 *        j0_c__S = 1 + j0_c__qS0*s^2 + ... + j0_c__qS5*s^12
 * and
 *      | qzero(x)/s +1.25-j0_c__R/j0_c__S | <= 2  ** ( -61.22)
 */
static const double j0_c__qR8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
  7.32421874999935051953e-02, /* 0x3FB2BFFF, 0xFFFFFE2C */
  1.17682064682252693899e+01, /* 0x40278952, 0x5BB334D6 */
  5.57673380256401856059e+02, /* 0x40816D63, 0x15301825 */
  8.85919720756468632317e+03, /* 0x40C14D99, 0x3E18F46D */
  3.70146267776887834771e+04, /* 0x40E212D4, 0x0E901566 */
};
static const double j0_c__qS8[6] = {
  1.63776026895689824414e+02, /* 0x406478D5, 0x365B39BC */
  8.09834494656449805916e+03, /* 0x40BFA258, 0x4E6B0563 */
  1.42538291419120476348e+05, /* 0x41016652, 0x54D38C3F */
  8.03309257119514397345e+05, /* 0x412883DA, 0x83A52B43 */
  8.40501579819060512818e+05, /* 0x4129A66B, 0x28DE0B3D */
 -3.43899293537866615225e+05, /* 0xC114FD6D, 0x2C9530C5 */
};

static const double j0_c__qR5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
  1.84085963594515531381e-11, /* 0x3DB43D8F, 0x29CC8CD9 */
  7.32421766612684765896e-02, /* 0x3FB2BFFF, 0xD172B04C */
  5.83563508962056953777e+00, /* 0x401757B0, 0xB9953DD3 */
  1.35111577286449829671e+02, /* 0x4060E392, 0x0A8788E9 */
  1.02724376596164097464e+03, /* 0x40900CF9, 0x9DC8C481 */
  1.98997785864605384631e+03, /* 0x409F17E9, 0x53C6E3A6 */
};
static const double j0_c__qS5[6] = {
  8.27766102236537761883e+01, /* 0x4054B1B3, 0xFB5E1543 */
  2.07781416421392987104e+03, /* 0x40A03BA0, 0xDA21C0CE */
  1.88472887785718085070e+04, /* 0x40D267D2, 0x7B591E6D */
  5.67511122894947329769e+04, /* 0x40EBB5E3, 0x97E02372 */
  3.59767538425114471465e+04, /* 0x40E19118, 0x1F7A54A0 */
 -5.35434275601944773371e+03, /* 0xC0B4EA57, 0xBEDBC609 */
};

static const double j0_c__qR3[6] = {/* for x in [4.547,2.8571]=1/[0.2199,0.35001] */
  4.37741014089738620906e-09, /* 0x3E32CD03, 0x6ADECB82 */
  7.32411180042911447163e-02, /* 0x3FB2BFEE, 0x0E8D0842 */
  3.34423137516170720929e+00, /* 0x400AC0FC, 0x61149CF5 */
  4.26218440745412650017e+01, /* 0x40454F98, 0x962DAEDD */
  1.70808091340565596283e+02, /* 0x406559DB, 0xE25EFD1F */
  1.66733948696651168575e+02, /* 0x4064D77C, 0x81FA21E0 */
};
static const double j0_c__qS3[6] = {
  4.87588729724587182091e+01, /* 0x40486122, 0xBFE343A6 */
  7.09689221056606015736e+02, /* 0x40862D83, 0x86544EB3 */
  3.70414822620111362994e+03, /* 0x40ACF04B, 0xE44DFC63 */
  6.46042516752568917582e+03, /* 0x40B93C6C, 0xD7C76A28 */
  2.51633368920368957333e+03, /* 0x40A3A8AA, 0xD94FB1C0 */
 -1.49247451836156386662e+02, /* 0xC062A7EB, 0x201CF40F */
};

static const double j0_c__qR2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
  1.50444444886983272379e-07, /* 0x3E84313B, 0x54F76BDB */
  7.32234265963079278272e-02, /* 0x3FB2BEC5, 0x3E883E34 */
  1.99819174093815998816e+00, /* 0x3FFFF897, 0xE727779C */
  1.44956029347885735348e+01, /* 0x402CFDBF, 0xAAF96FE5 */
  3.16662317504781540833e+01, /* 0x403FAA8E, 0x29FBDC4A */
  1.62527075710929267416e+01, /* 0x403040B1, 0x71814BB4 */
};
static const double j0_c__qS2[6] = {
  3.03655848355219184498e+01, /* 0x403E5D96, 0xF7C07AED */
  2.69348118608049844624e+02, /* 0x4070D591, 0xE4D14B40 */
  8.44783757595320139444e+02, /* 0x408A6645, 0x22B3BF22 */
  8.82935845112488550512e+02, /* 0x408B977C, 0x9C5CC214 */
  2.12666388511798828631e+02, /* 0x406A9553, 0x0E001365 */
 -5.31095493882666946917e+00, /* 0xC0153E6A, 0xF8B32931 */
};

static double qzero(double x) {
    const double *p,*q;
    double_t s,r,z;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x40200000){p = j0_c__qR8; q = j0_c__qS8;}
    else if (ix >= 0x40122E8B){p = j0_c__qR5; q = j0_c__qS5;}
    else if (ix >= 0x4006DB6D){p = j0_c__qR3; q = j0_c__qS3;}
    else /*ix >= 0x40000000*/ {p = j0_c__qR2; q = j0_c__qS2;}
    z = 1.0/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*(q[4]+z*q[5])))));
    return (-.125 + r/s)/x;
}

/// >>> START src/math/j0f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_j0f.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

static float pzerof(float), qzerof(float);

static const float
j0f_c__invsqrtpi = 5.6418961287e-01, /* 0x3f106ebb */
j0f_c__tpi       = 6.3661974669e-01; /* 0x3f22f983 */

static float j0f_c__common(uint32_t ix, float x, int y0) {
    float z,s,c,ss,cc;
    /*
     * j0(x) = 1/sqrt(j0f_c__pi) * (j0f_c__P(0,x)*cc - j0f_c__Q(0,x)*ss) / sqrt(x)
     * y0(x) = 1/sqrt(j0f_c__pi) * (j0f_c__P(0,x)*ss + j0f_c__Q(0,x)*cc) / sqrt(x)
     */
    s = sinf(x);
    c = cosf(x);
    if (y0)
        c = -c;
    cc = s+c;
    if (ix < 0x7f000000) {
        ss = s-c;
        z = -cosf(2*x);
        if (s*c < 0)
            cc = z/ss;
        else
            ss = z/cc;
        if (ix < 0x58800000) {
            if (y0)
                ss = -ss;
            cc = pzerof(x)*cc-qzerof(x)*ss;
        }
    }
    return j0f_c__invsqrtpi*cc/sqrtf(x);
}

/* R0/j0f_c__S0 on [0, 2.00] */
static const float
j0f_c__R02 =  1.5625000000e-02, /* 0x3c800000 */
j0f_c__R03 = -1.8997929874e-04, /* 0xb947352e */
j0f_c__R04 =  1.8295404516e-06, /* 0x35f58e88 */
j0f_c__R05 = -4.6183270541e-09, /* 0xb19eaf3c */
j0f_c__S01 =  1.5619102865e-02, /* 0x3c7fe744 */
j0f_c__S02 =  1.1692678527e-04, /* 0x38f53697 */
j0f_c__S03 =  5.1354652442e-07, /* 0x3509daa6 */
j0f_c__S04 =  1.1661400734e-09; /* 0x30a045e8 */

float j0f(float x) {
    float z,r,s;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;
    if (ix >= 0x7f800000)
        return 1/(x*x);
    x = fabsf(x);

    if (ix >= 0x40000000) {  /* |x| >= 2 */
        /* large ulp error near zeros */
        return j0f_c__common(ix, x, 0);
    }
    if (ix >= 0x3a000000) {  /* |x| >= 2**-11 */
        /* up to 4ulp error near 2 */
        z = x*x;
        r = z*(j0f_c__R02+z*(j0f_c__R03+z*(j0f_c__R04+z*j0f_c__R05)));
        s = 1+z*(j0f_c__S01+z*(j0f_c__S02+z*(j0f_c__S03+z*j0f_c__S04)));
        return (1+x/2)*(1-x/2) + z*(r/s);
    }
    if (ix >= 0x21800000)  /* |x| >= 2**-60 */
        x = 0.25f*x*x;
    return 1 - x;
}

static const float
j0f_c__u00  = -7.3804296553e-02, /* 0xbd9726b5 */
j0f_c__u01  =  1.7666645348e-01, /* 0x3e34e80d */
j0f_c__u02  = -1.3818567619e-02, /* 0xbc626746 */
j0f_c__u03  =  3.4745343146e-04, /* 0x39b62a69 */
j0f_c__u04  = -3.8140706238e-06, /* 0xb67ff53c */
j0f_c__u05  =  1.9559013964e-08, /* 0x32a802ba */
j0f_c__u06  = -3.9820518410e-11, /* 0xae2f21eb */
j0f_c__v01  =  1.2730483897e-02, /* 0x3c509385 */
j0f_c__v02  =  7.6006865129e-05, /* 0x389f65e0 */
j0f_c__v03  =  2.5915085189e-07, /* 0x348b216c */
j0f_c__v04  =  4.4111031494e-10; /* 0x2ff280c2 */

float y0f(float x) {
    float z,u,v;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    if ((ix & 0x7fffffff) == 0)
        return -1/0.0f;
    if (ix>>31)
        return 0/0.0f;
    if (ix >= 0x7f800000)
        return 1/x;
    if (ix >= 0x40000000) {  /* |x| >= 2.0 */
        /* large ulp error near zeros */
        return j0f_c__common(ix,x,1);
    }
    if (ix >= 0x39000000) {  /* x >= 2**-13 */
        /* large ulp error at x ~= 0.89 */
        z = x*x;
        u = j0f_c__u00+z*(j0f_c__u01+z*(j0f_c__u02+z*(j0f_c__u03+z*(j0f_c__u04+z*(j0f_c__u05+z*j0f_c__u06)))));
        v = 1+z*(j0f_c__v01+z*(j0f_c__v02+z*(j0f_c__v03+z*j0f_c__v04)));
        return u/v + j0f_c__tpi*(j0f(x)*logf(x));
    }
    return j0f_c__u00 + j0f_c__tpi*logf(x);
}

/* The asymptotic expansions of pzero is
 *      1 - 9/128 s^2 + 11025/98304 s^4 - ...,  where s = 1/x.
 * For x >= 2, We approximate pzero by
 *      pzero(x) = 1 + (j0f_c__R/j0f_c__S)
 * where  j0f_c__R = j0f_c__pR0 + j0f_c__pR1*s^2 + j0f_c__pR2*s^4 + ... + j0f_c__pR5*s^10
 *        j0f_c__S = 1 + j0f_c__pS0*s^2 + ... + j0f_c__pS4*s^10
 * and
 *      | pzero(x)-1-j0f_c__R/j0f_c__S | <= 2  ** ( -60.26)
 */
static const float j0f_c__pR8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.0000000000e+00, /* 0x00000000 */
 -7.0312500000e-02, /* 0xbd900000 */
 -8.0816707611e+00, /* 0xc1014e86 */
 -2.5706311035e+02, /* 0xc3808814 */
 -2.4852163086e+03, /* 0xc51b5376 */
 -5.2530439453e+03, /* 0xc5a4285a */
};
static const float j0f_c__pS8[5] = {
  1.1653436279e+02, /* 0x42e91198 */
  3.8337448730e+03, /* 0x456f9beb */
  4.0597855469e+04, /* 0x471e95db */
  1.1675296875e+05, /* 0x47e4087c */
  4.7627726562e+04, /* 0x473a0bba */
};
static const float j0f_c__pR5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
 -1.1412546255e-11, /* 0xad48c58a */
 -7.0312492549e-02, /* 0xbd8fffff */
 -4.1596107483e+00, /* 0xc0851b88 */
 -6.7674766541e+01, /* 0xc287597b */
 -3.3123129272e+02, /* 0xc3a59d9b */
 -3.4643338013e+02, /* 0xc3ad3779 */
};
static const float j0f_c__pS5[5] = {
  6.0753936768e+01, /* 0x42730408 */
  1.0512523193e+03, /* 0x44836813 */
  5.9789707031e+03, /* 0x45bad7c4 */
  9.6254453125e+03, /* 0x461665c8 */
  2.4060581055e+03, /* 0x451660ee */
};

static const float j0f_c__pR3[6] = {/* for x in [4.547,2.8571]=1/[0.2199,0.35001] */
 -2.5470459075e-09, /* 0xb12f081b */
 -7.0311963558e-02, /* 0xbd8fffb8 */
 -2.4090321064e+00, /* 0xc01a2d95 */
 -2.1965976715e+01, /* 0xc1afba52 */
 -5.8079170227e+01, /* 0xc2685112 */
 -3.1447946548e+01, /* 0xc1fb9565 */
};
static const float j0f_c__pS3[5] = {
  3.5856033325e+01, /* 0x420f6c94 */
  3.6151397705e+02, /* 0x43b4c1ca */
  1.1936077881e+03, /* 0x44953373 */
  1.1279968262e+03, /* 0x448cffe6 */
  1.7358093262e+02, /* 0x432d94b8 */
};

static const float j0f_c__pR2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
 -8.8753431271e-08, /* 0xb3be98b7 */
 -7.0303097367e-02, /* 0xbd8ffb12 */
 -1.4507384300e+00, /* 0xbfb9b1cc */
 -7.6356959343e+00, /* 0xc0f4579f */
 -1.1193166733e+01, /* 0xc1331736 */
 -3.2336456776e+00, /* 0xc04ef40d */
};
static const float j0f_c__pS2[5] = {
  2.2220300674e+01, /* 0x41b1c32d */
  1.3620678711e+02, /* 0x430834f0 */
  2.7047027588e+02, /* 0x43873c32 */
  1.5387539673e+02, /* 0x4319e01a */
  1.4657617569e+01, /* 0x416a859a */
};

static float pzerof(float x) {
    const float *p,*q;
    float_t z,r,s;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x41000000){p = j0f_c__pR8; q = j0f_c__pS8;}
    else if (ix >= 0x40f71c58){p = j0f_c__pR5; q = j0f_c__pS5;}
    else if (ix >= 0x4036db68){p = j0f_c__pR3; q = j0f_c__pS3;}
    else /*ix >= 0x40000000*/ {p = j0f_c__pR2; q = j0f_c__pS2;}
    z = 1.0f/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0f+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*q[4]))));
    return 1.0f + r/s;
}

/* For x >= 8, the asymptotic expansions of qzero is
 *      -1/8 s + 75/1024 s^3 - ..., where s = 1/x.
 * We approximate pzero by
 *      qzero(x) = s*(-1.25 + (j0f_c__R/j0f_c__S))
 * where  j0f_c__R = j0f_c__qR0 + j0f_c__qR1*s^2 + j0f_c__qR2*s^4 + ... + j0f_c__qR5*s^10
 *        j0f_c__S = 1 + j0f_c__qS0*s^2 + ... + j0f_c__qS5*s^12
 * and
 *      | qzero(x)/s +1.25-j0f_c__R/j0f_c__S | <= 2  ** ( -61.22)
 */
static const float j0f_c__qR8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.0000000000e+00, /* 0x00000000 */
  7.3242187500e-02, /* 0x3d960000 */
  1.1768206596e+01, /* 0x413c4a93 */
  5.5767340088e+02, /* 0x440b6b19 */
  8.8591972656e+03, /* 0x460a6cca */
  3.7014625000e+04, /* 0x471096a0 */
};
static const float j0f_c__qS8[6] = {
  1.6377603149e+02, /* 0x4323c6aa */
  8.0983447266e+03, /* 0x45fd12c2 */
  1.4253829688e+05, /* 0x480b3293 */
  8.0330925000e+05, /* 0x49441ed4 */
  8.4050156250e+05, /* 0x494d3359 */
 -3.4389928125e+05, /* 0xc8a7eb69 */
};

static const float j0f_c__qR5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
  1.8408595828e-11, /* 0x2da1ec79 */
  7.3242180049e-02, /* 0x3d95ffff */
  5.8356351852e+00, /* 0x40babd86 */
  1.3511157227e+02, /* 0x43071c90 */
  1.0272437744e+03, /* 0x448067cd */
  1.9899779053e+03, /* 0x44f8bf4b */
};
static const float j0f_c__qS5[6] = {
  8.2776611328e+01, /* 0x42a58da0 */
  2.0778142090e+03, /* 0x4501dd07 */
  1.8847289062e+04, /* 0x46933e94 */
  5.6751113281e+04, /* 0x475daf1d */
  3.5976753906e+04, /* 0x470c88c1 */
 -5.3543427734e+03, /* 0xc5a752be */
};

static const float j0f_c__qR3[6] = {/* for x in [4.547,2.8571]=1/[0.2199,0.35001] */
  4.3774099900e-09, /* 0x3196681b */
  7.3241114616e-02, /* 0x3d95ff70 */
  3.3442313671e+00, /* 0x405607e3 */
  4.2621845245e+01, /* 0x422a7cc5 */
  1.7080809021e+02, /* 0x432acedf */
  1.6673394775e+02, /* 0x4326bbe4 */
};
static const float j0f_c__qS3[6] = {
  4.8758872986e+01, /* 0x42430916 */
  7.0968920898e+02, /* 0x44316c1c */
  3.7041481934e+03, /* 0x4567825f */
  6.4604252930e+03, /* 0x45c9e367 */
  2.5163337402e+03, /* 0x451d4557 */
 -1.4924745178e+02, /* 0xc3153f59 */
};

static const float j0f_c__qR2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
  1.5044444979e-07, /* 0x342189db */
  7.3223426938e-02, /* 0x3d95f62a */
  1.9981917143e+00, /* 0x3fffc4bf */
  1.4495602608e+01, /* 0x4167edfd */
  3.1666231155e+01, /* 0x41fd5471 */
  1.6252708435e+01, /* 0x4182058c */
};
static const float j0f_c__qS2[6] = {
  3.0365585327e+01, /* 0x41f2ecb8 */
  2.6934811401e+02, /* 0x4386ac8f */
  8.4478375244e+02, /* 0x44533229 */
  8.8293585205e+02, /* 0x445cbbe5 */
  2.1266638184e+02, /* 0x4354aa98 */
 -5.3109550476e+00, /* 0xc0a9f358 */
};

static float qzerof(float x) {
    const float *p,*q;
    float_t s,r,z;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x41000000){p = j0f_c__qR8; q = j0f_c__qS8;}
    else if (ix >= 0x40f71c58){p = j0f_c__qR5; q = j0f_c__qS5;}
    else if (ix >= 0x4036db68){p = j0f_c__qR3; q = j0f_c__qS3;}
    else /*ix >= 0x40000000*/ {p = j0f_c__qR2; q = j0f_c__qS2;}
    z = 1.0f/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0f+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*(q[4]+z*q[5])))));
    return (-.125f + r/s)/x;
}

/// >>> START src/math/j1.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_j1.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* j1(x), y1(x)
 * Bessel function of the first and second kinds of order j1_c__zero.
 * Method -- j1(x):
 *      1. For j1_c__tiny x, we use j1(x) = x/2 - x^3/16 + x^5/384 - ...
 *      2. Reduce x to |x| since j1(x)=-j1(-x),  and
 *         for x in (0,2)
 *              j1(x) = x/2 + x*z*R0/j1_c__S0,  where z = x*x;
 *         (precision:  |j1/x - 1/2 - R0/j1_c__S0 |<2**-61.51 )
 *         for x in (2,inf)
 *              j1(x) = sqrt(2/(j1_c__pi*x))*(p1(x)*cos(x1)-q1(x)*sin(x1))
 *              y1(x) = sqrt(2/(j1_c__pi*x))*(p1(x)*sin(x1)+q1(x)*cos(x1))
 *         where x1 = x-3*j1_c__pi/4. It is better to compute sin(x1),cos(x1)
 *         as follow:
 *              cos(x1) =  cos(x)cos(3pi/4)+sin(x)sin(3pi/4)
 *                      =  1/sqrt(2) * (sin(x) - cos(x))
 *              sin(x1) =  sin(x)cos(3pi/4)-cos(x)sin(3pi/4)
 *                      = -1/sqrt(2) * (sin(x) + cos(x))
 *         (To avoid cancellation, use
 *              sin(x) +- cos(x) = -cos(2x)/(sin(x) -+ cos(x))
 *          to compute the worse one.)
 *
 *      3 Special cases
 *              j1(nan)= nan
 *              j1(0) = 0
 *              j1(inf) = 0
 *
 * Method -- y1(x):
 *      1. screen out x<=0 cases: y1(0)=-inf, y1(x<0)=NaN
 *      2. For x<2.
 *         Since
 *              y1(x) = 2/j1_c__pi*(j1(x)*(ln(x/2)+Euler)-1/x-x/2+5/64*x^3-...)
 *         therefore y1(x)-2/j1_c__pi*j1(x)*ln(x)-1/x is an odd function.
 *         We use the following function to approximate y1,
 *              y1(x) = x*U(z)/V(z) + (2/j1_c__pi)*(j1(x)*ln(x)-1/x), z= x^2
 *         where for x in [0,2] (abs err less than 2**-65.89)
 *              U(z) = j1_c__U0[0] + j1_c__U0[1]*z + ... + j1_c__U0[4]*z^4
 *              V(z) = 1  + j1_c__v0[0]*z + ... + j1_c__v0[4]*z^5
 *         Note: For j1_c__tiny x, 1/x dominate y1 and hence
 *              y1(j1_c__tiny) = -2/j1_c__pi/j1_c__tiny, (choose j1_c__tiny<2**-54)
 *      3. For x>=2.
 *              y1(x) = sqrt(2/(j1_c__pi*x))*(p1(x)*sin(x1)+q1(x)*cos(x1))
 *         where x1 = x-3*j1_c__pi/4. It is better to compute sin(x1),cos(x1)
 *         by method mentioned above.
 */

static double pone(double), qone(double);

static const double
j1_c__invsqrtpi = 5.64189583547756279280e-01, /* 0x3FE20DD7, 0x50429B6D */
j1_c__tpi       = 6.36619772367581382433e-01; /* 0x3FE45F30, 0x6DC9C883 */

static double j1_c__common(uint32_t ix, double x, int y1, int sign) {
    double z,s,c,ss,cc;

    /*
     * j1(x) = sqrt(2/(j1_c__pi*x))*(p1(x)*cos(x-3pi/4)-q1(x)*sin(x-3pi/4))
     * y1(x) = sqrt(2/(j1_c__pi*x))*(p1(x)*sin(x-3pi/4)+q1(x)*cos(x-3pi/4))
     *
     * sin(x-3pi/4) = -(sin(x) + cos(x))/sqrt(2)
     * cos(x-3pi/4) = (sin(x) - cos(x))/sqrt(2)
     * sin(x) +- cos(x) = -cos(2x)/(sin(x) -+ cos(x))
     */
    s = sin(x);
    if (y1)
        s = -s;
    c = cos(x);
    cc = s-c;
    if (ix < 0x7fe00000) {
        /* avoid overflow in 2*x */
        ss = -s-c;
        z = cos(2*x);
        if (s*c > 0)
            cc = z/ss;
        else
            ss = z/cc;
        if (ix < 0x48000000) {
            if (y1)
                ss = -ss;
            cc = pone(x)*cc-qone(x)*ss;
        }
    }
    if (sign)
        cc = -cc;
    return j1_c__invsqrtpi*cc/sqrt(x);
}

/* R0/j1_c__S0 on [0,2] */
static const double
j1_c__r00 = -6.25000000000000000000e-02, /* 0xBFB00000, 0x00000000 */
j1_c__r01 =  1.40705666955189706048e-03, /* 0x3F570D9F, 0x98472C61 */
j1_c__r02 = -1.59955631084035597520e-05, /* 0xBEF0C5C6, 0xBA169668 */
j1_c__r03 =  4.96727999609584448412e-08, /* 0x3E6AAAFA, 0x46CA0BD9 */
j1_c__s01 =  1.91537599538363460805e-02, /* 0x3F939D0B, 0x12637E53 */
j1_c__s02 =  1.85946785588630915560e-04, /* 0x3F285F56, 0xB9CDF664 */
j1_c__s03 =  1.17718464042623683263e-06, /* 0x3EB3BFF8, 0x333F8498 */
j1_c__s04 =  5.04636257076217042715e-09, /* 0x3E35AC88, 0xC97DFF2C */
j1_c__s05 =  1.23542274426137913908e-11; /* 0x3DAB2ACF, 0xCFB97ED8 */

double j1(double x) {
    double z,r,s;
    uint32_t ix;
    int sign;

    GET_HIGH_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7ff00000)
        return 1/(x*x);
    if (ix >= 0x40000000)  /* |x| >= 2 */
        return j1_c__common(ix, fabs(x), 0, sign);
    if (ix >= 0x38000000) {  /* |x| >= 2**-127 */
        z = x*x;
        r = z*(j1_c__r00+z*(j1_c__r01+z*(j1_c__r02+z*j1_c__r03)));
        s = 1+z*(j1_c__s01+z*(j1_c__s02+z*(j1_c__s03+z*(j1_c__s04+z*j1_c__s05))));
        z = r/s;
    } else
        /* avoid underflow, raise inexact if x!=0 */
        z = x;
    return (0.5 + z)*x;
}

static const double j1_c__U0[5] = {
 -1.96057090646238940668e-01, /* 0xBFC91866, 0x143CBC8A */
  5.04438716639811282616e-02, /* 0x3FA9D3C7, 0x76292CD1 */
 -1.91256895875763547298e-03, /* 0xBF5F55E5, 0x4844F50F */
  2.35252600561610495928e-05, /* 0x3EF8AB03, 0x8FA6B88E */
 -9.19099158039878874504e-08, /* 0xBE78AC00, 0x569105B8 */
};
static const double j1_c__V0[5] = {
  1.99167318236649903973e-02, /* 0x3F94650D, 0x3F4DA9F0 */
  2.02552581025135171496e-04, /* 0x3F2A8C89, 0x6C257764 */
  1.35608801097516229404e-06, /* 0x3EB6C05A, 0x894E8CA6 */
  6.22741452364621501295e-09, /* 0x3E3ABF1D, 0x5BA69A86 */
  1.66559246207992079114e-11, /* 0x3DB25039, 0xDACA772A */
};

double y1(double x) {
    double z,u,v;
    uint32_t ix,lx;

    EXTRACT_WORDS(ix, lx, x);
    /* y1(nan)=nan, y1(<0)=nan, y1(0)=-inf, y1(inf)=0 */
    if ((ix<<1 | lx) == 0)
        return -1/0.0;
    if (ix>>31)
        return 0/0.0;
    if (ix >= 0x7ff00000)
        return 1/x;

    if (ix >= 0x40000000)  /* x >= 2 */
        return j1_c__common(ix, x, 1, 0);
    if (ix < 0x3c900000)  /* x < 2**-54 */
        return -j1_c__tpi/x;
    z = x*x;
    u = j1_c__U0[0]+z*(j1_c__U0[1]+z*(j1_c__U0[2]+z*(j1_c__U0[3]+z*j1_c__U0[4])));
    v = 1+z*(j1_c__V0[0]+z*(j1_c__V0[1]+z*(j1_c__V0[2]+z*(j1_c__V0[3]+z*j1_c__V0[4]))));
    return x*(u/v) + j1_c__tpi*(j1(x)*log(x)-1/x);
}

/* For x >= 8, the asymptotic expansions of pone is
 *      1 + 15/128 s^2 - 4725/2^15 s^4 - ...,   where s = 1/x.
 * We approximate pone by
 *      pone(x) = 1 + (j1_c__R/j1_c__S)
 * where  j1_c__R = j1_c__pr0 + j1_c__pr1*s^2 + j1_c__pr2*s^4 + ... + j1_c__pr5*s^10
 *        j1_c__S = 1 + j1_c__ps0*s^2 + ... + j1_c__ps4*s^10
 * and
 *      | pone(x)-1-j1_c__R/j1_c__S | <= 2  ** ( -60.06)
 */

static const double j1_c__pr8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
  1.17187499999988647970e-01, /* 0x3FBDFFFF, 0xFFFFFCCE */
  1.32394806593073575129e+01, /* 0x402A7A9D, 0x357F7FCE */
  4.12051854307378562225e+02, /* 0x4079C0D4, 0x652EA590 */
  3.87474538913960532227e+03, /* 0x40AE457D, 0xA3A532CC */
  7.91447954031891731574e+03, /* 0x40BEEA7A, 0xC32782DD */
};
static const double j1_c__ps8[5] = {
  1.14207370375678408436e+02, /* 0x405C8D45, 0x8E656CAC */
  3.65093083420853463394e+03, /* 0x40AC85DC, 0x964D274F */
  3.69562060269033463555e+04, /* 0x40E20B86, 0x97C5BB7F */
  9.76027935934950801311e+04, /* 0x40F7D42C, 0xB28F17BB */
  3.08042720627888811578e+04, /* 0x40DE1511, 0x697A0B2D */
};

static const double j1_c__pr5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
  1.31990519556243522749e-11, /* 0x3DAD0667, 0xDAE1CA7D */
  1.17187493190614097638e-01, /* 0x3FBDFFFF, 0xE2C10043 */
  6.80275127868432871736e+00, /* 0x401B3604, 0x6E6315E3 */
  1.08308182990189109773e+02, /* 0x405B13B9, 0x452602ED */
  5.17636139533199752805e+02, /* 0x40802D16, 0xD052D649 */
  5.28715201363337541807e+02, /* 0x408085B8, 0xBB7E0CB7 */
};
static const double j1_c__ps5[5] = {
  5.92805987221131331921e+01, /* 0x404DA3EA, 0xA8AF633D */
  9.91401418733614377743e+02, /* 0x408EFB36, 0x1B066701 */
  5.35326695291487976647e+03, /* 0x40B4E944, 0x5706B6FB */
  7.84469031749551231769e+03, /* 0x40BEA4B0, 0xB8A5BB15 */
  1.50404688810361062679e+03, /* 0x40978030, 0x036F5E51 */
};

static const double j1_c__pr3[6] = {
  3.02503916137373618024e-09, /* 0x3E29FC21, 0xA7AD9EDD */
  1.17186865567253592491e-01, /* 0x3FBDFFF5, 0x5B21D17B */
  3.93297750033315640650e+00, /* 0x400F76BC, 0xE85EAD8A */
  3.51194035591636932736e+01, /* 0x40418F48, 0x9DA6D129 */
  9.10550110750781271918e+01, /* 0x4056C385, 0x4D2C1837 */
  4.85590685197364919645e+01, /* 0x4048478F, 0x8EA83EE5 */
};
static const double j1_c__ps3[5] = {
  3.47913095001251519989e+01, /* 0x40416549, 0xA134069C */
  3.36762458747825746741e+02, /* 0x40750C33, 0x07F1A75F */
  1.04687139975775130551e+03, /* 0x40905B7C, 0x5037D523 */
  8.90811346398256432622e+02, /* 0x408BD67D, 0xA32E31E9 */
  1.03787932439639277504e+02, /* 0x4059F26D, 0x7C2EED53 */
};

static const double j1_c__pr2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
  1.07710830106873743082e-07, /* 0x3E7CE9D4, 0xF65544F4 */
  1.17176219462683348094e-01, /* 0x3FBDFF42, 0xBE760D83 */
  2.36851496667608785174e+00, /* 0x4002F2B7, 0xF98FAEC0 */
  1.22426109148261232917e+01, /* 0x40287C37, 0x7F71A964 */
  1.76939711271687727390e+01, /* 0x4031B1A8, 0x177F8EE2 */
  5.07352312588818499250e+00, /* 0x40144B49, 0xA574C1FE */
};
static const double j1_c__ps2[5] = {
  2.14364859363821409488e+01, /* 0x40356FBD, 0x8AD5ECDC */
  1.25290227168402751090e+02, /* 0x405F5293, 0x14F92CD5 */
  2.32276469057162813669e+02, /* 0x406D08D8, 0xD5A2DBD9 */
  1.17679373287147100768e+02, /* 0x405D6B7A, 0xDA1884A9 */
  8.36463893371618283368e+00, /* 0x4020BAB1, 0xF44E5192 */
};

static double pone(double x) {
    const double *p,*q;
    double_t z,r,s;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x40200000){p = j1_c__pr8; q = j1_c__ps8;}
    else if (ix >= 0x40122E8B){p = j1_c__pr5; q = j1_c__ps5;}
    else if (ix >= 0x4006DB6D){p = j1_c__pr3; q = j1_c__ps3;}
    else /*ix >= 0x40000000*/ {p = j1_c__pr2; q = j1_c__ps2;}
    z = 1.0/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*q[4]))));
    return 1.0+ r/s;
}

/* For x >= 8, the asymptotic expansions of qone is
 *      3/8 s - 105/1024 s^3 - ..., where s = 1/x.
 * We approximate pone by
 *      qone(x) = s*(0.375 + (j1_c__R/j1_c__S))
 * where  j1_c__R = j1_c__qr1*s^2 + j1_c__qr2*s^4 + ... + j1_c__qr5*s^10
 *        j1_c__S = 1 + j1_c__qs1*s^2 + ... + j1_c__qs6*s^12
 * and
 *      | qone(x)/s -0.375-j1_c__R/j1_c__S | <= 2  ** ( -61.13)
 */

static const double j1_c__qr8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
 -1.02539062499992714161e-01, /* 0xBFBA3FFF, 0xFFFFFDF3 */
 -1.62717534544589987888e+01, /* 0xC0304591, 0xA26779F7 */
 -7.59601722513950107896e+02, /* 0xC087BCD0, 0x53E4B576 */
 -1.18498066702429587167e+04, /* 0xC0C724E7, 0x40F87415 */
 -4.84385124285750353010e+04, /* 0xC0E7A6D0, 0x65D09C6A */
};
static const double j1_c__qs8[6] = {
  1.61395369700722909556e+02, /* 0x40642CA6, 0xDE5BCDE5 */
  7.82538599923348465381e+03, /* 0x40BE9162, 0xD0D88419 */
  1.33875336287249578163e+05, /* 0x4100579A, 0xB0B75E98 */
  7.19657723683240939863e+05, /* 0x4125F653, 0x72869C19 */
  6.66601232617776375264e+05, /* 0x412457D2, 0x7719AD5C */
 -2.94490264303834643215e+05, /* 0xC111F969, 0x0EA5AA18 */
};

static const double j1_c__qr5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
 -2.08979931141764104297e-11, /* 0xBDB6FA43, 0x1AA1A098 */
 -1.02539050241375426231e-01, /* 0xBFBA3FFF, 0xCB597FEF */
 -8.05644828123936029840e+00, /* 0xC0201CE6, 0xCA03AD4B */
 -1.83669607474888380239e+02, /* 0xC066F56D, 0x6CA7B9B0 */
 -1.37319376065508163265e+03, /* 0xC09574C6, 0x6931734F */
 -2.61244440453215656817e+03, /* 0xC0A468E3, 0x88FDA79D */
};
static const double j1_c__qs5[6] = {
  8.12765501384335777857e+01, /* 0x405451B2, 0xFF5A11B2 */
  1.99179873460485964642e+03, /* 0x409F1F31, 0xE77BF839 */
  1.74684851924908907677e+04, /* 0x40D10F1F, 0x0D64CE29 */
  4.98514270910352279316e+04, /* 0x40E8576D, 0xAABAD197 */
  2.79480751638918118260e+04, /* 0x40DB4B04, 0xCF7C364B */
 -4.71918354795128470869e+03, /* 0xC0B26F2E, 0xFCFFA004 */
};

static const double j1_c__qr3[6] = {
 -5.07831226461766561369e-09, /* 0xBE35CFA9, 0xD38FC84F */
 -1.02537829820837089745e-01, /* 0xBFBA3FEB, 0x51AEED54 */
 -4.61011581139473403113e+00, /* 0xC01270C2, 0x3302D9FF */
 -5.78472216562783643212e+01, /* 0xC04CEC71, 0xC25D16DA */
 -2.28244540737631695038e+02, /* 0xC06C87D3, 0x4718D55F */
 -2.19210128478909325622e+02, /* 0xC06B66B9, 0x5F5C1BF6 */
};
static const double j1_c__qs3[6] = {
  4.76651550323729509273e+01, /* 0x4047D523, 0xCCD367E4 */
  6.73865112676699709482e+02, /* 0x40850EEB, 0xC031EE3E */
  3.38015286679526343505e+03, /* 0x40AA684E, 0x448E7C9A */
  5.54772909720722782367e+03, /* 0x40B5ABBA, 0xA61D54A6 */
  1.90311919338810798763e+03, /* 0x409DBC7A, 0x0DD4DF4B */
 -1.35201191444307340817e+02, /* 0xC060E670, 0x290A311F */
};

static const double j1_c__qr2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
 -1.78381727510958865572e-07, /* 0xBE87F126, 0x44C626D2 */
 -1.02517042607985553460e-01, /* 0xBFBA3E8E, 0x9148B010 */
 -2.75220568278187460720e+00, /* 0xC0060484, 0x69BB4EDA */
 -1.96636162643703720221e+01, /* 0xC033A9E2, 0xC168907F */
 -4.23253133372830490089e+01, /* 0xC04529A3, 0xDE104AAA */
 -2.13719211703704061733e+01, /* 0xC0355F36, 0x39CF6E52 */
};
static const double j1_c__qs2[6] = {
  2.95333629060523854548e+01, /* 0x403D888A, 0x78AE64FF */
  2.52981549982190529136e+02, /* 0x406F9F68, 0xDB821CBA */
  7.57502834868645436472e+02, /* 0x4087AC05, 0xCE49A0F7 */
  7.39393205320467245656e+02, /* 0x40871B25, 0x48D4C029 */
  1.55949003336666123687e+02, /* 0x40637E5E, 0x3C3ED8D4 */
 -4.95949898822628210127e+00, /* 0xC013D686, 0xE71BE86B */
};

static double qone(double x) {
    const double *p,*q;
    double_t s,r,z;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x40200000){p = j1_c__qr8; q = j1_c__qs8;}
    else if (ix >= 0x40122E8B){p = j1_c__qr5; q = j1_c__qs5;}
    else if (ix >= 0x4006DB6D){p = j1_c__qr3; q = j1_c__qs3;}
    else /*ix >= 0x40000000*/ {p = j1_c__qr2; q = j1_c__qs2;}
    z = 1.0/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*(q[4]+z*q[5])))));
    return (.375 + r/s)/x;
}

/// >>> START src/math/j1f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_j1f.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

static float ponef(float), qonef(float);

static const float
j1f_c__invsqrtpi = 5.6418961287e-01, /* 0x3f106ebb */
j1f_c__tpi       = 6.3661974669e-01; /* 0x3f22f983 */

static float j1f_c__common(uint32_t ix, float x, int y1, int sign) {
    double z,s,c,ss,cc;

    s = sinf(x);
    if (y1)
        s = -s;
    c = cosf(x);
    cc = s-c;
    if (ix < 0x7f000000) {
        ss = -s-c;
        z = cosf(2*x);
        if (s*c > 0)
            cc = z/ss;
        else
            ss = z/cc;
        if (ix < 0x58800000) {
            if (y1)
                ss = -ss;
            cc = ponef(x)*cc-qonef(x)*ss;
        }
    }
    if (sign)
        cc = -cc;
    return j1f_c__invsqrtpi*cc/sqrtf(x);
}

/* R0/j1f_c__S0 on [0,2] */
static const float
j1f_c__r00 = -6.2500000000e-02, /* 0xbd800000 */
j1f_c__r01 =  1.4070566976e-03, /* 0x3ab86cfd */
j1f_c__r02 = -1.5995563444e-05, /* 0xb7862e36 */
j1f_c__r03 =  4.9672799207e-08, /* 0x335557d2 */
j1f_c__s01 =  1.9153760746e-02, /* 0x3c9ce859 */
j1f_c__s02 =  1.8594678841e-04, /* 0x3942fab6 */
j1f_c__s03 =  1.1771846857e-06, /* 0x359dffc2 */
j1f_c__s04 =  5.0463624390e-09, /* 0x31ad6446 */
j1f_c__s05 =  1.2354227016e-11; /* 0x2d59567e */

float j1f(float x) {
    float z,r,s;
    uint32_t ix;
    int sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix >= 0x7f800000)
        return 1/(x*x);
    if (ix >= 0x40000000)  /* |x| >= 2 */
        return j1f_c__common(ix, fabsf(x), 0, sign);
    if (ix >= 0x32000000) {  /* |x| >= 2**-27 */
        z = x*x;
        r = z*(j1f_c__r00+z*(j1f_c__r01+z*(j1f_c__r02+z*j1f_c__r03)));
        s = 1+z*(j1f_c__s01+z*(j1f_c__s02+z*(j1f_c__s03+z*(j1f_c__s04+z*j1f_c__s05))));
        z = 0.5f + r/s;
    } else
        /* raise inexact if x!=0 */
        z = 0.5f + x;
    return z*x;
}

static const float j1f_c__U0[5] = {
 -1.9605709612e-01, /* 0xbe48c331 */
  5.0443872809e-02, /* 0x3d4e9e3c */
 -1.9125689287e-03, /* 0xbafaaf2a */
  2.3525259166e-05, /* 0x37c5581c */
 -9.1909917899e-08, /* 0xb3c56003 */
};
static const float j1f_c__V0[5] = {
  1.9916731864e-02, /* 0x3ca3286a */
  2.0255257550e-04, /* 0x3954644b */
  1.3560879779e-06, /* 0x35b602d4 */
  6.2274145840e-09, /* 0x31d5f8eb */
  1.6655924903e-11, /* 0x2d9281cf */
};

float y1f(float x) {
    float z,u,v;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    if ((ix & 0x7fffffff) == 0)
        return -1/0.0f;
    if (ix>>31)
        return 0/0.0f;
    if (ix >= 0x7f800000)
        return 1/x;
    if (ix >= 0x40000000)  /* |x| >= 2.0 */
        return j1f_c__common(ix,x,1,0);
    if (ix < 0x32000000)  /* x < 2**-27 */
        return -j1f_c__tpi/x;
    z = x*x;
    u = j1f_c__U0[0]+z*(j1f_c__U0[1]+z*(j1f_c__U0[2]+z*(j1f_c__U0[3]+z*j1f_c__U0[4])));
    v = 1.0f+z*(j1f_c__V0[0]+z*(j1f_c__V0[1]+z*(j1f_c__V0[2]+z*(j1f_c__V0[3]+z*j1f_c__V0[4]))));
    return x*(u/v) + j1f_c__tpi*(j1f(x)*logf(x)-1.0f/x);
}

/* For x >= 8, the asymptotic expansions of pone is
 *      1 + 15/128 s^2 - 4725/2^15 s^4 - ...,   where s = 1/x.
 * We approximate pone by
 *      pone(x) = 1 + (j1f_c__R/j1f_c__S)
 * where  j1f_c__R = j1f_c__pr0 + j1f_c__pr1*s^2 + j1f_c__pr2*s^4 + ... + j1f_c__pr5*s^10
 *        j1f_c__S = 1 + j1f_c__ps0*s^2 + ... + j1f_c__ps4*s^10
 * and
 *      | pone(x)-1-j1f_c__R/j1f_c__S | <= 2  ** ( -60.06)
 */

static const float j1f_c__pr8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.0000000000e+00, /* 0x00000000 */
  1.1718750000e-01, /* 0x3df00000 */
  1.3239480972e+01, /* 0x4153d4ea */
  4.1205184937e+02, /* 0x43ce06a3 */
  3.8747453613e+03, /* 0x45722bed */
  7.9144794922e+03, /* 0x45f753d6 */
};
static const float j1f_c__ps8[5] = {
  1.1420736694e+02, /* 0x42e46a2c */
  3.6509309082e+03, /* 0x45642ee5 */
  3.6956207031e+04, /* 0x47105c35 */
  9.7602796875e+04, /* 0x47bea166 */
  3.0804271484e+04, /* 0x46f0a88b */
};

static const float j1f_c__pr5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
  1.3199052094e-11, /* 0x2d68333f */
  1.1718749255e-01, /* 0x3defffff */
  6.8027510643e+00, /* 0x40d9b023 */
  1.0830818176e+02, /* 0x42d89dca */
  5.1763616943e+02, /* 0x440168b7 */
  5.2871520996e+02, /* 0x44042dc6 */
};
static const float j1f_c__ps5[5] = {
  5.9280597687e+01, /* 0x426d1f55 */
  9.9140142822e+02, /* 0x4477d9b1 */
  5.3532670898e+03, /* 0x45a74a23 */
  7.8446904297e+03, /* 0x45f52586 */
  1.5040468750e+03, /* 0x44bc0180 */
};

static const float j1f_c__pr3[6] = {
  3.0250391081e-09, /* 0x314fe10d */
  1.1718686670e-01, /* 0x3defffab */
  3.9329774380e+00, /* 0x407bb5e7 */
  3.5119403839e+01, /* 0x420c7a45 */
  9.1055007935e+01, /* 0x42b61c2a */
  4.8559066772e+01, /* 0x42423c7c */
};
static const float j1f_c__ps3[5] = {
  3.4791309357e+01, /* 0x420b2a4d */
  3.3676245117e+02, /* 0x43a86198 */
  1.0468714600e+03, /* 0x4482dbe3 */
  8.9081134033e+02, /* 0x445eb3ed */
  1.0378793335e+02, /* 0x42cf936c */
};

static const float j1f_c__pr2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
  1.0771083225e-07, /* 0x33e74ea8 */
  1.1717621982e-01, /* 0x3deffa16 */
  2.3685150146e+00, /* 0x401795c0 */
  1.2242610931e+01, /* 0x4143e1bc */
  1.7693971634e+01, /* 0x418d8d41 */
  5.0735230446e+00, /* 0x40a25a4d */
};
static const float j1f_c__ps2[5] = {
  2.1436485291e+01, /* 0x41ab7dec */
  1.2529022980e+02, /* 0x42fa9499 */
  2.3227647400e+02, /* 0x436846c7 */
  1.1767937469e+02, /* 0x42eb5bd7 */
  8.3646392822e+00, /* 0x4105d590 */
};

static float ponef(float x) {
    const float *p,*q;
    float_t z,r,s;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x41000000){p = j1f_c__pr8; q = j1f_c__ps8;}
    else if (ix >= 0x40f71c58){p = j1f_c__pr5; q = j1f_c__ps5;}
    else if (ix >= 0x4036db68){p = j1f_c__pr3; q = j1f_c__ps3;}
    else /*ix >= 0x40000000*/ {p = j1f_c__pr2; q = j1f_c__ps2;}
    z = 1.0f/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0f+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*q[4]))));
    return 1.0f + r/s;
}

/* For x >= 8, the asymptotic expansions of qone is
 *      3/8 s - 105/1024 s^3 - ..., where s = 1/x.
 * We approximate pone by
 *      qone(x) = s*(0.375 + (j1f_c__R/j1f_c__S))
 * where  j1f_c__R = j1f_c__qr1*s^2 + j1f_c__qr2*s^4 + ... + j1f_c__qr5*s^10
 *        j1f_c__S = 1 + j1f_c__qs1*s^2 + ... + j1f_c__qs6*s^12
 * and
 *      | qone(x)/s -0.375-j1f_c__R/j1f_c__S | <= 2  ** ( -61.13)
 */

static const float j1f_c__qr8[6] = { /* for x in [inf, 8]=1/[0,0.125] */
  0.0000000000e+00, /* 0x00000000 */
 -1.0253906250e-01, /* 0xbdd20000 */
 -1.6271753311e+01, /* 0xc1822c8d */
 -7.5960174561e+02, /* 0xc43de683 */
 -1.1849806641e+04, /* 0xc639273a */
 -4.8438511719e+04, /* 0xc73d3683 */
};
static const float j1f_c__qs8[6] = {
  1.6139537048e+02, /* 0x43216537 */
  7.8253862305e+03, /* 0x45f48b17 */
  1.3387534375e+05, /* 0x4802bcd6 */
  7.1965775000e+05, /* 0x492fb29c */
  6.6660125000e+05, /* 0x4922be94 */
 -2.9449025000e+05, /* 0xc88fcb48 */
};

static const float j1f_c__qr5[6] = { /* for x in [8,4.5454]=1/[0.125,0.22001] */
 -2.0897993405e-11, /* 0xadb7d219 */
 -1.0253904760e-01, /* 0xbdd1fffe */
 -8.0564479828e+00, /* 0xc100e736 */
 -1.8366960144e+02, /* 0xc337ab6b */
 -1.3731937256e+03, /* 0xc4aba633 */
 -2.6124443359e+03, /* 0xc523471c */
};
static const float j1f_c__qs5[6] = {
  8.1276550293e+01, /* 0x42a28d98 */
  1.9917987061e+03, /* 0x44f8f98f */
  1.7468484375e+04, /* 0x468878f8 */
  4.9851425781e+04, /* 0x4742bb6d */
  2.7948074219e+04, /* 0x46da5826 */
 -4.7191835938e+03, /* 0xc5937978 */
};

static const float j1f_c__qr3[6] = {
 -5.0783124372e-09, /* 0xb1ae7d4f */
 -1.0253783315e-01, /* 0xbdd1ff5b */
 -4.6101160049e+00, /* 0xc0938612 */
 -5.7847221375e+01, /* 0xc267638e */
 -2.2824453735e+02, /* 0xc3643e9a */
 -2.1921012878e+02, /* 0xc35b35cb */
};
static const float j1f_c__qs3[6] = {
  4.7665153503e+01, /* 0x423ea91e */
  6.7386511230e+02, /* 0x4428775e */
  3.3801528320e+03, /* 0x45534272 */
  5.5477290039e+03, /* 0x45ad5dd5 */
  1.9031191406e+03, /* 0x44ede3d0 */
 -1.3520118713e+02, /* 0xc3073381 */
};

static const float j1f_c__qr2[6] = {/* for x in [2.8570,2]=1/[0.3499,0.5] */
 -1.7838172539e-07, /* 0xb43f8932 */
 -1.0251704603e-01, /* 0xbdd1f475 */
 -2.7522056103e+00, /* 0xc0302423 */
 -1.9663616180e+01, /* 0xc19d4f16 */
 -4.2325313568e+01, /* 0xc2294d1f */
 -2.1371921539e+01, /* 0xc1aaf9b2 */
};
static const float j1f_c__qs2[6] = {
  2.9533363342e+01, /* 0x41ec4454 */
  2.5298155212e+02, /* 0x437cfb47 */
  7.5750280762e+02, /* 0x443d602e */
  7.3939318848e+02, /* 0x4438d92a */
  1.5594900513e+02, /* 0x431bf2f2 */
 -4.9594988823e+00, /* 0xc09eb437 */
};

static float qonef(float x) {
    const float *p,*q;
    float_t s,r,z;
    uint32_t ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;
    if      (ix >= 0x40200000){p = j1f_c__qr8; q = j1f_c__qs8;}
    else if (ix >= 0x40f71c58){p = j1f_c__qr5; q = j1f_c__qs5;}
    else if (ix >= 0x4036db68){p = j1f_c__qr3; q = j1f_c__qs3;}
    else /*ix >= 0x40000000*/ {p = j1f_c__qr2; q = j1f_c__qs2;}
    z = 1.0f/(x*x);
    r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
    s = 1.0f+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*(q[4]+z*q[5])))));
    return (.375f + r/s)/x;
}

/// >>> START src/math/jn.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_jn.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * jn(n, x), yn(n, x)
 * floating point Bessel's function of the 1st and 2nd kind
 * of order n
 *
 * Special cases:
 *      y0(0)=y1(0)=yn(n,0) = -inf with division by jn_c__zero signal;
 *      y0(-ve)=y1(-ve)=yn(n,-ve) are NaN with invalid signal.
 * Note 2. About jn(n,x), yn(n,x)
 *      For n=0, j0(x) is called,
 *      for n=1, j1(x) is called,
 *      for n<=x, forward recursion is used starting
 *      from values of j0(x) and j1(x).
 *      for n>x, a continued fraction approximation to
 *      j(n,x)/j(n-1,x) is evaluated and then backward
 *      recursion is used starting from a supposed value
 *      for j(n,x). The resulting value of j(0,x) is
 *      compared with the actual value to correct the
 *      supposed value of j(n,x).
 *
 *      yn(n,x) is similar in all respects, except
 *      that forward recursion is used for all
 *      values of n>1.
 */

static const double jn_c__invsqrtpi = 5.64189583547756279280e-01; /* 0x3FE20DD7, 0x50429B6D */

double jn(int n, double x) {
    uint32_t ix, lx;
    int nm1, i, sign;
    double a, b, temp;

    EXTRACT_WORDS(ix, lx, x);
    sign = ix>>31;
    ix &= 0x7fffffff;

    if ((ix | (lx|-lx)>>31) > 0x7ff00000) /* nan */
        return x;

    /* J(-n,x) = (-1)^n * J(n, x), J(n, -x) = (-1)^n * J(n, x)
     * Thus, J(-n,x) = J(n,-x)
     */
    /* nm1 = |n|-1 is used instead of |n| to handle n==INT_MIN */
    if (n == 0)
        return j0(x);
    if (n < 0) {
        nm1 = -(n+1);
        x = -x;
        sign ^= 1;
    } else
        nm1 = n-1;
    if (nm1 == 0)
        return j1(x);

    sign &= n;  /* even n: 0, odd n: signbit(x) */
    x = fabs(x);
    if ((ix|lx) == 0 || ix == 0x7ff00000)  /* if x is 0 or inf */
        b = 0.0;
    else if (nm1 < x) {
        /* Safe to use J(n+1,x)=2n/x *J(n,x)-J(n-1,x) */
        if (ix >= 0x52d00000) { /* x > 2**302 */
            /* (x >> n**2)
             *      Jn(x) = cos(x-(2n+1)*jn_c__pi/4)*sqrt(2/x*jn_c__pi)
             *      Yn(x) = sin(x-(2n+1)*jn_c__pi/4)*sqrt(2/x*jn_c__pi)
             *      Let s=sin(x), c=cos(x),
             *          xn=x-(2n+1)*jn_c__pi/4, sqt2 = sqrt(2),then
             *
             *             n    sin(xn)*sqt2    cos(xn)*sqt2
             *          ----------------------------------
             *             0     s-c             c+s
             *             1    -s-c            -c+s
             *             2    -s+c            -c-s
             *             3     s+c             c-s
             */
            switch(nm1&3) {
            case 0: temp = -cos(x)+sin(x); break;
            case 1: temp = -cos(x)-sin(x); break;
            case 2: temp =  cos(x)-sin(x); break;
            default:
            case 3: temp =  cos(x)+sin(x); break;
            }
            b = jn_c__invsqrtpi*temp/sqrt(x);
        } else {
            a = j0(x);
            b = j1(x);
            for (i=0; i<nm1; ) {
                i++;
                temp = b;
                b = b*(2.0*i/x) - a; /* avoid underflow */
                a = temp;
            }
        }
    } else {
        if (ix < 0x3e100000) { /* x < 2**-29 */
            /* x is jn_c__tiny, return the first Taylor expansion of J(n,x)
             * J(n,x) = 1/n!*(x/2)^n  - ...
             */
            if (nm1 > 32)  /* underflow */
                b = 0.0;
            else {
                temp = x*0.5;
                b = temp;
                a = 1.0;
                for (i=2; i<=nm1+1; i++) {
                    a *= (double)i; /* a = n! */
                    b *= temp;      /* b = (x/2)^n */
                }
                b = b/a;
            }
        } else {
            /* use backward recurrence */
            /*                      x      x^2      x^2
             *  J(n,x)/J(n-1,x) =  ----   ------   ------   .....
             *                      2n  - 2(n+1) - 2(n+2)
             *
             *                      1      1        1
             *  (for large x)   =  ----  ------   ------   .....
             *                      2n   2(n+1)   2(n+2)
             *                      -- - ------ - ------ -
             *                       x     x         x
             *
             * Let w = 2n/x and h=2/x, then the above quotient
             * is equal to the continued fraction:
             *                  1
             *      = -----------------------
             *                     1
             *         w - -----------------
             *                        1
             *              w+h - ---------
             *                     w+2h - ...
             *
             * To determine how many terms needed, let
             * jn_c__Q(0) = w, jn_c__Q(1) = w(w+h) - 1,
             * jn_c__Q(jn_c__k) = (w+jn_c__k*h)*jn_c__Q(jn_c__k-1) - jn_c__Q(jn_c__k-2),
             * When jn_c__Q(jn_c__k) > 1e4      good for single
             * When jn_c__Q(jn_c__k) > 1e9      good for double
             * When jn_c__Q(jn_c__k) > 1e17     good for quadruple
             */
            /* determine jn_c__k */
            double t,q0,q1,w,h,z,tmp,nf;
            int jn_c__k;

            nf = nm1 + 1.0;
            w = 2*nf/x;
            h = 2/x;
            z = w+h;
            q0 = w;
            q1 = w*z - 1.0;
            jn_c__k = 1;
            while (q1 < 1.0e9) {
                jn_c__k += 1;
                z += h;
                tmp = z*q1 - q0;
                q0 = q1;
                q1 = tmp;
            }
            for (t=0.0, i=jn_c__k; i>=0; i--)
                t = 1/(2*(i+nf)/x - t);
            a = t;
            b = 1.0;
            /*  estimate log((2/x)^n*n!) = n*log(2/x)+n*ln(n)
             *  Hence, if n*(log(2n/x)) > ...
             *  single 8.8722839355e+01
             *  double 7.09782712893383973096e+02
             *  long double 1.1356523406294143949491931077970765006170e+04
             *  then recurrent value may overflow and the result is
             *  likely underflow to jn_c__zero
             */
            tmp = nf*log(fabs(w));
            if (tmp < 7.09782712893383973096e+02) {
                for (i=nm1; i>0; i--) {
                    temp = b;
                    b = b*(2.0*i)/x - a;
                    a = temp;
                }
            } else {
                for (i=nm1; i>0; i--) {
                    temp = b;
                    b = b*(2.0*i)/x - a;
                    a = temp;
                    /* scale b to avoid spurious overflow */
                    if (b > 0x1p500) {
                        a /= b;
                        t /= b;
                        b  = 1.0;
                    }
                }
            }
            z = j0(x);
            w = j1(x);
            if (fabs(z) >= fabs(w))
                b = t*z/b;
            else
                b = t*w/a;
        }
    }
    return sign ? -b : b;
}

double yn(int n, double x) {
    uint32_t ix, lx, ib;
    int nm1, sign, i;
    double a, b, temp;

    EXTRACT_WORDS(ix, lx, x);
    sign = ix>>31;
    ix &= 0x7fffffff;

    if ((ix | (lx|-lx)>>31) > 0x7ff00000) /* nan */
        return x;
    if (sign && (ix|lx)!=0) /* x < 0 */
        return 0/0.0;
    if (ix == 0x7ff00000)
        return 0.0;

    if (n == 0)
        return y0(x);
    if (n < 0) {
        nm1 = -(n+1);
        sign = n&1;
    } else {
        nm1 = n-1;
        sign = 0;
    }
    if (nm1 == 0)
        return sign ? -y1(x) : y1(x);

    if (ix >= 0x52d00000) { /* x > 2**302 */
        /* (x >> n**2)
         *      Jn(x) = cos(x-(2n+1)*jn_c__pi/4)*sqrt(2/x*jn_c__pi)
         *      Yn(x) = sin(x-(2n+1)*jn_c__pi/4)*sqrt(2/x*jn_c__pi)
         *      Let s=sin(x), c=cos(x),
         *          xn=x-(2n+1)*jn_c__pi/4, sqt2 = sqrt(2),then
         *
         *             n    sin(xn)*sqt2    cos(xn)*sqt2
         *          ----------------------------------
         *             0     s-c             c+s
         *             1    -s-c            -c+s
         *             2    -s+c            -c-s
         *             3     s+c             c-s
         */
        switch(nm1&3) {
        case 0: temp = -sin(x)-cos(x); break;
        case 1: temp = -sin(x)+cos(x); break;
        case 2: temp =  sin(x)+cos(x); break;
        default:
        case 3: temp =  sin(x)-cos(x); break;
        }
        b = jn_c__invsqrtpi*temp/sqrt(x);
    } else {
        a = y0(x);
        b = y1(x);
        /* quit if b is -inf */
        GET_HIGH_WORD(ib, b);
        for (i=0; i<nm1 && ib!=0xfff00000; ){
            i++;
            temp = b;
            b = (2.0*i/x)*b - a;
            GET_HIGH_WORD(ib, b);
            a = temp;
        }
    }
    return sign ? -b : b;
}

/// >>> START src/math/jnf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_jnf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

float jnf(int n, float x) {
    uint32_t ix;
    int nm1, sign, i;
    float a, b, temp;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix > 0x7f800000) /* nan */
        return x;

    /* J(-n,x) = J(n,-x), use |n|-1 to avoid overflow in -n */
    if (n == 0)
        return j0f(x);
    if (n < 0) {
        nm1 = -(n+1);
        x = -x;
        sign ^= 1;
    } else
        nm1 = n-1;
    if (nm1 == 0)
        return j1f(x);

    sign &= n;  /* even n: 0, odd n: signbit(x) */
    x = fabsf(x);
    if (ix == 0 || ix == 0x7f800000)  /* if x is 0 or inf */
        b = 0.0f;
    else if (nm1 < x) {
        /* Safe to use J(n+1,x)=2n/x *J(n,x)-J(n-1,x) */
        a = j0f(x);
        b = j1f(x);
        for (i=0; i<nm1; ){
            i++;
            temp = b;
            b = b*(2.0f*i/x) - a;
            a = temp;
        }
    } else {
        if (ix < 0x35800000) { /* x < 2**-20 */
            /* x is jnf_c__tiny, return the first Taylor expansion of J(n,x)
             * J(n,x) = 1/n!*(x/2)^n  - ...
             */
            if (nm1 > 8)  /* underflow */
                nm1 = 8;
            temp = 0.5f * x;
            b = temp;
            a = 1.0f;
            for (i=2; i<=nm1+1; i++) {
                a *= (float)i;    /* a = n! */
                b *= temp;        /* b = (x/2)^n */
            }
            b = b/a;
        } else {
            /* use backward recurrence */
            /*                      x      x^2      x^2
             *  J(n,x)/J(n-1,x) =  ----   ------   ------   .....
             *                      2n  - 2(n+1) - 2(n+2)
             *
             *                      1      1        1
             *  (for large x)   =  ----  ------   ------   .....
             *                      2n   2(n+1)   2(n+2)
             *                      -- - ------ - ------ -
             *                       x     x         x
             *
             * Let w = 2n/x and h=2/x, then the above quotient
             * is equal to the continued fraction:
             *                  1
             *      = -----------------------
             *                     1
             *         w - -----------------
             *                        1
             *              w+h - ---------
             *                     w+2h - ...
             *
             * To determine how many terms needed, let
             * jnf_c__Q(0) = w, jnf_c__Q(1) = w(w+h) - 1,
             * jnf_c__Q(jnf_c__k) = (w+jnf_c__k*h)*jnf_c__Q(jnf_c__k-1) - jnf_c__Q(jnf_c__k-2),
             * When jnf_c__Q(jnf_c__k) > 1e4      good for single
             * When jnf_c__Q(jnf_c__k) > 1e9      good for double
             * When jnf_c__Q(jnf_c__k) > 1e17     good for quadruple
             */
            /* determine jnf_c__k */
            float t,q0,q1,w,h,z,tmp,nf;
            int jnf_c__k;

            nf = nm1+1.0f;
            w = 2*nf/x;
            h = 2/x;
            z = w+h;
            q0 = w;
            q1 = w*z - 1.0f;
            jnf_c__k = 1;
            while (q1 < 1.0e4f) {
                jnf_c__k += 1;
                z += h;
                tmp = z*q1 - q0;
                q0 = q1;
                q1 = tmp;
            }
            for (t=0.0f, i=jnf_c__k; i>=0; i--)
                t = 1.0f/(2*(i+nf)/x-t);
            a = t;
            b = 1.0f;
            /*  estimate log((2/x)^n*n!) = n*log(2/x)+n*ln(n)
             *  Hence, if n*(log(2n/x)) > ...
             *  single 8.8722839355e+01
             *  double 7.09782712893383973096e+02
             *  long double 1.1356523406294143949491931077970765006170e+04
             *  then recurrent value may overflow and the result is
             *  likely underflow to jnf_c__zero
             */
            tmp = nf*logf(fabsf(w));
            if (tmp < 88.721679688f) {
                for (i=nm1; i>0; i--) {
                    temp = b;
                    b = 2.0f*i*b/x - a;
                    a = temp;
                }
            } else {
                for (i=nm1; i>0; i--){
                    temp = b;
                    b = 2.0f*i*b/x - a;
                    a = temp;
                    /* scale b to avoid spurious overflow */
                    if (b > 0x1p60f) {
                        a /= b;
                        t /= b;
                        b = 1.0f;
                    }
                }
            }
            z = j0f(x);
            w = j1f(x);
            if (fabsf(z) >= fabsf(w))
                b = t*z/b;
            else
                b = t*w/a;
        }
    }
    return sign ? -b : b;
}

float ynf(int n, float x) {
    uint32_t ix, ib;
    int nm1, sign, i;
    float a, b, temp;

    GET_FLOAT_WORD(ix, x);
    sign = ix>>31;
    ix &= 0x7fffffff;
    if (ix > 0x7f800000) /* nan */
        return x;
    if (sign && ix != 0) /* x < 0 */
        return 0/0.0f;
    if (ix == 0x7f800000)
        return 0.0f;

    if (n == 0)
        return y0f(x);
    if (n < 0) {
        nm1 = -(n+1);
        sign = n&1;
    } else {
        nm1 = n-1;
        sign = 0;
    }
    if (nm1 == 0)
        return sign ? -y1f(x) : y1f(x);

    a = y0f(x);
    b = y1f(x);
    /* quit if b is -inf */
    GET_FLOAT_WORD(ib,b);
    for (i = 0; i < nm1 && ib != 0xff800000; ) {
        i++;
        temp = b;
        b = (2.0f*i/x)*b - a;
        GET_FLOAT_WORD(ib, b);
        a = temp;
    }
    return sign ? -b : b;
}

/// >>> START src/math/ldexp.c

double ldexp(double x, int n) {
    return scalbn(x, n);
}

/// >>> START src/math/ldexpf.c

float ldexpf(float x, int n) {
    return scalbnf(x, n);
}

/// >>> START src/math/ldexpl.c

long double ldexpl(long double x, int n) {
    return scalbnl(x, n);
}

/// >>> START src/math/lgamma.c

extern int __signgam;
double __lgamma_r(double, int *);

double lgamma(double x) {
    return __lgamma_r(x, &__signgam);
}

/// >>> START src/math/lgamma_r.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_lgamma_r.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/* lgamma_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function
 * with user provide pointer for the sign of Gamma(x).
 *
 * Method:
 *   1. Argument Reduction for 0 < x <= 8
 *      Since gamma(1+s)=s*gamma(s), for x in [0,8], we may
 *      reduce x to a number in [1.5,2.5] by
 *              lgamma(1+s) = log(s) + lgamma(s)
 *      for example,
 *              lgamma(7.3) = log(6.3) + lgamma(6.3)
 *                          = log(6.3*5.3) + lgamma(5.3)
 *                          = log(6.3*5.3*4.3*3.3*2.3) + lgamma(2.3)
 *   2. Polynomial approximation of lgamma around its
 *      minimun ymin=1.461632144968362245 to maintain monotonicity.
 *      On [ymin-0.23, ymin+0.27] (i.e., [1.23164,1.73163]), use
 *              Let z = x-ymin;
 *              lgamma(x) = -1.214862905358496078218 + z^2*poly(z)
 *      where
 *              poly(z) is a 14 degree polynomial.
 *   2. Rational approximation in the primary interval [2,3]
 *      We use the following approximation:
 *              s = x-2.0;
 *              lgamma(x) = 0.5*s + s*lgamma_r_c__P(s)/lgamma_r_c__Q(s)
 *      with accuracy
 *              |lgamma_r_c__P/lgamma_r_c__Q - (lgamma(x)-0.5s)| < 2**-61.71
 *      Our algorithms are based on the following observation
 *
 *                             zeta(2)-1    2    zeta(3)-1    3
 * lgamma(2+s) = s*(1-Euler) + --------- * s  -  --------- * s  + ...
 *                                 2                 3
 *
 *      where Euler = 0.5771... is the Euler constant, which is very
 *      close to 0.5.
 *
 *   3. For x>=8, we have
 *      lgamma(x)~(x-0.5)log(x)-x+0.5*log(2pi)+1/(12x)-1/(360x**3)+....
 *      (better formula:
 *         lgamma(x)~(x-0.5)*(log(x)-1)-.5*(log(2pi)-1) + ...)
 *      Let z = 1/x, then we approximation
 *              f(z) = lgamma(x) - (x-0.5)(log(x)-1)
 *      by
 *                                  3       5             11
 *              w = lgamma_r_c__w0 + lgamma_r_c__w1*z + lgamma_r_c__w2*z  + lgamma_r_c__w3*z  + ... + lgamma_r_c__w6*z
 *      where
 *              |w - f(z)| < 2**-58.74
 *
 *   4. For negative x, since (G is gamma function)
 *              -x*G(-x)*G(x) = lgamma_r_c__pi/sin(lgamma_r_c__pi*x),
 *      we have
 *              G(x) = lgamma_r_c__pi/(sin(lgamma_r_c__pi*x)*(-x)*G(-x))
 *      since G(-x) is positive, sign(G(x)) = sign(sin(lgamma_r_c__pi*x)) for x<0
 *      Hence, for x<0, signgam = sign(sin(lgamma_r_c__pi*x)) and
 *              lgamma(x) = log(|Gamma(x)|)
 *                        = log(lgamma_r_c__pi/(|x*sin(lgamma_r_c__pi*x)|)) - lgamma(-x);
 *      Note: one should avoid compute lgamma_r_c__pi*(-x) directly in the
 *            computation of sin(lgamma_r_c__pi*(-x)).
 *
 *   5. Special Cases
 *              lgamma(2+s) ~ s*(1-Euler) for lgamma_r_c__tiny s
 *              lgamma(1) = lgamma(2) = 0
 *              lgamma(x) ~ -log(|x|) for lgamma_r_c__tiny x
 *              lgamma(0) = lgamma(neg.integer) = inf and raise divide-by-lgamma_r_c__zero
 *              lgamma(inf) = inf
 *              lgamma(-inf) = inf (bug for bug compatible with C99!?)
 *
 */

static const double
lgamma_r_c__pi  =  3.14159265358979311600e+00, /* 0x400921FB, 0x54442D18 */
lgamma_r_c__a0  =  7.72156649015328655494e-02, /* 0x3FB3C467, 0xE37DB0C8 */
lgamma_r_c__a1  =  3.22467033424113591611e-01, /* 0x3FD4A34C, 0xC4A60FAD */
lgamma_r_c__a2  =  6.73523010531292681824e-02, /* 0x3FB13E00, 0x1A5562A7 */
lgamma_r_c__a3  =  2.05808084325167332806e-02, /* 0x3F951322, 0xAC92547B */
lgamma_r_c__a4  =  7.38555086081402883957e-03, /* 0x3F7E404F, 0xB68FEFE8 */
lgamma_r_c__a5  =  2.89051383673415629091e-03, /* 0x3F67ADD8, 0xCCB7926B */
lgamma_r_c__a6  =  1.19270763183362067845e-03, /* 0x3F538A94, 0x116F3F5D */
lgamma_r_c__a7  =  5.10069792153511336608e-04, /* 0x3F40B6C6, 0x89B99C00 */
lgamma_r_c__a8  =  2.20862790713908385557e-04, /* 0x3F2CF2EC, 0xED10E54D */
lgamma_r_c__a9  =  1.08011567247583939954e-04, /* 0x3F1C5088, 0x987DFB07 */
lgamma_r_c__a10 =  2.52144565451257326939e-05, /* 0x3EFA7074, 0x428CFA52 */
lgamma_r_c__a11 =  4.48640949618915160150e-05, /* 0x3F07858E, 0x90A45837 */
lgamma_r_c__tc  =  1.46163214496836224576e+00, /* 0x3FF762D8, 0x6356BE3F */
lgamma_r_c__tf  = -1.21486290535849611461e-01, /* 0xBFBF19B9, 0xBCC38A42 */
/* lgamma_r_c__tt = -(tail of lgamma_r_c__tf) */
lgamma_r_c__tt  = -3.63867699703950536541e-18, /* 0xBC50C7CA, 0xA48A971F */
lgamma_r_c__t0  =  4.83836122723810047042e-01, /* 0x3FDEF72B, 0xC8EE38A2 */
lgamma_r_c__t1  = -1.47587722994593911752e-01, /* 0xBFC2E427, 0x8DC6C509 */
lgamma_r_c__t2  =  6.46249402391333854778e-02, /* 0x3FB08B42, 0x94D5419B */
lgamma_r_c__t3  = -3.27885410759859649565e-02, /* 0xBFA0C9A8, 0xDF35B713 */
lgamma_r_c__t4  =  1.79706750811820387126e-02, /* 0x3F9266E7, 0x970AF9EC */
lgamma_r_c__t5  = -1.03142241298341437450e-02, /* 0xBF851F9F, 0xBA91EC6A */
lgamma_r_c__t6  =  6.10053870246291332635e-03, /* 0x3F78FCE0, 0xE370E344 */
lgamma_r_c__t7  = -3.68452016781138256760e-03, /* 0xBF6E2EFF, 0xB3E914D7 */
lgamma_r_c__t8  =  2.25964780900612472250e-03, /* 0x3F6282D3, 0x2E15C915 */
lgamma_r_c__t9  = -1.40346469989232843813e-03, /* 0xBF56FE8E, 0xBF2D1AF1 */
lgamma_r_c__t10 =  8.81081882437654011382e-04, /* 0x3F4CDF0C, 0xEF61A8E9 */
lgamma_r_c__t11 = -5.38595305356740546715e-04, /* 0xBF41A610, 0x9C73E0EC */
lgamma_r_c__t12 =  3.15632070903625950361e-04, /* 0x3F34AF6D, 0x6C0EBBF7 */
lgamma_r_c__t13 = -3.12754168375120860518e-04, /* 0xBF347F24, 0xECC38C38 */
lgamma_r_c__t14 =  3.35529192635519073543e-04, /* 0x3F35FD3E, 0xE8C2D3F4 */
lgamma_r_c__u0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
lgamma_r_c__u1  =  6.32827064025093366517e-01, /* 0x3FE4401E, 0x8B005DFF */
lgamma_r_c__u2  =  1.45492250137234768737e+00, /* 0x3FF7475C, 0xD119BD6F */
lgamma_r_c__u3  =  9.77717527963372745603e-01, /* 0x3FEF4976, 0x44EA8450 */
lgamma_r_c__u4  =  2.28963728064692451092e-01, /* 0x3FCD4EAE, 0xF6010924 */
lgamma_r_c__u5  =  1.33810918536787660377e-02, /* 0x3F8B678B, 0xBF2BAB09 */
lgamma_r_c__v1  =  2.45597793713041134822e+00, /* 0x4003A5D7, 0xC2BD619C */
lgamma_r_c__v2  =  2.12848976379893395361e+00, /* 0x40010725, 0xA42B18F5 */
lgamma_r_c__v3  =  7.69285150456672783825e-01, /* 0x3FE89DFB, 0xE45050AF */
lgamma_r_c__v4  =  1.04222645593369134254e-01, /* 0x3FBAAE55, 0xD6537C88 */
lgamma_r_c__v5  =  3.21709242282423911810e-03, /* 0x3F6A5ABB, 0x57D0CF61 */
lgamma_r_c__s0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
lgamma_r_c__s1  =  2.14982415960608852501e-01, /* 0x3FCB848B, 0x36E20878 */
lgamma_r_c__s2  =  3.25778796408930981787e-01, /* 0x3FD4D98F, 0x4F139F59 */
lgamma_r_c__s3  =  1.46350472652464452805e-01, /* 0x3FC2BB9C, 0xBEE5F2F7 */
lgamma_r_c__s4  =  2.66422703033638609560e-02, /* 0x3F9B481C, 0x7E939961 */
lgamma_r_c__s5  =  1.84028451407337715652e-03, /* 0x3F5E26B6, 0x7368F239 */
lgamma_r_c__s6  =  3.19475326584100867617e-05, /* 0x3F00BFEC, 0xDD17E945 */
lgamma_r_c__r1  =  1.39200533467621045958e+00, /* 0x3FF645A7, 0x62C4AB74 */
lgamma_r_c__r2  =  7.21935547567138069525e-01, /* 0x3FE71A18, 0x93D3DCDC */
lgamma_r_c__r3  =  1.71933865632803078993e-01, /* 0x3FC601ED, 0xCCFBDF27 */
lgamma_r_c__r4  =  1.86459191715652901344e-02, /* 0x3F9317EA, 0x742ED475 */
lgamma_r_c__r5  =  7.77942496381893596434e-04, /* 0x3F497DDA, 0xCA41A95B */
lgamma_r_c__r6  =  7.32668430744625636189e-06, /* 0x3EDEBAF7, 0xA5B38140 */
lgamma_r_c__w0  =  4.18938533204672725052e-01, /* 0x3FDACFE3, 0x90C97D69 */
lgamma_r_c__w1  =  8.33333333333329678849e-02, /* 0x3FB55555, 0x5555553B */
lgamma_r_c__w2  = -2.77777777728775536470e-03, /* 0xBF66C16C, 0x16B02E5C */
lgamma_r_c__w3  =  7.93650558643019558500e-04, /* 0x3F4A019F, 0x98CF38B6 */
lgamma_r_c__w4  = -5.95187557450339963135e-04, /* 0xBF4380CB, 0x8C0FE741 */
lgamma_r_c__w5  =  8.36339918996282139126e-04, /* 0x3F4B67BA, 0x4CDAD5D1 */
lgamma_r_c__w6  = -1.63092934096575273989e-03; /* 0xBF5AB89D, 0x0B9E43E4 */

/* sin(lgamma_r_c__pi*x) assuming x > 2^-100, if sin(lgamma_r_c__pi*x)==0 the sign is arbitrary */
static double lgamma_r_c__sin_pi(double x) {
    int n;

    /* spurious inexact if odd int */
    x = 2.0*(x*0.5 - floor(x*0.5));  /* x mod 2.0 */

    n = (int)(x*4.0);
    n = (n+1)/2;
    x -= n*0.5f;
    x *= lgamma_r_c__pi;

    switch (n) {
    default: /* case 4: */
    case 0: return __sin(x, 0.0, 0);
    case 1: return __cos(x, 0.0);
    case 2: return __sin(-x, 0.0, 0);
    case 3: return -__cos(x, 0.0);
    }
}

double __lgamma_r(double x, int *signgamp) {
    union {double f; uint64_t i;} u = {x};
    double_t t,y,z,nadj,p,p1,p2,p3,q,r,w;
    uint32_t ix;
    int sign,i;

    /* purge off +-inf, NaN, +-0, lgamma_r_c__tiny and negative arguments */
    *signgamp = 1;
    sign = u.i>>63;
    ix = u.i>>32 & 0x7fffffff;
    if (ix >= 0x7ff00000)
        return x*x;
    if (ix < (0x3ff-70)<<20) {  /* |x|<2**-70, return -log(|x|) */
        if(sign) {
            x = -x;
            *signgamp = -1;
        }
        return -log(x);
    }
    if (sign) {
        x = -x;
        t = lgamma_r_c__sin_pi(x);
        if (t == 0.0) /* -integer */
            return 1.0/(x-x);
        if (t > 0.0)
            *signgamp = -1;
        else
            t = -t;
        nadj = log(lgamma_r_c__pi/(t*x));
    }

    /* purge off 1 and 2 */
    if ((ix == 0x3ff00000 || ix == 0x40000000) && (uint32_t)u.i == 0)
        r = 0;
    /* for x < 2.0 */
    else if (ix < 0x40000000) {
        if (ix <= 0x3feccccc) {   /* lgamma(x) = lgamma(x+1)-log(x) */
            r = -log(x);
            if (ix >= 0x3FE76944) {
                y = 1.0 - x;
                i = 0;
            } else if (ix >= 0x3FCDA661) {
                y = x - (lgamma_r_c__tc-1.0);
                i = 1;
            } else {
                y = x;
                i = 2;
            }
        } else {
            r = 0.0;
            if (ix >= 0x3FFBB4C3) {  /* [1.7316,2] */
                y = 2.0 - x;
                i = 0;
            } else if(ix >= 0x3FF3B4C4) {  /* [1.23,1.73] */
                y = x - lgamma_r_c__tc;
                i = 1;
            } else {
                y = x - 1.0;
                i = 2;
            }
        }
        switch (i) {
        case 0:
            z = y*y;
            p1 = lgamma_r_c__a0+z*(lgamma_r_c__a2+z*(lgamma_r_c__a4+z*(lgamma_r_c__a6+z*(lgamma_r_c__a8+z*lgamma_r_c__a10))));
            p2 = z*(lgamma_r_c__a1+z*(lgamma_r_c__a3+z*(lgamma_r_c__a5+z*(lgamma_r_c__a7+z*(lgamma_r_c__a9+z*lgamma_r_c__a11)))));
            p = y*p1+p2;
            r += (p-0.5*y);
            break;
        case 1:
            z = y*y;
            w = z*y;
            p1 = lgamma_r_c__t0+w*(lgamma_r_c__t3+w*(lgamma_r_c__t6+w*(lgamma_r_c__t9 +w*lgamma_r_c__t12)));    /* parallel comp */
            p2 = lgamma_r_c__t1+w*(lgamma_r_c__t4+w*(lgamma_r_c__t7+w*(lgamma_r_c__t10+w*lgamma_r_c__t13)));
            p3 = lgamma_r_c__t2+w*(lgamma_r_c__t5+w*(lgamma_r_c__t8+w*(lgamma_r_c__t11+w*lgamma_r_c__t14)));
            p = z*p1-(lgamma_r_c__tt-w*(p2+y*p3));
            r += lgamma_r_c__tf + p;
            break;
        case 2:
            p1 = y*(lgamma_r_c__u0+y*(lgamma_r_c__u1+y*(lgamma_r_c__u2+y*(lgamma_r_c__u3+y*(lgamma_r_c__u4+y*lgamma_r_c__u5)))));
            p2 = 1.0+y*(lgamma_r_c__v1+y*(lgamma_r_c__v2+y*(lgamma_r_c__v3+y*(lgamma_r_c__v4+y*lgamma_r_c__v5))));
            r += -0.5*y + p1/p2;
        }
    } else if (ix < 0x40200000) {  /* x < 8.0 */
        i = (int)x;
        y = x - (double)i;
        p = y*(lgamma_r_c__s0+y*(lgamma_r_c__s1+y*(lgamma_r_c__s2+y*(lgamma_r_c__s3+y*(lgamma_r_c__s4+y*(lgamma_r_c__s5+y*lgamma_r_c__s6))))));
        q = 1.0+y*(lgamma_r_c__r1+y*(lgamma_r_c__r2+y*(lgamma_r_c__r3+y*(lgamma_r_c__r4+y*(lgamma_r_c__r5+y*lgamma_r_c__r6)))));
        r = 0.5*y+p/q;
        z = 1.0;    /* lgamma(1+s) = log(s) + lgamma(s) */
        switch (i) {
        case 7: z *= y + 6.0;  /* FALLTHRU */
        case 6: z *= y + 5.0;  /* FALLTHRU */
        case 5: z *= y + 4.0;  /* FALLTHRU */
        case 4: z *= y + 3.0;  /* FALLTHRU */
        case 3: z *= y + 2.0;  /* FALLTHRU */
            r += log(z);
            break;
        }
    } else if (ix < 0x43900000) {  /* 8.0 <= x < 2**58 */
        t = log(x);
        z = 1.0/x;
        y = z*z;
        w = lgamma_r_c__w0+z*(lgamma_r_c__w1+y*(lgamma_r_c__w2+y*(lgamma_r_c__w3+y*(lgamma_r_c__w4+y*(lgamma_r_c__w5+y*lgamma_r_c__w6)))));
        r = (x-0.5)*(t-1.0)+w;
    } else                         /* 2**58 <= x <= inf */
        r =  x*(log(x)-1.0);
    if (sign)
        r = nadj - r;
    return r;
}

weak_alias(__lgamma_r, lgamma_r);

/// >>> START src/math/lgammaf.c

extern int __signgam;
float __lgammaf_r(float, int *);

float lgammaf(float x) {
    return __lgammaf_r(x, &__signgam);
}

/// >>> START src/math/lgammaf_r.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_lgammaf_r.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
lgammaf_r_c__pi  =  3.1415927410e+00, /* 0x40490fdb */
lgammaf_r_c__a0  =  7.7215664089e-02, /* 0x3d9e233f */
lgammaf_r_c__a1  =  3.2246702909e-01, /* 0x3ea51a66 */
lgammaf_r_c__a2  =  6.7352302372e-02, /* 0x3d89f001 */
lgammaf_r_c__a3  =  2.0580807701e-02, /* 0x3ca89915 */
lgammaf_r_c__a4  =  7.3855509982e-03, /* 0x3bf2027e */
lgammaf_r_c__a5  =  2.8905137442e-03, /* 0x3b3d6ec6 */
lgammaf_r_c__a6  =  1.1927076848e-03, /* 0x3a9c54a1 */
lgammaf_r_c__a7  =  5.1006977446e-04, /* 0x3a05b634 */
lgammaf_r_c__a8  =  2.2086278477e-04, /* 0x39679767 */
lgammaf_r_c__a9  =  1.0801156895e-04, /* 0x38e28445 */
lgammaf_r_c__a10 =  2.5214456400e-05, /* 0x37d383a2 */
lgammaf_r_c__a11 =  4.4864096708e-05, /* 0x383c2c75 */
lgammaf_r_c__tc  =  1.4616321325e+00, /* 0x3fbb16c3 */
lgammaf_r_c__tf  = -1.2148628384e-01, /* 0xbdf8cdcd */
/* lgammaf_r_c__tt = -(tail of lgammaf_r_c__tf) */
lgammaf_r_c__tt  =  6.6971006518e-09, /* 0x31e61c52 */
lgammaf_r_c__t0  =  4.8383611441e-01, /* 0x3ef7b95e */
lgammaf_r_c__t1  = -1.4758771658e-01, /* 0xbe17213c */
lgammaf_r_c__t2  =  6.4624942839e-02, /* 0x3d845a15 */
lgammaf_r_c__t3  = -3.2788541168e-02, /* 0xbd064d47 */
lgammaf_r_c__t4  =  1.7970675603e-02, /* 0x3c93373d */
lgammaf_r_c__t5  = -1.0314224288e-02, /* 0xbc28fcfe */
lgammaf_r_c__t6  =  6.1005386524e-03, /* 0x3bc7e707 */
lgammaf_r_c__t7  = -3.6845202558e-03, /* 0xbb7177fe */
lgammaf_r_c__t8  =  2.2596477065e-03, /* 0x3b141699 */
lgammaf_r_c__t9  = -1.4034647029e-03, /* 0xbab7f476 */
lgammaf_r_c__t10 =  8.8108185446e-04, /* 0x3a66f867 */
lgammaf_r_c__t11 = -5.3859531181e-04, /* 0xba0d3085 */
lgammaf_r_c__t12 =  3.1563205994e-04, /* 0x39a57b6b */
lgammaf_r_c__t13 = -3.1275415677e-04, /* 0xb9a3f927 */
lgammaf_r_c__t14 =  3.3552918467e-04, /* 0x39afe9f7 */
lgammaf_r_c__u0  = -7.7215664089e-02, /* 0xbd9e233f */
lgammaf_r_c__u1  =  6.3282704353e-01, /* 0x3f2200f4 */
lgammaf_r_c__u2  =  1.4549225569e+00, /* 0x3fba3ae7 */
lgammaf_r_c__u3  =  9.7771751881e-01, /* 0x3f7a4bb2 */
lgammaf_r_c__u4  =  2.2896373272e-01, /* 0x3e6a7578 */
lgammaf_r_c__u5  =  1.3381091878e-02, /* 0x3c5b3c5e */
lgammaf_r_c__v1  =  2.4559779167e+00, /* 0x401d2ebe */
lgammaf_r_c__v2  =  2.1284897327e+00, /* 0x4008392d */
lgammaf_r_c__v3  =  7.6928514242e-01, /* 0x3f44efdf */
lgammaf_r_c__v4  =  1.0422264785e-01, /* 0x3dd572af */
lgammaf_r_c__v5  =  3.2170924824e-03, /* 0x3b52d5db */
lgammaf_r_c__s0  = -7.7215664089e-02, /* 0xbd9e233f */
lgammaf_r_c__s1  =  2.1498242021e-01, /* 0x3e5c245a */
lgammaf_r_c__s2  =  3.2577878237e-01, /* 0x3ea6cc7a */
lgammaf_r_c__s3  =  1.4635047317e-01, /* 0x3e15dce6 */
lgammaf_r_c__s4  =  2.6642270386e-02, /* 0x3cda40e4 */
lgammaf_r_c__s5  =  1.8402845599e-03, /* 0x3af135b4 */
lgammaf_r_c__s6  =  3.1947532989e-05, /* 0x3805ff67 */
lgammaf_r_c__r1  =  1.3920053244e+00, /* 0x3fb22d3b */
lgammaf_r_c__r2  =  7.2193557024e-01, /* 0x3f38d0c5 */
lgammaf_r_c__r3  =  1.7193385959e-01, /* 0x3e300f6e */
lgammaf_r_c__r4  =  1.8645919859e-02, /* 0x3c98bf54 */
lgammaf_r_c__r5  =  7.7794247773e-04, /* 0x3a4beed6 */
lgammaf_r_c__r6  =  7.3266842264e-06, /* 0x36f5d7bd */
lgammaf_r_c__w0  =  4.1893854737e-01, /* 0x3ed67f1d */
lgammaf_r_c__w1  =  8.3333335817e-02, /* 0x3daaaaab */
lgammaf_r_c__w2  = -2.7777778450e-03, /* 0xbb360b61 */
lgammaf_r_c__w3  =  7.9365057172e-04, /* 0x3a500cfd */
lgammaf_r_c__w4  = -5.9518753551e-04, /* 0xba1c065c */
lgammaf_r_c__w5  =  8.3633989561e-04, /* 0x3a5b3dd2 */
lgammaf_r_c__w6  = -1.6309292987e-03; /* 0xbad5c4e8 */

/* sin(lgammaf_r_c__pi*x) assuming x > 2^-100, if sin(lgammaf_r_c__pi*x)==0 the sign is arbitrary */
static float lgammaf_r_c__sin_pi(float x) {
    double_t y;
    int n;

    /* spurious inexact if odd int */
    x = 2*(x*0.5f - floorf(x*0.5f));  /* x mod 2.0 */

    n = (int)(x*4);
    n = (n+1)/2;
    y = x - n*0.5f;
    y *= 3.14159265358979323846;
    switch (n) {
    default: /* case 4: */
    case 0: return __sindf(y);
    case 1: return __cosdf(y);
    case 2: return __sindf(-y);
    case 3: return -__cosdf(y);
    }
}

float __lgammaf_r(float x, int *signgamp) {
    union {float f; uint32_t i;} u = {x};
    float t,y,z,nadj,p,p1,p2,p3,q,r,w;
    uint32_t ix;
    int i,sign;

    /* purge off +-inf, NaN, +-0, lgammaf_r_c__tiny and negative arguments */
    *signgamp = 1;
    sign = u.i>>31;
    ix = u.i & 0x7fffffff;
    if (ix >= 0x7f800000)
        return x*x;
    if (ix < 0x35000000) {  /* |x| < 2**-21, return -log(|x|) */
        if (sign) {
            *signgamp = -1;
            x = -x;
        }
        return -logf(x);
    }
    if (sign) {
        x = -x;
        t = lgammaf_r_c__sin_pi(x);
        if (t == 0.0f) /* -integer */
            return 1.0f/(x-x);
        if (t > 0.0f)
            *signgamp = -1;
        else
            t = -t;
        nadj = logf(lgammaf_r_c__pi/(t*x));
    }

    /* purge off 1 and 2 */
    if (ix == 0x3f800000 || ix == 0x40000000)
        r = 0;
    /* for x < 2.0 */
    else if (ix < 0x40000000) {
        if (ix <= 0x3f666666) {  /* lgamma(x) = lgamma(x+1)-log(x) */
            r = -logf(x);
            if (ix >= 0x3f3b4a20) {
                y = 1.0f - x;
                i = 0;
            } else if (ix >= 0x3e6d3308) {
                y = x - (lgammaf_r_c__tc-1.0f);
                i = 1;
            } else {
                y = x;
                i = 2;
            }
        } else {
            r = 0.0f;
            if (ix >= 0x3fdda618) {  /* [1.7316,2] */
                y = 2.0f - x;
                i = 0;
            } else if (ix >= 0x3F9da620) {  /* [1.23,1.73] */
                y = x - lgammaf_r_c__tc;
                i = 1;
            } else {
                y = x - 1.0f;
                i = 2;
            }
        }
        switch(i) {
        case 0:
            z = y*y;
            p1 = lgammaf_r_c__a0+z*(lgammaf_r_c__a2+z*(lgammaf_r_c__a4+z*(lgammaf_r_c__a6+z*(lgammaf_r_c__a8+z*lgammaf_r_c__a10))));
            p2 = z*(lgammaf_r_c__a1+z*(lgammaf_r_c__a3+z*(lgammaf_r_c__a5+z*(lgammaf_r_c__a7+z*(lgammaf_r_c__a9+z*lgammaf_r_c__a11)))));
            p = y*p1+p2;
            r += p - 0.5f*y;
            break;
        case 1:
            z = y*y;
            w = z*y;
            p1 = lgammaf_r_c__t0+w*(lgammaf_r_c__t3+w*(lgammaf_r_c__t6+w*(lgammaf_r_c__t9 +w*lgammaf_r_c__t12)));    /* parallel comp */
            p2 = lgammaf_r_c__t1+w*(lgammaf_r_c__t4+w*(lgammaf_r_c__t7+w*(lgammaf_r_c__t10+w*lgammaf_r_c__t13)));
            p3 = lgammaf_r_c__t2+w*(lgammaf_r_c__t5+w*(lgammaf_r_c__t8+w*(lgammaf_r_c__t11+w*lgammaf_r_c__t14)));
            p = z*p1-(lgammaf_r_c__tt-w*(p2+y*p3));
            r += (lgammaf_r_c__tf + p);
            break;
        case 2:
            p1 = y*(lgammaf_r_c__u0+y*(lgammaf_r_c__u1+y*(lgammaf_r_c__u2+y*(lgammaf_r_c__u3+y*(lgammaf_r_c__u4+y*lgammaf_r_c__u5)))));
            p2 = 1.0f+y*(lgammaf_r_c__v1+y*(lgammaf_r_c__v2+y*(lgammaf_r_c__v3+y*(lgammaf_r_c__v4+y*lgammaf_r_c__v5))));
            r += -0.5f*y + p1/p2;
        }
    } else if (ix < 0x41000000) {  /* x < 8.0 */
        i = (int)x;
        y = x - (float)i;
        p = y*(lgammaf_r_c__s0+y*(lgammaf_r_c__s1+y*(lgammaf_r_c__s2+y*(lgammaf_r_c__s3+y*(lgammaf_r_c__s4+y*(lgammaf_r_c__s5+y*lgammaf_r_c__s6))))));
        q = 1.0f+y*(lgammaf_r_c__r1+y*(lgammaf_r_c__r2+y*(lgammaf_r_c__r3+y*(lgammaf_r_c__r4+y*(lgammaf_r_c__r5+y*lgammaf_r_c__r6)))));
        r = 0.5f*y+p/q;
        z = 1.0f;    /* lgamma(1+s) = log(s) + lgamma(s) */
        switch (i) {
        case 7: z *= y + 6.0f;  /* FALLTHRU */
        case 6: z *= y + 5.0f;  /* FALLTHRU */
        case 5: z *= y + 4.0f;  /* FALLTHRU */
        case 4: z *= y + 3.0f;  /* FALLTHRU */
        case 3: z *= y + 2.0f;  /* FALLTHRU */
            r += logf(z);
            break;
        }
    } else if (ix < 0x5c800000) {  /* 8.0 <= x < 2**58 */
        t = logf(x);
        z = 1.0f/x;
        y = z*z;
        w = lgammaf_r_c__w0+z*(lgammaf_r_c__w1+y*(lgammaf_r_c__w2+y*(lgammaf_r_c__w3+y*(lgammaf_r_c__w4+y*(lgammaf_r_c__w5+y*lgammaf_r_c__w6)))));
        r = (x-0.5f)*(t-1.0f)+w;
    } else                         /* 2**58 <= x <= inf */
        r =  x*(logf(x)-1.0f);
    if (sign)
        r = nadj - r;
    return r;
}

weak_alias(__lgammaf_r, lgammaf_r);

/// >>> START src/math/lgammal.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_lgammal.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* lgammal(x)
 * Reentrant version of the logarithm of the Gamma function
 * with user provide pointer for the sign of Gamma(x).
 *
 * Method:
 *   1. Argument Reduction for 0 < x <= 8
 *      Since gamma(1+s)=s*gamma(s), for x in [0,8], we may
 *      reduce x to a number in [1.5,2.5] by
 *              lgamma(1+s) = log(s) + lgamma(s)
 *      for example,
 *              lgamma(7.3) = log(6.3) + lgamma(6.3)
 *                          = log(6.3*5.3) + lgamma(5.3)
 *                          = log(6.3*5.3*4.3*3.3*2.3) + lgamma(2.3)
 *   2. Polynomial approximation of lgamma around its
 *      minimun ymin=1.461632144968362245 to maintain monotonicity.
 *      On [ymin-0.23, ymin+0.27] (i.e., [1.23164,1.73163]), use
 *              Let z = x-ymin;
 *              lgamma(x) = -1.214862905358496078218 + z^2*poly(z)
 *   2. Rational approximation in the primary interval [2,3]
 *      We use the following approximation:
 *              s = x-2.0;
 *              lgamma(x) = 0.5*s + s*lgammal_c__P(s)/lgammal_c__Q(s)
 *      Our algorithms are based on the following observation
 *
 *                             zeta(2)-1    2    zeta(3)-1    3
 * lgamma(2+s) = s*(1-Euler) + --------- * s  -  --------- * s  + ...
 *                                 2                 3
 *
 *      where Euler = 0.5771... is the Euler constant, which is very
 *      close to 0.5.
 *
 *   3. For x>=8, we have
 *      lgamma(x)~(x-0.5)log(x)-x+0.5*log(2pi)+1/(12x)-1/(360x**3)+....
 *      (better formula:
 *         lgamma(x)~(x-0.5)*(log(x)-1)-.5*(log(2pi)-1) + ...)
 *      Let z = 1/x, then we approximation
 *              f(z) = lgamma(x) - (x-0.5)(log(x)-1)
 *      by
 *                                  3       5             11
 *              w = lgammal_c__w0 + lgammal_c__w1*z + lgammal_c__w2*z  + lgammal_c__w3*z  + ... + lgammal_c__w6*z
 *
 *   4. For negative x, since (G is gamma function)
 *              -x*G(-x)*G(x) = lgammal_c__pi/sin(lgammal_c__pi*x),
 *      we have
 *              G(x) = lgammal_c__pi/(sin(lgammal_c__pi*x)*(-x)*G(-x))
 *      since G(-x) is positive, sign(G(x)) = sign(sin(lgammal_c__pi*x)) for x<0
 *      Hence, for x<0, signgam = sign(sin(lgammal_c__pi*x)) and
 *              lgamma(x) = log(|Gamma(x)|)
 *                        = log(lgammal_c__pi/(|x*sin(lgammal_c__pi*x)|)) - lgamma(-x);
 *      Note: one should avoid compute lgammal_c__pi*(-x) directly in the
 *            computation of sin(lgammal_c__pi*(-x)).
 *
 *   5. Special Cases
 *              lgamma(2+s) ~ s*(1-Euler) for lgammal_c__tiny s
 *              lgamma(1)=lgamma(2)=0
 *              lgamma(x) ~ -log(x) for lgammal_c__tiny x
 *              lgamma(0) = lgamma(inf) = inf
 *              lgamma(-integer) = +-inf
 *
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
double __lgamma_r(double x, int *sg);

long double __lgammal_r(long double x, int *sg) {
    return __lgamma_r(x, sg);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
static const long double
lgammal_c__pi = 3.14159265358979323846264L,

/* lgam(1+x) = 0.5 x + x a(x)/b(x)
    -0.268402099609375 <= x <= 0
    peak relative error 6.6e-22 */
lgammal_c__a0 = -6.343246574721079391729402781192128239938E2L,
lgammal_c__a1 =  1.856560238672465796768677717168371401378E3L,
lgammal_c__a2 =  2.404733102163746263689288466865843408429E3L,
lgammal_c__a3 =  8.804188795790383497379532868917517596322E2L,
lgammal_c__a4 =  1.135361354097447729740103745999661157426E2L,
lgammal_c__a5 =  3.766956539107615557608581581190400021285E0L,

b0 =  8.214973713960928795704317259806842490498E3L,
b1 =  1.026343508841367384879065363925870888012E4L,
b2 =  4.553337477045763320522762343132210919277E3L,
b3 =  8.506975785032585797446253359230031874803E2L,
b4 =  6.042447899703295436820744186992189445813E1L,
/* b5 =  1.000000000000000000000000000000000000000E0 */

lgammal_c__tc =  1.4616321449683623412626595423257213284682E0L,
lgammal_c__tf = -1.2148629053584961146050602565082954242826E-1, /* double precision */
/* lgammal_c__tt = (tail of lgammal_c__tf), i.e. lgammal_c__tf + lgammal_c__tt has extended precision. */
lgammal_c__tt = 3.3649914684731379602768989080467587736363E-18L,
/* lgam ( 1.4616321449683623412626595423257213284682E0 ) =
-1.2148629053584960809551455717769158215135617312999903886372437313313530E-1 */

/* lgam (x + lgammal_c__tc) = lgammal_c__tf + lgammal_c__tt + x g(x)/h(x)
    -0.230003726999612341262659542325721328468 <= x
       <= 0.2699962730003876587373404576742786715318
     peak relative error 2.1e-21 */
g0 = 3.645529916721223331888305293534095553827E-18L,
g1 = 5.126654642791082497002594216163574795690E3L,
g2 = 8.828603575854624811911631336122070070327E3L,
g3 = 5.464186426932117031234820886525701595203E3L,
g4 = 1.455427403530884193180776558102868592293E3L,
g5 = 1.541735456969245924860307497029155838446E2L,
g6 = 4.335498275274822298341872707453445815118E0L,

h0 = 1.059584930106085509696730443974495979641E4L,
h1 = 2.147921653490043010629481226937850618860E4L,
h2 = 1.643014770044524804175197151958100656728E4L,
h3 = 5.869021995186925517228323497501767586078E3L,
h4 = 9.764244777714344488787381271643502742293E2L,
h5 = 6.442485441570592541741092969581997002349E1L,
/* h6 = 1.000000000000000000000000000000000000000E0 */

/* lgam (x+1) = -0.5 x + x u(x)/v(x)
    -0.100006103515625 <= x <= 0.231639862060546875
    peak relative error 1.3e-21 */
lgammal_c__u0 = -8.886217500092090678492242071879342025627E1L,
lgammal_c__u1 =  6.840109978129177639438792958320783599310E2L,
lgammal_c__u2 =  2.042626104514127267855588786511809932433E3L,
lgammal_c__u3 =  1.911723903442667422201651063009856064275E3L,
lgammal_c__u4 =  7.447065275665887457628865263491667767695E2L,
lgammal_c__u5 =  1.132256494121790736268471016493103952637E2L,
lgammal_c__u6 =  4.484398885516614191003094714505960972894E0L,

lgammal_c__v0 =  1.150830924194461522996462401210374632929E3L,
lgammal_c__v1 =  3.399692260848747447377972081399737098610E3L,
lgammal_c__v2 =  3.786631705644460255229513563657226008015E3L,
lgammal_c__v3 =  1.966450123004478374557778781564114347876E3L,
lgammal_c__v4 =  4.741359068914069299837355438370682773122E2L,
lgammal_c__v5 =  4.508989649747184050907206782117647852364E1L,
/* lgammal_c__v6 =  1.000000000000000000000000000000000000000E0 */

/* lgam (x+2) = .5 x + x s(x)/r(x)
     0 <= x <= 1
     peak relative error 7.2e-22 */
lgammal_c__s0 =  1.454726263410661942989109455292824853344E6L,
lgammal_c__s1 = -3.901428390086348447890408306153378922752E6L,
lgammal_c__s2 = -6.573568698209374121847873064292963089438E6L,
lgammal_c__s3 = -3.319055881485044417245964508099095984643E6L,
lgammal_c__s4 = -7.094891568758439227560184618114707107977E5L,
lgammal_c__s5 = -6.263426646464505837422314539808112478303E4L,
lgammal_c__s6 = -1.684926520999477529949915657519454051529E3L,

lgammal_c__r0 = -1.883978160734303518163008696712983134698E7L,
lgammal_c__r1 = -2.815206082812062064902202753264922306830E7L,
lgammal_c__r2 = -1.600245495251915899081846093343626358398E7L,
lgammal_c__r3 = -4.310526301881305003489257052083370058799E6L,
lgammal_c__r4 = -5.563807682263923279438235987186184968542E5L,
lgammal_c__r5 = -3.027734654434169996032905158145259713083E4L,
lgammal_c__r6 = -4.501995652861105629217250715790764371267E2L,
/* lgammal_c__r6 =  1.000000000000000000000000000000000000000E0 */

/* lgam(x) = ( x - 0.5 ) * log(x) - x + LS2PI + 1/x w(1/x^2)
    x >= 8
    Peak relative error 1.51e-21
lgammal_c__w0 = LS2PI - 0.5 */
lgammal_c__w0 =  4.189385332046727417803e-1L,
lgammal_c__w1 =  8.333333333333331447505E-2L,
lgammal_c__w2 = -2.777777777750349603440E-3L,
lgammal_c__w3 =  7.936507795855070755671E-4L,
lgammal_c__w4 = -5.952345851765688514613E-4L,
lgammal_c__w5 =  8.412723297322498080632E-4L,
lgammal_c__w6 = -1.880801938119376907179E-3L,
lgammal_c__w7 =  4.885026142432270781165E-3L;

/* sin(lgammal_c__pi*x) assuming x > 2^-1000, if sin(lgammal_c__pi*x)==0 the sign is arbitrary */
static long double lgammal_c__sin_pi(long double x) {
    int n;

    /* spurious inexact if odd int */
    x *= 0.5;
    x = 2.0*(x - floorl(x));  /* x mod 2.0 */

    n = (int)(x*4.0);
    n = (n+1)/2;
    x -= n*0.5f;
    x *= lgammal_c__pi;

    switch (n) {
    default: /* case 4: */
    case 0: return __sinl(x, 0.0, 0);
    case 1: return __cosl(x, 0.0);
    case 2: return __sinl(-x, 0.0, 0);
    case 3: return -__cosl(x, 0.0);
    }
}

long double __lgammal_r(long double x, int *sg) {
    long double t, y, z, nadj, p, p1, p2, q, r, w;
    union ldshape u = {x};
    uint32_t ix = (u.i.se & 0x7fffU)<<16 | u.i.m>>48;
    int sign = u.i.se >> 15;
    int i;

    *sg = 1;

    /* purge off +-inf, NaN, +-0, lgammal_c__tiny and negative arguments */
    if (ix >= 0x7fff0000)
        return x * x;
    if (ix < 0x3fc08000) {  /* |x|<2**-63, return -log(|x|) */
        if (sign) {
            *sg = -1;
            x = -x;
        }
        return -logl(x);
    }
    if (sign) {
        x = -x;
        t = lgammal_c__sin_pi(x);
        if (t == 0.0)
            return 1.0 / (x-x); /* -integer */
        if (t > 0.0)
            *sg = -1;
        else
            t = -t;
        nadj = logl(lgammal_c__pi / (t * x));
    }

    /* purge off 1 and 2 (so the sign is ok with downward rounding) */
    if ((ix == 0x3fff8000 || ix == 0x40008000) && u.i.m == 0) {
        r = 0;
    } else if (ix < 0x40008000) {  /* x < 2.0 */
        if (ix <= 0x3ffee666) {  /* 8.99993896484375e-1 */
            /* lgamma(x) = lgamma(x+1) - log(x) */
            r = -logl(x);
            if (ix >= 0x3ffebb4a) {  /* 7.31597900390625e-1 */
                y = x - 1.0;
                i = 0;
            } else if (ix >= 0x3ffced33) {  /* 2.31639862060546875e-1 */
                y = x - (lgammal_c__tc - 1.0);
                i = 1;
            } else { /* x < 0.23 */
                y = x;
                i = 2;
            }
        } else {
            r = 0.0;
            if (ix >= 0x3fffdda6) {  /* 1.73162841796875 */
                /* [1.7316,2] */
                y = x - 2.0;
                i = 0;
            } else if (ix >= 0x3fff9da6) {  /* 1.23162841796875 */
                /* [1.23,1.73] */
                y = x - lgammal_c__tc;
                i = 1;
            } else {
                /* [0.9, 1.23] */
                y = x - 1.0;
                i = 2;
            }
        }
        switch (i) {
        case 0:
            p1 = lgammal_c__a0 + y * (lgammal_c__a1 + y * (lgammal_c__a2 + y * (lgammal_c__a3 + y * (lgammal_c__a4 + y * lgammal_c__a5))));
            p2 = b0 + y * (b1 + y * (b2 + y * (b3 + y * (b4 + y))));
            r += 0.5 * y + y * p1/p2;
            break;
        case 1:
            p1 = g0 + y * (g1 + y * (g2 + y * (g3 + y * (g4 + y * (g5 + y * g6)))));
            p2 = h0 + y * (h1 + y * (h2 + y * (h3 + y * (h4 + y * (h5 + y)))));
            p = lgammal_c__tt + y * p1/p2;
            r += (lgammal_c__tf + p);
            break;
        case 2:
            p1 = y * (lgammal_c__u0 + y * (lgammal_c__u1 + y * (lgammal_c__u2 + y * (lgammal_c__u3 + y * (lgammal_c__u4 + y * (lgammal_c__u5 + y * lgammal_c__u6))))));
            p2 = lgammal_c__v0 + y * (lgammal_c__v1 + y * (lgammal_c__v2 + y * (lgammal_c__v3 + y * (lgammal_c__v4 + y * (lgammal_c__v5 + y)))));
            r += (-0.5 * y + p1 / p2);
        }
    } else if (ix < 0x40028000) {  /* 8.0 */
        /* x < 8.0 */
        i = (int)x;
        y = x - (double)i;
        p = y * (lgammal_c__s0 + y * (lgammal_c__s1 + y * (lgammal_c__s2 + y * (lgammal_c__s3 + y * (lgammal_c__s4 + y * (lgammal_c__s5 + y * lgammal_c__s6))))));
        q = lgammal_c__r0 + y * (lgammal_c__r1 + y * (lgammal_c__r2 + y * (lgammal_c__r3 + y * (lgammal_c__r4 + y * (lgammal_c__r5 + y * (lgammal_c__r6 + y))))));
        r = 0.5 * y + p / q;
        z = 1.0;
        /* lgamma(1+s) = log(s) + lgamma(s) */
        switch (i) {
        case 7:
            z *= (y + 6.0); /* FALLTHRU */
        case 6:
            z *= (y + 5.0); /* FALLTHRU */
        case 5:
            z *= (y + 4.0); /* FALLTHRU */
        case 4:
            z *= (y + 3.0); /* FALLTHRU */
        case 3:
            z *= (y + 2.0); /* FALLTHRU */
            r += logl(z);
            break;
        }
    } else if (ix < 0x40418000) {  /* 2^66 */
        /* 8.0 <= x < 2**66 */
        t = logl(x);
        z = 1.0 / x;
        y = z * z;
        w = lgammal_c__w0 + z * (lgammal_c__w1 + y * (lgammal_c__w2 + y * (lgammal_c__w3 + y * (lgammal_c__w4 + y * (lgammal_c__w5 + y * (lgammal_c__w6 + y * lgammal_c__w7))))));
        r = (x - 0.5) * (t - 1.0) + w;
    } else /* 2**66 <= x <= inf */
        r = x * (logl(x) - 1.0);
    if (sign)
        r = nadj - r;
    return r;
}
#endif

#if (LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024) || (LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384)
extern int __signgam;

long double lgammal(long double x) {
    return __lgammal_r(x, &__signgam);
}

weak_alias(__lgammal_r, lgammal_r);
#endif

/// >>> START src/math/llrint.c

/* uses LLONG_MAX > 2^53, see comments in lrint.c */

long long llrint(double x) {
    return rint(x);
}

/// >>> START src/math/llrintf.c

/* uses LLONG_MAX > 2^24, see comments in lrint.c */

long long llrintf(float x) {
    return rintf(x);
}

/// >>> START src/math/llrintl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long long llrintl(long double x) {
    return llrint(x);
}
#elif defined(FE_INEXACT)
/*
see comments in lrint.c

Note that if LLONG_MAX == 0x7fffffffffffffff && LDBL_MANT_DIG == 64
then x == 2**63 - 0.5 is the only input that overflows and
raises inexact (with tonearest or upward rounding mode)
*/
long long llrintl(long double x) {
    int e;

    e = fetestexcept(FE_INEXACT);
    x = rintl(x);
    if (!e && (x > LLONG_MAX || x < LLONG_MIN))
        feclearexcept(FE_INEXACT);
    /* conversion */
    return x;
}
#else
long long llrintl(long double x) {
    return rintl(x);
}
#endif

/// >>> START src/math/llround.c

long long llround(double x) {
    return round(x);
}

/// >>> START src/math/llroundf.c

long long llroundf(float x) {
    return roundf(x);
}

/// >>> START src/math/llroundl.c

long long llroundl(long double x) {
    return roundl(x);
}

/// >>> START src/math/log.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_log.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* log(x)
 * Return the logarithm of x
 *
 * Method :
 *   1. Argument Reduction: find log_c__k and f such that
 *                      x = 2^log_c__k * (1+f),
 *         where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *      Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *               = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *               = 2s + s*log_c__R
 *      We use a special Remez algorithm on [0,0.1716] to generate
 *      a polynomial of degree 14 to approximate log_c__R The maximum error
 *      of this polynomial approximation is bounded by 2**-58.45. In
 *      other words,
 *                      2      4      6      8      10      12      14
 *          log_c__R(z) ~ log_c__Lg1*s +log_c__Lg2*s +log_c__Lg3*s +log_c__Lg4*s +log_c__Lg5*s  +log_c__Lg6*s  +log_c__Lg7*s
 *      (the values of log_c__Lg1 to log_c__Lg7 are listed in the program)
 *      and
 *          |      2          14          |     -58.45
 *          | log_c__Lg1*s +...+log_c__Lg7*s    -  log_c__R(z) | <= 2
 *          |                             |
 *      Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *      In order to guarantee error in log below 1ulp, we compute log
 *      by
 *              log(1+f) = f - s*(f - log_c__R)        (if f is not too large)
 *              log(1+f) = f - (hfsq - s*(hfsq+log_c__R)).     (better accuracy)
 *
 *      3. Finally,  log(x) = log_c__k*ln2 + log(1+f).
 *                          = log_c__k*log_c__ln2_hi+(f-(hfsq-(s*(hfsq+log_c__R)+log_c__k*log_c__ln2_lo)))
 *         Here ln2 is split into two floating point number:
 *                      log_c__ln2_hi + log_c__ln2_lo,
 *         where n*log_c__ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *      log(x) is NaN with signal if x < 0 (including -INF) ;
 *      log(+INF) is +INF; log(0) is -INF with signal;
 *      log(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
log_c__ln2_hi = 6.93147180369123816490e-01,  /* 3fe62e42 fee00000 */
log_c__ln2_lo = 1.90821492927058770002e-10,  /* 3dea39ef 35793c76 */
log_c__Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
log_c__Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
log_c__Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
log_c__Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
log_c__Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
log_c__Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
log_c__Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double log(double x) {
    union {double f; uint64_t i;} u = {x};
    double_t hfsq,f,s,z,log_c__R,w,log_c__t1,log_c__t2,dk;
    uint32_t hx;
    int log_c__k;

    hx = u.i>>32;
    log_c__k = 0;
    if (hx < 0x00100000 || hx>>31) {
        if (u.i<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (hx>>31)
            return (x-x)/0.0; /* log(-#) = NaN */
        /* subnormal number, scale x up */
        log_c__k -= 54;
        x *= 0x1p54;
        u.f = x;
        hx = u.i>>32;
    } else if (hx >= 0x7ff00000) {
        return x;
    } else if (hx == 0x3ff00000 && u.i<<32 == 0)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    hx += 0x3ff00000 - 0x3fe6a09e;
    log_c__k += (int)(hx>>20) - 0x3ff;
    hx = (hx&0x000fffff) + 0x3fe6a09e;
    u.i = (uint64_t)hx<<32 | (u.i&0xffffffff);
    x = u.f;

    f = x - 1.0;
    hfsq = 0.5*f*f;
    s = f/(2.0+f);
    z = s*s;
    w = z*z;
    log_c__t1 = w*(log_c__Lg2+w*(log_c__Lg4+w*log_c__Lg6));
    log_c__t2 = z*(log_c__Lg1+w*(log_c__Lg3+w*(log_c__Lg5+w*log_c__Lg7)));
    log_c__R = log_c__t2 + log_c__t1;
    dk = log_c__k;
    return s*(hfsq+log_c__R) + dk*log_c__ln2_lo - hfsq + f + dk*log_c__ln2_hi;
}

/// >>> START src/math/log10.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_log10.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * Return the base 10 logarithm of x.  See log.c for most comments.
 *
 * Reduce x to 2^log10_c__k (1+f) and calculate r = log(1+f) - f + f*f/2
 * as in log.c, then combine and scale in extra precision:
 *    log10(x) = (f - f*f/2 + r)/log(10) + log10_c__k*log10(2)
 */

static const double
log10_c__ivln10hi  = 4.34294481878168880939e-01, /* 0x3fdbcb7b, 0x15200000 */
log10_c__ivln10lo  = 2.50829467116452752298e-11, /* 0x3dbb9438, 0xca9aadd5 */
log10_c__log10_2hi = 3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
log10_c__log10_2lo = 3.69423907715893078616e-13, /* 0x3D59FEF3, 0x11F12B36 */
log10_c__Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
log10_c__Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
log10_c__Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
log10_c__Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
log10_c__Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
log10_c__Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
log10_c__Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double log10(double x) {
    union {double f; uint64_t i;} u = {x};
    double_t hfsq,f,s,z,log10_c__R,w,log10_c__t1,log10_c__t2,dk,y,hi,lo,val_hi,val_lo;
    uint32_t hx;
    int log10_c__k;

    hx = u.i>>32;
    log10_c__k = 0;
    if (hx < 0x00100000 || hx>>31) {
        if (u.i<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (hx>>31)
            return (x-x)/0.0; /* log(-#) = NaN */
        /* subnormal number, scale x up */
        log10_c__k -= 54;
        x *= 0x1p54;
        u.f = x;
        hx = u.i>>32;
    } else if (hx >= 0x7ff00000) {
        return x;
    } else if (hx == 0x3ff00000 && u.i<<32 == 0)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    hx += 0x3ff00000 - 0x3fe6a09e;
    log10_c__k += (int)(hx>>20) - 0x3ff;
    hx = (hx&0x000fffff) + 0x3fe6a09e;
    u.i = (uint64_t)hx<<32 | (u.i&0xffffffff);
    x = u.f;

    f = x - 1.0;
    hfsq = 0.5*f*f;
    s = f/(2.0+f);
    z = s*s;
    w = z*z;
    log10_c__t1 = w*(log10_c__Lg2+w*(log10_c__Lg4+w*log10_c__Lg6));
    log10_c__t2 = z*(log10_c__Lg1+w*(log10_c__Lg3+w*(log10_c__Lg5+w*log10_c__Lg7)));
    log10_c__R = log10_c__t2 + log10_c__t1;

    /* See log2.c for details. */
    /* hi+lo = f - hfsq + s*(hfsq+log10_c__R) ~ log(1+f) */
    hi = f - hfsq;
    u.f = hi;
    u.i &= (uint64_t)-1<<32;
    hi = u.f;
    lo = f - hi - hfsq + s*(hfsq+log10_c__R);

    /* val_hi+val_lo ~ log10(1+f) + log10_c__k*log10(2) */
    val_hi = hi*log10_c__ivln10hi;
    dk = log10_c__k;
    y = dk*log10_c__log10_2hi;
    val_lo = dk*log10_c__log10_2lo + (lo+hi)*log10_c__ivln10lo + lo*log10_c__ivln10hi;

    /*
     * Extra precision in for adding y is not strictly needed
     * since there is no very large cancellation near x = sqrt(2) or
     * x = 1/sqrt(2), but we do it anyway since it costs little on CPUs
     * with some parallelism and it reduces the error for many args.
     */
    w = y + val_hi;
    val_lo += (y - w) + val_hi;
    val_hi = w;

    return val_lo + val_hi;
}

/// >>> START src/math/log10f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_log10f.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in log10.c.
 */

static const float
log10f_c__ivln10hi  =  4.3432617188e-01, /* 0x3ede6000 */
log10f_c__ivln10lo  = -3.1689971365e-05, /* 0xb804ead9 */
log10f_c__log10_2hi =  3.0102920532e-01, /* 0x3e9a2080 */
log10f_c__log10_2lo =  7.9034151668e-07, /* 0x355427db */
/* |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11]). */
log10f_c__Lg1 = 0xaaaaaa.0p-24, /* 0.66666662693 */
log10f_c__Lg2 = 0xccce13.0p-25, /* 0.40000972152 */
log10f_c__Lg3 = 0x91e9ee.0p-25, /* 0.28498786688 */
log10f_c__Lg4 = 0xf89e26.0p-26; /* 0.24279078841 */

float log10f(float x) {
    union {float f; uint32_t i;} u = {x};
    float_t hfsq,f,s,z,log10f_c__R,w,log10f_c__t1,log10f_c__t2,dk,hi,lo;
    uint32_t ix;
    int log10f_c__k;

    ix = u.i;
    log10f_c__k = 0;
    if (ix < 0x00800000 || ix>>31) {  /* x < 2**-126  */
        if (ix<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (ix>>31)
            return (x-x)/0.0f; /* log(-#) = NaN */
        /* subnormal number, scale up x */
        log10f_c__k -= 25;
        x *= 0x1p25f;
        u.f = x;
        ix = u.i;
    } else if (ix >= 0x7f800000) {
        return x;
    } else if (ix == 0x3f800000)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    ix += 0x3f800000 - 0x3f3504f3;
    log10f_c__k += (int)(ix>>23) - 0x7f;
    ix = (ix&0x007fffff) + 0x3f3504f3;
    u.i = ix;
    x = u.f;

    f = x - 1.0f;
    s = f/(2.0f + f);
    z = s*s;
    w = z*z;
    log10f_c__t1= w*(log10f_c__Lg2+w*log10f_c__Lg4);
    log10f_c__t2= z*(log10f_c__Lg1+w*log10f_c__Lg3);
    log10f_c__R = log10f_c__t2 + log10f_c__t1;
    hfsq = 0.5f*f*f;

    hi = f - hfsq;
    u.f = hi;
    u.i &= 0xfffff000;
    hi = u.f;
    lo = f - hi - hfsq + s*(hfsq+log10f_c__R);
    dk = log10f_c__k;
    return dk*log10f_c__log10_2lo + (lo+hi)*log10f_c__ivln10lo + lo*log10f_c__ivln10hi + hi*log10f_c__ivln10hi + dk*log10f_c__log10_2hi;
}

/// >>> START src/math/log10l.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_log10l.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Common logarithm, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, log10l();
 *
 * y = log10l( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns the base 10 logarithm of x.
 *
 * The argument is separated into its exponent and fractional
 * parts.  If the exponent is between -1 and +1, the logarithm
 * of the fraction is approximated by
 *
 *     log(1+x) = x - 0.5 x**2 + x**3 log10l_c__P(x)/log10l_c__Q(x).
 *
 * Otherwise, setting  z = 2(x-1)/x+1),
 *
 *     log(x) = z + z**3 log10l_c__P(z)/log10l_c__Q(z).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0.5, 2.0     30000      9.0e-20     2.6e-20
 *    IEEE     exp(+-10000)  30000      6.0e-20     2.3e-20
 *
 * In the tests over the interval exp(+-10000), the logarithms
 * of the random arguments were uniformly distributed over
 * [-10000, +10000].
 *
 * ERROR MESSAGES:
 *
 * log singularity:  x = 0; returns MINLOG
 * log domain:       x < 0; returns MINLOG
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double log10l(long double x) {
    return log10(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* Coefficients for log(1+x) = x - x**2/2 + x**3 log10l_c__P(x)/log10l_c__Q(x)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.2e-22
 */
static const long double log10l_c__P[] = {
 4.9962495940332550844739E-1L,
 1.0767376367209449010438E1L,
 7.7671073698359539859595E1L,
 2.5620629828144409632571E2L,
 4.2401812743503691187826E2L,
 3.4258224542413922935104E2L,
 1.0747524399916215149070E2L,
};
static const long double log10l_c__Q[] = {
/* 1.0000000000000000000000E0,*/
 2.3479774160285863271658E1L,
 1.9444210022760132894510E2L,
 7.7952888181207260646090E2L,
 1.6911722418503949084863E3L,
 2.0307734695595183428202E3L,
 1.2695660352705325274404E3L,
 3.2242573199748645407652E2L,
};

/* Coefficients for log(x) = z + z^3 log10l_c__P(z^2)/log10l_c__Q(z^2),
 * where z = 2(x-1)/(x+1)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.16e-22
 */
static const long double log10l_c__R[4] = {
 1.9757429581415468984296E-3L,
-7.1990767473014147232598E-1L,
 1.0777257190312272158094E1L,
-3.5717684488096787370998E1L,
};
static const long double log10l_c__S[4] = {
/* 1.00000000000000000000E0L,*/
-2.6201045551331104417768E1L,
 1.9361891836232102174846E2L,
-4.2861221385716144629696E2L,
};
/* log10(2) */
#undef L102A
#define L102A 0.3125L
#undef L102B
#define L102B -1.1470004336018804786261e-2L
/* log10(e) */
#undef L10EA
#define L10EA 0.5L
#undef L10EB
#define L10EB -6.5705518096748172348871e-2L

#undef SQRTH
#define SQRTH 0.70710678118654752440L

long double log10l(long double x) {
    long double y, z;
    int e;

    if (isnan(x))
        return x;
    if(x <= 0.0) {
        if(x == 0.0)
            return -1.0 / (x*x);
        return (x - x) / 0.0;
    }
    if (x == INFINITY)
        return INFINITY;
    /* separate mantissa from exponent */
    /* Note, frexp is used so that denormal numbers
     * will be handled properly.
     */
    x = frexpl(x, &e);

    /* logarithm using log(x) = z + z**3 log10l_c__P(z)/log10l_c__Q(z),
     * where z = 2(x-1)/x+1)
     */
    if (e > 2 || e < -2) {
        if (x < SQRTH) {  /* 2(2x-1)/(2x+1) */
            e -= 1;
            z = x - 0.5;
            y = 0.5 * z + 0.5;
        } else {  /*  2 (x-1)/(x+1)   */
            z = x - 0.5;
            z -= 0.5;
            y = 0.5 * x  + 0.5;
        }
        x = z / y;
        z = x*x;
        y = x * (z * __polevll(z, log10l_c__R, 3) / __p1evll(z, log10l_c__S, 3));
        goto done;
    }

    /* logarithm using log(1+x) = x - .5x**2 + x**3 log10l_c__P(x)/log10l_c__Q(x) */
    if (x < SQRTH) {
        e -= 1;
        x = 2.0*x - 1.0;
    } else {
        x = x - 1.0;
    }
    z = x*x;
    y = x * (z * __polevll(x, log10l_c__P, 6) / __p1evll(x, log10l_c__Q, 7));
    y = y - 0.5*z;

done:
    /* Multiply log of fraction by log10(e)
     * and base 2 exponent by log10(2).
     *
     * ***CAUTION***
     *
     * This sequence of operations is critical and it may
     * be horribly defeated by some compiler optimizers.
     */
    z = y * (L10EB);
    z += x * (L10EB);
    z += e * (L102B);
    z += y * (L10EA);
    z += x * (L10EA);
    z += e * (L102A);
    return z;
}
#endif

/// >>> START src/math/log1p.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_log1p.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* double log1p(double x)
 * Return the natural logarithm of 1+x.
 *
 * Method :
 *   1. Argument Reduction: find log1p_c__k and f such that
 *                      1+x = 2^log1p_c__k * (1+f),
 *         where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *      Note. If log1p_c__k=0, then f=x is exact. However, if log1p_c__k!=0, then f
 *      may not be representable exactly. In that case, a correction
 *      term is need. Let u=1+x rounded. Let c = (1+x)-u, then
 *      log(1+x) - log(u) ~ c/u. Thus, we proceed to compute log(u),
 *      and add back the correction term c/u.
 *      (Note: when x > 2**53, one can simply return log(x))
 *
 *   2. Approximation of log(1+f): See log.c
 *
 *   3. Finally, log1p(x) = log1p_c__k*ln2 + log(1+f) + c/u. See log.c
 *
 * Special cases:
 *      log1p(x) is NaN with signal if x < -1 (including -INF) ;
 *      log1p(+INF) is +INF; log1p(-1) is -INF with signal;
 *      log1p(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 *
 * Note: Assuming log() return accurate answer, the following
 *       algorithm can be used to compute log1p(x) to within a few ULP:
 *
 *              u = 1+x;
 *              if(u==1.0) return x ; else
 *                         return log(u)*(x/(u-1.0));
 *
 *       See HP-15C Advanced Functions Handbook, p.193.
 */

static const double
log1p_c__ln2_hi = 6.93147180369123816490e-01,  /* 3fe62e42 fee00000 */
log1p_c__ln2_lo = 1.90821492927058770002e-10,  /* 3dea39ef 35793c76 */
log1p_c__Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
log1p_c__Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
log1p_c__Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
log1p_c__Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
log1p_c__Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
log1p_c__Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
log1p_c__Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double log1p(double x) {
    union {double f; uint64_t i;} u = {x};
    double_t hfsq,f,c,s,z,log1p_c__R,w,log1p_c__t1,log1p_c__t2,dk;
    uint32_t hx,hu;
    int log1p_c__k;

    hx = u.i>>32;
    log1p_c__k = 1;
    if (hx < 0x3fda827a || hx>>31) {  /* 1+x < sqrt(2)+ */
        if (hx >= 0xbff00000) {  /* x <= -1.0 */
            if (x == -1)
                return x/0.0; /* log1p(-1) = -inf */
            return (x-x)/0.0;     /* log1p(x<-1) = NaN */
        }
        if (hx<<1 < 0x3ca00000<<1) {  /* |x| < 2**-53 */
            /* underflow if subnormal */
            if ((hx&0x7ff00000) == 0)
                FORCE_EVAL((float)x);
            return x;
        }
        if (hx <= 0xbfd2bec4) {  /* sqrt(2)/2- <= 1+x < sqrt(2)+ */
            log1p_c__k = 0;
            c = 0;
            f = x;
        }
    } else if (hx >= 0x7ff00000)
        return x;
    if (log1p_c__k) {
        u.f = 1 + x;
        hu = u.i>>32;
        hu += 0x3ff00000 - 0x3fe6a09e;
        log1p_c__k = (int)(hu>>20) - 0x3ff;
        /* correction term ~ log(1+x)-log(u), avoid underflow in c/u */
        if (log1p_c__k < 54) {
            c = log1p_c__k >= 2 ? 1-(u.f-x) : x-(u.f-1);
            c /= u.f;
        } else
            c = 0;
        /* reduce u into [sqrt(2)/2, sqrt(2)] */
        hu = (hu&0x000fffff) + 0x3fe6a09e;
        u.i = (uint64_t)hu<<32 | (u.i&0xffffffff);
        f = u.f - 1;
    }
    hfsq = 0.5*f*f;
    s = f/(2.0+f);
    z = s*s;
    w = z*z;
    log1p_c__t1 = w*(log1p_c__Lg2+w*(log1p_c__Lg4+w*log1p_c__Lg6));
    log1p_c__t2 = z*(log1p_c__Lg1+w*(log1p_c__Lg3+w*(log1p_c__Lg5+w*log1p_c__Lg7)));
    log1p_c__R = log1p_c__t2 + log1p_c__t1;
    dk = log1p_c__k;
    return s*(hfsq+log1p_c__R) + (dk*log1p_c__ln2_lo+c) - hfsq + f + dk*log1p_c__ln2_hi;
}

/// >>> START src/math/log1pf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_log1pf.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
log1pf_c__ln2_hi = 6.9313812256e-01, /* 0x3f317180 */
log1pf_c__ln2_lo = 9.0580006145e-06, /* 0x3717f7d1 */
/* |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11]). */
log1pf_c__Lg1 = 0xaaaaaa.0p-24, /* 0.66666662693 */
log1pf_c__Lg2 = 0xccce13.0p-25, /* 0.40000972152 */
log1pf_c__Lg3 = 0x91e9ee.0p-25, /* 0.28498786688 */
log1pf_c__Lg4 = 0xf89e26.0p-26; /* 0.24279078841 */

float log1pf(float x) {
    union {float f; uint32_t i;} u = {x};
    float_t hfsq,f,c,s,z,log1pf_c__R,w,log1pf_c__t1,log1pf_c__t2,dk;
    uint32_t ix,iu;
    int log1pf_c__k;

    ix = u.i;
    log1pf_c__k = 1;
    if (ix < 0x3ed413d0 || ix>>31) {  /* 1+x < sqrt(2)+  */
        if (ix >= 0xbf800000) {  /* x <= -1.0 */
            if (x == -1)
                return x/0.0f; /* log1p(-1)=+inf */
            return (x-x)/0.0f;     /* log1p(x<-1)=NaN */
        }
        if (ix<<1 < 0x33800000<<1) {   /* |x| < 2**-24 */
            /* underflow if subnormal */
            if ((ix&0x7f800000) == 0)
                FORCE_EVAL(x*x);
            return x;
        }
        if (ix <= 0xbe95f619) { /* sqrt(2)/2- <= 1+x < sqrt(2)+ */
            log1pf_c__k = 0;
            c = 0;
            f = x;
        }
    } else if (ix >= 0x7f800000)
        return x;
    if (log1pf_c__k) {
        u.f = 1 + x;
        iu = u.i;
        iu += 0x3f800000 - 0x3f3504f3;
        log1pf_c__k = (int)(iu>>23) - 0x7f;
        /* correction term ~ log(1+x)-log(u), avoid underflow in c/u */
        if (log1pf_c__k < 25) {
            c = log1pf_c__k >= 2 ? 1-(u.f-x) : x-(u.f-1);
            c /= u.f;
        } else
            c = 0;
        /* reduce u into [sqrt(2)/2, sqrt(2)] */
        iu = (iu&0x007fffff) + 0x3f3504f3;
        u.i = iu;
        f = u.f - 1;
    }
    s = f/(2.0f + f);
    z = s*s;
    w = z*z;
    log1pf_c__t1= w*(log1pf_c__Lg2+w*log1pf_c__Lg4);
    log1pf_c__t2= z*(log1pf_c__Lg1+w*log1pf_c__Lg3);
    log1pf_c__R = log1pf_c__t2 + log1pf_c__t1;
    hfsq = 0.5f*f*f;
    dk = log1pf_c__k;
    return s*(hfsq+log1pf_c__R) + (dk*log1pf_c__ln2_lo+c) - hfsq + f + dk*log1pf_c__ln2_hi;
}

/// >>> START src/math/log1pl.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/s_log1pl.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Relative error logarithm
 *      Natural logarithm of 1+x, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, log1pl();
 *
 * y = log1pl( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns the base e (2.718...) logarithm of 1+x.
 *
 * The argument 1+x is separated into its exponent and fractional
 * parts.  If the exponent is between -1 and +1, the logarithm
 * of the fraction is approximated by
 *
 *     log(1+x) = x - 0.5 x^2 + x^3 log1pl_c__P(x)/log1pl_c__Q(x).
 *
 * Otherwise, setting  z = 2(x-1)/x+1),
 *
 *     log(x) = z + z^3 log1pl_c__P(z)/log1pl_c__Q(z).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     -1.0, 9.0    100000      8.2e-20    2.5e-20
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double log1pl(long double x) {
    return log1p(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* Coefficients for log(1+x) = x - x^2 / 2 + x^3 log1pl_c__P(x)/log1pl_c__Q(x)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 2.32e-20
 */
static const long double log1pl_c__P[] = {
 4.5270000862445199635215E-5L,
 4.9854102823193375972212E-1L,
 6.5787325942061044846969E0L,
 2.9911919328553073277375E1L,
 6.0949667980987787057556E1L,
 5.7112963590585538103336E1L,
 2.0039553499201281259648E1L,
};
static const long double log1pl_c__Q[] = {
/* 1.0000000000000000000000E0,*/
 1.5062909083469192043167E1L,
 8.3047565967967209469434E1L,
 2.2176239823732856465394E2L,
 3.0909872225312059774938E2L,
 2.1642788614495947685003E2L,
 6.0118660497603843919306E1L,
};

/* Coefficients for log(x) = z + z^3 log1pl_c__P(z^2)/log1pl_c__Q(z^2),
 * where z = 2(x-1)/(x+1)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.16e-22
 */
static const long double log1pl_c__R[4] = {
 1.9757429581415468984296E-3L,
-7.1990767473014147232598E-1L,
 1.0777257190312272158094E1L,
-3.5717684488096787370998E1L,
};
static const long double log1pl_c__S[4] = {
/* 1.00000000000000000000E0L,*/
-2.6201045551331104417768E1L,
 1.9361891836232102174846E2L,
-4.2861221385716144629696E2L,
};
static const long double log1pl_c__C1 = 6.9314575195312500000000E-1L;
static const long double log1pl_c__C2 = 1.4286068203094172321215E-6L;

#undef SQRTH
#define SQRTH 0.70710678118654752440L

long double log1pl(long double xm1) {
    long double x, y, z;
    int e;

    if (isnan(xm1))
        return xm1;
    if (xm1 == INFINITY)
        return xm1;
    if (xm1 == 0.0)
        return xm1;

    x = xm1 + 1.0;

    /* Test for domain errors.  */
    if (x <= 0.0) {
        if (x == 0.0)
            return -1/(x*x); /* -inf with divbyzero */
        return 0/0.0f; /* nan with invalid */
    }

    /* Separate mantissa from exponent.
       Use frexp so that denormal numbers will be handled properly.  */
    x = frexpl(x, &e);

    /* logarithm using log(x) = z + z^3 log1pl_c__P(z)/log1pl_c__Q(z),
       where z = 2(x-1)/x+1)  */
    if (e > 2 || e < -2) {
        if (x < SQRTH) { /* 2(2x-1)/(2x+1) */
            e -= 1;
            z = x - 0.5;
            y = 0.5 * z + 0.5;
        } else { /*  2 (x-1)/(x+1)   */
            z = x - 0.5;
            z -= 0.5;
            y = 0.5 * x  + 0.5;
        }
        x = z / y;
        z = x*x;
        z = x * (z * __polevll(z, log1pl_c__R, 3) / __p1evll(z, log1pl_c__S, 3));
        z = z + e * log1pl_c__C2;
        z = z + x;
        z = z + e * log1pl_c__C1;
        return z;
    }

    /* logarithm using log(1+x) = x - .5x**2 + x**3 log1pl_c__P(x)/log1pl_c__Q(x) */
    if (x < SQRTH) {
        e -= 1;
        if (e != 0)
            x = 2.0 * x - 1.0;
        else
            x = xm1;
    } else {
        if (e != 0)
            x = x - 1.0;
        else
            x = xm1;
    }
    z = x*x;
    y = x * (z * __polevll(x, log1pl_c__P, 6) / __p1evll(x, log1pl_c__Q, 6));
    y = y + e * log1pl_c__C2;
    z = y - 0.5 * z;
    z = z + x;
    z = z + e * log1pl_c__C1;
    return z;
}
#endif

/// >>> START src/math/log2.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_log2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * Return the base 2 logarithm of x.  See log.c for most comments.
 *
 * Reduce x to 2^log2_c__k (1+f) and calculate r = log(1+f) - f + f*f/2
 * as in log.c, then combine and scale in extra precision:
 *    log2(x) = (f - f*f/2 + r)/log(2) + log2_c__k
 */

static const double
log2_c__ivln2hi = 1.44269504072144627571e+00, /* 0x3ff71547, 0x65200000 */
log2_c__ivln2lo = 1.67517131648865118353e-10, /* 0x3de705fc, 0x2eefa200 */
log2_c__Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
log2_c__Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
log2_c__Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
log2_c__Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
log2_c__Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
log2_c__Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
log2_c__Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double log2(double x) {
    union {double f; uint64_t i;} u = {x};
    double_t hfsq,f,s,z,log2_c__R,w,log2_c__t1,log2_c__t2,y,hi,lo,val_hi,val_lo;
    uint32_t hx;
    int log2_c__k;

    hx = u.i>>32;
    log2_c__k = 0;
    if (hx < 0x00100000 || hx>>31) {
        if (u.i<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (hx>>31)
            return (x-x)/0.0; /* log(-#) = NaN */
        /* subnormal number, scale x up */
        log2_c__k -= 54;
        x *= 0x1p54;
        u.f = x;
        hx = u.i>>32;
    } else if (hx >= 0x7ff00000) {
        return x;
    } else if (hx == 0x3ff00000 && u.i<<32 == 0)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    hx += 0x3ff00000 - 0x3fe6a09e;
    log2_c__k += (int)(hx>>20) - 0x3ff;
    hx = (hx&0x000fffff) + 0x3fe6a09e;
    u.i = (uint64_t)hx<<32 | (u.i&0xffffffff);
    x = u.f;

    f = x - 1.0;
    hfsq = 0.5*f*f;
    s = f/(2.0+f);
    z = s*s;
    w = z*z;
    log2_c__t1 = w*(log2_c__Lg2+w*(log2_c__Lg4+w*log2_c__Lg6));
    log2_c__t2 = z*(log2_c__Lg1+w*(log2_c__Lg3+w*(log2_c__Lg5+w*log2_c__Lg7)));
    log2_c__R = log2_c__t2 + log2_c__t1;

    /*
     * f-hfsq must (for args near 1) be evaluated in extra precision
     * to avoid a large cancellation when x is near sqrt(2) or 1/sqrt(2).
     * This is fairly efficient since f-hfsq only depends on f, so can
     * be evaluated in parallel with log2_c__R.  Not combining hfsq with log2_c__R also
     * keeps log2_c__R small (though not as small as a true `lo' term would be),
     * so that extra precision is not needed for terms involving log2_c__R.
     *
     * Compiler bugs involving extra precision used to break Dekker's
     * theorem for spitting f-hfsq as hi+lo, unless double_t was used
     * or the multi-precision calculations were avoided when double_t
     * has extra precision.  These problems are now automatically
     * avoided as a side effect of the optimization of combining the
     * Dekker splitting step with the clear-low-bits step.
     *
     * y must (for args near sqrt(2) and 1/sqrt(2)) be added in extra
     * precision to avoid a very large cancellation when x is very near
     * these values.  Unlike the above cancellations, this problem is
     * specific to base 2.  It is strange that adding +-1 is so much
     * harder than adding +-ln2 or +-log10_2.
     *
     * This uses Dekker's theorem to normalize y+val_hi, so the
     * compiler bugs are back in some configurations, sigh.  And I
     * don't want to used double_t to avoid them, since that gives a
     * pessimization and the support for avoiding the pessimization
     * is not yet available.
     *
     * The multi-precision calculations for the multiplications are
     * routine.
     */

    /* hi+lo = f - hfsq + s*(hfsq+log2_c__R) ~ log(1+f) */
    hi = f - hfsq;
    u.f = hi;
    u.i &= (uint64_t)-1<<32;
    hi = u.f;
    lo = f - hi - hfsq + s*(hfsq+log2_c__R);

    val_hi = hi*log2_c__ivln2hi;
    val_lo = (lo+hi)*log2_c__ivln2lo + lo*log2_c__ivln2hi;

    /* spadd(val_hi, val_lo, y), except for not using double_t: */
    y = log2_c__k;
    w = y + val_hi;
    val_lo += (y - w) + val_hi;
    val_hi = w;

    return val_lo + val_hi;
}

/// >>> START src/math/log2f.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_log2f.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in log2.c.
 */

static const float
log2f_c__ivln2hi =  1.4428710938e+00, /* 0x3fb8b000 */
log2f_c__ivln2lo = -1.7605285393e-04, /* 0xb9389ad4 */
/* |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11]). */
log2f_c__Lg1 = 0xaaaaaa.0p-24, /* 0.66666662693 */
log2f_c__Lg2 = 0xccce13.0p-25, /* 0.40000972152 */
log2f_c__Lg3 = 0x91e9ee.0p-25, /* 0.28498786688 */
log2f_c__Lg4 = 0xf89e26.0p-26; /* 0.24279078841 */

float log2f(float x) {
    union {float f; uint32_t i;} u = {x};
    float_t hfsq,f,s,z,log2f_c__R,w,log2f_c__t1,log2f_c__t2,hi,lo;
    uint32_t ix;
    int log2f_c__k;

    ix = u.i;
    log2f_c__k = 0;
    if (ix < 0x00800000 || ix>>31) {  /* x < 2**-126  */
        if (ix<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (ix>>31)
            return (x-x)/0.0f; /* log(-#) = NaN */
        /* subnormal number, scale up x */
        log2f_c__k -= 25;
        x *= 0x1p25f;
        u.f = x;
        ix = u.i;
    } else if (ix >= 0x7f800000) {
        return x;
    } else if (ix == 0x3f800000)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    ix += 0x3f800000 - 0x3f3504f3;
    log2f_c__k += (int)(ix>>23) - 0x7f;
    ix = (ix&0x007fffff) + 0x3f3504f3;
    u.i = ix;
    x = u.f;

    f = x - 1.0f;
    s = f/(2.0f + f);
    z = s*s;
    w = z*z;
    log2f_c__t1= w*(log2f_c__Lg2+w*log2f_c__Lg4);
    log2f_c__t2= z*(log2f_c__Lg1+w*log2f_c__Lg3);
    log2f_c__R = log2f_c__t2 + log2f_c__t1;
    hfsq = 0.5f*f*f;

    hi = f - hfsq;
    u.f = hi;
    u.i &= 0xfffff000;
    hi = u.f;
    lo = f - hi - hfsq + s*(hfsq+log2f_c__R);
    return (lo+hi)*log2f_c__ivln2lo + lo*log2f_c__ivln2hi + hi*log2f_c__ivln2hi + log2f_c__k;
}

/// >>> START src/math/log2l.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_log2l.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Base 2 logarithm, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, log2l();
 *
 * y = log2l( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns the base 2 logarithm of x.
 *
 * The argument is separated into its exponent and fractional
 * parts.  If the exponent is between -1 and +1, the (natural)
 * logarithm of the fraction is approximated by
 *
 *     log(1+x) = x - 0.5 x**2 + x**3 log2l_c__P(x)/log2l_c__Q(x).
 *
 * Otherwise, setting  z = 2(x-1)/x+1),
 *
 *     log(x) = z + z**3 log2l_c__P(z)/log2l_c__Q(z).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0.5, 2.0     30000      9.8e-20     2.7e-20
 *    IEEE     exp(+-10000)  70000      5.4e-20     2.3e-20
 *
 * In the tests over the interval exp(+-10000), the logarithms
 * of the random arguments were uniformly distributed over
 * [-10000, +10000].
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double log2l(long double x) {
    return log2(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* Coefficients for ln(1+x) = x - x**2/2 + x**3 log2l_c__P(x)/log2l_c__Q(x)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.2e-22
 */
static const long double log2l_c__P[] = {
 4.9962495940332550844739E-1L,
 1.0767376367209449010438E1L,
 7.7671073698359539859595E1L,
 2.5620629828144409632571E2L,
 4.2401812743503691187826E2L,
 3.4258224542413922935104E2L,
 1.0747524399916215149070E2L,
};
static const long double log2l_c__Q[] = {
/* 1.0000000000000000000000E0,*/
 2.3479774160285863271658E1L,
 1.9444210022760132894510E2L,
 7.7952888181207260646090E2L,
 1.6911722418503949084863E3L,
 2.0307734695595183428202E3L,
 1.2695660352705325274404E3L,
 3.2242573199748645407652E2L,
};

/* Coefficients for log(x) = z + z^3 log2l_c__P(z^2)/log2l_c__Q(z^2),
 * where z = 2(x-1)/(x+1)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.16e-22
 */
static const long double log2l_c__R[4] = {
 1.9757429581415468984296E-3L,
-7.1990767473014147232598E-1L,
 1.0777257190312272158094E1L,
-3.5717684488096787370998E1L,
};
static const long double log2l_c__S[4] = {
/* 1.00000000000000000000E0L,*/
-2.6201045551331104417768E1L,
 1.9361891836232102174846E2L,
-4.2861221385716144629696E2L,
};
/* log2(e) - 1 */
#undef log2l_c__LOG2EA
#define log2l_c__LOG2EA 4.4269504088896340735992e-1L

#undef SQRTH
#define SQRTH 0.70710678118654752440L

long double log2l(long double x) {
    long double y, z;
    int e;

    if (isnan(x))
        return x;
    if (x == INFINITY)
        return x;
    if (x <= 0.0) {
        if (x == 0.0)
            return -1/(x*x); /* -inf with divbyzero */
        return 0/0.0f; /* nan with invalid */
    }

    /* separate mantissa from exponent */
    /* Note, frexp is used so that denormal numbers
     * will be handled properly.
     */
    x = frexpl(x, &e);

    /* logarithm using log(x) = z + z**3 log2l_c__P(z)/log2l_c__Q(z),
     * where z = 2(x-1)/x+1)
     */
    if (e > 2 || e < -2) {
        if (x < SQRTH) {  /* 2(2x-1)/(2x+1) */
            e -= 1;
            z = x - 0.5;
            y = 0.5 * z + 0.5;
        } else {  /*  2 (x-1)/(x+1)   */
            z = x - 0.5;
            z -= 0.5;
            y = 0.5 * x + 0.5;
        }
        x = z / y;
        z = x*x;
        y = x * (z * __polevll(z, log2l_c__R, 3) / __p1evll(z, log2l_c__S, 3));
        goto done;
    }

    /* logarithm using log(1+x) = x - .5x**2 + x**3 log2l_c__P(x)/log2l_c__Q(x) */
    if (x < SQRTH) {
        e -= 1;
        x = 2.0*x - 1.0;
    } else {
        x = x - 1.0;
    }
    z = x*x;
    y = x * (z * __polevll(x, log2l_c__P, 6) / __p1evll(x, log2l_c__Q, 7));
    y = y - 0.5*z;

done:
    /* Multiply log of fraction by log2(e)
     * and base 2 exponent by 1
     *
     * ***CAUTION***
     *
     * This sequence of operations is critical and it may
     * be horribly defeated by some compiler optimizers.
     */
    z = y * log2l_c__LOG2EA;
    z += x * log2l_c__LOG2EA;
    z += y;
    z += x;
    z += e;
    return z;
}
#endif

/// >>> START src/math/logb.c

/*
special cases:
    logb(+-0) = -inf, and raise divbyzero
    logb(+-inf) = +inf
    logb(nan) = nan
*/

double logb(double x) {
    if (!isfinite(x))
        return x * x;
    if (x == 0)
        return -1/(x*x);
    return ilogb(x);
}

/// >>> START src/math/logbf.c

float logbf(float x) {
    if (!isfinite(x))
        return x * x;
    if (x == 0)
        return -1/(x*x);
    return ilogbf(x);
}

/// >>> START src/math/logbl.c
#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double logbl(long double x) {
    return logb(x);
}
#else
long double logbl(long double x) {
    if (!isfinite(x))
        return x * x;
    if (x == 0)
        return -1/(x*x);
    return ilogbl(x);
}
#endif

/// >>> START src/math/logf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_logf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
logf_c__ln2_hi = 6.9313812256e-01, /* 0x3f317180 */
logf_c__ln2_lo = 9.0580006145e-06, /* 0x3717f7d1 */
/* |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11]). */
logf_c__Lg1 = 0xaaaaaa.0p-24, /* 0.66666662693 */
logf_c__Lg2 = 0xccce13.0p-25, /* 0.40000972152 */
logf_c__Lg3 = 0x91e9ee.0p-25, /* 0.28498786688 */
logf_c__Lg4 = 0xf89e26.0p-26; /* 0.24279078841 */

float logf(float x) {
    union {float f; uint32_t i;} u = {x};
    float_t hfsq,f,s,z,logf_c__R,w,logf_c__t1,logf_c__t2,dk;
    uint32_t ix;
    int logf_c__k;

    ix = u.i;
    logf_c__k = 0;
    if (ix < 0x00800000 || ix>>31) {  /* x < 2**-126  */
        if (ix<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (ix>>31)
            return (x-x)/0.0f; /* log(-#) = NaN */
        /* subnormal number, scale up x */
        logf_c__k -= 25;
        x *= 0x1p25f;
        u.f = x;
        ix = u.i;
    } else if (ix >= 0x7f800000) {
        return x;
    } else if (ix == 0x3f800000)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    ix += 0x3f800000 - 0x3f3504f3;
    logf_c__k += (int)(ix>>23) - 0x7f;
    ix = (ix&0x007fffff) + 0x3f3504f3;
    u.i = ix;
    x = u.f;

    f = x - 1.0f;
    s = f/(2.0f + f);
    z = s*s;
    w = z*z;
    logf_c__t1= w*(logf_c__Lg2+w*logf_c__Lg4);
    logf_c__t2= z*(logf_c__Lg1+w*logf_c__Lg3);
    logf_c__R = logf_c__t2 + logf_c__t1;
    hfsq = 0.5f*f*f;
    dk = logf_c__k;
    return s*(hfsq+logf_c__R) + dk*logf_c__ln2_lo - hfsq + f + dk*logf_c__ln2_hi;
}

/// >>> START src/math/logl.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_logl.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Natural logarithm, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, logl();
 *
 * y = logl( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns the base e (2.718...) logarithm of x.
 *
 * The argument is separated into its exponent and fractional
 * parts.  If the exponent is between -1 and +1, the logarithm
 * of the fraction is approximated by
 *
 *     log(1+x) = x - 0.5 x**2 + x**3 logl_c__P(x)/logl_c__Q(x).
 *
 * Otherwise, setting  z = 2(x-1)/(x+1),
 *
 *     log(x) = log(1+z/2) - log(1-z/2) = z + z**3 logl_c__P(z)/logl_c__Q(z).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0.5, 2.0    150000      8.71e-20    2.75e-20
 *    IEEE     exp(+-10000) 100000      5.39e-20    2.34e-20
 *
 * In the tests over the interval exp(+-10000), the logarithms
 * of the random arguments were uniformly distributed over
 * [-10000, +10000].
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double logl(long double x) {
    return log(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* Coefficients for log(1+x) = x - x**2/2 + x**3 logl_c__P(x)/logl_c__Q(x)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 2.32e-20
 */
static const long double logl_c__P[] = {
 4.5270000862445199635215E-5L,
 4.9854102823193375972212E-1L,
 6.5787325942061044846969E0L,
 2.9911919328553073277375E1L,
 6.0949667980987787057556E1L,
 5.7112963590585538103336E1L,
 2.0039553499201281259648E1L,
};
static const long double logl_c__Q[] = {
/* 1.0000000000000000000000E0,*/
 1.5062909083469192043167E1L,
 8.3047565967967209469434E1L,
 2.2176239823732856465394E2L,
 3.0909872225312059774938E2L,
 2.1642788614495947685003E2L,
 6.0118660497603843919306E1L,
};

/* Coefficients for log(x) = z + z^3 logl_c__P(z^2)/logl_c__Q(z^2),
 * where z = 2(x-1)/(x+1)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.16e-22
 */
static const long double logl_c__R[4] = {
 1.9757429581415468984296E-3L,
-7.1990767473014147232598E-1L,
 1.0777257190312272158094E1L,
-3.5717684488096787370998E1L,
};
static const long double logl_c__S[4] = {
/* 1.00000000000000000000E0L,*/
-2.6201045551331104417768E1L,
 1.9361891836232102174846E2L,
-4.2861221385716144629696E2L,
};
static const long double logl_c__C1 = 6.9314575195312500000000E-1L;
static const long double logl_c__C2 = 1.4286068203094172321215E-6L;

#undef SQRTH
#define SQRTH 0.70710678118654752440L

long double logl(long double x) {
    long double y, z;
    int e;

    if (isnan(x))
        return x;
    if (x == INFINITY)
        return x;
    if (x <= 0.0) {
        if (x == 0.0)
            return -1/(x*x); /* -inf with divbyzero */
        return 0/0.0f; /* nan with invalid */
    }

    /* separate mantissa from exponent */
    /* Note, frexp is used so that denormal numbers
     * will be handled properly.
     */
    x = frexpl(x, &e);

    /* logarithm using log(x) = z + z**3 logl_c__P(z)/logl_c__Q(z),
     * where z = 2(x-1)/(x+1)
     */
    if (e > 2 || e < -2) {
        if (x < SQRTH) {  /* 2(2x-1)/(2x+1) */
            e -= 1;
            z = x - 0.5;
            y = 0.5 * z + 0.5;
        } else {  /*  2 (x-1)/(x+1)   */
            z = x - 0.5;
            z -= 0.5;
            y = 0.5 * x  + 0.5;
        }
        x = z / y;
        z = x*x;
        z = x * (z * __polevll(z, logl_c__R, 3) / __p1evll(z, logl_c__S, 3));
        z = z + e * logl_c__C2;
        z = z + x;
        z = z + e * logl_c__C1;
        return z;
    }

    /* logarithm using log(1+x) = x - .5x**2 + x**3 logl_c__P(x)/logl_c__Q(x) */
    if (x < SQRTH) {
        e -= 1;
        x = 2.0*x - 1.0;
    } else {
        x = x - 1.0;
    }
    z = x*x;
    y = x * (z * __polevll(x, logl_c__P, 6) / __p1evll(x, logl_c__Q, 6));
    y = y + e * logl_c__C2;
    z = y - 0.5*z;
    /* Note, the sum of above terms does not exceed x/4,
     * so it contributes at most about 1/4 lsb to the error.
     */
    z = z + x;
    z = z + e * logl_c__C1; /* This sum has an error of 1/2 lsb. */
    return z;
}
#endif

/// >>> START src/math/lrint.c

/*
If the result cannot be represented (overflow, nan), then
lrint raises the invalid exception.

Otherwise if the input was not an integer then the inexact
exception is raised.

C99 is a bit vague about whether inexact exception is
allowed to be raised when invalid is raised.
(F.9 explicitly allows spurious inexact exceptions, F.9.6.5
does not make it clear if that rule applies to lrint, but
IEEE 754r 7.8 seems to forbid spurious inexact exception in
the ineger conversion functions)

So we lrint_c__try to make sure that no spurious inexact exception is
raised in case of an overflow.

If the bit size of long > precision of double, then there
cannot be inexact rounding in case the result overflows,
otherwise LONG_MAX and LONG_MIN can be represented exactly
as a double.
*/

#if LONG_MAX < 1U<<53 && defined(FE_INEXACT)
long lrint(double x) {
    int e;

    e = fetestexcept(FE_INEXACT);
    x = rint(x);
    if (!e && (x > LONG_MAX || x < LONG_MIN))
        feclearexcept(FE_INEXACT);
    /* conversion */
    return x;
}
#else
long lrint(double x) {
    return rint(x);
}
#endif

/// >>> START src/math/lrintf.c

/* uses LONG_MAX > 2^24, see comments in lrint.c */

long lrintf(float x) {
    return rintf(x);
}

/// >>> START src/math/lrintl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long lrintl(long double x) {
    return lrint(x);
}
#elif defined(FE_INEXACT)
/*
see comments in lrint.c

Note that if LONG_MAX == 0x7fffffffffffffff && LDBL_MANT_DIG == 64
then x == 2**63 - 0.5 is the only input that overflows and
raises inexact (with tonearest or upward rounding mode)
*/
long lrintl(long double x) {
    int e;

    e = fetestexcept(FE_INEXACT);
    x = rintl(x);
    if (!e && (x > LONG_MAX || x < LONG_MIN))
        feclearexcept(FE_INEXACT);
    /* conversion */
    return x;
}
#else
long lrintl(long double x) {
    return rintl(x);
}
#endif

/// >>> START src/math/lround.c

long lround(double x) {
    return round(x);
}

/// >>> START src/math/lroundf.c

long lroundf(float x) {
    return roundf(x);
}

/// >>> START src/math/lroundl.c

long lroundl(long double x) {
    return roundl(x);
}

/// >>> START src/math/modf.c

double modf(double x, double *iptr) {
    union {double f; uint64_t i;} u = {x};
    uint64_t mask;
    int e = (int)(u.i>>52 & 0x7ff) - 0x3ff;

    /* no fractional part */
    if (e >= 52) {
        *iptr = x;
        if (e == 0x400 && u.i<<12 != 0) /* nan */
            return x;
        u.i &= 1ULL<<63;
        return u.f;
    }

    /* no integral part*/
    if (e < 0) {
        u.i &= 1ULL<<63;
        *iptr = u.f;
        return x;
    }

    mask = -1ULL>>12>>e;
    if ((u.i & mask) == 0) {
        *iptr = x;
        u.i &= 1ULL<<63;
        return u.f;
    }
    u.i &= ~mask;
    *iptr = u.f;
    return x - u.f;
}

/// >>> START src/math/modff.c

float modff(float x, float *iptr) {
    union {float f; uint32_t i;} u = {x};
    uint32_t mask;
    int e = (int)(u.i>>23 & 0xff) - 0x7f;

    /* no fractional part */
    if (e >= 23) {
        *iptr = x;
        if (e == 0x80 && u.i<<9 != 0) { /* nan */
            return x;
        }
        u.i &= 0x80000000;
        return u.f;
    }
    /* no integral part */
    if (e < 0) {
        u.i &= 0x80000000;
        *iptr = u.f;
        return x;
    }

    mask = 0x007fffff>>e;
    if ((u.i & mask) == 0) {
        *iptr = x;
        u.i &= 0x80000000;
        return u.f;
    }
    u.i &= ~mask;
    *iptr = u.f;
    return x - u.f;
}

/// >>> START src/math/modfl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double modfl(long double x, long double *iptr) {
    return modf(x, (double *)iptr);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double modfl(long double x, long double *iptr) {
    union ldshape u = {x};
    int e = (u.i.se & 0x7fff) - 0x3fff;
    int s = u.i.se >> 15;
    long double absx;
    long double y;

    /* no fractional part */
    if (e >= LDBL_MANT_DIG-1) {
        *iptr = x;
        if (isnan(x))
            return x;
        return s ? -0.0 : 0.0;
    }

    /* no integral part*/
    if (e < 0) {
        *iptr = s ? -0.0 : 0.0;
        return x;
    }

    /* raises spurious inexact */
    absx = s ? -x : x;
    y = absx + TOINT - TOINT - absx;
    if (y == 0) {
        *iptr = x;
        return s ? -0.0 : 0.0;
    }
    if (y > 0)
        y -= 1;
    if (s)
        y = -y;
    *iptr = x + y;
    return -y;
}
#endif

/// >>> START src/math/nan.c

double nan(const char *s) {
    return NAN;
}

/// >>> START src/math/nanf.c

float nanf(const char *s) {
    return NAN;
}

/// >>> START src/math/nanl.c

long double nanl(const char *s) {
    return NAN;
}

/// >>> START src/math/nearbyint.c

/* nearbyint is the same as rint, but it must not raise the inexact exception */

double nearbyint(double x) {
#ifdef FE_INEXACT
    int e;

    e = fetestexcept(FE_INEXACT);
#endif
    x = rint(x);
#ifdef FE_INEXACT
    if (!e)
        feclearexcept(FE_INEXACT);
#endif
    return x;
}

/// >>> START src/math/nearbyintf.c

float nearbyintf(float x) {
#ifdef FE_INEXACT
    int e;

    e = fetestexcept(FE_INEXACT);
#endif
    x = rintf(x);
#ifdef FE_INEXACT
    if (!e)
        feclearexcept(FE_INEXACT);
#endif
    return x;
}

/// >>> START src/math/nearbyintl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double nearbyintl(long double x) {
    return nearbyint(x);
}
#else
long double nearbyintl(long double x) {
#ifdef FE_INEXACT
    int e;

    e = fetestexcept(FE_INEXACT);
#endif
    x = rintl(x);
#ifdef FE_INEXACT
    if (!e)
        feclearexcept(FE_INEXACT);
#endif
    return x;
}
#endif

/// >>> START src/math/nextafter.c

double nextafter(double x, double y) {
    union {double f; uint64_t i;} ux={x}, uy={y};
    uint64_t ax, ay;
    int e;

    if (isnan(x) || isnan(y))
        return x + y;
    if (ux.i == uy.i)
        return y;
    ax = ux.i & -1ULL/2;
    ay = uy.i & -1ULL/2;
    if (ax == 0) {
        if (ay == 0)
            return y;
        ux.i = (uy.i & 1ULL<<63) | 1;
    } else if (ax > ay || ((ux.i ^ uy.i) & 1ULL<<63))
        ux.i--;
    else
        ux.i++;
    e = ux.i >> 52 & 0x7ff;
    /* raise overflow if ux.f is infinite and x is finite */
    if (e == 0x7ff)
        FORCE_EVAL(x+x);
    /* raise underflow if ux.f is subnormal or nextafter_c__zero */
    if (e == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}

/// >>> START src/math/nextafterf.c

float nextafterf(float x, float y) {
    union {float f; uint32_t i;} ux={x}, uy={y};
    uint32_t ax, ay, e;

    if (isnan(x) || isnan(y))
        return x + y;
    if (ux.i == uy.i)
        return y;
    ax = ux.i & 0x7fffffff;
    ay = uy.i & 0x7fffffff;
    if (ax == 0) {
        if (ay == 0)
            return y;
        ux.i = (uy.i & 0x80000000) | 1;
    } else if (ax > ay || ((ux.i ^ uy.i) & 0x80000000))
        ux.i--;
    else
        ux.i++;
    e = ux.i & 0x7f800000;
    /* raise overflow if ux.f is infinite and x is finite */
    if (e == 0x7f800000)
        FORCE_EVAL(x+x);
    /* raise underflow if ux.f is subnormal or nextafterf_c__zero */
    if (e == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}

/// >>> START src/math/nextafterl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double nextafterl(long double x, long double y) {
    return nextafter(x, y);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
long double nextafterl(long double x, long double y) {
    union ldshape ux, uy;

    if (isnan(x) || isnan(y))
        return x + y;
    if (x == y)
        return y;
    ux.f = x;
    if (x == 0) {
        uy.f = y;
        ux.i.m = 1;
        ux.i.se = uy.i.se & 0x8000;
    } else if ((x < y) == !(ux.i.se & 0x8000)) {
        ux.i.m++;
        if (ux.i.m << 1 == 0) {
            ux.i.m = 1ULL << 63;
            ux.i.se++;
        }
    } else {
        if (ux.i.m << 1 == 0) {
            ux.i.se--;
            if (ux.i.se)
                ux.i.m = 0;
        }
        ux.i.m--;
    }
    /* raise overflow if ux is infinite and x is finite */
    if ((ux.i.se & 0x7fff) == 0x7fff)
        return x + x;
    /* raise underflow if ux is subnormal or nextafterl_c__zero */
    if ((ux.i.se & 0x7fff) == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
long double nextafterl(long double x, long double y) {
    union ldshape ux, uy;

    if (isnan(x) || isnan(y))
        return x + y;
    if (x == y)
        return y;
    ux.f = x;
    if (x == 0) {
        uy.f = y;
        ux.i.lo = 1;
        ux.i.se = uy.i.se & 0x8000;
    } else if ((x < y) == !(ux.i.se & 0x8000)) {
        ux.i2.lo++;
        if (ux.i2.lo == 0)
            ux.i2.hi++;
    } else {
        if (ux.i2.lo == 0)
            ux.i2.hi--;
        ux.i2.lo--;
    }
    /* raise overflow if ux is infinite and x is finite */
    if ((ux.i.se & 0x7fff) == 0x7fff)
        return x + x;
    /* raise underflow if ux is subnormal or nextafterl_c__zero */
    if ((ux.i.se & 0x7fff) == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}
#endif

/// >>> START src/math/nexttoward.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
double nexttoward(double x, long double y) {
    return nextafter(x, y);
}
#else
double nexttoward(double x, long double y) {
    union {double f; uint64_t i;} ux = {x};
    int e;

    if (isnan(x) || isnan(y))
        return x + y;
    if (x == y)
        return y;
    if (x == 0) {
        ux.i = 1;
        if (signbit(y))
            ux.i |= 1ULL<<63;
    } else if (x < y) {
        if (signbit(x))
            ux.i--;
        else
            ux.i++;
    } else {
        if (signbit(x))
            ux.i++;
        else
            ux.i--;
    }
    e = ux.i>>52 & 0x7ff;
    /* raise overflow if ux.f is infinite and x is finite */
    if (e == 0x7ff)
        FORCE_EVAL(x+x);
    /* raise underflow if ux.f is subnormal or nexttoward_c__zero */
    if (e == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}
#endif

/// >>> START src/math/nexttowardf.c

float nexttowardf(float x, long double y) {
    union {float f; uint32_t i;} ux = {x};
    uint32_t e;

    if (isnan(x) || isnan(y))
        return x + y;
    if (x == y)
        return y;
    if (x == 0) {
        ux.i = 1;
        if (signbit(y))
            ux.i |= 0x80000000;
    } else if (x < y) {
        if (signbit(x))
            ux.i--;
        else
            ux.i++;
    } else {
        if (signbit(x))
            ux.i++;
        else
            ux.i--;
    }
    e = ux.i & 0x7f800000;
    /* raise overflow if ux.f is infinite and x is finite */
    if (e == 0x7f800000)
        FORCE_EVAL(x+x);
    /* raise underflow if ux.f is subnormal or nexttowardf_c__zero */
    if (e == 0)
        FORCE_EVAL(x*x + ux.f*ux.f);
    return ux.f;
}

/// >>> START src/math/nexttowardl.c

long double nexttowardl(long double x, long double y) {
    return nextafterl(x, y);
}

/// >>> START src/math/pow.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_pow.c */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* pow(x,y) return x**y
 *
 *                    n
 * Method:  Let x =  2   * (1+f)
 *      1. Compute and return log2(x) in two pieces:
 *              log2(x) = pow_c__w1 + pow_c__w2,
 *         where pow_c__w1 has 53-24 = 29 bit trailing zeros.
 *      2. Perform y*log2(x) = n+y' by simulating muti-precision
 *         arithmetic, where |y'|<=0.5.
 *      3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *      1.  (anything) ** 0  is 1
 *      2.  1 ** (anything)  is 1
 *      3.  (anything except 1) ** NAN is NAN
 *      4.  NAN ** (anything except 0) is NAN
 *      5.  +-(|x| > 1) **  +INF is +INF
 *      6.  +-(|x| > 1) **  -INF is +0
 *      7.  +-(|x| < 1) **  +INF is +0
 *      8.  +-(|x| < 1) **  -INF is +INF
 *      9.  -1          ** +-INF is 1
 *      10. +0 ** (+anything except 0, NAN)               is +0
 *      11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *      12. +0 ** (-anything except 0, NAN)               is +INF, raise divbyzero
 *      13. -0 ** (-anything except 0, NAN, odd integer)  is +INF, raise divbyzero
 *      14. -0 ** (+odd integer) is -0
 *      15. -0 ** (-odd integer) is -INF, raise divbyzero
 *      16. +INF ** (+anything except 0,NAN) is +INF
 *      17. +INF ** (-anything except 0,NAN) is +0
 *      18. -INF ** (+odd integer) is -INF
 *      19. -INF ** (anything) = -0 ** (-anything), (anything except odd integer)
 *      20. (anything) ** 1 is (anything)
 *      21. (anything) ** -1 is 1/(anything)
 *      22. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *      23. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 * Accuracy:
 *      pow(x,y) returns x**y nearly rounded. In particular
 *                      pow(integer,integer)
 *      always returns the correct integer provided it is
 *      representable.
 *
 * Constants :
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
pow_c__bp[]   = {1.0, 1.5,},
pow_c__dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
pow_c__dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
pow_c__two53  =  9007199254740992.0, /* 0x43400000, 0x00000000 */
pow_c__huge   =  1.0e300,
pow_c__tiny   =  1.0e-300,
/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
pow_c__L1 =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
pow_c__L2 =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
pow_c__L3 =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
pow_c__L4 =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
pow_c__L5 =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
pow_c__L6 =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
pow_c__P1 =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
pow_c__P2 = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
pow_c__P3 =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
pow_c__P4 = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
pow_c__P5 =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
pow_c__lg2     =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
pow_c__lg2_h   =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
pow_c__lg2_l   = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
pow_c__ovt     =  8.0085662595372944372e-017, /* -(1024-log2(ovfl+.5ulp)) */
pow_c__cp      =  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
pow_c__cp_h    =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)pow_c__cp */
pow_c__cp_l    = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of pow_c__cp_h*/
pow_c__ivln2   =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
pow_c__ivln2_h =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
pow_c__ivln2_l =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

double pow(double x, double y) {
    double z,ax,z_h,z_l,p_h,p_l;
    double y1,pow_c__t1,pow_c__t2,r,s,t,u,v,w;
    int32_t i,j,pow_c__k,yisint,n;
    int32_t hx,hy,ix,iy;
    uint32_t lx,ly;

    EXTRACT_WORDS(hx, lx, x);
    EXTRACT_WORDS(hy, ly, y);
    ix = hx & 0x7fffffff;
    iy = hy & 0x7fffffff;

    /* x**0 = 1, even if x is NaN */
    if ((iy|ly) == 0)
        return 1.0;
    /* 1**y = 1, even if y is NaN */
    if (hx == 0x3ff00000 && lx == 0)
        return 1.0;
    /* NaN if either arg is NaN */
    if (ix > 0x7ff00000 || (ix == 0x7ff00000 && lx != 0) ||
        iy > 0x7ff00000 || (iy == 0x7ff00000 && ly != 0))
        return x + y;

    /* determine if y is an odd int when x < 0
     * yisint = 0       ... y is not an integer
     * yisint = 1       ... y is an odd int
     * yisint = 2       ... y is an even int
     */
    yisint = 0;
    if (hx < 0) {
        if (iy >= 0x43400000)
            yisint = 2; /* even integer y */
        else if (iy >= 0x3ff00000) {
            pow_c__k = (iy>>20) - 0x3ff;  /* exponent */
            if (pow_c__k > 20) {
                j = ly>>(52-pow_c__k);
                if ((j<<(52-pow_c__k)) == ly)
                    yisint = 2 - (j&1);
            } else if (ly == 0) {
                j = iy>>(20-pow_c__k);
                if ((j<<(20-pow_c__k)) == iy)
                    yisint = 2 - (j&1);
            }
        }
    }

    /* special value of y */
    if (ly == 0) {
        if (iy == 0x7ff00000) {  /* y is +-inf */
            if (((ix-0x3ff00000)|lx) == 0)  /* (-1)**+-inf is 1 */
                return 1.0;
            else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
                return hy >= 0 ? y : 0.0;
            else if ((ix|lx) != 0)     /* (|x|<1)**+-inf = 0,inf if x!=0 */
                return hy >= 0 ? 0.0 : -y;
        }
        if (iy == 0x3ff00000) {    /* y is +-1 */
            if (hy >= 0)
                return x;
            y = 1/x;
#if FLT_EVAL_METHOD!=0
            {
                union {double f; uint64_t i;} u = {y};
                uint64_t i = u.i & -1ULL/2;
                if (i>>52 == 0 && (i&(i-1)))
                    FORCE_EVAL((float)y);
            }
#endif
            return y;
        }
        if (hy == 0x40000000)    /* y is 2 */
            return x*x;
        if (hy == 0x3fe00000) {  /* y is 0.5 */
            if (hx >= 0)     /* x >= +0 */
                return sqrt(x);
        }
    }

    ax = fabs(x);
    /* special value of x */
    if (lx == 0) {
        if (ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000) { /* x is +-0,+-inf,+-1 */
            z = ax;
            if (hy < 0)   /* z = (1/|x|) */
                z = 1.0/z;
            if (hx < 0) {
                if (((ix-0x3ff00000)|yisint) == 0) {
                    z = (z-z)/(z-z); /* (-1)**non-int is NaN */
                } else if (yisint == 1)
                    z = -z;          /* (x<0)**odd = -(|x|**odd) */
            }
            return z;
        }
    }

    s = 1.0; /* sign of result */
    if (hx < 0) {
        if (yisint == 0) /* (x<0)**(non-int) is NaN */
            return (x-x)/(x-x);
        if (yisint == 1) /* (x<0)**(odd int) */
            s = -1.0;
    }

    /* |y| is pow_c__huge */
    if (iy > 0x41e00000) { /* if |y| > 2**31 */
        if (iy > 0x43f00000) {  /* if |y| > 2**64, must o/uflow */
            if (ix <= 0x3fefffff)
                return hy < 0 ? pow_c__huge*pow_c__huge : pow_c__tiny*pow_c__tiny;
            if (ix >= 0x3ff00000)
                return hy > 0 ? pow_c__huge*pow_c__huge : pow_c__tiny*pow_c__tiny;
        }
        /* over/underflow if x is not close to one */
        if (ix < 0x3fefffff)
            return hy < 0 ? s*pow_c__huge*pow_c__huge : s*pow_c__tiny*pow_c__tiny;
        if (ix > 0x3ff00000)
            return hy > 0 ? s*pow_c__huge*pow_c__huge : s*pow_c__tiny*pow_c__tiny;
        /* now |1-x| is pow_c__tiny <= 2**-20, suffice to compute
           log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = ax - 1.0;       /* t has 20 trailing zeros */
        w = (t*t)*(0.5 - t*(0.3333333333333333333333-t*0.25));
        u = pow_c__ivln2_h*t;      /* pow_c__ivln2_h has 21 sig. bits */
        v = t*pow_c__ivln2_l - w*pow_c__ivln2;
        pow_c__t1 = u + v;
        SET_LOW_WORD(pow_c__t1, 0);
        pow_c__t2 = v - (pow_c__t1-u);
    } else {
        double ss,pow_c__s2,s_h,s_l,t_h,t_l;
        n = 0;
        /* take care subnormal number */
        if (ix < 0x00100000) {
            ax *= pow_c__two53;
            n -= 53;
            GET_HIGH_WORD(ix,ax);
        }
        n += ((ix)>>20) - 0x3ff;
        j = ix & 0x000fffff;
        /* determine interval */
        ix = j | 0x3ff00000;   /* normalize ix */
        if (j <= 0x3988E)      /* |x|<sqrt(3/2) */
            pow_c__k = 0;
        else if (j < 0xBB67A)  /* |x|<sqrt(3)   */
            pow_c__k = 1;
        else {
            pow_c__k = 0;
            n += 1;
            ix -= 0x00100000;
        }
        SET_HIGH_WORD(ax, ix);

        /* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax - pow_c__bp[pow_c__k];        /* pow_c__bp[0]=1.0, pow_c__bp[1]=1.5 */
        v = 1.0/(ax+pow_c__bp[pow_c__k]);
        ss = u*v;
        s_h = ss;
        SET_LOW_WORD(s_h, 0);
        /* t_h=ax+pow_c__bp[pow_c__k] High */
        t_h = 0.0;
        SET_HIGH_WORD(t_h, ((ix>>1)|0x20000000) + 0x00080000 + (pow_c__k<<18));
        t_l = ax - (t_h-pow_c__bp[pow_c__k]);
        s_l = v*((u-s_h*t_h)-s_h*t_l);
        /* compute log(ax) */
        pow_c__s2 = ss*ss;
        r = pow_c__s2*pow_c__s2*(pow_c__L1+pow_c__s2*(pow_c__L2+pow_c__s2*(pow_c__L3+pow_c__s2*(pow_c__L4+pow_c__s2*(pow_c__L5+pow_c__s2*pow_c__L6)))));
        r += s_l*(s_h+ss);
        pow_c__s2 = s_h*s_h;
        t_h = 3.0 + pow_c__s2 + r;
        SET_LOW_WORD(t_h, 0);
        t_l = r - ((t_h-3.0)-pow_c__s2);
        /* u+v = ss*(1+...) */
        u = s_h*t_h;
        v = s_l*t_h + t_l*ss;
        /* 2/(3log2)*(ss+...) */
        p_h = u + v;
        SET_LOW_WORD(p_h, 0);
        p_l = v - (p_h-u);
        z_h = pow_c__cp_h*p_h;        /* pow_c__cp_h+pow_c__cp_l = 2/(3*log2) */
        z_l = pow_c__cp_l*p_h+p_l*pow_c__cp + pow_c__dp_l[pow_c__k];
        /* log2(ax) = (ss+..)*2/(3*log2) = n + pow_c__dp_h + z_h + z_l */
        t = (double)n;
        pow_c__t1 = ((z_h + z_l) + pow_c__dp_h[pow_c__k]) + t;
        SET_LOW_WORD(pow_c__t1, 0);
        pow_c__t2 = z_l - (((pow_c__t1 - t) - pow_c__dp_h[pow_c__k]) - z_h);
    }

    /* split up y into y1+y2 and compute (y1+y2)*(pow_c__t1+pow_c__t2) */
    y1 = y;
    SET_LOW_WORD(y1, 0);
    p_l = (y-y1)*pow_c__t1 + y*pow_c__t2;
    p_h = y1*pow_c__t1;
    z = p_l + p_h;
    EXTRACT_WORDS(j, i, z);
    if (j >= 0x40900000) {                      /* z >= 1024 */
        if (((j-0x40900000)|i) != 0)        /* if z > 1024 */
            return s*pow_c__huge*pow_c__huge;         /* overflow */
        if (p_l + pow_c__ovt > z - p_h)
            return s*pow_c__huge*pow_c__huge;         /* overflow */
    } else if ((j&0x7fffffff) >= 0x4090cc00) {  /* z <= -1075 */  // FIXME: instead of abs(j) use unsigned j
        if (((j-0xc090cc00)|i) != 0)        /* z < -1075 */
            return s*pow_c__tiny*pow_c__tiny;         /* underflow */
        if (p_l <= z - p_h)
            return s*pow_c__tiny*pow_c__tiny;         /* underflow */
    }
    /*
     * compute 2**(p_h+p_l)
     */
    i = j & 0x7fffffff;
    pow_c__k = (i>>20) - 0x3ff;
    n = 0;
    if (i > 0x3fe00000) {  /* if |z| > 0.5, set n = [z+0.5] */
        n = j + (0x00100000>>(pow_c__k+1));
        pow_c__k = ((n&0x7fffffff)>>20) - 0x3ff;  /* new pow_c__k for n */
        t = 0.0;
        SET_HIGH_WORD(t, n & ~(0x000fffff>>pow_c__k));
        n = ((n&0x000fffff)|0x00100000)>>(20-pow_c__k);
        if (j < 0)
            n = -n;
        p_h -= t;
    }
    t = p_l + p_h;
    SET_LOW_WORD(t, 0);
    u = t*pow_c__lg2_h;
    v = (p_l-(t-p_h))*pow_c__lg2 + t*pow_c__lg2_l;
    z = u + v;
    w = v - (z-u);
    t = z*z;
    pow_c__t1 = z - t*(pow_c__P1+t*(pow_c__P2+t*(pow_c__P3+t*(pow_c__P4+t*pow_c__P5))));
    r = (z*pow_c__t1)/(pow_c__t1-2.0) - (w + z*w);
    z = 1.0 - (r-z);
    GET_HIGH_WORD(j, z);
    j += n<<20;
    if ((j>>20) <= 0)  /* subnormal output */
        z = scalbn(z,n);
    else
        SET_HIGH_WORD(z, j);
    return s*z;
}

/// >>> START src/math/powf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_powf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float
powf_c__bp[]   = {1.0, 1.5,},
powf_c__dp_h[] = { 0.0, 5.84960938e-01,}, /* 0x3f15c000 */
powf_c__dp_l[] = { 0.0, 1.56322085e-06,}, /* 0x35d1cfdc */
powf_c__two24  =  16777216.0,  /* 0x4b800000 */
powf_c__huge   =  1.0e30,
powf_c__tiny   =  1.0e-30,
/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
powf_c__L1 =  6.0000002384e-01, /* 0x3f19999a */
powf_c__L2 =  4.2857143283e-01, /* 0x3edb6db7 */
powf_c__L3 =  3.3333334327e-01, /* 0x3eaaaaab */
powf_c__L4 =  2.7272811532e-01, /* 0x3e8ba305 */
powf_c__L5 =  2.3066075146e-01, /* 0x3e6c3255 */
powf_c__L6 =  2.0697501302e-01, /* 0x3e53f142 */
powf_c__P1 =  1.6666667163e-01, /* 0x3e2aaaab */
powf_c__P2 = -2.7777778450e-03, /* 0xbb360b61 */
powf_c__P3 =  6.6137559770e-05, /* 0x388ab355 */
powf_c__P4 = -1.6533901999e-06, /* 0xb5ddea0e */
powf_c__P5 =  4.1381369442e-08, /* 0x3331bb4c */
powf_c__lg2     =  6.9314718246e-01, /* 0x3f317218 */
powf_c__lg2_h   =  6.93145752e-01,   /* 0x3f317200 */
powf_c__lg2_l   =  1.42860654e-06,   /* 0x35bfbe8c */
powf_c__ovt     =  4.2995665694e-08, /* -(128-log2(ovfl+.5ulp)) */
powf_c__cp      =  9.6179670095e-01, /* 0x3f76384f =2/(3ln2) */
powf_c__cp_h    =  9.6191406250e-01, /* 0x3f764000 =12b powf_c__cp */
powf_c__cp_l    = -1.1736857402e-04, /* 0xb8f623c6 =tail of powf_c__cp_h */
powf_c__ivln2   =  1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
powf_c__ivln2_h =  1.4426879883e+00, /* 0x3fb8aa00 =16b 1/ln2*/
powf_c__ivln2_l =  7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/

float powf(float x, float y) {
    float z,ax,z_h,z_l,p_h,p_l;
    float y1,powf_c__t1,powf_c__t2,r,s,sn,t,u,v,w;
    int32_t i,j,powf_c__k,yisint,n;
    int32_t hx,hy,ix,iy,is;

    GET_FLOAT_WORD(hx, x);
    GET_FLOAT_WORD(hy, y);
    ix = hx & 0x7fffffff;
    iy = hy & 0x7fffffff;

    /* x**0 = 1, even if x is NaN */
    if (iy == 0)
        return 1.0f;
    /* 1**y = 1, even if y is NaN */
    if (hx == 0x3f800000)
        return 1.0f;
    /* NaN if either arg is NaN */
    if (ix > 0x7f800000 || iy > 0x7f800000)
        return x + y;

    /* determine if y is an odd int when x < 0
     * yisint = 0       ... y is not an integer
     * yisint = 1       ... y is an odd int
     * yisint = 2       ... y is an even int
     */
    yisint  = 0;
    if (hx < 0) {
        if (iy >= 0x4b800000)
            yisint = 2; /* even integer y */
        else if (iy >= 0x3f800000) {
            powf_c__k = (iy>>23) - 0x7f;         /* exponent */
            j = iy>>(23-powf_c__k);
            if ((j<<(23-powf_c__k)) == iy)
                yisint = 2 - (j & 1);
        }
    }

    /* special value of y */
    if (iy == 0x7f800000) {  /* y is +-inf */
        if (ix == 0x3f800000)      /* (-1)**+-inf is 1 */
            return 1.0f;
        else if (ix > 0x3f800000)  /* (|x|>1)**+-inf = inf,0 */
            return hy >= 0 ? y : 0.0f;
        else if (ix != 0)          /* (|x|<1)**+-inf = 0,inf if x!=0 */
            return hy >= 0 ? 0.0f: -y;
    }
    if (iy == 0x3f800000)    /* y is +-1 */
        return hy >= 0 ? x : 1.0f/x;
    if (hy == 0x40000000)    /* y is 2 */
        return x*x;
    if (hy == 0x3f000000) {  /* y is  0.5 */
        if (hx >= 0)     /* x >= +0 */
            return sqrtf(x);
    }

    ax = fabsf(x);
    /* special value of x */
    if (ix == 0x7f800000 || ix == 0 || ix == 0x3f800000) { /* x is +-0,+-inf,+-1 */
        z = ax;
        if (hy < 0)  /* z = (1/|x|) */
            z = 1.0f/z;
        if (hx < 0) {
            if (((ix-0x3f800000)|yisint) == 0) {
                z = (z-z)/(z-z); /* (-1)**non-int is NaN */
            } else if (yisint == 1)
                z = -z;          /* (x<0)**odd = -(|x|**odd) */
        }
        return z;
    }

    sn = 1.0f; /* sign of result */
    if (hx < 0) {
        if (yisint == 0) /* (x<0)**(non-int) is NaN */
            return (x-x)/(x-x);
        if (yisint == 1) /* (x<0)**(odd int) */
            sn = -1.0f;
    }

    /* |y| is powf_c__huge */
    if (iy > 0x4d000000) { /* if |y| > 2**27 */
        /* over/underflow if x is not close to one */
        if (ix < 0x3f7ffff8)
            return hy < 0 ? sn*powf_c__huge*powf_c__huge : sn*powf_c__tiny*powf_c__tiny;
        if (ix > 0x3f800007)
            return hy > 0 ? sn*powf_c__huge*powf_c__huge : sn*powf_c__tiny*powf_c__tiny;
        /* now |1-x| is powf_c__tiny <= 2**-20, suffice to compute
           log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = ax - 1;     /* t has 20 trailing zeros */
        w = (t*t)*(0.5f - t*(0.333333333333f - t*0.25f));
        u = powf_c__ivln2_h*t;  /* powf_c__ivln2_h has 16 sig. bits */
        v = t*powf_c__ivln2_l - w*powf_c__ivln2;
        powf_c__t1 = u + v;
        GET_FLOAT_WORD(is, powf_c__t1);
        SET_FLOAT_WORD(powf_c__t1, is & 0xfffff000);
        powf_c__t2 = v - (powf_c__t1-u);
    } else {
        float powf_c__s2,s_h,s_l,t_h,t_l;
        n = 0;
        /* take care subnormal number */
        if (ix < 0x00800000) {
            ax *= powf_c__two24;
            n -= 24;
            GET_FLOAT_WORD(ix, ax);
        }
        n += ((ix)>>23) - 0x7f;
        j = ix & 0x007fffff;
        /* determine interval */
        ix = j | 0x3f800000;     /* normalize ix */
        if (j <= 0x1cc471)       /* |x|<sqrt(3/2) */
            powf_c__k = 0;
        else if (j < 0x5db3d7)   /* |x|<sqrt(3)   */
            powf_c__k = 1;
        else {
            powf_c__k = 0;
            n += 1;
            ix -= 0x00800000;
        }
        SET_FLOAT_WORD(ax, ix);

        /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax - powf_c__bp[powf_c__k];   /* powf_c__bp[0]=1.0, powf_c__bp[1]=1.5 */
        v = 1.0f/(ax+powf_c__bp[powf_c__k]);
        s = u*v;
        s_h = s;
        GET_FLOAT_WORD(is, s_h);
        SET_FLOAT_WORD(s_h, is & 0xfffff000);
        /* t_h=ax+powf_c__bp[powf_c__k] High */
        is = ((ix>>1) & 0xfffff000) | 0x20000000;
        SET_FLOAT_WORD(t_h, is + 0x00400000 + (powf_c__k<<21));
        t_l = ax - (t_h - powf_c__bp[powf_c__k]);
        s_l = v*((u - s_h*t_h) - s_h*t_l);
        /* compute log(ax) */
        powf_c__s2 = s*s;
        r = powf_c__s2*powf_c__s2*(powf_c__L1+powf_c__s2*(powf_c__L2+powf_c__s2*(powf_c__L3+powf_c__s2*(powf_c__L4+powf_c__s2*(powf_c__L5+powf_c__s2*powf_c__L6)))));
        r += s_l*(s_h+s);
        powf_c__s2 = s_h*s_h;
        t_h = 3.0f + powf_c__s2 + r;
        GET_FLOAT_WORD(is, t_h);
        SET_FLOAT_WORD(t_h, is & 0xfffff000);
        t_l = r - ((t_h - 3.0f) - powf_c__s2);
        /* u+v = s*(1+...) */
        u = s_h*t_h;
        v = s_l*t_h + t_l*s;
        /* 2/(3log2)*(s+...) */
        p_h = u + v;
        GET_FLOAT_WORD(is, p_h);
        SET_FLOAT_WORD(p_h, is & 0xfffff000);
        p_l = v - (p_h - u);
        z_h = powf_c__cp_h*p_h;  /* powf_c__cp_h+powf_c__cp_l = 2/(3*log2) */
        z_l = powf_c__cp_l*p_h + p_l*powf_c__cp+powf_c__dp_l[powf_c__k];
        /* log2(ax) = (s+..)*2/(3*log2) = n + powf_c__dp_h + z_h + z_l */
        t = (float)n;
        powf_c__t1 = (((z_h + z_l) + powf_c__dp_h[powf_c__k]) + t);
        GET_FLOAT_WORD(is, powf_c__t1);
        SET_FLOAT_WORD(powf_c__t1, is & 0xfffff000);
        powf_c__t2 = z_l - (((powf_c__t1 - t) - powf_c__dp_h[powf_c__k]) - z_h);
    }

    /* split up y into y1+y2 and compute (y1+y2)*(powf_c__t1+powf_c__t2) */
    GET_FLOAT_WORD(is, y);
    SET_FLOAT_WORD(y1, is & 0xfffff000);
    p_l = (y-y1)*powf_c__t1 + y*powf_c__t2;
    p_h = y1*powf_c__t1;
    z = p_l + p_h;
    GET_FLOAT_WORD(j, z);
    if (j > 0x43000000)          /* if z > 128 */
        return sn*powf_c__huge*powf_c__huge;  /* overflow */
    else if (j == 0x43000000) {  /* if z == 128 */
        if (p_l + powf_c__ovt > z - p_h)
            return sn*powf_c__huge*powf_c__huge;  /* overflow */
    } else if ((j&0x7fffffff) > 0x43160000)  /* z < -150 */ // FIXME: check should be  (uint32_t)j > 0xc3160000
        return sn*powf_c__tiny*powf_c__tiny;  /* underflow */
    else if (j == 0xc3160000) {  /* z == -150 */
        if (p_l <= z-p_h)
            return sn*powf_c__tiny*powf_c__tiny;  /* underflow */
    }
    /*
     * compute 2**(p_h+p_l)
     */
    i = j & 0x7fffffff;
    powf_c__k = (i>>23) - 0x7f;
    n = 0;
    if (i > 0x3f000000) {   /* if |z| > 0.5, set n = [z+0.5] */
        n = j + (0x00800000>>(powf_c__k+1));
        powf_c__k = ((n&0x7fffffff)>>23) - 0x7f;  /* new powf_c__k for n */
        SET_FLOAT_WORD(t, n & ~(0x007fffff>>powf_c__k));
        n = ((n&0x007fffff)|0x00800000)>>(23-powf_c__k);
        if (j < 0)
            n = -n;
        p_h -= t;
    }
    t = p_l + p_h;
    GET_FLOAT_WORD(is, t);
    SET_FLOAT_WORD(t, is & 0xffff8000);
    u = t*powf_c__lg2_h;
    v = (p_l-(t-p_h))*powf_c__lg2 + t*powf_c__lg2_l;
    z = u + v;
    w = v - (z - u);
    t = z*z;
    powf_c__t1 = z - t*(powf_c__P1+t*(powf_c__P2+t*(powf_c__P3+t*(powf_c__P4+t*powf_c__P5))));
    r = (z*powf_c__t1)/(powf_c__t1-2.0f) - (w+z*w);
    z = 1.0f - (r - z);
    GET_FLOAT_WORD(j, z);
    j += n<<23;
    if ((j>>23) <= 0)  /* subnormal output */
        z = scalbnf(z, n);
    else
        SET_FLOAT_WORD(z, j);
    return sn*z;
}

/// >>> START src/math/powl.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_powl.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*                                                      powl.c
 *
 *      Power function, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, z, powl();
 *
 * z = powl( x, y );
 *
 *
 * DESCRIPTION:
 *
 * Computes x raised to the yth power.  Analytically,
 *
 *      x**y  =  exp( y log(x) ).
 *
 * Following Cody and Waite, this program uses a lookup powl_c__table
 * of 2**-i/32 and pseudo extended precision arithmetic to
 * obtain several extra bits of accuracy in both the logarithm
 * and the exponential.
 *
 *
 * ACCURACY:
 *
 * The relative error of pow(x,y) can be estimated
 * by   y dl ln(2),   where dl is the absolute error of
 * the internally computed base 2 logarithm.  At the ends
 * of the approximation interval the logarithm equal 1/32
 * and its relative error is about 1 lsb = 1.1e-19.  Hence
 * the predicted relative error in the result is 2.3e-21 y .
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *
 *    IEEE     +-1000       40000      2.8e-18      3.7e-19
 * .001 < x < 1000, with log(x) uniformly distributed.
 * -1000 < y < 1000, y uniformly distributed.
 *
 *    IEEE     0,8700       60000      6.5e-18      1.0e-18
 * 0.99 < x < 1.01, 0 < y < 8700, uniformly distributed.
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * pow overflow     x**y > MAXNUM      INFINITY
 * pow underflow   x**y < 1/MAXNUM       0.0
 * pow domain      x<0 and y noninteger  0.0
 *
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double powl(long double x, long double y) {
    return pow(x, y);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384

/* Table size */
#undef NXT
#define NXT 32

/* log(1+x) =  x - .5x^2 + x^3 *  powl_c__P(z)/powl_c__Q(z)
 * on the domain  2^(-1/32) - 1  <=  x  <=  2^(1/32) - 1
 */
static const long double powl_c__P[] = {
 8.3319510773868690346226E-4L,
 4.9000050881978028599627E-1L,
 1.7500123722550302671919E0L,
 1.4000100839971580279335E0L,
};
static const long double powl_c__Q[] = {
/* 1.0000000000000000000000E0L,*/
 5.2500282295834889175431E0L,
 8.4000598057587009834666E0L,
 4.2000302519914740834728E0L,
};
/* A[i] = 2^(-i/32), rounded to IEEE long double precision.
 * If i is even, A[i] + B[i/2] gives additional accuracy.
 */
static const long double A[33] = {
 1.0000000000000000000000E0L,
 9.7857206208770013448287E-1L,
 9.5760328069857364691013E-1L,
 9.3708381705514995065011E-1L,
 9.1700404320467123175367E-1L,
 8.9735453750155359320742E-1L,
 8.7812608018664974155474E-1L,
 8.5930964906123895780165E-1L,
 8.4089641525371454301892E-1L,
 8.2287773907698242225554E-1L,
 8.0524516597462715409607E-1L,
 7.8799042255394324325455E-1L,
 7.7110541270397041179298E-1L,
 7.5458221379671136985669E-1L,
 7.3841307296974965571198E-1L,
 7.2259040348852331001267E-1L,
 7.0710678118654752438189E-1L,
 6.9195494098191597746178E-1L,
 6.7712777346844636413344E-1L,
 6.6261832157987064729696E-1L,
 6.4841977732550483296079E-1L,
 6.3452547859586661129850E-1L,
 6.2092890603674202431705E-1L,
 6.0762367999023443907803E-1L,
 5.9460355750136053334378E-1L,
 5.8186242938878875689693E-1L,
 5.6939431737834582684856E-1L,
 5.5719337129794626814472E-1L,
 5.4525386633262882960438E-1L,
 5.3357020033841180906486E-1L,
 5.2213689121370692017331E-1L,
 5.1094857432705833910408E-1L,
 5.0000000000000000000000E-1L,
};
static const long double B[17] = {
 0.0000000000000000000000E0L,
 2.6176170809902549338711E-20L,
-1.0126791927256478897086E-20L,
 1.3438228172316276937655E-21L,
 1.2207982955417546912101E-20L,
-6.3084814358060867200133E-21L,
 1.3164426894366316434230E-20L,
-1.8527916071632873716786E-20L,
 1.8950325588932570796551E-20L,
 1.5564775779538780478155E-20L,
 6.0859793637556860974380E-21L,
-2.0208749253662532228949E-20L,
 1.4966292219224761844552E-20L,
 3.3540909728056476875639E-21L,
-8.6987564101742849540743E-22L,
-1.2327176863327626135542E-20L,
 0.0000000000000000000000E0L,
};

/* 2^x = 1 + x powl_c__P(x),
 * on the interval -1/32 <= x <= 0
 */
static const long double powl_c__R[] = {
 1.5089970579127659901157E-5L,
 1.5402715328927013076125E-4L,
 1.3333556028915671091390E-3L,
 9.6181291046036762031786E-3L,
 5.5504108664798463044015E-2L,
 2.4022650695910062854352E-1L,
 6.9314718055994530931447E-1L,
};

#undef MEXP
#define MEXP (NXT*16384.0L)
/* The following if denormal numbers are supported, else -MEXP: */
#undef MNEXP
#define MNEXP (-NXT*(16384.0L+64.0L))
/* log2(e) - 1 */
#undef powl_c__LOG2EA
#define powl_c__LOG2EA 0.44269504088896340735992L

#undef F
#define F W
#undef Fa
#define Fa Wa
#undef Fb
#define Fb Wb
#undef G
#define G W
#undef Ga
#define Ga Wa
#undef Gb
#define Gb u
#undef H
#define H W
#undef Ha
#define Ha Wb
#undef Hb
#define Hb Wb

static const long double MAXLOGL = 1.1356523406294143949492E4L;
static const long double MINLOGL = -1.13994985314888605586758E4L;
static const long double LOGE2L = 6.9314718055994530941723E-1L;
static const long double powl_c__huge = 0x1p10000L;
/* XXX Prevent gcc from erroneously constant folding this. */
static const volatile long double twom10000 = 0x1p-10000L;

static long double reducl(long double);
static long double powil(long double, int);

long double powl(long double x, long double y) {
    /* double F, Fa, Fb, G, Ga, Gb, H, Ha, Hb */
    int i, nflg, iyflg, yoddint;
    long e;
    volatile long double z=0;
    long double w=0, W=0, Wa=0, Wb=0, ya=0, yb=0, u=0;

    /* make sure no invalid exception is raised by nan comparision */
    if (isnan(x)) {
        if (!isnan(y) && y == 0.0)
            return 1.0;
        return x;
    }
    if (isnan(y)) {
        if (x == 1.0)
            return 1.0;
        return y;
    }
    if (x == 1.0)
        return 1.0; /* 1**y = 1, even if y is nan */
    if (x == -1.0 && !isfinite(y))
        return 1.0; /* -1**inf = 1 */
    if (y == 0.0)
        return 1.0; /* x**0 = 1, even if x is nan */
    if (y == 1.0)
        return x;
    if (y >= LDBL_MAX) {
        if (x > 1.0 || x < -1.0)
            return INFINITY;
        if (x != 0.0)
            return 0.0;
    }
    if (y <= -LDBL_MAX) {
        if (x > 1.0 || x < -1.0)
            return 0.0;
        if (x != 0.0)
            return INFINITY;
    }
    if (x >= LDBL_MAX) {
        if (y > 0.0)
            return INFINITY;
        return 0.0;
    }

    w = floorl(y);

    /* Set iyflg to 1 if y is an integer. */
    iyflg = 0;
    if (w == y)
        iyflg = 1;

    /* Test for odd integer y. */
    yoddint = 0;
    if (iyflg) {
        ya = fabsl(y);
        ya = floorl(0.5 * ya);
        yb = 0.5 * fabsl(w);
        if( ya != yb )
            yoddint = 1;
    }

    if (x <= -LDBL_MAX) {
        if (y > 0.0) {
            if (yoddint)
                return -INFINITY;
            return INFINITY;
        }
        if (y < 0.0) {
            if (yoddint)
                return -0.0;
            return 0.0;
        }
    }
    nflg = 0; /* (x<0)**(odd int) */
    if (x <= 0.0) {
        if (x == 0.0) {
            if (y < 0.0) {
                if (signbit(x) && yoddint)
                    /* (-0.0)**(-odd int) = -inf, divbyzero */
                    return -1.0/0.0;
                /* (+-0.0)**(negative) = inf, divbyzero */
                return 1.0/0.0;
            }
            if (signbit(x) && yoddint)
                return -0.0;
            return 0.0;
        }
        if (iyflg == 0)
            return (x - x) / (x - x); /* (x<0)**(non-int) is NaN */
        /* (x<0)**(integer) */
        if (yoddint)
            nflg = 1; /* negate result */
        x = -x;
    }
    /* (+integer)**(integer)  */
    if (iyflg && floorl(x) == x && fabsl(y) < 32768.0) {
        w = powil(x, (int)y);
        return nflg ? -w : w;
    }

    /* separate significand from exponent */
    x = frexpl(x, &i);
    e = i;

    /* find significand in antilog powl_c__table A[] */
    i = 1;
    if (x <= A[17])
        i = 17;
    if (x <= A[i+8])
        i += 8;
    if (x <= A[i+4])
        i += 4;
    if (x <= A[i+2])
        i += 2;
    if (x >= A[1])
        i = -1;
    i += 1;

    /* Find (x - A[i])/A[i]
     * in order to compute log(x/A[i]):
     *
     * log(x) = log( a x/a ) = log(a) + log(x/a)
     *
     * log(x/a) = log(1+v),  v = x/a - 1 = (x-a)/a
     */
    x -= A[i];
    x -= B[i/2];
    x /= A[i];

    /* rational approximation for log(1+v):
     *
     * log(1+v)  =  v  -  v**2/2  +  v**3 powl_c__P(v) / powl_c__Q(v)
     */
    z = x*x;
    w = x * (z * __polevll(x, powl_c__P, 3) / __p1evll(x, powl_c__Q, 3));
    w = w - 0.5*z;

    /* Convert to base 2 logarithm:
     * multiply by log2(e) = 1 + powl_c__LOG2EA
     */
    z = powl_c__LOG2EA * w;
    z += w;
    z += powl_c__LOG2EA * x;
    z += x;

    /* Compute exponent term of the base 2 logarithm. */
    w = -i;
    w /= NXT;
    w += e;
    /* Now base 2 log of x is w + z. */

    /* Multiply base 2 log by y, in extended precision. */

    /* separate y into large part ya
     * and small part yb less than 1/NXT
     */
    ya = reducl(y);
    yb = y - ya;

    /* (w+z)(ya+yb)
     * = w*ya + w*yb + z*y
     */
    F = z * y  +  w * yb;
    Fa = reducl(F);
    Fb = F - Fa;

    G = Fa + w * ya;
    Ga = reducl(G);
    Gb = G - Ga;

    H = Fb + Gb;
    Ha = reducl(H);
    w = (Ga + Ha) * NXT;

    /* Test the power of 2 for overflow */
    if (w > MEXP)
        return powl_c__huge * powl_c__huge;  /* overflow */
    if (w < MNEXP)
        return twom10000 * twom10000;  /* underflow */

    e = w;
    Hb = H - Ha;

    if (Hb > 0.0) {
        e += 1;
        Hb -= 1.0/NXT;  /*0.0625L;*/
    }

    /* Now the product y * log2(x)  =  Hb + e/NXT.
     *
     * Compute base 2 exponential of Hb,
     * where -0.0625 <= Hb <= 0.
     */
    z = Hb * __polevll(Hb, powl_c__R, 6);  /*  z = 2**Hb - 1  */

    /* Express e/NXT as an integer plus a negative number of (1/NXT)ths.
     * Find lookup powl_c__table entry for the fractional power of 2.
     */
    if (e < 0)
        i = 0;
    else
        i = 1;
    i = e/NXT + i;
    e = NXT*i - e;
    w = A[e];
    z = w * z;  /*  2**-e * ( 1 + (2**Hb-1) )  */
    z = z + w;
    z = scalbnl(z, i);  /* multiply by integer power of 2 */

    if (nflg)
        z = -z;
    return z;
}

/* Find a multiple of 1/NXT that is within 1/NXT of x. */
static long double reducl(long double x) {
    long double t;

    t = x * NXT;
    t = floorl(t);
    t = t / NXT;
    return t;
}

/*
 *      Positive real raised to integer power, long double precision
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, powil();
 * int n;
 *
 * y = powil( x, n );
 *
 *
 * DESCRIPTION:
 *
 * Returns argument x>0 raised to the nth power.
 * The routine efficiently decomposes n as a sum of powers of
 * two. The desired power is a product of two-to-the-kth
 * powers of x.  Thus to compute the 32767 power of x requires
 * 28 multiplications instead of 32767 multiplications.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   x domain   n domain  # trials      peak         rms
 *    IEEE     .001,1000  -1022,1023  50000       4.3e-17     7.8e-18
 *    IEEE        1,2     -1022,1023  20000       3.9e-17     7.6e-18
 *    IEEE     .99,1.01     0,8700    10000       3.6e-16     7.2e-17
 *
 * Returns MAXNUM on overflow, powl_c__zero on underflow.
 */

static long double powil(long double x, int nn) {
    long double ww, y;
    long double s;
    int n, e, sign, lx;

    if (nn == 0)
        return 1.0;

    if (nn < 0) {
        sign = -1;
        n = -nn;
    } else {
        sign = 1;
        n = nn;
    }

    /* Overflow detection */

    /* Calculate approximate logarithm of answer */
    s = x;
    s = frexpl( s, &lx);
    e = (lx - 1)*n;
    if ((e == 0) || (e > 64) || (e < -64)) {
        s = (s - 7.0710678118654752e-1L) / (s +  7.0710678118654752e-1L);
        s = (2.9142135623730950L * s - 0.5 + lx) * nn * LOGE2L;
    } else {
        s = LOGE2L * e;
    }

    if (s > MAXLOGL)
        return powl_c__huge * powl_c__huge;  /* overflow */

    if (s < MINLOGL)
        return twom10000 * twom10000;  /* underflow */
    /* Handle powl_c__tiny denormal answer, but with less accuracy
     * since roundoff error in 1.0/x will be amplified.
     * The precise demarcation should be the gradual underflow threshold.
     */
    if (s < -MAXLOGL+2.0) {
        x = 1.0/x;
        sign = -sign;
    }

    /* First bit of the power */
    if (n & 1)
        y = x;
    else
        y = 1.0;

    ww = x;
    n >>= 1;
    while (n) {
        ww = ww * ww;   /* arg to the 2-to-the-kth power */
        if (n & 1)     /* if that bit is set, then include in product */
            y *= ww;
        n >>= 1;
    }

    if (sign < 0)
        y = 1.0/y;
    return y;
}

#endif

/// >>> START src/math/remainder.c

double remainder(double x, double y) {
    int q;
    return remquo(x, y, &q);
}

weak_alias(remainder, drem);

/// >>> START src/math/remainderf.c

float remainderf(float x, float y) {
    int q;
    return remquof(x, y, &q);
}

weak_alias(remainderf, dremf);

/// >>> START src/math/remainderl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double remainderl(long double x, long double y) {
    return remainder(x, y);
}
#else
long double remainderl(long double x, long double y) {
    int q;
    return remquol(x, y, &q);
}
#endif

/// >>> START src/math/remquo.c

double remquo(double x, double y, int *quo) {
    union {double f; uint64_t i;} ux = {x}, uy = {y};
    int ex = ux.i>>52 & 0x7ff;
    int ey = uy.i>>52 & 0x7ff;
    int sx = ux.i>>63;
    int sy = uy.i>>63;
    uint32_t q;
    uint64_t i;
    uint64_t uxi = ux.i;

    *quo = 0;
    if (uy.i<<1 == 0 || isnan(y) || ex == 0x7ff)
        return (x*y)/(x*y);
    if (ux.i<<1 == 0)
        return x;

    /* normalize x and y */
    if (!ex) {
        for (i = uxi<<12; i>>63 == 0; ex--, i <<= 1);
        uxi <<= -ex + 1;
    } else {
        uxi &= -1ULL >> 12;
        uxi |= 1ULL << 52;
    }
    if (!ey) {
        for (i = uy.i<<12; i>>63 == 0; ey--, i <<= 1);
        uy.i <<= -ey + 1;
    } else {
        uy.i &= -1ULL >> 12;
        uy.i |= 1ULL << 52;
    }

    q = 0;
    if (ex < ey) {
        if (ex+1 == ey)
            goto end;
        return x;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 63 == 0) {
            uxi = i;
            q++;
        }
        uxi <<= 1;
        q <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 63 == 0) {
        uxi = i;
        q++;
    }
    if (uxi == 0)
        ex = -60;
    else
        for (; uxi>>52 == 0; uxi <<= 1, ex--);
end:
    /* scale result and decide between |x| and |x|-|y| */
    if (ex > 0) {
        uxi -= 1ULL << 52;
        uxi |= (uint64_t)ex << 52;
    } else {
        uxi >>= -ex + 1;
    }
    ux.i = uxi;
    x = ux.f;
    if (sy)
        y = -y;
    if (ex == ey || (ex+1 == ey && (2*x > y || (2*x == y && q%2)))) {
        x -= y;
        q++;
    }
    q &= 0x7fffffff;
    *quo = sx^sy ? -(int)q : (int)q;
    return sx ? -x : x;
}

/// >>> START src/math/remquof.c

float remquof(float x, float y, int *quo) {
    union {float f; uint32_t i;} ux = {x}, uy = {y};
    int ex = ux.i>>23 & 0xff;
    int ey = uy.i>>23 & 0xff;
    int sx = ux.i>>31;
    int sy = uy.i>>31;
    uint32_t q;
    uint32_t i;
    uint32_t uxi = ux.i;

    *quo = 0;
    if (uy.i<<1 == 0 || isnan(y) || ex == 0xff)
        return (x*y)/(x*y);
    if (ux.i<<1 == 0)
        return x;

    /* normalize x and y */
    if (!ex) {
        for (i = uxi<<9; i>>31 == 0; ex--, i <<= 1);
        uxi <<= -ex + 1;
    } else {
        uxi &= -1U >> 9;
        uxi |= 1U << 23;
    }
    if (!ey) {
        for (i = uy.i<<9; i>>31 == 0; ey--, i <<= 1);
        uy.i <<= -ey + 1;
    } else {
        uy.i &= -1U >> 9;
        uy.i |= 1U << 23;
    }

    q = 0;
    if (ex < ey) {
        if (ex+1 == ey)
            goto end;
        return x;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 31 == 0) {
            uxi = i;
            q++;
        }
        uxi <<= 1;
        q <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 31 == 0) {
        uxi = i;
        q++;
    }
    if (uxi == 0)
        ex = -30;
    else
        for (; uxi>>23 == 0; uxi <<= 1, ex--);
end:
    /* scale result and decide between |x| and |x|-|y| */
    if (ex > 0) {
        uxi -= 1U << 23;
        uxi |= (uint32_t)ex << 23;
    } else {
        uxi >>= -ex + 1;
    }
    ux.i = uxi;
    x = ux.f;
    if (sy)
        y = -y;
    if (ex == ey || (ex+1 == ey && (2*x > y || (2*x == y && q%2)))) {
        x -= y;
        q++;
    }
    q &= 0x7fffffff;
    *quo = sx^sy ? -(int)q : (int)q;
    return sx ? -x : x;
}

/// >>> START src/math/remquol.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double remquol(long double x, long double y, int *quo) {
    return remquo(x, y, quo);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double remquol(long double x, long double y, int *quo) {
    union ldshape ux = {x}, uy = {y};
    int ex = ux.i.se & 0x7fff;
    int ey = uy.i.se & 0x7fff;
    int sx = ux.i.se >> 15;
    int sy = uy.i.se >> 15;
    uint32_t q;

    *quo = 0;
    if (y == 0 || isnan(y) || ex == 0x7fff)
        return (x*y)/(x*y);
    if (x == 0)
        return x;

    /* normalize x and y */
    if (!ex) {
        ux.i.se = ex;
        ux.f *= 0x1p120f;
        ex = ux.i.se - 120;
    }
    if (!ey) {
        uy.i.se = ey;
        uy.f *= 0x1p120f;
        ey = uy.i.se - 120;
    }

    q = 0;
    if (ex >= ey) {
        /* x mod y */
#if LDBL_MANT_DIG == 64
        uint64_t i, mx, my;
        mx = ux.i.m;
        my = uy.i.m;
        for (; ex > ey; ex--) {
            i = mx - my;
            if (mx >= my) {
                mx = 2*i;
                q++;
                q <<= 1;
            } else if (2*mx < mx) {
                mx = 2*mx - my;
                q <<= 1;
                q++;
            } else {
                mx = 2*mx;
                q <<= 1;
            }
        }
        i = mx - my;
        if (mx >= my) {
            mx = i;
            q++;
        }
        if (mx == 0)
            ex = -120;
        else
            for (; mx >> 63 == 0; mx *= 2, ex--);
        ux.i.m = mx;
#elif LDBL_MANT_DIG == 113
        uint64_t hi, lo, xhi, xlo, yhi, ylo;
        xhi = (ux.i2.hi & -1ULL>>16) | 1ULL<<48;
        yhi = (uy.i2.hi & -1ULL>>16) | 1ULL<<48;
        xlo = ux.i2.lo;
        ylo = ux.i2.lo;
        for (; ex > ey; ex--) {
            hi = xhi - yhi;
            lo = xlo - ylo;
            if (xlo < ylo)
                hi -= 1;
            if (hi >> 63 == 0) {
                xhi = 2*hi + (lo>>63);
                xlo = 2*lo;
                q++;
            } else {
                xhi = 2*xhi + (xlo>>63);
                xlo = 2*xlo;
            }
            q <<= 1;
        }
        hi = xhi - yhi;
        lo = xlo - ylo;
        if (xlo < ylo)
            hi -= 1;
        if (hi >> 63 == 0) {
            xhi = hi;
            xlo = lo;
            q++;
        }
        if ((xhi|xlo) == 0)
            ex = -120;
        else
            for (; xhi >> 48 == 0; xhi = 2*xhi + (xlo>>63), xlo = 2*xlo, ex--);
        ux.i2.hi = xhi;
        ux.i2.lo = xlo;
#endif
    }

    /* scale result and decide between |x| and |x|-|y| */
    if (ex <= 0) {
        ux.i.se = ex + 120;
        ux.f *= 0x1p-120f;
    } else
        ux.i.se = ex;
    x = ux.f;
    if (sy)
        y = -y;
    if (ex == ey || (ex+1 == ey && (2*x > y || (2*x == y && q%2)))) {
        x -= y;
        q++;
    }
    q &= 0x7fffffff;
    *quo = sx^sy ? -(int)q : (int)q;
    return sx ? -x : x;
}
#endif

/// >>> START src/math/rint.c

double rint(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = u.i>>52 & 0x7ff;
    int s = u.i>>63;
    double_t y;

    if (e >= 0x3ff+52)
        return x;
    if (s)
        y = (double)(x - 0x1p52) + 0x1p52;
    else
        y = (double)(x + 0x1p52) - 0x1p52;
    if (y == 0)
        return s ? -0.0 : 0;
    return y;
}

/// >>> START src/math/rintf.c

float rintf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = u.i>>23 & 0xff;
    int s = u.i>>31;
    float_t y;

    if (e >= 0x7f+23)
        return x;
    if (s)
        y = (float)(x - 0x1p23f) + 0x1p23f;
    else
        y = (float)(x + 0x1p23f) - 0x1p23f;
    if (y == 0)
        return s ? -0.0f : 0.0f;
    return y;
}

/// >>> START src/math/rintl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double rintl(long double x) {
    return rint(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double rintl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    int s = u.i.se >> 15;
    long double y;

    if (e >= 0x3fff+LDBL_MANT_DIG-1)
        return x;
    if (s)
        y = x - TOINT + TOINT;
    else
        y = x + TOINT - TOINT;
    if (y == 0)
        return 0*x;
    return y;
}
#endif

/// >>> START src/math/round.c

double round(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = u.i >> 52 & 0x7ff;
    double_t y;

    if (e >= 0x3ff+52)
        return x;
    if (u.i >> 63)
        x = -x;
    if (e < 0x3ff-1) {
        /* raise inexact if x!=0 */
        FORCE_EVAL(x + 0x1p52);
        return 0*u.f;
    }
    y = (double)(x + 0x1p52) - 0x1p52 - x;
    if (y > 0.5)
        y = y + x - 1;
    else if (y <= -0.5)
        y = y + x + 1;
    else
        y = y + x;
    if (u.i >> 63)
        y = -y;
    return y;
}

/// >>> START src/math/roundf.c

float roundf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = u.i >> 23 & 0xff;
    float_t y;

    if (e >= 0x7f+23)
        return x;
    if (u.i >> 31)
        x = -x;
    if (e < 0x7f-1) {
        FORCE_EVAL(x + 0x1p23f);
        return 0*u.f;
    }
    y = (float)(x + 0x1p23f) - 0x1p23f - x;
    if (y > 0.5f)
        y = y + x - 1;
    else if (y <= -0.5f)
        y = y + x + 1;
    else
        y = y + x;
    if (u.i >> 31)
        y = -y;
    return y;
}

/// >>> START src/math/roundl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double roundl(long double x) {
    return round(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double roundl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    long double y;

    if (e >= 0x3fff+LDBL_MANT_DIG-1)
        return x;
    if (u.i.se >> 15)
        x = -x;
    if (e < 0x3fff-1) {
        FORCE_EVAL(x + TOINT);
        return 0*u.f;
    }
    y = x + TOINT - TOINT - x;
    if (y > 0.5)
        y = y + x - 1;
    else if (y <= -0.5)
        y = y + x + 1;
    else
        y = y + x;
    if (u.i.se >> 15)
        y = -y;
    return y;
}
#endif

/// >>> START src/math/scalb.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_scalb.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * scalb(x, fn) is provide for
 * passing various standard test suite. One
 * should use scalbn() instead.
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

double scalb(double x, double fn) {
    if (isnan(x) || isnan(fn))
        return x*fn;
    if (!isfinite(fn)) {
        if (fn > 0.0)
            return x*fn;
        else
            return x/(-fn);
    }
    if (rint(fn) != fn) return (fn-fn)/(fn-fn);
    if ( fn > 65000.0) return scalbn(x, 65000);
    if (-fn > 65000.0) return scalbn(x,-65000);
    return scalbn(x,(int)fn);
}

/// >>> START src/math/scalbf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_scalbf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

float scalbf(float x, float fn) {
    if (isnan(x) || isnan(fn)) return x*fn;
    if (!isfinite(fn)) {
        if (fn > 0.0f)
            return x*fn;
        else
            return x/(-fn);
    }
    if (rintf(fn) != fn) return (fn-fn)/(fn-fn);
    if ( fn > 65000.0f) return scalbnf(x, 65000);
    if (-fn > 65000.0f) return scalbnf(x,-65000);
    return scalbnf(x,(int)fn);
}

/// >>> START src/math/scalbln.c

double scalbln(double x, long n) {
    if (n > INT_MAX)
        n = INT_MAX;
    else if (n < INT_MIN)
        n = INT_MIN;
    return scalbn(x, n);
}

/// >>> START src/math/scalblnf.c

float scalblnf(float x, long n) {
    if (n > INT_MAX)
        n = INT_MAX;
    else if (n < INT_MIN)
        n = INT_MIN;
    return scalbnf(x, n);
}

/// >>> START src/math/scalblnl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double scalblnl(long double x, long n) {
    return scalbln(x, n);
}
#else
long double scalblnl(long double x, long n) {
    if (n > INT_MAX)
        n = INT_MAX;
    else if (n < INT_MIN)
        n = INT_MIN;
    return scalbnl(x, n);
}
#endif

/// >>> START src/math/scalbn.c

double scalbn(double x, int n) {
    union {double f; uint64_t i;} u;
    double_t y = x;

    if (n > 1023) {
        y *= 0x1p1023;
        n -= 1023;
        if (n > 1023) {
            y *= 0x1p1023;
            n -= 1023;
            if (n > 1023)
                n = 1023;
        }
    } else if (n < -1022) {
        y *= 0x1p-1022;
        n += 1022;
        if (n < -1022) {
            y *= 0x1p-1022;
            n += 1022;
            if (n < -1022)
                n = -1022;
        }
    }
    u.i = (uint64_t)(0x3ff+n)<<52;
    x = y * u.f;
    return x;
}

/// >>> START src/math/scalbnf.c

float scalbnf(float x, int n) {
    union {float f; uint32_t i;} u;
    float_t y = x;

    if (n > 127) {
        y *= 0x1p127f;
        n -= 127;
        if (n > 127) {
            y *= 0x1p127f;
            n -= 127;
            if (n > 127)
                n = 127;
        }
    } else if (n < -126) {
        y *= 0x1p-126f;
        n += 126;
        if (n < -126) {
            y *= 0x1p-126f;
            n += 126;
            if (n < -126)
                n = -126;
        }
    }
    u.i = (uint32_t)(0x7f+n)<<23;
    x = y * u.f;
    return x;
}

/// >>> START src/math/scalbnl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double scalbnl(long double x, int n) {
    return scalbn(x, n);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double scalbnl(long double x, int n) {
    union ldshape u;

    if (n > 16383) {
        x *= 0x1p16383L;
        n -= 16383;
        if (n > 16383) {
            x *= 0x1p16383L;
            n -= 16383;
            if (n > 16383)
                n = 16383;
        }
    } else if (n < -16382) {
        x *= 0x1p-16382L;
        n += 16382;
        if (n < -16382) {
            x *= 0x1p-16382L;
            n += 16382;
            if (n < -16382)
                n = -16382;
        }
    }
    u.f = 1.0;
    u.i.se = 0x3fff + n;
    return x * u.f;
}
#endif

/// >>> START src/math/signgam.c

int __signgam = 0;

weak_alias(__signgam, signgam);

/// >>> START src/math/significand.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

double significand(double x) {
    return scalbn(x, -ilogb(x));
}

/// >>> START src/math/significandf.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

float significandf(float x) {
    return scalbnf(x, -ilogbf(x));
}

/// >>> START src/math/sin.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_sin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *      __sin            ... sine function on [-sin_c__pi/4,sin_c__pi/4]
 *      __cos            ... cose function on [-sin_c__pi/4,sin_c__pi/4]
 *      __rem_pio2       ... argument reduction routine
 *
 * Method.
 *      Let sin_c__S,C and sin_c__T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-sin_c__k*sin_c__pi/2
 *      in [-sin_c__pi/4 , +sin_c__pi/4], and let n = sin_c__k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          sin_c__S           C             sin_c__T
 *          1          C          -sin_c__S            -1/sin_c__T
 *          2         -sin_c__S          -C             sin_c__T
 *          3         -C           sin_c__S            -1/sin_c__T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double sin(double x) {
    double y[2];
    uint32_t ix;
    unsigned n;

    /* High word of x. */
    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    /* |x| ~< sin_c__pi/4 */
    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e500000) {  /* |x| < 2**-26 */
            /* raise inexact if x != 0 and underflow if subnormal*/
            FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __sin(x, 0.0, 0);
    }

    /* sin(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000)
        return x - x;

    /* argument reduction needed */
    n = __rem_pio2(x, y);
    switch (n&3) {
    case 0: return  __sin(y[0], y[1], 1);
    case 1: return  __cos(y[0], y[1]);
    case 2: return -__sin(y[0], y[1], 1);
    default:
        return -__cos(y[0], y[1]);
    }
}

/// >>> START src/math/sincos.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_sin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

void sincos(double x, double *sin, double *cos) {
    double y[2], s, c;
    uint32_t ix;
    unsigned n;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    /* |x| ~< sincos_c__pi/4 */
    if (ix <= 0x3fe921fb) {
        /* if |x| < 2**-27 * sqrt(2) */
        if (ix < 0x3e46a09e) {
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
            *sin = x;
            *cos = 1.0;
            return;
        }
        *sin = __sin(x, 0.0, 0);
        *cos = __cos(x, 0.0);
        return;
    }

    /* sincos(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000) {
        *sin = *cos = x - x;
        return;
    }

    /* argument reduction needed */
    n = __rem_pio2(x, y);
    s = __sin(y[0], y[1], 1);
    c = __cos(y[0], y[1]);
    switch (n&3) {
    case 0:
        *sin = s;
        *cos = c;
        break;
    case 1:
        *sin = c;
        *cos = -s;
        break;
    case 2:
        *sin = -s;
        *cos = -c;
        break;
    case 3:
    default:
        *sin = -c;
        *cos = s;
        break;
    }
}

/// >>> START src/math/sincosf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_sinf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#undef _GNU_SOURCE
#define _GNU_SOURCE

/* Small multiples of sincosf_c__pi/2 rounded to double precision. */
static const double
sincosf_c__s1pio2 = 1*M_PI_2, /* 0x3FF921FB, 0x54442D18 */
sincosf_c__s2pio2 = 2*M_PI_2, /* 0x400921FB, 0x54442D18 */
sincosf_c__s3pio2 = 3*M_PI_2, /* 0x4012D97C, 0x7F3321D2 */
sincosf_c__s4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

void sincosf(float x, float *sin, float *cos) {
    double y;
    float_t s, c;
    uint32_t ix;
    unsigned n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    /* |x| ~<= sincosf_c__pi/4 */
    if (ix <= 0x3f490fda) {
        /* |x| < 2**-12 */
        if (ix < 0x39800000) {
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
            *sin = x;
            *cos = 1.0f;
            return;
        }
        *sin = __sindf(x);
        *cos = __cosdf(x);
        return;
    }

    /* |x| ~<= 5*sincosf_c__pi/4 */
    if (ix <= 0x407b53d1) {
        if (ix <= 0x4016cbe3) {  /* |x| ~<= 3pi/4 */
            if (sign) {
                *sin = -__cosdf(x + sincosf_c__s1pio2);
                *cos = __sindf(x + sincosf_c__s1pio2);
            } else {
                *sin = __cosdf(sincosf_c__s1pio2 - x);
                *cos = __sindf(sincosf_c__s1pio2 - x);
            }
            return;
        }
        /* -sin(x+c) is not correct if x+c could be 0: -0 vs +0 */
        *sin = -__sindf(sign ? x + sincosf_c__s2pio2 : x - sincosf_c__s2pio2);
        *cos = -__cosdf(sign ? x + sincosf_c__s2pio2 : x - sincosf_c__s2pio2);
        return;
    }

    /* |x| ~<= 9*sincosf_c__pi/4 */
    if (ix <= 0x40e231d5) {
        if (ix <= 0x40afeddf) {  /* |x| ~<= 7*sincosf_c__pi/4 */
            if (sign) {
                *sin = __cosdf(x + sincosf_c__s3pio2);
                *cos = -__sindf(x + sincosf_c__s3pio2);
            } else {
                *sin = -__cosdf(x - sincosf_c__s3pio2);
                *cos = __sindf(x - sincosf_c__s3pio2);
            }
            return;
        }
        *sin = __sindf(sign ? x + sincosf_c__s4pio2 : x - sincosf_c__s4pio2);
        *cos = __cosdf(sign ? x + sincosf_c__s4pio2 : x - sincosf_c__s4pio2);
        return;
    }

    /* sin(Inf or NaN) is NaN */
    if (ix >= 0x7f800000) {
        *sin = *cos = x - x;
        return;
    }

    /* general argument reduction needed */
    n = __rem_pio2f(x, &y);
    s = __sindf(y);
    c = __cosdf(y);
    switch (n&3) {
    case 0:
        *sin = s;
        *cos = c;
        break;
    case 1:
        *sin = c;
        *cos = -s;
        break;
    case 2:
        *sin = -s;
        *cos = -c;
        break;
    case 3:
    default:
        *sin = -c;
        *cos = s;
        break;
    }
}

/// >>> START src/math/sincosl.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
void sincosl(long double x, long double *sin, long double *cos) {
    sincos(x, (double *)sin, (double *)cos);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
void sincosl(long double x, long double *sin, long double *cos) {
    union ldshape u = {x};
    unsigned n;
    long double y[2], s, c;

    u.i.se &= 0x7fff;
    if (u.i.se == 0x7fff) {
        *sin = *cos = x - x;
        return;
    }
    if (u.f < M_PI_4) {
        if (u.i.se < 0x3fff - LDBL_MANT_DIG) {
            /* raise underflow if subnormal */
            if (u.i.se == 0) FORCE_EVAL(x*0x1p-120f);
            *sin = x;
            /* raise inexact if x!=0 */
            *cos = 1.0 + x;
            return;
        }
        *sin = __sinl(x, 0, 0);
        *cos = __cosl(x, 0);
        return;
    }
    n = __rem_pio2l(x, y);
    s = __sinl(y[0], y[1], 1);
    c = __cosl(y[0], y[1]);
    switch (n & 3) {
    case 0:
        *sin = s;
        *cos = c;
        break;
    case 1:
        *sin = c;
        *cos = -s;
        break;
    case 2:
        *sin = -s;
        *cos = -c;
        break;
    case 3:
    default:
        *sin = -c;
        *cos = s;
        break;
    }
}
#endif

/// >>> START src/math/sinf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_sinf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* Small multiples of sinf_c__pi/2 rounded to double precision. */
static const double
sinf_c__s1pio2 = 1*M_PI_2, /* 0x3FF921FB, 0x54442D18 */
sinf_c__s2pio2 = 2*M_PI_2, /* 0x400921FB, 0x54442D18 */
sinf_c__s3pio2 = 3*M_PI_2, /* 0x4012D97C, 0x7F3321D2 */
sinf_c__s4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

float sinf(float x) {
    double y;
    uint32_t ix;
    int n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= sinf_c__pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00800000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __sindf(x);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*sinf_c__pi/4 */
        if (ix <= 0x4016cbe3) {  /* |x| ~<= 3pi/4 */
            if (sign)
                return -__cosdf(x + sinf_c__s1pio2);
            else
                return __cosdf(x - sinf_c__s1pio2);
        }
        return __sindf(sign ? -(x + sinf_c__s2pio2) : -(x - sinf_c__s2pio2));
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*sinf_c__pi/4 */
        if (ix <= 0x40afeddf) {  /* |x| ~<= 7*sinf_c__pi/4 */
            if (sign)
                return __cosdf(x + sinf_c__s3pio2);
            else
                return -__cosdf(x - sinf_c__s3pio2);
        }
        return __sindf(sign ? x + sinf_c__s4pio2 : x - sinf_c__s4pio2);
    }

    /* sin(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x - x;

    /* general argument reduction needed */
    n = __rem_pio2f(x, &y);
    switch (n&3) {
    case 0: return  __sindf(y);
    case 1: return  __cosdf(y);
    case 2: return  __sindf(-y);
    default:
        return -__cosdf(y);
    }
}

/// >>> START src/math/sinh.c

/* sinh(x) = (exp(x) - 1/exp(x))/2
 *         = (exp(x)-1 + (exp(x)-1)/exp(x))/2
 *         = x + x^3/6 + o(x^5)
 */
double sinh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    uint32_t w;
    double t, h, absx;

    h = 0.5;
    if (u.i >> 63)
        h = -h;
    /* |x| */
    u.i &= (uint64_t)-1/2;
    absx = u.f;
    w = u.i >> 32;

    /* |x| < log(DBL_MAX) */
    if (w < 0x40862e42) {
        t = expm1(absx);
        if (w < 0x3ff00000) {
            if (w < 0x3ff00000 - (26<<20))
                /* note: inexact and underflow are raised by expm1 */
                /* note: this branch avoids spurious underflow */
                return x;
            return h*(2*t - t*t/(t+1));
        }
        /* note: |x|>log(0x1p26)+eps could be just h*exp(x) */
        return h*(t + t/(t+1));
    }

    /* |x| > log(DBL_MAX) or nan */
    /* note: the result is stored to handle overflow */
    t = 2*h*__expo2(absx);
    return t;
}

/// >>> START src/math/sinhf.c

float sinhf(float x) {
    union {float f; uint32_t i;} u = {.f = x};
    uint32_t w;
    float t, h, absx;

    h = 0.5;
    if (u.i >> 31)
        h = -h;
    /* |x| */
    u.i &= 0x7fffffff;
    absx = u.f;
    w = u.i;

    /* |x| < log(FLT_MAX) */
    if (w < 0x42b17217) {
        t = expm1f(absx);
        if (w < 0x3f800000) {
            if (w < 0x3f800000 - (12<<23))
                return x;
            return h*(2*t - t*t/(t+1));
        }
        return h*(t + t/(t+1));
    }

    /* |x| > logf(FLT_MAX) or nan */
    t = 2*h*__expo2f(absx);
    return t;
}

/// >>> START src/math/sinhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double sinhl(long double x) {
    return sinh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
long double sinhl(long double x) {
    union ldshape u = {x};
    unsigned ex = u.i.se & 0x7fff;
    long double h, t, absx;

    h = 0.5;
    if (u.i.se & 0x8000)
        h = -h;
    /* |x| */
    u.i.se = ex;
    absx = u.f;

    /* |x| < log(LDBL_MAX) */
    if (ex < 0x3fff+13 || (ex == 0x3fff+13 && u.i.m>>32 < 0xb17217f7)) {
        t = expm1l(absx);
        if (ex < 0x3fff) {
            if (ex < 0x3fff-32)
                return x;
            return h*(2*t - t*t/(1+t));
        }
        return h*(t + t/(t+1));
    }

    /* |x| > log(LDBL_MAX) or nan */
    t = expl(0.5*absx);
    return h*t*t;
}
#endif

/// >>> START src/math/sinl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double sinl(long double x) {
    return sin(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double sinl(long double x) {
    union ldshape u = {x};
    unsigned n;
    long double y[2], hi, lo;

    u.i.se &= 0x7fff;
    if (u.i.se == 0x7fff)
        return x - x;
    if (u.f < M_PI_4) {
        if (u.i.se < 0x3fff - LDBL_MANT_DIG/2) {
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(u.i.se == 0 ? x*0x1p-120f : x+0x1p120f);
            return x;
        }
        return __sinl(x, 0.0, 0);
    }
    n = __rem_pio2l(x, y);
    hi = y[0];
    lo = y[1];
    switch (n & 3) {
    case 0:
        return __sinl(hi, lo, 1);
    case 1:
        return __cosl(hi, lo);
    case 2:
        return -__sinl(hi, lo, 1);
    case 3:
    default:
        return -__cosl(hi, lo);
    }
}
#endif

/// >>> START src/math/sqrt.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_sqrt.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* sqrt(x)
 * Return correctly rounded sqrt.
 *           ------------------------------------------
 *           |  Use the hardware sqrt if you have one |
 *           ------------------------------------------
 * Method:
 *   Bit by bit method using integer arithmetic. (Slow, but portable)
 *   1. Normalization
 *      Scale x to y in [1,4) with even powers of 2:
 *      find an integer sqrt_c__k such that  1 <= (y=x*2^(2k)) < 4, then
 *              sqrt(x) = 2^sqrt_c__k * sqrt(y)
 *   2. Bit by bit computation
 *      Let q  = sqrt(y) truncated to i bit after binary point (q = 1),
 *           i                                                   0
 *                                     i+1         2
 *          s  = 2*q , and      y  =  2   * ( y - q  ).         (1)
 *           i      i            i                 i
 *
 *      To compute q    from q , one checks whether
 *                  i+1       i
 *
 *                            -(i+1) 2
 *                      (q + 2      ) <= y.                     (2)
 *                        i
 *                                                            -(i+1)
 *      If (2) is false, then q   = q ; otherwise q   = q  + 2      .
 *                             i+1   i             i+1   i
 *
 *      With some algebric manipulation, it is not difficult to see
 *      that (2) is equivalent to
 *                             -(i+1)
 *                      s  +  2       <= y                      (3)
 *                       i                i
 *
 *      The advantage of (3) is that s  and y  can be computed by
 *                                    i      i
 *      the following recurrence formula:
 *          if (3) is false
 *
 *          s     =  s  ,       y    = y   ;                    (4)
 *           i+1      i          i+1    i
 *
 *          otherwise,
 *                         -i                     -(i+1)
 *          s     =  s  + 2  ,  y    = y  -  s  - 2             (5)
 *           i+1      i          i+1    i     i
 *
 *      One may easily use induction to prove (4) and (5).
 *      Note. Since the left hand side of (3) contain only i+2 bits,
 *            it does not necessary to do a full (53-bit) comparison
 *            in (3).
 *   3. Final rounding
 *      After generating the 53 bits result, we compute one more bit.
 *      Together with the remainder, we can decide whether the
 *      result is exact, bigger than 1/2ulp, or less than 1/2ulp
 *      (it will never equal to 1/2ulp).
 *      The rounding mode can be detected by checking whether
 *      sqrt_c__huge + sqrt_c__tiny is equal to sqrt_c__huge, and whether sqrt_c__huge - sqrt_c__tiny is
 *      equal to sqrt_c__huge for some floating point number "sqrt_c__huge" and "sqrt_c__tiny".
 *
 * Special cases:
 *      sqrt(+-0) = +-0         ... exact
 *      sqrt(inf) = inf
 *      sqrt(-ve) = NaN         ... with invalid signal
 *      sqrt(NaN) = NaN         ... with invalid signal for signaling NaN
 */

static const double sqrt_c__tiny = 1.0e-300;

double sqrt(double x) {
    double z;
    int32_t sign = (int)0x80000000;
    int32_t ix0,sqrt_c__s0,q,m,t,i;
    uint32_t r,sqrt_c__t1,sqrt_c__s1,ix1,q1;

    EXTRACT_WORDS(ix0, ix1, x);

    /* take care of Inf and NaN */
    if ((ix0&0x7ff00000) == 0x7ff00000) {
        return x*x + x;  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */
    }
    /* take care of sqrt_c__zero */
    if (ix0 <= 0) {
        if (((ix0&~sign)|ix1) == 0)
            return x;  /* sqrt(+-0) = +-0 */
        if (ix0 < 0)
            return (x-x)/(x-x);  /* sqrt(-ve) = sNaN */
    }
    /* normalize x */
    m = ix0>>20;
    if (m == 0) {  /* subnormal x */
        while (ix0 == 0) {
            m -= 21;
            ix0 |= (ix1>>11);
            ix1 <<= 21;
        }
        for (i=0; (ix0&0x00100000) == 0; i++)
            ix0<<=1;
        m -= i - 1;
        ix0 |= ix1>>(32-i);
        ix1 <<= i;
    }
    m -= 1023;    /* unbias exponent */
    ix0 = (ix0&0x000fffff)|0x00100000;
    if (m & 1) {  /* odd m, double x to make it even */
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
    }
    m >>= 1;      /* m = [m/2] */

    /* generate sqrt(x) bit by bit */
    ix0 += ix0 + ((ix1&sign)>>31);
    ix1 += ix1;
    q = q1 = sqrt_c__s0 = sqrt_c__s1 = 0;  /* [q,q1] = sqrt(x) */
    r = 0x00200000;        /* r = moving bit from right to left */

    while (r != 0) {
        t = sqrt_c__s0 + r;
        if (t <= ix0) {
            sqrt_c__s0   = t + r;
            ix0 -= t;
            q   += r;
        }
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
        r >>= 1;
    }

    r = sign;
    while (r != 0) {
        sqrt_c__t1 = sqrt_c__s1 + r;
        t  = sqrt_c__s0;
        if (t < ix0 || (t == ix0 && sqrt_c__t1 <= ix1)) {
            sqrt_c__s1 = sqrt_c__t1 + r;
            if ((sqrt_c__t1&sign) == sign && (sqrt_c__s1&sign) == 0)
                sqrt_c__s0++;
            ix0 -= t;
            if (ix1 < sqrt_c__t1)
                ix0--;
            ix1 -= sqrt_c__t1;
            q1 += r;
        }
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
        r >>= 1;
    }

    /* use floating add to find out rounding direction */
    if ((ix0|ix1) != 0) {
        z = 1.0 - sqrt_c__tiny; /* raise inexact flag */
        if (z >= 1.0) {
            z = 1.0 + sqrt_c__tiny;
            if (q1 == (uint32_t)0xffffffff) {
                q1 = 0;
                q++;
            } else if (z > 1.0) {
                if (q1 == (uint32_t)0xfffffffe)
                    q++;
                q1 += 2;
            } else
                q1 += q1 & 1;
        }
    }
    ix0 = (q>>1) + 0x3fe00000;
    ix1 = q1>>1;
    if (q&1)
        ix1 |= sign;
    ix0 += m << 20;
    INSERT_WORDS(z, ix0, ix1);
    return z;
}

/// >>> START src/math/sqrtf.c
/* origin: FreeBSD /usr/src/lib/msun/src/e_sqrtf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const float sqrtf_c__tiny = 1.0e-30;

float sqrtf(float x) {
    float z;
    int32_t sign = (int)0x80000000;
    int32_t ix,s,q,m,t,i;
    uint32_t r;

    GET_FLOAT_WORD(ix, x);

    /* take care of Inf and NaN */
    if ((ix&0x7f800000) == 0x7f800000)
        return x*x + x; /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */

    /* take care of sqrtf_c__zero */
    if (ix <= 0) {
        if ((ix&~sign) == 0)
            return x;  /* sqrt(+-0) = +-0 */
        if (ix < 0)
            return (x-x)/(x-x);  /* sqrt(-ve) = sNaN */
    }
    /* normalize x */
    m = ix>>23;
    if (m == 0) {  /* subnormal x */
        for (i = 0; (ix&0x00800000) == 0; i++)
            ix<<=1;
        m -= i - 1;
    }
    m -= 127;  /* unbias exponent */
    ix = (ix&0x007fffff)|0x00800000;
    if (m&1)  /* odd m, double x to make it even */
        ix += ix;
    m >>= 1;  /* m = [m/2] */

    /* generate sqrt(x) bit by bit */
    ix += ix;
    q = s = 0;       /* q = sqrt(x) */
    r = 0x01000000;  /* r = moving bit from right to left */

    while (r != 0) {
        t = s + r;
        if (t <= ix) {
            s = t+r;
            ix -= t;
            q += r;
        }
        ix += ix;
        r >>= 1;
    }

    /* use floating add to find out rounding direction */
    if (ix != 0) {
        z = 1.0f - sqrtf_c__tiny; /* raise inexact flag */
        if (z >= 1.0f) {
            z = 1.0f + sqrtf_c__tiny;
            if (z > 1.0f)
                q += 2;
            else
                q += q & 1;
        }
    }
    ix = (q>>1) + 0x3f000000;
    ix += m << 23;
    SET_FLOAT_WORD(z, ix);
    return z;
}

/// >>> START src/math/sqrtl.c

long double sqrtl(long double x) {
    /* FIXME: implement in C, this is for LDBL_MANT_DIG == 64 only */
    return sqrt(x);
}

/// >>> START src/math/tan.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_tan.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* tan(x)
 * Return tangent function of x.
 *
 * kernel function:
 *      __tan           ... tangent function on [-tan_c__pi/4,tan_c__pi/4]
 *      __rem_pio2      ... argument reduction routine
 *
 * Method.
 *      Let tan_c__S,C and tan_c__T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-tan_c__k*tan_c__pi/2
 *      in [-tan_c__pi/4 , +tan_c__pi/4], and let n = tan_c__k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          tan_c__S           C             tan_c__T
 *          1          C          -tan_c__S            -1/tan_c__T
 *          2         -tan_c__S          -C             tan_c__T
 *          3         -C           tan_c__S            -1/tan_c__T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double tan(double x) {
    double y[2];
    uint32_t ix;
    unsigned n;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    /* |x| ~< tan_c__pi/4 */
    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e400000) { /* |x| < 2**-27 */
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __tan(x, 0.0, 0);
    }

    /* tan(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000)
        return x - x;

    /* argument reduction */
    n = __rem_pio2(x, y);
    return __tan(y[0], y[1], n&1);
}

/// >>> START src/math/tanf.c
/* origin: FreeBSD /usr/src/lib/msun/src/s_tanf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* Small multiples of tanf_c__pi/2 rounded to double precision. */
static const double
t1pio2 = 1*M_PI_2, /* 0x3FF921FB, 0x54442D18 */
t2pio2 = 2*M_PI_2, /* 0x400921FB, 0x54442D18 */
t3pio2 = 3*M_PI_2, /* 0x4012D97C, 0x7F3321D2 */
t4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

float tanf(float x) {
    double y;
    uint32_t ix;
    unsigned n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= tanf_c__pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00800000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __tandf(x, 0);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*tanf_c__pi/4 */
        if (ix <= 0x4016cbe3)  /* |x| ~<= 3pi/4 */
            return __tandf((sign ? x+t1pio2 : x-t1pio2), 1);
        else
            return __tandf((sign ? x+t2pio2 : x-t2pio2), 0);
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*tanf_c__pi/4 */
        if (ix <= 0x40afeddf)  /* |x| ~<= 7*tanf_c__pi/4 */
            return __tandf((sign ? x+t3pio2 : x-t3pio2), 1);
        else
            return __tandf((sign ? x+t4pio2 : x-t4pio2), 0);
    }

    /* tan(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x - x;

    /* argument reduction */
    n = __rem_pio2f(x, &y);
    return __tandf(y, n&1);
}

/// >>> START src/math/tanh.c

/* tanh(x) = (exp(x) - exp(-x))/(exp(x) + exp(-x))
 *         = (exp(2*x) - 1)/(exp(2*x) - 1 + 2)
 *         = (1 - exp(-2*x))/(exp(-2*x) - 1 + 2)
 */
double tanh(double x) {
    union {double f; uint64_t i;} u = {.f = x};
    uint32_t w;
    int sign;
    double_t t;

    /* x = |x| */
    sign = u.i >> 63;
    u.i &= (uint64_t)-1/2;
    x = u.f;
    w = u.i >> 32;

    if (w > 0x3fe193ea) {
        /* |x| > log(3)/2 ~= 0.5493 or nan */
        if (w > 0x40340000) {
            /* |x| > 20 or nan */
            /* note: this branch avoids raising overflow */
            t = 1 - 0/x;
        } else {
            t = expm1(2*x);
            t = 1 - 2/(t+2);
        }
    } else if (w > 0x3fd058ae) {
        /* |x| > log(5/3)/2 ~= 0.2554 */
        t = expm1(2*x);
        t = t/(t+2);
    } else if (w >= 0x00100000) {
        /* |x| >= 0x1p-1022, up to 2ulp error in [0.1,0.2554] */
        t = expm1(-2*x);
        t = -t/(t+2);
    } else {
        /* |x| is subnormal */
        /* note: the branch above would not raise underflow in [0x1p-1023,0x1p-1022) */
        FORCE_EVAL((float)x);
        t = x;
    }
    return sign ? -t : t;
}

/// >>> START src/math/tanhf.c

float tanhf(float x) {
    union {float f; uint32_t i;} u = {.f = x};
    uint32_t w;
    int sign;
    float t;

    /* x = |x| */
    sign = u.i >> 31;
    u.i &= 0x7fffffff;
    x = u.f;
    w = u.i;

    if (w > 0x3f0c9f54) {
        /* |x| > log(3)/2 ~= 0.5493 or nan */
        if (w > 0x41200000) {
            /* |x| > 10 */
            t = 1 + 0/x;
        } else {
            t = expm1f(2*x);
            t = 1 - 2/(t+2);
        }
    } else if (w > 0x3e82c578) {
        /* |x| > log(5/3)/2 ~= 0.2554 */
        t = expm1f(2*x);
        t = t/(t+2);
    } else if (w >= 0x00800000) {
        /* |x| >= 0x1p-126 */
        t = expm1f(-2*x);
        t = -t/(t+2);
    } else {
        /* |x| is subnormal */
        FORCE_EVAL(x*x);
        t = x;
    }
    return sign ? -t : t;
}

/// >>> START src/math/tanhl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double tanhl(long double x) {
    return tanh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
long double tanhl(long double x) {
    union ldshape u = {x};
    unsigned ex = u.i.se & 0x7fff;
    unsigned sign = u.i.se & 0x8000;
    uint32_t w;
    long double t;

    /* x = |x| */
    u.i.se = ex;
    x = u.f;
    w = u.i.m >> 32;

    if (ex > 0x3ffe || (ex == 0x3ffe && w > 0x8c9f53d5)) {
        /* |x| > log(3)/2 ~= 0.5493 or nan */
        if (ex >= 0x3fff+5) {
            /* |x| >= 32 */
            t = 1 + 0/(x + 0x1p-120f);
        } else {
            t = expm1l(2*x);
            t = 1 - 2/(t+2);
        }
    } else if (ex > 0x3ffd || (ex == 0x3ffd && w > 0x82c577d4)) {
        /* |x| > log(5/3)/2 ~= 0.2554 */
        t = expm1l(2*x);
        t = t/(t+2);
    } else {
        /* |x| is small */
        t = expm1l(-2*x);
        t = -t/(t+2);
    }
    return sign ? -t : t;
}
#endif

/// >>> START src/math/tanl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double tanl(long double x) {
    return tan(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double tanl(long double x) {
    union ldshape u = {x};
    long double y[2];
    unsigned n;

    u.i.se &= 0x7fff;
    if (u.i.se == 0x7fff)
        return x - x;
    if (u.f < M_PI_4) {
        if (u.i.se < 0x3fff - LDBL_MANT_DIG/2) {
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(u.i.se == 0 ? x*0x1p-120f : x+0x1p120f);
            return x;
        }
        return __tanl(x, 0, 0);
    }
    n = __rem_pio2l(x, y);
    return __tanl(y[0], y[1], n&1);
}
#endif

/// >>> START src/math/tgamma.c
/*
"A Precision Approximation of the Gamma Function" - Cornelius Lanczos (1964)
"Lanczos Implementation of the Gamma Function" - Paul Godfrey (2001)
"An Analysis of the Lanczos Gamma Approximation" - Glendon Ralph Pugh (2004)

approximation method:

                        (x - 0.5)         tgamma_c__S(x)
Gamma(x) = (x + g - 0.5)         *  ----------------
                                    exp(x + g - 0.5)

with
                 tgamma_c__a1      tgamma_c__a2      tgamma_c__a3            aN
tgamma_c__S(x) ~= [ tgamma_c__a0 + ----- + ----- + ----- + ... + ----- ]
               x + 1   x + 2   x + 3         x + N

with tgamma_c__a0, tgamma_c__a1, tgamma_c__a2, tgamma_c__a3,.. aN constants which depend on g.

for x < 0 the following reflection formula is used:

Gamma(x)*Gamma(-x) = -tgamma_c__pi/(x sin(tgamma_c__pi x))

most ideas and constants are from boost and python
*/

static const double tgamma_c__pi = 3.141592653589793238462643383279502884;

/* sin(tgamma_c__pi x) with x > 0x1p-100, if sin(tgamma_c__pi*x)==0 the sign is arbitrary */
static double sinpi(double x) {
    int n;

    /* argument reduction: x = |x| mod 2 */
    /* spurious inexact when x is odd int */
    x = x * 0.5;
    x = 2 * (x - floor(x));

    /* reduce x into [-.25,.25] */
    n = 4 * x;
    n = (n+1)/2;
    x -= n * 0.5;

    x *= tgamma_c__pi;
    switch (n) {
    default: /* case 4 */
    case 0:
        return __sin(x, 0, 0);
    case 1:
        return __cos(x, 0);
    case 2:
        return __sin(-x, 0, 0);
    case 3:
        return -__cos(x, 0);
    }
}

#undef N
#define N 12
//static const double g = 6.024680040776729583740234375;
static const double gmhalf = 5.524680040776729583740234375;
static const double Snum[N+1] = {
    23531376880.410759688572007674451636754734846804940,
    42919803642.649098768957899047001988850926355848959,
    35711959237.355668049440185451547166705960488635843,
    17921034426.037209699919755754458931112671403265390,
    6039542586.3520280050642916443072979210699388420708,
    1439720407.3117216736632230727949123939715485786772,
    248874557.86205415651146038641322942321632125127801,
    31426415.585400194380614231628318205362874684987640,
    2876370.6289353724412254090516208496135991145378768,
    186056.26539522349504029498971604569928220784236328,
    8071.6720023658162106380029022722506138218516325024,
    210.82427775157934587250973392071336271166969580291,
    2.5066282746310002701649081771338373386264310793408,
};
static const double Sden[N+1] = {
    0, 39916800, 120543840, 150917976, 105258076, 45995730, 13339535,
    2637558, 357423, 32670, 1925, 66, 1,
};
/* n! for small integer n */
static const double fact[] = {
    1, 1, 2, 6, 24, 120, 720, 5040.0, 40320.0, 362880.0, 3628800.0, 39916800.0,
    479001600.0, 6227020800.0, 87178291200.0, 1307674368000.0, 20922789888000.0,
    355687428096000.0, 6402373705728000.0, 121645100408832000.0,
    2432902008176640000.0, 51090942171709440000.0, 1124000727777607680000.0,
};

/* tgamma_c__S(x) rational function for positive x */
static double tgamma_c__S(double x) {
    double_t num = 0, den = 0;
    int i;

    /* to avoid overflow handle large x differently */
    if (x < 8)
        for (i = N; i >= 0; i--) {
            num = num * x + Snum[i];
            den = den * x + Sden[i];
        }
    else
        for (i = 0; i <= N; i++) {
            num = num / x + Snum[i];
            den = den / x + Sden[i];
        }
    return num/den;
}

double tgamma(double x) {
    union {double f; uint64_t i;} u = {x};
    double absx, y;
    double_t dy, z, r;
    uint32_t ix = u.i>>32 & 0x7fffffff;
    int sign = u.i>>63;

    /* special cases */
    if (ix >= 0x7ff00000)
        /* tgamma(nan)=nan, tgamma(inf)=inf, tgamma(-inf)=nan with invalid */
        return x + INFINITY;
    if (ix < (0x3ff-54)<<20)
        /* |x| < 2^-54: tgamma(x) ~ 1/x, +-0 raises div-by-tgamma_c__zero */
        return 1/x;

    /* integer arguments */
    /* raise inexact when non-integer */
    if (x == floor(x)) {
        if (sign)
            return 0/0.0;
        if (x <= sizeof fact/sizeof *fact)
            return fact[(int)x - 1];
    }

    /* x >= 172: tgamma(x)=inf with overflow */
    /* x =< -184: tgamma(x)=+-0 with underflow */
    if (ix >= 0x40670000) { /* |x| >= 184 */
        if (sign) {
            FORCE_EVAL((float)(0x1p-126/x));
            if (floor(x) * 0.5 == floor(x * 0.5))
                return 0;
            return -0.0;
        }
        x *= 0x1p1023;
        return x;
    }

    absx = sign ? -x : x;

    /* handle the error of x + g - 0.5 */
    y = absx + gmhalf;
    if (absx > gmhalf) {
        dy = y - absx;
        dy -= gmhalf;
    } else {
        dy = y - gmhalf;
        dy -= absx;
    }

    z = absx - 0.5;
    r = tgamma_c__S(absx) * exp(-y);
    if (x < 0) {
        /* reflection formula for negative x */
        /* sinpi(absx) is not 0, integers are already handled */
        r = -tgamma_c__pi / (sinpi(absx) * absx * r);
        dy = -dy;
        z = -z;
    }
    r += dy * (gmhalf+0.5) * r / y;
    z = pow(y, 0.5*z);
    y = r * z * z;
    return y;
}

#if 0
double __lgamma_r(double x, int *sign) {
    double r, absx;

    *sign = 1;

    /* special cases */
    if (!isfinite(x))
        /* lgamma(nan)=nan, lgamma(+-inf)=inf */
        return x*x;

    /* integer arguments */
    if (x == floor(x) && x <= 2) {
        /* n <= 0: lgamma(n)=inf with divbyzero */
        /* n == 1,2: lgamma(n)=0 */
        if (x <= 0)
            return 1/0.0;
        return 0;
    }

    absx = fabs(x);

    /* lgamma(x) ~ -log(|x|) for tgamma_c__tiny |x| */
    if (absx < 0x1p-54) {
        *sign = 1 - 2*!!signbit(x);
        return -log(absx);
    }

    /* use tgamma for smaller |x| */
    if (absx < 128) {
        x = tgamma(x);
        *sign = 1 - 2*!!signbit(x);
        return log(fabs(x));
    }

    /* second term (log(tgamma_c__S)-g) could be more precise here.. */
    /* or with stirling: (|x|-0.5)*(log(|x|)-1) + poly(1/|x|) */
    r = (absx-0.5)*(log(absx+gmhalf)-1) + (log(tgamma_c__S(absx)) - (gmhalf+0.5));
    if (x < 0) {
        /* reflection formula for negative x */
        x = sinpi(absx);
        *sign = 2*!!signbit(x) - 1;
        r = log(tgamma_c__pi/(fabs(x)*absx)) - r;
    }
    return r;
}

weak_alias(__lgamma_r, lgamma_r);
#endif

/// >>> START src/math/tgammaf.c

float tgammaf(float x) {
    return tgamma(x);
}

/// >>> START src/math/tgammal.c
/* origin: OpenBSD /usr/src/lib/libm/src/ld80/e_tgammal.c */
/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 *      Gamma function
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, tgammal();
 *
 * y = tgammal( x );
 *
 *
 * DESCRIPTION:
 *
 * Returns gamma function of the argument.  The result is
 * correctly signed.
 *
 * Arguments |x| <= 13 are reduced by recurrence and the function
 * approximated by a rational function of degree 7/8 in the
 * interval (2,3).  Large arguments are handled by Stirling's
 * formula. Large negative arguments are made positive using
 * a reflection formula.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     -40,+40      10000       3.6e-19     7.9e-20
 *    IEEE    -1755,+1755   10000       4.8e-18     6.5e-19
 *
 * Accuracy for large arguments is dominated by error in powl().
 *
 */

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double tgammal(long double x) {
    return tgamma(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/*
tgamma(x+2) = tgamma(x+2) tgammal_c__P(x)/tgammal_c__Q(x)
0 <= x <= 1
Relative error
n=7, d=8
Peak error =  1.83e-20
Relative error spread =  8.4e-23
*/
static const long double tgammal_c__P[8] = {
 4.212760487471622013093E-5L,
 4.542931960608009155600E-4L,
 4.092666828394035500949E-3L,
 2.385363243461108252554E-2L,
 1.113062816019361559013E-1L,
 3.629515436640239168939E-1L,
 8.378004301573126728826E-1L,
 1.000000000000000000009E0L,
};
static const long double tgammal_c__Q[9] = {
-1.397148517476170440917E-5L,
 2.346584059160635244282E-4L,
-1.237799246653152231188E-3L,
-7.955933682494738320586E-4L,
 2.773706565840072979165E-2L,
-4.633887671244534213831E-2L,
-2.243510905670329164562E-1L,
 4.150160950588455434583E-1L,
 9.999999999999999999908E-1L,
};

/*
static const long double tgammal_c__P[] = {
-3.01525602666895735709e0L,
-3.25157411956062339893e1L,
-2.92929976820724030353e2L,
-1.70730828800510297666e3L,
-7.96667499622741999770e3L,
-2.59780216007146401957e4L,
-5.99650230220855581642e4L,
-7.15743521530849602425e4L
};
static const long double tgammal_c__Q[] = {
 1.00000000000000000000e0L,
-1.67955233807178858919e1L,
 8.85946791747759881659e1L,
 5.69440799097468430177e1L,
-1.98526250512761318471e3L,
 3.31667508019495079814e3L,
 1.60577839621734713377e4L,
-2.97045081369399940529e4L,
-7.15743521530849602412e4L
};
*/
#undef MAXGAML
#define MAXGAML 1755.455L
/*static const long double LOGPI = 1.14472988584940017414L;*/

/* Stirling's formula for the gamma function
tgamma(x) = sqrt(2 tgammal_c__pi) x^(x-.5) exp(-x) (1 + 1/x tgammal_c__P(1/x))
z(x) = x
13 <= x <= 1024
Relative error
n=8, d=0
Peak error =  9.44e-21
Relative error spread =  8.8e-4
*/
static const long double STIR[9] = {
 7.147391378143610789273E-4L,
-2.363848809501759061727E-5L,
-5.950237554056330156018E-4L,
 6.989332260623193171870E-5L,
 7.840334842744753003862E-4L,
-2.294719747873185405699E-4L,
-2.681327161876304418288E-3L,
 3.472222222230075327854E-3L,
 8.333333333333331800504E-2L,
};

#undef MAXSTIR
#define MAXSTIR 1024.0L
static const long double SQTPI = 2.50662827463100050242E0L;

/* 1/tgamma(x) = z tgammal_c__P(z)
 * z(x) = 1/x
 * 0 < x < 0.03125
 * Peak relative error 4.2e-23
 */
static const long double tgammal_c__S[9] = {
-1.193945051381510095614E-3L,
 7.220599478036909672331E-3L,
-9.622023360406271645744E-3L,
-4.219773360705915470089E-2L,
 1.665386113720805206758E-1L,
-4.200263503403344054473E-2L,
-6.558780715202540684668E-1L,
 5.772156649015328608253E-1L,
 1.000000000000000000000E0L,
};

/* 1/tgamma(-x) = z tgammal_c__P(z)
 * z(x) = 1/x
 * 0 < x < 0.03125
 * Peak relative error 5.16e-23
 * Relative error spread =  2.5e-24
 */
static const long double SN[9] = {
 1.133374167243894382010E-3L,
 7.220837261893170325704E-3L,
 9.621911155035976733706E-3L,
-4.219773343731191721664E-2L,
-1.665386113944413519335E-1L,
-4.200263503402112910504E-2L,
 6.558780715202536547116E-1L,
 5.772156649015328608727E-1L,
-1.000000000000000000000E0L,
};

static const long double tgammal_c__PIL = 3.1415926535897932384626L;

/* Gamma function computed by Stirling's formula.
 */
static long double stirf(long double x) {
    long double y, w, v;

    w = 1.0/x;
    /* For large x, use rational coefficients from the analytical expansion.  */
    if (x > 1024.0)
        w = (((((6.97281375836585777429E-5L * w
         + 7.84039221720066627474E-4L) * w
         - 2.29472093621399176955E-4L) * w
         - 2.68132716049382716049E-3L) * w
         + 3.47222222222222222222E-3L) * w
         + 8.33333333333333333333E-2L) * w
         + 1.0;
    else
        w = 1.0 + w * __polevll(w, STIR, 8);
    y = expl(x);
    if (x > MAXSTIR) { /* Avoid overflow in pow() */
        v = powl(x, 0.5L * x - 0.25L);
        y = v * (v / y);
    } else {
        y = powl(x, x - 0.5L) / y;
    }
    y = SQTPI * y * w;
    return y;
}

long double tgammal(long double x) {
    long double p, q, z;

    if (!isfinite(x))
        return x + INFINITY;

    q = fabsl(x);
    if (q > 13.0) {
        if (x < 0.0) {
            p = floorl(q);
            z = q - p;
            if (z == 0)
                return 0 / z;
            if (q > MAXGAML) {
                z = 0;
            } else {
                if (z > 0.5) {
                    p += 1.0;
                    z = q - p;
                }
                z = q * sinl(tgammal_c__PIL * z);
                z = fabsl(z) * stirf(q);
                z = tgammal_c__PIL/z;
            }
            if (0.5 * p == floorl(q * 0.5))
                z = -z;
        } else if (x > MAXGAML) {
            z = x * 0x1p16383L;
        } else {
            z = stirf(x);
        }
        return z;
    }

    z = 1.0;
    while (x >= 3.0) {
        x -= 1.0;
        z *= x;
    }
    while (x < -0.03125L) {
        z /= x;
        x += 1.0;
    }
    if (x <= 0.03125L)
        goto small;
    while (x < 2.0) {
        z /= x;
        x += 1.0;
    }
    if (x == 2.0)
        return z;

    x -= 2.0;
    p = __polevll(x, tgammal_c__P, 7);
    q = __polevll(x, tgammal_c__Q, 8);
    z = z * p / q;
    return z;

small:
    /* z==1 if x was originally +-0 */
    if (x == 0 && z != 1)
        return x / x;
    if (x < 0.0) {
        x = -x;
        q = z / (x * __polevll(x, SN, 8));
    } else
        q = z / (x * __polevll(x, tgammal_c__S, 8));
    return q;
}
#endif

/// >>> START src/math/trunc.c

double trunc(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = (int)(u.i >> 52 & 0x7ff) - 0x3ff + 12;
    uint64_t m;

    if (e >= 52 + 12)
        return x;
    if (e < 12)
        e = 1;
    m = -1ULL >> e;
    if ((u.i & m) == 0)
        return x;
    FORCE_EVAL(x + 0x1p120f);
    u.i &= ~m;
    return u.f;
}

/// >>> START src/math/truncf.c

float truncf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = (int)(u.i >> 23 & 0xff) - 0x7f + 9;
    uint32_t m;

    if (e >= 23 + 9)
        return x;
    if (e < 9)
        e = 1;
    m = -1U >> e;
    if ((u.i & m) == 0)
        return x;
    FORCE_EVAL(x + 0x1p120f);
    u.i &= ~m;
    return u.f;
}

/// >>> START src/math/truncl.c

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double truncl(long double x) {
    return trunc(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#undef TOINT
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#undef TOINT
#define TOINT 0x1p112
#endif
long double truncl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    int s = u.i.se >> 15;
    long double y;

    if (e >= 0x3fff+LDBL_MANT_DIG-1)
        return x;
    if (e <= 0x3fff-1) {
        FORCE_EVAL(x + 0x1p120f);
        return x*0;
    }
    /* y = int(|x|) - |x|, where int(|x|) is an integer neighbor of |x| */
    if (s)
        x = -x;
    y = x + TOINT - TOINT - x;
    if (y > 0)
        y -= 1;
    x += y;
    return s ? -x : x;
}
#endif

/// >>> START src/misc/a64l.c

static const char digits[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

long a64l(const char *s) {
    int e;
    uint32_t x = 0;
    for (e=0; e<36 && *s; e+=6, s++)
        x |= (strchr(digits, *s)-digits)<<e;
    return x;
}

char *l64a(long x0) {
    static char s[7];
    char *p;
    uint32_t x = x0;
    for (p=s; x; p++, x>>=6)
        *p = digits[x&63];
    *p = 0;
    return s;
}

/// >>> START src/misc/basename.c

char *basename(char *s) {
    size_t i;
    if (!s || !*s) return ".";
    i = strlen(s)-1;
    for (; i&&s[i]=='/'; i--) s[i] = 0;
    for (; i&&s[i-1]!='/'; i--);
    return s+i;
}

weak_alias(basename, __xpg_basename);

/// >>> START src/misc/dirname.c

char *dirname(char *s) {
    size_t i;
    if (!s || !*s) return ".";
    i = strlen(s)-1;
    for (; s[i]=='/'; i--) if (!i) return "/";
    for (; s[i]!='/'; i--) if (!i) return ".";
    for (; s[i]=='/'; i--) if (!i) return "/";
    s[i+1] = 0;
    return s;
}

/// >>> START src/misc/ffs.c

int ffs(int i) {
    return i ? a_ctz_l(i)+1 : 0;
}

/// >>> START src/misc/get_current_dir_name.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *get_current_dir_name(void) {
    struct stat a, b;
    char *res = getenv("PWD");
    if (res && *res && !stat(res, &a) && !stat(".", &b)
        && (a.st_dev == b.st_dev) && (a.st_ino == b.st_ino))
        return strdup(res);
    return getcwd(0, 0);
}

/// >>> START src/misc/getdomainname.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int getdomainname(char *name, size_t len) {
    struct utsname temp;
    uname(&temp);
    if (!len || strlen(temp.domainname) >= len) {
        errno = EINVAL;
        return -1;
    }
    strcpy(name, temp.domainname);
    return 0;
}

/// >>> START src/misc/getgrouplist.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int getgrouplist(const char *user, gid_t gid, gid_t *groups, int *ngroups) {
    size_t n, i;
    struct group *gr;
    if (*ngroups<1) return -1;
    n = *ngroups;
    *groups++ = gid;
    *ngroups = 1;

    setgrent();
    while ((gr = getgrent()) && *ngroups < INT_MAX) {
        for (i=0; gr->gr_mem[i] && strcmp(user, gr->gr_mem[i]); i++);
        if (!gr->gr_mem[i]) continue;
        if (++*ngroups <= n) *groups++ = gr->gr_gid;
    }
    endgrent();

    return *ngroups > n ? -1 : *ngroups;
}

/// >>> START src/misc/gethostid.c
long gethostid() {
    return 0;
}

/// >>> START src/misc/getopt.c

char *optarg;
int optind=1, opterr=1, optopt, __optpos, __optreset=0;

#undef optpos
#define optpos __optpos
weak_alias(__optreset, optreset);

int getopt(int argc, char * const argv[], const char *optstring) {
    int i;
    wchar_t c, d;
    int getopt_c__k, l;
    char *optchar;

    if (!optind || __optreset) {
        __optreset = 0;
        __optpos = 0;
        optind = 1;
    }

    if (optind >= argc || !argv[optind] || argv[optind][0] != '-' || !argv[optind][1])
        return -1;
    if (argv[optind][1] == '-' && !argv[optind][2])
        return optind++, -1;

    if (!optpos) optpos++;
    if ((getopt_c__k = mbtowc(&c, argv[optind]+optpos, MB_LEN_MAX)) < 0) {
        getopt_c__k = 1;
        c = 0xfffd; /* replacement char */
    }
    optchar = argv[optind]+optpos;
    optopt = c;
    optpos += getopt_c__k;

    if (!argv[optind][optpos]) {
        optind++;
        optpos = 0;
    }

    for (i=0; (l = mbtowc(&d, optstring+i, MB_LEN_MAX)) && d!=c; i+=l>0?l:1);

    if (d != c) {
        if (optstring[0] != ':' && opterr) {
            write(2, argv[0], strlen(argv[0]));
            write(2, ": illegal option: ", 18);
            write(2, optchar, getopt_c__k);
            write(2, "\n", 1);
        }
        return '?';
    }
    if (optstring[i+1] == ':') {
        if (optind >= argc) {
            if (optstring[0] == ':') return ':';
            if (opterr) {
                write(2, argv[0], strlen(argv[0]));
                write(2, ": option requires an argument: ", 31);
                write(2, optchar, getopt_c__k);
                write(2, "\n", 1);
            }
            return '?';
        }
        optarg = argv[optind++] + optpos;
        optpos = 0;
    }
    return c;
}

weak_alias(getopt, __posix_getopt);

/// >>> START src/misc/getopt_long.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

extern int __optpos, __optreset;

static int __getopt_long(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx, int longonly) {
    if (!optind || __optreset) {
        __optreset = 0;
        __optpos = 0;
        optind = 1;
    }
    if (optind >= argc || !argv[optind] || argv[optind][0] != '-') return -1;
    if ((longonly && argv[optind][1]) ||
        (argv[optind][1] == '-' && argv[optind][2])) {
        int i;
        for (i=0; longopts[i].name; i++) {
            const char *name = longopts[i].name;
            char *opt = argv[optind]+1;
            if (*opt == '-') opt++;
            for (; *name && *name == *opt; name++, opt++);
            if (*name || (*opt && *opt != '=')) continue;
            if (*opt == '=') {
                if (!longopts[i].has_arg) continue;
                optarg = opt+1;
            } else {
                if (longopts[i].has_arg == required_argument) {
                    if (!(optarg = argv[++optind]))
                        return ':';
                } else optarg = NULL;
            }
            optind++;
            if (idx) *idx = i;
            if (longopts[i].flag) {
                *longopts[i].flag = longopts[i].val;
                return 0;
            }
            return longopts[i].val;
        }
        if (argv[optind][1] == '-') {
            optind++;
            return '?';
        }
    }
    return getopt(argc, argv, optstring);
}

int getopt_long(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return __getopt_long(argc, argv, optstring, longopts, idx, 0);
}

int getopt_long_only(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return __getopt_long(argc, argv, optstring, longopts, idx, 1);
}

/// >>> START src/misc/getsubopt.c

int getsubopt(char **opt, char *const *keys, char **val) {
    char *s = *opt;
    int i;

    *val = NULL;
    *opt = strchr(s, ',');
    if (*opt) *(*opt)++ = 0;
    else *opt = s + strlen(s);

    for (i=0; keys[i]; i++) {
        size_t l = strlen(keys[i]);
        if (strncmp(keys[i], s, l)) continue;
        if (s[l] == '=')
            *val = s + l;
        else if (s[l]) continue;
        return i;
    }
    return -1;
}

/// >>> START src/misc/initgroups.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int initgroups(const char *user, gid_t gid) {
    gid_t groups[NGROUPS_MAX];
    int count = NGROUPS_MAX;
    if (getgrouplist(user, gid, groups, &count) < 0) return -1;
    return setgroups(count, groups);
}

/// >>> START src/misc/ioctl.c

int ioctl(int fd, int req, ...) {
    void *arg;
    va_list ap;
    va_start(ap, req);
    arg = va_arg(ap, void *);
    va_end(ap);
    return syscall(SYS_ioctl, fd, req, arg);
}

/// >>> START src/misc/lockf.c

int lockf(int fd, int op, off_t size) {
    struct flock l = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_CUR,
        .l_len = size,
    };
    switch (op) {
    case F_TEST:
        l.l_type = F_RDLCK;
        if (fcntl(fd, F_GETLK, &l) < 0)
            return -1;
        if (l.l_type == F_UNLCK || l.l_pid == getpid())
            return 0;
        errno = EACCES;
        return -1;
    case F_ULOCK:
        l.l_type = F_UNLCK;
    case F_TLOCK:
        return fcntl(fd, F_SETLK, &l);
    case F_LOCK:
        return fcntl(fd, F_SETLKW, &l);
    }
    errno = EINVAL;
    return -1;
}

/// >>> START src/misc/nftw.c

struct history {
    struct history *chain;
    dev_t dev;
    ino_t ino;
    int level;
    int base;
};

#undef dirfd
#undef dirfd
#define dirfd(d) (*(int *)d)

static int do_nftw(char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags, struct history *h) {
    size_t l = strlen(path), j = l && path[l-1]=='/' ? l-1 : l;
    struct stat nftw_c__st;
    struct history new;
    int type;
    int r;
    struct FTW lev;
    char *name;

    if ((flags & FTW_PHYS) ? lstat(path, &nftw_c__st) : stat(path, &nftw_c__st) < 0) {
        if (!(flags & FTW_PHYS) && errno==ENOENT && !lstat(path, &nftw_c__st))
            type = FTW_SLN;
        else if (errno != EACCES) return -1;
        else type = FTW_NS;
    } else if (S_ISDIR(nftw_c__st.st_mode)) {
        if (access(path, R_OK) < 0) type = FTW_DNR;
        else if (flags & FTW_DEPTH) type = FTW_DP;
        else type = FTW_D;
    } else if (S_ISLNK(nftw_c__st.st_mode)) {
        if (flags & FTW_PHYS) type = FTW_SL;
        else type = FTW_SLN;
    } else {
        type = FTW_F;
    }

    if ((flags & FTW_MOUNT) && h
     && (nftw_c__st.st_dev != h->dev || nftw_c__st.st_ino != h->ino))
        return 0;
    
    new.chain = h;
    new.dev = nftw_c__st.st_dev;
    new.ino = nftw_c__st.st_ino;
    new.level = h ? h->level+1 : 0;
    new.base = l+1;
    
    lev.level = new.level;
    lev.base = h ? h->base : (name=strrchr(path, '/')) ? name-path : 0;

    if (!(flags & FTW_DEPTH) && (r=fn(path, &nftw_c__st, type, &lev)))
        return r;

    for (; h; h = h->chain)
        if (h->dev == nftw_c__st.st_dev && h->ino == nftw_c__st.st_ino)
            return 0;

    if ((type == FTW_D || type == FTW_DP) && fd_limit) {
        DIR *d = opendir(path);
        if (d) {
            struct dirent *de;
            while ((de = readdir(d))) {
                if (de->d_name[0] == '.'
                 && (!de->d_name[1]
                  || (de->d_name[1]=='.'
                   && !de->d_name[2]))) continue;
                if (strlen(de->d_name) >= PATH_MAX-l) {
                    errno = ENAMETOOLONG;
                    closedir(d);
                    return -1;
                }
                path[j]='/';
                strcpy(path+j+1, de->d_name);
                if ((r=do_nftw(path, fn, fd_limit-1, flags, &new))) {
                    closedir(d);
                    return r;
                }
            }
            closedir(d);
        } else if (errno != EACCES) {
            return -1;
        }
    }

    path[l] = 0;
    if ((flags & FTW_DEPTH) && (r=fn(path, &nftw_c__st, type, &lev)))
        return r;

    return 0;
}

int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags) {
    int r, cs;
    size_t l;
    char pathbuf[PATH_MAX+1];

    if (fd_limit <= 0) return 0;

    l = strlen(path);
    if (l > PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(pathbuf, path, l+1);
    
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
    r = do_nftw(pathbuf, fn, fd_limit, flags, NULL);
    pthread_setcancelstate(cs, 0);
    return r;
}

/// >>> START src/misc/openpty.c

/* Nonstandard, but vastly superior to the standard functions */

int openpty(int *m, int *s, char *name, const struct termios *tio, const struct winsize *ws) {
    int n=0;
    char buf[20];

    *m = open("/dev/ptmx", O_RDWR|O_NOCTTY);
    if (*m < 0) return -1;

    if (ioctl(*m, TIOCSPTLCK, &n) || ioctl (*m, TIOCGPTN, &n)) {
        close(*m);
        return -1;
    }

    if (!name) name = buf;
    snprintf(name, sizeof buf, "/dev/pts/%d", n);
    if ((*s = open(name, O_RDWR|O_NOCTTY)) < 0) {
        close(*m);
        return -1;
    }

    if (tio) tcsetattr(*s, TCSANOW, tio);
    if (ws) ioctl(*s, TIOCSWINSZ, ws);

    return 0;
}

/// >>> START src/misc/ptsname.c

int __ptsname_r(int, char *, size_t);

char *ptsname(int fd) {
    static char buf[9 + sizeof(int)*3 + 1];
    int err = __ptsname_r(fd, buf, sizeof buf);
    if (err) {
        errno = err;
        return 0;
    }
    return buf;
}

/// >>> START src/misc/pty.c

int posix_openpt(int flags) {
    return open("/dev/ptmx", flags);
}

int grantpt(int fd) {
    return 0;
}

int unlockpt(int fd) {
    int unlock = 0;
    return ioctl(fd, TIOCSPTLCK, &unlock);
}

int __ptsname_r(int fd, char *buf, size_t len) {
    int pty, err;
    if (!buf) len = 0;
    if ((err = __syscall(SYS_ioctl, fd, TIOCGPTN, &pty))) return err;
    if (snprintf(buf, len, "/dev/pts/%d", pty) >= len) return ERANGE;
    return 0;
}

weak_alias(__ptsname_r, ptsname_r);

/// >>> START src/misc/realpath.c

void __procfdname(char *, unsigned);

char *realpath(const char *restrict filename, char *restrict resolved) {
    int fd;
    ssize_t r;
    struct stat realpath_c__st1, realpath_c__st2;
    char buf[15+3*sizeof(int)];
    char tmp[PATH_MAX];

    if (!filename) {
        errno = EINVAL;
        return 0;
    }

    fd = syscall(SYS_open, filename, O_PATH|O_NONBLOCK|O_CLOEXEC|O_LARGEFILE|O_CLOEXEC);
    if (fd < 0) return 0;
    __procfdname(buf, fd);

    r = readlink(buf, tmp, sizeof tmp - 1);
    if (r < 0) goto err;
    tmp[r] = 0;

    fstat(fd, &realpath_c__st1);
    r = stat(tmp, &realpath_c__st2);
    if (r<0 || realpath_c__st1.st_dev != realpath_c__st2.st_dev || realpath_c__st1.st_ino != realpath_c__st2.st_ino) {
        if (!r) errno = ELOOP;
        goto err;
    }

    __syscall(SYS_close, fd);
    return resolved ? strcpy(resolved, tmp) : strdup(tmp);
err:
    __syscall(SYS_close, fd);
    return 0;
}

/// >>> START src/multibyte/btowc.c

wint_t btowc(int c) {
    return c<128U ? c : EOF;
}

/// >>> START src/multibyte/internal.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

/// >>> START src/multibyte/internal.h
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

#undef bittab
#define bittab __fsmu8

extern const uint32_t bittab[] ATTR_LIBC_VISIBILITY;

/* Upper 6 state bits are a negative integer offset to bound-check next byte */
/*    equivalent to: ( (b-0x80) | (b+offset) ) & ~0x3f      */
#undef OOB
#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)

/* Interval [a,b). Either a must be 80 or b must be c0, lower 3 bits clear. */
#undef mb_internal__R
#define mb_internal__R(a,b) ((uint32_t)((a==0x80 ? 0x40-b : -a) << 23))
#undef FAILSTATE
#define FAILSTATE mb_internal__R(0x80,0x80)

#undef SA
#define SA 0xc2u
#undef SB
#define SB 0xf4u

/// >>> CONTINUE src/multibyte/internal.c

#undef C
#define C(x) ( x<2 ? -1 : ( mb_internal__R(0x80,0xc0) | x ) )
#undef D
#define D(x) C((x+16))
#undef E
#define E(x) ( ( x==0 ? mb_internal__R(0xa0,0xc0) : \
                 x==0xd ? mb_internal__R(0x80,0xa0) : \
                 mb_internal__R(0x80,0xc0) ) \
             | ( mb_internal__R(0x80,0xc0) >> 6 ) \
             | x )
#undef F
#define F(x) ( ( x>=5 ? 0 : \
                 x==0 ? mb_internal__R(0x90,0xc0) : \
                 x==4 ? mb_internal__R(0x80,0xa0) : \
                 mb_internal__R(0x80,0xc0) ) \
             | ( mb_internal__R(0x80,0xc0) >> 6 ) \
             | ( mb_internal__R(0x80,0xc0) >> 12 ) \
             | x )

const uint32_t bittab[] = {
                  C(0x2),C(0x3),C(0x4),C(0x5),C(0x6),C(0x7),
    C(0x8),C(0x9),C(0xa),C(0xb),C(0xc),C(0xd),C(0xe),C(0xf),
    D(0x0),D(0x1),D(0x2),D(0x3),D(0x4),D(0x5),D(0x6),D(0x7),
    D(0x8),D(0x9),D(0xa),D(0xb),D(0xc),D(0xd),D(0xe),D(0xf),
    E(0x0),E(0x1),E(0x2),E(0x3),E(0x4),E(0x5),E(0x6),E(0x7),
    E(0x8),E(0x9),E(0xa),E(0xb),E(0xc),E(0xd),E(0xe),E(0xf),
    F(0x0),F(0x1),F(0x2),F(0x3),F(0x4)
};

#ifdef BROKEN_VISIBILITY
__asm__(".hidden __fsmu8");
#endif

/// >>> START src/multibyte/mblen.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

int mblen(const char *s, size_t n) {
    return mbtowc(0, s, n);
}

/// >>> START src/multibyte/mbrlen.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t mbrlen(const char *restrict s, size_t n, mbstate_t *restrict mbrlen_c__st) {
    static unsigned internal;
    return mbrtowc(0, s, n, mbrlen_c__st ? mbrlen_c__st : (mbstate_t *)&internal);
}

/// >>> START src/multibyte/mbrtowc.c
#undef N
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t mbrtowc(wchar_t *restrict mbrtowc_c__wc, const char *restrict src, size_t n, mbstate_t *restrict mbrtowc_c__st) {
    static unsigned internal_state;
    unsigned c;
    const unsigned char *s = (const void *)src;
    const unsigned N = n;

    if (!mbrtowc_c__st) mbrtowc_c__st = (void *)&internal_state;
    c = *(unsigned *)mbrtowc_c__st;
    
    if (!s) {
        if (c) goto ilseq;
        return 0;
    } else if (!mbrtowc_c__wc) mbrtowc_c__wc = (void *)&mbrtowc_c__wc;

    if (!n) return -2;
    if (!c) {
        if (*s < 0x80) return !!(*mbrtowc_c__wc = *s);
        if (*s-SA > SB-SA) goto ilseq;
        c = bittab[*s++-SA]; n--;
    }

    if (n) {
        if (OOB(c,*s)) goto ilseq;
loop:
        c = c<<6 | *s++-0x80; n--;
        if (!(c&(1U<<31))) {
            *(unsigned *)mbrtowc_c__st = 0;
            *mbrtowc_c__wc = c;
            return N-n;
        }
        if (n) {
            if (*s-0x80u >= 0x40) goto ilseq;
            goto loop;
        }
    }

    *(unsigned *)mbrtowc_c__st = c;
    return -2;
ilseq:
    *(unsigned *)mbrtowc_c__st = 0;
    errno = EILSEQ;
    return -1;
}

/// >>> START src/multibyte/mbsinit.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

int mbsinit(const mbstate_t *mbsinit_c__st) {
    return !mbsinit_c__st || !*(unsigned *)mbsinit_c__st;
}

/// >>> START src/multibyte/mbsnrtowcs.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t mbsnrtowcs(wchar_t *restrict wcs, const char **restrict src, size_t n, size_t wn, mbstate_t *restrict mbsnrtowcs_c__st) {
    size_t l, cnt=0, n2;
    wchar_t *ws, wbuf[256];
    const char *s = *src;

    if (!wcs) ws = wbuf, wn = sizeof wbuf / sizeof *wbuf;
    else ws = wcs;

    /* making sure output buffer size is at most n/4 will ensure
     * that mbsrtowcs never reads more than n input bytes. thus
     * we can use mbsrtowcs as long as it's practical.. */

    while ( s && wn && ( (n2=n/4)>=wn || n2>32 ) ) {
        if (n2>=wn) n2=wn;
        n -= n2;
        l = mbsrtowcs(ws, &s, n2, mbsnrtowcs_c__st);
        if (!(l+1)) {
            cnt = l;
            wn = 0;
            break;
        }
        if (ws != wbuf) {
            ws += l;
            wn -= l;
        }
        cnt += l;
    }
    if (s) while (wn && n) {
        l = mbrtowc(ws, s, n, mbsnrtowcs_c__st);
        if (l+2<=2) {
            if (!(l+1)) {
                cnt = l;
                break;
            }
            if (!l) {
                s = 0;
                break;
            }
            /* have to roll back partial character */
            *(unsigned *)mbsnrtowcs_c__st = 0;
            break;
        }
        s += l; n -= l;
        /* safe - this loop runs fewer than sizeof(wbuf)/8 times */
        ws++; wn--;
        cnt++;
    }
    if (wcs) *src = s;
    return cnt;
}

/// >>> START src/multibyte/mbsrtowcs.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t mbsrtowcs(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict mbsrtowcs_c__st) {
    const unsigned char *s = (const void *)*src;
    size_t wn0 = wn;
    unsigned c = 0;

    if (mbsrtowcs_c__st && (c = *(unsigned *)mbsrtowcs_c__st)) {
        if (ws) {
            *(unsigned *)mbsrtowcs_c__st = 0;
            goto resume;
        } else {
            goto resume0;
        }
    }

    if (!ws) for (;;) {
        if (*s-1u < 0x7f && (uintptr_t)s%4 == 0) {
            while (!(( *(uint32_t*)s | *(uint32_t*)s-0x01010101) & 0x80808080)) {
                s += 4;
                wn -= 4;
            }
        }
        if (*s-1u < 0x7f) {
            s++;
            wn--;
            continue;
        }
        if (*s-SA > SB-SA) break;
        c = bittab[*s++-SA];
resume0:
        if (OOB(c,*s)) { s--; break; }
        s++;
        if (c&(1U<<25)) {
            if (*s-0x80u >= 0x40) { s-=2; break; }
            s++;
            if (c&(1U<<19)) {
                if (*s-0x80u >= 0x40) { s-=3; break; }
                s++;
            }
        }
        wn--;
        c = 0;
    } else for (;;) {
        if (!wn) {
            *src = (const void *)s;
            return wn0;
        }
        if (*s-1u < 0x7f && (uintptr_t)s%4 == 0) {
            while (wn>=5 && !(( *(uint32_t*)s | *(uint32_t*)s-0x01010101) & 0x80808080)) {
                *ws++ = *s++;
                *ws++ = *s++;
                *ws++ = *s++;
                *ws++ = *s++;
                wn -= 4;
            }
        }
        if (*s-1u < 0x7f) {
            *ws++ = *s++;
            wn--;
            continue;
        }
        if (*s-SA > SB-SA) break;
        c = bittab[*s++-SA];
resume:
        if (OOB(c,*s)) { s--; break; }
        c = (c<<6) | *s++-0x80;
        if (c&(1U<<31)) {
            if (*s-0x80u >= 0x40) { s-=2; break; }
            c = (c<<6) | *s++-0x80;
            if (c&(1U<<31)) {
                if (*s-0x80u >= 0x40) { s-=3; break; }
                c = (c<<6) | *s++-0x80;
            }
        }
        *ws++ = c;
        wn--;
        c = 0;
    }

    if (!c && !*s) {
        if (ws) {
            *ws = 0;
            *src = 0;
        }
        return wn0-wn;
    }
    errno = EILSEQ;
    if (ws) *src = (const void *)s;
    return -1;
}

/// >>> START src/multibyte/mbstowcs.c

size_t mbstowcs(wchar_t *restrict ws, const char *restrict s, size_t wn) {
    return mbsrtowcs(ws, (void*)&s, wn, 0);
}

/// >>> START src/multibyte/mbtowc.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

int mbtowc(wchar_t *restrict mbtowc_c__wc, const char *restrict src, size_t n) {
    unsigned c;
    const unsigned char *s = (const void *)src;

    if (!s) return 0;
    if (!n) goto ilseq;
    if (!mbtowc_c__wc) mbtowc_c__wc = (void *)&mbtowc_c__wc;

    if (*s < 0x80) return !!(*mbtowc_c__wc = *s);
    if (*s-SA > SB-SA) goto ilseq;
    c = bittab[*s++-SA];

    /* Avoid excessive checks against n: If shifting the state n-1
     * times does not clear the high bit, then the value of n is
     * insufficient to read a character */
    if (n<4 && ((c<<(6*n-6)) & (1U<<31))) goto ilseq;

    if (OOB(c,*s)) goto ilseq;
    c = c<<6 | *s++-0x80;
    if (!(c&(1U<<31))) {
        *mbtowc_c__wc = c;
        return 2;
    }

    if (*s-0x80u >= 0x40) goto ilseq;
    c = c<<6 | *s++-0x80;
    if (!(c&(1U<<31))) {
        *mbtowc_c__wc = c;
        return 3;
    }

    if (*s-0x80u >= 0x40) goto ilseq;
    *mbtowc_c__wc = c<<6 | *s++-0x80;
    return 4;

ilseq:
    errno = EILSEQ;
    return -1;
}

/// >>> START src/multibyte/wcrtomb.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t wcrtomb(char *restrict s, wchar_t wcrtomb_c__wc, mbstate_t *restrict wcrtomb_c__st) {
    if (!s) return 1;
    if ((unsigned)wcrtomb_c__wc < 0x80) {
        *s = wcrtomb_c__wc;
        return 1;
    } else if ((unsigned)wcrtomb_c__wc < 0x800) {
        *s++ = 0xc0 | (wcrtomb_c__wc>>6);
        *s = 0x80 | (wcrtomb_c__wc&0x3f);
        return 2;
    } else if ((unsigned)wcrtomb_c__wc < 0xd800 || (unsigned)wcrtomb_c__wc-0xe000 < 0x2000) {
        *s++ = 0xe0 | (wcrtomb_c__wc>>12);
        *s++ = 0x80 | ((wcrtomb_c__wc>>6)&0x3f);
        *s = 0x80 | (wcrtomb_c__wc&0x3f);
        return 3;
    } else if ((unsigned)wcrtomb_c__wc-0x10000 < 0x100000) {
        *s++ = 0xf0 | (wcrtomb_c__wc>>18);
        *s++ = 0x80 | ((wcrtomb_c__wc>>12)&0x3f);
        *s++ = 0x80 | ((wcrtomb_c__wc>>6)&0x3f);
        *s = 0x80 | (wcrtomb_c__wc&0x3f);
        return 4;
    }
    errno = EILSEQ;
    return -1;
}

/// >>> START src/multibyte/wcsnrtombs.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t wcsnrtombs(char *restrict dst, const wchar_t **restrict wcs, size_t wn, size_t n, mbstate_t *restrict wcsnrtombs_c__st) {
    size_t l, cnt=0, n2;
    char *s, buf[256];
    const wchar_t *ws = *wcs;

    if (!dst) s = buf, n = sizeof buf;
    else s = dst;

    while ( ws && n && ( (n2=wn)>=n || n2>32 ) ) {
        if (n2>=n) n2=n;
        wn -= n2;
        l = wcsrtombs(s, &ws, n2, 0);
        if (!(l+1)) {
            cnt = l;
            n = 0;
            break;
        }
        if (s != buf) {
            s += l;
            n -= l;
        }
        cnt += l;
    }
    if (ws) while (n && wn) {
        l = wcrtomb(s, *ws, 0);
        if ((l+1)<=1) {
            if (!l) ws = 0;
            else cnt = l;
            break;
        }
        ws++; wn--;
        /* safe - this loop runs fewer than sizeof(buf) times */
        s+=l; n-=l;
        cnt++;
    }
    if (dst) *wcs = ws;
    return cnt;
}

/// >>> START src/multibyte/wcsrtombs.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

size_t wcsrtombs(char *restrict s, const wchar_t **restrict ws, size_t n, mbstate_t *restrict wcsrtombs_c__st) {
    const wchar_t *ws2;
    char buf[4];
    size_t N = n, l;
    if (!s) {
        for (n=0, ws2=*ws; *ws2; ws2++) {
            if (*ws2 >= 0x80u) {
                l = wcrtomb(buf, *ws2, 0);
                if (!(l+1)) return -1;
                n += l;
            } else n++;
        }
        return n;
    }
    while (n>=4 && **ws) {
        if (**ws >= 0x80u) {
            l = wcrtomb(s, **ws, 0);
            if (!(l+1)) return -1;
            s += l;
            n -= l;
        } else {
            *s++ = **ws;
            n--;
        }
        (*ws)++;
    }
    while (n && **ws) {
        if (**ws >= 0x80u) {
            l = wcrtomb(buf, **ws, 0);
            if (!(l+1)) return -1;
            if (l>n) return N-n;
            wcrtomb(s, **ws, 0);
            s += l;
            n -= l;
        } else {
            *s++ = **ws;
            n--;
        }
        (*ws)++;
    }
    if (n) *s = 0;
    *ws = 0;
    return N-n;
}

/// >>> START src/multibyte/wcstombs.c

size_t wcstombs(char *restrict s, const wchar_t *restrict ws, size_t n) {
    return wcsrtombs(s, &(const wchar_t *){ws}, n, 0);
}

/// >>> START src/multibyte/wctob.c

int wctob(wint_t c) {
    if (c < 128U) return c;
    return EOF;
}

/// >>> START src/multibyte/wctomb.c
/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

int wctomb(char *s, wchar_t wctomb_c__wc) {
    if (!s) return 0;
    return wcrtomb(s, wctomb_c__wc, 0);
}

/// >>> START src/network/__ipparse.c
/// >>> START src/network/__dns.h

#undef RR_A
#define RR_A 1
#undef RR_CNAME
#define RR_CNAME 5
#undef RR_PTR
#define RR_PTR 12
#undef RR_AAAA
#define RR_AAAA 28

int __dns_count_addrs(const unsigned char *, int);
int __dns_get_rr(void *, size_t, size_t, size_t, const unsigned char *, int, int);

int __dns_query(unsigned char *, const void *, int, int);
int __ipparse(void *, int, const char *);

int __dns_doqueries(unsigned char *, const char *, int *, int);

/// >>> CONTINUE src/network/__ipparse.c

int __ipparse(void *dest, int family, const char *__ipparse_c__s0) {
    const char *s = __ipparse_c__s0;
    unsigned char *d = dest;
    unsigned long a[16] = { 0 };
    char *z;
    int i;

    if (family == AF_INET6) goto not_v4;

    for (i=0; i<4; i++) {
        a[i] = strtoul(s, &z, 0);
        if (z==s || (*z && *z != '.') || !isdigit(*s)) {
            if (family == AF_INET) return -1;
            goto not_v4;
        }
        if (!*z) break;
        s=z+1;
    }
    if (i==4) return -1;
    switch (i) {
    case 0:
        a[1] = a[0] & 0xffffff;
        a[0] >>= 24;
    case 1:
        a[2] = a[1] & 0xffff;
        a[1] >>= 16;
    case 2:
        a[3] = a[2] & 0xff;
        a[2] >>= 8;
    }
    ((struct sockaddr_in *)d)->sin_family = AF_INET;
    d = (void *)&((struct sockaddr_in *)d)->sin_addr;
    for (i=0; i<4; i++) {
        if (a[i] > 255) return -1;
        d[i] = a[i];
    }
    return 0;

not_v4:
    s = __ipparse_c__s0;
    ((struct sockaddr_in6 *)d)->sin6_family = AF_INET6;
    return inet_pton(AF_INET6, s, (void *)&((struct sockaddr_in6 *)d)->sin6_addr) <= 0 ? -1 : 0;
}

/// >>> START src/network/accept.c

int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len) {
    return socketcall_cp(accept, fd, addr, len, 0, 0, 0);
}

/// >>> START src/network/dn_comp.c

int __dn_comp(const char *src, unsigned char *dst, int space, unsigned char **dnptrs, unsigned char **lastdnptr) {
    return -1;
}

weak_alias(__dn_comp, dn_comp);

/// >>> START src/network/dn_expand.c

int __dn_expand(const unsigned char *base, const unsigned char *end, const unsigned char *src, char *dest, int space) {
    const unsigned char *p = src;
    int len = -1, j;
    if (space > 256) space = 256;
    if (p==end || !*p) return -1;
    for (;;) {
        if (*p & 0xc0) {
            if (p+1==end) return -1;
            j = ((p[0] & 0x3f) << 8) | p[1];
            if (len < 0) len = p+2-src;
            if (j >= end-base) return -1;
            p = base+j;
        } else if (*p) {
            j = *p+1;
            if (j>=end-p || j>space) return -1;
            while (--j) *dest++ = *++p;
            *dest++ = *++p ? '.' : 0;
        } else {
            if (len < 0) len = p+1-src;
            return len;
        }
    }
}

weak_alias(__dn_expand, dn_expand);

/// >>> START src/network/dn_skipname.c

int dn_skipname(const unsigned char *s, const unsigned char *end) {
    const unsigned char *p;
    for (p=s; p<end; p++)
        if (!*p) return p-s+1;
        else if (*p>=192)
            if (p+1<end) return p-s+2;
            else break;
    return -1;
}

/// >>> START src/network/ether.c

struct ether_addr *ether_aton_r (const char *x, struct ether_addr *p_a) {
    struct ether_addr a;
    char *y;
    for (int ii = 0; ii < 6; ii++) {
        unsigned long int n;
        if (ii != 0) {
            if (x[0] != ':') return 0; /* bad format */
            else x++;
        }
        n = strtoul (x, &y, 16);
        x = y;
        if (n > 0xFF) return 0; /* bad byte */
        a.ether_addr_octet[ii] = n;
    }
    if (x[0] != 0) return 0; /* bad format */
    *p_a = a;
    return p_a;
}

struct ether_addr *ether_aton (const char *x) {
    static struct ether_addr a;
    return ether_aton_r (x, &a);
}

char *ether_ntoa_r (const struct ether_addr *p_a, char *x) {
    char *y;
    y = x;
    for (int ii = 0; ii < 6; ii++) {
        x += sprintf (x, ii == 0 ? "%.2X" : ":%.2X", p_a->ether_addr_octet[ii]);
    }
    return y;
}

char *ether_ntoa (const struct ether_addr *p_a) {
    static char x[18];
    return ether_ntoa_r (p_a, x);
}

int ether_line(const char *l, struct ether_addr *e, char *hostname) {
    return -1;
}

int ether_ntohost(char *hostname, const struct ether_addr *e) {
    return -1;
}

int ether_hostton(const char *hostname, struct ether_addr *e) {
    return -1;
}

/// >>> START src/network/freeaddrinfo.c

void freeaddrinfo(struct addrinfo *p) {
    free(p);
}

/// >>> START src/network/gai_strerror.c

static const char gai_strerror_c__msgs[] =
    "Invalid flags\0"
    "Name does not resolve\0"
    "Try again\0"
    "Non-recoverable error\0"
    "Unknown error\0"
    "Unrecognized address family or invalid length\0"
    "Unrecognized socket type\0"
    "Unrecognized service\0"
    "Unknown error\0"
    "Out of memory\0"
    "System error\0"
    "Overflow\0"
    "\0Unknown error";

const char *gai_strerror(int ecode) {
    const char *s;
    for (s=gai_strerror_c__msgs, ecode++; ecode && *s; ecode++, s++) for (; *s; s++);
    return *s ? s : s+1;
}

/// >>> START src/network/gethostbyname.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

struct hostent *gethostbyname(const char *name) {
    return gethostbyname2(name, AF_INET);
}

#if 0
struct hostent *gethostbyname(const char *name) {
    static struct hostent h;
    static char *h_aliases[3];
    static char h_canon[256];
    static char *h_addr_list[10];
    static char h_addr_data[10][4];
    static const struct addrinfo hint = {
        .ai_family = AF_INET, .ai_flags = AI_CANONNAME
    };
    struct addrinfo *ai, *p;
    int i;

    switch (getaddrinfo(name, 0, &hint, &ai)) {
    case EAI_NONAME:
        h_errno = HOST_NOT_FOUND;
        break;
    case EAI_AGAIN:
        h_errno = TRY_AGAIN;
        break;
    case EAI_FAIL:
        h_errno = NO_RECOVERY;
        break;
    default:
    case EAI_MEMORY:
    case EAI_SYSTEM:
        h_errno = NO_DATA;
        break;
    case 0:
        break;
    }

    strcpy(h_canon, ai->ai_canonname);
    h.h_name = h_canon;
    h.h_aliases = h_aliases;
    h.h_aliases[0] = h_canon;
    h.h_aliases[1] = strcmp(h_canon, name) ? (char *)name : 0;
    h.h_length = 4;
    h.h_addr_list = h_addr_list;
    for (i=0, p=ai; i<sizeof h_addr_data/4 && p; i++, p=p->ai_next) {
        h.h_addr_list[i] = h_addr_data[i];
        memcpy(h.h_addr_list[i],
            &((struct sockaddr_in *)p->ai_addr)->sin_addr, 4);
    }
    h.h_addr_list[i] = 0;
    h.h_addrtype = AF_INET;
    freeaddrinfo(ai);
    return &h;
}
#endif

/// >>> START src/network/gethostbyname2.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

struct hostent *gethostbyname2(const char *name, int af) {
    static struct hostent *h;
    size_t size = 63;
    struct hostent *res;
    int err;
    do {
        free(h);
        h = malloc(size+=size+1);
        if (!h) {
            h_errno = NO_RECOVERY;
            return 0;
        }
        err = gethostbyname2_r(name, af, h,
            (void *)(h+1), size-sizeof *h, &res, &h_errno);
    } while (err == ERANGE);
    return err ? 0 : h;
}

/// >>> START src/network/gethostbyname2_r.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int gethostbyname2_r(const char *name, int af,
    struct hostent *h, char *buf, size_t buflen,
    struct hostent **res, int *err) {
    struct addrinfo hint = {
        .ai_family = af==AF_INET6 ? af : AF_INET,
        .ai_flags = AI_CANONNAME
    };
    struct addrinfo *ai, *p;
    int i;
    size_t need;
    const char *canon;

    af = hint.ai_family;

    /* Align buffer */
    i = (uintptr_t)buf & sizeof(char *)-1;
    if (i) {
        if (buflen < sizeof(char *)-i) return ERANGE;
        buf += sizeof(char *)-i;
        buflen -= sizeof(char *)-i;
    }

    switch (getaddrinfo(name, 0, &hint, &ai)) {
    case EAI_NONAME:
        *err = HOST_NOT_FOUND;
        return errno;
    case EAI_AGAIN:
        *err = TRY_AGAIN;
        return errno;
    default:
    case EAI_MEMORY:
    case EAI_SYSTEM:
    case EAI_FAIL:
        *err = NO_RECOVERY;
        return errno;
    case 0:
        break;
    }

    h->h_addrtype = af;
    h->h_length = af==AF_INET6 ? 16 : 4;

    canon = ai->ai_canonname ? ai->ai_canonname : name;
    need = 4*sizeof(char *);
    for (i=0, p=ai; p; i++, p=p->ai_next)
        need += sizeof(char *) + h->h_length;
    need += strlen(name)+1;
    need += strlen(canon)+1;

    if (need > buflen) {
        freeaddrinfo(ai);
        return ERANGE;
    }

    h->h_aliases = (void *)buf;
    buf += 3*sizeof(char *);
    h->h_addr_list = (void *)buf;
    buf += (i+1)*sizeof(char *);

    h->h_name = h->h_aliases[0] = buf;
    strcpy(h->h_name, canon);
    buf += strlen(h->h_name)+1;

    if (strcmp(h->h_name, name)) {
        h->h_aliases[1] = buf;
        strcpy(h->h_aliases[1], name);
        buf += strlen(h->h_aliases[1])+1;
    } else h->h_aliases[1] = 0;

    h->h_aliases[2] = 0;

    for (i=0, p=ai; p; i++, p=p->ai_next) {
        h->h_addr_list[i] = (void *)buf;
        buf += h->h_length;
        memcpy(h->h_addr_list[i],
            &((struct sockaddr_in *)p->ai_addr)->sin_addr,
            h->h_length);
    }
    h->h_addr_list[i] = 0;

    *res = h;
    freeaddrinfo(ai);
    return 0;
}

/// >>> START src/network/gethostbyname_r.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int gethostbyname_r(const char *name,
    struct hostent *h, char *buf, size_t buflen,
    struct hostent **res, int *err) {
    return gethostbyname2_r(name, AF_INET, h, buf, buflen, res, err);
}

/// >>> START src/network/getifaddrs.c
/* (C) 2013 John Spencer. released under musl's standard MIT license. */
#undef _GNU_SOURCE
#undef _GNU_SOURCE
#define _GNU_SOURCE

typedef union {
    struct sockaddr_in6 getifaddrs_c__v6;
    struct sockaddr_in getifaddrs_c__v4;
} soa;

typedef struct ifaddrs_storage {
    struct ifaddrs ifa;
    soa addr;
    soa netmask;
    soa dst;
    char name[IFNAMSIZ+1];
} stor;
#undef next
#define next ifa.ifa_next

static stor* list_add(stor** list, stor** head, char* ifname) {
    stor* curr = calloc(1, sizeof(stor));
    if(curr) {
        strcpy(curr->name, ifname);
        curr->ifa.ifa_name = curr->name;
        if(*head) (*head)->next = (struct ifaddrs*) curr;
        *head = curr;
        if(!*list) *list = curr;
    }
    return curr;
}

void freeifaddrs(struct ifaddrs *ifp) {
    stor *head = (stor *) ifp;
    while(head) {
        void *p = head;
        head = (stor *) head->next;
        free(p);
    }
}

static void ipv6netmask(unsigned prefix_length, struct sockaddr_in6 *sa) {
    unsigned char* hb = sa->sin6_addr.s6_addr;
    unsigned onebytes = prefix_length / 8;
    unsigned bits = prefix_length % 8;
    unsigned nullbytes = 16 - onebytes;
    memset(hb, -1, onebytes);
    memset(hb+onebytes, 0, nullbytes);
    if(bits) {
        unsigned char x = -1;
        x <<= 8 - bits;
        hb[onebytes] = x;
    }
}

static void dealwithipv6(stor **list, stor** head) {
    FILE* f = fopen("/proc/net/if_inet6", "rbe");
    /* 00000000000000000000000000000001 01 80 10 80 lo
       A                                B  C  D  E  F
       all numbers in hex
       A = addr B=netlink device#, C=prefix length,
       D = scope value (ipv6.h) E = interface flags (rnetlink.h, addrconf.c)
       F = if name */
    char v6conv[32 + 7 + 1], *getifaddrs_c__v6;
    char *line, linebuf[512];
    if(!f) return;
    while((line = fgets(linebuf, sizeof linebuf, f))) {
        getifaddrs_c__v6 = v6conv;
        size_t i = 0;
        for(; i < 8; i++) {
            memcpy(getifaddrs_c__v6, line, 4);
            getifaddrs_c__v6+=4;
            *getifaddrs_c__v6++=':';
            line+=4;
        }
        --getifaddrs_c__v6; *getifaddrs_c__v6 = 0;
        line++;
        unsigned b, c, d, e;
        char name[IFNAMSIZ+1];
        if(5 == sscanf(line, "%x %x %x %x %s", &b, &c, &d, &e, name)) {
            struct sockaddr_in6 sa = {0};
            if(1 == inet_pton(AF_INET6, v6conv, &sa.sin6_addr)) {
                sa.sin6_family = AF_INET6;
                stor* curr = list_add(list, head, name);
                if(!curr) goto out;
                curr->addr.getifaddrs_c__v6 = sa;
                curr->ifa.ifa_addr = (struct sockaddr*) &curr->addr;
                ipv6netmask(c, &sa);
                curr->netmask.getifaddrs_c__v6 = sa;
                curr->ifa.ifa_netmask = (struct sockaddr*) &curr->netmask;
                /* find ipv4 struct with the same interface name to copy flags */
                stor* scan = *list;
                for(;scan && strcmp(name, scan->name);scan=(stor*)scan->next);
                if(scan) curr->ifa.ifa_flags = scan->ifa.ifa_flags;
                else curr->ifa.ifa_flags = 0;
            } else errno = 0;
        }
    }
    out:
    fclose(f);
}

int getifaddrs(struct ifaddrs **ifap) {
    stor *list = 0, *head = 0;
    struct if_nameindex* ii = if_nameindex();
    if(!ii) return -1;
    size_t i;
    for(i = 0; ii[i].if_index || ii[i].if_name; i++) {
        stor* curr = list_add(&list, &head, ii[i].if_name);
        if(!curr) {
            if_freenameindex(ii);
            goto err2;
        }
    }
    if_freenameindex(ii);

    int sock = socket(PF_INET, SOCK_DGRAM|SOCK_CLOEXEC, IPPROTO_IP);
    if(sock == -1) goto err2;
    struct ifreq reqs[32]; /* arbitrary chosen boundary */
    struct ifconf conf = {.ifc_len = sizeof reqs, .ifc_req = reqs};
    if(-1 == ioctl(sock, SIOCGIFCONF, &conf)) goto err;
    size_t reqitems = conf.ifc_len / sizeof(struct ifreq);
    for(head = list; head; head = (stor*)head->next) {
        for(i = 0; i < reqitems; i++) {
            // get SIOCGIFADDR of active interfaces.
            if(!strcmp(reqs[i].ifr_name, head->name)) {
                head->addr.getifaddrs_c__v4 = *(struct sockaddr_in*)&reqs[i].ifr_addr;
                head->ifa.ifa_addr = (struct sockaddr*) &head->addr;
                break;
            }
        }
        struct ifreq req;
        snprintf(req.ifr_name, sizeof req.ifr_name, "%s", head->name);
        if(-1 == ioctl(sock, SIOCGIFFLAGS, &req)) goto err;

        head->ifa.ifa_flags = req.ifr_flags;
        if(head->ifa.ifa_addr) {
            /* or'ing flags with IFF_LOWER_UP on active interfaces to mimic glibc */
            head->ifa.ifa_flags |= IFF_LOWER_UP; 
            if(-1 == ioctl(sock, SIOCGIFNETMASK, &req)) goto err;
            head->netmask.getifaddrs_c__v4 = *(struct sockaddr_in*)&req.ifr_netmask;
            head->ifa.ifa_netmask = (struct sockaddr*) &head->netmask;
    
            if(head->ifa.ifa_flags & IFF_POINTOPOINT) {
                if(-1 == ioctl(sock, SIOCGIFDSTADDR, &req)) goto err;
                head->dst.getifaddrs_c__v4 = *(struct sockaddr_in*)&req.ifr_dstaddr;
            } else {
                if(-1 == ioctl(sock, SIOCGIFBRDADDR, &req)) goto err;
                head->dst.getifaddrs_c__v4 = *(struct sockaddr_in*)&req.ifr_broadaddr;
            }
            head->ifa.ifa_ifu.ifu_dstaddr = (struct sockaddr*) &head->dst;
        }
    }
    close(sock);
    void* last = 0;
    for(head = list; head; head=(stor*)head->next) last=head;
    head = last;
    dealwithipv6(&list, &head);
    *ifap = (struct ifaddrs*) list;
    return 0;
    err:
    close(sock);
    err2:
    freeifaddrs((struct ifaddrs*) list);
    return -1;
}

#undef next

/// >>> START src/network/getservbyname.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

struct servent *getservbyname(const char *name, const char *prots) {
    static struct servent se;
    static long buf[32/sizeof(long)];
    struct servent *res;
    if (getservbyname_r(name, prots, &se, (void *)buf, sizeof buf, &res))
        return 0;
    return &se;
}

/// >>> START src/network/getservbyname_r.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int getservbyname_r(const char *name, const char *prots,
    struct servent *se, char *buf, size_t buflen, struct servent **res) {
    struct addrinfo *ai, hint = { .ai_family = AF_INET };
    int i;

    if (!prots) {
        int r = getservbyname_r(name, "tcp", se, buf, buflen, res);
        if (r) r = getservbyname_r(name, "udp", se, buf, buflen, res);
        return r;
    }

    /* Align buffer */
    i = (uintptr_t)buf & sizeof(char *)-1;
    if (!i) i = sizeof(char *);
    if (buflen < 3*sizeof(char *)-i)
        return ERANGE;
    buf += sizeof(char *)-i;
    buflen -= sizeof(char *)-i;

    if (!strcmp(prots, "tcp")) hint.ai_protocol = IPPROTO_TCP;
    else if (!strcmp(prots, "udp")) hint.ai_protocol = IPPROTO_UDP;
    else return EINVAL;

    switch (getaddrinfo(0, name, &hint, &ai)) {
    case EAI_MEMORY:
    case EAI_SYSTEM:
        return ENOMEM;
    default:
        return ENOENT;
    case 0:
        break;
    }

    se->s_name = (char *)name;
    se->s_aliases = (void *)buf;
    se->s_aliases[0] = se->s_name;
    se->s_aliases[1] = 0;
    se->s_port = ((struct sockaddr_in *)ai->ai_addr)->sin_port;
    se->s_proto = (char *)prots;

    freeaddrinfo(ai);
    *res = se;
    return 0;
}

/// >>> START src/network/h_errno.c

#undef h_errno
int h_errno;

int *__h_errno_location(void) {
    return &h_errno;
}

/// >>> START src/network/herror.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

void herror(const char *msg) {
    fprintf(stderr, "%s%s%s", msg?msg:"", msg?": ":"", hstrerror(h_errno));
}

/// >>> START src/network/hstrerror.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

static const char hstrerror_c__msgs[] =
    "Host not found\0"
    "Try again\0"
    "Non-recoverable error\0"
    "Address not available\0"
    "\0Unknown error";

const char *hstrerror(int ecode) {
    const char *s;
    for (s=hstrerror_c__msgs, ecode--; ecode && *s; ecode--, s++) for (; *s; s++);
    return *s ? s : s+1;
}

/// >>> START src/network/htonl.c

uint32_t htonl(uint32_t n) {
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_32(n) : n;
}

/// >>> START src/network/htons.c

uint16_t htons(uint16_t n) {
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_16(n) : n;
}

/// >>> START src/network/if_freenameindex.c

void if_freenameindex(struct if_nameindex *idx) {
    free(idx);
}

/// >>> START src/network/if_indextoname.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *if_indextoname(unsigned index, char *name) {
    struct ifreq ifr;
    int fd, r;

    if ((fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0)) < 0) return 0;
    ifr.ifr_ifindex = index;
    r = ioctl(fd, SIOCGIFNAME, &ifr);
    __syscall(SYS_close, fd);
    return r < 0 ? 0 : strncpy(name, ifr.ifr_name, IF_NAMESIZE);
}

/// >>> START src/network/if_nameindex.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

static void *do_nameindex(int s, size_t n) {
    size_t i, len, if_nameindex_c__k;
    struct ifconf conf;
    struct if_nameindex *idx;

    idx = malloc(n * (sizeof(struct if_nameindex)+sizeof(struct ifreq)));
    if (!idx) return 0;

    conf.ifc_buf = (void *)&idx[n];
    conf.ifc_len = len = n * sizeof(struct ifreq);
    if (ioctl(s, SIOCGIFCONF, &conf) < 0) {
        free(idx);
        return 0;
    }
    if (conf.ifc_len == len) {
        free(idx);
        return (void *)-1;
    }

    n = conf.ifc_len / sizeof(struct ifreq);
    for (i=if_nameindex_c__k=0; i<n; i++) {
        if (ioctl(s, SIOCGIFINDEX, &conf.ifc_req[i]) < 0) {
            if_nameindex_c__k++;
            continue;
        }
        idx[i-if_nameindex_c__k].if_index = conf.ifc_req[i].ifr_ifindex;
        idx[i-if_nameindex_c__k].if_name = conf.ifc_req[i].ifr_name;
    }
    idx[i-if_nameindex_c__k].if_name = 0;
    idx[i-if_nameindex_c__k].if_index = 0;

    return idx;
}

struct if_nameindex *if_nameindex() {
    size_t n;
    void *p = 0;
    int s = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0);
    if (s>=0) {
        for (n=0; (p=do_nameindex(s, n)) == (void *)-1; n++);
        __syscall(SYS_close, s);
    }
    errno = ENOBUFS;
    return p;
}

/// >>> START src/network/if_nametoindex.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

unsigned if_nametoindex(const char *name) {
    struct ifreq ifr;
    int fd, r;

    if ((fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0)) < 0) return -1;
    strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);
    r = ioctl(fd, SIOCGIFINDEX, &ifr);
    __syscall(SYS_close, fd);
    return r < 0 ? r : ifr.ifr_ifindex;
}

/// >>> START src/network/in6addr_any.c

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;

/// >>> START src/network/in6addr_loopback.c

const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;

/// >>> START src/network/inet_addr.c

in_addr_t inet_addr(const char *p) {
    struct sockaddr_in sin;
    if (__ipparse(&sin, AF_INET, p) < 0) return -1;
    return sin.sin_addr.s_addr;
}

/// >>> START src/network/inet_legacy.c

in_addr_t inet_network(const char *p) {
    return ntohl(inet_addr(p));
}

int inet_aton(const char *inet_legacy_c__cp, struct in_addr *inp) {
    struct sockaddr_in sin;
    if (__ipparse(&sin, AF_INET, inet_legacy_c__cp) < 0) return 0;
    *inp = sin.sin_addr;
    return 1;
}

struct in_addr inet_makeaddr(in_addr_t n, in_addr_t h) {
    if (n < 256) h |= n<<24;
    else if (n < 65536) h |= n<<16;
    else h |= n<<8;
    return (struct in_addr){ h };
}

in_addr_t inet_lnaof(struct in_addr in) {
    uint32_t h = in.s_addr;
    if (h>>24 < 128) return h & 0xffffff;
    if (h>>24 < 192) return h & 0xffff;
    return h & 0xff;
}

in_addr_t inet_netof(struct in_addr in) {
    uint32_t h = in.s_addr;
    if (h>>24 < 128) return h >> 24;
    if (h>>24 < 192) return h >> 16;
    return h >> 8;
}

/// >>> START src/network/inet_ntoa.c

char *inet_ntoa(struct in_addr in) {
    static char buf[16];
    unsigned char *a = (void *)&in;
    snprintf(buf, sizeof buf, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
    return buf;
}

/// >>> START src/network/inet_ntop.c

const char *inet_ntop(int af, const void *restrict inet_ntop_c__a0, char *restrict s, socklen_t l) {
    const unsigned char *a = inet_ntop_c__a0;
    int i, j, max, best;
    char buf[100];

    switch (af) {
    case AF_INET:
        if (snprintf(s, l, "%d.%d.%d.%d", a[0],a[1],a[2],a[3]) < l)
            return s;
        break;
    case AF_INET6:
        if (memcmp(a, "\0\0\0\0\0\0\0\0\0\0\377\377", 12))
            snprintf(buf, sizeof buf,
                "%x:%x:%x:%x:%x:%x:%x:%x",
                256*a[0]+a[1],256*a[2]+a[3],
                256*a[4]+a[5],256*a[6]+a[7],
                256*a[8]+a[9],256*a[10]+a[11],
                256*a[12]+a[13],256*a[14]+a[15]);
        else
            snprintf(buf, sizeof buf,
                "%x:%x:%x:%x:%x:%x:%d.%d.%d.%d",
                256*a[0]+a[1],256*a[2]+a[3],
                256*a[4]+a[5],256*a[6]+a[7],
                256*a[8]+a[9],256*a[10]+a[11],
                a[12],a[13],a[14],a[15]);
        /* Replace longest /(^0|:)[:0]{2,}/ with "::" */
        for (i=best=0, max=2; buf[i]; i++) {
            if (i && buf[i] != ':') continue;
            j = strspn(buf+i, ":0");
            if (j>max) best=i, max=j;
        }
        if (max>2) {
            buf[best] = buf[best+1] = ':';
            memmove(buf+best+2, buf+best+max, i-best-max+1);
        }
        if (strlen(buf) < l) {
            strcpy(s, buf);
            return s;
        }
        break;
    default:
        errno = EAFNOSUPPORT;
        return 0;
    }
    errno = ENOSPC;
    return 0;
}

/// >>> START src/network/inet_pton.c

static int hexval(unsigned c) {
    if (c-'0'<10) return c-'0';
    c |= 32;
    if (c-'a'<6) return c-'a'+10;
    return -1;
}

int inet_pton(int af, const char *restrict s, void *restrict inet_pton_c__a0) {
    uint16_t ip[8];
    unsigned char *a = inet_pton_c__a0;
    int i, j, v, d, brk=-1, need_v4=0;

    if (af==AF_INET) {
        for (i=0; i<4; i++) {
            for (v=j=0; j<3 && isdigit(s[j]); j++)
                v = 10*v + s[j]-'0';
            if (j==0 || (j>1 && s[0]=='0') || v>255) return 0;
            a[i] = v;
            if (s[j]==0 && i==3) return 1;
            if (s[j]!='.') return 0;
            s += j+1;
        }
        return 0;
    } else if (af!=AF_INET6) {
        errno = EAFNOSUPPORT;
        return -1;
    }

    if (*s==':' && *++s!=':') return 0;

    for (i=0; ; i++) {
        if (s[0]==':' && brk<0) {
            brk=i;
            ip[i]=0;
            if (!*++s) break;
            continue;
        }
        for (v=j=0; j<4 && (d=hexval(s[j]))>=0; j++)
            v=16*v+d;
        if (j==0) return 0;
        ip[i] = v;
        if (!s[j] && (brk>=0 || i==7)) break;
        if (i==7) return 0;
        if (s[j]!=':') {
            if (s[j]!='.' || (i<6 && brk<0)) return 0;
            need_v4=1;
            i++;
            break;
        }
        s += j+1;
    }
    if (brk>=0) {
        memmove(ip+brk+7-i, ip+brk, 2*(i+1-brk));
        for (j=0; j<7-i; j++) ip[brk+j] = 0;
    }
    for (j=0; j<8; j++) {
        *a++ = ip[j]>>8;
        *a++ = ip[j];
    }
    if (need_v4 && inet_pton(AF_INET, (void *)s, a-4) <= 0) return 0;
    return 1;
}

/// >>> START src/network/netname.c

struct netent *getnetbyaddr(uint32_t net, int type) {
    return 0;
}

struct netent *getnetbyname(const char *name) {
    return 0;
}

/// >>> START src/network/ntohl.c

uint32_t ntohl(uint32_t n) {
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_32(n) : n;
}

/// >>> START src/network/ntohs.c

uint16_t ntohs(uint16_t n) {
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_16(n) : n;
}

/// >>> START src/network/proto.c

/* do we really need all these?? */

static int idx;
static const unsigned char protos[][6] = {
    "\000ip",
    "\001icmp",
    "\002igmp",
    "\003ggp",
    "\006tcp",
    "\014pup",
    "\021udp",
    "\026idp",
    "\377raw"
    "\0\0"
};

void endprotoent(void) {
    idx = 0;
}

void setprotoent(int stayopen) {
    idx = 0;
}

struct protoent *getprotoent(void) {
    static struct protoent p;
    static const char *aliases;
    if (!protos[idx][1]) return NULL;
    p.p_proto = protos[idx][0];
    p.p_name = (char *)protos[idx++]+1;
    p.p_aliases = (char **)&aliases;
    return &p;
}

struct protoent *getprotobyname(const char *name) {
    struct protoent *p;
    endprotoent();
    do p = getprotoent();
    while (p && strcmp(name, p->p_name));
    return p;
}

struct protoent *getprotobynumber(int num) {
    struct protoent *p;
    endprotoent();
    do p = getprotoent();
    while (p && p->p_proto != num);
    return p;
}

/// >>> START src/network/serv.c

void endservent(void) {
}

void setservent(int stayopen) {
}

struct servent *getservent(void) {
    return 0;
}

/// >>> START src/network/sockatmark.c

int sockatmark(int s) {
    int ret;
    if (ioctl(s, SIOCATMARK, &ret) < 0)
        return -1;
    return ret;
}

/// >>> START src/passwd/fgetgrent.c
#undef _GNU_SOURCE
#define _GNU_SOURCE
/// >>> START src/passwd/pwf.h

struct passwd *__getpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size);
struct group *__getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem);
int __parsespent(char *s, struct spwd *sp);

/// >>> CONTINUE src/passwd/fgetgrent.c

struct group *fgetgrent(FILE *f) {
    static char *line, **mem;
    static struct group gr;
    size_t size=0, nmem=0;
    return __getgrent_a(f, &gr, &line, &size, &mem, &nmem);
}

/// >>> START src/passwd/fgetpwent.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

struct passwd *fgetpwent(FILE *f) {
    static char *line;
    static struct passwd pw;
    size_t size=0;
    return __getpwent_a(f, &pw, &line, &size);
}

/// >>> START src/passwd/fgetspent.c

struct spwd *fgetspent(FILE *f) {
    static char *line;
    static struct spwd sp;
    size_t size = 0;
    struct spwd *res = 0;
    int cs;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
    if (getline(&line, &size, f) >= 0 && __parsespent(line, &sp) >= 0) res = &sp;
    pthread_setcancelstate(cs, 0);
    return res;
}

/// >>> START src/passwd/getgr_r.c

#undef FIX
#define FIX(x) (gr->gr_##x = gr->gr_##x-line+buf)

static int getgr_r(const char *name, gid_t gid, struct group *gr, char *buf, size_t size, struct group **res) {
    FILE *f;
    char *line = 0;
    size_t len = 0;
    char **mem = 0;
    size_t nmem = 0;
    int rv = 0;
    size_t i;
    int cs;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

    f = fopen("/etc/group", "rbe");
    if (!f) {
        rv = errno;
        goto done;
    }

    *res = 0;
    while (__getgrent_a(f, gr, &line, &len, &mem, &nmem)) {
        if (name && !strcmp(name, gr->gr_name)
        || !name && gr->gr_gid == gid) {
            if (size < len + (nmem+1)*sizeof(char *) + 32) {
                rv = ERANGE;
                break;
            }
            *res = gr;
            buf += (16-(uintptr_t)buf)%16;
            gr->gr_mem = (void *)buf;
            buf += (nmem+1)*sizeof(char *);
            memcpy(buf, line, len);
            FIX(name);
            FIX(passwd);
            for (i=0; mem[i]; i++)
                gr->gr_mem[i] = mem[i]-line+buf;
            gr->gr_mem[i] = 0;
            break;
        }
    }
     free(mem);
     free(line);
    fclose(f);
done:
    pthread_setcancelstate(cs, 0);
    return rv;
}

int getgrnam_r(const char *name, struct group *gr, char *buf, size_t size, struct group **res) {
    return getgr_r(name, 0, gr, buf, size, res);
}

int getgrgid_r(gid_t gid, struct group *gr, char *buf, size_t size, struct group **res) {
    return getgr_r(0, gid, gr, buf, size, res);
}

/// >>> START src/passwd/getgrent.c

static FILE *f;

void setgrent() {
    if (f) fclose(f);
    f = 0;
}

weak_alias(setgrent, endgrent);

struct group *getgrent() {
    static char *line, **mem;
    static struct group gr;
    size_t size=0, nmem=0;
    if (!f) f = fopen("/etc/group", "rbe");
    if (!f) return 0;
    return __getgrent_a(f, &gr, &line, &size, &mem, &nmem);
}

struct group *getgrgid(gid_t gid) {
    struct group *gr;
    int errno_saved;
    setgrent();
    while ((gr=getgrent()) && gr->gr_gid != gid);
    errno_saved = errno;
    endgrent();
    errno = errno_saved;
    return gr;
}

struct group *getgrnam(const char *name) {
    struct group *gr;
    int errno_saved;
    setgrent();
    while ((gr=getgrent()) && strcmp(gr->gr_name, name));
    errno_saved = errno;
    endgrent();
    errno = errno_saved;
    return gr;
}

/// >>> START src/passwd/getgrent_a.c

static unsigned getgrent_a_c__atou(char **s) {
    unsigned x;
    for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
    return x;
}

struct group *__getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem) {
    ssize_t l;
    char *s, *mems;
    size_t i;
    int cs;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
    for (;;) {
        if ((l=getline(line, size, f)) < 0) {
            free(*line);
            *line = 0;
            gr = 0;
            goto end;
        }
        line[0][l-1] = 0;

        s = line[0];
        gr->gr_name = s++;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; gr->gr_passwd = s;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; gr->gr_gid = getgrent_a_c__atou(&s);
        if (*s != ':') continue;

        *s++ = 0; mems = s;
        break;
    }

    for (*nmem=!!*s; *s; s++)
        if (*s==',') ++*nmem;
    free(*mem);
    *mem = calloc(sizeof(char *), *nmem+1);
    if (!*mem) {
        free(*line);
        *line = 0;
        return 0;
    }
    if (*mems) {
        mem[0][0] = mems;
        for (s=mems, i=0; *s; s++)
            if (*s==',') *s++ = 0, mem[0][++i] = s;
        mem[0][++i] = 0;
    } else {
        mem[0][0] = 0;
    }
    gr->gr_mem = *mem;
end:
    pthread_setcancelstate(cs, 0);
    return gr;
}

/// >>> START src/passwd/getpw_r.c

#undef FIX
#define FIX(x) (pw->pw_##x = pw->pw_##x-line+buf)

static int getpw_r(const char *name, uid_t uid, struct passwd *pw, char *buf, size_t size, struct passwd **res) {
    FILE *f;
    char *line = 0;
    size_t len = 0;
    int rv = 0;
    int cs;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

    f = fopen("/etc/passwd", "rbe");
    if (!f) {
        rv = errno;
        goto done;
    }

    *res = 0;
    while (__getpwent_a(f, pw, &line, &len)) {
        if (name && !strcmp(name, pw->pw_name)
        || !name && pw->pw_uid == uid) {
            if (size < len) {
                rv = ERANGE;
                break;
            }
            *res = pw;
            memcpy(buf, line, len);
            FIX(name);
            FIX(passwd);
            FIX(gecos);
            FIX(dir);
            FIX(shell);
            break;
        }
    }
     free(line);
    fclose(f);
done:
    pthread_setcancelstate(cs, 0);
    return rv;
}

int getpwnam_r(const char *name, struct passwd *pw, char *buf, size_t size, struct passwd **res) {
    return getpw_r(name, 0, pw, buf, size, res);
}

int getpwuid_r(uid_t uid, struct passwd *pw, char *buf, size_t size, struct passwd **res) {
    return getpw_r(0, uid, pw, buf, size, res);
}

/// >>> START src/passwd/getpwent.c

static FILE *f;

void setpwent() {
    if (f) fclose(f);
    f = 0;
}

weak_alias(setpwent, endpwent);

struct passwd *getpwent() {
    static char *line;
    static struct passwd pw;
    size_t size=0;
    if (!f) f = fopen("/etc/passwd", "rbe");
    if (!f) return 0;
    return __getpwent_a(f, &pw, &line, &size);
}

struct passwd *getpwuid(uid_t uid) {
    struct passwd *pw;
    int errno_saved;
    setpwent();
    while ((pw=getpwent()) && pw->pw_uid != uid);
    errno_saved = errno;
    endpwent();
    errno = errno_saved;
    return pw;
}

struct passwd *getpwnam(const char *name) {
    struct passwd *pw;
    int errno_saved;
    setpwent();
    while ((pw=getpwent()) && strcmp(pw->pw_name, name));
    errno_saved = errno;
    endpwent();
    errno = errno_saved;
    return pw;
}

/// >>> START src/passwd/getpwent_a.c

static unsigned getpwent_a_c__atou(char **s) {
    unsigned x;
    for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
    return x;
}

struct passwd *__getpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size) {
    ssize_t l;
    char *s;
    int cs;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
    for (;;) {
        if ((l=getline(line, size, f)) < 0) {
            free(*line);
            *line = 0;
            pw = 0;
            break;
        }
        line[0][l-1] = 0;

        s = line[0];
        pw->pw_name = s++;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; pw->pw_passwd = s;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; pw->pw_uid = getpwent_a_c__atou(&s);
        if (*s != ':') continue;

        *s++ = 0; pw->pw_gid = getpwent_a_c__atou(&s);
        if (*s != ':') continue;

        *s++ = 0; pw->pw_gecos = s;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; pw->pw_dir = s;
        if (!(s = strchr(s, ':'))) continue;

        *s++ = 0; pw->pw_shell = s;
        break;
    }
    pthread_setcancelstate(cs, 0);
    return pw;
}

/// >>> START src/passwd/getspent.c

void setspent() {
}

void endspent() {
}

struct spwd *getspent() {
    return 0;
}

/// >>> START src/passwd/getspnam.c

#undef LINE_LIM
#define LINE_LIM 256

struct spwd *getspnam(const char *name) {
    static struct spwd sp;
    static char *line;
    struct spwd *res;
    int e;

    if (!line) line = malloc(LINE_LIM);
    if (!line) return 0;
    e = getspnam_r(name, &sp, line, LINE_LIM, &res);
    if (e) errno = e;
    return res;
}

/// >>> START src/passwd/getspnam_r.c

/* This implementation support Openwall-style TCB passwords in place of
 * traditional shadow, if the appropriate directories and files exist.
 * Thus, it is careful to avoid following symlinks or blocking on fifos
 * which a malicious user might create in place of his or her TCB shadow
 * file. It also avoids any allocation to prevent memory-exhaustion
 * attacks via getspnam_r_c__huge TCB shadow files. */

static long xatol(char **s) {
    long x;
    if (**s == ':' || **s == '\n') return -1;
    for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
    return x;
}

int __parsespent(char *s, struct spwd *sp) {
    sp->sp_namp = s;
    if (!(s = strchr(s, ':'))) return -1;
    *s = 0;

    sp->sp_pwdp = ++s;
    if (!(s = strchr(s, ':'))) return -1;
    *s = 0;

    s++; sp->sp_lstchg = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_min = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_max = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_warn = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_inact = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_expire = xatol(&s);
    if (*s != ':') return -1;

    s++; sp->sp_flag = xatol(&s);
    if (*s != '\n') return -1;
    return 0;
}

static void cleanup(void *p) {
    fclose(p);
}

int getspnam_r(const char *name, struct spwd *sp, char *buf, size_t size, struct spwd **res) {
    char path[20+NAME_MAX];
    FILE *f = 0;
    int rv = 0;
    int fd;
    size_t getspnam_r_c__k, l = strlen(name);
    int skip = 0;
    int cs;

    *res = 0;

    /* Disallow potentially-malicious user names */
    if (*name=='.' || strchr(name, '/') || !l)
        return EINVAL;

    /* Buffer size must at least be able to hold name, plus some.. */
    if (size < l+100) return ERANGE;

    /* Protect against truncation */
    if (snprintf(path, sizeof path, "/etc/tcb/%s/shadow", name) >= sizeof path)
        return EINVAL;

    fd = open(path, O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if (fd >= 0) {
        struct stat getspnam_r_c__st = { 0 };
        errno = EINVAL;
        if (fstat(fd, &getspnam_r_c__st) || !S_ISREG(getspnam_r_c__st.st_mode) || !(f = fdopen(fd, "rb"))) {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
            close(fd);
            pthread_setcancelstate(cs, 0);
            return errno;
        }
    } else {
        f = fopen("/etc/shadow", "rbe");
        if (!f) return errno;
    }

    pthread_cleanup_push(cleanup, f);
    while (fgets(buf, size, f) && (getspnam_r_c__k=strlen(buf))>0) {
        if (skip || strncmp(name, buf, l)) {
            skip = buf[getspnam_r_c__k-1] != '\n';
            continue;
        }
        if (buf[getspnam_r_c__k-1] != '\n') {
            rv = ERANGE;
            break;
        }

        if (__parsespent(buf, sp) < 0) continue;
        *res = sp;
        break;
    }
    pthread_cleanup_pop(1);
    return rv;
}

/// >>> START src/passwd/lckpwdf.c

int lckpwdf() {
    return 0;
}

int ulckpwdf() {
    return 0;
}

/// >>> START src/passwd/putpwent.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int putpwent(const struct passwd *pw, FILE *f) {
    return fprintf(f, "%s:%s:%d:%d:%s:%s:%s\n",
        pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
        pw->pw_gecos, pw->pw_dir, pw->pw_shell)<0 ? -1 : 0;
}

/// >>> START src/passwd/putspent.c

#undef NUM
#define NUM(n) ((n) == -1 ? 0 : -1), ((n) == -1 ? 0 : (n))
#undef STR
#define STR(s) ((s) ? (s) : "")

int putspent(const struct spwd *sp, FILE *f) {
    return fprintf(f, "%s:%s:%.*ld:%.*ld:%.*ld:%.*ld:%.*ld:%.*ld:%.*lu\n",
        STR(sp->sp_namp), STR(sp->sp_pwdp), NUM(sp->sp_lstchg),
        NUM(sp->sp_min), NUM(sp->sp_max), NUM(sp->sp_warn),
        NUM(sp->sp_inact), NUM(sp->sp_expire), NUM(sp->sp_flag)) < 0 ? -1 : 0;
}

/// >>> START src/search/hsearch.c

/*
open addressing hash hsearch_c__table with 2^n hsearch_c__table size
quadratic probing is used in case of hash collision
tab indices and hash are size_t
after resize fails with ENOMEM the state of tab is still usable

with the posix api items cannot be iterated and length cannot be queried
*/

#undef MINSIZE
#define MINSIZE 8
#undef MAXSIZE
#define MAXSIZE ((size_t)-1/2 + 1)

struct elem {
    ENTRY item;
    size_t hash;
};

static size_t mask;
static size_t used;
static struct elem *tab;

static size_t keyhash(char *hsearch_c__k) {
    unsigned char *p = (void *)hsearch_c__k;
    size_t h = 0;

    while (*p)
        h = 31*h + *p++;
    return h;
}

static int resize(size_t nel) {
    size_t newsize;
    size_t i, j;
    struct elem *e, *newe;
    struct elem *oldtab = tab;
    struct elem *oldend = tab + mask + 1;

    if (nel > MAXSIZE)
        nel = MAXSIZE;
    for (newsize = MINSIZE; newsize < nel; newsize *= 2);
    tab = calloc(newsize, sizeof *tab);
    if (!tab) {
        tab = oldtab;
        return 0;
    }
    mask = newsize - 1;
    if (!oldtab)
        return 1;
    for (e = oldtab; e < oldend; e++)
        if (e->item.key) {
            for (i=e->hash,j=1; ; i+=j++) {
                newe = tab + (i & mask);
                if (!newe->item.key)
                    break;
            }
            *newe = *e;
        }
    free(oldtab);
    return 1;
}

int hcreate(size_t nel) {
    mask = 0;
    used = 0;
    tab = 0;
    return resize(nel);
}

void hdestroy(void) {
    free(tab);
    tab = 0;
    mask = 0;
    used = 0;
}

static struct elem *lookup(char *key, size_t hash) {
    size_t i, j;
    struct elem *e;

    for (i=hash,j=1; ; i+=j++) {
        e = tab + (i & mask);
        if (!e->item.key ||
            (e->hash==hash && strcmp(e->item.key, key)==0))
            break;
    }
    return e;
}

ENTRY *hsearch(ENTRY item, ACTION action) {
    size_t hash = keyhash(item.key);
    struct elem *e = lookup(item.key, hash);

    if (e->item.key)
        return &e->item;
    if (action == FIND)
        return 0;
    e->item = item;
    e->hash = hash;
    if (++used > mask - mask/4) {
        if (!resize(2*used)) {
            used--;
            e->item.key = 0;
            return 0;
        }
        e = lookup(item.key, hash);
    }
    return &e->item;
}

/// >>> START src/search/insque.c

struct insque_c__node {
    struct insque_c__node *next;
    struct insque_c__node *prev;
};

void insque(void *element, void *pred) {
    struct insque_c__node *e = element;
    struct insque_c__node *p = pred;

    if (!p) {
        e->next = e->prev = 0;
        return;
    }
    e->next = p->next;
    e->prev = p;
    p->next = e;
    if (e->next)
        e->next->prev = e;
}

void remque(void *element) {
    struct insque_c__node *e = element;

    if (e->next)
        e->next->prev = e->prev;
    if (e->prev)
        e->prev->next = e->next;
}

/// >>> START src/search/lsearch.c

void *lsearch(const void *key, void *base, size_t *nelp, size_t width,
    int (*compar)(const void *, const void *)) {
    char (*p)[width] = base;
    size_t n = *nelp;
    size_t i;

    for (i = 0; i < n; i++)
        if (compar(p[i], key) == 0)
            return p[i];
    *nelp = n+1;
    return memcpy(p[n], key, width);
}

void *lfind(const void *key, const void *base, size_t *nelp,
    size_t width, int (*compar)(const void *, const void *)) {
    char (*p)[width] = (void *)base;
    size_t n = *nelp;
    size_t i;

    for (i = 0; i < n; i++)
        if (compar(p[i], key) == 0)
            return p[i];
    return 0;
}

/// >>> START src/search/tdestroy.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

struct tdestroy_c__node {
    void *key;
    struct tdestroy_c__node *left;
    struct tdestroy_c__node *right;
};

void tdestroy(void *root, void (*freekey)(void *)) {
    struct tdestroy_c__node *r = root;

    if (r == 0)
        return;
    tdestroy(r->left, freekey);
    tdestroy(r->right, freekey);
    if (freekey) freekey(r->key);
    free(r);
}

/// >>> START src/search/tsearch_avl.c

/*
avl tree implementation using recursive functions
the height of an n tsearch_avl_c__node tree is less than 1.44*log2(n+2)-1
(so the max recursion depth in case of a tree with 2^32 nodes is 45)
*/

struct tsearch_avl_c__node {
    const void *key;
    struct tsearch_avl_c__node *left;
    struct tsearch_avl_c__node *right;
    int height;
};

static int delta(struct tsearch_avl_c__node *n) {
    return (n->left ? n->left->height:0) - (n->right ? n->right->height:0);
}

static void updateheight(struct tsearch_avl_c__node *n) {
    n->height = 0;
    if (n->left && n->left->height > n->height)
        n->height = n->left->height;
    if (n->right && n->right->height > n->height)
        n->height = n->right->height;
    n->height++;
}

static struct tsearch_avl_c__node *rotl(struct tsearch_avl_c__node *n) {
    struct tsearch_avl_c__node *r = n->right;
    n->right = r->left;
    r->left = n;
    updateheight(n);
    updateheight(r);
    return r;
}

static struct tsearch_avl_c__node *rotr(struct tsearch_avl_c__node *n) {
    struct tsearch_avl_c__node *l = n->left;
    n->left = l->right;
    l->right = n;
    updateheight(n);
    updateheight(l);
    return l;
}

static struct tsearch_avl_c__node *balance(struct tsearch_avl_c__node *n) {
    int d = delta(n);

    if (d < -1) {
        if (delta(n->right) > 0)
            n->right = rotr(n->right);
        return rotl(n);
    } else if (d > 1) {
        if (delta(n->left) < 0)
            n->left = rotl(n->left);
        return rotr(n);
    }
    updateheight(n);
    return n;
}

static struct tsearch_avl_c__node *find(struct tsearch_avl_c__node *n, const void *tsearch_avl_c__k,
    int (*cmp)(const void *, const void *)) {
    int c;

    if (!n)
        return 0;
    c = cmp(tsearch_avl_c__k, n->key);
    if (c == 0)
        return n;
    if (c < 0)
        return find(n->left, tsearch_avl_c__k, cmp);
    else
        return find(n->right, tsearch_avl_c__k, cmp);
}

static struct tsearch_avl_c__node *insert(struct tsearch_avl_c__node **n, const void *tsearch_avl_c__k,
    int (*cmp)(const void *, const void *), int *new) {
    struct tsearch_avl_c__node *r = *n;
    int c;

    if (!r) {
        *n = r = malloc(sizeof **n);
        if (r) {
            r->key = tsearch_avl_c__k;
            r->left = r->right = 0;
            r->height = 1;
        }
        *new = 1;
        return r;
    }
    c = cmp(tsearch_avl_c__k, r->key);
    if (c == 0)
        return r;
    if (c < 0)
        r = insert(&r->left, tsearch_avl_c__k, cmp, new);
    else
        r = insert(&r->right, tsearch_avl_c__k, cmp, new);
    if (*new)
        *n = balance(*n);
    return r;
}

static struct tsearch_avl_c__node *movr(struct tsearch_avl_c__node *n, struct tsearch_avl_c__node *r) {
    if (!n)
        return r;
    n->right = movr(n->right, r);
    return balance(n);
}

static struct tsearch_avl_c__node *tsearch_avl_c__remove(struct tsearch_avl_c__node **n, const void *tsearch_avl_c__k,
    int (*cmp)(const void *, const void *), struct tsearch_avl_c__node *parent) {
    int c;

    if (!*n)
        return 0;
    c = cmp(tsearch_avl_c__k, (*n)->key);
    if (c == 0) {
        struct tsearch_avl_c__node *r = *n;
        *n = movr(r->left, r->right);
        free(r);
        return parent;
    }
    if (c < 0)
        parent = tsearch_avl_c__remove(&(*n)->left, tsearch_avl_c__k, cmp, *n);
    else
        parent = tsearch_avl_c__remove(&(*n)->right, tsearch_avl_c__k, cmp, *n);
    if (parent)
        *n = balance(*n);
    return parent;
}

void *tdelete(const void *restrict key, void **restrict rootp,
    int(*compar)(const void *, const void *)) {
    struct tsearch_avl_c__node *n = *rootp;
    struct tsearch_avl_c__node *ret;
    /* last argument is arbitrary non-null pointer
       which is returned when the root tsearch_avl_c__node is deleted */
    ret = tsearch_avl_c__remove(&n, key, compar, n);
    *rootp = n;
    return ret;
}

void *tfind(const void *key, void *const *rootp,
    int(*compar)(const void *, const void *)) {
    return find(*rootp, key, compar);
}

void *tsearch(const void *key, void **rootp,
    int (*compar)(const void *, const void *)) {
    int new = 0;
    struct tsearch_avl_c__node *n = *rootp;
    struct tsearch_avl_c__node *ret;
    ret = insert(&n, key, compar, &new);
    *rootp = n;
    return ret;
}

static void walk(const struct tsearch_avl_c__node *r, void (*action)(const void *, VISIT, int), int d) {
    if (r == 0)
        return;
    if (r->left == 0 && r->right == 0)
        action(r, leaf, d);
    else {
        action(r, preorder, d);
        walk(r->left, action, d+1);
        action(r, postorder, d);
        walk(r->right, action, d+1);
        action(r, endorder, d);
    }
}

void twalk(const void *root, void (*action)(const void *, VISIT, int)) {
    walk(root, action, 0);
}

/// >>> START src/signal/block.c
/// >>> START src/internal/pthread_impl.h
#ifndef _PTHREAD_IMPL_H
#undef _PTHREAD_IMPL_H
#define _PTHREAD_IMPL_H

#undef pthread
#define pthread __pthread

struct pthread {
    struct pthread *self;
    void **dtv, *unused1, *unused2;
    uintptr_t sysinfo;
    uintptr_t canary;
    pid_t tid, pid;
    int tsd_used, errno_val, *errno_ptr;
    volatile int cancel, canceldisable, cancelasync;
    int detached;
    unsigned char *map_base;
    size_t map_size;
    void *stack;
    size_t stack_size;
    void *start_arg;
    void *(*start)(void *);
    void *result;
    struct __ptcb *cancelbuf;
    void **tsd;
    pthread_attr_t attr;
    volatile int dead;
    struct {
        void **head;
        long off;
        void *pending;
    } robust_list;
    int unblock_cancel;
    int timer_id;
    locale_t locale;
    int killlock[2];
    int exitlock[2];
    int startlock[2];
    unsigned long sigmask[_NSIG/8/sizeof(long)];
};

struct __timer {
    int timerid;
    pthread_t thread;
};

#undef __SU
#define __SU (sizeof(size_t)/sizeof(int))

#undef _a_stacksize
#define _a_stacksize __u.__s[0]
#undef _a_guardsize
#define _a_guardsize __u.__s[1]
#undef _a_stackaddr
#define _a_stackaddr __u.__s[2]
#undef _a_detach
#define _a_detach __u.__i[3*__SU+0]
#undef _a_sched
#define _a_sched __u.__i[3*__SU+1]
#undef _a_policy
#define _a_policy __u.__i[3*__SU+2]
#undef _a_prio
#define _a_prio __u.__i[3*__SU+3]
#undef _m_type
#define _m_type __u.__i[0]
#undef _m_lock
#define _m_lock __u.__i[1]
#undef _m_waiters
#define _m_waiters __u.__i[2]
#undef _m_prev
#define _m_prev __u.__p[3]
#undef _m_next
#define _m_next __u.__p[4]
#undef _m_count
#define _m_count __u.__i[5]
#undef _c_mutex
#define _c_mutex __u.__p[0]
#undef _c_seq
#define _c_seq __u.__i[2]
#undef _c_waiters
#define _c_waiters __u.__i[3]
#undef _c_clock
#define _c_clock __u.__i[4]
#undef _c_lock
#define _c_lock __u.__i[5]
#undef _c_lockwait
#define _c_lockwait __u.__i[6]
#undef _c_waiters2
#define _c_waiters2 __u.__i[7]
#undef _c_destroy
#define _c_destroy __u.__i[8]
#undef _rw_lock
#define _rw_lock __u.__i[0]
#undef _rw_waiters
#define _rw_waiters __u.__i[1]
#undef _b_lock
#define _b_lock __u.__i[0]
#undef _b_waiters
#define _b_waiters __u.__i[1]
#undef _b_limit
#define _b_limit __u.__i[2]
#undef _b_count
#define _b_count __u.__i[3]
#undef _b_waiters2
#define _b_waiters2 __u.__i[4]
#undef _b_inst
#define _b_inst __u.__p[3]

#undef SIGTIMER
#define SIGTIMER 32
#undef SIGCANCEL
#define SIGCANCEL 33
#undef SIGSYNCCALL
#define SIGSYNCCALL 34

#undef SIGALL_SET
#define SIGALL_SET ((sigset_t *)(const unsigned long long [2]){ -1,-1 })
#undef SIGPT_SET
#define SIGPT_SET \
    ((sigset_t *)(const unsigned long [_NSIG/8/sizeof(long)]){ \
    [sizeof(long)==4] = 3UL<<(32*(sizeof(long)>4)) })
#undef SIGTIMER_SET
#define SIGTIMER_SET \
    ((sigset_t *)(const unsigned long [_NSIG/8/sizeof(long)]){ \
     0x80000000 })

pthread_t __pthread_self_init(void);

int __clone(int (*)(void *), void *, int, void *, ...);
int __set_thread_area(void *);
int __libc_sigaction(int, const struct sigaction *, struct sigaction *);
int __libc_sigprocmask(int, const sigset_t *, sigset_t *);
void __lock(volatile int *);
void __unmapself(void *, size_t);

int __timedwait(volatile int *, int, clockid_t, const struct timespec *, void (*)(void *), void *, int);
void __wait(volatile int *, volatile int *, int, int);
#undef __wake
#define __wake(addr, cnt, priv) \
    __syscall(SYS_futex, addr, FUTEX_WAKE, (cnt)<0?INT_MAX:(cnt))

void __acquire_ptc();
void __release_ptc();
void __inhibit_ptc();

void __block_all_sigs(void *);
void __block_app_sigs(void *);
void __restore_sigs(void *);

#undef DEFAULT_STACK_SIZE
#define DEFAULT_STACK_SIZE 81920
#undef DEFAULT_GUARD_SIZE
#define DEFAULT_GUARD_SIZE PAGE_SIZE

#endif

/// >>> CONTINUE src/signal/block.c

static const unsigned long all_mask[] = {
#if ULONG_MAX == 0xffffffff && _NSIG == 129
    -1UL, -1UL, -1UL, -1UL
#elif ULONG_MAX == 0xffffffff
    -1UL, -1UL
#else
    -1UL
#endif
};

static const unsigned long app_mask[] = {
#if ULONG_MAX == 0xffffffff
#if _NSIG == 65
    0x7fffffff, 0xfffffffc
#else
    0x7fffffff, 0xfffffffc, -1UL, -1UL
#endif
#else
#if _NSIG == 65
    0xfffffffc7fffffff
#else
    0xfffffffc7fffffff, -1UL
#endif
#endif
};

void __block_all_sigs(void *set) {
    __syscall(SYS_rt_sigprocmask, SIG_BLOCK, &all_mask, set, _NSIG/8);
}

void __block_app_sigs(void *set) {
    __syscall(SYS_rt_sigprocmask, SIG_BLOCK, &app_mask, set, _NSIG/8);
}

void __restore_sigs(void *set) {
    __syscall(SYS_rt_sigprocmask, SIG_SETMASK, set, 0, _NSIG/8);
}

/// >>> START src/signal/getitimer.c

int getitimer(int which, struct itimerval *old) {
    return syscall(SYS_getitimer, which, old);
}

/// >>> START src/signal/killpg.c

int killpg(pid_t pgid, int sig) {
    if (pgid < 0) {
        errno = EINVAL;
        return -1;
    }
    return kill(-pgid, sig);
}

/// >>> START src/signal/psiginfo.c

void psiginfo(const siginfo_t *si, const char *msg) {
    char *s = strsignal(si->si_signo);
    if (msg) fprintf(stderr, "%s: %s\n", msg, s);
    else fprintf(stderr, "%s\n", s);
}

/// >>> START src/signal/psignal.c

void psignal(int sig, const char *msg) {
    char *s = strsignal(sig);
    if (msg) fprintf(stderr, "%s: %s\n", msg, s);
    else fprintf(stderr, "%s\n", s);
}

/// >>> START src/signal/raise.c

int raise(int sig) {
    int pid, tid, ret;
    sigset_t set;
    __block_app_sigs(&set);
    tid = __syscall(SYS_gettid);
    pid = __syscall(SYS_getpid);
    ret = syscall(SYS_tgkill, pid, tid, sig);
    __restore_sigs(&set);
    return ret;
}

/// >>> START src/signal/restore.c
/* These functions will not work, but suffice for targets where the
 * kernel sigaction structure does not actually use sa_restorer. */

void __restore() {
}

void __restore_rt() {
}

/// >>> START src/signal/setitimer.c

int setitimer(int which, const struct itimerval *restrict new, struct itimerval *restrict old) {
    return syscall(SYS_setitimer, which, new, old);
}

/// >>> START src/signal/sigaction.c

void __restore(), __restore_rt();

static pthread_t sigaction_c__dummy(void) { return 0; }
weak_alias(sigaction_c__dummy, __pthread_self_def);

static unsigned long handler_set[_NSIG/(8*sizeof(long))];

void __get_handler_set(sigset_t *set) {
    memcpy(set, handler_set, sizeof handler_set);
}

int __libc_sigaction(int sig, const struct sigaction *restrict sa, struct sigaction *restrict old) {
    struct k_sigaction ksa, ksa_old;
    if (sig >= (unsigned)_NSIG) {
        errno = EINVAL;
        return -1;
    }
    if (sa) {
        if ((uintptr_t)sa->sa_handler > 1UL) {
            a_or_l(handler_set+(sig-1)/(8*sizeof(long)),
                1UL<<(sig-1)%(8*sizeof(long)));
            __pthread_self_def();
        }
        ksa.handler = sa->sa_handler;
        ksa.flags = sa->sa_flags | SA_RESTORER;
        ksa.restorer = (sa->sa_flags & SA_SIGINFO) ? __restore_rt : __restore;
        memcpy(&ksa.mask, &sa->sa_mask, sizeof ksa.mask);
    }
    if (syscall(SYS_rt_sigaction, sig, sa?&ksa:0, old?&ksa_old:0, sizeof ksa.mask))
        return -1;
    if (old) {
        old->sa_handler = ksa_old.handler;
        old->sa_flags = ksa_old.flags;
        memcpy(&old->sa_mask, &ksa_old.mask, sizeof ksa_old.mask);
    }
    return 0;
}

int __sigaction(int sig, const struct sigaction *restrict sa, struct sigaction *restrict old) {
    if (sig-32U < 3) {
        errno = EINVAL;
        return -1;
    }
    return __libc_sigaction(sig, sa, old);
}

weak_alias(__sigaction, sigaction);

/// >>> START src/signal/sigaddset.c

int sigaddset(sigset_t *set, int sig) {
    unsigned s = sig-1;
    if (s >= _NSIG-1 || sig-32U < 3) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[s/8/sizeof *set->__bits] |= 1UL<<(s&8*sizeof *set->__bits-1);
    return 0;
}

/// >>> START src/signal/sigaltstack.c

int sigaltstack(const stack_t *restrict ss, stack_t *restrict old) {
    if (ss) {
        if (ss->ss_size < MINSIGSTKSZ) {
            errno = ENOMEM;
            return -1;
        }
        if (ss->ss_flags & ~SS_DISABLE) {
            errno = EINVAL;
            return -1;
        }
    }
    return syscall(SYS_sigaltstack, ss, old);
}

/// >>> START src/signal/sigandset.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

#undef SST_SIZE
#define SST_SIZE (_NSIG/8/sizeof(long))

int sigandset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
    unsigned long i = 0, *d = (void*) dest, *l = (void*) left, *r = (void*) right;
    for(; i < SST_SIZE; i++) d[i] = l[i] & r[i];
    return 0;
}

/// >>> START src/signal/sigdelset.c

int sigdelset(sigset_t *set, int sig) {
    unsigned s = sig-1;
    if (s >= _NSIG-1 || sig-32U < 3) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[s/8/sizeof *set->__bits] &=~(1UL<<(s&8*sizeof *set->__bits-1));
    return 0;
}

/// >>> START src/signal/sigemptyset.c

int sigemptyset(sigset_t *set) {
    set->__bits[0] = 0;
    if (sizeof(long)==4 || _NSIG > 65) set->__bits[1] = 0;
    if (sizeof(long)==4 && _NSIG > 65) {
        set->__bits[2] = 0;
        set->__bits[3] = 0;
    }
    return 0;
}

/// >>> START src/signal/sigfillset.c

int sigfillset(sigset_t *set) {
#if ULONG_MAX == 0xffffffff
    set->__bits[0] = 0x7ffffffful;
    set->__bits[1] = 0xfffffffcul;
    if (_NSIG > 65) {
        set->__bits[2] = 0xfffffffful;
        set->__bits[3] = 0xfffffffful;
    }
#else
    set->__bits[0] = 0xfffffffc7ffffffful;
    if (_NSIG > 65) set->__bits[1] = 0xfffffffffffffffful;
#endif
    return 0;
}

/// >>> START src/signal/sighold.c

int sighold(int sig) {
    sigset_t mask;

    sigemptyset(&mask);
    if (sigaddset(&mask, sig) < 0) return -1;
    return sigprocmask(SIG_BLOCK, &mask, 0);
}

/// >>> START src/signal/sigignore.c

int sigignore(int sig) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    return sigaction(sig, &sa, 0);
}

/// >>> START src/signal/siginterrupt.c

int siginterrupt(int sig, int flag) {
    struct sigaction sa;

    sigaction(sig, 0, &sa);
    if (flag) sa.sa_flags &= ~SA_RESTART;
    else sa.sa_flags |= SA_RESTART;

    return sigaction(sig, &sa, 0);
}

/// >>> START src/signal/sigisemptyset.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int sigisemptyset(const sigset_t *set) {
    static const unsigned long zeroset[_NSIG/8/sizeof(long)];
    return !memcmp(set, &zeroset, _NSIG/8);
}

/// >>> START src/signal/sigismember.c

int sigismember(const sigset_t *set, int sig) {
    unsigned s = sig-1;
    if (s >= _NSIG-1) return 0;
    return !!(set->__bits[s/8/sizeof *set->__bits] & 1UL<<(s&8*sizeof *set->__bits-1));
}

/// >>> START src/signal/signal.c

int __sigaction(int, const struct sigaction *, struct sigaction *);

void (*signal(int sig, void (*func)(int)))(int) {
    struct sigaction sa_old, sa = { .sa_handler = func, .sa_flags = SA_RESTART };
    if (__sigaction(sig, &sa, &sa_old) < 0)
        return SIG_ERR;
    return sa_old.sa_handler;
}

weak_alias(signal, bsd_signal);

/// >>> START src/signal/sigorset.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

#undef SST_SIZE
#define SST_SIZE (_NSIG/8/sizeof(long))

int sigorset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
    unsigned long i = 0, *d = (void*) dest, *l = (void*) left, *r = (void*) right;
    for(; i < SST_SIZE; i++) d[i] = l[i] | r[i];
    return 0;
}

/// >>> START src/signal/sigpause.c

int sigpause(int sig) {
    sigset_t mask;
    sigprocmask(0, 0, &mask);
    sigdelset(&mask, sig);
    return sigsuspend(&mask);
}

/// >>> START src/signal/sigpending.c

int sigpending(sigset_t *set) {
    return syscall(SYS_rt_sigpending, set, _NSIG/8);
}

/// >>> START src/signal/sigrtmax.c

int __libc_current_sigrtmax() {
    return _NSIG-1;
}

/// >>> START src/signal/sigrtmin.c
int __libc_current_sigrtmin() {
    return 35;
}

/// >>> START src/signal/sigset.c

void (*sigset(int sig, void (*handler)(int)))(int) {
    struct sigaction sa, sa_old;
    sigset_t mask;

    sigemptyset(&mask);
    if (sigaddset(&mask, sig) < 0)
        return SIG_ERR;
    
    if (handler == SIG_HOLD) {
        if (sigaction(sig, 0, &sa_old) < 0)
            return SIG_ERR;
        if (sigprocmask(SIG_BLOCK, &mask, &mask) < 0)
            return SIG_ERR;
    } else {
        sa.sa_handler = handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig, &sa, &sa_old) < 0)
            return SIG_ERR;
        if (sigprocmask(SIG_UNBLOCK, &mask, &mask) < 0)
            return SIG_ERR;
    }
    return sigismember(&mask, sig) ? SIG_HOLD : sa_old.sa_handler;
}

/// >>> START src/signal/sigsuspend.c

int sigsuspend(const sigset_t *mask) {
    return syscall_cp(SYS_rt_sigsuspend, mask, _NSIG/8);
}

/// >>> START src/signal/sigtimedwait.c

int sigtimedwait(const sigset_t *restrict mask, siginfo_t *restrict si, const struct timespec *restrict timeout) {
    int ret;
    do ret = syscall_cp(SYS_rt_sigtimedwait, mask,
        si, timeout, _NSIG/8);
    while (ret<0 && errno==EINTR);
    return ret;
}

/// >>> START src/signal/sigwait.c

int sigwait(const sigset_t *restrict mask, int *restrict sig) {
    siginfo_t si;
    if (sigtimedwait(mask, &si, 0) < 0)
        return -1;
    *sig = si.si_signo;
    return 0;
}

/// >>> START src/signal/sigwaitinfo.c

int sigwaitinfo(const sigset_t *restrict mask, siginfo_t *restrict si) {
    return sigtimedwait(mask, si, 0);
}

/// >>> START src/stdlib/abs.c
int abs(int a) {
    return a>0 ? a : -a;
}

/// >>> START src/stdlib/atof.c

double atof(const char *s) {
    return strtod(s, 0);
}

/// >>> START src/stdlib/atoi.c

int atoi(const char *s) {
    int n=0, neg=0;
    while (isspace(*s)) s++;
    switch (*s) {
    case '-': neg=1;
    case '+': s++;
    }
    /* Compute n as a negative number to avoid overflow on INT_MIN */
    while (isdigit(*s))
        n = 10*n - (*s++ - '0');
    return neg ? n : -n;
}

/// >>> START src/stdlib/atol.c

long atol(const char *s) {
    long n=0;
    int neg=0;
    while (isspace(*s)) s++;
    switch (*s) {
    case '-': neg=1;
    case '+': s++;
    }
    /* Compute n as a negative number to avoid overflow on LONG_MIN */
    while (isdigit(*s))
        n = 10*n - (*s++ - '0');
    return neg ? n : -n;
}

/// >>> START src/stdlib/atoll.c

long long atoll(const char *s) {
    long long n=0;
    int neg=0;
    while (isspace(*s)) s++;
    switch (*s) {
    case '-': neg=1;
    case '+': s++;
    }
    /* Compute n as a negative number to avoid overflow on LLONG_MIN */
    while (isdigit(*s))
        n = 10*n - (*s++ - '0');
    return neg ? n : -n;
}

/// >>> START src/stdlib/bsearch.c

void *bsearch(const void *key, const void *base, size_t nel, size_t width, int (*cmp)(const void *, const void *)) {
    void *bsearch_c__try;
    int sign;
    while (nel > 0) {
        bsearch_c__try = (char *)base + width*(nel/2);
        sign = cmp(key, bsearch_c__try);
        if (!sign) return bsearch_c__try;
        else if (nel == 1) break;
        else if (sign < 0)
            nel /= 2;
        else {
            base = bsearch_c__try;
            nel -= nel/2;
        }
    }
    return NULL;
}

/// >>> START src/stdlib/div.c

div_t div(int num, int den) {
    return (div_t){ num/den, num%den };
}

/// >>> START src/stdlib/ecvt.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *ecvt(double x, int n, int *dp, int *sign) {
    static char buf[16];
    char tmp[32];
    int i, j;

    if (n-1U > 15) n = 15;
    sprintf(tmp, "%.*e", n-1, x);
    i = *sign = (tmp[0]=='-');
    for (j=0; tmp[i]!='e'; j+=(tmp[i++]!='.'))
        buf[j] = tmp[i];
    buf[j] = 0;
    *dp = atoi(tmp+i+1)+1;

    return buf;
}

/// >>> START src/stdlib/fcvt.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *fcvt(double x, int n, int *dp, int *sign) {
    char tmp[1500];
    int i, lz;

    if (n > 1400U) n = 1400;
    sprintf(tmp, "%.*f", n, x);
    i = (tmp[0] == '-');
    if (tmp[i] == '0') lz = strspn(tmp+i+2, "0");
    else lz = -(int)strcspn(tmp+i, ".");

    if (n<=lz) {
        *sign = i;
        *dp = 1;
        if (n>14U) n = 14;
        return "000000000000000"+14-n;
    }

    return ecvt(x, n-lz, dp, sign);
}

/// >>> START src/stdlib/gcvt.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *gcvt(double x, int n, char *b) {
    sprintf(b, "%.*g", n, x);
    return b;
}

/// >>> START src/stdlib/imaxabs.c

intmax_t imaxabs(intmax_t a) {
    return a>0 ? a : -a;
}

/// >>> START src/stdlib/imaxdiv.c

imaxdiv_t imaxdiv(intmax_t num, intmax_t den) {
    return (imaxdiv_t){ num/den, num%den };
}

/// >>> START src/stdlib/labs.c
long labs(long a) {
    return a>0 ? a : -a;
}

/// >>> START src/stdlib/ldiv.c

ldiv_t ldiv(long num, long den) {
    return (ldiv_t){ num/den, num%den };
}

/// >>> START src/stdlib/llabs.c
long long llabs(long long a) {
    return a>0 ? a : -a;
}

/// >>> START src/stdlib/lldiv.c

lldiv_t lldiv(long long num, long long den) {
    return (lldiv_t){ num/den, num%den };
}

/// >>> START src/stdlib/qsort.c
/* Copyright (C) 2011 by Valentin Ochs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Minor changes by Rich Felker for integration in musl, 2011-04-27. */

#undef ntz
#define ntz(x) a_ctz_l((x))

typedef int (*cmpfun)(const void *, const void *);

static inline int pntz(size_t p[2]) {
    int r = ntz(p[0] - 1);
    if(r != 0 || (r = 8*sizeof(size_t) + ntz(p[1])) != 8*sizeof(size_t)) {
        return r;
    }
    return 0;
}

static void cycle(size_t width, unsigned char* ar[], int n) {
    unsigned char tmp[256];
    size_t l;
    int i;

    if(n < 2) {
        return;
    }

    ar[n] = tmp;
    while(width) {
        l = sizeof(tmp) < width ? sizeof(tmp) : width;
        memcpy(ar[n], ar[0], l);
        for(i = 0; i < n; i++) {
            memcpy(ar[i], ar[i + 1], l);
            ar[i] += l;
        }
        width -= l;
    }
}

/* shl() and shr() need n > 0 */
static inline void shl(size_t p[2], int n) {
    if(n >= 8 * sizeof(size_t)) {
        n -= 8 * sizeof(size_t);
        p[1] = p[0];
        p[0] = 0;
    }
    p[1] <<= n;
    p[1] |= p[0] >> (sizeof(size_t) * 8 - n);
    p[0] <<= n;
}

static inline void shr(size_t p[2], int n) {
    if(n >= 8 * sizeof(size_t)) {
        n -= 8 * sizeof(size_t);
        p[0] = p[1];
        p[1] = 0;
    }
    p[0] >>= n;
    p[0] |= p[1] << (sizeof(size_t) * 8 - n);
    p[1] >>= n;
}

static void sift(unsigned char *head, size_t width, cmpfun cmp, int pshift, size_t lp[]) {
    unsigned char *qsort_c__rt, *lf;
    unsigned char *ar[14 * sizeof(size_t) + 1];
    int i = 1;

    ar[0] = head;
    while(pshift > 1) {
        qsort_c__rt = head - width;
        lf = head - width - lp[pshift - 2];

        if((*cmp)(ar[0], lf) >= 0 && (*cmp)(ar[0], qsort_c__rt) >= 0) {
            break;
        }
        if((*cmp)(lf, qsort_c__rt) >= 0) {
            ar[i++] = lf;
            head = lf;
            pshift -= 1;
        } else {
            ar[i++] = qsort_c__rt;
            head = qsort_c__rt;
            pshift -= 2;
        }
    }
    cycle(width, ar, i);
}

static void trinkle(unsigned char *head, size_t width, cmpfun cmp, size_t pp[2], int pshift, int trusty, size_t lp[]) {
    unsigned char *stepson,
                  *qsort_c__rt, *lf;
    size_t p[2];
    unsigned char *ar[14 * sizeof(size_t) + 1];
    int i = 1;
    int trail;

    p[0] = pp[0];
    p[1] = pp[1];

    ar[0] = head;
    while(p[0] != 1 || p[1] != 0) {
        stepson = head - lp[pshift];
        if((*cmp)(stepson, ar[0]) <= 0) {
            break;
        }
        if(!trusty && pshift > 1) {
            qsort_c__rt = head - width;
            lf = head - width - lp[pshift - 2];
            if((*cmp)(qsort_c__rt, stepson) >= 0 || (*cmp)(lf, stepson) >= 0) {
                break;
            }
        }

        ar[i++] = stepson;
        head = stepson;
        trail = pntz(p);
        shr(p, trail);
        pshift += trail;
        trusty = 0;
    }
    if(!trusty) {
        cycle(width, ar, i);
        sift(head, width, cmp, pshift, lp);
    }
}

void qsort(void *base, size_t nel, size_t width, cmpfun cmp) {
    size_t lp[12*sizeof(size_t)];
    size_t i, size = width * nel;
    unsigned char *head, *high;
    size_t p[2] = {1, 0};
    int pshift = 1;
    int trail;

    if (!size) return;

    head = base;
    high = head + size - width;

    /* Precompute Leonardo numbers, scaled by element width */
    for(lp[0]=lp[1]=width, i=2; (lp[i]=lp[i-2]+lp[i-1]+width) < size; i++);

    while(head < high) {
        if((p[0] & 3) == 3) {
            sift(head, width, cmp, pshift, lp);
            shr(p, 2);
            pshift += 2;
        } else {
            if(lp[pshift - 1] >= high - head) {
                trinkle(head, width, cmp, p, pshift, 0, lp);
            } else {
                sift(head, width, cmp, pshift, lp);
            }
            
            if(pshift == 1) {
                shl(p, 1);
                pshift = 0;
            } else {
                shl(p, pshift - 1);
                pshift = 1;
            }
        }
        
        p[0] |= 1;
        head += width;
    }

    trinkle(head, width, cmp, p, pshift, 0, lp);

    while(pshift != 1 || p[0] != 1 || p[1] != 0) {
        if(pshift <= 1) {
            trail = pntz(p);
            shr(p, trail);
            pshift += trail;
        } else {
            shl(p, 2);
            pshift -= 2;
            p[0] ^= 7;
            shr(p, 1);
            trinkle(head - lp[pshift] - width, width, cmp, p, pshift + 1, 1, lp);
            shl(p, 1);
            p[0] |= 1;
            trinkle(head - width, width, cmp, p, pshift, 1, lp);
        }
        head -= width;
    }
}

/// >>> START src/string/bcmp.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

int bcmp(const void *bcmp_c__s1, const void *bcmp_c__s2, size_t n) {
    return memcmp(bcmp_c__s1, bcmp_c__s2, n);
}

/// >>> START src/string/bcopy.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

void bcopy(const void *bcopy_c__s1, void *bcopy_c__s2, size_t n) {
    memmove(bcopy_c__s2, bcopy_c__s1, n);
}

/// >>> START src/string/bzero.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

void bzero(void *s, size_t n) {
    memset(s, 0, n);
}

/// >>> START src/string/index.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

char *index(const char *s, int c) {
    return strchr(s, c);
}

/// >>> START src/string/rindex.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

char *rindex(const char *s, int c) {
    return strrchr(s, c);
}

/// >>> START src/string/stpcpy.c

#undef ALIGN
#define ALIGN (sizeof(size_t))
#undef ONES
#define ONES ((size_t)-1/UCHAR_MAX)
#undef HIGHS
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#undef HASZERO
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__stpcpy(char *restrict d, const char *restrict s) {
    size_t *wd;
    const size_t *ws;

    if ((uintptr_t)s % ALIGN == (uintptr_t)d % ALIGN) {
        for (; (uintptr_t)s % ALIGN; s++, d++)
            if (!(*d=*s)) return d;
        wd=(void *)d; ws=(const void *)s;
        for (; !HASZERO(*ws); *wd++ = *ws++);
        d=(void *)wd; s=(const void *)ws;
    }
    for (; (*d=*s); s++, d++);

    return d;
}

weak_alias(__stpcpy, stpcpy);

/// >>> START src/string/stpncpy.c

#undef ALIGN
#define ALIGN (sizeof(size_t)-1)
#undef ONES
#define ONES ((size_t)-1/UCHAR_MAX)
#undef HIGHS
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#undef HASZERO
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__stpncpy(char *restrict d, const char *restrict s, size_t n) {
    size_t *wd;
    const size_t *ws;

    if (((uintptr_t)s & ALIGN) == ((uintptr_t)d & ALIGN)) {
        for (; ((uintptr_t)s & ALIGN) && n && (*d=*s); n--, s++, d++);
        if (!n || !*s) goto tail;
        wd=(void *)d; ws=(const void *)s;
        for (; n>=sizeof(size_t) && !HASZERO(*ws);
               n-=sizeof(size_t), ws++, wd++) *wd = *ws;
        d=(void *)wd; s=(const void *)ws;
    }
    for (; n && (*d=*s); n--, s++, d++);
tail:
    memset(d, 0, n);
    return d;
}

weak_alias(__stpncpy, stpncpy);

/// >>> START src/string/strcasecmp.c

int strcasecmp(const char *_l, const char *_r) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    for (; *l && *r && (*l == *r || tolower(*l) == tolower(*r)); l++, r++);
    return tolower(*l) - tolower(*r);
}

/// >>> START src/string/strcasestr.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *strcasestr(const char *h, const char *n) {
    size_t l = strlen(n);
    for (; *h; h++) if (!strncasecmp(h, n, l)) return (char *)h;
    return 0;
}

/// >>> START src/string/strcat.c

char *strcat(char *restrict dest, const char *restrict src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}

/// >>> START src/string/strchr.c

char *__strchrnul(const char *, int);

char *strchr(const char *s, int c) {
    char *r = __strchrnul(s, c);
    return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

/// >>> START src/string/strchrnul.c

#undef ALIGN
#define ALIGN (sizeof(size_t))
#undef ONES
#define ONES ((size_t)-1/UCHAR_MAX)
#undef HIGHS
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#undef HASZERO
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__strchrnul(const char *s, int c) {
    size_t *w, strchrnul_c__k;

    c = (unsigned char)c;
    if (!c) return (char *)s + strlen(s);

    for (; (uintptr_t)s % ALIGN; s++)
        if (!*s || *(unsigned char *)s == c) return (char *)s;
    strchrnul_c__k = ONES * c;
    for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^strchrnul_c__k); w++);
    for (s = (void *)w; *s && *(unsigned char *)s != c; s++);
    return (char *)s;
}

weak_alias(__strchrnul, strchrnul);

/// >>> START src/string/strcmp.c

int strcmp(const char *l, const char *r) {
    for (; *l==*r && *l; l++, r++);
    return *(unsigned char *)l - *(unsigned char *)r;
}

/// >>> START src/string/strcpy.c

char *__stpcpy(char *, const char *);

char *strcpy(char *restrict dest, const char *restrict src) {
#if 1
    __stpcpy(dest, src);
    return dest;
#else
    const unsigned char *s = src;
    unsigned char *d = dest;
    while ((*d++ = *s++));
    return dest;
#endif
}

/// >>> START src/string/strcspn.c

#undef BITOP
#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

char *__strchrnul(const char *, int);

size_t strcspn(const char *s, const char *c) {
    const char *a = s;
    size_t byteset[32/sizeof(size_t)];

    if (!c[0] || !c[1]) return __strchrnul(s, *c)-a;

    memset(byteset, 0, sizeof byteset);
    for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
    for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
    return s-a;
}

/// >>> START src/string/strdup.c

char *__strdup(const char *s) {
    size_t l = strlen(s);
    char *d = malloc(l+1);
    if (!d) return NULL;
    return memcpy(d, s, l+1);
}

weak_alias(__strdup, strdup);

/// >>> START src/string/strerror_r.c

int strerror_r(int err, char *buf, size_t buflen) {
    char *msg = strerror(err);
    size_t l = strlen(msg);
    if (l >= buflen) {
        if (buflen) {
            memcpy(buf, msg, buflen-1);
            buf[buflen-1] = 0;
        }
        return ERANGE;
    }
    memcpy(buf, msg, l+1);
    return 0;
}

weak_alias(strerror_r, __xpg_strerror_r);

/// >>> START src/string/strlcat.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

size_t strlcat(char *d, const char *s, size_t n) {
    size_t l = strnlen(d, n);
    if (l == n) return l + strlen(s);
    return l + strlcpy(d+l, s, n-l);
}

/// >>> START src/string/strlcpy.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

#undef ALIGN
#define ALIGN (sizeof(size_t)-1)
#undef ONES
#define ONES ((size_t)-1/UCHAR_MAX)
#undef HIGHS
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#undef HASZERO
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

size_t strlcpy(char *d, const char *s, size_t n) {
    char *d0 = d;
    size_t *wd;
    const size_t *ws;

    if (!n--) goto finish;
    if (((uintptr_t)s & ALIGN) == ((uintptr_t)d & ALIGN)) {
        for (; ((uintptr_t)s & ALIGN) && n && (*d=*s); n--, s++, d++);
        if (n && *s) {
            wd=(void *)d; ws=(const void *)s;
            for (; n>=sizeof(size_t) && !HASZERO(*ws);
                   n-=sizeof(size_t), ws++, wd++) *wd = *ws;
            d=(void *)wd; s=(const void *)ws;
        }
    }
    for (; n && (*d=*s); n--, s++, d++);
    *d = 0;
finish:
    return d-d0 + strlen(s);
}

/// >>> START src/string/strlen.c

#undef ALIGN
#define ALIGN (sizeof(size_t))
#undef ONES
#define ONES ((size_t)-1/UCHAR_MAX)
#undef HIGHS
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#undef HASZERO
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

size_t strlen(const char *s) {
    const char *a = s;
    const size_t *w;
    for (; (uintptr_t)s % ALIGN; s++) if (!*s) return s-a;
    for (w = (const void *)s; !HASZERO(*w); w++);
    for (s = (const void *)w; *s; s++);
    return s-a;
}

/// >>> START src/string/strncasecmp.c

int strncasecmp(const char *_l, const char *_r, size_t n) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    if (!n--) return 0;
    for (; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r)); l++, r++, n--);
    return tolower(*l) - tolower(*r);
}

/// >>> START src/string/strncat.c

char *strncat(char *restrict d, const char *restrict s, size_t n) {
    char *a = d;
    d += strlen(d);
    while (n && *s) n--, *d++ = *s++;
    *d++ = 0;
    return a;
}

/// >>> START src/string/strncmp.c

int strncmp(const char *_l, const char *_r, size_t n) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    if (!n--) return 0;
    for (; *l && *r && n && *l == *r ; l++, r++, n--);
    return *l - *r;
}

/// >>> START src/string/strncpy.c

char *__stpncpy(char *, const char *, size_t);

char *strncpy(char *restrict d, const char *restrict s, size_t n) {
    __stpncpy(d, s, n);
    return d;
}

/// >>> START src/string/strndup.c

char *strndup(const char *s, size_t n) {
    size_t l = strnlen(s, n);
    char *d = malloc(l+1);
    if (!d) return NULL;
    memcpy(d, s, l);
    d[l] = 0;
    return d;
}

/// >>> START src/string/strnlen.c

size_t strnlen(const char *s, size_t n) {
    const char *p = memchr(s, 0, n);
    return p ? p-s : n;
}

/// >>> START src/string/strpbrk.c

char *strpbrk(const char *s, const char *b) {
    s += strcspn(s, b);
    return *s ? (char *)s : 0;
}

/// >>> START src/string/strrchr.c

void *__memrchr(const void *, int, size_t);

char *strrchr(const char *s, int c) {
    return __memrchr(s, c, strlen(s) + 1);
}

/// >>> START src/string/strsep.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *strsep(char **str, const char *sep) {
    char *s = *str, *end;
    if (!s) return NULL;
    end = s + strcspn(s, sep);
    if (*end) *end++ = 0;
    else end = 0;
    *str = end;
    return s;
}

/// >>> START src/string/strsignal.c

#if (SIGHUP == 1) && (SIGINT == 2) && (SIGQUIT == 3) && (SIGILL == 4) \
 && (SIGTRAP == 5) && (SIGABRT == 6) && (SIGBUS == 7) && (SIGFPE == 8) \
 && (SIGKILL == 9) && (SIGUSR1 == 10) && (SIGSEGV == 11) && (SIGUSR2 == 12) \
 && (SIGPIPE == 13) && (SIGALRM == 14) && (SIGTERM == 15) && (SIGSTKFLT == 16) \
 && (SIGCHLD == 17) && (SIGCONT == 18) && (SIGSTOP == 19) && (SIGTSTP == 20) \
 && (SIGTTIN == 21) && (SIGTTOU == 22) && (SIGURG == 23) && (SIGXCPU == 24) \
 && (SIGXFSZ == 25) && (SIGVTALRM == 26) && (SIGPROF == 27) && (SIGWINCH == 28) \
 && (SIGPOLL == 29) && (SIGPWR == 30) && (SIGSYS == 31)

#undef sigmap
#define sigmap(x) x

#else

static const char map[] = {
    [SIGHUP]    = 1,
    [SIGINT]    = 2,
    [SIGQUIT]   = 3,
    [SIGILL]    = 4,
    [SIGTRAP]   = 5,
    [SIGABRT]   = 6,
    [SIGBUS]    = 7,
    [SIGFPE]    = 8,
    [SIGKILL]   = 9,
    [SIGUSR1]   = 10,
    [SIGSEGV]   = 11,
    [SIGUSR2]   = 12,
    [SIGPIPE]   = 13,
    [SIGALRM]   = 14,
    [SIGTERM]   = 15,
    [SIGSTKFLT] = 16,
    [SIGCHLD]   = 17,
    [SIGCONT]   = 18,
    [SIGSTOP]   = 19,
    [SIGTSTP]   = 20,
    [SIGTTIN]   = 21,
    [SIGTTOU]   = 22,
    [SIGURG]    = 23,
    [SIGXCPU]   = 24,
    [SIGXFSZ]   = 25,
    [SIGVTALRM] = 26,
    [SIGPROF]   = 27,
    [SIGWINCH]  = 28,
    [SIGPOLL]   = 29,
    [SIGPWR]    = 30,
    [SIGSYS]    = 31
};

#undef sigmap
#define sigmap(x) ((x) >= sizeof map ? (x) : map[(x)])

#endif

static const char strings[] =
    "Unknown signal\0"
    "Hangup\0"
    "Interrupt\0"
    "Quit\0"
    "Illegal instruction\0"
    "Trace/breakpoint trap\0"
    "Aborted\0"
    "Bus error\0"
    "Arithmetic exception\0"
    "Killed\0"
    "User defined signal 1\0"
    "Segmentation fault\0"
    "User defined signal 2\0"
    "Broken pipe\0"
    "Alarm clock\0"
    "Terminated\0"
    "Stack fault\0"
    "Child process status\0"
    "Continued\0"
    "Stopped (signal)\0"
    "Stopped\0"
    "Stopped (tty input)\0"
    "Stopped (tty output)\0"
    "Urgent I/O condition\0"
    "CPU time limit exceeded\0"
    "File size limit exceeded\0"
    "Virtual timer expired\0"
    "Profiling timer expired\0"
    "Window changed\0"
    "I/O possible\0"
    "Power failure\0"
    "Bad system call\0"
    "RT32"
    "\0RT33\0RT34\0RT35\0RT36\0RT37\0RT38\0RT39\0RT40"
    "\0RT41\0RT42\0RT43\0RT44\0RT45\0RT46\0RT47\0RT48"
    "\0RT49\0RT50\0RT51\0RT52\0RT53\0RT54\0RT55\0RT56"
    "\0RT57\0RT58\0RT59\0RT60\0RT61\0RT62\0RT63\0RT64"
#if _NSIG > 65
    "\0RT65\0RT66\0RT67\0RT68\0RT69\0RT70\0RT71\0RT72"
    "\0RT73\0RT74\0RT75\0RT76\0RT77\0RT78\0RT79\0RT80"
    "\0RT81\0RT82\0RT83\0RT84\0RT85\0RT86\0RT87\0RT88"
    "\0RT89\0RT90\0RT91\0RT92\0RT93\0RT94\0RT95\0RT96"
    "\0RT97\0RT98\0RT99\0RT100\0RT101\0RT102\0RT103\0RT104"
    "\0RT105\0RT106\0RT107\0RT108\0RT109\0RT110\0RT111\0RT112"
    "\0RT113\0RT114\0RT115\0RT116\0RT117\0RT118\0RT119\0RT120"
    "\0RT121\0RT122\0RT123\0RT124\0RT125\0RT126\0RT127\0RT128"
#endif
    "";

char *strsignal(int signum) {
    char *s = (char *)strings;

    signum = sigmap(signum);
    if (signum - 1U >= _NSIG-1) signum = 0;

    for (; signum--; s++) for (; *s; s++);

    return s;
}

/// >>> START src/string/strspn.c

#undef BITOP
#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

size_t strspn(const char *s, const char *c) {
    const char *a = s;
    size_t byteset[32/sizeof(size_t)] = { 0 };

    if (!c[0]) return 0;
    if (!c[1]) {
        for (; *s == *c; s++);
        return s-a;
    }

    for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
    for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);
    return s-a;
}

/// >>> START src/string/strstr.c

static char *twobyte_strstr(const unsigned char *h, const unsigned char *n) {
    uint16_t nw = n[0]<<8 | n[1], hw = h[0]<<8 | h[1];
    for (h++; *h && hw != nw; hw = hw<<8 | *++h);
    return *h ? (char *)h-1 : 0;
}

static char *threebyte_strstr(const unsigned char *h, const unsigned char *n) {
    uint32_t nw = n[0]<<24 | n[1]<<16 | n[2]<<8;
    uint32_t hw = h[0]<<24 | h[1]<<16 | h[2]<<8;
    for (h+=2; *h && hw != nw; hw = (hw|*++h)<<8);
    return *h ? (char *)h-2 : 0;
}

static char *fourbyte_strstr(const unsigned char *h, const unsigned char *n) {
    uint32_t nw = n[0]<<24 | n[1]<<16 | n[2]<<8 | n[3];
    uint32_t hw = h[0]<<24 | h[1]<<16 | h[2]<<8 | h[3];
    for (h+=3; *h && hw != nw; hw = hw<<8 | *++h);
    return *h ? (char *)h-3 : 0;
}

#undef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

#undef BITOP
#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

static char *twoway_strstr(const unsigned char *h, const unsigned char *n) {
    const unsigned char *z;
    size_t l, ip, jp, strstr_c__k, p, ms, p0, mem, mem0;
    size_t byteset[32 / sizeof(size_t)] = { 0 };
    size_t shift[256];

    /* Computing length of needle and fill shift strstr_c__table */
    for (l=0; n[l] && h[l]; l++)
        BITOP(byteset, n[l], |=), shift[n[l]] = l+1;
    if (n[l]) return 0; /* hit the end of h */

    /* Compute maximal suffix */
    ip = -1; jp = 0; strstr_c__k = p = 1;
    while (jp+strstr_c__k<l) {
        if (n[ip+strstr_c__k] == n[jp+strstr_c__k]) {
            if (strstr_c__k == p) {
                jp += p;
                strstr_c__k = 1;
            } else strstr_c__k++;
        } else if (n[ip+strstr_c__k] > n[jp+strstr_c__k]) {
            jp += strstr_c__k;
            strstr_c__k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            strstr_c__k = p = 1;
        }
    }
    ms = ip;
    p0 = p;

    /* And with the opposite comparison */
    ip = -1; jp = 0; strstr_c__k = p = 1;
    while (jp+strstr_c__k<l) {
        if (n[ip+strstr_c__k] == n[jp+strstr_c__k]) {
            if (strstr_c__k == p) {
                jp += p;
                strstr_c__k = 1;
            } else strstr_c__k++;
        } else if (n[ip+strstr_c__k] < n[jp+strstr_c__k]) {
            jp += strstr_c__k;
            strstr_c__k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            strstr_c__k = p = 1;
        }
    }
    if (ip+1 > ms+1) ms = ip;
    else p = p0;

    /* Periodic needle? */
    if (memcmp(n, n+p, ms+1)) {
        mem0 = 0;
        p = MAX(ms, l-ms-1) + 1;
    } else mem0 = l-p;
    mem = 0;

    /* Initialize incremental end-of-haystack pointer */
    z = h;

    /* Search loop */
    for (;;) {
        /* Update incremental end-of-haystack pointer */
        if (z-h < l) {
            /* Fast estimate for MIN(l,63) */
            size_t grow = l | 63;
            const unsigned char *z2 = memchr(z, 0, grow);
            if (z2) {
                z = z2;
                if (z-h < l) return 0;
            } else z += grow;
        }

        /* Check last byte first; advance by shift on mismatch */
        if (BITOP(byteset, h[l-1], &)) {
            strstr_c__k = l-shift[h[l-1]];
            //printf("adv by %zu (on %c) at [%s] (%zu;l=%zu)\n", strstr_c__k, h[l-1], h, shift[h[l-1]], l);
            if (strstr_c__k) {
                if (mem0 && mem && strstr_c__k < p) strstr_c__k = l-p;
                h += strstr_c__k;
                mem = 0;
                continue;
            }
        } else {
            h += l;
            mem = 0;
            continue;
        }

        /* Compare right strstr_c__half */
        for (strstr_c__k=MAX(ms+1,mem); n[strstr_c__k] && n[strstr_c__k] == h[strstr_c__k]; strstr_c__k++);
        if (n[strstr_c__k]) {
            h += strstr_c__k-ms;
            mem = 0;
            continue;
        }
        /* Compare left strstr_c__half */
        for (strstr_c__k=ms+1; strstr_c__k>mem && n[strstr_c__k-1] == h[strstr_c__k-1]; strstr_c__k--);
        if (strstr_c__k == mem) return (char *)h;
        h += p;
        mem = mem0;
    }
}

char *strstr(const char *h, const char *n) {
    /* Return immediately on empty needle */
    if (!n[0]) return (char *)h;

    /* Use faster algorithms for short needles */
    h = strchr(h, *n);
    if (!h || !n[1]) return (char *)h;
    if (!h[1]) return 0;
    if (!n[2]) return twobyte_strstr((void *)h, (void *)n);
    if (!h[2]) return 0;
    if (!n[3]) return threebyte_strstr((void *)h, (void *)n);
    if (!h[3]) return 0;
    if (!n[4]) return fourbyte_strstr((void *)h, (void *)n);

    return twoway_strstr((void *)h, (void *)n);
}

/// >>> START src/string/strtok.c

char *strtok(char *restrict s, const char *restrict sep) {
    static char *p;
    if (!s && !(s = p)) return NULL;
    s += strspn(s, sep);
    if (!*s) return p = 0;
    p = s + strcspn(s, sep);
    if (*p) *p++ = 0;
    else p = 0;
    return s;
}

/// >>> START src/string/strtok_r.c

char *strtok_r(char *restrict s, const char *restrict sep, char **restrict p) {
    if (!s && !(s = *p)) return NULL;
    s += strspn(s, sep);
    if (!*s) return *p = 0;
    *p = s + strcspn(s, sep);
    if (**p) *(*p)++ = 0;
    else *p = 0;
    return s;
}

/// >>> START src/string/strverscmp.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

int strverscmp(const char *l, const char *r) {
    int haszero=1;
    while (*l==*r) {
        if (!*l) return 0;

        if (*l=='0') {
            if (haszero==1) {
                haszero=0;
            }
        } else if (isdigit(*l)) {
            if (haszero==1) {
                haszero=2;
            }
        } else {
            haszero=1;
        }
        l++; r++;
    }
    if (haszero==1 && (*l=='0' || *r=='0')) {
        haszero=0;
    }
    if ((isdigit(*l) && isdigit(*r) ) && haszero) {
        size_t lenl=0, lenr=0;
        while (isdigit(l[lenl]) ) lenl++;
        while (isdigit(r[lenr]) ) lenr++;
        if (lenl==lenr) {
            return (*l -  *r);
        } else if (lenl>lenr) {
            return 1;
        } else {
            return -1;
        }
    } else {
        return (*l -  *r);
    }
}

/// >>> START src/string/swab.c

void swab(const void *restrict _src, void *restrict _dest, ssize_t n) {
    const char *src = _src;
    char *dest = _dest;
    for (; n>0; n-=2) {
        dest[0] = src[1];
        dest[1] = src[0];
        dest += 2;
        src += 2;
    }
}

/// >>> START src/string/wcpcpy.c

wchar_t *wcpcpy(wchar_t *restrict d, const wchar_t *restrict s) {
    return wcscpy(d, s) + wcslen(s);
}

/// >>> START src/string/wcpncpy.c

wchar_t *wcpncpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n) {
    return wcsncpy(d, s, n) + wcsnlen(s, n);
}

/// >>> START src/string/wcscasecmp.c

int wcscasecmp(const wchar_t *l, const wchar_t *r) {
    return wcsncasecmp(l, r, -1);
}

/// >>> START src/string/wcscasecmp_l.c

int wcscasecmp_l(const wchar_t *l, const wchar_t *r, locale_t locale) {
    return wcscasecmp(l, r);
}

/// >>> START src/string/wcscat.c

wchar_t *wcscat(wchar_t *restrict dest, const wchar_t *restrict src) {
    wcscpy(dest + wcslen(dest), src);
    return dest;
}

/// >>> START src/string/wcschr.c

wchar_t *wcschr(const wchar_t *s, wchar_t c) {
    if (!c) return (wchar_t *)s + wcslen(s);
    for (; *s && *s != c; s++);
    return *s ? (wchar_t *)s : 0;
}

/// >>> START src/string/wcscmp.c

int wcscmp(const wchar_t *l, const wchar_t *r) {
    for (; *l==*r && *l && *r; l++, r++);
    return *l - *r;
}

/// >>> START src/string/wcscpy.c

wchar_t *wcscpy(wchar_t *restrict d, const wchar_t *restrict s) {
    wchar_t *a = d;
    while ((*d++ = *s++));
    return a;
}

/// >>> START src/string/wcscspn.c

size_t wcscspn(const wchar_t *s, const wchar_t *c) {
    const wchar_t *a;
    if (!c[0]) return wcslen(s);
    if (!c[1]) return (s=wcschr(a=s, *c)) ? s-a : wcslen(a);
    for (a=s; *s && !wcschr(c, *s); s++);
    return s-a;
}

/// >>> START src/string/wcsdup.c

wchar_t *wcsdup(const wchar_t *s) {
    size_t l = wcslen(s);
    wchar_t *d = malloc((l+1)*sizeof(wchar_t));
    if (!d) return NULL;
    return wmemcpy(d, s, l+1);
}

/// >>> START src/string/wcslen.c

size_t wcslen(const wchar_t *s) {
    const wchar_t *a;
    for (a=s; *s; s++);
    return s-a;
}

/// >>> START src/string/wcsncasecmp.c

int wcsncasecmp(const wchar_t *l, const wchar_t *r, size_t n) {
    if (!n--) return 0;
    for (; *l && *r && n && (*l == *r || towlower(*l) == towlower(*r)); l++, r++, n--);
    return towlower(*l) - towlower(*r);
}

/// >>> START src/string/wcsncasecmp_l.c

int wcsncasecmp_l(const wchar_t *l, const wchar_t *r, size_t n, locale_t locale) {
    return wcsncasecmp(l, r, n);
}

/// >>> START src/string/wcsncat.c

wchar_t *wcsncat(wchar_t *restrict d, const wchar_t *restrict s, size_t n) {
    wchar_t *a = d;
    d += wcslen(d);
    while (n && *s) n--, *d++ = *s++;
    *d++ = 0;
    return a;
}

/// >>> START src/string/wcsncmp.c

int wcsncmp(const wchar_t *l, const wchar_t *r, size_t n) {
    for (; n && *l==*r && *l && *r; n--, l++, r++);
    return n ? *l - *r : 0;
}

/// >>> START src/string/wcsncpy.c

wchar_t *wcsncpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n) {
    wchar_t *a = d;
    while (n && *s) n--, *d++ = *s++;
    wmemset(d, 0, n);
    return a;
}

/// >>> START src/string/wcsnlen.c

size_t wcsnlen(const wchar_t *s, size_t n) {
    const wchar_t *z = wmemchr(s, 0, n);
    if (z) n = z-s;
    return n;
}

/// >>> START src/string/wcspbrk.c

wchar_t *wcspbrk(const wchar_t *s, const wchar_t *b) {
    s += wcscspn(s, b);
    return *s ? (wchar_t *)s : NULL;
}

/// >>> START src/string/wcsrchr.c

wchar_t *wcsrchr(const wchar_t *s, wchar_t c) {
    const wchar_t *p;
    for (p=s+wcslen(s); p>=s && *p!=c; p--);
    return p>=s ? (wchar_t *)p : 0;
}

/// >>> START src/string/wcsspn.c

size_t wcsspn(const wchar_t *s, const wchar_t *c) {
    const wchar_t *a;
    for (a=s; *s && wcschr(c, *s); s++);
    return s-a;
}

/// >>> START src/string/wcsstr.c

#undef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

static wchar_t *twoway_wcsstr(const wchar_t *h, const wchar_t *n) {
    const wchar_t *z;
    size_t l, ip, jp, wcsstr_c__k, p, ms, p0, mem, mem0;

    /* Computing length of needle */
    for (l=0; n[l] && h[l]; l++);
    if (n[l]) return 0; /* hit the end of h */

    /* Compute maximal suffix */
    ip = -1; jp = 0; wcsstr_c__k = p = 1;
    while (jp+wcsstr_c__k<l) {
        if (n[ip+wcsstr_c__k] == n[jp+wcsstr_c__k]) {
            if (wcsstr_c__k == p) {
                jp += p;
                wcsstr_c__k = 1;
            } else wcsstr_c__k++;
        } else if (n[ip+wcsstr_c__k] > n[jp+wcsstr_c__k]) {
            jp += wcsstr_c__k;
            wcsstr_c__k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            wcsstr_c__k = p = 1;
        }
    }
    ms = ip;
    p0 = p;

    /* And with the opposite comparison */
    ip = -1; jp = 0; wcsstr_c__k = p = 1;
    while (jp+wcsstr_c__k<l) {
        if (n[ip+wcsstr_c__k] == n[jp+wcsstr_c__k]) {
            if (wcsstr_c__k == p) {
                jp += p;
                wcsstr_c__k = 1;
            } else wcsstr_c__k++;
        } else if (n[ip+wcsstr_c__k] < n[jp+wcsstr_c__k]) {
            jp += wcsstr_c__k;
            wcsstr_c__k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            wcsstr_c__k = p = 1;
        }
    }
    if (ip+1 > ms+1) ms = ip;
    else p = p0;

    /* Periodic needle? */
    if (wmemcmp(n, n+p, ms+1)) {
        mem0 = 0;
        p = MAX(ms, l-ms-1) + 1;
    } else mem0 = l-p;
    mem = 0;

    /* Initialize incremental end-of-haystack pointer */
    z = h;

    /* Search loop */
    for (;;) {
        /* Update incremental end-of-haystack pointer */
        if (z-h < l) {
            /* Fast estimate for MIN(l,63) */
            size_t grow = l | 63;
            const wchar_t *z2 = wmemchr(z, 0, grow);
            if (z2) {
                z = z2;
                if (z-h < l) return 0;
            } else z += grow;
        }

        /* Compare right wcsstr_c__half */
        for (wcsstr_c__k=MAX(ms+1,mem); n[wcsstr_c__k] && n[wcsstr_c__k] == h[wcsstr_c__k]; wcsstr_c__k++);
        if (n[wcsstr_c__k]) {
            h += wcsstr_c__k-ms;
            mem = 0;
            continue;
        }
        /* Compare left wcsstr_c__half */
        for (wcsstr_c__k=ms+1; wcsstr_c__k>mem && n[wcsstr_c__k-1] == h[wcsstr_c__k-1]; wcsstr_c__k--);
        if (wcsstr_c__k == mem) return (wchar_t *)h;
        h += p;
        mem = mem0;
    }
}

wchar_t *wcsstr(const wchar_t *restrict h, const wchar_t *restrict n) {
    /* Return immediately on empty needle or haystack */
    if (!n[0]) return (wchar_t *)h;
    if (!h[0]) return 0;

    /* Use faster algorithms for short needles */
    h = wcschr(h, *n);
    if (!h || !n[1]) return (wchar_t *)h;
    if (!h[1]) return 0;

    return twoway_wcsstr(h, n);
}

/// >>> START src/string/wcstok.c

wchar_t *wcstok(wchar_t *restrict s, const wchar_t *restrict sep, wchar_t **restrict p) {
    if (!s && !(s = *p)) return NULL;
    s += wcsspn(s, sep);
    if (!*s) return *p = 0;
    *p = s + wcscspn(s, sep);
    if (**p) *(*p)++ = 0;
    else *p = 0;
    return s;
}

/// >>> START src/string/wcswcs.c

wchar_t *wcswcs(const wchar_t *haystack, const wchar_t *needle) {
    return wcsstr(haystack, needle);
}

/// >>> START src/string/wmemchr.c

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n) {
    for (; n && *s != c; n--, s++);
    return n ? (wchar_t *)s : 0;
}

/// >>> START src/string/wmemcmp.c

int wmemcmp(const wchar_t *l, const wchar_t *r, size_t n) {
    for (; n && *l==*r; n--, l++, r++);
    return n ? *l-*r : 0;
}

/// >>> START src/string/wmemcpy.c

wchar_t *wmemcpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n) {
    wchar_t *a = d;
    while (n--) *d++ = *s++;
    return a;
}

/// >>> START src/string/wmemmove.c

wchar_t *wmemmove(wchar_t *d, const wchar_t *s, size_t n) {
    wchar_t *d0 = d;
    if ((size_t)(d-s) < n)
        while (n--) d[n] = s[n];
    else
        while (n--) *d++ = *s++;
    return d0;
}

/// >>> START src/string/wmemset.c

wchar_t *wmemset(wchar_t *d, wchar_t c, size_t n) {
    wchar_t *ret = d;
    while (n--) *d++ = c;
    return ret;
}

/// >>> START src/temp/__randname.c

int __clock_gettime(clockid_t, struct timespec *);

/* This assumes that a check for the
   template size has already been made */
char *__randname(char *template) {
    int i;
    struct timespec ts;
    unsigned long r;

    __clock_gettime(CLOCK_REALTIME, &ts);
    r = ts.tv_nsec*65537 ^ (uintptr_t)&ts / 16 + (uintptr_t)template;
    for (i=0; i<6; i++, r>>=5)
        template[i] = 'A'+(r&15)+(r&16)*2;

    return template;
}

/// >>> START src/temp/mkdtemp.c

char *__randname(char *);

char *mkdtemp(char *template) {
    size_t l = strlen(template);
    int retries = 100;

    if (l<6 || memcmp(template+l-6, "XXXXXX", 6)) {
        errno = EINVAL;
        return 0;
    }

    do {
        __randname(template+l-6);
        if (!mkdir(template, 0700)) return template;
    } while (--retries && errno == EEXIST);

    memcpy(template+l-6, "XXXXXX", 6);
    return 0;
}

/// >>> START src/temp/mkostemp.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

int __mkostemps(char *, int, int);

int mkostemp(char *template, int flags) {
    return __mkostemps(template, 0, flags);
}

/// >>> START src/temp/mkostemps.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

char *__randname(char *);

int __mkostemps(char *template, int len, int flags) {
    size_t l = strlen(template);
    if (l<6 || len>l-6 || memcmp(template+l-len-6, "XXXXXX", 6)) {
        errno = EINVAL;
        return -1;
    }

    int fd, retries = 100;
    do {
        __randname(template+l-len-6);
        if ((fd = open(template, flags | O_RDWR | O_CREAT | O_EXCL, 0600))>=0)
            return fd;
    } while (--retries && errno == EEXIST);

    memcpy(template+l-len-6, "XXXXXX", 6);
    return -1;
}

weak_alias(__mkostemps, mkostemps);
weak_alias(__mkostemps, mkostemps64);

/// >>> START src/temp/mkstemp.c

int __mkostemps(char *, int, int);

int mkstemp(char *template) {
    return __mkostemps(template, 0, 0);
}

/// >>> START src/temp/mkstemps.c
#undef _BSD_SOURCE
#define _BSD_SOURCE

int __mkostemps(char *, int, int);

int mkstemps(char *template, int len) {
    return __mkostemps(template, len, 0);
}

/// >>> START src/temp/mktemp.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

char *__randname(char *);

char *mktemp(char *template) {
    size_t l = strlen(template);
    int retries = 100;
    struct stat mktemp_c__st;

    if (l < 6 || memcmp(template+l-6, "XXXXXX", 6)) {
        errno = EINVAL;
        *template = 0;
        return template;
    }

    do {
        __randname(template+l-6);
        if (stat(template, &mktemp_c__st)) {
            if (errno != ENOENT) *template = 0;
            return template;
        }
    } while (--retries);

    *template = 0;
    errno = EEXIST;
    return template;
}

/// >>> START src/termios/cfgetospeed.c

speed_t cfgetospeed(const struct termios *tio) {
    return tio->c_cflag & CBAUD;
}

speed_t cfgetispeed(const struct termios *tio) {
    return cfgetospeed(tio);
}

/// >>> START src/termios/cfmakeraw.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t->c_cflag &= ~(CSIZE|PARENB);
    t->c_cflag |= CS8;
    t->c_cc[VMIN] = 1;
    t->c_cc[VTIME] = 0;
}

/// >>> START src/termios/cfsetospeed.c

int cfsetospeed(struct termios *tio, speed_t speed) {
    if (speed & ~CBAUD) {
        errno = EINVAL;
        return -1;
    }
    tio->c_cflag &= ~CBAUD;
    tio->c_cflag |= speed;
    return 0;
}

int cfsetispeed(struct termios *tio, speed_t speed) {
    return speed ? cfsetospeed(tio, speed) : 0;
}

weak_alias(cfsetospeed, cfsetspeed);

/// >>> START src/termios/tcdrain.c

int tcdrain(int fd) {
    return syscall_cp(SYS_ioctl, fd, TCSBRK, 1);
}

/// >>> START src/termios/tcflow.c

int tcflow(int fd, int action) {
    return ioctl(fd, TCXONC, action);
}

/// >>> START src/termios/tcflush.c

int tcflush(int fd, int queue) {
    return ioctl(fd, TCFLSH, queue);
}

/// >>> START src/termios/tcgetattr.c

int tcgetattr(int fd, struct termios *tio) {
    if (ioctl(fd, TCGETS, tio))
        return -1;
    return 0;
}

/// >>> START src/termios/tcgetsid.c

pid_t tcgetsid(int fd) {
    int sid;
    if (ioctl(fd, TIOCGSID, &sid) < 0)
        return -1;
    return sid;
}

/// >>> START src/termios/tcsendbreak.c

int tcsendbreak(int fd, int dur) {
    /* nonzero duration is implementation-defined, so ignore it */
    return ioctl(fd, TCSBRK, 0);
}

/// >>> START src/termios/tcsetattr.c

int tcsetattr(int fd, int act, const struct termios *tio) {
    if (act < 0 || act > 2) {
        errno = EINVAL;
        return -1;
    }
    return ioctl(fd, TCSETS+act, tio);
}

/// >>> START src/time/__asctime.c

char *__nl_langinfo(nl_item);

char *__asctime(const struct tm *restrict tm, char *restrict buf) {
    /* FIXME: change __nl_langinfo to __nl_langinfo_l with explicit C
     * locale once we have locales */
    if (snprintf(buf, 26, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
        __nl_langinfo(ABDAY_1+tm->tm_wday),
        __nl_langinfo(ABMON_1+tm->tm_mon),
        tm->tm_mday, tm->tm_hour,
        tm->tm_min, tm->tm_sec,
        1900 + tm->tm_year) >= 26) {
        /* ISO C requires us to use the above format string,
         * even if it will not fit in the buffer. Thus asctime_r
         * is _supposed_ to crash if the fields in tm are too large.
         * We follow this behavior and crash "gracefully" to warn
         * application developers that they may not be so lucky
         * on other implementations (e.g. stack smashing..).
         */
        a_crash();
    }
    return buf;
}

/// >>> START src/time/__map_file.c

void *__mmap(void *, size_t, int, int, int, off_t);

const char unsigned *__map_file(const char *pathname, size_t *size) {
    struct stat __map_file_c__st;
    const unsigned char *map = MAP_FAILED;
    int flags = O_RDONLY|O_LARGEFILE|O_CLOEXEC|O_NONBLOCK;
    int fd = __syscall(SYS_open, pathname, flags);
    if (fd < 0) return 0;
    if (!__syscall(SYS_fstat, fd, &__map_file_c__st))
        map = __mmap(0, __map_file_c__st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    __syscall(SYS_close, fd);
    *size = __map_file_c__st.st_size;
    return map == MAP_FAILED ? 0 : map;
}

/// >>> START src/time/__month_to_secs.c
int __month_to_secs(int month, int is_leap) {
    static const int secs_through_month[] = {
        0, 31*86400, 59*86400, 90*86400,
        120*86400, 151*86400, 181*86400, 212*86400,
        243*86400, 273*86400, 304*86400, 334*86400 };
    int t = secs_through_month[month];
    if (is_leap && month >= 2) t+=86400;
    return t;
}

/// >>> START src/time/__secs_to_tm.c
/// >>> START src/time/time_impl.h

int __days_in_month(int, int);
int __month_to_secs(int, int);
long long __year_to_secs(long long, int *);
long long __tm_to_secs(const struct tm *);
int __secs_to_tm(long long, struct tm *);
void __secs_to_zone(long long, int, int *, long *, long *, const char **);
const unsigned char *__map_file(const char *, size_t *);

/// >>> CONTINUE src/time/__secs_to_tm.c

/* 2000-03-01 (mod 400 year, immediately after feb29 */
#undef LEAPOCH
#define LEAPOCH (946684800LL + 86400*(31+29))

#undef DAYS_PER_400Y
#define DAYS_PER_400Y (365*400 + 97)
#undef DAYS_PER_100Y
#define DAYS_PER_100Y (365*100 + 24)
#undef DAYS_PER_4Y
#define DAYS_PER_4Y   (365*4   + 1)

int __secs_to_tm(long long t, struct tm *tm) {
    long long days, secs;
    int remdays, remsecs, remyears;
    int qc_cycles, c_cycles, q_cycles;
    int years, months;
    int wday, yday, leap;
    static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

    /* Reject time_t values whose year would overflow int */
    if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
        return -1;

    secs = t - LEAPOCH;
    days = secs / 86400;
    remsecs = secs % 86400;
    if (remsecs < 0) {
        remsecs += 86400;
        days--;
    }

    wday = (3+days)%7;
    if (wday < 0) wday += 7;

    qc_cycles = days / DAYS_PER_400Y;
    remdays = days % DAYS_PER_400Y;
    if (remdays < 0) {
        remdays += DAYS_PER_400Y;
        qc_cycles--;
    }

    c_cycles = remdays / DAYS_PER_100Y;
    if (c_cycles == 4) c_cycles--;
    remdays -= c_cycles * DAYS_PER_100Y;

    q_cycles = remdays / DAYS_PER_4Y;
    if (q_cycles == 25) q_cycles--;
    remdays -= q_cycles * DAYS_PER_4Y;

    remyears = remdays / 365;
    if (remyears == 4) remyears--;
    remdays -= remyears * 365;

    leap = !remyears && (q_cycles || !c_cycles);
    yday = remdays + 31 + 28 + leap;
    if (yday >= 365+leap) yday -= 365+leap;

    years = remyears + 4*q_cycles + 100*c_cycles + 400*qc_cycles;

    for (months=0; days_in_month[months] <= remdays; months++)
        remdays -= days_in_month[months];

    if (years+100 > INT_MAX || years+100 < INT_MIN)
        return -1;

    tm->tm_year = years + 100;
    tm->tm_mon = months + 2;
    if (tm->tm_mon >= 12) {
        tm->tm_mon -=12;
        tm->tm_year++;
    }
    tm->tm_mday = remdays + 1;
    tm->tm_wday = wday;
    tm->tm_yday = yday;

    tm->tm_hour = remsecs / 3600;
    tm->tm_min = remsecs / 60 % 60;
    tm->tm_sec = remsecs % 60;

    return 0;
}

/// >>> START src/time/__tm_to_secs.c

long long __tm_to_secs(const struct tm *tm) {
    int is_leap;
    long long year = tm->tm_year;
    int month = tm->tm_mon;
    if (month >= 12 || month < 0) {
        int adj = month / 12;
        month %= 12;
        if (month < 0) {
            adj--;
            month += 12;
        }
        year += adj;
    }
    long long t = __year_to_secs(year, &is_leap);
    t += __month_to_secs(month, is_leap);
    t += 86400LL * (tm->tm_mday-1);
    t += 3600LL * tm->tm_hour;
    t += 60LL * tm->tm_min;
    t += tm->tm_sec;
    return t;
}

/// >>> START src/time/__year_to_secs.c
long long __year_to_secs(long long year, int *is_leap) {
    if (year-2ULL <= 136) {
        int y = year;
        int leaps = (y-68)>>2;
        if (!((y-68)&3)) {
            leaps--;
            if (is_leap) *is_leap = 1;
        } else if (is_leap) *is_leap = 0;
        return 31536000*(y-70) + 86400*leaps;
    }

    int cycles, centuries, leaps, rem;

    if (!is_leap) is_leap = &(int){0};
    cycles = (year-100) / 400;
    rem = (year-100) % 400;
    if (rem < 0) {
        cycles--;
        rem += 400;
    }
    if (!rem) {
        *is_leap = 1;
        centuries = 0;
        leaps = 0;
    } else {
        if (rem >= 200) {
            if (rem >= 300) centuries = 3, rem -= 300;
            else centuries = 2, rem -= 200;
        } else {
            if (rem >= 100) centuries = 1, rem -= 100;
            else centuries = 0;
        }
        if (!rem) {
            *is_leap = 0;
            leaps = 0;
        } else {
            leaps = rem / 4U;
            rem %= 4U;
            *is_leap = !rem;
        }
    }

    leaps += 97*cycles + 24*centuries - *is_leap;

    return (year-100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}

/// >>> START src/time/asctime.c

char *__asctime(const struct tm *, char *);

char *asctime(const struct tm *tm) {
    static char buf[26];
    return __asctime(tm, buf);
}

/// >>> START src/time/asctime_r.c

char *__asctime(const struct tm *restrict, char *restrict);

char *asctime_r(const struct tm *restrict tm, char *restrict buf) {
    return __asctime(tm, buf);
}

/// >>> START src/time/clock.c

int __clock_gettime(clockid_t, struct timespec *);

clock_t clock() {
    struct timespec ts;

    if (__clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts))
        return -1;

    if (ts.tv_sec > LONG_MAX/1000000
     || ts.tv_nsec/1000 > LONG_MAX-1000000*ts.tv_sec)
        return -1;

    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

/// >>> START src/time/clock_getcpuclockid.c

int clock_getcpuclockid(pid_t pid, clockid_t *clk) {
    struct timespec ts;
    clockid_t id = (-pid-1)*8U + 2;
    int ret = __syscall(SYS_clock_getres, id, &ts);
    if (ret) return -ret;
    *clk = id;
    return 0;
}

/// >>> START src/time/clock_gettime.c

static int sc_clock_gettime(clockid_t clk, struct timespec *ts) {
    int r = __syscall(SYS_clock_gettime, clk, ts);
    if (!r) return r;
    if (r == -ENOSYS) {
        if (clk == CLOCK_REALTIME) {
            __syscall(SYS_gettimeofday, clk, ts, 0);
            ts->tv_nsec = (int)ts->tv_nsec * 1000;
            return 0;
        }
        r = -EINVAL;
    }
    errno = -r;
    return -1;
}

weak_alias(sc_clock_gettime, __vdso_clock_gettime);

int (*__cgt)(clockid_t, struct timespec *) = __vdso_clock_gettime;

int __clock_gettime(clockid_t clk, struct timespec *ts) {
    /* Conditional is to make this work prior to dynamic linking */
    return __cgt ? __cgt(clk, ts) : sc_clock_gettime(clk, ts);
}

weak_alias(__clock_gettime, clock_gettime);

/// >>> START src/time/ctime.c

char *ctime(const time_t *t) {
    return asctime(localtime(t));
}

/// >>> START src/time/ctime_r.c

char *ctime_r(const time_t *t, char *buf) {
    struct tm tm;
    localtime_r(t, &tm);
    return asctime_r(&tm, buf);
}

/// >>> START src/time/difftime.c

double difftime(time_t difftime_c__t1, time_t difftime_c__t0) {
    return difftime_c__t1-difftime_c__t0;
}

/// >>> START src/time/ftime.c

int ftime(struct timeb *tp) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    tp->time = ts.tv_sec;
    tp->millitm = ts.tv_nsec / 1000000;
    tp->timezone = tp->dstflag = 0;
    return 0;
}

/// >>> START src/time/getdate.c

int getdate_err;

struct tm *getdate(const char *s) {
    static struct tm tmbuf;
    struct tm *ret = 0;
    char *datemsk = getenv("DATEMSK");
    FILE *f = 0;
    char fmt[100], *p;
    int cs;

    pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, &cs);

    if (!datemsk) {
        getdate_err = 1;
        goto out;
    }

    f = fopen(datemsk, "rbe");
    if (!f) {
        if (errno == ENOMEM) getdate_err = 6;
        else getdate_err = 2;
        goto out;
    }

    while (fgets(fmt, sizeof fmt, f)) {
        p = strptime(s, fmt, &tmbuf);
        if (p && !*p) {
            ret = &tmbuf;
            goto out;
        }
    }

    getdate_err = 7;
out:
    if (f) fclose(f);
    pthread_setcancelstate(cs, 0);
    return ret;
}

/// >>> START src/time/gmtime.c

struct tm *__gmtime_r(const time_t *restrict, struct tm *restrict);

struct tm *gmtime(const time_t *t) {
    static struct tm tm;
    return __gmtime_r(t, &tm);
}

/// >>> START src/time/gmtime_r.c

extern const char __gmt[];

struct tm *__gmtime_r(const time_t *restrict t, struct tm *restrict tm) {
    if (__secs_to_tm(*t, tm) < 0) {
        errno = EOVERFLOW;
        return 0;
    }
    tm->tm_isdst = 0;
    tm->__tm_gmtoff = 0;
    tm->__tm_zone = __gmt;
    return tm;
}

weak_alias(__gmtime_r, gmtime_r);

/// >>> START src/time/localtime.c

struct tm *__localtime_r(const time_t *restrict, struct tm *restrict);

struct tm *localtime(const time_t *t) {
    static struct tm tm;
    return __localtime_r(t, &tm);
}

/// >>> START src/time/strptime.c

char *strptime(const char *restrict s, const char *restrict f, struct tm *restrict tm) {
    int i, w, neg, adj, min, range, *dest;
    const char *ex;
    size_t len;
    while (*f) {
        if (*f != '%') {
            if (isspace(*f)) for (; *s && isspace(*s); s++);
            else if (*s != *f) return 0;
            else s++;
            f++;
            continue;
        }
        f++;
        if (*f == '+') f++;
        if (isdigit(*f)) w=strtoul(f, (void *)&f, 10);
        else w=-1;
        adj=0;
        switch (*f++) {
        case 'a': case 'A':
            dest = &tm->tm_wday;
            min = ABDAY_1;
            range = 7;
            goto symbolic_range;
        case 'b': case 'B': case 'h':
            dest = &tm->tm_mon;
            min = ABMON_1;
            range = 12;
            goto symbolic_range;
        case 'c':
            s = strptime(s, nl_langinfo(D_T_FMT), tm);
            if (!s) return 0;
            break;
        case 'C':
        case 'd': case 'e':
            dest = &tm->tm_mday;
            min = 1;
            range = 31;
            goto numeric_range;
        case 'D':
            s = strptime(s, "%m/%d/%y", tm);
            if (!s) return 0;
            break;
        case 'H':
            dest = &tm->tm_hour;
            min = 0;
            range = 24;
            goto numeric_range;
        case 'I':
            dest = &tm->tm_hour;
            min = 1;
            range = 12;
            goto numeric_range;
        case 'j':
            dest = &tm->tm_yday;
            min = 1;
            range = 366;
            goto numeric_range;
        case 'm':
            dest = &tm->tm_mon;
            min = 1;
            range = 12;
            adj = 1;
            goto numeric_range;
        case 'M':
            dest = &tm->tm_min;
            min = 0;
            range = 60;
            goto numeric_range;
        case 'n': case 't':
            for (; *s && isspace(*s); s++);
            break;
        case 'p':
            ex = nl_langinfo(AM_STR);
            len = strlen(ex);
            if (!strncasecmp(s, ex, len)) {
                tm->tm_hour %= 12;
                break;
            }
            ex = nl_langinfo(PM_STR);
            len = strlen(ex);
            if (!strncasecmp(s, ex, len)) {
                tm->tm_hour %= 12;
                tm->tm_hour += 12;
                break;
            }
            return 0;
        case 'r':
            s = strptime(s, nl_langinfo(T_FMT_AMPM), tm);
            if (!s) return 0;
            break;
        case 'R':
            s = strptime(s, "%H:%M", tm);
            if (!s) return 0;
            break;
        case 'S':
            dest = &tm->tm_sec;
            min = 0;
            range = 61;
            goto numeric_range;
        case 'T':
            s = strptime(s, "%H:%M:%strptime_c__S", tm);
            if (!s) return 0;
            break;
        case 'U':
        case 'W':
            //FIXME
            return 0;
        case 'w':
            dest = &tm->tm_wday;
            min = 0;
            range = 7;
            goto numeric_range;
        case 'x':
            s = strptime(s, nl_langinfo(D_FMT), tm);
            if (!s) return 0;
            break;
        case 'X':
            s = strptime(s, nl_langinfo(T_FMT), tm);
            if (!s) return 0;
            break;
        case 'y':
            //FIXME
            return 0;
        case 'Y':
            dest = &tm->tm_year;
            if (w<0) w=4;
            adj = 1900;
            goto numeric_digits;
        case '%':
            if (*s++ != '%') return 0;
            break;
        numeric_range:
            if (!isdigit(*s)) return 0;
            *dest = 0;
            for (i=1; i<=min+range && isdigit(*s); i*=10)
                *dest = *dest * 10 + *s++ - '0';
            if (*dest - min >= (unsigned)range) return 0;
            *dest -= adj;
            switch((char *)dest - (char *)tm) {
            case offsetof(struct tm, tm_yday):
                ;
            }
            goto update;
        numeric_digits:
            neg = 0;
            if (*s == '+') s++;
            else if (*s == '-') neg=1, s++;
            if (!isdigit(*s)) return 0;
            for (*dest=i=0; i<w && isdigit(*s); i++)
                *dest = *dest * 10 + *s++ - '0';
            if (neg) *dest = -*dest;
            *dest -= adj;
            goto update;
        symbolic_range:
            for (i=2*range-1; i>=0; i--) {
                ex = nl_langinfo(min+i);
                len = strlen(ex);
                if (strncasecmp(s, ex, len)) continue;
                s += len;
                *dest = i % range;
                break;
            }
            if (i<0) return 0;
            goto update;
        update:
            //FIXME
            ;
        }
    }
    return (char *)s;
}

/// >>> START src/time/timegm.c
#undef _GNU_SOURCE
#define _GNU_SOURCE

extern const char __gmt[];

time_t timegm(struct tm *tm) {
    struct tm new;
    long long t = __tm_to_secs(tm);
    if (__secs_to_tm(t, &new) < 0) {
        errno = EOVERFLOW;
        return -1;
    }
    *tm = new;
    tm->tm_isdst = 0;
    tm->__tm_gmtoff = 0;
    tm->__tm_zone = __gmt;
    return t;
}

/// >>> START src/stdio/fprintf.c

int fprintf(FILE *restrict f, const char *restrict fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vfprintf(f, fmt, ap);
    va_end(ap);
    return ret;
}

/// >>> START src/stdio/printf.c

int printf(const char *restrict fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return ret;
}

/// >>> START src/stdio/snprintf.c

int snprintf(char *restrict s, size_t n, const char *restrict fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsnprintf(s, n, fmt, ap);
    va_end(ap);
    return ret;
}

/// >>> START src/stdio/sprintf.c

int sprintf(char *restrict s, const char *restrict fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsprintf(s, fmt, ap);
    va_end(ap);
    return ret;
}

/// >>> START src/stdio/tempnam.c

#undef MAXTRIES
#define MAXTRIES 100

char *tempnam(const char *dir, const char *pfx) {
    static int index;
    char *s;
    struct timespec ts;
    int pid = getpid();
    size_t l;
    int n;
    int tempnam_c__try=0;

    if (!dir) dir = P_tmpdir;
    if (!pfx) pfx = "temp";

    if (access(dir, R_OK|W_OK|X_OK) != 0)
        return NULL;

    l = strlen(dir) + 1 + strlen(pfx) + 3*(sizeof(int)*3+2) + 1;
    s = malloc(l);
    if (!s) return s;

    do {
        clock_gettime(CLOCK_REALTIME, &ts);
        n = ts.tv_nsec ^ (uintptr_t)&s ^ (uintptr_t)s;
        snprintf(s, l, "%s/%s-%d-%d-%x", dir, pfx, pid, a_fetch_add(&index, 1), n);
    } while (!access(s, F_OK) && tempnam_c__try++<MAXTRIES);
    if (tempnam_c__try>=MAXTRIES) {
        free(s);
        return 0;
    }
    return s;
}

/// >>> START src/stdio/tmpfile.c

#undef MAXTRIES
#define MAXTRIES 100

FILE *tmpfile(void) {
    char buf[L_tmpnam], *s;
    int fd;
    FILE *f;
    int tmpfile_c__try;
    for (tmpfile_c__try=0; tmpfile_c__try<MAXTRIES; tmpfile_c__try++) {
        s = tmpnam(buf);
        if (!s) return 0;
        fd = syscall(SYS_open, s, O_RDWR|O_CREAT|O_EXCL|O_LARGEFILE|O_CLOEXEC, 0600);
        if (fd >= 0) {
            f = fdopen(fd, "w+");
            __syscall(SYS_unlink, s);
            return f;
        }
    }
    return 0;
}

/// >>> START src/stdio/tmpnam.c

#undef MAXTRIES
#define MAXTRIES 100

char *tmpnam(char *s) {
    static int index;
    static char tmpnam_c__s2[L_tmpnam];
    struct timespec ts;
    int tmpnam_c__try = 0;
    unsigned n;

    if (!s) s = tmpnam_c__s2;

    if (__syscall(SYS_access, P_tmpdir, R_OK|W_OK|X_OK) != 0)
        return NULL;

    do {
        __syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts, 0);
        n = ts.tv_nsec ^ (uintptr_t)&s ^ (uintptr_t)s;
        snprintf(s, L_tmpnam, "/tmp/t%x-%x", a_fetch_add(&index, 1), n);
    } while (!__syscall(SYS_access, s, F_OK) && tmpnam_c__try++<MAXTRIES);
    return tmpnam_c__try>=MAXTRIES ? 0 : s;
}

/// >>> START src/stdio/vprintf.c

int vprintf(const char *restrict fmt, va_list ap) {
    return vfprintf(stdout, fmt, ap);
}

/// >>> START src/stdio/vsprintf.c

int vsprintf(char *restrict s, const char *restrict fmt, va_list ap) {
    return vsnprintf(s, INT_MAX, fmt, ap);
}

/// >>> START src/unistd/isatty.c

int isatty(int fd) {
    struct termios t;
    return tcgetattr(fd, &t) == 0;
}

/// >>> START src/unistd/ttyname.c

char *ttyname(int fd) {
    static char buf[TTY_NAME_MAX];
    int result;
    if ((result = ttyname_r(fd, buf, sizeof buf))) {
        errno = result;
        return NULL;
    }
    return buf;
}

/// >>> START src/unistd/ttyname_r.c

void __procfdname(char *, unsigned);

int ttyname_r(int fd, char *name, size_t size) {
    char procname[sizeof "/proc/self/fd/" + 3*sizeof(int) + 2];
    ssize_t l;

    if (!isatty(fd)) return ENOTTY;

    __procfdname(procname, fd);
    l = readlink(procname, name, size);

    if (l < 0) return errno;
    else if (l == size) return ERANGE;
    else {
        name[l] = 0;
        return 0;
    }
}

/// >>> START src/internal/syscall.c

/// >>> START src/internal/syscall_ret.c

long __syscall_ret(unsigned long r) {
    if (r > -4096UL) {
        errno = -r;
        return -1;
    }
    return r;
}

/// >>> START src/internal/procfdname.c
void __procfdname(char *buf, unsigned fd) {
    unsigned i, j;
    for (i=0; (buf[i] = "/proc/self/fd/"[i]); i++);
    if (!fd) {
        buf[i] = '0';
        buf[i+1] = 0;
        return;
    }
    for (j=fd; j; j/=10, i++);
    buf[i] = 0;
    for (; fd; fd/=10) buf[--i] = '0' + fd%10;
}

/// >>> START src/exit/abort.c

_Noreturn void abort(void) {
    raise(SIGABRT);
    raise(SIGKILL);
    for (;;);
}

/// >>> START src/linux/prctl.c

int prctl(int op, ...) {
    unsigned long x[4];
    int i;
    va_list ap;
    va_start(ap, op);
    for (i=0; i<4; i++) x[i] = va_arg(ap, unsigned long);
    va_end(ap);
    return syscall(SYS_prctl, op, x[0], x[1], x[2], x[3]);
}

/// >>> START src/prng/rand.c

static uint64_t seed;

void srand(unsigned s) {
    seed = s-1;
}

int rand(void) {
    seed = 6364136223846793005ULL*seed + 1;
    return seed>>33;
}

/// >>> START src/prng/rand_r.c

static unsigned temper(unsigned x) {
    x ^= x>>11;
    x ^= x<<7 & 0x9D2C5680;
    x ^= x<<15 & 0xEFC60000;
    x ^= x>>18;
    return x;
}

int rand_r(unsigned *seed) {
    return temper(*seed = *seed * 1103515245 + 12345)/2;
}

/// END musl code

#undef __syscall_cp

long __syscall_cp(long n, long a, long b, long c, long d, long e, long f) {
    return __syscall(n, a, b, c, d, e, f);
}

#undef syscall

long syscall(long n, ...) {
    va_list ap;
    long a, b, c, d, e, f;
    va_start(ap, n);
    a = va_arg(ap, long);
    b = va_arg(ap, long);
    c = va_arg(ap, long);
    d = va_arg(ap, long);
    e = va_arg(ap, long);
    f = va_arg(ap, long);
    va_end(ap);
    return __syscall_ret(__syscall(n, a, b, c, d, e, f));
}

#undef __syscall

weak_alias(syscall, __syscall);
