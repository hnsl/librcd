#include "crt.h"

/*===-- absvdi2.c - Implement __absvdi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===
 *
 * This file implements __absvdi2 for the compiler_rt library.
 *
 *===----------------------------------------------------------------------===
 */


/* Returns: absolute value */

/* Effects: aborts if abs(x) < 0 */

COMPILER_RT_ABI di_int
__absvdi2(di_int a)
{
    const int N = (int)(sizeof(di_int) * CHAR_BIT);
    if (a == ((di_int)1 << (N-1)))
        compilerrt_abort();
    const di_int t = a >> (N - 1);
    return (a ^ t) - t;
}
/* ===-- absvsi2.c - Implement __absvsi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __absvsi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: absolute value */

/* Effects: aborts if abs(x) < 0 */

COMPILER_RT_ABI si_int
__absvsi2(si_int a)
{
    const int N = (int)(sizeof(si_int) * CHAR_BIT);
    if (a == (1 << (N-1)))
        compilerrt_abort();
    const si_int t = a >> (N - 1);
    return (a ^ t) - t;
}
/* ===-- absvti2.c - Implement __absvdi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __absvti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: absolute value */

/* Effects: aborts if abs(x) < 0 */

ti_int
__absvti2(ti_int a)
{
    const int N = (int)(sizeof(ti_int) * CHAR_BIT);
    if (a == ((ti_int)1 << (N-1)))
        compilerrt_abort();
    const ti_int s = a >> (N - 1);
    return (a ^ s) - s;
}

#endif
//===-- lib/adddf3.c - Double-precision addition ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float addition with the IEEE-754
// default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

ARM_EABI_FNALIAS(dadd, adddf3)

COMPILER_RT_ABI fp_t
__adddf3(fp_t a, fp_t b) {

    rep_t aRep = toRep(a);
    rep_t bRep = toRep(b);
    const rep_t aAbs = aRep & absMask;
    const rep_t bAbs = bRep & absMask;

    // Detect if a or b is zero, infinity, or NaN.
    if (aAbs - 1U >= infRep - 1U || bAbs - 1U >= infRep - 1U) {

        // NaN + anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything + NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // +/-infinity + -/+infinity = qNaN
            if ((toRep(a) ^ toRep(b)) == signBit) return fromRep(qnanRep);
            // +/-infinity + anything remaining = +/- infinity
            else return a;
        }

        // anything remaining + +/-infinity = +/-infinity
        if (bAbs == infRep) return b;

        // zero + anything = anything
        if (!aAbs) {
            // but we need to get the sign right for zero + zero
            if (!bAbs) return fromRep(toRep(a) & toRep(b));
            else return b;
        }

        // anything + zero = anything
        if (!bAbs) return a;
    }

    // Swap a and b if necessary so that a has the larger absolute value.
    if (bAbs > aAbs) {
        const rep_t temp = aRep;
        aRep = bRep;
        bRep = temp;
    }

    // Extract the exponent and significand from the (possibly swapped) a and b.
    int aExponent = aRep >> significandBits & maxExponent;
    int bExponent = bRep >> significandBits & maxExponent;
    rep_t aSignificand = aRep & significandMask;
    rep_t bSignificand = bRep & significandMask;

    // Normalize any denormals, and adjust the exponent accordingly.
    if (aExponent == 0) aExponent = normalize(&aSignificand);
    if (bExponent == 0) bExponent = normalize(&bSignificand);

    // The sign of the result is the sign of the larger operand, a.  If they
    // have opposite signs, we are performing a subtraction; otherwise addition.
    const rep_t resultSign = aRep & signBit;
    const bool subtraction = (aRep ^ bRep) & signBit;

    // Shift the significands to give us round, guard and sticky, and or in the
    // implicit significand bit.  (If we fell through from the denormal path it
    // was already set by normalize( ), but setting it twice won't hurt
    // anything.)
    aSignificand = (aSignificand | implicitBit) << 3;
    bSignificand = (bSignificand | implicitBit) << 3;

    // Shift the significand of b by the difference in exponents, with a sticky
    // bottom bit to get rounding correct.
    const unsigned int align = aExponent - bExponent;
    if (align) {
        if (align < typeWidth) {
            const bool sticky = bSignificand << (typeWidth - align);
            bSignificand = bSignificand >> align | sticky;
        } else {
            bSignificand = 1; // sticky; b is known to be non-zero.
        }
    }

    if (subtraction) {
        aSignificand -= bSignificand;

        // If a == -b, return +zero.
        if (aSignificand == 0) return fromRep(0);

        // If partial cancellation occured, we need to left-shift the result
        // and adjust the exponent:
        if (aSignificand < implicitBit << 3) {
            const int shift = rep_clz(aSignificand) - rep_clz(implicitBit << 3);
            aSignificand <<= shift;
            aExponent -= shift;
        }
    }

    else /* addition */ {
        aSignificand += bSignificand;

        // If the addition carried up, we need to right-shift the result and
        // adjust the exponent:
        if (aSignificand & implicitBit << 4) {
            const bool sticky = aSignificand & 1;
            aSignificand = aSignificand >> 1 | sticky;
            aExponent += 1;
        }
    }

    // If we have overflowed the type, return +/- infinity:
    if (aExponent >= maxExponent) return fromRep(infRep | resultSign);

    if (aExponent <= 0) {
        // Result is denormal before rounding; the exponent is zero and we
        // need to shift the significand.
        const int shift = 1 - aExponent;
        const bool sticky = aSignificand << (typeWidth - shift);
        aSignificand = aSignificand >> shift | sticky;
        aExponent = 0;
    }

    // Low three bits are round, guard, and sticky.
    const int roundGuardSticky = aSignificand & 0x7;

    // Shift the significand into place, and mask off the implicit bit.
    rep_t result = aSignificand >> 3 & significandMask;

    // Insert the exponent and sign.
    result |= (rep_t)aExponent << significandBits;
    result |= resultSign;

    // Final rounding.  The result may overflow to infinity, but that is the
    // correct result in that case.
    if (roundGuardSticky > 0x4) result++;
    if (roundGuardSticky == 0x4) result += result & 1;
    return fromRep(result);
}
//===-- lib/addsf3.c - Single-precision addition ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision soft-float addition with the IEEE-754
// default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

ARM_EABI_FNALIAS(fadd, addsf3)

fp_t __addsf3(fp_t a, fp_t b) {

    rep_t aRep = toRep(a);
    rep_t bRep = toRep(b);
    const rep_t aAbs = aRep & absMask;
    const rep_t bAbs = bRep & absMask;

    // Detect if a or b is zero, infinity, or NaN.
    if (aAbs - 1U >= infRep - 1U || bAbs - 1U >= infRep - 1U) {

        // NaN + anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything + NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // +/-infinity + -/+infinity = qNaN
            if ((toRep(a) ^ toRep(b)) == signBit) return fromRep(qnanRep);
            // +/-infinity + anything remaining = +/- infinity
            else return a;
        }

        // anything remaining + +/-infinity = +/-infinity
        if (bAbs == infRep) return b;

        // zero + anything = anything
        if (!aAbs) {
            // but we need to get the sign right for zero + zero
            if (!bAbs) return fromRep(toRep(a) & toRep(b));
            else return b;
        }

        // anything + zero = anything
        if (!bAbs) return a;
    }

    // Swap a and b if necessary so that a has the larger absolute value.
    if (bAbs > aAbs) {
        const rep_t temp = aRep;
        aRep = bRep;
        bRep = temp;
    }

    // Extract the exponent and significand from the (possibly swapped) a and b.
    int aExponent = aRep >> significandBits & maxExponent;
    int bExponent = bRep >> significandBits & maxExponent;
    rep_t aSignificand = aRep & significandMask;
    rep_t bSignificand = bRep & significandMask;

    // Normalize any denormals, and adjust the exponent accordingly.
    if (aExponent == 0) aExponent = normalize(&aSignificand);
    if (bExponent == 0) bExponent = normalize(&bSignificand);

    // The sign of the result is the sign of the larger operand, a.  If they
    // have opposite signs, we are performing a subtraction; otherwise addition.
    const rep_t resultSign = aRep & signBit;
    const bool subtraction = (aRep ^ bRep) & signBit;

    // Shift the significands to give us round, guard and sticky, and or in the
    // implicit significand bit.  (If we fell through from the denormal path it
    // was already set by normalize( ), but setting it twice won't hurt
    // anything.)
    aSignificand = (aSignificand | implicitBit) << 3;
    bSignificand = (bSignificand | implicitBit) << 3;

    // Shift the significand of b by the difference in exponents, with a sticky
    // bottom bit to get rounding correct.
    const unsigned int align = aExponent - bExponent;
    if (align) {
        if (align < typeWidth) {
            const bool sticky = bSignificand << (typeWidth - align);
            bSignificand = bSignificand >> align | sticky;
        } else {
            bSignificand = 1; // sticky; b is known to be non-zero.
        }
    }

    if (subtraction) {
        aSignificand -= bSignificand;

        // If a == -b, return +zero.
        if (aSignificand == 0) return fromRep(0);

        // If partial cancellation occured, we need to left-shift the result
        // and adjust the exponent:
        if (aSignificand < implicitBit << 3) {
            const int shift = rep_clz(aSignificand) - rep_clz(implicitBit << 3);
            aSignificand <<= shift;
            aExponent -= shift;
        }
    }

    else /* addition */ {
        aSignificand += bSignificand;

        // If the addition carried up, we need to right-shift the result and
        // adjust the exponent:
        if (aSignificand & implicitBit << 4) {
            const bool sticky = aSignificand & 1;
            aSignificand = aSignificand >> 1 | sticky;
            aExponent += 1;
        }
    }

    // If we have overflowed the type, return +/- infinity:
    if (aExponent >= maxExponent) return fromRep(infRep | resultSign);

    if (aExponent <= 0) {
        // Result is denormal before rounding; the exponent is zero and we
        // need to shift the significand.
        const int shift = 1 - aExponent;
        const bool sticky = aSignificand << (typeWidth - shift);
        aSignificand = aSignificand >> shift | sticky;
        aExponent = 0;
    }

    // Low three bits are round, guard, and sticky.
    const int roundGuardSticky = aSignificand & 0x7;

    // Shift the significand into place, and mask off the implicit bit.
    rep_t result = aSignificand >> 3 & significandMask;

    // Insert the exponent and sign.
    result |= (rep_t)aExponent << significandBits;
    result |= resultSign;

    // Final rounding.  The result may overflow to infinity, but that is the
    // correct result in that case.
    if (roundGuardSticky > 0x4) result++;
    if (roundGuardSticky == 0x4) result += result & 1;
    return fromRep(result);
}
/* ===-- addvdi3.c - Implement __addvdi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __addvdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a + b */

/* Effects: aborts if a + b overflows */

COMPILER_RT_ABI di_int
__addvdi3(di_int a, di_int b)
{
    di_int s = a + b;
    if (b >= 0)
    {
        if (s < a)
            compilerrt_abort();
    }
    else
    {
        if (s >= a)
            compilerrt_abort();
    }
    return s;
}
/* ===-- addvsi3.c - Implement __addvsi3 -----------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __addvsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a + b */

/* Effects: aborts if a + b overflows */

COMPILER_RT_ABI si_int
__addvsi3(si_int a, si_int b)
{
    si_int s = a + b;
    if (b >= 0)
    {
        if (s < a)
            compilerrt_abort();
    }
    else
    {
        if (s >= a)
            compilerrt_abort();
    }
    return s;
}
/* ===-- addvti3.c - Implement __addvti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __addvti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: a + b */

/* Effects: aborts if a + b overflows */

ti_int
__addvti3(ti_int a, ti_int b)
{
    ti_int s = a + b;
    if (b >= 0)
    {
        if (s < a)
            compilerrt_abort();
    }
    else
    {
        if (s >= a)
            compilerrt_abort();
    }
    return s;
}

#endif
/* ====-- ashldi3.c - Implement __ashldi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ashldi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a << b */

/* Precondition:  0 <= b < bits_in_dword */

ARM_EABI_FNALIAS(llsl, ashldi3)

COMPILER_RT_ABI di_int
__ashldi3(di_int a, si_int b)
{
    const int bits_in_word = (int)(sizeof(si_int) * CHAR_BIT);
    dwords input;
    dwords result;
    input.all = a;
    if (b & bits_in_word)  /* bits_in_word <= b < bits_in_dword */
    {
        result.s.low = 0;
        result.s.high = input.s.low << (b - bits_in_word);
    }
    else  /* 0 <= b < bits_in_word */
    {
        if (b == 0)
            return a;
        result.s.low  = input.s.low << b;
        result.s.high = (input.s.high << b) | (input.s.low >> (bits_in_word - b));
    }
    return result.all;
}
/* ===-- ashlti3.c - Implement __ashlti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ashlti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: a << b */

/* Precondition:  0 <= b < bits_in_tword */

ti_int
__ashlti3(ti_int a, si_int b)
{
    const int bits_in_dword = (int)(sizeof(di_int) * CHAR_BIT);
    twords input;
    twords result;
    input.all = a;
    if (b & bits_in_dword)  /* bits_in_dword <= b < bits_in_tword */
    {
        result.s.low = 0;
        result.s.high = input.s.low << (b - bits_in_dword);
    }
    else  /* 0 <= b < bits_in_dword */
    {
        if (b == 0)
            return a;
        result.s.low  = input.s.low << b;
        result.s.high = (input.s.high << b) | (input.s.low >> (bits_in_dword - b));
    }
    return result.all;
}

#endif /* __x86_64 */
/*===-- ashrdi3.c - Implement __ashrdi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ashrdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: arithmetic a >> b */

/* Precondition:  0 <= b < bits_in_dword */

ARM_EABI_FNALIAS(lasr, ashrdi3)

COMPILER_RT_ABI di_int
__ashrdi3(di_int a, si_int b)
{
    const int bits_in_word = (int)(sizeof(si_int) * CHAR_BIT);
    dwords input;
    dwords result;
    input.all = a;
    if (b & bits_in_word)  /* bits_in_word <= b < bits_in_dword */
    {
        /* result.s.high = input.s.high < 0 ? -1 : 0 */
        result.s.high = input.s.high >> (bits_in_word - 1);
        result.s.low = input.s.high >> (b - bits_in_word);
    }
    else  /* 0 <= b < bits_in_word */
    {
        if (b == 0)
            return a;
        result.s.high  = input.s.high >> b;
        result.s.low = (input.s.high << (bits_in_word - b)) | (input.s.low >> b);
    }
    return result.all;
}
/* ===-- ashrti3.c - Implement __ashrti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ashrti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: arithmetic a >> b */

/* Precondition:  0 <= b < bits_in_tword */

ti_int
__ashrti3(ti_int a, si_int b)
{
    const int bits_in_dword = (int)(sizeof(di_int) * CHAR_BIT);
    twords input;
    twords result;
    input.all = a;
    if (b & bits_in_dword)  /* bits_in_dword <= b < bits_in_tword */
    {
        /* result.s.high = input.s.high < 0 ? -1 : 0 */
        result.s.high = input.s.high >> (bits_in_dword - 1);
        result.s.low = input.s.high >> (b - bits_in_dword);
    }
    else  /* 0 <= b < bits_in_dword */
    {
        if (b == 0)
            return a;
        result.s.high  = input.s.high >> b;
        result.s.low = (input.s.high << (bits_in_dword - b)) | (input.s.low >> b);
    }
    return result.all;
}

#endif /* __x86_64 */
/* ===-- clzdi2.c - Implement __clzdi2 -------------------------------------===
 *
 *               The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __clzdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the number of leading 0-bits */

/* Precondition: a != 0 */

COMPILER_RT_ABI si_int
__clzdi2(di_int a)
{
    dwords x;
    x.all = a;
    const si_int f = -(x.s.high == 0);
    return __builtin_clz((x.s.high & ~f) | (x.s.low & f)) +
           (f & ((si_int)(sizeof(si_int) * CHAR_BIT)));
}
/* ===-- clzsi2.c - Implement __clzsi2 -------------------------------------===
 *
 *               The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __clzsi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the number of leading 0-bits */

/* Precondition: a != 0 */

COMPILER_RT_ABI si_int
__clzsi2(si_int a)
{
    su_int x = (su_int)a;
    si_int t = ((x & 0xFFFF0000) == 0) << 4;  /* if (x is small) t = 16 else 0 */
    x >>= 16 - t;      /* x = [0 - 0xFFFF] */
    su_int r = t;       /* r = [0, 16] */
    /* return r + clz(x) */
    t = ((x & 0xFF00) == 0) << 3;
    x >>= 8 - t;       /* x = [0 - 0xFF] */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + clz(x) */
    t = ((x & 0xF0) == 0) << 2;
    x >>= 4 - t;       /* x = [0 - 0xF] */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + clz(x) */
    t = ((x & 0xC) == 0) << 1;
    x >>= 2 - t;       /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */
    /* return r + clz(x) */
/*     switch (x)
 *     {
 *     case 0:
 *         return r + 2;
 *     case 1:
 *         return r + 1;
 *     case 2:
 *     case 3:
 *         return r;
 *     }
 */
    return r + ((2 - x) & -((x & 2) == 0));
}
/* ===-- clzti2.c - Implement __clzti2 -------------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __clzti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: the number of leading 0-bits */

/* Precondition: a != 0 */

si_int
__clzti2(ti_int a)
{
    twords x;
    x.all = a;
    const di_int f = -(x.s.high == 0);
    return __builtin_clzll((x.s.high & ~f) | (x.s.low & f)) +
           ((si_int)f & ((si_int)(sizeof(di_int) * CHAR_BIT)));
}

#endif /* __x86_64 */
/* ===-- cmpdi2.c - Implement __cmpdi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __cmpdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: if (a <  b) returns 0
*           if (a == b) returns 1
*           if (a >  b) returns 2
*/

COMPILER_RT_ABI si_int
__cmpdi2(di_int a, di_int b)
{
    dwords x;
    x.all = a;
    dwords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#ifdef __ARM_EABI__
/* Returns: if (a <  b) returns -1
*           if (a == b) returns  0
*           if (a >  b) returns  1
*/
COMPILER_RT_ABI si_int
__aeabi_lcmp(di_int a, di_int b)
{
	return __cmpdi2(a, b) - 1;
}
#endif

/* ===-- cmpti2.c - Implement __cmpti2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __cmpti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns:  if (a <  b) returns 0
 *           if (a == b) returns 1
 *           if (a >  b) returns 2
 */

si_int
__cmpti2(ti_int a, ti_int b)
{
    twords x;
    x.all = a;
    twords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#endif
//===-- lib/comparedf2.c - Double-precision comparisons -----------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// // This file implements the following soft-float comparison routines:
//
//   __eqdf2   __gedf2   __unorddf2
//   __ledf2   __gtdf2
//   __ltdf2
//   __nedf2
//
// The semantics of the routines grouped in each column are identical, so there
// is a single implementation for each, and wrappers to provide the other names.
//
// The main routines behave as follows:
//
//   __ledf2(a,b) returns -1 if a < b
//                         0 if a == b
//                         1 if a > b
//                         1 if either a or b is NaN
//
//   __gedf2(a,b) returns -1 if a < b
//                         0 if a == b
//                         1 if a > b
//                        -1 if either a or b is NaN
//
//   __unorddf2(a,b) returns 0 if both a and b are numbers
//                           1 if either a or b is NaN
//
// Note that __ledf2( ) and __gedf2( ) are identical except in their handling of
// NaN values.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

#define LE_RESULT _DP_LE_RESULT
#define LE_LESS _DP_LE_LESS
#define LE_EQUAL _DP_LE_EQUAL
#define LE_GREATER _DP_LE_GREATER
#define LE_UNORDERED _DP_LE_UNORDERED

enum LE_RESULT {
    LE_LESS      = -1,
    LE_EQUAL     =  0,
    LE_GREATER   =  1,
    LE_UNORDERED =  1
};

enum LE_RESULT __ledf2(fp_t a, fp_t b) {

    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;

    // If either a or b is NaN, they are unordered.
    if (aAbs > infRep || bAbs > infRep) return LE_UNORDERED;

    // If a and b are both zeros, they are equal.
    if ((aAbs | bAbs) == 0) return LE_EQUAL;

    // If at least one of a and b is positive, we get the same result comparing
    // a and b as signed integers as we would with a floating-point compare.
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }

    // Otherwise, both are negative, so we need to flip the sense of the
    // comparison to get the correct result.  (This assumes a twos- or ones-
    // complement integer representation; if integers are represented in a
    // sign-magnitude representation, then this flip is incorrect).
    else {
        if (aInt > bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }
}

#define GE_RESULT _DP_GE_RESULT
#define GE_LESS _DP_GE_LESS
#define GE_EQUAL _DP_GE_EQUAL
#define GE_GREATER _DP_GE_GREATER
#define GE_UNORDERED _DP_GE_UNORDERED

enum GE_RESULT {
    GE_LESS      = -1,
    GE_EQUAL     =  0,
    GE_GREATER   =  1,
    GE_UNORDERED = -1   // Note: different from LE_UNORDERED
};

enum GE_RESULT __gedf2(fp_t a, fp_t b) {

    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;

    if (aAbs > infRep || bAbs > infRep) return GE_UNORDERED;
    if ((aAbs | bAbs) == 0) return GE_EQUAL;
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    } else {
        if (aInt > bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    }
}

ARM_EABI_FNALIAS(dcmpun, unorddf2)

int __unorddf2(fp_t a, fp_t b) {
    const rep_t aAbs = toRep(a) & absMask;
    const rep_t bAbs = toRep(b) & absMask;
    return aAbs > infRep || bAbs > infRep;
}

// The following are alternative names for the preceeding routines.

enum LE_RESULT __eqdf2(fp_t a, fp_t b) {
    return __ledf2(a, b);
}

enum LE_RESULT __ltdf2(fp_t a, fp_t b) {
    return __ledf2(a, b);
}

enum LE_RESULT __nedf2(fp_t a, fp_t b) {
    return __ledf2(a, b);
}

enum GE_RESULT __gtdf2(fp_t a, fp_t b) {
    return __gedf2(a, b);
}

//===-- lib/comparesf2.c - Single-precision comparisons -----------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the following soft-fp_t comparison routines:
//
//   __eqsf2   __gesf2   __unordsf2
//   __lesf2   __gtsf2
//   __ltsf2
//   __nesf2
//
// The semantics of the routines grouped in each column are identical, so there
// is a single implementation for each, and wrappers to provide the other names.
//
// The main routines behave as follows:
//
//   __lesf2(a,b) returns -1 if a < b
//                         0 if a == b
//                         1 if a > b
//                         1 if either a or b is NaN
//
//   __gesf2(a,b) returns -1 if a < b
//                         0 if a == b
//                         1 if a > b
//                        -1 if either a or b is NaN
//
//   __unordsf2(a,b) returns 0 if both a and b are numbers
//                           1 if either a or b is NaN
//
// Note that __lesf2( ) and __gesf2( ) are identical except in their handling of
// NaN values.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

#undef LE_RESULT
#undef LE_LESS
#undef LE_EQUAL
#undef LE_GREATER
#undef LE_UNORDERED

#define LE_RESULT _SP_LE_RESULT
#define LE_LESS _SP_LE_LESS
#define LE_EQUAL _SP_LE_EQUAL
#define LE_GREATER _SP_LE_GREATER
#define LE_UNORDERED _SP_LE_UNORDERED

enum LE_RESULT {
    LE_LESS      = -1,
    LE_EQUAL     =  0,
    LE_GREATER   =  1,
    LE_UNORDERED =  1
};

enum LE_RESULT __lesf2(fp_t a, fp_t b) {

    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;

    // If either a or b is NaN, they are unordered.
    if (aAbs > infRep || bAbs > infRep) return LE_UNORDERED;

    // If a and b are both zeros, they are equal.
    if ((aAbs | bAbs) == 0) return LE_EQUAL;

    // If at least one of a and b is positive, we get the same result comparing
    // a and b as signed integers as we would with a fp_ting-point compare.
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }

    // Otherwise, both are negative, so we need to flip the sense of the
    // comparison to get the correct result.  (This assumes a twos- or ones-
    // complement integer representation; if integers are represented in a
    // sign-magnitude representation, then this flip is incorrect).
    else {
        if (aInt > bInt) return LE_LESS;
        else if (aInt == bInt) return LE_EQUAL;
        else return LE_GREATER;
    }
}

#undef GE_RESULT
#undef GE_LESS
#undef GE_EQUAL
#undef GE_GREATER
#undef GE_UNORDERED

#define GE_RESULT _SP_GE_RESULT
#define GE_LESS _SP_GE_LESS
#define GE_EQUAL _SP_GE_EQUAL
#define GE_GREATER _SP_GE_GREATER
#define GE_UNORDERED _SP_GE_UNORDERED

enum GE_RESULT {
    GE_LESS      = -1,
    GE_EQUAL     =  0,
    GE_GREATER   =  1,
    GE_UNORDERED = -1   // Note: different from LE_UNORDERED
};

enum GE_RESULT __gesf2(fp_t a, fp_t b) {

    const srep_t aInt = toRep(a);
    const srep_t bInt = toRep(b);
    const rep_t aAbs = aInt & absMask;
    const rep_t bAbs = bInt & absMask;

    if (aAbs > infRep || bAbs > infRep) return GE_UNORDERED;
    if ((aAbs | bAbs) == 0) return GE_EQUAL;
    if ((aInt & bInt) >= 0) {
        if (aInt < bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    } else {
        if (aInt > bInt) return GE_LESS;
        else if (aInt == bInt) return GE_EQUAL;
        else return GE_GREATER;
    }
}

ARM_EABI_FNALIAS(fcmpun, unordsf2)

int __unordsf2(fp_t a, fp_t b) {
    const rep_t aAbs = toRep(a) & absMask;
    const rep_t bAbs = toRep(b) & absMask;
    return aAbs > infRep || bAbs > infRep;
}

// The following are alternative names for the preceeding routines.

enum LE_RESULT __eqsf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum LE_RESULT __ltsf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum LE_RESULT __nesf2(fp_t a, fp_t b) {
    return __lesf2(a, b);
}

enum GE_RESULT __gtsf2(fp_t a, fp_t b) {
    return __gesf2(a, b);
}
/* ===-- ctzdi2.c - Implement __ctzdi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ctzdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the number of trailing 0-bits  */

/* Precondition: a != 0 */

COMPILER_RT_ABI si_int
__ctzdi2(di_int a)
{
    dwords x;
    x.all = a;
    const si_int f = -(x.s.low == 0);
    return __builtin_ctz((x.s.high & f) | (x.s.low & ~f)) +
              (f & ((si_int)(sizeof(si_int) * CHAR_BIT)));
}
/* ===-- ctzsi2.c - Implement __ctzsi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ctzsi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the number of trailing 0-bits */

/* Precondition: a != 0 */

COMPILER_RT_ABI si_int
__ctzsi2(si_int a)
{
    su_int x = (su_int)a;
    si_int t = ((x & 0x0000FFFF) == 0) << 4;  /* if (x has no small bits) t = 16 else 0 */
    x >>= t;           /* x = [0 - 0xFFFF] + higher garbage bits */
    su_int r = t;       /* r = [0, 16]  */
    /* return r + ctz(x) */
    t = ((x & 0x00FF) == 0) << 3;
    x >>= t;           /* x = [0 - 0xFF] + higher garbage bits */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + ctz(x) */
    t = ((x & 0x0F) == 0) << 2;
    x >>= t;           /* x = [0 - 0xF] + higher garbage bits */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + ctz(x) */
    t = ((x & 0x3) == 0) << 1;
    x >>= t;
    x &= 3;            /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */
    /* return r + ctz(x) */

/*  The branch-less return statement below is equivalent
 *  to the following switch statement:
 *     switch (x)
 *    {
 *     case 0:
 *         return r + 2;
 *     case 2:
 *         return r + 1;
 *     case 1:
 *     case 3:
 *         return r;
 *     }
 */
    return r + ((2 - (x >> 1)) & -((x & 1) == 0));
}
/* ===-- ctzti2.c - Implement __ctzti2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ctzti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: the number of trailing 0-bits */

/* Precondition: a != 0 */

si_int
__ctzti2(ti_int a)
{
    twords x;
    x.all = a;
    const di_int f = -(x.s.low == 0);
    return __builtin_ctzll((x.s.high & f) | (x.s.low & ~f)) +
              ((si_int)f & ((si_int)(sizeof(di_int) * CHAR_BIT)));
}

#endif
/* ===-- divdc3.c - Implement __divdc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divdc3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the quotient of (a + ib) / (c + id) */

double _Complex
__divdc3(double __a, double __b, double __c, double __d)
{
    int __ilogbw = 0;
    double __logbw = crt_logb(crt_fmax(crt_fabs(__c), crt_fabs(__d)));
    if (crt_isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = crt_scalbn(__c, -__ilogbw);
        __d = crt_scalbn(__d, -__ilogbw);
    }
    double __denom = __c * __c + __d * __d;
    double _Complex z;
    __real__ z = crt_scalbn((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = crt_scalbn((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        if ((__denom == 0.0) && (!crt_isnan(__a) || !crt_isnan(__b)))
        {
            __real__ z = crt_copysign(CRT_INFINITY, __c) * __a;
            __imag__ z = crt_copysign(CRT_INFINITY, __c) * __b;
        }
        else if ((crt_isinf(__a) || crt_isinf(__b)) &&
                 crt_isfinite(__c) && crt_isfinite(__d))
        {
            __a = crt_copysign(crt_isinf(__a) ? 1.0 : 0.0, __a);
            __b = crt_copysign(crt_isinf(__b) ? 1.0 : 0.0, __b);
            __real__ z = CRT_INFINITY * (__a * __c + __b * __d);
            __imag__ z = CRT_INFINITY * (__b * __c - __a * __d);
        }
        else if (crt_isinf(__logbw) && __logbw > 0.0 &&
                 crt_isfinite(__a) && crt_isfinite(__b))
        {
            __c = crt_copysign(crt_isinf(__c) ? 1.0 : 0.0, __c);
            __d = crt_copysign(crt_isinf(__d) ? 1.0 : 0.0, __d);
            __real__ z = 0.0 * (__a * __c + __b * __d);
            __imag__ z = 0.0 * (__b * __c - __a * __d);
        }
    }
    return z;
}
//===-- lib/divdf3.c - Double-precision division ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float division
// with the IEEE-754 default rounding (to nearest, ties to even).
//
// For simplicity, this implementation currently flushes denormals to zero.
// It should be a fairly straightforward exercise to implement gradual
// underflow with correct rounding.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

ARM_EABI_FNALIAS(ddiv, divdf3)

fp_t __divdf3(fp_t a, fp_t b) {

    const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
    const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
    const rep_t quotientSign = (toRep(a) ^ toRep(b)) & signBit;

    rep_t aSignificand = toRep(a) & significandMask;
    rep_t bSignificand = toRep(b) & significandMask;
    int scale = 0;

    // Detect if a or b is zero, denormal, infinity, or NaN.
    if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {

        const rep_t aAbs = toRep(a) & absMask;
        const rep_t bAbs = toRep(b) & absMask;

        // NaN / anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything / NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // infinity / infinity = NaN
            if (bAbs == infRep) return fromRep(qnanRep);
            // infinity / anything else = +/- infinity
            else return fromRep(aAbs | quotientSign);
        }

        // anything else / infinity = +/- 0
        if (bAbs == infRep) return fromRep(quotientSign);

        if (!aAbs) {
            // zero / zero = NaN
            if (!bAbs) return fromRep(qnanRep);
            // zero / anything else = +/- zero
            else return fromRep(quotientSign);
        }
        // anything else / zero = +/- infinity
        if (!bAbs) return fromRep(infRep | quotientSign);

        // one or both of a or b is denormal, the other (if applicable) is a
        // normal number.  Renormalize one or both of a and b, and set scale to
        // include the necessary exponent adjustment.
        if (aAbs < implicitBit) scale += normalize(&aSignificand);
        if (bAbs < implicitBit) scale -= normalize(&bSignificand);
    }

    // Or in the implicit significand bit.  (If we fell through from the
    // denormal path it was already set by normalize( ), but setting it twice
    // won't hurt anything.)
    aSignificand |= implicitBit;
    bSignificand |= implicitBit;
    int quotientExponent = aExponent - bExponent + scale;

    // Align the significand of b as a Q31 fixed-point number in the range
    // [1, 2.0) and get a Q32 approximate reciprocal using a small minimax
    // polynomial approximation: reciprocal = 3/4 + 1/sqrt(2) - b/2.  This
    // is accurate to about 3.5 binary digits.
    const uint32_t q31b = bSignificand >> 21;
    uint32_t recip32 = UINT32_C(0x7504f333) - q31b;

    // Now refine the reciprocal estimate using a Newton-Raphson iteration:
    //
    //     x1 = x0 * (2 - x0 * b)
    //
    // This doubles the number of correct binary digits in the approximation
    // with each iteration, so after three iterations, we have about 28 binary
    // digits of accuracy.
    uint32_t correction32;
    correction32 = -((uint64_t)recip32 * q31b >> 32);
    recip32 = (uint64_t)recip32 * correction32 >> 31;
    correction32 = -((uint64_t)recip32 * q31b >> 32);
    recip32 = (uint64_t)recip32 * correction32 >> 31;
    correction32 = -((uint64_t)recip32 * q31b >> 32);
    recip32 = (uint64_t)recip32 * correction32 >> 31;

    // recip32 might have overflowed to exactly zero in the preceeding
    // computation if the high word of b is exactly 1.0.  This would sabotage
    // the full-width final stage of the computation that follows, so we adjust
    // recip32 downward by one bit.
    recip32--;

    // We need to perform one more iteration to get us to 56 binary digits;
    // The last iteration needs to happen with extra precision.
    const uint32_t q63blo = bSignificand << 11;
    uint64_t correction, reciprocal;
    correction = -((uint64_t)recip32*q31b + ((uint64_t)recip32*q63blo >> 32));
    uint32_t cHi = correction >> 32;
    uint32_t cLo = correction;
    reciprocal = (uint64_t)recip32*cHi + ((uint64_t)recip32*cLo >> 32);

    // We already adjusted the 32-bit estimate, now we need to adjust the final
    // 64-bit reciprocal estimate downward to ensure that it is strictly smaller
    // than the infinitely precise exact reciprocal.  Because the computation
    // of the Newton-Raphson step is truncating at every step, this adjustment
    // is small; most of the work is already done.
    reciprocal -= 2;

    // The numerical reciprocal is accurate to within 2^-56, lies in the
    // interval [0.5, 1.0), and is strictly smaller than the true reciprocal
    // of b.  Multiplying a by this reciprocal thus gives a numerical q = a/b
    // in Q53 with the following properties:
    //
    //    1. q < a/b
    //    2. q is in the interval [0.5, 2.0)
    //    3. the error in q is bounded away from 2^-53 (actually, we have a
    //       couple of bits to spare, but this is all we need).

    // We need a 64 x 64 multiply high to compute q, which isn't a basic
    // operation in C, so we need to be a little bit fussy.
    rep_t quotient, quotientLo;
    wideMultiply(aSignificand << 2, reciprocal, &quotient, &quotientLo);

    // Two cases: quotient is in [0.5, 1.0) or quotient is in [1.0, 2.0).
    // In either case, we are going to compute a residual of the form
    //
    //     r = a - q*b
    //
    // We know from the construction of q that r satisfies:
    //
    //     0 <= r < ulp(q)*b
    //
    // if r is greater than 1/2 ulp(q)*b, then q rounds up.  Otherwise, we
    // already have the correct result.  The exact halfway case cannot occur.
    // We also take this time to right shift quotient if it falls in the [1,2)
    // range and adjust the exponent accordingly.
    rep_t residual;
    if (quotient < (implicitBit << 1)) {
        residual = (aSignificand << 53) - quotient * bSignificand;
        quotientExponent--;
    } else {
        quotient >>= 1;
        residual = (aSignificand << 52) - quotient * bSignificand;
    }

    const int writtenExponent = quotientExponent + exponentBias;

    if (writtenExponent >= maxExponent) {
        // If we have overflowed the exponent, return infinity.
        return fromRep(infRep | quotientSign);
    }

    else if (writtenExponent < 1) {
        // Flush denormals to zero.  In the future, it would be nice to add
        // code to round them correctly.
        return fromRep(quotientSign);
    }

    else {
        const bool round = (residual << 1) > bSignificand;
        // Clear the implicit bit
        rep_t absResult = quotient & significandMask;
        // Insert the exponent
        absResult |= (rep_t)writtenExponent << significandBits;
        // Round
        absResult += round;
        // Insert the sign and return
        const double result = fromRep(absResult | quotientSign);
        return result;
    }
}
/* ===-- divdi3.c - Implement __divdi3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


du_int COMPILER_RT_ABI __udivmoddi4(du_int a, du_int b, du_int* rem);

/* Returns: a / b */

COMPILER_RT_ABI di_int
__divdi3(di_int a, di_int b)
{
    const int bits_in_dword_m1 = (int)(sizeof(di_int) * CHAR_BIT) - 1;
    di_int s_a = a >> bits_in_dword_m1;           /* s_a = a < 0 ? -1 : 0 */
    di_int s_b = b >> bits_in_dword_m1;           /* s_b = b < 0 ? -1 : 0 */
    a = (a ^ s_a) - s_a;                         /* negate if s_a == -1 */
    b = (b ^ s_b) - s_b;                         /* negate if s_b == -1 */
    s_a ^= s_b;                                  /*sign of quotient */
    return (__udivmoddi4(a, b, (du_int*)0) ^ s_a) - s_a;  /* negate if s_a == -1 */
}
/*===-- divmoddi4.c - Implement __divmoddi4 --------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divmoddi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


extern COMPILER_RT_ABI di_int __divdi3(di_int a, di_int b);

/* Returns: a / b, *rem = a % b  */

COMPILER_RT_ABI di_int
__divmoddi4(di_int a, di_int b, di_int* rem)
{
  di_int d = __divdi3(a,b);
  *rem = a - (d*b);
  return d;
}
/*===-- divmodsi4.c - Implement __divmodsi4 --------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divmodsi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


extern COMPILER_RT_ABI si_int __divsi3(si_int a, si_int b);


/* Returns: a / b, *rem = a % b  */

COMPILER_RT_ABI si_int
__divmodsi4(si_int a, si_int b, si_int* rem)
{
  si_int d = __divsi3(a,b);
  *rem = a - (d*b);
  return d;
}


/*===-- divsc3.c - Implement __divsc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divsc3 for the compiler_rt library.
 *
 *===----------------------------------------------------------------------===
 */


/* Returns: the quotient of (a + ib) / (c + id) */

float _Complex
__divsc3(float __a, float __b, float __c, float __d)
{
    int __ilogbw = 0;
    float __logbw = crt_logbf(crt_fmaxf(crt_fabsf(__c), crt_fabsf(__d)));
    if (crt_isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = crt_scalbnf(__c, -__ilogbw);
        __d = crt_scalbnf(__d, -__ilogbw);
    }
    float __denom = __c * __c + __d * __d;
    float _Complex z;
    __real__ z = crt_scalbnf((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = crt_scalbnf((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        if ((__denom == 0) && (!crt_isnan(__a) || !crt_isnan(__b)))
        {
            __real__ z = crt_copysignf(CRT_INFINITY, __c) * __a;
            __imag__ z = crt_copysignf(CRT_INFINITY, __c) * __b;
        }
        else if ((crt_isinf(__a) || crt_isinf(__b)) &&
                 crt_isfinite(__c) && crt_isfinite(__d))
        {
            __a = crt_copysignf(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignf(crt_isinf(__b) ? 1 : 0, __b);
            __real__ z = CRT_INFINITY * (__a * __c + __b * __d);
            __imag__ z = CRT_INFINITY * (__b * __c - __a * __d);
        }
        else if (crt_isinf(__logbw) && __logbw > 0 &&
                 crt_isfinite(__a) && crt_isfinite(__b))
        {
            __c = crt_copysignf(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignf(crt_isinf(__d) ? 1 : 0, __d);
            __real__ z = 0 * (__a * __c + __b * __d);
            __imag__ z = 0 * (__b * __c - __a * __d);
        }
    }
    return z;
}
//===-- lib/divsf3.c - Single-precision division ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision soft-float division
// with the IEEE-754 default rounding (to nearest, ties to even).
//
// For simplicity, this implementation currently flushes denormals to zero.
// It should be a fairly straightforward exercise to implement gradual
// underflow with correct rounding.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

ARM_EABI_FNALIAS(fdiv, divsf3)

fp_t __divsf3(fp_t a, fp_t b) {

    const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
    const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
    const rep_t quotientSign = (toRep(a) ^ toRep(b)) & signBit;

    rep_t aSignificand = toRep(a) & significandMask;
    rep_t bSignificand = toRep(b) & significandMask;
    int scale = 0;

    // Detect if a or b is zero, denormal, infinity, or NaN.
    if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {

        const rep_t aAbs = toRep(a) & absMask;
        const rep_t bAbs = toRep(b) & absMask;

        // NaN / anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything / NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // infinity / infinity = NaN
            if (bAbs == infRep) return fromRep(qnanRep);
            // infinity / anything else = +/- infinity
            else return fromRep(aAbs | quotientSign);
        }

        // anything else / infinity = +/- 0
        if (bAbs == infRep) return fromRep(quotientSign);

        if (!aAbs) {
            // zero / zero = NaN
            if (!bAbs) return fromRep(qnanRep);
            // zero / anything else = +/- zero
            else return fromRep(quotientSign);
        }
        // anything else / zero = +/- infinity
        if (!bAbs) return fromRep(infRep | quotientSign);

        // one or both of a or b is denormal, the other (if applicable) is a
        // normal number.  Renormalize one or both of a and b, and set scale to
        // include the necessary exponent adjustment.
        if (aAbs < implicitBit) scale += normalize(&aSignificand);
        if (bAbs < implicitBit) scale -= normalize(&bSignificand);
    }

    // Or in the implicit significand bit.  (If we fell through from the
    // denormal path it was already set by normalize( ), but setting it twice
    // won't hurt anything.)
    aSignificand |= implicitBit;
    bSignificand |= implicitBit;
    int quotientExponent = aExponent - bExponent + scale;

    // Align the significand of b as a Q31 fixed-point number in the range
    // [1, 2.0) and get a Q32 approximate reciprocal using a small minimax
    // polynomial approximation: reciprocal = 3/4 + 1/sqrt(2) - b/2.  This
    // is accurate to about 3.5 binary digits.
    uint32_t q31b = bSignificand << 8;
    uint32_t reciprocal = UINT32_C(0x7504f333) - q31b;

    // Now refine the reciprocal estimate using a Newton-Raphson iteration:
    //
    //     x1 = x0 * (2 - x0 * b)
    //
    // This doubles the number of correct binary digits in the approximation
    // with each iteration, so after three iterations, we have about 28 binary
    // digits of accuracy.
    uint32_t correction;
    correction = -((uint64_t)reciprocal * q31b >> 32);
    reciprocal = (uint64_t)reciprocal * correction >> 31;
    correction = -((uint64_t)reciprocal * q31b >> 32);
    reciprocal = (uint64_t)reciprocal * correction >> 31;
    correction = -((uint64_t)reciprocal * q31b >> 32);
    reciprocal = (uint64_t)reciprocal * correction >> 31;

    // Exhaustive testing shows that the error in reciprocal after three steps
    // is in the interval [-0x1.f58108p-31, 0x1.d0e48cp-29], in line with our
    // expectations.  We bump the reciprocal by a tiny value to force the error
    // to be strictly positive (in the range [0x1.4fdfp-37,0x1.287246p-29], to
    // be specific).  This also causes 1/1 to give a sensible approximation
    // instead of zero (due to overflow).
    reciprocal -= 2;

    // The numerical reciprocal is accurate to within 2^-28, lies in the
    // interval [0x1.000000eep-1, 0x1.fffffffcp-1], and is strictly smaller
    // than the true reciprocal of b.  Multiplying a by this reciprocal thus
    // gives a numerical q = a/b in Q24 with the following properties:
    //
    //    1. q < a/b
    //    2. q is in the interval [0x1.000000eep-1, 0x1.fffffffcp0)
    //    3. the error in q is at most 2^-24 + 2^-27 -- the 2^24 term comes
    //       from the fact that we truncate the product, and the 2^27 term
    //       is the error in the reciprocal of b scaled by the maximum
    //       possible value of a.  As a consequence of this error bound,
    //       either q or nextafter(q) is the correctly rounded
    rep_t quotient = (uint64_t)reciprocal*(aSignificand << 1) >> 32;

    // Two cases: quotient is in [0.5, 1.0) or quotient is in [1.0, 2.0).
    // In either case, we are going to compute a residual of the form
    //
    //     r = a - q*b
    //
    // We know from the construction of q that r satisfies:
    //
    //     0 <= r < ulp(q)*b
    //
    // if r is greater than 1/2 ulp(q)*b, then q rounds up.  Otherwise, we
    // already have the correct result.  The exact halfway case cannot occur.
    // We also take this time to right shift quotient if it falls in the [1,2)
    // range and adjust the exponent accordingly.
    rep_t residual;
    if (quotient < (implicitBit << 1)) {
        residual = (aSignificand << 24) - quotient * bSignificand;
        quotientExponent--;
    } else {
        quotient >>= 1;
        residual = (aSignificand << 23) - quotient * bSignificand;
    }

    const int writtenExponent = quotientExponent + exponentBias;

    if (writtenExponent >= maxExponent) {
        // If we have overflowed the exponent, return infinity.
        return fromRep(infRep | quotientSign);
    }

    else if (writtenExponent < 1) {
        // Flush denormals to zero.  In the future, it would be nice to add
        // code to round them correctly.
        return fromRep(quotientSign);
    }

    else {
        const bool round = (residual << 1) > bSignificand;
        // Clear the implicit bit
        rep_t absResult = quotient & significandMask;
        // Insert the exponent
        absResult |= (rep_t)writtenExponent << significandBits;
        // Round
        absResult += round;
        // Insert the sign and return
        return fromRep(absResult | quotientSign);
    }
}
/* ===-- divsi3.c - Implement __divsi3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


su_int COMPILER_RT_ABI __udivsi3(su_int n, su_int d);

/* Returns: a / b */

ARM_EABI_FNALIAS(idiv, divsi3)

COMPILER_RT_ABI si_int
__divsi3(si_int a, si_int b)
{
    const int bits_in_word_m1 = (int)(sizeof(si_int) * CHAR_BIT) - 1;
    si_int s_a = a >> bits_in_word_m1;           /* s_a = a < 0 ? -1 : 0 */
    si_int s_b = b >> bits_in_word_m1;           /* s_b = b < 0 ? -1 : 0 */
    a = (a ^ s_a) - s_a;                         /* negate if s_a == -1 */
    b = (b ^ s_b) - s_b;                         /* negate if s_b == -1 */
    s_a ^= s_b;                                  /* sign of quotient */
    /*
     * On CPUs without unsigned hardware division support,
     *  this calls __udivsi3 (notice the cast to su_int).
     * On CPUs with unsigned hardware division support,
     *  this uses the unsigned division instruction.
     */
    return ((su_int)a/(su_int)b ^ s_a) - s_a;    /* negate if s_a == -1 */
}
/* ===-- divti3.c - Implement __divti3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

tu_int __udivmodti4(tu_int a, tu_int b, tu_int* rem);

/* Returns: a / b */

ti_int
__divti3(ti_int a, ti_int b)
{
    const int bits_in_tword_m1 = (int)(sizeof(ti_int) * CHAR_BIT) - 1;
    ti_int s_a = a >> bits_in_tword_m1;           /* s_a = a < 0 ? -1 : 0 */
    ti_int s_b = b >> bits_in_tword_m1;           /* s_b = b < 0 ? -1 : 0 */
    a = (a ^ s_a) - s_a;                         /* negate if s_a == -1 */
    b = (b ^ s_b) - s_b;                         /* negate if s_b == -1 */
    s_a ^= s_b;                                  /* sign of quotient */
    return (__udivmodti4(a, b, (tu_int*)0) ^ s_a) - s_a;  /* negate if s_a == -1 */
}

#endif
/* ===-- divxc3.c - Implement __divxc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __divxc3 for the compiler_rt library.
 *
 */

#if !_ARCH_PPC


/* Returns: the quotient of (a + ib) / (c + id) */

long double _Complex
__divxc3(long double __a, long double __b, long double __c, long double __d)
{
    int __ilogbw = 0;
    long double __logbw = crt_logbl(crt_fmaxl(crt_fabsl(__c), crt_fabsl(__d)));
    if (crt_isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = crt_scalbnl(__c, -__ilogbw);
        __d = crt_scalbnl(__d, -__ilogbw);
    }
    long double __denom = __c * __c + __d * __d;
    long double _Complex z;
    __real__ z = crt_scalbnl((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = crt_scalbnl((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        if ((__denom == 0) && (!crt_isnan(__a) || !crt_isnan(__b)))
        {
            __real__ z = crt_copysignl(CRT_INFINITY, __c) * __a;
            __imag__ z = crt_copysignl(CRT_INFINITY, __c) * __b;
        }
        else if ((crt_isinf(__a) || crt_isinf(__b)) &&
                 crt_isfinite(__c) && crt_isfinite(__d))
        {
            __a = crt_copysignl(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignl(crt_isinf(__b) ? 1 : 0, __b);
            __real__ z = CRT_INFINITY * (__a * __c + __b * __d);
            __imag__ z = CRT_INFINITY * (__b * __c - __a * __d);
        }
        else if (crt_isinf(__logbw) && __logbw > 0 &&
                 crt_isfinite(__a) && crt_isfinite(__b))
        {
            __c = crt_copysignl(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignl(crt_isinf(__d) ? 1 : 0, __d);
            __real__ z = 0 * (__a * __c + __b * __d);
            __imag__ z = 0 * (__b * __c - __a * __d);
        }
    }
    return z;
}

#endif
//===-- lib/extendsfdf2.c - single -> double conversion -----------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a fairly generic conversion from a narrower to a wider
// IEEE-754 floating-point type.  The constants and types defined following the
// includes below parameterize the conversion.
//
// This routine can be trivially adapted to support conversions from
// half-precision or to quad-precision. It does not support types that don't
// use the usual IEEE-754 interchange formats; specifically, some work would be
// needed to adapt it to (for example) the Intel 80-bit format or PowerPC
// double-double format.
//
// Note please, however, that this implementation is only intended to support
// *widening* operations; if you need to convert to a *narrower* floating-point
// type (e.g. double -> float), then this routine will not do what you want it
// to.
//
// It also requires that integer types at least as large as both formats
// are available on the target platform; this may pose a problem when trying
// to add support for quad on some 32-bit systems, for example.  You also may
// run into trouble finding an appropriate CLZ function for wide source types;
// you will likely need to roll your own on some platforms.
//
// Finally, the following assumptions are made:
//
// 1. floating-point types and integer types have the same endianness on the
//    target platform
//
// 2. quiet NaNs, if supported, are indicated by the leading bit of the
//    significand field being set
//
//===----------------------------------------------------------------------===//


typedef float flt_src_t;
typedef uint32_t u32_src_rep_t;
#define SRC_REP_C UINT32_C
#define src_rep_t_clz __builtin_clz

typedef double dbl_dst_t;
typedef uint64_t u64_dst_rep_t;
#define DST_REP_C UINT64_C

// End of specialization parameters.  Two helper routines for conversion to and
// from the representation of floating-point data as integer values follow.

static inline u32_src_rep_t extendsfdf2_srcToRep(flt_src_t x) {
    const union { flt_src_t f; u32_src_rep_t i; } rep = {.f = x};
    return rep.i;
}

static inline dbl_dst_t extendsfdf2_dstFromRep(u64_dst_rep_t x) {
    const union { dbl_dst_t f; u64_dst_rep_t i; } rep = {.i = x};
    return rep.f;
}

// End helper routines.  Conversion implementation follows.

ARM_EABI_FNALIAS(f2d, extendsfdf2)

dbl_dst_t __extendsfdf2(flt_src_t a) {
    static const int srcSigBits = 23;
    static const int dstSigBits = 52;

    // Various constants whose values follow from the type parameters.
    // Any reasonable optimizer will fold and propagate all of these.
    const int srcBits = sizeof(flt_src_t)*CHAR_BIT;
    const int srcExpBits = srcBits - srcSigBits - 1;
    const int srcInfExp = (1 << srcExpBits) - 1;
    const int srcExpBias = srcInfExp >> 1;

    const u32_src_rep_t srcMinNormal = SRC_REP_C(1) << srcSigBits;
    const u32_src_rep_t srcInfinity = (u32_src_rep_t)srcInfExp << srcSigBits;
    const u32_src_rep_t srcSignMask = SRC_REP_C(1) << (srcSigBits + srcExpBits);
    const u32_src_rep_t srcAbsMask = srcSignMask - 1;
    const u32_src_rep_t srcQNaN = SRC_REP_C(1) << (srcSigBits - 1);
    const u32_src_rep_t srcNaNCode = srcQNaN - 1;

    const int dstBits = sizeof(dbl_dst_t)*CHAR_BIT;
    const int dstExpBits = dstBits - dstSigBits - 1;
    const int dstInfExp = (1 << dstExpBits) - 1;
    const int dstExpBias = dstInfExp >> 1;

    const u64_dst_rep_t dstMinNormal = DST_REP_C(1) << dstSigBits;

    // Break a into a sign and representation of the absolute value
    const u32_src_rep_t aRep = extendsfdf2_srcToRep(a);
    const u32_src_rep_t aAbs = aRep & srcAbsMask;
    const u32_src_rep_t sign = aRep & srcSignMask;
    u64_dst_rep_t absResult;

    if (aAbs - srcMinNormal < srcInfinity - srcMinNormal) {
        // a is a normal number.
        // Extend to the destination type by shifting the significand and
        // exponent into the proper position and rebiasing the exponent.
        absResult = (u64_dst_rep_t)aAbs << (dstSigBits - srcSigBits);
        absResult += (u64_dst_rep_t)(dstExpBias - srcExpBias) << dstSigBits;
    }

    else if (aAbs >= srcInfinity) {
        // a is NaN or infinity.
        // Conjure the result by beginning with infinity, then setting the qNaN
        // bit (if needed) and right-aligning the rest of the trailing NaN
        // payload field.
        absResult = (u64_dst_rep_t)dstInfExp << dstSigBits;
        absResult |= (u64_dst_rep_t)(aAbs & srcQNaN) << (dstSigBits - srcSigBits);
        absResult |= aAbs & srcNaNCode;
    }

    else if (aAbs) {
        // a is denormal.
        // renormalize the significand and clear the leading bit, then insert
        // the correct adjusted exponent in the destination type.
        const int scale = src_rep_t_clz(aAbs) - src_rep_t_clz(srcMinNormal);
        absResult = (u64_dst_rep_t)aAbs << (dstSigBits - srcSigBits + scale);
        absResult ^= dstMinNormal;
        const int resultExponent = dstExpBias - srcExpBias - scale + 1;
        absResult |= (u64_dst_rep_t)resultExponent << dstSigBits;
    }

    else {
        // a is zero.
        absResult = 0;
    }

    // Apply the signbit to (dst_t)abs(a).
    const u64_dst_rep_t result = absResult | (u64_dst_rep_t)sign << (dstBits - srcBits);
    return extendsfdf2_dstFromRep(result);
}

#undef SRC_REP_C
#undef src_rep_t_clz
#undef DST_REP_C

/* ===-- ffsdi2.c - Implement __ffsdi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ffsdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the index of the least significant 1-bit in a, or
 * the value zero if a is zero. The least significant bit is index one.
 */

COMPILER_RT_ABI si_int
__ffsdi2(di_int a)
{
    dwords x;
    x.all = a;
    if (x.s.low == 0)
    {
        if (x.s.high == 0)
            return 0;
        return __builtin_ctz(x.s.high) + (1 + sizeof(si_int) * CHAR_BIT);
    }
    return __builtin_ctz(x.s.low) + 1;
}
/* ===-- ffsti2.c - Implement __ffsti2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ffsti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: the index of the least significant 1-bit in a, or
 * the value zero if a is zero. The least significant bit is index one.
 */

si_int
__ffsti2(ti_int a)
{
    twords x;
    x.all = a;
    if (x.s.low == 0)
    {
        if (x.s.high == 0)
            return 0;
        return __builtin_ctzll(x.s.high) + (1 + sizeof(di_int) * CHAR_BIT);
    }
    return __builtin_ctzll(x.s.low) + 1;
}

#endif /* __x86_64 */
/* ===-- fixdfdi.c - Implement __fixdfdi -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixdfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: double is a IEEE 64 bit floating point type
 *            su_int is a 32 bit integral type
 *            value in double is representable in di_int (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(d2lz, fixdfdi)

di_int
__fixdfdi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0)
        return 0;
    di_int s = (si_int)(fb.u.s.high & 0x80000000) >> 31;
    dwords r;
    r.s.high = (fb.u.s.high & 0x000FFFFF) | 0x00100000;
    r.s.low = fb.u.s.low;
    if (e > 52)
        r.all <<= (e - 52);
    else
        r.all >>= (52 - e);
    return (r.all ^ s) - s;
}
//===-- lib/fixdfsi.c - Double-precision -> integer conversion ----*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision to integer conversion for the
// compiler-rt library.  No range checking is performed; the behavior of this
// conversion is undefined for out of range values in the C standard.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"


ARM_EABI_FNALIAS(d2iz, fixdfsi)

int __fixdfsi(fp_t a) {

    // Break a into sign, exponent, significand
    const rep_t aRep = toRep(a);
    const rep_t aAbs = aRep & absMask;
    const int sign = aRep & signBit ? -1 : 1;
    const int exponent = (aAbs >> significandBits) - exponentBias;
    const rep_t significand = (aAbs & significandMask) | implicitBit;

    // If 0 < exponent < significandBits, right shift to get the result.
    if ((unsigned int)exponent < significandBits) {
        return sign * (significand >> (significandBits - exponent));
    }

    // If exponent is negative, the result is zero.
    else if (exponent < 0) {
        return 0;
    }

    // If significandBits < exponent, left shift to get the result.  This shift
    // may end up being larger than the type width, which incurs undefined
    // behavior, but the conversion itself is undefined in that case, so
    // whatever the compiler decides to do is fine.
    else {
        return sign * (significand << (exponent - significandBits));
    }
}
/* ===-- fixdfti.c - Implement __fixdfti -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixdfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: double is a IEEE 64 bit floating point type
 *             su_int is a 32 bit integral type
 *             value in double is representable in ti_int (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ti_int
__fixdfti(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0)
        return 0;
    ti_int s = (si_int)(fb.u.s.high & 0x80000000) >> 31;
    ti_int r = 0x0010000000000000uLL | (0x000FFFFFFFFFFFFFuLL & fb.u.all);
    if (e > 52)
        r <<= (e - 52);
    else
        r >>= (52 - e);
    return (r ^ s) - s;
}

#endif
/* ===-- fixsfdi.c - Implement __fixsfdi -----------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixsfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: float is a IEEE 32 bit floating point type
 *             su_int is a 32 bit integral type
 *             value in float is representable in di_int (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(f2lz, fixsfdi)

COMPILER_RT_ABI di_int
__fixsfdi(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0)
        return 0;
    di_int s = (si_int)(fb.u & 0x80000000) >> 31;
    di_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return (r ^ s) - s;
}
//===-- lib/fixsfsi.c - Single-precision -> integer conversion ----*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision to integer conversion for the
// compiler-rt library.  No range checking is performed; the behavior of this
// conversion is undefined for out of range values in the C standard.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

ARM_EABI_FNALIAS(f2iz, fixsfsi)

COMPILER_RT_ABI int
__fixsfsi(fp_t a) {
    // Break a into sign, exponent, significand
    const rep_t aRep = toRep(a);
    const rep_t aAbs = aRep & absMask;
    const int sign = aRep & signBit ? -1 : 1;
    const int exponent = (aAbs >> significandBits) - exponentBias;
    const rep_t significand = (aAbs & significandMask) | implicitBit;

    // If 0 < exponent < significandBits, right shift to get the result.
    if ((unsigned int)exponent < significandBits) {
        return sign * (significand >> (significandBits - exponent));
    }

    // If exponent is negative, the result is zero.
    else if (exponent < 0) {
        return 0;
    }

    // If significandBits < exponent, left shift to get the result.  This shift
    // may end up being larger than the type width, which incurs undefined
    // behavior, but the conversion itself is undefined in that case, so
    // whatever the compiler decides to do is fine.
    else {
        return sign * (significand << (exponent - significandBits));
    }
}
/* ===-- fixsfti.c - Implement __fixsfti -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixsfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: float is a IEEE 32 bit floating point type
 *             su_int is a 32 bit integral type
 *             value in float is representable in ti_int (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ti_int
__fixsfti(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0)
        return 0;
    ti_int s = (si_int)(fb.u & 0x80000000) >> 31;
    ti_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return (r ^ s) - s;
}

#endif
/* ===-- fixunsdfdi.c - Implement __fixunsdfdi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsdfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: double is a IEEE 64 bit floating point type
 *             du_int is a 64 bit integral type
 *             value in double is representable in du_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(d2ulz, fixunsdfdi)

COMPILER_RT_ABI du_int
__fixunsdfdi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
        return 0;
    udwords r;
    r.s.high = (fb.u.s.high & 0x000FFFFF) | 0x00100000;
    r.s.low = fb.u.s.low;
    if (e > 52)
        r.all <<= (e - 52);
    else
        r.all >>= (52 - e);
    return r.all;
}
/* ===-- fixunsdfsi.c - Implement __fixunsdfsi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsdfsi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: convert a to a unsigned int, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: double is a IEEE 64 bit floating point type
 *             su_int is a 32 bit integral type
 *             value in double is representable in su_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(d2uiz, fixunsdfsi)

COMPILER_RT_ABI su_int
__fixunsdfsi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
        return 0;
    return (
                0x80000000u                      |
                ((fb.u.s.high & 0x000FFFFF) << 11) |
                (fb.u.s.low >> 21)
           ) >> (31 - e);
}
/* ===-- fixunsdfti.c - Implement __fixunsdfti -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsdfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: double is a IEEE 64 bit floating point type
 *             tu_int is a 64 bit integral type
 *             value in double is representable in tu_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

tu_int
__fixunsdfti(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
        return 0;
    tu_int r = 0x0010000000000000uLL | (fb.u.all & 0x000FFFFFFFFFFFFFuLL);
    if (e > 52)
        r <<= (e - 52);
    else
        r >>= (52 - e);
    return r;
}

#endif
/* ===-- fixunssfdi.c - Implement __fixunssfdi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunssfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: float is a IEEE 32 bit floating point type
 *             du_int is a 64 bit integral type
 *             value in float is representable in du_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(f2ulz, fixunssfdi)

COMPILER_RT_ABI du_int
__fixunssfdi(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0 || (fb.u & 0x80000000))
        return 0;
    du_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return r;
}
/* ===-- fixunssfsi.c - Implement __fixunssfsi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunssfsi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: convert a to a unsigned int, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: float is a IEEE 32 bit floating point type
 *             su_int is a 32 bit integral type
 *             value in float is representable in su_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(f2uiz, fixunssfsi)

COMPILER_RT_ABI su_int
__fixunssfsi(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0 || (fb.u & 0x80000000))
        return 0;
    su_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return r;
}
/* ===-- fixunssfti.c - Implement __fixunssfti -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunssfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: float is a IEEE 32 bit floating point type
 *             tu_int is a 64 bit integral type
 *             value in float is representable in tu_int or is negative
 *                 (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

tu_int
__fixunssfti(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0 || (fb.u & 0x80000000))
        return 0;
    tu_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return r;
}

#endif
/* ===-- fixunsxfdi.c - Implement __fixunsxfdi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsxfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#if !_ARCH_PPC


/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             du_int is a 64 bit integral type
 *             value in long double is representable in du_int or is negative
 *                 (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

du_int
__fixunsxfdi(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0 || (fb.u.high.s.low & 0x00008000))
        return 0;
    return fb.u.low.all >> (63 - e);
}

#endif
/* ===-- fixunsxfsi.c - Implement __fixunsxfsi -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsxfsi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#if !_ARCH_PPC


/* Returns: convert a to a unsigned int, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             su_int is a 32 bit integral type
 *             value in long double is representable in su_int or is negative
 *                 (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

su_int
__fixunsxfsi(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0 || (fb.u.high.s.low & 0x00008000))
        return 0;
    return fb.u.low.s.high >> (31 - e);
}

#endif /* !_ARCH_PPC */
/* ===-- fixunsxfti.c - Implement __fixunsxfti -----------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixunsxfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             tu_int is a 64 bit integral type
 *             value in long double is representable in tu_int or is negative
 *                 (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

tu_int
__fixunsxfti(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0 || (fb.u.high.s.low & 0x00008000))
        return 0;
    tu_int r = fb.u.low.all;
    if (e > 63)
        r <<= (e - 63);
    else
        r >>= (63 - e);
    return r;
}

#endif
/* ===-- fixxfdi.c - Implement __fixxfdi -----------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixxfdi for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#if !_ARCH_PPC


/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             su_int is a 32 bit integral type
 *             value in long double is representable in di_int (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

di_int
__fixxfdi(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0)
        return 0;
    di_int s = -(si_int)((fb.u.high.s.low & 0x00008000) >> 15);
    di_int r = fb.u.low.all;
    r = (du_int)r >> (63 - e);
    return (r ^ s) - s;
}

#endif /* !_ARCH_PPC */
/* ===-- fixxfti.c - Implement __fixxfti -----------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __fixxfti for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             su_int is a 32 bit integral type
 *             value in long double is representable in ti_int (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

ti_int
__fixxfti(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0)
        return 0;
    ti_int s = -(si_int)((fb.u.high.s.low & 0x00008000) >> 15);
    ti_int r = fb.u.low.all;
    if (e > 63)
        r <<= (e - 63);
    else
        r >>= (63 - e);
    return (r ^ s) - s;
}

#endif /* __x86_64 */
//===-- lib/floatsidf.c - integer -> double-precision conversion --*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements integer to double-precision conversion for the
// compiler-rt library in the IEEE-754 default round-to-nearest, ties-to-even
// mode.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"


ARM_EABI_FNALIAS(i2d, floatsidf)

fp_t __floatsidf(int a) {

    const int aWidth = sizeof a * CHAR_BIT;

    // Handle zero as a special case to protect clz
    if (a == 0)
        return fromRep(0);

    // All other cases begin by extracting the sign and absolute value of a
    rep_t sign = 0;
    if (a < 0) {
        sign = signBit;
        a = -a;
    }

    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;

    // Shift a into the significand field and clear the implicit bit.  Extra
    // cast to unsigned int is necessary to get the correct behavior for
    // the input INT_MIN.
    const int shift = significandBits - exponent;
    result = (rep_t)(unsigned int)a << shift ^ implicitBit;

    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    // Insert the sign bit and return
    return fromRep(result | sign);
}
//===-- lib/floatsisf.c - integer -> single-precision conversion --*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements integer to single-precision conversion for the
// compiler-rt library in the IEEE-754 default round-to-nearest, ties-to-even
// mode.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"


ARM_EABI_FNALIAS(i2f, floatsisf)

fp_t __floatsisf(int a) {

    const int aWidth = sizeof a * CHAR_BIT;

    // Handle zero as a special case to protect clz
    if (a == 0)
        return fromRep(0);

    // All other cases begin by extracting the sign and absolute value of a
    rep_t sign = 0;
    if (a < 0) {
        sign = signBit;
        a = -a;
    }

    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;

    // Shift a into the significand field, rounding if it is a right-shift
    if (exponent <= significandBits) {
        const int shift = significandBits - exponent;
        result = (rep_t)a << shift ^ implicitBit;
    } else {
        const int shift = exponent - significandBits;
        result = (rep_t)a >> shift ^ implicitBit;
        rep_t round = (rep_t)a << (typeWidth - shift);
        if (round > signBit) result++;
        if (round == signBit) result += result & 1;
    }

    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    // Insert the sign bit and return
    return fromRep(result | sign);
}
/* ===-- floattidf.c - Implement __floattidf -------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floattidf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a double, rounding toward even.*/

/* Assumption: double is a IEEE 64 bit floating point type
 *            ti_int is a 128 bit integral type
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

si_int __clzti2(ti_int a);

double
__floattidf(ti_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(ti_int) * CHAR_BIT;
    const ti_int s = a >> (N-1);
    a = (a ^ s) - s;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > DBL_MANT_DIG)
    {
        /* start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                               12345678901234567890123456
         * 1 = msb 1 bit
         * P = bit DBL_MANT_DIG-1 bits to the right of 1
         * Q = bit DBL_MANT_DIG bits to the right of 1
         * R = "or" of all bits to the right of Q
         */
        switch (sd)
        {
        case DBL_MANT_DIG + 1:
            a <<= 1;
            break;
        case DBL_MANT_DIG + 2:
            break;
        default:
            a = ((tu_int)a >> (sd - (DBL_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + DBL_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to DBL_MANT_DIG or DBL_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << DBL_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    else
    {
        a <<= (DBL_MANT_DIG - sd);
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    double_bits fb;
    fb.u.s.high = ((su_int)s & 0x80000000) |        /* sign */
                ((e + 1023) << 20)      |        /* exponent */
                ((su_int)(a >> 32) & 0x000FFFFF); /* mantissa-high */
    fb.u.s.low = (su_int)a;                         /* mantissa-low */
    return fb.f;
}

#endif
/* ===-- floattisf.c - Implement __floattisf -------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floattisf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a float, rounding toward even. */

/* Assumption: float is a IEEE 32 bit floating point type
 *             ti_int is a 128 bit integral type
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

si_int __clzti2(ti_int a);

float
__floattisf(ti_int a)
{
    if (a == 0)
        return 0.0F;
    const unsigned N = sizeof(ti_int) * CHAR_BIT;
    const ti_int s = a >> (N-1);
    a = (a ^ s) - s;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > FLT_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
        * finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
        *                                                12345678901234567890123456
        *  1 = msb 1 bit
        *  P = bit FLT_MANT_DIG-1 bits to the right of 1
        *  Q = bit FLT_MANT_DIG bits to the right of 1
        *  R = "or" of all bits to the right of Q
        */
        switch (sd)
        {
        case FLT_MANT_DIG + 1:
            a <<= 1;
            break;
        case FLT_MANT_DIG + 2:
            break;
        default:
            a = ((tu_int)a >> (sd - (FLT_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + FLT_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to FLT_MANT_DIG or FLT_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << FLT_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    else
    {
        a <<= (FLT_MANT_DIG - sd);
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    float_bits fb;
    fb.u = ((su_int)s & 0x80000000) |  /* sign */
           ((e + 127) << 23)       |  /* exponent */
           ((su_int)a & 0x007FFFFF);   /* mantissa */
    return fb.f;
}

#endif
/* ===-- floattixf.c - Implement __floattixf -------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floattixf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a long double, rounding toward even. */

/* Assumption: long double is a IEEE 80 bit floating point type padded to 128 bits
 *             ti_int is a 128 bit integral type
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

si_int __clzti2(ti_int a);

long double
__floattixf(ti_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(ti_int) * CHAR_BIT;
    const ti_int s = a >> (N-1);
    a = (a ^ s) - s;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > LDBL_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                                12345678901234567890123456
         *  1 = msb 1 bit
         *  P = bit LDBL_MANT_DIG-1 bits to the right of 1
         *  Q = bit LDBL_MANT_DIG bits to the right of 1
         *  R = "or" of all bits to the right of Q
	      */
        switch (sd)
        {
        case LDBL_MANT_DIG + 1:
            a <<= 1;
            break;
        case LDBL_MANT_DIG + 2:
            break;
        default:
            a = ((tu_int)a >> (sd - (LDBL_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + LDBL_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to LDBL_MANT_DIG or LDBL_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << LDBL_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to LDBL_MANT_DIG bits */
    }
    else
    {
        a <<= (LDBL_MANT_DIG - sd);
        /* a is now rounded to LDBL_MANT_DIG bits */
    }
    long_double_bits fb;
    fb.u.high.s.low = ((su_int)s & 0x8000) |        /* sign */
                    (e + 16383);                  /* exponent */
    fb.u.low.all = (du_int)a;                     /* mantissa */
    return fb.f;
}

#endif
//===-- lib/floatunsidf.c - uint -> double-precision conversion ---*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements unsigned integer to double-precision conversion for the
// compiler-rt library in the IEEE-754 default round-to-nearest, ties-to-even
// mode.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"


ARM_EABI_FNALIAS(ui2d, floatunsidf)

fp_t __floatunsidf(unsigned int a) {

    const int aWidth = sizeof a * CHAR_BIT;

    // Handle zero as a special case to protect clz
    if (a == 0) return fromRep(0);

    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;

    // Shift a into the significand field and clear the implicit bit.
    const int shift = significandBits - exponent;
    result = (rep_t)a << shift ^ implicitBit;

    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    return fromRep(result);
}
//===-- lib/floatunsisf.c - uint -> single-precision conversion ---*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements unsigned integer to single-precision conversion for the
// compiler-rt library in the IEEE-754 default round-to-nearest, ties-to-even
// mode.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"


ARM_EABI_FNALIAS(ui2f, floatunsisf)

fp_t __floatunsisf(unsigned int a) {

    const int aWidth = sizeof a * CHAR_BIT;

    // Handle zero as a special case to protect clz
    if (a == 0) return fromRep(0);

    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    rep_t result;

    // Shift a into the significand field, rounding if it is a right-shift
    if (exponent <= significandBits) {
        const int shift = significandBits - exponent;
        result = (rep_t)a << shift ^ implicitBit;
    } else {
        const int shift = exponent - significandBits;
        result = (rep_t)a >> shift ^ implicitBit;
        rep_t round = (rep_t)a << (typeWidth - shift);
        if (round > signBit) result++;
        if (round == signBit) result += result & 1;
    }

    // Insert the exponent
    result += (rep_t)(exponent + exponentBias) << significandBits;
    return fromRep(result);
}
/* ===-- floatuntidf.c - Implement __floatuntidf ---------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floatuntidf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a double, rounding toward even. */

/* Assumption: double is a IEEE 64 bit floating point type
 *             tu_int is a 128 bit integral type
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

si_int __clzti2(ti_int a);

double
__floatuntidf(tu_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(tu_int) * CHAR_BIT;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > DBL_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                                12345678901234567890123456
         *  1 = msb 1 bit
         *  P = bit DBL_MANT_DIG-1 bits to the right of 1
         *  Q = bit DBL_MANT_DIG bits to the right of 1
         *  R = "or" of all bits to the right of Q
	 */
        switch (sd)
        {
        case DBL_MANT_DIG + 1:
            a <<= 1;
            break;
        case DBL_MANT_DIG + 2:
            break;
        default:
            a = (a >> (sd - (DBL_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + DBL_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to DBL_MANT_DIG or DBL_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << DBL_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    else
    {
        a <<= (DBL_MANT_DIG - sd);
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    double_bits fb;
    fb.u.s.high = ((e + 1023) << 20)      |        /* exponent */
                ((su_int)(a >> 32) & 0x000FFFFF); /* mantissa-high */
    fb.u.s.low = (su_int)a;                         /* mantissa-low */
    return fb.f;
}

#endif
/* ===-- floatuntisf.c - Implement __floatuntisf ---------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floatuntisf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a float, rounding toward even. */

/* Assumption: float is a IEEE 32 bit floating point type
 *             tu_int is a 128 bit integral type
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

si_int __clzti2(ti_int a);

float
__floatuntisf(tu_int a)
{
    if (a == 0)
        return 0.0F;
    const unsigned N = sizeof(tu_int) * CHAR_BIT;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > FLT_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                                12345678901234567890123456
         *  1 = msb 1 bit
         *  P = bit FLT_MANT_DIG-1 bits to the right of 1
         *  Q = bit FLT_MANT_DIG bits to the right of 1
         *  R = "or" of all bits to the right of Q
	 */
        switch (sd)
        {
        case FLT_MANT_DIG + 1:
            a <<= 1;
            break;
        case FLT_MANT_DIG + 2:
            break;
        default:
            a = (a >> (sd - (FLT_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + FLT_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to FLT_MANT_DIG or FLT_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << FLT_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    else
    {
        a <<= (FLT_MANT_DIG - sd);
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    float_bits fb;
    fb.u = ((e + 127) << 23)       |  /* exponent */
           ((su_int)a & 0x007FFFFF);  /* mantissa */
    return fb.f;
}

#endif
/* ===-- floatuntixf.c - Implement __floatuntixf ---------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __floatuntixf for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: convert a to a long double, rounding toward even. */

/* Assumption: long double is a IEEE 80 bit floating point type padded to 128 bits
 *             tu_int is a 128 bit integral type
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

si_int __clzti2(ti_int a);

long double
__floatuntixf(tu_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(tu_int) * CHAR_BIT;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > LDBL_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                                12345678901234567890123456
         *  1 = msb 1 bit
         *  P = bit LDBL_MANT_DIG-1 bits to the right of 1
         *  Q = bit LDBL_MANT_DIG bits to the right of 1
         *  R = "or" of all bits to the right of Q
	 */
        switch (sd)
        {
        case LDBL_MANT_DIG + 1:
            a <<= 1;
            break;
        case LDBL_MANT_DIG + 2:
            break;
        default:
            a = (a >> (sd - (LDBL_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + LDBL_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to LDBL_MANT_DIG or LDBL_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << LDBL_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to LDBL_MANT_DIG bits */
    }
    else
    {
        a <<= (LDBL_MANT_DIG - sd);
        /* a is now rounded to LDBL_MANT_DIG bits */
    }
    long_double_bits fb;
    fb.u.high.s.low = (e + 16383);                  /* exponent */
    fb.u.low.all = (du_int)a;                     /* mantissa */
    return fb.f;
}

#endif
/* ===-- int_util.c - Implement internal utilities --------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 */


__attribute__((weak))
__attribute__((visibility("hidden")))
void compilerrt_abort_impl(const char *file, int line, const char *function) {
  abort();
}
/* ===-- lshrdi3.c - Implement __lshrdi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __lshrdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: logical a >> b */

/* Precondition:  0 <= b < bits_in_dword */

ARM_EABI_FNALIAS(llsr, lshrdi3)

COMPILER_RT_ABI di_int
__lshrdi3(di_int a, si_int b)
{
    const int bits_in_word = (int)(sizeof(si_int) * CHAR_BIT);
    udwords input;
    udwords result;
    input.all = a;
    if (b & bits_in_word)  /* bits_in_word <= b < bits_in_dword */
    {
        result.s.high = 0;
        result.s.low = input.s.high >> (b - bits_in_word);
    }
    else  /* 0 <= b < bits_in_word */
    {
        if (b == 0)
            return a;
        result.s.high  = input.s.high >> b;
        result.s.low = (input.s.high << (bits_in_word - b)) | (input.s.low >> b);
    }
    return result.all;
}
/* ===-- lshrti3.c - Implement __lshrti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __lshrti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: logical a >> b */

/* Precondition:  0 <= b < bits_in_tword */

ti_int
__lshrti3(ti_int a, si_int b)
{
    const int bits_in_dword = (int)(sizeof(di_int) * CHAR_BIT);
    utwords input;
    utwords result;
    input.all = a;
    if (b & bits_in_dword)  /* bits_in_dword <= b < bits_in_tword */
    {
        result.s.high = 0;
        result.s.low = input.s.high >> (b - bits_in_dword);
    }
    else  /* 0 <= b < bits_in_dword */
    {
        if (b == 0)
            return a;
        result.s.high  = input.s.high >> b;
        result.s.low = (input.s.high << (bits_in_dword - b)) | (input.s.low >> b);
    }
    return result.all;
}

#endif /* __x86_64 */
/*===-- moddi3.c - Implement __moddi3 -------------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __moddi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


COMPILER_RT_ABI du_int __udivmoddi4(du_int a, du_int b, du_int* rem);

/* Returns: a % b */

COMPILER_RT_ABI di_int
__moddi3(di_int a, di_int b)
{
    const int bits_in_dword_m1 = (int)(sizeof(di_int) * CHAR_BIT) - 1;
    di_int s = b >> bits_in_dword_m1;  /* s = b < 0 ? -1 : 0 */
    b = (b ^ s) - s;                   /* negate if s == -1 */
    s = a >> bits_in_dword_m1;         /* s = a < 0 ? -1 : 0 */
    a = (a ^ s) - s;                   /* negate if s == -1 */
    di_int r;
    __udivmoddi4(a, b, (du_int*)&r);
    return (r ^ s) - s;                /* negate if s == -1 */
}
/* ===-- modsi3.c - Implement __modsi3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __modsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a % b */

COMPILER_RT_ABI si_int
__modsi3(si_int a, si_int b)
{
    return a - __divsi3(a, b) * b;
}
/* ===-- modti3.c - Implement __modti3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __modti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

tu_int __udivmodti4(tu_int a, tu_int b, tu_int* rem);

/*Returns: a % b */

ti_int
__modti3(ti_int a, ti_int b)
{
    const int bits_in_tword_m1 = (int)(sizeof(ti_int) * CHAR_BIT) - 1;
    ti_int s = b >> bits_in_tword_m1;  /* s = b < 0 ? -1 : 0 */
    b = (b ^ s) - s;                   /* negate if s == -1 */
    s = a >> bits_in_tword_m1;         /* s = a < 0 ? -1 : 0 */
    a = (a ^ s) - s;                   /* negate if s == -1 */
    ti_int r;
    __udivmodti4(a, b, (tu_int*)&r);
    return (r ^ s) - s;                /* negate if s == -1 */
}

#endif
/* ===-- muldc3.c - Implement __muldc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __muldc3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the product of a + ib and c + id */

double _Complex
__muldc3(double __a, double __b, double __c, double __d)
{
    double __ac = __a * __c;
    double __bd = __b * __d;
    double __ad = __a * __d;
    double __bc = __b * __c;
    double _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        int __recalc = 0;
        if (crt_isinf(__a) || crt_isinf(__b))
        {
            __a = crt_copysign(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysign(crt_isinf(__b) ? 1 : 0, __b);
            if (crt_isnan(__c))
                __c = crt_copysign(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysign(0, __d);
            __recalc = 1;
        }
        if (crt_isinf(__c) || crt_isinf(__d))
        {
            __c = crt_copysign(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysign(crt_isinf(__d) ? 1 : 0, __d);
            if (crt_isnan(__a))
                __a = crt_copysign(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysign(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (crt_isinf(__ac) || crt_isinf(__bd) ||
                          crt_isinf(__ad) || crt_isinf(__bc)))
        {
            if (crt_isnan(__a))
                __a = crt_copysign(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysign(0, __b);
            if (crt_isnan(__c))
                __c = crt_copysign(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysign(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = CRT_INFINITY * (__a * __c - __b * __d);
            __imag__ z = CRT_INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}
//===-- lib/muldf3.c - Double-precision multiplication ------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float multiplication
// with the IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

ARM_EABI_FNALIAS(dmul, muldf3)

COMPILER_RT_ABI fp_t
__muldf3(fp_t a, fp_t b) {

    const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
    const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
    const rep_t productSign = (toRep(a) ^ toRep(b)) & signBit;

    rep_t aSignificand = toRep(a) & significandMask;
    rep_t bSignificand = toRep(b) & significandMask;
    int scale = 0;

    // Detect if a or b is zero, denormal, infinity, or NaN.
    if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {

        const rep_t aAbs = toRep(a) & absMask;
        const rep_t bAbs = toRep(b) & absMask;

        // NaN * anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything * NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // infinity * non-zero = +/- infinity
            if (bAbs) return fromRep(aAbs | productSign);
            // infinity * zero = NaN
            else return fromRep(qnanRep);
        }

        if (bAbs == infRep) {
            // non-zero * infinity = +/- infinity
            if (aAbs) return fromRep(bAbs | productSign);
            // zero * infinity = NaN
            else return fromRep(qnanRep);
        }

        // zero * anything = +/- zero
        if (!aAbs) return fromRep(productSign);
        // anything * zero = +/- zero
        if (!bAbs) return fromRep(productSign);

        // one or both of a or b is denormal, the other (if applicable) is a
        // normal number.  Renormalize one or both of a and b, and set scale to
        // include the necessary exponent adjustment.
        if (aAbs < implicitBit) scale += normalize(&aSignificand);
        if (bAbs < implicitBit) scale += normalize(&bSignificand);
    }

    // Or in the implicit significand bit.  (If we fell through from the
    // denormal path it was already set by normalize( ), but setting it twice
    // won't hurt anything.)
    aSignificand |= implicitBit;
    bSignificand |= implicitBit;

    // Get the significand of a*b.  Before multiplying the significands, shift
    // one of them left to left-align it in the field.  Thus, the product will
    // have (exponentBits + 2) integral digits, all but two of which must be
    // zero.  Normalizing this result is just a conditional left-shift by one
    // and bumping the exponent accordingly.
    rep_t productHi, productLo;
    wideMultiply(aSignificand, bSignificand << exponentBits,
                 &productHi, &productLo);

    int productExponent = aExponent + bExponent - exponentBias + scale;

    // Normalize the significand, adjust exponent if needed.
    if (productHi & implicitBit) productExponent++;
    else wideLeftShift(&productHi, &productLo, 1);

    // If we have overflowed the type, return +/- infinity.
    if (productExponent >= maxExponent) return fromRep(infRep | productSign);

    if (productExponent <= 0) {
        // Result is denormal before rounding
        //
        // If the result is so small that it just underflows to zero, return
        // a zero of the appropriate sign.  Mathematically there is no need to
        // handle this case separately, but we make it a special case to
        // simplify the shift logic.
        const unsigned int shift = 1U - (unsigned int)productExponent;
        if (shift >= typeWidth) return fromRep(productSign);

        // Otherwise, shift the significand of the result so that the round
        // bit is the high bit of productLo.
        wideRightShiftWithSticky(&productHi, &productLo, shift);
    }

    else {
        // Result is normal before rounding; insert the exponent.
        productHi &= significandMask;
        productHi |= (rep_t)productExponent << significandBits;
    }

    // Insert the sign of the result:
    productHi |= productSign;

    // Final rounding.  The final result may overflow to infinity, or underflow
    // to zero, but those are the correct results in those cases.  We use the
    // default IEEE-754 round-to-nearest, ties-to-even rounding mode.
    if (productLo > signBit) productHi++;
    if (productLo == signBit) productHi += productHi & 1;
    return fromRep(productHi);
}
/* ===-- muldi3.c - Implement __muldi3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __muldi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a * b */

static
di_int
__muldsi3(su_int a, su_int b)
{
    dwords r;
    const int bits_in_word_2 = (int)(sizeof(si_int) * CHAR_BIT) / 2;
    const su_int lower_mask = (su_int)~0 >> bits_in_word_2;
    r.s.low = (a & lower_mask) * (b & lower_mask);
    su_int t = r.s.low >> bits_in_word_2;
    r.s.low &= lower_mask;
    t += (a >> bits_in_word_2) * (b & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_word_2;
    r.s.high = t >> bits_in_word_2;
    t = r.s.low >> bits_in_word_2;
    r.s.low &= lower_mask;
    t += (b >> bits_in_word_2) * (a & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_word_2;
    r.s.high += t >> bits_in_word_2;
    r.s.high += (a >> bits_in_word_2) * (b >> bits_in_word_2);
    return r.all;
}

/* Returns: a * b */

ARM_EABI_FNALIAS(lmul, muldi3)

COMPILER_RT_ABI di_int
__muldi3(di_int a, di_int b)
{
    dwords x;
    x.all = a;
    dwords y;
    y.all = b;
    dwords r;
    r.all = __muldsi3(x.s.low, y.s.low);
    r.s.high += x.s.high * y.s.low + x.s.low * y.s.high;
    return r.all;
}
/*===-- mulodi4.c - Implement __mulodi4 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulodi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a * b */

/* Effects: sets *overflow to 1  if a * b overflows */

di_int
__mulodi4(di_int a, di_int b, int* overflow)
{
    const int N = (int)(sizeof(di_int) * CHAR_BIT);
    const di_int MIN = (di_int)1 << (N-1);
    const di_int MAX = ~MIN;
    *overflow = 0;
    di_int result = a * b;
    if (a == MIN)
    {
        if (b != 0 && b != 1)
	    *overflow = 1;
	return result;
    }
    if (b == MIN)
    {
        if (a != 0 && a != 1)
	    *overflow = 1;
        return result;
    }
    di_int sa = a >> (N - 1);
    di_int abs_a = (a ^ sa) - sa;
    di_int sb = b >> (N - 1);
    di_int abs_b = (b ^ sb) - sb;
    if (abs_a < 2 || abs_b < 2)
        return result;
    if (sa == sb)
    {
        if (abs_a > MAX / abs_b)
            *overflow = 1;
    }
    else
    {
        if (abs_a > MIN / -abs_b)
            *overflow = 1;
    }
    return result;
}
/*===-- mulosi4.c - Implement __mulosi4 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulosi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a * b */

/* Effects: sets *overflow to 1  if a * b overflows */

si_int
__mulosi4(si_int a, si_int b, int* overflow)
{
    const int N = (int)(sizeof(si_int) * CHAR_BIT);
    const si_int MIN = (si_int)1 << (N-1);
    const si_int MAX = ~MIN;
    *overflow = 0;
    si_int result = a * b;
    if (a == MIN)
    {
        if (b != 0 && b != 1)
	    *overflow = 1;
	return result;
    }
    if (b == MIN)
    {
        if (a != 0 && a != 1)
	    *overflow = 1;
        return result;
    }
    si_int sa = a >> (N - 1);
    si_int abs_a = (a ^ sa) - sa;
    si_int sb = b >> (N - 1);
    si_int abs_b = (b ^ sb) - sb;
    if (abs_a < 2 || abs_b < 2)
        return result;
    if (sa == sb)
    {
        if (abs_a > MAX / abs_b)
            *overflow = 1;
    }
    else
    {
        if (abs_a > MIN / -abs_b)
            *overflow = 1;
    }
    return result;
}
/* ===-- mulsc3.c - Implement __mulsc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulsc3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: the product of a + ib and c + id */

float _Complex
__mulsc3(float __a, float __b, float __c, float __d)
{
    float __ac = __a * __c;
    float __bd = __b * __d;
    float __ad = __a * __d;
    float __bc = __b * __c;
    float _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        int __recalc = 0;
        if (crt_isinf(__a) || crt_isinf(__b))
        {
            __a = crt_copysignf(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignf(crt_isinf(__b) ? 1 : 0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignf(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignf(0, __d);
            __recalc = 1;
        }
        if (crt_isinf(__c) || crt_isinf(__d))
        {
            __c = crt_copysignf(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignf(crt_isinf(__d) ? 1 : 0, __d);
            if (crt_isnan(__a))
                __a = crt_copysignf(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignf(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (crt_isinf(__ac) || crt_isinf(__bd) ||
                          crt_isinf(__ad) || crt_isinf(__bc)))
        {
            if (crt_isnan(__a))
                __a = crt_copysignf(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignf(0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignf(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignf(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = CRT_INFINITY * (__a * __c - __b * __d);
            __imag__ z = CRT_INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}
//===-- lib/mulsf3.c - Single-precision multiplication ------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision soft-float multiplication
// with the IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

ARM_EABI_FNALIAS(fmul, mulsf3)

COMPILER_RT_ABI fp_t
__mulsf3(fp_t a, fp_t b) {

    const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
    const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
    const rep_t productSign = (toRep(a) ^ toRep(b)) & signBit;

    rep_t aSignificand = toRep(a) & significandMask;
    rep_t bSignificand = toRep(b) & significandMask;
    int scale = 0;

    // Detect if a or b is zero, denormal, infinity, or NaN.
    if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {

        const rep_t aAbs = toRep(a) & absMask;
        const rep_t bAbs = toRep(b) & absMask;

        // NaN * anything = qNaN
        if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
        // anything * NaN = qNaN
        if (bAbs > infRep) return fromRep(toRep(b) | quietBit);

        if (aAbs == infRep) {
            // infinity * non-zero = +/- infinity
            if (bAbs) return fromRep(aAbs | productSign);
            // infinity * zero = NaN
            else return fromRep(qnanRep);
        }

        if (bAbs == infRep) {
            // non-zero * infinity = +/- infinity
            if (aAbs) return fromRep(bAbs | productSign);
            // zero * infinity = NaN
            else return fromRep(qnanRep);
        }

        // zero * anything = +/- zero
        if (!aAbs) return fromRep(productSign);
        // anything * zero = +/- zero
        if (!bAbs) return fromRep(productSign);

        // one or both of a or b is denormal, the other (if applicable) is a
        // normal number.  Renormalize one or both of a and b, and set scale to
        // include the necessary exponent adjustment.
        if (aAbs < implicitBit) scale += normalize(&aSignificand);
        if (bAbs < implicitBit) scale += normalize(&bSignificand);
    }

    // Or in the implicit significand bit.  (If we fell through from the
    // denormal path it was already set by normalize( ), but setting it twice
    // won't hurt anything.)
    aSignificand |= implicitBit;
    bSignificand |= implicitBit;

    // Get the significand of a*b.  Before multiplying the significands, shift
    // one of them left to left-align it in the field.  Thus, the product will
    // have (exponentBits + 2) integral digits, all but two of which must be
    // zero.  Normalizing this result is just a conditional left-shift by one
    // and bumping the exponent accordingly.
    rep_t productHi, productLo;
    wideMultiply(aSignificand, bSignificand << exponentBits,
                 &productHi, &productLo);

    int productExponent = aExponent + bExponent - exponentBias + scale;

    // Normalize the significand, adjust exponent if needed.
    if (productHi & implicitBit) productExponent++;
    else wideLeftShift(&productHi, &productLo, 1);

    // If we have overflowed the type, return +/- infinity.
    if (productExponent >= maxExponent) return fromRep(infRep | productSign);

    if (productExponent <= 0) {
        // Result is denormal before rounding, the exponent is zero and we
        // need to shift the significand.
        wideRightShiftWithSticky(&productHi, &productLo, 1U - (unsigned)productExponent);
    }

    else {
        // Result is normal before rounding; insert the exponent.
        productHi &= significandMask;
        productHi |= (rep_t)productExponent << significandBits;
    }

    // Insert the sign of the result:
    productHi |= productSign;

    // Final rounding.  The final result may overflow to infinity, or underflow
    // to zero, but those are the correct results in those cases.
    if (productLo > signBit) productHi++;
    if (productLo == signBit) productHi += productHi & 1;
    return fromRep(productHi);
}
/* ===-- multi3.c - Implement __multi3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===

 * This file implements __multi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: a * b */

static
ti_int
__mulddi3(du_int a, du_int b)
{
    twords r;
    const int bits_in_dword_2 = (int)(sizeof(di_int) * CHAR_BIT) / 2;
    const du_int lower_mask = (du_int)~0 >> bits_in_dword_2;
    r.s.low = (a & lower_mask) * (b & lower_mask);
    du_int t = r.s.low >> bits_in_dword_2;
    r.s.low &= lower_mask;
    t += (a >> bits_in_dword_2) * (b & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_dword_2;
    r.s.high = t >> bits_in_dword_2;
    t = r.s.low >> bits_in_dword_2;
    r.s.low &= lower_mask;
    t += (b >> bits_in_dword_2) * (a & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_dword_2;
    r.s.high += t >> bits_in_dword_2;
    r.s.high += (a >> bits_in_dword_2) * (b >> bits_in_dword_2);
    return r.all;
}

/* Returns: a * b */

ti_int
__multi3(ti_int a, ti_int b)
{
    twords x;
    x.all = a;
    twords y;
    y.all = b;
    twords r;
    r.all = __mulddi3(x.s.low, y.s.low);
    r.s.high += x.s.high * y.s.low + x.s.low * y.s.high;
    return r.all;
}

#endif /* __x86_64 */
/*===-- mulvdi3.c - Implement __mulvdi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulvdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a * b */

/* Effects: aborts if a * b overflows */

di_int
__mulvdi3(di_int a, di_int b)
{
    const int N = (int)(sizeof(di_int) * CHAR_BIT);
    const di_int MIN = (di_int)1 << (N-1);
    const di_int MAX = ~MIN;
    if (a == MIN)
    {
        if (b == 0 || b == 1)
            return a * b;
        compilerrt_abort();
    }
    if (b == MIN)
    {
        if (a == 0 || a == 1)
            return a * b;
        compilerrt_abort();
    }
    di_int sa = a >> (N - 1);
    di_int abs_a = (a ^ sa) - sa;
    di_int sb = b >> (N - 1);
    di_int abs_b = (b ^ sb) - sb;
    if (abs_a < 2 || abs_b < 2)
        return a * b;
    if (sa == sb)
    {
        if (abs_a > MAX / abs_b)
            compilerrt_abort();
    }
    else
    {
        if (abs_a > MIN / -abs_b)
            compilerrt_abort();
    }
    return a * b;
}
/* ===-- mulvsi3.c - Implement __mulvsi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulvsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a * b */

/* Effects: aborts if a * b overflows */

si_int
__mulvsi3(si_int a, si_int b)
{
    const int N = (int)(sizeof(si_int) * CHAR_BIT);
    const si_int MIN = (si_int)1 << (N-1);
    const si_int MAX = ~MIN;
    if (a == MIN)
    {
        if (b == 0 || b == 1)
            return a * b;
        compilerrt_abort();
    }
    if (b == MIN)
    {
        if (a == 0 || a == 1)
            return a * b;
        compilerrt_abort();
    }
    si_int sa = a >> (N - 1);
    si_int abs_a = (a ^ sa) - sa;
    si_int sb = b >> (N - 1);
    si_int abs_b = (b ^ sb) - sb;
    if (abs_a < 2 || abs_b < 2)
        return a * b;
    if (sa == sb)
    {
        if (abs_a > MAX / abs_b)
            compilerrt_abort();
    }
    else
    {
        if (abs_a > MIN / -abs_b)
            compilerrt_abort();
    }
    return a * b;
}
/* ===-- mulxc3.c - Implement __mulxc3 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __mulxc3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#if !_ARCH_PPC


/* Returns: the product of a + ib and c + id */

long double _Complex
__mulxc3(long double __a, long double __b, long double __c, long double __d)
{
    long double __ac = __a * __c;
    long double __bd = __b * __d;
    long double __ad = __a * __d;
    long double __bc = __b * __c;
    long double _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        int __recalc = 0;
        if (crt_isinf(__a) || crt_isinf(__b))
        {
            __a = crt_copysignl(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysignl(crt_isinf(__b) ? 1 : 0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignl(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignl(0, __d);
            __recalc = 1;
        }
        if (crt_isinf(__c) || crt_isinf(__d))
        {
            __c = crt_copysignl(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysignl(crt_isinf(__d) ? 1 : 0, __d);
            if (crt_isnan(__a))
                __a = crt_copysignl(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignl(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (crt_isinf(__ac) || crt_isinf(__bd) ||
                          crt_isinf(__ad) || crt_isinf(__bc)))
        {
            if (crt_isnan(__a))
                __a = crt_copysignl(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysignl(0, __b);
            if (crt_isnan(__c))
                __c = crt_copysignl(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysignl(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = CRT_INFINITY * (__a * __c - __b * __d);
            __imag__ z = CRT_INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}

#endif
//===-- lib/negdf2.c - double-precision negation ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float negation.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

ARM_EABI_FNALIAS(dneg, negdf2)

fp_t __negdf2(fp_t a) {
    return fromRep(toRep(a) ^ signBit);
}
/* ===-- negdi2.c - Implement __negdi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __negdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: -a */

di_int
__negdi2(di_int a)
{
    /* Note: this routine is here for API compatibility; any sane compiler
     * should expand it inline.
     */
    return -a;
}
//===-- lib/negsf2.c - single-precision negation ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision soft-float negation.
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

ARM_EABI_FNALIAS(fneg, negsf2)

COMPILER_RT_ABI fp_t
__negsf2(fp_t a) {
    return fromRep(toRep(a) ^ signBit);
}
/* ===-- negti2.c - Implement __negti2 -------------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __negti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: -a */

ti_int
__negti2(ti_int a)
{
    /* Note: this routine is here for API compatibility; any sane compiler
     * should expand it inline.
     */
    return -a;
}

#endif
/* ===-- negvdi2.c - Implement __negvdi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __negvdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: -a */

/* Effects: aborts if -a overflows */

COMPILER_RT_ABI di_int
__negvdi2(di_int a)
{
    const di_int MIN = (di_int)1 << ((int)(sizeof(di_int) * CHAR_BIT)-1);
    if (a == MIN)
        compilerrt_abort();
    return -a;
}
/* ===-- negvsi2.c - Implement __negvsi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __negvsi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: -a */

/* Effects: aborts if -a overflows */

COMPILER_RT_ABI si_int
__negvsi2(si_int a)
{
    const si_int MIN = (si_int)1 << ((int)(sizeof(si_int) * CHAR_BIT)-1);
    if (a == MIN)
        compilerrt_abort();
    return -a;
}
/* ===-- paritydi2.c - Implement __paritydi2 -------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __paritydi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: 1 if number of bits is odd else returns 0 */

si_int COMPILER_RT_ABI __paritysi2(si_int a);

COMPILER_RT_ABI si_int
__paritydi2(di_int a)
{
    dwords x;
    x.all = a;
    return __paritysi2(x.s.high ^ x.s.low);
}
/* ===-- paritysi2.c - Implement __paritysi2 -------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __paritysi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: 1 if number of bits is odd else returns 0 */

COMPILER_RT_ABI si_int
__paritysi2(si_int a)
{
    su_int x = (su_int)a;
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    return (0x6996 >> (x & 0xF)) & 1;
}
/* ===-- parityti2.c - Implement __parityti2 -------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __parityti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: 1 if number of bits is odd else returns 0 */

si_int __paritydi2(di_int a);

si_int
__parityti2(ti_int a)
{
    twords x;
    x.all = a;
    return __paritydi2(x.s.high ^ x.s.low);
}

#endif
/* ===-- popcountdi2.c - Implement __popcountdi2 ----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __popcountdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: count of 1 bits */

COMPILER_RT_ABI si_int
__popcountdi2(di_int a)
{
    du_int x2 = (du_int)a;
    x2 = x2 - ((x2 >> 1) & 0x5555555555555555uLL);
    /* Every 2 bits holds the sum of every pair of bits (32) */
    x2 = ((x2 >> 2) & 0x3333333333333333uLL) + (x2 & 0x3333333333333333uLL);
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) (16) */
    x2 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0FuLL;
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) (8) */
    su_int x = (su_int)(x2 + (x2 >> 32));
    /* The lower 32 bits hold four 16 bit sums (5 significant bits). */
    /*   Upper 32 bits are garbage */
    x = x + (x >> 16);
    /* The lower 16 bits hold two 32 bit sums (6 significant bits). */
    /*   Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0x0000007F;  /* (7 significant bits) */
}
/* ===-- popcountsi2.c - Implement __popcountsi2 ---------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __popcountsi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: count of 1 bits */

COMPILER_RT_ABI si_int
__popcountsi2(si_int a)
{
    su_int x = (su_int)a;
    x = x - ((x >> 1) & 0x55555555);
    /* Every 2 bits holds the sum of every pair of bits */
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) */
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) */
    x = (x + (x >> 16));
    /* The lower 16 bits hold two 8 bit sums (5 significant bits).*/
    /*    Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0x0000003F;  /* (6 significant bits) */
}
/* ===-- popcountti2.c - Implement __popcountti2 ----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __popcountti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: count of 1 bits */

si_int
__popcountti2(ti_int a)
{
    tu_int x3 = (tu_int)a;
    x3 = x3 - ((x3 >> 1) & (((tu_int)0x5555555555555555uLL << 64) |
                                     0x5555555555555555uLL));
    /* Every 2 bits holds the sum of every pair of bits (64) */
    x3 = ((x3 >> 2) & (((tu_int)0x3333333333333333uLL << 64) | 0x3333333333333333uLL))
       + (x3 & (((tu_int)0x3333333333333333uLL << 64) | 0x3333333333333333uLL));
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) (32) */
    x3 = (x3 + (x3 >> 4))
       & (((tu_int)0x0F0F0F0F0F0F0F0FuLL << 64) | 0x0F0F0F0F0F0F0F0FuLL);
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) (16) */
    du_int x2 = (du_int)(x3 + (x3 >> 64));
    /* Every 8 bits holds the sum of every 8-set of bits (5 significant bits) (8) */
    su_int x = (su_int)(x2 + (x2 >> 32));
    /* Every 8 bits holds the sum of every 8-set of bits (6 significant bits) (4) */
    x = x + (x >> 16);
    /* Every 8 bits holds the sum of every 8-set of bits (7 significant bits) (2) */
    /* Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0xFF;  /* (8 significant bits) */
}

#endif
/* ===-- powidf2.cpp - Implement __powidf2 ---------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __powidf2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a ^ b */

COMPILER_RT_ABI double
__powidf2(double a, si_int b)
{
    const int recip = b < 0;
    double r = 1;
    while (1)
    {
        if (b & 1)
            r *= a;
        b /= 2;
        if (b == 0)
            break;
        a *= a;
    }
    return recip ? 1/r : r;
}
/*===-- powisf2.cpp - Implement __powisf2 ---------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __powisf2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a ^ b */

COMPILER_RT_ABI float
__powisf2(float a, si_int b)
{
    const int recip = b < 0;
    float r = 1;
    while (1)
    {
        if (b & 1)
            r *= a;
        b /= 2;
        if (b == 0)
            break;
        a *= a;
    }
    return recip ? 1/r : r;
}
/* ===-- powitf2.cpp - Implement __powitf2 ---------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __powitf2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if _ARCH_PPC

/* Returns: a ^ b */

long double
__powitf2(long double a, si_int b)
{
    const int recip = b < 0;
    long double r = 1;
    while (1)
    {
        if (b & 1)
            r *= a;
        b /= 2;
        if (b == 0)
            break;
        a *= a;
    }
    return recip ? 1/r : r;
}

#endif
/* ===-- powixf2.cpp - Implement __powixf2 ---------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __powixf2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#if !_ARCH_PPC


/* Returns: a ^ b */

long double
__powixf2(long double a, si_int b)
{
    const int recip = b < 0;
    long double r = 1;
    while (1)
    {
        if (b & 1)
            r *= a;
        b /= 2;
        if (b == 0)
            break;
        a *= a;
    }
    return recip ? 1/r : r;
}

#endif
//===-- lib/adddf3.c - Double-precision subtraction ---------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float subtraction with the
// IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-double.h"

fp_t COMPILER_RT_ABI __adddf3(fp_t a, fp_t b);


ARM_EABI_FNALIAS(dsub, subdf3)

// Subtraction; flip the sign bit of b and add.
COMPILER_RT_ABI fp_t
__subdf3(fp_t a, fp_t b) {
    return __adddf3(a, fromRep(toRep(b) ^ signBit));
}

/* FIXME: rsub for ARM EABI */
//===-- lib/subsf3.c - Single-precision subtraction ---------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements single-precision soft-float subtraction with the
// IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#include "crt-fp-single.h"

fp_t COMPILER_RT_ABI __addsf3(fp_t a, fp_t b);

ARM_EABI_FNALIAS(fsub, subsf3)

// Subtraction; flip the sign bit of b and add.
COMPILER_RT_ABI fp_t
__subsf3(fp_t a, fp_t b) {
    return __addsf3(a, fromRep(toRep(b) ^ signBit));
}

/* FIXME: rsub for ARM EABI */
/* ===-- subvdi3.c - Implement __subvdi3 -----------------------------------===
 *
 *                The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __subvdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a - b */

/* Effects: aborts if a - b overflows */

COMPILER_RT_ABI di_int
__subvdi3(di_int a, di_int b)
{
    di_int s = a - b;
    if (b >= 0)
    {
        if (s > a)
            compilerrt_abort();
    }
    else
    {
        if (s <= a)
            compilerrt_abort();
    }
    return s;
}
/* ===-- subvsi3.c - Implement __subvsi3 -----------------------------------===
 *
 *                The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __subvsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a - b */

/* Effects: aborts if a - b overflows */

COMPILER_RT_ABI si_int
__subvsi3(si_int a, si_int b)
{
    si_int s = a - b;
    if (b >= 0)
    {
        if (s > a)
            compilerrt_abort();
    }
    else
    {
        if (s <= a)
            compilerrt_abort();
    }
    return s;
}
/* ===-- subvti3.c - Implement __subvti3 -----------------------------------===
 *
 *      	       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __subvti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns: a - b */

/* Effects: aborts if a - b overflows */

ti_int
__subvti3(ti_int a, ti_int b)
{
    ti_int s = a - b;
    if (b >= 0)
    {
        if (s > a)
            compilerrt_abort();
    }
    else
    {
        if (s <= a)
            compilerrt_abort();
    }
    return s;
}

#endif /* __x86_64 */

//===-- lib/truncdfsf2.c - double -> single conversion ------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a fairly generic conversion from a wider to a narrower
// IEEE-754 floating-point type in the default (round to nearest, ties to even)
// rounding mode.  The constants and types defined following the includes below
// parameterize the conversion.
//
// This routine can be trivially adapted to support conversions to
// half-precision or from quad-precision. It does not support types that don't
// use the usual IEEE-754 interchange formats; specifically, some work would be
// needed to adapt it to (for example) the Intel 80-bit format or PowerPC
// double-double format.
//
// Note please, however, that this implementation is only intended to support
// *narrowing* operations; if you need to convert to a *wider* floating-point
// type (e.g. float -> double), then this routine will not do what you want it
// to.
//
// It also requires that integer types at least as large as both formats
// are available on the target platform; this may pose a problem when trying
// to add support for quad on some 32-bit systems, for example.
//
// Finally, the following assumptions are made:
//
// 1. floating-point types and integer types have the same endianness on the
//    target platform
//
// 2. quiet NaNs, if supported, are indicated by the leading bit of the
//    significand field being set
//
//===----------------------------------------------------------------------===//


typedef double dbl_src_t;
typedef uint64_t u64_src_rep_t;
#define SRC_REP_C UINT64_C

typedef float flt_dst_t;
typedef uint32_t u32_dst_rep_t;
#define DST_REP_C UINT32_C

// End of specialization parameters.  Two helper routines for conversion to and
// from the representation of floating-point data as integer values follow.

static inline u64_src_rep_t truncdfsf2_srcToRep(dbl_src_t x) {
    const union { dbl_src_t f; u64_src_rep_t i; } rep = {.f = x};
    return rep.i;
}

static inline flt_dst_t truncdfsf2_dstFromRep(u32_dst_rep_t x) {
    const union { flt_dst_t f; u32_dst_rep_t i; } rep = {.i = x};
    return rep.f;
}

// End helper routines.  Conversion implementation follows.

ARM_EABI_FNALIAS(d2f, truncdfsf2)

COMPILER_RT_ABI flt_dst_t
__truncdfsf2(dbl_src_t a) {
    static const int srcSigBits = 52;
    static const int dstSigBits = 23;

    // Various constants whose values follow from the type parameters.
    // Any reasonable optimizer will fold and propagate all of these.
    const int srcBits = sizeof(dbl_src_t)*CHAR_BIT;
    const int srcExpBits = srcBits - srcSigBits - 1;
    const int srcInfExp = (1 << srcExpBits) - 1;
    const int srcExpBias = srcInfExp >> 1;

    const u64_src_rep_t srcMinNormal = SRC_REP_C(1) << srcSigBits;
    const u64_src_rep_t significandMask_v = srcMinNormal - 1;
    const u64_src_rep_t srcInfinity = (u64_src_rep_t)srcInfExp << srcSigBits;
    const u64_src_rep_t srcSignMask = SRC_REP_C(1) << (srcSigBits + srcExpBits);
    const u64_src_rep_t srcAbsMask = srcSignMask - 1;
    const u64_src_rep_t roundMask = (SRC_REP_C(1) << (srcSigBits - dstSigBits)) - 1;
    const u64_src_rep_t halfway = SRC_REP_C(1) << (srcSigBits - dstSigBits - 1);

    const int dstBits = sizeof(flt_dst_t)*CHAR_BIT;
    const int dstExpBits = dstBits - dstSigBits - 1;
    const int dstInfExp = (1 << dstExpBits) - 1;
    const int dstExpBias = dstInfExp >> 1;

    const int underflowExponent = srcExpBias + 1 - dstExpBias;
    const int overflowExponent = srcExpBias + dstInfExp - dstExpBias;
    const u64_src_rep_t underflow = (u64_src_rep_t)underflowExponent << srcSigBits;
    const u64_src_rep_t overflow = (u64_src_rep_t)overflowExponent << srcSigBits;

    const u32_dst_rep_t dstQNaN = DST_REP_C(1) << (dstSigBits - 1);
    const u32_dst_rep_t dstNaNCode = dstQNaN - 1;

    // Break a into a sign and representation of the absolute value
    const u64_src_rep_t aRep = truncdfsf2_srcToRep(a);
    const u64_src_rep_t aAbs = aRep & srcAbsMask;
    const u64_src_rep_t sign = aRep & srcSignMask;
    u32_dst_rep_t absResult;

    if (aAbs - underflow < aAbs - overflow) {
        // The exponent of a is within the range of normal numbers in the
        // destination format.  We can convert by simply right-shifting with
        // rounding and adjusting the exponent.
        absResult = aAbs >> (srcSigBits - dstSigBits);
        absResult -= (u32_dst_rep_t)(srcExpBias - dstExpBias) << dstSigBits;

        const u64_src_rep_t roundBits = aAbs & roundMask;

        // Round to nearest
        if (roundBits > halfway)
            absResult++;

        // Ties to even
        else if (roundBits == halfway)
            absResult += absResult & 1;
    }

    else if (aAbs > srcInfinity) {
        // a is NaN.
        // Conjure the result by beginning with infinity, setting the qNaN
        // bit and inserting the (truncated) trailing NaN field.
        absResult = (u32_dst_rep_t)dstInfExp << dstSigBits;
        absResult |= dstQNaN;
        absResult |= aAbs & dstNaNCode;
    }

    else if (aAbs > overflow) {
        // a overflows to infinity.
        absResult = (u32_dst_rep_t)dstInfExp << dstSigBits;
    }

    else {
        // a underflows on conversion to the destination type or is an exact
        // zero.  The result may be a denormal or zero.  Extract the exponent
        // to get the shift amount for the denormalization.
        const int aExp = aAbs >> srcSigBits;
        const int shift = srcExpBias - dstExpBias - aExp + 1;

        const u64_src_rep_t significand = (aRep & significandMask_v) | srcMinNormal;

        // Right shift by the denormalization amount with sticky.
        if (shift > srcSigBits) {
            absResult = 0;
        } else {
            const bool sticky = significand << (srcBits - shift);
            u64_src_rep_t denormalizedSignificand = significand >> shift | sticky;
            absResult = denormalizedSignificand >> (srcSigBits - dstSigBits);
            const u64_src_rep_t roundBits = denormalizedSignificand & roundMask;
            // Round to nearest
            if (roundBits > halfway)
                absResult++;
            // Ties to even
            else if (roundBits == halfway)
                absResult += absResult & 1;
        }
    }

    // Apply the signbit to (dst_t)abs(a).
    const u32_dst_rep_t result = absResult | sign >> (srcBits - dstBits);
    return truncdfsf2_dstFromRep(result);

}
/* ===-- ucmpdi2.c - Implement __ucmpdi2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ucmpdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns:  if (a <  b) returns 0
 *           if (a == b) returns 1
 *           if (a >  b) returns 2
 */

COMPILER_RT_ABI si_int
__ucmpdi2(du_int a, du_int b)
{
    udwords x;
    x.all = a;
    udwords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#ifdef __ARM_EABI__
/* Returns: if (a <  b) returns -1
*           if (a == b) returns  0
*           if (a >  b) returns  1
*/
COMPILER_RT_ABI si_int
__aeabi_ulcmp(di_int a, di_int b)
{
	return __ucmpdi2(a, b) - 1;
}
#endif

/* ===-- ucmpti2.c - Implement __ucmpti2 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ucmpti2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Returns:  if (a <  b) returns 0
 *           if (a == b) returns 1
 *           if (a >  b) returns 2
 */

si_int
__ucmpti2(tu_int a, tu_int b)
{
    utwords x;
    x.all = a;
    utwords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#endif
/* ===-- udivdi3.c - Implement __udivdi3 -----------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivdi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


du_int COMPILER_RT_ABI __udivmoddi4(du_int a, du_int b, du_int* rem);

/* Returns: a / b */

COMPILER_RT_ABI du_int
__udivdi3(du_int a, du_int b)
{
    return __udivmoddi4(a, b, 0);
}
/* ===-- udivmoddi4.c - Implement __udivmoddi4 -----------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivmoddi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Effects: if rem != 0, *rem = a % b
 * Returns: a / b
 */

/* Translated from Figure 3-40 of The PowerPC Compiler Writer's Guide */

COMPILER_RT_ABI du_int
__udivmoddi4(du_int a, du_int b, du_int* rem)
{
    const unsigned n_uword_bits = sizeof(su_int) * CHAR_BIT;
    const unsigned n_udword_bits = sizeof(du_int) * CHAR_BIT;
    udwords n;
    n.all = a;
    udwords d;
    d.all = b;
    udwords q;
    udwords r;
    unsigned sr;
    /* special cases, X is unknown, K != 0 */
    if (n.s.high == 0)
    {
        if (d.s.high == 0)
        {
            /* 0 X
             * ---
             * 0 X
             */
            if (rem)
                *rem = n.s.low % d.s.low;
            return n.s.low / d.s.low;
        }
        /* 0 X
         * ---
         * K X
         */
        if (rem)
            *rem = n.s.low;
        return 0;
    }
    /* n.s.high != 0 */
    if (d.s.low == 0)
    {
        if (d.s.high == 0)
        {
            /* K X
             * ---
             * 0 0
             */
            if (rem)
                *rem = n.s.high % d.s.low;
            return n.s.high / d.s.low;
        }
        /* d.s.high != 0 */
        if (n.s.low == 0)
        {
            /* K 0
             * ---
             * K 0
             */
            if (rem)
            {
                r.s.high = n.s.high % d.s.high;
                r.s.low = 0;
                *rem = r.all;
            }
            return n.s.high / d.s.high;
        }
        /* K K
         * ---
         * K 0
         */
        if ((d.s.high & (d.s.high - 1)) == 0)     /* if d is a power of 2 */
        {
            if (rem)
            {
                r.s.low = n.s.low;
                r.s.high = n.s.high & (d.s.high - 1);
                *rem = r.all;
            }
            return n.s.high >> __builtin_ctz(d.s.high);
        }
        /* K K
         * ---
         * K 0
         */
        sr = __builtin_clz(d.s.high) - __builtin_clz(n.s.high);
        /* 0 <= sr <= n_uword_bits - 2 or sr large */
        if (sr > n_uword_bits - 2)
        {
           if (rem)
                *rem = n.all;
            return 0;
        }
        ++sr;
        /* 1 <= sr <= n_uword_bits - 1 */
        /* q.all = n.all << (n_udword_bits - sr); */
        q.s.low = 0;
        q.s.high = n.s.low << (n_uword_bits - sr);
        /* r.all = n.all >> sr; */
        r.s.high = n.s.high >> sr;
        r.s.low = (n.s.high << (n_uword_bits - sr)) | (n.s.low >> sr);
    }
    else  /* d.s.low != 0 */
    {
        if (d.s.high == 0)
        {
            /* K X
             * ---
             * 0 K
             */
            if ((d.s.low & (d.s.low - 1)) == 0)     /* if d is a power of 2 */
            {
                if (rem)
                    *rem = n.s.low & (d.s.low - 1);
                if (d.s.low == 1)
                    return n.all;
                sr = __builtin_ctz(d.s.low);
                q.s.high = n.s.high >> sr;
                q.s.low = (n.s.high << (n_uword_bits - sr)) | (n.s.low >> sr);
                return q.all;
            }
            /* K X
             * ---
             *0 K
             */
            sr = 1 + n_uword_bits + __builtin_clz(d.s.low) - __builtin_clz(n.s.high);
            /* 2 <= sr <= n_udword_bits - 1
             * q.all = n.all << (n_udword_bits - sr);
             * r.all = n.all >> sr;
             * if (sr == n_uword_bits)
             * {
             *     q.s.low = 0;
             *     q.s.high = n.s.low;
             *     r.s.high = 0;
             *     r.s.low = n.s.high;
             * }
             * else if (sr < n_uword_bits)  // 2 <= sr <= n_uword_bits - 1
             * {
             *     q.s.low = 0;
             *     q.s.high = n.s.low << (n_uword_bits - sr);
             *     r.s.high = n.s.high >> sr;
             *     r.s.low = (n.s.high << (n_uword_bits - sr)) | (n.s.low >> sr);
             * }
             * else              // n_uword_bits + 1 <= sr <= n_udword_bits - 1
             * {
             *     q.s.low = n.s.low << (n_udword_bits - sr);
             *     q.s.high = (n.s.high << (n_udword_bits - sr)) |
             *              (n.s.low >> (sr - n_uword_bits));
             *     r.s.high = 0;
             *     r.s.low = n.s.high >> (sr - n_uword_bits);
             * }
             */
            q.s.low =  (n.s.low << (n_udword_bits - sr)) &
                     ((si_int)(n_uword_bits - sr) >> (n_uword_bits-1));
            q.s.high = ((n.s.low << ( n_uword_bits - sr))                       &
                     ((si_int)(sr - n_uword_bits - 1) >> (n_uword_bits-1))) |
                     (((n.s.high << (n_udword_bits - sr))                     |
                     (n.s.low >> (sr - n_uword_bits)))                        &
                     ((si_int)(n_uword_bits - sr) >> (n_uword_bits-1)));
            r.s.high = (n.s.high >> sr) &
                     ((si_int)(sr - n_uword_bits) >> (n_uword_bits-1));
            r.s.low =  ((n.s.high >> (sr - n_uword_bits))                       &
                     ((si_int)(n_uword_bits - sr - 1) >> (n_uword_bits-1))) |
                     (((n.s.high << (n_uword_bits - sr))                      |
                     (n.s.low >> sr))                                         &
                     ((si_int)(sr - n_uword_bits) >> (n_uword_bits-1)));
        }
        else
        {
            /* K X
             * ---
             * K K
             */
            sr = __builtin_clz(d.s.high) - __builtin_clz(n.s.high);
            /* 0 <= sr <= n_uword_bits - 1 or sr large */
            if (sr > n_uword_bits - 1)
            {
               if (rem)
                    *rem = n.all;
                return 0;
            }
            ++sr;
            /* 1 <= sr <= n_uword_bits */
            /*  q.all = n.all << (n_udword_bits - sr); */
            q.s.low = 0;
            q.s.high = n.s.low << (n_uword_bits - sr);
            /* r.all = n.all >> sr;
             * if (sr < n_uword_bits)
             * {
             *     r.s.high = n.s.high >> sr;
             *     r.s.low = (n.s.high << (n_uword_bits - sr)) | (n.s.low >> sr);
             * }
             * else
             * {
             *     r.s.high = 0;
             *     r.s.low = n.s.high;
             * }
             */
            r.s.high = (n.s.high >> sr) &
                     ((si_int)(sr - n_uword_bits) >> (n_uword_bits-1));
            r.s.low = (n.s.high << (n_uword_bits - sr)) |
                    ((n.s.low >> sr)                  &
                    ((si_int)(sr - n_uword_bits) >> (n_uword_bits-1)));
        }
    }
    /* Not a special case
     * q and r are initialized with:
     * q.all = n.all << (n_udword_bits - sr);
     * r.all = n.all >> sr;
     * 1 <= sr <= n_udword_bits - 1
     */
    su_int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r.s.high = (r.s.high << 1) | (r.s.low  >> (n_uword_bits - 1));
        r.s.low  = (r.s.low  << 1) | (q.s.high >> (n_uword_bits - 1));
        q.s.high = (q.s.high << 1) | (q.s.low  >> (n_uword_bits - 1));
        q.s.low  = (q.s.low  << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const di_int s = (di_int)(d.all - r.all - 1) >> (n_udword_bits - 1);
        carry = s & 1;
        r.all -= d.all & s;
    }
    q.all = (q.all << 1) | carry;
    if (rem)
        *rem = r.all;
    return q.all;
}
/*===-- udivmodsi4.c - Implement __udivmodsi4 ------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivmodsi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


extern su_int COMPILER_RT_ABI __udivsi3(su_int n, su_int d);


/* Returns: a / b, *rem = a % b  */

COMPILER_RT_ABI su_int
__udivmodsi4(su_int a, su_int b, su_int* rem)
{
  si_int d = __udivsi3(a,b);
  *rem = a - (d*b);
  return d;
}


/* ===-- udivmodti4.c - Implement __udivmodti4 -----------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivmodti4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

/* Effects: if rem != 0, *rem = a % b
 * Returns: a / b
 */

/* Translated from Figure 3-40 of The PowerPC Compiler Writer's Guide */

tu_int
__udivmodti4(tu_int a, tu_int b, tu_int* rem)
{
    const unsigned n_udword_bits = sizeof(du_int) * CHAR_BIT;
    const unsigned n_utword_bits = sizeof(tu_int) * CHAR_BIT;
    utwords n;
    n.all = a;
    utwords d;
    d.all = b;
    utwords q;
    utwords r;
    unsigned sr;
    /* special cases, X is unknown, K != 0 */
    if (n.s.high == 0)
    {
        if (d.s.high == 0)
        {
            /* 0 X
             * ---
             * 0 X
             */
            if (rem)
                *rem = n.s.low % d.s.low;
            return n.s.low / d.s.low;
        }
        /* 0 X
         * ---
         * K X
         */
        if (rem)
            *rem = n.s.low;
        return 0;
    }
    /* n.s.high != 0 */
    if (d.s.low == 0)
    {
        if (d.s.high == 0)
        {
            /* K X
             * ---
             * 0 0
             */
            if (rem)
                *rem = n.s.high % d.s.low;
            return n.s.high / d.s.low;
        }
        /* d.s.high != 0 */
        if (n.s.low == 0)
        {
            /* K 0
             * ---
             * K 0
             */
            if (rem)
            {
                r.s.high = n.s.high % d.s.high;
                r.s.low = 0;
                *rem = r.all;
            }
            return n.s.high / d.s.high;
        }
        /* K K
         * ---
         * K 0
         */
        if ((d.s.high & (d.s.high - 1)) == 0)     /* if d is a power of 2 */
        {
            if (rem)
            {
                r.s.low = n.s.low;
                r.s.high = n.s.high & (d.s.high - 1);
                *rem = r.all;
            }
            return n.s.high >> __builtin_ctzll(d.s.high);
        }
        /* K K
         * ---
         * K 0
         */
        sr = __builtin_clzll(d.s.high) - __builtin_clzll(n.s.high);
        /* 0 <= sr <= n_udword_bits - 2 or sr large */
        if (sr > n_udword_bits - 2)
        {
           if (rem)
                *rem = n.all;
            return 0;
        }
        ++sr;
        /* 1 <= sr <= n_udword_bits - 1 */
        /* q.all = n.all << (n_utword_bits - sr); */
        q.s.low = 0;
        q.s.high = n.s.low << (n_udword_bits - sr);
        /* r.all = n.all >> sr; */
        r.s.high = n.s.high >> sr;
        r.s.low = (n.s.high << (n_udword_bits - sr)) | (n.s.low >> sr);
    }
    else  /* d.s.low != 0 */
    {
        if (d.s.high == 0)
        {
            /* K X
             * ---
             * 0 K
             */
            if ((d.s.low & (d.s.low - 1)) == 0)     /* if d is a power of 2 */
            {
                if (rem)
                    *rem = n.s.low & (d.s.low - 1);
                if (d.s.low == 1)
                    return n.all;
                sr = __builtin_ctzll(d.s.low);
                q.s.high = n.s.high >> sr;
                q.s.low = (n.s.high << (n_udword_bits - sr)) | (n.s.low >> sr);
                return q.all;
            }
            /* K X
             * ---
             * 0 K
             */
            sr = 1 + n_udword_bits + __builtin_clzll(d.s.low)
                                   - __builtin_clzll(n.s.high);
            /* 2 <= sr <= n_utword_bits - 1
             * q.all = n.all << (n_utword_bits - sr);
             * r.all = n.all >> sr;
             * if (sr == n_udword_bits)
             * {
             *     q.s.low = 0;
             *     q.s.high = n.s.low;
             *     r.s.high = 0;
             *     r.s.low = n.s.high;
             * }
             * else if (sr < n_udword_bits)  // 2 <= sr <= n_udword_bits - 1
             * {
             *     q.s.low = 0;
             *     q.s.high = n.s.low << (n_udword_bits - sr);
             *     r.s.high = n.s.high >> sr;
             *     r.s.low = (n.s.high << (n_udword_bits - sr)) | (n.s.low >> sr);
             * }
             * else              // n_udword_bits + 1 <= sr <= n_utword_bits - 1
             * {
             *     q.s.low = n.s.low << (n_utword_bits - sr);
             *     q.s.high = (n.s.high << (n_utword_bits - sr)) |
             *              (n.s.low >> (sr - n_udword_bits));
             *     r.s.high = 0;
             *     r.s.low = n.s.high >> (sr - n_udword_bits);
             * }
             */
            q.s.low =  (n.s.low << (n_utword_bits - sr)) &
                     ((di_int)(int)(n_udword_bits - sr) >> (n_udword_bits-1));
            q.s.high = ((n.s.low << ( n_udword_bits - sr))                        &
                     ((di_int)(int)(sr - n_udword_bits - 1) >> (n_udword_bits-1))) |
                     (((n.s.high << (n_utword_bits - sr))                       |
                     (n.s.low >> (sr - n_udword_bits)))                         &
                     ((di_int)(int)(n_udword_bits - sr) >> (n_udword_bits-1)));
            r.s.high = (n.s.high >> sr) &
                     ((di_int)(int)(sr - n_udword_bits) >> (n_udword_bits-1));
            r.s.low =  ((n.s.high >> (sr - n_udword_bits))                        &
                     ((di_int)(int)(n_udword_bits - sr - 1) >> (n_udword_bits-1))) |
                     (((n.s.high << (n_udword_bits - sr))                       |
                     (n.s.low >> sr))                                           &
                     ((di_int)(int)(sr - n_udword_bits) >> (n_udword_bits-1)));
        }
        else
        {
            /* K X
             * ---
             * K K
             */
            sr = __builtin_clzll(d.s.high) - __builtin_clzll(n.s.high);
            /*0 <= sr <= n_udword_bits - 1 or sr large */
            if (sr > n_udword_bits - 1)
            {
               if (rem)
                    *rem = n.all;
                return 0;
            }
            ++sr;
            /* 1 <= sr <= n_udword_bits */
            /* q.all = n.all << (n_utword_bits - sr); */
            q.s.low = 0;
            q.s.high = n.s.low << (n_udword_bits - sr);
            /* r.all = n.all >> sr;
             * if (sr < n_udword_bits)
             * {
             *     r.s.high = n.s.high >> sr;
             *     r.s.low = (n.s.high << (n_udword_bits - sr)) | (n.s.low >> sr);
             * }
             * else
             * {
             *     r.s.high = 0;
             *     r.s.low = n.s.high;
             * }
             */
            r.s.high = (n.s.high >> sr) &
                     ((di_int)(int)(sr - n_udword_bits) >> (n_udword_bits-1));
            r.s.low = (n.s.high << (n_udword_bits - sr)) |
                    ((n.s.low >> sr)                   &
                    ((di_int)(int)(sr - n_udword_bits) >> (n_udword_bits-1)));
        }
    }
    /* Not a special case
     * q and r are initialized with:
     * q.all = n.all << (n_utword_bits - sr);
     * r.all = n.all >> sr;
     * 1 <= sr <= n_utword_bits - 1
     */
    su_int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r.s.high = (r.s.high << 1) | (r.s.low  >> (n_udword_bits - 1));
        r.s.low  = (r.s.low  << 1) | (q.s.high >> (n_udword_bits - 1));
        q.s.high = (q.s.high << 1) | (q.s.low  >> (n_udword_bits - 1));
        q.s.low  = (q.s.low  << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *     r.all -= d.all;
         *      carry = 1;
         * }
         */
        const ti_int s = (ti_int)(d.all - r.all - 1) >> (n_utword_bits - 1);
        carry = s & 1;
        r.all -= d.all & s;
    }
    q.all = (q.all << 1) | carry;
    if (rem)
        *rem = r.all;
    return q.all;
}

#endif /* __x86_64 */
/* ===-- udivsi3.c - Implement __udivsi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a / b */

/* Translated from Figure 3-40 of The PowerPC Compiler Writer's Guide */

ARM_EABI_FNALIAS(uidiv, udivsi3)

/* This function should not call __divsi3! */
COMPILER_RT_ABI su_int
__udivsi3(su_int n, su_int d)
{
    const unsigned n_uword_bits = sizeof(su_int) * CHAR_BIT;
    su_int q;
    su_int r;
    unsigned sr;
    /* special cases */
    if (d == 0)
        return 0; /* ?! */
    if (n == 0)
        return 0;
    sr = __builtin_clz(d) - __builtin_clz(n);
    /* 0 <= sr <= n_uword_bits - 1 or sr large */
    if (sr > n_uword_bits - 1)  /* d > r */
        return 0;
    if (sr == n_uword_bits - 1)  /* d == 1 */
        return n;
    ++sr;
    /* 1 <= sr <= n_uword_bits - 1 */
    /* Not a special case */
    q = n << (n_uword_bits - sr);
    r = n >> sr;
    su_int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r = (r << 1) | (q >> (n_uword_bits - 1));
        q = (q << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const si_int s = (si_int)(d - r - 1) >> (n_uword_bits - 1);
        carry = s & 1;
        r -= d & s;
    }
    q = (q << 1) | carry;
    return q;
}
/* ===-- udivti3.c - Implement __udivti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

tu_int __udivmodti4(tu_int a, tu_int b, tu_int* rem);

/* Returns: a / b */

tu_int
__udivti3(tu_int a, tu_int b)
{
    return __udivmodti4(a, b, 0);
}

#endif /* __x86_64 */
/* ===-- umoddi3.c - Implement __umoddi3 -----------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __umoddi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


du_int COMPILER_RT_ABI __udivmoddi4(du_int a, du_int b, du_int* rem);

/* Returns: a % b */

COMPILER_RT_ABI du_int
__umoddi3(du_int a, du_int b)
{
    du_int r;
    __udivmoddi4(a, b, &r);
    return r;
}
/* ===-- umodsi3.c - Implement __umodsi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __umodsi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


/* Returns: a % b */

su_int COMPILER_RT_ABI __udivsi3(su_int a, su_int b);

COMPILER_RT_ABI su_int
__umodsi3(su_int a, su_int b)
{
    return a - __udivsi3(a, b) * b;
}
/* ===-- umodti3.c - Implement __umodti3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __umodti3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */


#if __x86_64

tu_int __udivmodti4(tu_int a, tu_int b, tu_int* rem);

/* Returns: a % b */

tu_int
__umodti3(tu_int a, tu_int b)
{
    tu_int r;
    __udivmodti4(a, b, &r);
    return r;
}

#endif
/* This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 */

/* double __floatdidf(di_int a); */

#ifdef __x86_64__

double __floatdidf(int64_t a)
{
	return (double)a;
}

#endif /* __x86_64__ */
/* This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 */

#ifdef __x86_64__

float __floatdisf(int64_t a)
{
	return (float)a;
}

#endif /* __x86_64__ */
/* This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 */

/* long double __floatdixf(di_int a); */

#ifdef __x86_64__

long double __floatdixf(int64_t a)
{
	return (long double)a;
}

#endif /* __i386__ */
