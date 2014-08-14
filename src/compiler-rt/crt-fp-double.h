//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#undef rep_t
#undef srep_t
#undef fp_t
#undef rep_clz
#undef wideMultiply
#undef toRep
#undef fromRep
#undef normalize
#undef wideLeftShift
#undef wideRightShiftWithSticky

#define rep_t _dp_rep_t
#define srep_t _dp_srep_t
#define fp_t _dp_fp_t
#define rep_clz _dp_rep_clz
#define wideMultiply _dp_wideMultiply
#define toRep _dp_toRep
#define fromRep _dp_fromRep
#define normalize _dp_normalize
#define wideLeftShift _dp_wideLeftShift
#define wideRightShiftWithSticky _dp_wideRightShiftWithSticky

#undef SINGLE_PRECISION
#undef DOUBLE_PRECISION
#define DOUBLE_PRECISION

#undef REP_C
#undef significandBits
#define REP_C UINT64_C
#define significandBits 52

#undef typeWidth
#undef exponentBits
#undef maxExponent
#undef exponentBias
#define typeWidth       (sizeof(rep_t)*CHAR_BIT)
#define exponentBits    (typeWidth - significandBits - 1)
#define maxExponent     ((1 << exponentBits) - 1)
#define exponentBias    (maxExponent >> 1)

#undef implicitBit
#undef significandMask
#undef signBit
#undef absMask
#undef exponentMask
#undef oneRep
#undef infRep
#undef quietBit
#undef qnanRep
#define implicitBit     (REP_C(1) << significandBits)
#define significandMask (implicitBit - 1U)
#define signBit         (REP_C(1) << (significandBits + exponentBits))
#define absMask         (signBit - 1U)
#define exponentMask    (absMask ^ significandMask)
#define oneRep          ((rep_t)exponentBias << significandBits)
#define infRep          exponentMask
#define quietBit        (implicitBit >> 1)
#define qnanRep         (exponentMask | quietBit)
