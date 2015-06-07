/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"
#include "dtoa.h"
#include "polarssl/md5.h"
#include "polarssl/sha1.h"
#include "polarssl/sha256.h"
#include "utf8proc.h"

typedef enum {
    step_A, step_B, step_C
} base64_encodestep;

typedef struct {
    base64_encodestep step;
    char result;
    int stepcount;
} base64_encodestate;

typedef enum {
    step_a, step_b, step_c, step_d
} base64_decodestep;

typedef struct {
    base64_decodestep step;
    char plainchar;
} base64_decodestate;

const static int8_t fstr_serial_index_c2n[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
};

const static uint8_t fstr_serial_index_n2c[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
    'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z',
};

const static uint8_t fstr_base64_alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const static int8_t fstr_base64_value[] = {
    -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
    -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
    -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, 62,  -1, -1, -1, 63,
    52, 53, 54, 55,  56, 57, 58, 59,  60, 61, -1, -1,  -1, -1, -1, -1,
    -1,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
    15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25, -1,  -1, -1, -1, -1,
    -1, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
    41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51, -1,  -1, -1, -1, -1
};

const int CHARS_PER_LINE = 72;

#pragma librcd

void _fstr_unpack_error() {
    throw("end of buffer reached when unpacking", exception_io);
}

void _fstr_fixed_len_error(size_t expect_length, size_t actual_length) {
    sub_heap_e(throw(concs("invalid fixed string length, expected [", i2fs(expect_length), " b], got: [", i2fs(actual_length), " b]"), exception_fatal));
}

fstr_mem_t* fstr_alloc(size_t length) {
    fstr_mem_t* fstr_mem = lwt_alloc_new(sizeof(fstr_mem_t) + length);
    fstr_mem->len = length;
    return fstr_mem;
}

fstr_mem_t* fstr_alloc_buffer(size_t min_length) {
    size_t final_length;
    fstr_mem_t* fstr_mem = lwt_alloc_buffer(sizeof(fstr_mem_t) + min_length, &final_length);
    fstr_mem->len = final_length - sizeof(fstr_mem_t);
    return fstr_mem;
}

fstr_t fstr_fix_cstr(const char* c_string) {
    fstr_t fstr = {.len = strlen(c_string), .str = (uint8_t*) c_string};
    return fstr;
}

fstr_t fstr_fix_buffer(void* buffer, size_t length) {
    fstr_t fstr = {.len = length, .str = (uint8_t*) buffer};
    return fstr;
}

char* fstr_to_cstr(fstr_t fstr) {
    char* cstr = lwt_alloc_new(fstr.len + 1);
    if (fstr.len > 0)
        memcpy(cstr, fstr.str, fstr.len);
    cstr[fstr.len] = 0;
    return cstr;
}

bool fstr_unserial_uint(fstr_t fstr, uint8_t base, uint128_t* out_uint128) {
    if (base < 2 || base > 36)
        return false;
    bool mul_overflow = false;
    uint128_t mul = 1, result = 0;
    for (uint8_t* ptr = fstr.str + fstr.len - 1;; ptr--) {
        if (ptr < fstr.str)
            break;
        uint16_t v_index = (uint16_t)(*ptr) - '0';
        if (v_index >= LENGTHOF(fstr_serial_index_c2n))
            continue;
        int8_t v = fstr_serial_index_c2n[v_index];
        if (v < 0 || v >= base)
            continue;
        if (!mul_overflow) {
            if (v > 0) {
                uint128_t v_mul;
                if (!arth_safe_mul_uint128(v, mul, &v_mul))
                    return false;
                if (!arth_safe_add_uint128(result, v_mul, &result))
                    return false;
            }
            if (!arth_safe_mul_uint128(mul, base, &mul))
                mul_overflow = true;
        } else {
            // We can only proceed of the value number is zero as we have a
            // multiplicator overflow for this offset in this base.
            if (v > 0)
                return false;
        }
    }
    *out_uint128 = result;
    return true;
}

bool fstr_unserial_int(fstr_t fstr, uint8_t base, int128_t* out_int128) {
    bool negative = fstr_equal(fstr_slice(fstr, 0, 1), "-");
    fstr_t numeric_serial = negative? fstr_slice(fstr, 1, -1): fstr;
    uint128_t number;
    if (!fstr_unserial_uint(numeric_serial, base, &number))
        return false;
    if (negative? number > -INT128_MIN: number > INT128_MAX)
        return false;
    *out_int128 = (int128_t) (negative? -number: number);
    return true;
}

uint128_t fstr_to_uint(fstr_t fstr, uint8_t base) {
    uint128_t number;
    return fstr_unserial_uint(fstr, base, &number)? number: 0;
}

int128_t fstr_to_int(fstr_t fstr, uint8_t base) {
    int128_t number;
    return fstr_unserial_int(fstr, base, &number)? number: 0;
}

double fstr_to_double(fstr_t fstr) { sub_heap {
    char* cstr = fstr_to_cstr(fstr);
    return strtod(cstr, 0);
}}

fstr_t fstr_serial_uint(fstr_t fstr_dest, uint128_t i, uint8_t base) {
    if (fstr_dest.len == 0)
        return fstr_dest;
    if (base < 2 || base > 36)
        return (fstr_t) {.str = fstr_dest.str, .len = 0};
    uint8_t* dst = fstr_dest.str + (fstr_dest.len - 1);
    for (uint128_t cur_i = i;;) {
        uint8_t numeral = cur_i % base;
        *dst = fstr_serial_index_n2c[numeral];
        cur_i /= base;
        if (cur_i == 0)
            break;
        dst--;
        if (dst < fstr_dest.str)
            break;
    }
    fstr_t r_str = {.str = dst, .len = fstr_dest.len - (dst - fstr_dest.str)};
    for (dst--; dst >= fstr_dest.str; dst--)
        *dst = fstr_serial_index_n2c[0];
    return r_str;
}

fstr_t fstr_serial_int(fstr_t fstr_dest, int128_t i, uint8_t base) {
    bool neg;
    if (i < 0) {
        i = -i;
        neg = true;
    } else {
        neg = false;
    }
    fstr_t tail_used = fstr_serial_uint(fstr_dest, i, base);
    if (neg && tail_used.str > fstr_dest.str) {
        tail_used.str[-1] = '-';
        tail_used.str--;
        tail_used.len++;
    }
    return tail_used;
}

static __inline int fstr_measure_serialized_integer(uint128_t i, uint8_t base) {
    int len = 1;
    for (uint128_t cur_i = i;;) {
        cur_i /= base;
        if (cur_i == 0)
            break;
        len++;
    }
    return len;
}

fstr_mem_t* fstr_from_int_pad(int128_t i, uint8_t base, size_t padding) {
    uint128_t abs_i = (i < 0? -i: i);
    int len = fstr_measure_serialized_integer(abs_i, base);
    fstr_mem_t* fstr_mem = fstr_alloc(MAX(padding, len) + (i < 0? 1: 0));
    fstr_serial_int(fstr_str(fstr_mem), i, base);
    return fstr_mem;
}

fstr_mem_t* fstr_from_int(int128_t i, uint8_t base) {
    return fstr_from_int_pad(i, base, 0);
}

fstr_mem_t* fstr_from_uint_pad(uint128_t i, uint8_t base, size_t padding) {
    int len = fstr_measure_serialized_integer(i, base);
    fstr_mem_t* fstr_mem = fstr_alloc(MAX(padding, len));
    fstr_serial_uint(fstr_str(fstr_mem), i, base);
    return fstr_mem;
}

fstr_mem_t* fstr_from_uint(uint128_t i, uint8_t base) {
    return fstr_from_uint_pad(i, base, 0);
}

fstr_mem_t* fstr_from_double(double d) {
    fstr_t e_buf;
    FSTR_STACK_DECL(e_buf, 16);
    int32_t decpt, sign;
    char* rve;
    char* dtoa_r = dtoa(d, 0, 0, &decpt, &sign, &rve);
    fstr_t tokens[6];
    uint8_t w_offs = 0;
    int32_t len = rve - dtoa_r;
    assert(len != 0);
    if (decpt == 9999) {
        // Special value (inf/nan).
        assert(!isfinite(d));
        tokens[w_offs++] = (fstr_t) {.len = len, .str = (uint8_t*) dtoa_r};
    } else {
        if (sign != 0) {
            tokens[w_offs++] = "-";
        }
        if (decpt <= -5) {
            // Very small value (1 ppm or smaller). Use normalized scientific notation.
            tokens[w_offs++] = (fstr_t) {.len = 1, .str = (uint8_t*) dtoa_r};
            if (len > 1) {
                tokens[w_offs++] = ".";
                tokens[w_offs++] = (fstr_t) {.len = len - 1, .str = (uint8_t*) (dtoa_r + 1)};
            }
            tokens[w_offs++] = "e-";
            tokens[w_offs++] = fstr_serial_uint(e_buf, -decpt + 1, 10);
        } else if (decpt <= 0) {
            fstr_t zero_str = "0.00000";
            tokens[w_offs++] = (fstr_t) {.len = 2 - decpt, .str = zero_str.str};
            tokens[w_offs++] = (fstr_t) {.len = len, .str = (uint8_t*) dtoa_r};
        } else {
            // Copy the digits before the decimal point.
            fstr_t bdp_part = {.len = MIN(len, decpt), .str = (uint8_t*) dtoa_r};
            if (decpt < len) {
                tokens[w_offs++] = bdp_part;
                tokens[w_offs++] = ".";
                // Copy digits after decimal point.
                tokens[w_offs++] = (fstr_t) {.len = len - decpt, .str = (uint8_t*) (dtoa_r + decpt)};
            } else if (decpt > len) {
                int32_t e = decpt - len;
                if (d >= 1e12 || d <= -1e12) {
                    // Very large negative or positive number. Use normalized scientific notation.
                    if (bdp_part.len > 1) {
                        e += bdp_part.len - 1;
                        tokens[w_offs++] = (fstr_t) {.len = 1, .str = bdp_part.str};
                        tokens[w_offs++] = ".";
                        tokens[w_offs++] = (fstr_t) {.len = bdp_part.len - 1, .str = bdp_part.str + 1};
                    } else {
                        tokens[w_offs++] = bdp_part;
                    }
                    tokens[w_offs++] = "e";
                    tokens[w_offs++] = fstr_serial_uint(e_buf, e, 10);
                } else {
                    // Using standard form for numbers with absolute value smaller than a trillion.
                    tokens[w_offs++] = bdp_part;
                    fstr_t zero_str = "000000000000";
                    tokens[w_offs++] = (fstr_t) {.len = e, .str = zero_str.str};
                }
            } else {
                tokens[w_offs++] = bdp_part;
            }
        }
    }
    fstr_mem_t* fstr = fstr_concat(tokens, w_offs, "");
    freedtoa(dtoa_r);
    return fstr;
}

fstr_mem_t* fstr_from_cstr(const char* src) {
    fstr_mem_t* fstr_mem = fstr_alloc(strlen(src));
    memcpy(fstr_mem->str, src, fstr_mem->len);
    return fstr_mem;
}

fstr_mem_t* fstr_cpy(fstr_t src) {
    fstr_mem_t* fstr_dst_mem = fstr_alloc(src.len);
    memcpy(fstr_dst_mem->str, src.str, src.len);
    return fstr_dst_mem;
}

fstr_t fstr_cpy_over(fstr_t dst, fstr_t src, fstr_t* out_dst_tail, fstr_t* out_src_tail) {
    size_t len = MIN(dst.len, src.len);
    if (len > 0)
        memcpy(dst.str, src.str, len);
    if (out_dst_tail != 0)
        *out_dst_tail = fstr_slice(dst, len, -1);
    if (out_src_tail != 0)
        *out_src_tail = fstr_slice(src, len, -1);
    return fstr_slice(dst, 0, len);
}

fstr_t fstr_tail(fstr_t full, fstr_t mark) {
    if (mark.len == 0)
        return "";
    if (mark.str < full.str)
        return full;
    return fstr_slice(full, mark.str - full.str, -1);
}

fstr_t fstr_detail(fstr_t full, fstr_t tail) {
    if (tail.len == 0)
        return full;
    if (tail.str < full.str || (tail.str + tail.len) != (full.str + full.len))
        return (fstr_t) {.str = full.str, .len = 0};
    return (fstr_t) {.str = full.str, .len = full.len - tail.len};
}

void fstr_fill(fstr_t dst, uint8_t chr) {
    memset(dst.str, chr, dst.len);
}

fstr_mem_t* fstr_pseudo_random(size_t length) { sub_heap {
    prng_state_t* prng_s = prng_get_thread_state();
    prng_seed_time(prng_s);
    fstr_mem_t* out_str = fstr_alloc(length);
    for (size_t i = 0; i < length; i += sizeof(uint64_t)) {
        uint64_t rnd_64 = prng_rand_nds(prng_s);
        fstr_cpy_over(fstr_slice(fstr_str(out_str), i, i + sizeof(uint64_t)), FSTR_PACK(rnd_64), 0, 0);
    }
    return escape(out_str);
}}

fstr_mem_t* fstr_hexrandom(size_t length) { sub_heap {
    prng_state_t* prng_s = prng_get_thread_state();
    prng_seed_time(prng_s);
    fstr_mem_t* out_hex_str = fstr_alloc(length);
    const size_t hex_per_uint64 = (sizeof(uint64_t) * 2);
    for (size_t i = 0; i < length; i += hex_per_uint64) {
        uint64_t rnd_64 = prng_rand_nds(prng_s);
        fstr_serial_uint(fstr_slice(fstr_str(out_hex_str), i, i + hex_per_uint64), rnd_64, 16);
    }
    return escape(out_hex_str);
}}

fstr_mem_t* fstr_hexencode(fstr_t bin_src) {
    size_t len = bin_src.len * 2;
    fstr_mem_t* hex_dest = fstr_alloc(len);
    for (int i = 0; i < bin_src.len; i++)
        fstr_serial_uint(fstr_slice(fstr_str(hex_dest), i * 2, (i + 1) * 2), bin_src.str[i], 16);
    return hex_dest;
}

fstr_mem_t* fstr_hexdecode(fstr_t hex_src) {
    size_t bin_destl = 0;
    fstr_mem_t* bin_dest_mem = fstr_alloc(hex_src.len / 2 + 1);
    fstr_t bin_dest = fstr_str(bin_dest_mem);
    uint8_t hex_bytel = 0;
    uint8_t hex_bytes[2];
    for (int i = 0; i < hex_src.len; i++) {
        uint8_t hex_byte = hex_src.str[i];
        if ((hex_byte >= '0' && hex_byte <= '9') || (hex_byte >= 'a' && hex_byte <= 'f') || (hex_byte >= 'A' && hex_byte <= 'F')) {
            hex_bytes[hex_bytel] = hex_byte;
            hex_bytel++;
            if (hex_bytel == 2) {
                hex_bytel = 0;
                fstr_t src_hexpair = {.len = 2, .str = hex_bytes};
                bin_dest.str[bin_destl] = (uint8_t) fstr_to_uint(src_hexpair, 16);
                bin_destl++;
            }
        }
    }
    bin_dest_mem->len = bin_destl;
    return bin_dest_mem;
}

fstr_mem_t* fstr_hexdump(fstr_t src) { sub_heap {
    uint8_t nonprintable_char = '.';
    list(fstr_t)* fstr_list = new_list(fstr_t);
    for (size_t i = 0; i < src.len; i += 16) {
        list_push_end(fstr_list, fstr_t, fstr_str(fstr_from_uint_pad(i, 16, 6)));
        list_push_end(fstr_list, fstr_t, ": ");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < src.len) {
                list_push_end(fstr_list, fstr_t, fstr_str(fstr_from_uint_pad(src.str[i + j], 16, 2)));
                list_push_end(fstr_list, fstr_t, " ");
            } else {
                list_push_end(fstr_list, fstr_t, "   ");
            }
        }
        list_push_end(fstr_list, fstr_t, " ");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < src.len) {
                uint8_t* c = isprint(src.str[i + j])? &src.str[i + j]: &nonprintable_char;
                list_push_end(fstr_list, fstr_t, fstr_fix_buffer(c, 1));
            }
        }
        list_push_end(fstr_list, fstr_t, "\n");
    }
    list_push_end(fstr_list, fstr_t, "\n");
    return escape(fstr_implode(fstr_list, ""));
}}

fstr_mem_t* fstr_hex_from_ptr(void* ptr) {
    size_t nhex = sizeof(ptr) * 2;
    fstr_mem_t* str = fstr_alloc(nhex + 2);
    fstr_t tail = fss(str);
    fstr_putc(&tail, '0');
    fstr_putc(&tail, 'x');
    fstr_serial_uint(tail, (size_t) ptr, 16);
    return str;
}

bool fstr_equal(const fstr_t str1, const fstr_t str2) {
    return fstr_cmp(str1, str2) == 0;
}

bool fstr_equal_case(fstr_t str1, fstr_t str2) {
    return fstr_cmp_case(str1, str2) == 0;
}

bool fstr_prefixes(fstr_t str, fstr_t prefix) {
    return fstr_equal(fstr_slice(str, 0, prefix.len), prefix);
}

bool fstr_prefixes_case(fstr_t str, fstr_t prefix) {
    return fstr_equal_case(fstr_slice(str, 0, prefix.len), prefix);
}

bool fstr_suffixes(fstr_t str, fstr_t suffix) {
    return fstr_equal(fstr_slice(str, -suffix.len - 1, -1), suffix);
}

bool fstr_suffixes_case(fstr_t str, fstr_t suffix) {
    return fstr_equal_case(fstr_slice(str, -suffix.len - 1, -1), suffix);
}

int64_t fstr_cmp(const fstr_t str1, const fstr_t str2) {
    int len_dif = str1.len - str2.len;
    if (len_dif != 0)
        return len_dif;
    if (str1.str == str2.str)
        return 0;
    return memcmp(str1.str, str2.str, str1.len);
}

int64_t fstr_cmp_case(fstr_t str1, fstr_t str2) {
    int len_dif = str1.len - str2.len;
    if (len_dif != 0)
        return len_dif;
    if (str1.str == str2.str)
        return 0;
    for (size_t i = 0; i < str1.len; i++) {
        uint8_t a = fstr_ctolower(str1.str[i]);
        uint8_t b = fstr_ctolower(str2.str[i]);
        if (a != b)
            return (int64_t) a - (int64_t) b;
    }
    return 0;
}

int64_t fstr_cmp_lexical(const fstr_t str1, const fstr_t str2) {
    int cmp = memcmp(str1.str, str2.str, MIN(str1.len, str2.len));
    return cmp != 0? cmp: str1.len - str2.len;
}

static fstr_t fstr_nat_consume_str(fstr_t* io) {
    fstr_t out = {.len = 0};
    fstr_t str = *io;
    #pragma re2c(str): ^([^0-9]*){out} ([0-9]|$) {@match}
    match: {
        *io = fstr_slice(*io, out.len, -1);
        return out;
    }
}

static fstr_t fstr_nat_consume_num(fstr_t* io) {
    fstr_t out = {.len = 0};
    fstr_t str = *io;
    #pragma re2c(str): ^([0-9]*){out} ([^0-9]|$) {@match}
    match: {
        *io = fstr_slice(*io, out.len, -1);
        return out;
    }
}

int64_t fstr_cmp_natural(fstr_t str1, fstr_t str2) {
    // Humans see strings as a series of tokens which consist of characters
    // grouped by their class. In this natural sort implementation we only
    // consider numbers and non-numbers.
    while (str1.len > 0 || str2.len > 0) {
        // Compare next string token.
        fstr_t str1s = fstr_nat_consume_str(&str1);
        fstr_t str2s = fstr_nat_consume_str(&str2);
        int32_t cmp1_v = fstr_cmp(str1s, str2s);
        if (cmp1_v != 0)
            return cmp1_v;
        // Compare next numeric token.
        fstr_t str1n = fstr_nat_consume_num(&str1);
        fstr_t str2n = fstr_nat_consume_num(&str2);
        int32_t cmp2_v = fstr_cmp(str1n, str2n);
        if (cmp2_v != 0)
            return cmp2_v;
    }
    return 0;
}

static int32_t fstr_sort_cmp(const void* arg1, const void* arg2) {
    fstr_t *str1_ptr = (void*) arg1, *str2_ptr = (void*) arg2;
    int64_t cmp_v = fstr_cmp(*str1_ptr, *str2_ptr);
    return cmp_v == 0? 0: (cmp_v > 0? 1: -1);
}

static int32_t fstr_sort_cmp_lexical(const void* arg1, const void* arg2) {
    fstr_t *str1_ptr = (void*) arg1, *str2_ptr = (void*) arg2;
    int64_t cmp_v = fstr_cmp_lexical(*str1_ptr, *str2_ptr);
    return cmp_v == 0? 0: (cmp_v > 0? 1: -1);
}

static int32_t fstr_sort_cmp_nat(const void* arg1, const void* arg2) {
    fstr_t *str1_ptr = (void*) arg1, *str2_ptr = (void*) arg2;
    int64_t cmp_v = fstr_cmp_natural(*str1_ptr, *str2_ptr);
    return cmp_v == 0? 0: (cmp_v > 0? 1: -1);
}

void fstr_sort(fstr_t fstr_list[], size_t n_fstr_list, bool lexical) {
    sort(fstr_list, n_fstr_list, sizeof(*fstr_list), lexical? fstr_sort_cmp_lexical: fstr_sort_cmp, 0);
}

void fstr_sort_nat(fstr_t fstr_list[], size_t n_fstr_list) {
    sort(fstr_list, n_fstr_list, sizeof(*fstr_list), fstr_sort_cmp_nat, 0);
}

fstr_mem_t* fstr_order_next(fstr_t src) { sub_heap {
    fstr_mem_t* dst = fstr_alloc(src.len + 1);
    for (ssize_t i = (ssize_t)(src.len) - 1;; i--) {
        if (i == -1) {
            // Overflow, utilize last byte.
            dst->str[src.len] = '\x00';
            break;
        }
        uint8_t chr = src.str[i] + 1;
        dst->str[i] = chr;
        if (chr != 0) {
            // Not overflow, write rest, adjust dst length and return.
            if (i > 0)
                memcpy(dst->str, src.str, i);
            dst->len--;
            break;
        }
    }
    return escape(dst);
}}

fstr_mem_t* fstr_order_prev(fstr_t src) { sub_heap {
    if (src.len == 0)
        throw("fstr_order_prev() got zero length string", exception_arg);
    fstr_mem_t* dst = fstr_alloc(src.len);
    for (ssize_t i = (ssize_t)(src.len) - 1;; i--) {
        if (i == -1) {
            // Overflow, reduce length.
            dst->len--;
            break;
        }
        uint8_t chr = src.str[i] - 1;
        dst->str[i] = chr;
        if (chr != 0xff) {
            // Not overflow, write rest and return.
            if (i > 0)
                memcpy(dst->str, src.str, i);
            break;
        }
    }
    return escape(dst);
}}

fstr_mem_t* fstr_lower(fstr_t src) {
    fstr_mem_t* dst = fstr_alloc(src.len);
    for (size_t i = 0; i < src.len; i++)
        dst->str[i] = fstr_ctolower(src.str[i]);
    return dst;
}

fstr_mem_t* fstr_upper(fstr_t src) {
    fstr_mem_t* dst = fstr_alloc(src.len);
    for (size_t i = 0; i < src.len; i++)
        dst->str[i] = fstr_ctoupper(src.str[i]);
    return dst;
}

int64_t fstr_scan(fstr_t str, fstr_t sub_str) {
    if (sub_str.len > str.len)
        return -1;
    size_t end_i = (str.len - sub_str.len);
    for (size_t i = 0; i <= end_i; i++) {
        for (size_t j = 0;; j++) {
            if (j == sub_str.len)
                return i;
            assert((j) < sub_str.len && (i + j) < str.len);
            if (sub_str.str[j] != str.str[i + j])
                break;
        }
    }
    return -1;
}

int64_t fstr_rscan(fstr_t str, fstr_t sub_str) {
    if (sub_str.len > str.len)
        return -1;
    size_t end_i = (str.len - sub_str.len);
    for (size_t i = end_i + 1; i > 0; i--) {
        for (size_t j = sub_str.len;; j--) {
            if (j == 0)
                return (i - 1);
            assert((j) >= 1 && (j - 1) < sub_str.len && (i + j) >= 2 && (i + j - 2) < str.len);
            if (sub_str.str[j - 1] != str.str[i + j - 2])
                break;
        }
    }
    return -1;
}

fstr_mem_t* fstr_concat(fstr_t fstr_list[], size_t n_fstr_list, fstr_t glue) {
    if (fstr_list == 0 || n_fstr_list == 0)
        return fstr_alloc(0);
    size_t len = glue.len * (n_fstr_list - 1);
    for (size_t i = 0; i < n_fstr_list; i++)
        len += fstr_list[i].len;
    fstr_mem_t* fstr = fstr_alloc(len);
    uint8_t* write_ptr = fstr->str;
    bool first_piece = true;
    for (size_t i = 0; i < n_fstr_list; i++) {
        if (glue.len > 0) {
            if (!first_piece) {
                memcpy(write_ptr, glue.str, glue.len);
                write_ptr += glue.len;
            } else {
                first_piece = false;
            }
        }
        memcpy(write_ptr, fstr_list[i].str, fstr_list[i].len);
        write_ptr += fstr_list[i].len;
    }
    return fstr;
}

fstr_mem_t* fstr_concatv(vec(fstr_t)* vec, fstr_t glue) {
    fstr_t* a_vec = vec_array(vec, fstr_t);
    size_t n_vec = vec_count(vec, fstr_t);
    return fstr_concat(a_vec, n_vec, glue);
}

fstr_mem_t* fstr_implode(list(fstr_t)* fstr_list, fstr_t glue) {
    if (fstr_list == 0 || list_count(fstr_list, fstr_t) == 0)
        return fstr_alloc(0);
    size_t len = glue.len * (list_count(fstr_list, fstr_t) - 1);
    list_foreach(fstr_list, fstr_t, piece)
        len += piece.len;
    fstr_mem_t* fstr = fstr_alloc(len);
    uint8_t* write_ptr = fstr->str;
    bool first_piece = true;
    list_foreach(fstr_list, fstr_t, piece) {
        if (glue.len > 0) {
            if (!first_piece) {
                memcpy(write_ptr, glue.str, glue.len);
                write_ptr += glue.len;
            } else {
                first_piece = false;
            }
        }
        memcpy(write_ptr, piece.str, piece.len);
        write_ptr += piece.len;
    }
    return fstr;
}

list(fstr_t)* fstr_explode(fstr_t src, fstr_t delimiter) {
    if (delimiter.len == 0 || src.len < delimiter.len)
        return new_list(fstr_t, src);
    // We do inefficient O(N*M) matching here. Ocre2c should be used for fast
    // matching and we don't expect explode to get long delimiters anyway.
    // Doing more efficient matching in run-time (e.g. boyer moore) requires
    // a lot of setup which should make this type of matching much faster for
    // small values of N and M.
    size_t end_i = (src.len - delimiter.len);
    list(fstr_t)* list = new_list(fstr_t);
    if (src.len > 0) {
        fstr_t next_chunk = {.str = src.str};
        for (size_t i = 0; i <= end_i; i++) {
            for (size_t j = 0;; j++) {
                if (j == delimiter.len) {
                    next_chunk.len = &src.str[i] - next_chunk.str;
                    list_push_end(list, fstr_t, next_chunk);
                    next_chunk.str = &src.str[i + delimiter.len];
                    i += delimiter.len - 1;
                    break;
                }
                if (delimiter.str[j] != src.str[i + j])
                    break;
            }
        }
        next_chunk.len = &src.str[src.len] - next_chunk.str;
        list_push_end(list, fstr_t, next_chunk);
    }
    return list;
}

fstr_mem_t* fstr_replace(fstr_t source, fstr_t find, fstr_t replace) { sub_heap {
    return escape(fstr_implode(fstr_explode(source, find), replace));
}}

static inline bool fstr_divide_dir(fstr_t src, fstr_t separator, fstr_t* out_before, fstr_t* out_after, bool reverse) {
    int64_t s_offs = (reverse? fstr_rscan: fstr_scan)(src, separator);
    if (s_offs == -1)
        return false;
    if (out_before != 0)
        *out_before = fstr_slice(src, 0, s_offs);
    if (out_after != 0)
        *out_after = fstr_slice(src, s_offs + separator.len, -1);
    return true;
}

bool fstr_divide(fstr_t src, fstr_t separator, fstr_t* out_before, fstr_t* out_after) {
    return fstr_divide_dir(src, separator, out_before, out_after, false);
}

bool fstr_rdivide(fstr_t src, fstr_t separator, fstr_t* out_before, fstr_t* out_after) {
    return fstr_divide_dir(src, separator, out_before, out_after, true);
}

static inline bool fstr_iterate_dir(fstr_t* io_str, fstr_t separator, fstr_t* out_next, bool reverse) {
    if (separator.len == 0)
        return false;
    if (io_str->len == 0)
        return false;
    fstr_t before, after;
    if (fstr_divide_dir(*io_str, separator, &before, &after, reverse)) {
        *io_str = (reverse? before: after);
        if (out_next != 0)
            *out_next = (reverse? after: before);
    } else {
        if (out_next != 0)
            *out_next = *io_str;
        if (reverse) {
            io_str->len = 0;
        } else {
            io_str->str += io_str->len;
            io_str->len = 0;
        }
    }
    return true;
}

static inline bool fstr_iterate_trim_dir(fstr_t* io_str, fstr_t separator, fstr_t* out_next, bool reverse) {
    fstr_t tr_io_str = fstr_trim(*io_str);
    bool cont = fstr_iterate_dir(&tr_io_str, separator, out_next, reverse);
    if (cont && out_next != 0) {
        *out_next = fstr_trim(*out_next);
    }
    *io_str = tr_io_str;
    return cont;
}

bool fstr_iterate(fstr_t* io_str, fstr_t separator, fstr_t* out_next) {
    return fstr_iterate_dir(io_str, separator, out_next, false);
}

bool fstr_riterate(fstr_t* io_str, fstr_t separator, fstr_t* out_next) {
    return fstr_iterate_dir(io_str, separator, out_next, true);
}

bool fstr_iterate_trim(fstr_t* io_str, fstr_t separator, fstr_t* out_next) {
    return fstr_iterate_trim_dir(io_str, separator, out_next, false);
}

bool fstr_riterate_trim(fstr_t* io_str, fstr_t separator, fstr_t* out_next) {
    return fstr_iterate_trim_dir(io_str, separator, out_next, true);
}

fstr_t fstr_trim(fstr_t fstr) {
    for (ssize_t i = 0;; i++) {
        if (i == fstr.len)
            return fstr_slice(fstr, 0, 0);
        if (fstr.str[i] > 0x20) {
            fstr = fstr_slice(fstr, i, -1);
            break;
        }
    }
    for (ssize_t i = fstr.len - 1;; i--) {
        if (fstr.str[i] > 0x20)
            return fstr_slice(fstr, 0, i + 1);
    }
}

fstr_mem_t* fstr_base64_encode(fstr_t binary_data) {
    size_t base64_max_len = 4 * binary_data.len / 3 + 4;
    fstr_mem_t* base64_buf = fstr_alloc(base64_max_len);
    uint8_t* wptr = base64_buf->str;
    for (size_t i = 0; i < binary_data.len; i++) {
        uint8_t in[3];
        for (size_t j = 0;; j++) {
            in[j] = binary_data.str[i];
            if (j == 2)
                break;
            i++;
            if (i == binary_data.len) {
                wptr[0] = fstr_base64_alpha[in[0] >> 2];
                uint8_t in_frag = (in[0] << 4) & 0x30;
                if (j == 0) {
                    wptr[2] = '=';
                } else {
                    in_frag |= in[1] >> 4;
                    wptr[2] = fstr_base64_alpha[(in[1] << 2) & 0x3c];
                }
                wptr[1] = fstr_base64_alpha[in_frag];
                wptr[3] = '=';
                wptr += 4;
                goto done;
            }
        }
        wptr[0] = fstr_base64_alpha[in[0] >> 2];
        wptr[1] = fstr_base64_alpha[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        wptr[2] = fstr_base64_alpha[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        wptr[3] = fstr_base64_alpha[in[2] & 0x3f];
        wptr += 4;
    } done: {
        assert(wptr <= (base64_buf->str + base64_buf->len));
        base64_buf->len = (wptr - base64_buf->str);
        return base64_buf;
    }
}

fstr_mem_t* fstr_base64_decode(fstr_t base64_text) {
    size_t binary_max_len = 3 * base64_text.len / 4 + 3;
    fstr_mem_t* binary_buf = fstr_alloc(binary_max_len);
    uint8_t* wptr = binary_buf->str;
    for (size_t i = 0; i < base64_text.len; i++) {
        uint8_t in[4];
        for (size_t j = 0;;) {
            uint8_t raw = base64_text.str[i];
            if (raw >= LENGTHOF(fstr_base64_value))
                goto next_raw_char;
            int8_t raw_64v = fstr_base64_value[raw];
            if (raw_64v < 0)
                goto next_raw_char;
            in[j] = (uint8_t) raw_64v;
            j++;
            if (j == 4)
                break;
            next_raw_char: {
                i++;
                if (i == base64_text.len) {
                    if (j < 2) {
                        // If we have only read one valid byte the base64 text
                        // is broken and we only return the partial result.
                        goto done;
                    }
                    *wptr = (in[0] << 2) | (in[1] >> 4);
                    wptr++;
                    if (j < 3)
                        goto done;
                    *wptr = ((in[1] << 4) & 0xf0) | (in[2] >> 2);
                    wptr++;
                    goto done;
                }
            }
        }
        wptr[0] = (in[0] << 2) | (in[1] >> 4);
        wptr[1] = ((in[1] << 4) & 0xf0) | (in[2] >> 2);
        wptr[2] = ((in[2] << 6) & 0xc0) | in[3];
        wptr += 3;
    } done: {
        assert(wptr <= (binary_buf->str + binary_buf->len));
        binary_buf->len = (wptr - binary_buf->str);
        return binary_buf;
    }
}

fstr_mem_t* fstr_base32_encode(fstr_t s) { sub_heap {
    if (s.len == 0 || s.len > (1 << 28))
        return escape(fstr_alloc(0));
    fstr_t base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    fstr_mem_t* output = fstr_alloc((s.len + 4) / 5 * 8);
    fstr_t output_tail = fss(output);
    uint32_t buffer = s.str[0];
    uint32_t next = 1;
    uint32_t bits_left = 8;
    while (output_tail.len > 0 && (bits_left > 0 || next < s.len)) {
        if (bits_left < 5) {
            if (next < s.len) {
                buffer <<= 8;
                buffer |= s.str[next++] & 0xFF;
                bits_left += 8;
            } else {
                uint8_t pad = 5 - bits_left;
                buffer <<= pad;
                bits_left += pad;
            }
        }
        fstr_putc(&output_tail, base32_chars.str[(0x1F & (buffer >> (bits_left - 5)))]);
        bits_left -= 5;
    }
    while (output_tail.len > 0)
        fstr_putc(&output_tail, '=');
    return escape(output);
}}

fstr_mem_t* fstr_base32_decode(fstr_t s) { sub_heap {
    uint32_t buffer = 0, bits_left = 0;
    fstr_mem_t* output = fstr_alloc(s.len);
    fstr_t output_tail = fss(output);
    for (size_t i = 0; i < s.len; i++) {
        uint8_t ch = s.str[i];
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-')
            continue;
        buffer <<= 5;
        if (ch == '0') {
            ch = 'O';
        } else if (ch == '1') {
            ch = 'L';
        } else if (ch == '8') {
            ch = 'B';
        }
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            ch = (ch & 0x1F) - 1;
        } else if (ch >= '2' && ch <= '7') {
            ch -= '2' - 26;
        } else {
            break;
        }
        buffer |= ch;
        bits_left += 5;
        if (bits_left >= 8) {
            fstr_putc(&output_tail, buffer >> (bits_left - 8));
            bits_left -= 8;
        }
    }
    output->len -= output_tail.len;
    return escape(output);
}}

fstr_mem_t* fstr_ace_encode(fstr_t decoded) { sub_heap {
    // Worst case is every character being hex encoded and inflated by 4 characters (c -> \xXX).
    fstr_mem_t* ret = fstr_alloc(decoded.len * 4);
    fstr_t rtail = fss(ret);
    for (size_t i = 0; i < decoded.len; i++) {
        uint8_t c = decoded.str[i];
        switch (c) {{
        } case '\a': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'a');
            break;
        } case '\b': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'b');
            break;
        } case '\t': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 't');
            break;
        } case '\n': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'n');
            break;
        } case '\v': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'v');
            break;
        } case '\f': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'f');
            break;
        } case '\r': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, 'r');
            break;
        } case '"': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, '"');
            break;
        } case '\'': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, '\'');
            break;
        } case '\\': {
            fstr_putc(&rtail, '\\');
            fstr_putc(&rtail, '\\');
            break;
        } default: {
            if (c < 0x20 || c >= 0x7f) {
                // Escape control characters and undefined characters.
                fstr_putc(&rtail, '\\');
                fstr_putc(&rtail, 'x');
                fstr_t buf = fstr_slice(rtail, 0, 2);
                fstr_serial_int(buf, c, 16);
                rtail = fstr_slice(rtail, 2, -1);
            } else {
                fstr_putc(&rtail, c);
            }
            break;
        }}
    }
    ret->len -= rtail.len;
    return escape(ret);
}}

fstr_mem_t* fstr_ace_decode(fstr_t encoded) { sub_heap {
    fstr_mem_t* ret = fstr_alloc(encoded.len);
    fstr_t rtail = fss(ret);
    bool escaping = false;
    for (size_t i = 0; i < encoded.len; i++) {
        uint8_t c = encoded.str[i];
        if (escaping) {
            switch (c) {{
            } case 'a': {
                fstr_putc(&rtail, '\a');
                break;
            } case 'b': {
                fstr_putc(&rtail, '\b');
                break;
            } case 't': {
                fstr_putc(&rtail, '\t');
                break;
            } case 'n': {
                fstr_putc(&rtail, '\n');
                break;
            } case 'v': {
                fstr_putc(&rtail, '\v');
                break;
            } case 'f': {
                fstr_putc(&rtail, '\f');
                break;
            } case 'r': {
                fstr_putc(&rtail, '\r');
                break;
            } case '"': {
                fstr_putc(&rtail, '\"');
                break;
            } case '\'': {
                fstr_putc(&rtail, '\'');
                break;
            } case '\\': {
                fstr_putc(&rtail, '\\');
                break;
            } case 'x': {
                fstr_t hex = fstr_slice(encoded, i + 1, i + 3);
                {
                    fstr_t hv = hex;
                    #pragma re2c(hv): ^[a-zA-Z0-9]{2,2}$ {@hex_valid}
                    throw(concs("syntax error at offset ", i, ": hex sequence is not valid"), exception_io);
                    hex_valid:;
                }
                fstr_putc(&rtail, fstr_to_int(hex, 16));
                i += 2;
                break;
            } default: {
                throw(concs("syntax error at offset ", i, ": unknown escape character [", c, "]"), exception_io);
                break;
            }}
            escaping = false;
        } else {
            switch (c) {{
            } case '\\': {
                escaping = true;
                break;
            } default: {
                fstr_putc(&rtail, c);
                break;
            }}
        }
    }
    ret->len -= rtail.len;
    return escape(ret);
}}

flstr(16)* fstr_md5(fstr_t data) {
    md5_context ctx;
    md5_starts(&ctx);
    md5_update(&ctx, data.str, data.len);
    flstr(16)* md5_hash = new_flstr(16);
    md5_finish(&ctx, (uint8_t*) md5_hash);
    return md5_hash;
}

flstr(20)* fstr_sha1(fstr_t data) {
    sha1_context ctx;
    sha1_starts(&ctx);
    sha1_update(&ctx, data.str, data.len);
    flstr(20)* sha1_hash = new_flstr(20);
    sha1_finish(&ctx, (uint8_t*) sha1_hash);
    return sha1_hash;
}

flstr(32)* fstr_sha256(fstr_t data) {
    sha256_context ctx;
    sha256_starts(&ctx, 0);
    sha256_update(&ctx, data.str, data.len);
    flstr(32)* sha256_hash = new_flstr(32);
    sha256_finish(&ctx, (uint8_t*) sha256_hash);
    return sha256_hash;
}

void fstr_reverse_buffer(fstr_t buffer) {
    uint8_t* b_ptr = buffer.str;
    uint8_t* e_ptr = buffer.str + (buffer.len - 1);
    uint64_t mid = buffer.len / 2;
    for (size_t i = 0; i < mid; i++, e_ptr--, b_ptr++)
        FLIP(*b_ptr, *e_ptr);
}

fstr_mem_t* fstr_reverse(fstr_t source) {
    fstr_mem_t* dest = fstr_alloc(source.len);
    if (source.len > 0) {
        const uint8_t* sb_ptr = source.str;
        const uint8_t* se_ptr = source.str + (source.len - 1);
        uint8_t* db_ptr = dest->str;
        uint8_t* de_ptr = dest->str + (dest->len - 1);
        uint64_t mid = source.len / 2;
        for (size_t i = 0; i < mid; i++, sb_ptr++, se_ptr--, db_ptr++, de_ptr--) {
            *db_ptr = *se_ptr;
            *de_ptr = *sb_ptr;
        }
        *db_ptr = *se_ptr;
    }
    return dest;
}

fstr_cfifo_slicev_t fstr_cfifo_slice(fstr_cfifo_t* cfifo, bool free_slice) {
    fstr_cfifo_slicev_t slices = {0};
    size_t offs = (free_slice? (cfifo->data_offs + cfifo->data_len): cfifo->data_offs) % cfifo->buffer.len;
    size_t len = free_slice? (cfifo->buffer.len - cfifo->data_len): cfifo->data_len;
    for (;;) {
        fstr_t slice = fstr_slice(cfifo->buffer, offs, offs + len);
        if (slice.len == 0)
            return slices;
        slices.vec[slices.len] = slice;
        slices.len++;
        offs = 0;
        len -= slice.len;
    }
}

fstr_t fstr_cfifo_read(fstr_cfifo_t* cfifo, fstr_t dst, bool peek) {
    fstr_cfifo_slicev_t slices = fstr_cfifo_slice(cfifo, false);
    fstr_t dst_tail = dst;
    for (uint8_t i = 0; i < slices.len; i++) {
        fstr_t slice = fstr_cpy_over(dst_tail, slices.vec[i], &dst_tail, 0);
        if (!peek) {
            cfifo->data_offs = (cfifo->data_offs + slice.len) % cfifo->buffer.len;
            cfifo->data_len -= slice.len;
        }
    }
    return fstr_detail(dst, dst_tail);
}

fstr_t fstr_cfifo_write(fstr_cfifo_t* cfifo, fstr_t src, bool overwrite) {
    // If we're overwriting we are taking the tail of the source buffer that should fit in the buffer so we always return an empty string.
    // This emulates overwriting the incoming buffer itself without spending the time to do it.
    fstr_t src_tail = overwrite? fstr_slice(src, -cfifo->buffer.len - 1, -1): src;
    for (uint8_t mode = 0; mode < (overwrite? 2: 1) && src_tail.len > 0; mode++) {
        bool is_replacing = (mode == 1);
        fstr_cfifo_slicev_t slices = fstr_cfifo_slice(cfifo, !is_replacing);
        for (uint8_t i = 0; i < slices.len; i++) {
            fstr_t slice = fstr_cpy_over(slices.vec[i], src_tail, 0, &src_tail);
            if (is_replacing) {
                cfifo->data_offs = (cfifo->data_offs + slice.len) % cfifo->buffer.len;
            } else {
                cfifo->data_len += slice.len;
            }
        }
    }
    return src_tail;
}

fstr_t fstr_path_base(fstr_t file_path) {
    size_t i = file_path.len;
    for (;; i--) {
        if (i == 0)
            return fstr_slice(file_path, -1, -1);
        if (file_path.str[i - 1] != '/')
            break;
    }
    size_t j = i;
    for (;; j--) {
        if (j == 0 || file_path.str[j - 1] == '/')
            break;
    }
    return fstr_slice(file_path, j, i);
}

bool fstr_utf8_validate(fstr_t str) {
    fstr_t src_tail = str;
    while (src_tail.len > 0) {
        int32_t uc_point;
        ssize_t i_ret = utf8proc_iterate(src_tail.str, src_tail.len, &uc_point);
        if (i_ret < 0)
            return false;
        assert(i_ret > 0 && i_ret <= 4 && i_ret <= src_tail.len);
        src_tail = fstr_slice(src_tail, i_ret, -1);
    }
    return true;
}

fstr_mem_t* fstr_utf8_clean(fstr_t str) {
    fstr_mem_t* dst_buf = fstr_alloc(str.len * 3);
    fstr_t dst_tail = fss(dst_buf);
    fstr_t src_tail = str;
    while (src_tail.len > 0) {
        // Iterate to next generic character.
        int32_t uc_point;
        ssize_t i_ret = utf8proc_iterate(src_tail.str, src_tail.len, &uc_point);
        if (i_ret < 0) {
            // Unrecognized character, use replacement character fffd.
            ssize_t e_ret = utf8proc_encode_char(0xfffd, dst_tail.str);
            assert(e_ret == 3);
            dst_tail = fstr_slice(dst_tail, 3, -1);
            src_tail = fstr_slice(src_tail, 1, -1);
        } else {
            // Copy over the already encoded character.
            assert(i_ret > 0 && i_ret <= 4);
            fstr_t vc = fstr_slice(src_tail, 0, i_ret);
            (void) fstr_cpy_over(dst_tail, vc, &dst_tail, 0);
            src_tail = fstr_slice(src_tail, i_ret, -1);
        }
    }
    dst_buf->len -= dst_tail.len;
    return dst_buf;
}

#define FSTR_UTF8_NORMALIZE_FN(FUNCTION_NAME, NORMALIZE_FN) fstr_mem_t* FUNCTION_NAME(fstr_t str) { sub_heap { \
    uint8_t* cstr = (void*) fstr_to_cstr(str); \
    uint8_t* cnormal = NORMALIZE_FN(cstr); \
    fstr_mem_t* normal = fstr_from_cstr((void*) cnormal); \
    free(cnormal); \
    return escape(normal); \
}}

FSTR_UTF8_NORMALIZE_FN(fstr_utf8_nrm_nfc, utf8proc_NFC);
FSTR_UTF8_NORMALIZE_FN(fstr_utf8_nrm_nfd, utf8proc_NFD);
FSTR_UTF8_NORMALIZE_FN(fstr_utf8_nrm_nfkc, utf8proc_NFKC);
FSTR_UTF8_NORMALIZE_FN(fstr_utf8_nrm_nfkd, utf8proc_NFKD);

#undef FSTR_UTF8_NORMALIZE_FN

utf8_xid_profile_t fstr_utf8_xidmod(uint32_t chr) {
    utf8_xid_profile_t profile;
    utf8_xid_modification(chr, &profile.status, &profile.type);
    return profile;
}

fstr_mem_t* fstr_utf8_xidmod_filter(fstr_t str, utf8_xid_status_t status_mask, utf8_xid_type_t type_mask) { sub_heap {
    fstr_t src_tail = str;
    // In the worst case we copy the entire string.
    fstr_mem_t* out_buf = fstr_alloc(str.len);
    fstr_t dst_tail = fss(out_buf);
    while (src_tail.len > 0) {
        // Iterate to next Unicode character.
        int32_t uc_point;
        ssize_t i_ret = utf8proc_iterate(src_tail.str, src_tail.len, &uc_point);
        if (i_ret < 0) {
            // Unrecognized character, we always filter those and move one byte forward.
            src_tail = fstr_slice(src_tail, 1, -1);
            continue;
        }
        // Profile this character.
        utf8_xid_profile_t profile = fstr_utf8_xidmod(uc_point);
        if ((profile.status & status_mask) != 0 || (profile.type & type_mask) != 0) {
            // Character not filtered, copy over the already encoded character.
            fstr_t vc = fstr_slice(src_tail, 0, i_ret);
            (void) fstr_cpy_over(dst_tail, vc, &dst_tail, 0);
        }
        // Move forward i_ret bytes.
        src_tail = fstr_slice(src_tail, i_ret, -1);
    }
    // Adjust length of out buffer and return it.
    out_buf->len -= dst_tail.len;
    return escape(out_buf);
}}

fstr_mem_t* fstr_utf8_skeleton(fstr_t str) { sub_heap {
    // 1. "Converting X to NFD format, as described in [UAX15]."
    fstr_t str_nfd = fss(fstr_utf8_nrm_nfd(str));
    // 2. "Successively mapping each source character in X to the target string
    //    according to the specified data table."
    fstr_t src_tail = str_nfd;
    vec(uint32_t)* sk_v = new_vec(uint32_t);
    for (size_t i = 0; i < str_nfd.len; i++) {
        // Iterate to next generic character.
        int32_t uc_point;
        ssize_t i_ret = utf8proc_iterate(src_tail.str, src_tail.len, &uc_point);
        if (i_ret < 0) {
            // Unrecognized character, use replacement character fffd and move one byte forward.
            uc_point = 0xfffd;
            src_tail = fstr_slice(src_tail, 1, -1);
        } else {
            // Move forward i_ret bytes.
            src_tail = fstr_slice(src_tail, i_ret, -1);
        }
        // Map character to confusable character.
        const uint32_t* cf_chrv;
        uint8_t cf_len;
        if (utf8_confusable_ma(uc_point, &cf_chrv, &cf_len)) {
            // Append the skeleton representation characters.
            for (size_t j = 0; j < cf_len; j++) {
                vec_append(sk_v, uint32_t, cf_chrv[j]);
            }
        } else {
            // The character does not belong to a confusable group.
            // Append the character directly to the vector.
            vec_append(sk_v, uint32_t, uc_point);
        }
    }
    // Iterate through the resulting vector and build the UTF-8 string.
    // No UTF-8 character currently need more than 4 bytes.
    fstr_t utf8_buf = fss(fstr_alloc(vec_count(sk_v, uint32_t) * 4));
    fstr_t tail = utf8_buf;
    vec_foreach(sk_v, uint32_t, i, uc_point) {
        if (tail.len < 4)
            break;
        ssize_t len = utf8proc_encode_char(uc_point, tail.str);
        if (len > 0) {
            tail = fstr_slice(tail, len, -1);
        }
    }
    fstr_t mapped_str = fstr_detail(utf8_buf, tail);
    // 3. "Reapplying NFD."
    fstr_mem_t* skeleton = fstr_utf8_nrm_nfd(mapped_str);
    return escape(skeleton);
}}
