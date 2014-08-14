/* This file contains source code copied from the llvm clang project. */
/* See the COPYING file distributed with this project for more information. */
/*===-----------------------------------------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef SSE_H
#define SSE_H

#define _mm_load_pd1(dp) _mm_load1_pd(dp)

#define _mm_slli_si128(a, count) __extension__ ({ \
  __m128i __a = (a); \
  (__m128i)__builtin_ia32_pslldqi128(__a, (count)*8); })

#define _mm_srli_si128(a, count) __extension__ ({ \
  __m128i __a = (a); \
  (__m128i)__builtin_ia32_psrldqi128(__a, (count)*8); })

#define _mm_shuffle_epi32(a, imm) __extension__ ({ \
  __m128i __a = (a); \
  (__m128i)__builtin_shufflevector((__v4si)__a, (__v4si) _mm_set1_epi32(0), \
                                   (imm) & 0x3, ((imm) & 0xc) >> 2, \
                                   ((imm) & 0x30) >> 4, ((imm) & 0xc0) >> 6); })

#define _mm_shufflelo_epi16(a, imm) __extension__ ({ \
  __m128i __a = (a); \
  (__m128i)__builtin_shufflevector((__v8hi)__a, (__v8hi) _mm_set1_epi16(0), \
                                   (imm) & 0x3, ((imm) & 0xc) >> 2, \
                                   ((imm) & 0x30) >> 4, ((imm) & 0xc0) >> 6, \
                                   4, 5, 6, 7); })

#define _mm_shufflehi_epi16(a, imm) __extension__ ({ \
  __m128i __a = (a); \
  (__m128i)__builtin_shufflevector((__v8hi)__a, (__v8hi) _mm_set1_epi16(0), \
                                   0, 1, 2, 3, \
                                   4 + (((imm) & 0x03) >> 0), \
                                   4 + (((imm) & 0x0c) >> 2), \
                                   4 + (((imm) & 0x30) >> 4), \
                                   4 + (((imm) & 0xc0) >> 6)); })

#define _mm_shuffle_pd(a, b, i) __extension__ ({ \
  __m128d __a = (a); \
  __m128d __b = (b); \
  __builtin_shufflevector(__a, __b, (i) & 1, (((i) & 2) >> 1) + 2); })

#define _MM_SHUFFLE2(x, y) (((x) << 1) | (y))

typedef long long __m64 __attribute__((__vector_size__(8)));
typedef int __v2si __attribute__((__vector_size__(8)));
typedef short __v4hi __attribute__((__vector_size__(8)));
typedef char __v8qi __attribute__((__vector_size__(8)));

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_empty(void) {
    __builtin_ia32_emms();
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_si64(int __i) {
    return (__m64) __builtin_ia32_vec_init_v2si(__i, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_si32(__m64 __m) {
    return __builtin_ia32_vec_ext_v2si((__v2si) __m, 0);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_m64(long long __i) {
    return (__m64) __i;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtm64_si64(__m64 __m) {
    return (long long) __m;
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_packs_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_packsswb((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_packs_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_packssdw((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_packs_pu16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_packuswb((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpckhbw((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpckhwd((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpckhdq((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpcklbw((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpcklwd((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_punpckldq((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_add_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_add_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_add_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddd((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_adds_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddsb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_adds_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddsw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_adds_pu8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddusb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_adds_pu16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_paddusw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sub_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sub_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sub_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubd((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_subs_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubsb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_subs_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubsw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_subs_pu8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubusb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_subs_pu16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_psubusw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_madd_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pmaddwd((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pmulhw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mullo_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pmullw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sll_pi16(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psllw((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_slli_pi16(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psllwi((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sll_pi32(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_pslld((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_slli_pi32(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_pslldi((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sll_si64(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psllq(__m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_slli_si64(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psllqi(__m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sra_pi16(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psraw((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srai_pi16(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psrawi((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sra_pi32(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psrad((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srai_pi32(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psradi((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srl_pi16(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psrlw((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srli_pi16(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psrlwi((__v4hi) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srl_pi32(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psrld((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srli_pi32(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psrldi((__v2si) __m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srl_si64(__m64 __m, __m64 __count) {
    return (__m64) __builtin_ia32_psrlq(__m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_srli_si64(__m64 __m, int __count) {
    return (__m64) __builtin_ia32_psrlqi(__m, __count);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_and_si64(__m64 __m1, __m64 __m2) {
    return __builtin_ia32_pand(__m1, __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_andnot_si64(__m64 __m1, __m64 __m2) {
    return __builtin_ia32_pandn(__m1, __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_or_si64(__m64 __m1, __m64 __m2) {
    return __builtin_ia32_por(__m1, __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_xor_si64(__m64 __m1, __m64 __m2) {
    return __builtin_ia32_pxor(__m1, __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpeqb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpeqw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpeqd((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pi8(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpgtb((__v8qi) __m1, (__v8qi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pi16(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpgtw((__v4hi) __m1, (__v4hi) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pi32(__m64 __m1, __m64 __m2) {
    return (__m64) __builtin_ia32_pcmpgtd((__v2si) __m1, (__v2si) __m2);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_setzero_si64(void) {

    return (__m64) {
        0LL
    };
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set_pi32(int __i1, int __i0) {
    return (__m64) __builtin_ia32_vec_init_v2si(__i0, __i1);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set_pi16(short __s3, short __s2, short __s1, short __s0) {
    return (__m64) __builtin_ia32_vec_init_v4hi(__s0, __s1, __s2, __s3);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set_pi8(char __b7, char __b6, char __b5, char __b4, char __b3, char __b2,
        char __b1, char __b0) {
    return (__m64) __builtin_ia32_vec_init_v8qi(__b0, __b1, __b2, __b3,
            __b4, __b5, __b6, __b7);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set1_pi32(int __i) {
    return _mm_set_pi32(__i, __i);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set1_pi16(short __w) {
    return _mm_set_pi16(__w, __w, __w, __w);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_set1_pi8(char __b) {
    return _mm_set_pi8(__b, __b, __b, __b, __b, __b, __b, __b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_setr_pi32(int __i0, int __i1) {
    return _mm_set_pi32(__i1, __i0);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_setr_pi16(short __w0, short __w1, short __w2, short __w3) {
    return _mm_set_pi16(__w3, __w2, __w1, __w0);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_setr_pi8(char __b0, char __b1, char __b2, char __b3, char __b4, char __b5,
        char __b6, char __b7) {
    return _mm_set_pi8(__b7, __b6, __b5, __b4, __b3, __b2, __b1, __b0);
}

typedef int __v4si __attribute__((__vector_size__(16)));
typedef float __v4sf __attribute__((__vector_size__(16)));
typedef float __m128 __attribute__((__vector_size__(16)));

// librcd does not support side-effect-full malloc/free. We might need to implement a replacement to this in the future though.

/*
extern int posix_memalign(void **memptr, size_t alignment, size_t size);

static __inline__ void *__attribute__((__always_inline__, __nodebug__,
        __malloc__))
_mm_malloc(size_t size, size_t align) {
    if (align == 1) {
        return malloc(size);
    }

    if (!(align & (align - 1)) && align < sizeof (void *))
        align = sizeof (void *);

    void *mallocedMemory;

    if (posix_memalign(&mallocedMemory, align, size))
        return 0;

    return mallocedMemory;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_free(void *p) {
    free(p);
}*/

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ss(__m128 a, __m128 b) {
    a[0] += b[0];
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ps(__m128 a, __m128 b) {
    return a + b;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ss(__m128 a, __m128 b) {
    a[0] -= b[0];
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ps(__m128 a, __m128 b) {
    return a - b;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ss(__m128 a, __m128 b) {
    a[0] *= b[0];
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ps(__m128 a, __m128 b) {
    return a * b;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ss(__m128 a, __m128 b) {
    a[0] /= b[0];
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ps(__m128 a, __m128 b) {
    return a / b;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ss(__m128 a) {
    return __builtin_ia32_sqrtss(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ps(__m128 a) {
    return __builtin_ia32_sqrtps(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ss(__m128 a) {
    return __builtin_ia32_rcpss(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ps(__m128 a) {
    return __builtin_ia32_rcpps(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt_ss(__m128 a) {
    return __builtin_ia32_rsqrtss(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt_ps(__m128 a) {
    return __builtin_ia32_rsqrtps(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ss(__m128 a, __m128 b) {
    return __builtin_ia32_minss(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ps(__m128 a, __m128 b) {
    return __builtin_ia32_minps(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ss(__m128 a, __m128 b) {
    return __builtin_ia32_maxss(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ps(__m128 a, __m128 b) {
    return __builtin_ia32_maxps(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_and_ps(__m128 a, __m128 b) {
    return (__m128) ((__v4si) a & (__v4si) b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_andnot_ps(__m128 a, __m128 b) {
    return (__m128) (~(__v4si) a & (__v4si) b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_or_ps(__m128 a, __m128 b) {
    return (__m128) ((__v4si) a | (__v4si) b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_xor_ps(__m128 a, __m128 b) {
    return (__m128) ((__v4si) a ^ (__v4si) b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 1);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 1);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 2);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 2);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(b, a, 1);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(b, a, 1);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(b, a, 2);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(b, a, 2);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 4);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 4);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 6);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 6);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(b, a, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(b, a, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(b, a, 6);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(b, a, 6);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 7);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 7);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_ss(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpss(a, b, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_ps(__m128 a, __m128 b) {
    return (__m128) __builtin_ia32_cmpps(a, b, 3);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comieq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comilt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comile(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comigt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comige(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_ss(__m128 a, __m128 b) {
    return __builtin_ia32_comineq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomieq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomilt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomile(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomigt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomige(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_ss(__m128 a, __m128 b) {
    return __builtin_ia32_ucomineq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_si32(__m128 a) {
    return __builtin_ia32_cvtss2si(a);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvt_ss2si(__m128 a) {
    return _mm_cvtss_si32(a);
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_si64(__m128 a) {
    return __builtin_ia32_cvtss2si64(a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pi32(__m128 a) {
    return (__m64) __builtin_ia32_cvtps2pi(a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvt_ps2pi(__m128 a) {
    return _mm_cvtps_pi32(a);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvttss_si32(__m128 a) {
    return a[0];
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtt_ss2si(__m128 a) {
    return _mm_cvttss_si32(a);
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttss_si64(__m128 a) {
    return a[0];
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvttps_pi32(__m128 a) {
    return (__m64) __builtin_ia32_cvttps2pi(a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtt_ps2pi(__m128 a) {
    return _mm_cvttps_pi32(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_ss(__m128 a, int b) {
    a[0] = b;
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvt_si2ss(__m128 a, int b) {
    return _mm_cvtsi32_ss(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_ss(__m128 a, long long b) {
    a[0] = b;
    return a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi32_ps(__m128 a, __m64 b) {
    return __builtin_ia32_cvtpi2ps(a, (__v2si) b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvt_pi2ps(__m128 a, __m64 b) {
    return _mm_cvtpi32_ps(a, b);
}

static __inline__ float __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_f32(__m128 a) {
    return a[0];
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadh_pi(__m128 a, const __m64 *p) {
    typedef float __mm_loadh_pi_v2f32 __attribute__((__vector_size__(8)));

    struct __mm_loadh_pi_struct {
        __mm_loadh_pi_v2f32 u;
    } __attribute__((__packed__, __may_alias__));
    __mm_loadh_pi_v2f32 b = ((struct __mm_loadh_pi_struct*) p)->u;
    __m128 bb = __builtin_shufflevector(b, b, 0, 1, 0, 1);
    return __builtin_shufflevector(a, bb, 0, 1, 4, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadl_pi(__m128 a, const __m64 *p) {
    typedef float __mm_loadl_pi_v2f32 __attribute__((__vector_size__(8)));

    struct __mm_loadl_pi_struct {
        __mm_loadl_pi_v2f32 u;
    } __attribute__((__packed__, __may_alias__));
    __mm_loadl_pi_v2f32 b = ((struct __mm_loadl_pi_struct*) p)->u;
    __m128 bb = __builtin_shufflevector(b, b, 0, 1, 0, 1);
    return __builtin_shufflevector(a, bb, 4, 5, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ss(const float *p) {

    struct __mm_load_ss_struct {
        float u;
    } __attribute__((__packed__, __may_alias__));
    float u = ((struct __mm_load_ss_struct*) p)->u;

    return (__m128) {
        u, 0, 0, 0
    };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load1_ps(const float *p) {

    struct __mm_load1_ps_struct {
        float u;
    } __attribute__((__packed__, __may_alias__));
    float u = ((struct __mm_load1_ps_struct*) p)->u;

    return (__m128) {
        u, u, u, u
    };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ps(const float *p) {
    return *(__m128*) p;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadu_ps(const float *p) {

    struct __loadu_ps {
        __m128 v;
    } __attribute__((__packed__, __may_alias__));
    return ((struct __loadu_ps*) p)->v;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadr_ps(const float *p) {
    __m128 a = _mm_load_ps(p);
    return __builtin_shufflevector(a, a, 3, 2, 1, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ss(float w) {
    return (__m128) {
        w, 0, 0, 0
    };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set1_ps(float w) {
    return (__m128) {
        w, w, w, w
    };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps1(float w) {
    return _mm_set1_ps(w);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps(float z, float y, float x, float w) {
    return (__m128) {
        w, x, y, z
    };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_setr_ps(float z, float y, float x, float w) {
    return (__m128) {
        z, y, x, w
    };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void) {
    return (__m128) {
        0, 0, 0, 0
    };
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeh_pi(__m64 *p, __m128 a) {
    __builtin_ia32_storehps((__v2si *) p, a);
}

static __inline__ void __attribute__((__always_inline__))
_mm_storel_pi(__m64 *p, __m128 a) {
    __builtin_ia32_storelps((__v2si *) p, a);
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ss(float *p, __m128 a) {

    struct __mm_store_ss_struct {
        float u;
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_store_ss_struct*) p)->u = a[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_ps(float *p, __m128 a) {
    __builtin_ia32_storeups(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store1_ps(float *p, __m128 a) {
    a = __builtin_shufflevector(a, a, 0, 0, 0, 0);
    _mm_storeu_ps(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps1(float *p, __m128 a) {
    return _mm_store1_ps(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps(float *p, __m128 a) {
    *(__m128 *) p = a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_ps(float *p, __m128 a) {
    a = __builtin_shufflevector(a, a, 3, 2, 1, 0);
    _mm_store_ps(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_pi(__m64 *p, __m64 a) {
    __builtin_ia32_movntq(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_ps(float *p, __m128 a) {
    __builtin_ia32_movntps(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_sfence(void) {
    __builtin_ia32_sfence();
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_extract_pi16(__m64 a, int n) {
    __v4hi b = (__v4hi) a;
    return (unsigned short) b[n & 3];
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_insert_pi16(__m64 a, int d, int n) {
    __v4hi b = (__v4hi) a;
    b[n & 3] = d;
    return (__m64) b;
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_max_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pmaxsw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_max_pu8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pmaxub((__v8qi) a, (__v8qi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_min_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pminsw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_min_pu8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pminub((__v8qi) a, (__v8qi) b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_pi8(__m64 a) {
    return __builtin_ia32_pmovmskb((__v8qi) a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_pu16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pmulhuw((__v4hi) a, (__v4hi) b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskmove_si64(__m64 d, __m64 n, char *p) {
    __builtin_ia32_maskmovq((__v8qi) d, (__v8qi) n, p);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_avg_pu8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pavgb((__v8qi) a, (__v8qi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_avg_pu16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pavgw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sad_pu8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_psadbw((__v8qi) a, (__v8qi) b);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_getcsr(void) {
    return __builtin_ia32_stmxcsr();
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_setcsr(unsigned int i) {
    __builtin_ia32_ldmxcsr(i);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_ps(__m128 a, __m128 b) {
    return __builtin_shufflevector(a, b, 2, 6, 3, 7);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_ps(__m128 a, __m128 b) {
    return __builtin_shufflevector(a, b, 0, 4, 1, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_move_ss(__m128 a, __m128 b) {
    return __builtin_shufflevector(a, b, 4, 1, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehl_ps(__m128 a, __m128 b) {
    return __builtin_shufflevector(a, b, 6, 7, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movelh_ps(__m128 a, __m128 b) {
    return __builtin_shufflevector(a, b, 0, 1, 4, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi16_ps(__m64 a) {
    __m64 b, c;
    __m128 r;

    b = _mm_setzero_si64();
    b = _mm_cmpgt_pi16(b, a);
    c = _mm_unpackhi_pi16(a, b);
    r = _mm_setzero_ps();
    r = _mm_cvtpi32_ps(r, c);
    r = _mm_movelh_ps(r, r);
    c = _mm_unpacklo_pi16(a, b);
    r = _mm_cvtpi32_ps(r, c);

    return r;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpu16_ps(__m64 a) {
    __m64 b, c;
    __m128 r;

    b = _mm_setzero_si64();
    c = _mm_unpackhi_pi16(a, b);
    r = _mm_setzero_ps();
    r = _mm_cvtpi32_ps(r, c);
    r = _mm_movelh_ps(r, r);
    c = _mm_unpacklo_pi16(a, b);
    r = _mm_cvtpi32_ps(r, c);

    return r;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi8_ps(__m64 a) {
    __m64 b;

    b = _mm_setzero_si64();
    b = _mm_cmpgt_pi8(b, a);
    b = _mm_unpacklo_pi8(a, b);

    return _mm_cvtpi16_ps(b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpu8_ps(__m64 a) {
    __m64 b;

    b = _mm_setzero_si64();
    b = _mm_unpacklo_pi8(a, b);

    return _mm_cvtpi16_ps(b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi32x2_ps(__m64 a, __m64 b) {
    __m128 c;

    c = _mm_setzero_ps();
    c = _mm_cvtpi32_ps(c, b);
    c = _mm_movelh_ps(c, c);

    return _mm_cvtpi32_ps(c, a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pi16(__m128 a) {
    __m64 b, c;

    b = _mm_cvtps_pi32(a);
    a = _mm_movehl_ps(a, a);
    c = _mm_cvtps_pi32(a);

    return _mm_packs_pi16(b, c);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pi8(__m128 a) {
    __m64 b, c;

    b = _mm_cvtps_pi16(a);
    c = _mm_setzero_si64();

    return _mm_packs_pi16(b, c);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_ps(__m128 a) {
    return __builtin_ia32_movmskps(a);
}

typedef double __m128d __attribute__((__vector_size__(16)));
typedef long long __m128i __attribute__((__vector_size__(16)));

typedef double __v2df __attribute__((__vector_size__(16)));
typedef long long __v2di __attribute__((__vector_size__(16)));
typedef short __v8hi __attribute__((__vector_size__(16)));
typedef char __v16qi __attribute__((__vector_size__(16)));

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_sd(__m128d a, __m128d b) {
    a[0] += b[0];
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_pd(__m128d a, __m128d b) {
    return a + b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_sd(__m128d a, __m128d b) {
    a[0] -= b[0];
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_pd(__m128d a, __m128d b) {
    return a - b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_sd(__m128d a, __m128d b) {
    a[0] *= b[0];
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_pd(__m128d a, __m128d b) {
    return a * b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_sd(__m128d a, __m128d b) {
    a[0] /= b[0];
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_pd(__m128d a, __m128d b) {
    return a / b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_sd(__m128d a, __m128d b) {
    __m128d c = __builtin_ia32_sqrtsd(b);

    return (__m128d) {
        c[0], a[1]
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_pd(__m128d a) {
    return __builtin_ia32_sqrtpd(a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_sd(__m128d a, __m128d b) {
    return __builtin_ia32_minsd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_pd(__m128d a, __m128d b) {
    return __builtin_ia32_minpd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_sd(__m128d a, __m128d b) {
    return __builtin_ia32_maxsd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_pd(__m128d a, __m128d b) {
    return __builtin_ia32_maxpd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_and_pd(__m128d a, __m128d b) {
    return (__m128d) ((__v4si) a & (__v4si) b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_andnot_pd(__m128d a, __m128d b) {
    return (__m128d) (~(__v4si) a & (__v4si) b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_or_pd(__m128d a, __m128d b) {
    return (__m128d) ((__v4si) a | (__v4si) b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_xor_pd(__m128d a, __m128d b) {
    return (__m128d) ((__v4si) a ^ (__v4si) b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(b, a, 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(b, a, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 7);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 3);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 4);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 5);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(a, b, 6);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(b, a, 5);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_pd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmppd(b, a, 6);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(b, a, 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(b, a, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 7);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 3);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 4);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 5);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(a, b, 6);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(b, a, 5);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_sd(__m128d a, __m128d b) {
    return (__m128d) __builtin_ia32_cmpsd(b, a, 6);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdeq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdlt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdle(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdgt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdge(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_sd(__m128d a, __m128d b) {
    return __builtin_ia32_comisdneq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdeq(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdlt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdle(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdgt(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdge(a, b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_sd(__m128d a, __m128d b) {
    return __builtin_ia32_ucomisdneq(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_ps(__m128d a) {
    return __builtin_ia32_cvtpd2ps(a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pd(__m128 a) {
    return __builtin_ia32_cvtps2pd(a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_pd(__m128i a) {
    return __builtin_ia32_cvtdq2pd((__v4si) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_epi32(__m128d a) {
    return __builtin_ia32_cvtpd2dq(a);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si32(__m128d a) {
    return __builtin_ia32_cvtsd2si(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_ss(__m128 a, __m128d b) {
    a[0] = b[0];
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_sd(__m128d a, int b) {
    a[0] = b;
    return a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_sd(__m128d a, __m128 b) {
    a[0] = b[0];
    return a;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvttpd_epi32(__m128d a) {
    return (__m128i) __builtin_ia32_cvttpd2dq(a);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si32(__m128d a) {
    return a[0];
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_pi32(__m128d a) {
    return (__m64) __builtin_ia32_cvtpd2pi(a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvttpd_pi32(__m128d a) {
    return (__m64) __builtin_ia32_cvttpd2pi(a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi32_pd(__m64 a) {
    return __builtin_ia32_cvtpi2pd((__v2si) a);
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_f64(__m128d a) {
    return a[0];
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load_pd(double const *dp) {
    return *(__m128d*) dp;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load1_pd(double const *dp) {
    struct __mm_load1_pd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    double u = ((struct __mm_load1_pd_struct*) dp)->u;
    return (__m128d) {
        u, u
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadr_pd(double const *dp) {
    __m128d u = *(__m128d*) dp;
    return __builtin_shufflevector(u, u, 1, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadu_pd(double const *dp) {
    struct __loadu_pd {
        __m128d v;
    } __attribute__((packed, may_alias));
    return ((struct __loadu_pd*) dp)->v;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load_sd(double const *dp) {

    struct __mm_load_sd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    double u = ((struct __mm_load_sd_struct*) dp)->u;

    return (__m128d) {
        u, 0
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadh_pd(__m128d a, double const *dp) {
    struct __mm_loadh_pd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    double u = ((struct __mm_loadh_pd_struct*) dp)->u;
    return (__m128d) {
        a[0], u
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadl_pd(__m128d a, double const *dp) {
    struct __mm_loadl_pd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    double u = ((struct __mm_loadl_pd_struct*) dp)->u;
    return (__m128d) {
        u, a[1]
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_sd(double w) {

    return (__m128d) {
        w, 0
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set1_pd(double w) {

    return (__m128d) {
        w, w
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_pd(double w, double x) {

    return (__m128d) {
        x, w
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setr_pd(double w, double x) {

    return (__m128d) {
        w, x
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setzero_pd(void) {

    return (__m128d) {
        0, 0
    };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_move_sd(__m128d a, __m128d b) {

    return (__m128d) {
        b[0], a[1]
    };
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_sd(double *dp, __m128d a) {

    struct __mm_store_sd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_store_sd_struct*) dp)->u = a[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store1_pd(double *dp, __m128d a) {

    struct __mm_store1_pd_struct {
        double u[2];
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_store1_pd_struct*) dp)->u[0] = a[0];
    ((struct __mm_store1_pd_struct*) dp)->u[1] = a[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_pd(double *dp, __m128d a) {
    *(__m128d *) dp = a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_pd(double *dp, __m128d a) {
    __builtin_ia32_storeupd(dp, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_pd(double *dp, __m128d a) {
    a = __builtin_shufflevector(a, a, 1, 0);
    *(__m128d *) dp = a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeh_pd(double *dp, __m128d a) {

    struct __mm_storeh_pd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_storeh_pd_struct*) dp)->u = a[1];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_pd(double *dp, __m128d a) {

    struct __mm_storeh_pd_struct {
        double u;
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_storeh_pd_struct*) dp)->u = a[0];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi8(__m128i a, __m128i b) {
    return (__m128i) ((__v16qi) a + (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi16(__m128i a, __m128i b) {
    return (__m128i) ((__v8hi) a + (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi32(__m128i a, __m128i b) {
    return (__m128i) ((__v4si) a + (__v4si) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_add_si64(__m64 a, __m64 b) {
    return a + b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi64(__m128i a, __m128i b) {
    return a + b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_paddsb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_paddsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_paddusb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_paddusw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pavgb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pavgw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_madd_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmaddwd128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmaxsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmaxub128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pminsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pminub128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmulhw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epu16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmulhuw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mullo_epi16(__m128i a, __m128i b) {
    return (__m128i) ((__v8hi) a * (__v8hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mul_su32(__m64 a, __m64 b) {
    return __builtin_ia32_pmuludq((__v2si) a, (__v2si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epu32(__m128i a, __m128i b) {
    return __builtin_ia32_pmuludq128((__v4si) a, (__v4si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sad_epu8(__m128i a, __m128i b) {
    return __builtin_ia32_psadbw128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi8(__m128i a, __m128i b) {
    return (__m128i) ((__v16qi) a - (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi16(__m128i a, __m128i b) {
    return (__m128i) ((__v8hi) a - (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi32(__m128i a, __m128i b) {
    return (__m128i) ((__v4si) a - (__v4si) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sub_si64(__m64 a, __m64 b) {
    return a - b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi64(__m128i a, __m128i b) {
    return a - b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psubsb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psubsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psubusb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psubusw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_and_si128(__m128i a, __m128i b) {
    return a & b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_andnot_si128(__m128i a, __m128i b) {
    return ~a & b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_or_si128(__m128i a, __m128i b) {
    return a | b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_xor_si128(__m128i a, __m128i b) {
    return a ^ b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi16(__m128i a, int count) {
    return (__m128i) __builtin_ia32_psllwi128((__v8hi) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi16(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_psllw128((__v8hi) a, (__v8hi) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi32(__m128i a, int count) {
    return (__m128i) __builtin_ia32_pslldi128((__v4si) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi32(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_pslld128((__v4si) a, (__v4si) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi64(__m128i a, int count) {
    return __builtin_ia32_psllqi128(a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi64(__m128i a, __m128i count) {
    return __builtin_ia32_psllq128(a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi16(__m128i a, int count) {
    return (__m128i) __builtin_ia32_psrawi128((__v8hi) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi16(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_psraw128((__v8hi) a, (__v8hi) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi32(__m128i a, int count) {
    return (__m128i) __builtin_ia32_psradi128((__v4si) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi32(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_psrad128((__v4si) a, (__v4si) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi16(__m128i a, int count) {
    return (__m128i) __builtin_ia32_psrlwi128((__v8hi) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi16(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_psrlw128((__v8hi) a, (__v8hi) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi32(__m128i a, int count) {
    return (__m128i) __builtin_ia32_psrldi128((__v4si) a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi32(__m128i a, __m128i count) {
    return (__m128i) __builtin_ia32_psrld128((__v4si) a, (__v4si) count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi64(__m128i a, int count) {
    return __builtin_ia32_psrlqi128(a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi64(__m128i a, __m128i count) {
    return __builtin_ia32_psrlq128(a, count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi8(__m128i a, __m128i b) {
    return (__m128i) ((__v16qi) a == (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi16(__m128i a, __m128i b) {
    return (__m128i) ((__v8hi) a == (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi32(__m128i a, __m128i b) {
    return (__m128i) ((__v4si) a == (__v4si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi8(__m128i a, __m128i b) {

    typedef signed char __v16qs __attribute__((__vector_size__(16)));
    return (__m128i) ((__v16qs) a > (__v16qs) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi16(__m128i a, __m128i b) {
    return (__m128i) ((__v8hi) a > (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi32(__m128i a, __m128i b) {
    return (__m128i) ((__v4si) a > (__v4si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi8(__m128i a, __m128i b) {
    return _mm_cmpgt_epi8(b, a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi16(__m128i a, __m128i b) {
    return _mm_cmpgt_epi16(b, a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi32(__m128i a, __m128i b) {
    return _mm_cmpgt_epi32(b, a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_sd(__m128d a, long long b) {
    a[0] = b;
    return a;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si64(__m128d a) {
    return __builtin_ia32_cvtsd2si64(a);
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si64(__m128d a) {
    return a[0];
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_ps(__m128i a) {
    return __builtin_ia32_cvtdq2ps((__v4si) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_epi32(__m128 a) {
    return (__m128i) __builtin_ia32_cvtps2dq(a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvttps_epi32(__m128 a) {
    return (__m128i) __builtin_ia32_cvttps2dq(a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_si128(int a) {
    return (__m128i) (__v4si) {
        a, 0, 0, 0
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_si128(long long a) {

    return (__m128i) {
        a, 0
    };
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si32(__m128i a) {
    __v4si b = (__v4si) a;
    return b[0];
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si64(__m128i a) {
    return a[0];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_load_si128(__m128i const *p) {
    return *p;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si128(__m128i const *p) {

    struct __loadu_si128 {
        __m128i v;
    } __attribute__((packed, may_alias));
    return ((struct __loadu_si128*) p)->v;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadl_epi64(__m128i const *p) {

    struct __mm_loadl_epi64_struct {
        long long u;
    } __attribute__((__packed__, __may_alias__));

    return (__m128i) {
        ((struct __mm_loadl_epi64_struct*) p)->u, 0
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi64x(long long q1, long long q0) {

    return (__m128i) {
        q0, q1
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi64(__m64 q1, __m64 q0) {
    return (__m128i) {
        (long long) q0, (long long) q1
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi32(int i3, int i2, int i1, int i0) {
    return (__m128i) (__v4si) {
        i0, i1, i2, i3
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi16(short w7, short w6, short w5, short w4, short w3, short w2, short w1, short w0) {
    return (__m128i) (__v8hi) {
        w0, w1, w2, w3, w4, w5, w6, w7
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi8(char b15, char b14, char b13, char b12, char b11, char b10, char b9, char b8, char b7, char b6, char b5, char b4, char b3, char b2, char b1, char b0) {

    return (__m128i) (__v16qi) {
        b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi64x(long long q) {

    return (__m128i) {
        q, q
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi64(__m64 q) {

    return (__m128i) {
        (long long) q, (long long) q
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi32(int i) {

    return (__m128i) (__v4si) {
        i, i, i, i
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi16(short w) {

    return (__m128i) (__v8hi) {
        w, w, w, w, w, w, w, w
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi8(char b) {

    return (__m128i) (__v16qi) {
        b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi64(__m64 q0, __m64 q1) {

    return (__m128i) {
        (long long) q0, (long long) q1
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi32(int i0, int i1, int i2, int i3) {

    return (__m128i) (__v4si) {
        i0, i1, i2, i3
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi16(short w0, short w1, short w2, short w3, short w4, short w5, short w6, short w7) {

    return (__m128i) (__v8hi) {
        w0, w1, w2, w3, w4, w5, w6, w7
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi8(char b0, char b1, char b2, char b3, char b4, char b5, char b6, char b7, char b8, char b9, char b10, char b11, char b12, char b13, char b14, char b15) {

    return (__m128i) (__v16qi) {
        b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setzero_si128(void) {

    return (__m128i) {
        0LL, 0LL
    };
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_si128(__m128i *p, __m128i b) {
    *p = b;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si128(__m128i *p, __m128i b) {
    __builtin_ia32_storedqu((char *) p, (__v16qi) b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskmoveu_si128(__m128i d, __m128i n, char *p) {
    __builtin_ia32_maskmovdqu((__v16qi) d, (__v16qi) n, p);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_epi64(__m128i *p, __m128i a) {

    struct __mm_storel_epi64_struct {
        long long u;
    } __attribute__((__packed__, __may_alias__));
    ((struct __mm_storel_epi64_struct*) p)->u = a[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_pd(double *p, __m128d a) {
    __builtin_ia32_movntpd(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si128(__m128i *p, __m128i a) {
    __builtin_ia32_movntdq(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si32(int *p, int a) {
    __builtin_ia32_movnti(p, a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clflush(void const *p) {
    __builtin_ia32_clflush(p);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_lfence(void) {
    __builtin_ia32_lfence();
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_mfence(void) {
    __builtin_ia32_mfence();
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_packsswb128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_packssdw128((__v4si) a, (__v4si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packus_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_packuswb128((__v8hi) a, (__v8hi) b);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_extract_epi16(__m128i a, int imm) {
    __v8hi b = (__v8hi) a;
    return (unsigned short) b[imm];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_insert_epi16(__m128i a, int b, int imm) {
    __v8hi c = (__v8hi) a;
    c[imm & 7] = b;
    return (__m128i) c;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_epi8(__m128i a) {
    return __builtin_ia32_pmovmskb128((__v16qi) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v16qi) a, (__v16qi) b, 8, 16 + 8, 9, 16 + 9, 10, 16 + 10, 11, 16 + 11, 12, 16 + 12, 13, 16 + 13, 14, 16 + 14, 15, 16 + 15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v8hi) a, (__v8hi) b, 4, 8 + 4, 5, 8 + 5, 6, 8 + 6, 7, 8 + 7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v4si) a, (__v4si) b, 2, 4 + 2, 3, 4 + 3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi64(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector(a, b, 1, 2 + 1);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v16qi) a, (__v16qi) b, 0, 16 + 0, 1, 16 + 1, 2, 16 + 2, 3, 16 + 3, 4, 16 + 4, 5, 16 + 5, 6, 16 + 6, 7, 16 + 7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v8hi) a, (__v8hi) b, 0, 8 + 0, 1, 8 + 1, 2, 8 + 2, 3, 8 + 3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector((__v4si) a, (__v4si) b, 0, 4 + 0, 1, 4 + 1);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi64(__m128i a, __m128i b) {
    return (__m128i) __builtin_shufflevector(a, b, 0, 2 + 0);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_movepi64_pi64(__m128i a) {
    return (__m64) a[0];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_movpi64_pi64(__m64 a) {

    return (__m128i) {
        (long long) a, 0
    };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_move_epi64(__m128i a) {

    return __builtin_shufflevector(a, (__m128i) {
        0}, 0, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pd(__m128d a, __m128d b) {
    return __builtin_shufflevector(a, b, 1, 2 + 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pd(__m128d a, __m128d b) {
    return __builtin_shufflevector(a, b, 0, 2 + 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_pd(__m128d a) {
    return __builtin_ia32_movmskpd(a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_castpd_ps(__m128d in) {
    return (__m128) in;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_castpd_si128(__m128d in) {
    return (__m128i) in;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_castps_pd(__m128 in) {
    return (__m128d) in;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_castps_si128(__m128 in) {
    return (__m128i) in;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_castsi128_ps(__m128i in) {
    return (__m128) in;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_castsi128_pd(__m128i in) {
    return (__m128d) in;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_pause(void) {
    __asm__ volatile ("pause");
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_lddqu_si128(__m128i const *p) {
    return (__m128i) __builtin_ia32_lddqu((char const *) p);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_addsub_ps(__m128 a, __m128 b) {
    return __builtin_ia32_addsubps(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_hadd_ps(__m128 a, __m128 b) {
    return __builtin_ia32_haddps(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_hsub_ps(__m128 a, __m128 b) {
    return __builtin_ia32_hsubps(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehdup_ps(__m128 a) {
    return __builtin_shufflevector(a, a, 1, 1, 3, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_moveldup_ps(__m128 a) {
    return __builtin_shufflevector(a, a, 0, 0, 2, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_addsub_pd(__m128d a, __m128d b) {
    return __builtin_ia32_addsubpd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_hadd_pd(__m128d a, __m128d b) {
    return __builtin_ia32_haddpd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_hsub_pd(__m128d a, __m128d b) {
    return __builtin_ia32_hsubpd(a, b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_movedup_pd(__m128d a) {
    return __builtin_shufflevector(a, a, 0, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_monitor(void const *p, unsigned extensions, unsigned hints) {
    __builtin_ia32_monitor((void *) p, extensions, hints);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_mwait(unsigned extensions, unsigned hints) {
    __builtin_ia32_mwait(extensions, hints);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_abs_pi8(__m64 a) {
    return (__m64) __builtin_ia32_pabsb((__v8qi) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi8(__m128i a) {
    return (__m128i) __builtin_ia32_pabsb128((__v16qi) a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_abs_pi16(__m64 a) {
    return (__m64) __builtin_ia32_pabsw((__v4hi) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi16(__m128i a) {
    return (__m128i) __builtin_ia32_pabsw128((__v8hi) a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_abs_pi32(__m64 a) {
    return (__m64) __builtin_ia32_pabsd((__v2si) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi32(__m128i a) {
    return (__m128i) __builtin_ia32_pabsd128((__v4si) a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phaddw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phaddd128((__v4si) a, (__v4si) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hadd_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phaddw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hadd_pi32(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phaddd((__v2si) a, (__v2si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadds_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phaddsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hadds_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phaddsw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phsubw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phsubd128((__v4si) a, (__v4si) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hsub_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phsubw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hsub_pi32(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phsubd((__v2si) a, (__v2si) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsubs_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_phsubsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_hsubs_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_phsubsw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maddubs_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmaddubsw128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_maddubs_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pmaddubsw((__v8qi) a, (__v8qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhrs_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pmulhrsw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mulhrs_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pmulhrsw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_shuffle_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_pshufb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_shuffle_pi8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_pshufb((__v8qi) a, (__v8qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi8(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psignb128((__v16qi) a, (__v16qi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi16(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psignw128((__v8hi) a, (__v8hi) b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi32(__m128i a, __m128i b) {
    return (__m128i) __builtin_ia32_psignd128((__v4si) a, (__v4si) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sign_pi8(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_psignb((__v8qi) a, (__v8qi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sign_pi16(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_psignw((__v4hi) a, (__v4hi) b);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sign_pi32(__m64 a, __m64 b) {
    return (__m64) __builtin_ia32_psignd((__v2si) a, (__v2si) b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_blendv_pd(__m128d __V1, __m128d __V2, __m128d __M) {
    return (__m128d) __builtin_ia32_blendvpd((__v2df) __V1, (__v2df) __V2,
            (__v2df) __M);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_blendv_ps(__m128 __V1, __m128 __V2, __m128 __M) {
    return (__m128) __builtin_ia32_blendvps((__v4sf) __V1, (__v4sf) __V2,
            (__v4sf) __M);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_blendv_epi8(__m128i __V1, __m128i __V2, __m128i __M) {
    return (__m128i) __builtin_ia32_pblendvb128((__v16qi) __V1, (__v16qi) __V2,
            (__v16qi) __M);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mullo_epi32(__m128i __V1, __m128i __V2) {
    return (__m128i) ((__v4si) __V1 * (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epi32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pmuldq128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_stream_load_si128(__m128i *__V) {
    return (__m128i) __builtin_ia32_movntdqa((__v2di *) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi8(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pminsb128((__v16qi) __V1, (__v16qi) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi8(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pmaxsb128((__v16qi) __V1, (__v16qi) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu16(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pminuw128((__v8hi) __V1, (__v8hi) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu16(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pmaxuw128((__v8hi) __V1, (__v8hi) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pminsd128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pmaxsd128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pminud128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_pmaxud128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_si128(__m128i __M, __m128i __V) {
    return __builtin_ia32_ptestz128((__v2di) __M, (__v2di) __V);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_si128(__m128i __M, __m128i __V) {
    return __builtin_ia32_ptestc128((__v2di) __M, (__v2di) __V);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_si128(__m128i __M, __m128i __V) {
    return __builtin_ia32_ptestnzc128((__v2di) __M, (__v2di) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi64(__m128i __V1, __m128i __V2) {
    return (__m128i) ((__v2di) __V1 == (__v2di) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi16(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxbw128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi32(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxbd128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxbq128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi16_epi32(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxwd128((__v8hi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi16_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxwq128((__v8hi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovsxdq128((__v4si) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi16(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxbw128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi32(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxbd128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxbq128((__v16qi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu16_epi32(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxwd128((__v8hi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu16_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxwq128((__v8hi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu32_epi64(__m128i __V) {
    return (__m128i) __builtin_ia32_pmovzxdq128((__v4si) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packus_epi32(__m128i __V1, __m128i __V2) {
    return (__m128i) __builtin_ia32_packusdw128((__v4si) __V1, (__v4si) __V2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_minpos_epu16(__m128i __V) {
    return (__m128i) __builtin_ia32_phminposuw128((__v8hi) __V);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi64(__m128i __V1, __m128i __V2) {
    return (__m128i) ((__v2di) __V1 > (__v2di) __V2);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_crc32_u8(unsigned int __C, unsigned char __D) {
    return __builtin_ia32_crc32qi(__C, __D);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_crc32_u16(unsigned int __C, unsigned short __D) {
    return __builtin_ia32_crc32hi(__C, __D);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_crc32_u32(unsigned int __C, unsigned int __D) {
    return __builtin_ia32_crc32si(__C, __D);
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
_mm_crc32_u64(unsigned long long __C, unsigned long long __D) {
    return __builtin_ia32_crc32di(__C, __D);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_popcnt_u32(unsigned int __A) {
    return __builtin_popcount(__A);
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_popcnt_u64(unsigned long long __A) {
    return __builtin_popcountll(__A);
}

#endif	/* SSE_H */
