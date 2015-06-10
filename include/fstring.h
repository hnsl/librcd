/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef RCD_FSTRING_H
#define	RCD_FSTRING_H

#include "rcd.h"

/// A type representing a string of a fixed length.
#define flstr(fixed_length) \
    struct __rcd_flstr__##fixed_length

/// Converts a fixed-length string to an fstring.
#define flstr_to_fstr(flstr_value, fixed_length) ({ \
    flstr(fixed_length)* typed_flstr = flstr_value; \
    size_t typed_fixed_length = fixed_length; \
    (fstr_t) {.len = typed_fixed_length, .str = (uint8_t*) typed_flstr}; \
})

/// Converts an fstring to a flstr. Throws a fatal exception if the length is wrong.
#define flstr_from_fstr(fstr_value, fixed_length) ({ \
    fstr_t typed_fstr = fstr_value; \
    size_t typed_fixed_length = fixed_length; \
    if (typed_fstr.len != typed_fixed_length) \
        _fstr_fixed_len_error(typed_fixed_length, typed_fstr.len); \
    flstr(fixed_length)* typed_flstr = (flstr(fixed_length)*) typed_fstr.str; \
    typed_flstr; \
})

/// Allocates memory for a new flstr, without initializing it.
#define new_flstr(size) ((flstr(size)*) lwt_alloc_new(size))

/// Returns true if specified byte is an ASCII upper case character.
#define fstr_cisupper(c) ((uint8_t) (c - 'A') < 26)

/// Returns true if specified byte is an ASCII lower case character.
#define fstr_cislower(c) ((uint8_t) (c - 'a') < 26)

/// Returns the specified byte as an ASCII upper case character if it is an
/// ASCII lower case character, otherwise returns the same byte.
#define fstr_ctoupper(c) ({uint8_t _c = c; fstr_cislower(_c)? (_c & 0x5f): _c;})

/// Returns the specified byte as an ASCII lower case character if it is an
/// ASCII upper case character, otherwise returns the same byte.
#define fstr_ctolower(c) ({uint8_t _c = c; fstr_cisupper(_c)? (_c | 32): _c;})

/// Defines a fstr that wraps a C string at compile time.
#define FSTR_CSTR(str) fstr(str "\0")

/// Allocates 'length' bytes of memory on the stack, and assigns it to an fstr.
/// Using a non-constant length *is* possible, but pointless, because variable-
/// length arrays in librcd are allocated on the heap.
#define FSTR_STACK_DECL(name, length) \
    uint8_t _##name##_buf[length]; \
    name.len = length; \
    name.str = _##name##_buf

/// Concatenates all arguments together into a single string.
#define FSTR_CONCAT(...) \
    fstr_concat((fstr_t[]) {__VA_ARGS__}, VA_NARGS(__VA_ARGS__), ((fstr_t){0}))

#define _STR_COMMA(x) STR(x),

/// Converts all arguments into strings with 'STR', and concatenates them.
#define FSTR_CONCAT_ANY(...) \
    fstr_concat((fstr_t[]) { FOR_EACH_ARG(_STR_COMMA, __VA_ARGS__) }, VA_NARGS(__VA_ARGS__), ((fstr_t){0}))

/// Packs a variable of arbitrary type into an fstring by taking its address
/// and size. Useful for serialization.
#define FSTR_PACK(value) \
    ((fstr_t) {.str = (void*) &value, .len = sizeof(value)})

/// Unpacks and returns a variable of a given type from the front of a buffer,
/// then advances the buffer's head by 'sizeof(type)'. Throws exception_io if
/// not enough bytes are available.
#define FSTR_UNPACK(buffer, type) \
    (*(type*)(FSTR_UNPACK_MEM(buffer, sizeof(type)).str))

/// Returns 'memory_length' bytes from the front of a buffer, and advances the
/// buffer's head by the same amount. Throws exception_io if not enough bytes
/// are available.
#define FSTR_UNPACK_MEM(buffer, memory_length) ({ \
    fstr_t v_slice = fstr_slice(buffer, 0, memory_length); \
    buffer = fstr_sslice(buffer, v_slice.len, -1); \
    if (v_slice.len < memory_length) \
        _fstr_unpack_error(); \
    v_slice; \
})

/// Same as FSTR_UNPACK_MEM, but returns an flstr.
#define FSTR_UNPACK_FLSTR(buffer, constant_length) \
    flstr_from_fstr(FSTR_UNPACK_MEM(buffer, constant_length), constant_length)

/// A structure representing allocated memory, to be used as a pointer type.
typedef struct fstr_mem {
    size_t len;
    uint8_t str[];
} fstr_mem_t;

/// The fundamental string type, representing a slice of memory.
typedef struct fstr {
    uint8_t* str;
    size_t len;
} fstr_t;

/// Fixed string circular fifo buffer.
typedef struct fstr_cfifo {
    fstr_t buffer;
    size_t data_offs;
    size_t data_len;
} fstr_cfifo_t;

/// A vector of sliced cfifo parts.
typedef struct fstr_cfifo_slicev {
    uint8_t len;
    fstr_t vec[2];
} fstr_cfifo_slicev_t;

typedef struct fixed_str_buffer fsbuf_t;

typedef int64_t (*fstr_cmp_fn_t)(const fstr_t, const fstr_t);

typedef enum {
    utf8_xid_restricted = 1 << 0,
    utf8_xid_allowed    = 1 << 1,
} utf8_xid_status_t;

typedef enum {
    // Restricted types.
    utf8_xid_default_ignorable = 1 << 0,
    utf8_xid_historic          = 1 << 1,
    utf8_xid_limited_use       = 1 << 2,
    utf8_xid_not_chars         = 1 << 3,
    utf8_xid_not_nfkc          = 1 << 4,
    utf8_xid_not_xid           = 1 << 5,
    utf8_xid_obsolete          = 1 << 6,
    utf8_xid_technical         = 1 << 7,

    // Allowed types.
    utf8_xid_inclusion         = 1 << 8,
    utf8_xid_recommended       = 1 << 9,
} utf8_xid_type_t;

typedef struct utf8_xid_profile {
    utf8_xid_status_t status;
    utf8_xid_type_t type;
} utf8_xid_profile_t;

// Predeclare list(fstr_t) and vec(fstr_t), see https://stackoverflow.com/q/16831605.
list(fstr_t);
vec(fstr_t);

void _fstr_unpack_error();

void _fstr_fixed_len_error(size_t expect_length, size_t actual_length);

/// Converts a c string declaration to a fstring. Not actually implemented
/// as it can't be used to declare fstrings on compile time but instead used as
/// an indicator for the occ preprocessor (occpp) to replace the declaration
/// with a fixed string.
fstr_t* fstr(char* c_string);

/// Converts a c string declaration to a 128 bit hash value. Not actually
/// implemented as it can't be used to declare uids on compile time but instead
/// used as an indicator for the occ preprocessor (occpp) to replace the
/// declaration with a fixed string.
uint128_t fstr_pphash(char* c_string);

/// Returns the fixed string representation of the given memory.
static inline fstr_t fstr_str(fstr_mem_t* fstr_mem) {
    return (fstr_t) {.len = fstr_mem->len, .str = fstr_mem->str};
}

/// Allocates a new fixed string.
/// It guarantees that the returned buffer has space for length bytes.
/// The content that the buffer contains is undefined.
fstr_mem_t* fstr_alloc(size_t length);

/// Like fstr_alloc() but attempts to align the allocation so no space is
/// wasted in memory, useful for buffer when the exact size is not important.
fstr_mem_t* fstr_alloc_buffer(size_t length);

/// Calculates the length of the null terminated c string and initializes a
/// fixed string based on the information, essentially fixates it.
///  (Returns the null terminated c string)
fstr_t fstr_fix_cstr(const char* c_string);

/// Creates a fixed string from the specified fixed length buffer in place.
fstr_t fstr_fix_buffer(void* buffer, size_t length);

/// Allocates a new c string and copies the fixed string to it and null
/// terminates the buffer. As fixed strings is not necessarily null terminated
/// this function should be used when passing the fixed string to interfaces
/// that accept c strings.
char* fstr_to_cstr(fstr_t fstr);

/// Converts a fixed string representing a number and serialized in a certain
/// base to an integer. Ignores characters not specific for the specified base.
/// Returns false if fstr contains a number that is larger than ULONG_MAX.
/// Returns true if unserialization was successful and out_uint64 was set to
/// the unserialized integer. No found specific characters counts as a success
/// and 0 is returned in out_uint64.
bool fstr_unserial_uint(fstr_t fstr, uint8_t base, uint128_t* out_uint128);

/// Exactly like fstr_unserial_uint() but for signed integers.
/// If the given string contains a minus sign prefix it is interpreted as a
/// negative value. Returns zero if the number is larger than LONG_MAX or
/// smaller than LONG_MIN.
bool fstr_unserial_int(fstr_t fstr, uint8_t base, int128_t* out_int128);

/// Attempts to convert a serialized number to an unsigned integer.
/// Returns zero for the same reasons fstr_unserial_uint() would return false.
uint128_t fstr_to_uint(fstr_t fstr, uint8_t base);

/// Attempts to convert a serialized number to an signed integer.
/// Returns zero for the same reasons fstr_unserial_int() would return false.
int128_t fstr_to_int(fstr_t fstr, uint8_t base);

/// Attempts to convert a serialized double to a double.
/// Returns zero if the syntax could not be understood.
double fstr_to_double(fstr_t fstr);

/// Serializes an unsigned integer in specified base and fills a preallocated
/// buffer with the numeric value from the right. If the buffer is too large
/// it will be left padded with zeroes. If the buffer is to small the content
/// is undefined. The slice of the buffer that contains the number without
/// padding is returned.
/// Any text symbols returned are always lowercase.
fstr_t fstr_serial_uint(fstr_t fstr_dest, uint128_t i, uint8_t base);

/// Exactly like fstr_fill_uint() but for signed integers. Returns a minus
/// sign prefix if the number is negative.
/// Any text symbols returned are always lowercase.
fstr_t fstr_serial_int(fstr_t fstr_dest, int128_t i, uint8_t base);

/// Converts signed integer to string with specified padding.
/// Any text symbols returned are always lowercase.
fstr_mem_t* fstr_from_int_pad(int128_t i, uint8_t base, size_t padding);

/// Converts signed integer to string.
/// Any text symbols returned are always lowercase.
fstr_mem_t* fstr_from_int(int128_t i, uint8_t base);

/// Converts unsigned integer to string with specified padding.
/// Any text symbols returned are always lowercase.
fstr_mem_t* fstr_from_uint_pad(uint128_t i, uint8_t base, size_t padding);

/// Converts unsigned integer to string.
/// Any text symbols returned are always lowercase.
fstr_mem_t* fstr_from_uint(uint128_t i, uint8_t base);

/// Converts a double to a string. The returned string will be an exact
/// representation of the input double, including NaN and infinity values.
fstr_mem_t* fstr_from_double(double d);

/// Copies a null terminated c string into a fixed string.
fstr_mem_t* fstr_from_cstr(const char* src);

/// Copies a fixed string.
fstr_mem_t* fstr_cpy(fstr_t src);

/// Copies a fixed string to the destination buffer and returns the
/// slice of the destination buffer that was copied over. Optionally
/// the remaining destination and source tail can be provided as a convenience
/// which is usually useful for buffer operations.
fstr_t fstr_cpy_over(fstr_t dst, fstr_t src, fstr_t* out_dst_tail, fstr_t* out_src_tail);

/// Copies a character to the destination buffer and returns true if there
/// was room for the character or false if the buffer had no remaining space.
static inline bool fstr_putc(fstr_t* dst, uint8_t c) {
    if (dst->len == 0)
        return false;
    dst->str[0] = c;
    dst->len--;
    dst->str++;
    return true;
}

/// Returns the tail of the buffer that starts on "mark".
/// When mark has non zero length the pointer is considered.
/// The pointer represents the first byte in the tail slice returned.
/// When the pointer is before the first byte in full, full is returned.
/// When the pointer is beyond the last byte in full, a zero length string is returned.
/// When mark has zero length a zero length string is returned.
fstr_t fstr_tail(fstr_t full, fstr_t mark);

/// Returns the head of the buffer that is not member of tail.
/// Tail must either have a zero length (in which case full will be returned)
/// or specify a range that is either full or a subset of full where the last
/// byte is also the last byte of full.
/// On error a zero length string is returned.
fstr_t fstr_detail(fstr_t full, fstr_t tail);

/// Completely overwrites all characters in the specified fixed string with
/// the specified character.
void fstr_fill(fstr_t dst, uint8_t chr);

/// Returns a pseudo random string with the specified number of bytes.
/// This stream is not suitable for cryptographic applications.
fstr_mem_t* fstr_pseudo_random(size_t length);

/// Returns a pseudo random hexadecimal string with the specified number of bytes.
fstr_mem_t* fstr_hexrandom(size_t length);

/// Hex encodes a fixed string to lowercase alpha numeric text.
fstr_mem_t* fstr_hexencode(fstr_t bin_src);

/// Hex decodes a fixed string, ignores invalid characters and scans the full
/// string it's given.
fstr_mem_t* fstr_hexdecode(fstr_t hex_src);

/// Generates a hex dump for a binary buffer. Useful for debug output.
fstr_mem_t* fstr_hexdump(fstr_t buf);

/// Generates a hex description of a pointer. Useful for debug output.
fstr_mem_t* fstr_hex_from_ptr(void* ptr);

/// Compares two fixed strings for equality.
bool fstr_equal(const fstr_t str1, const fstr_t str2);

/// Like fstr_equal() but case insensitive (ASCII Latin).
bool fstr_equal_case(fstr_t str1, fstr_t str2);

/// Returns true if str has given prefix.
bool fstr_prefixes(fstr_t str, fstr_t prefix);

/// Like fstr_prefixes() but case insensitive (ASCII Latin).
bool fstr_prefixes_case(fstr_t str, fstr_t prefix);

/// Returns true if str has given suffix.
bool fstr_suffixes(fstr_t str, fstr_t suffix);

/// Like fstr_suffixes() but case insensitive (ASCII Latin).
bool fstr_suffixes_case(fstr_t str, fstr_t suffix);

/// Compares two fixed strings in an undefined but deterministic order
/// that is optimized for speed. Memory is never compared unless the strings
/// have an equal size and point to different regions of memory.
int64_t fstr_cmp(const fstr_t str1, const fstr_t str2);

/// Like fstr_cmp_case() but case insensitive (ASCII Latin).
int64_t fstr_cmp_case(fstr_t str1, fstr_t str2);

/// Compares two fixed strings according to the rules of lexicographical order
/// which is slower than undefined deterministic ordering.
int64_t fstr_cmp_lexical(const fstr_t str1, const fstr_t str2);

/// Compares two fixed strings natural order relationship.
int64_t fstr_cmp_natural(fstr_t str1, fstr_t str2);

/// Takes a C array of fixed strings and sorts them.
/// If lexical is true the sorting will be done in lexical order.
void fstr_sort(fstr_t fstr_list[], size_t n_fstr_list, bool lexical);

/// Takes a C array of fixed strings and sorts them in natural order.
void fstr_sort_nat(fstr_t fstr_list[], size_t n_fstr_list);

/// Gets the next string in non lexicographical order.
/// E.g. "" -> "\x00" -> "\x01" -> ... -> "\xff" -> "\x00\x00" -> "\x00\x01"
fstr_mem_t* fstr_order_next(fstr_t src);

/// Gets the previous string in non lexicographical order.
/// Throws logical exception if given the empty string.
fstr_mem_t* fstr_order_prev(fstr_t src);

/// Copies a string to lower case (ASCII Latin).
fstr_mem_t* fstr_lower(fstr_t src);

/// Copies a string to upper case (ASCII Latin).
fstr_mem_t* fstr_upper(fstr_t src);

/// Converts the given string to lower case (ASCII Latin).
/// This function modifies memory in-place.
static inline void fstr_tolower(fstr_t str) {
    for (size_t i = 0; i < str.len; i++)
        str.str[i] = fstr_ctolower(str.str[i]);
}

/// Converts the given string to upper case (ASCII Latin).
/// This function modifies memory in-place.
static inline void fstr_toupper(fstr_t str) {
    for (size_t i = 0; i < str.len; i++)
        str.str[i] = fstr_ctoupper(str.str[i]);
}

/// Creates a reference to the slice of memory that str contains. Offs0 is the
/// offset where the slice starts and offs1 is the offset where the next slice
/// starts that will be cut off from the region returned.
/// If offs0 > offs1 the returned .len will be zero but the .str is undefined.
/// The offset coordinate system allows negative offsets. Negative offsets are
/// interpreted as "distance from two characters after the last character".
/// This means -1 refers to the character after the last character in the
/// string, -2 refers to the last character and so on.
/// Examples for "abcdefg":
///     (0, 0) -> "", (0, 1) -> "a", (0, 2) -> "ab", (1, 3) -> "bc"
///     (0, -1) -> "abcdefg", (0, -2) -> "abcdef", (2, -3) -> "cde"
///     (-2, -1) -> "g", (-3, -1) -> "fg", (-5, -2) -> "def",
static inline fstr_t fstr_slice(fstr_t str, int64_t offs0, int64_t offs1) {
    // Negative adjust + cutoff.
    if (offs0 < 0LL)
        offs0 = MAX(0LL, str.len + 1 + offs0);
    if (offs1 < 0LL)
        offs1 = MAX(0LL, str.len + 1 + offs1);
    // Positive cutoff.
    offs0 = MIN(offs0, str.len);
    offs1 = MIN(offs1, str.len);
    // Slice.
    fstr_t new_str;
    if (offs0 > offs1) {
        new_str.str = str.str;
        new_str.len = 0;
    } else {
        new_str.str = str.str + offs0;
        new_str.len = offs1 - offs0;
    }
    return new_str;
}

/// Deprecated alias for fstr_slice().
#define fstr_sslice fstr_slice

/// Scans the string from left to right after a sub string and returns the
/// offset where it is first found. If no such substring is found the function
/// returns -1. This scanning has quadratic worst-case complexity (N*M) where N
/// is the length of str and M is the length of sub_str.
int64_t fstr_scan(fstr_t str, fstr_t sub_str);

/// Like fstr_scan but scans in reverse. It returns the offset where the
/// specified sub string is last found. If no such substring is found the
/// function returns -1.
int64_t fstr_rscan(fstr_t str, fstr_t sub_str);

/// Takes a C array of fixed strings and concatenates them.
/// Returns an allocated empty string if n_fstr_list = 0.
fstr_mem_t* fstr_concat(fstr_t fstr_list[], size_t n_fstr_list, fstr_t glue);

/// Takes a vector of fixed strings and concatenates them.
/// Returns an allocated empty string if length is zero.
fstr_mem_t* fstr_concatv(vec(fstr_t)* vec, fstr_t glue);

/// Takes a list of fixed strings and glues them together.
/// Returns an allocated empty string if list_count(fstr_list, fstr_t) == 0.
fstr_mem_t* fstr_implode(list(fstr_t)* fstr_list, fstr_t glue);

/// Takes a fixed string and returns a C array of chunks based on a certain
/// delimiter. The references returned refers to in-place slices
/// of the source string. If the delimiter has zero length a list is returned
/// with a singe element, the specified source string.
list(fstr_t)* fstr_explode(fstr_t src, fstr_t delimiter);

/// Takes a source text, finds elements and replaces them. This function
/// is implemented using fstr_implode() and fstr_explode() and has the same
/// run-time complexity limitations.
fstr_mem_t* fstr_replace(fstr_t source, fstr_t find, fstr_t replace);

/// Finds the first instance of the specified separator in the source and
/// returns the range before and after unless null.
/// Returns false if the separator is not found.
bool fstr_divide(fstr_t src, fstr_t separator, fstr_t* out_before, fstr_t* out_after);

/// Like fstr_divide() but divides in reverse.
bool fstr_rdivide(fstr_t src, fstr_t separator, fstr_t* out_before, fstr_t* out_after);

/// Iterates over a series of chunks in a string joined with the specified
/// separator. The io_str will be modified so the first chunk and separator
/// is skipped. The next chunk to iterate over is returned in out_next unless
/// it is 0. The length of the io_str string will shrink for each iteration.
/// When the function is called with a zero length string in io_str or separator
/// the function return false, otherwise the function will always return true
/// and return the next chunk in out_next.
bool fstr_iterate(fstr_t* io_str, fstr_t separator, fstr_t* out_next);

/// Like fstr_iterate() but iterates in reverse.
bool fstr_riterate(fstr_t* io_str, fstr_t separator, fstr_t* out_next);

/// Like fstr_iterate() but will trim the input string before iterating over it.
/// This means that leading and trailing whitespace have no effect.
/// The returned separator in out_next is also trimmed. Useful when parsing
/// text files or data that is heavily white space separated.
/// The function returns false immediately if given a string with only
/// whitespace since it will have a zero initial length.
bool fstr_iterate_trim(fstr_t* io_str, fstr_t separator, fstr_t* out_next);

/// Like fstr_iterate_trim() but iterates in reverse.
bool fstr_riterate_trim(fstr_t* io_str, fstr_t separator, fstr_t* out_next);

/// Takes a fixed string and returns the slice of it that has been rid of any
/// leading or trailing whitespace.
fstr_t fstr_trim(fstr_t fstr);

/// Encodes binary data in base64.
/// The returned output is approximately 4 / 3 the size of the binary data.
/// It includes padding '=' characters to make the returned length dividable by 4.
fstr_mem_t* fstr_base64_encode(fstr_t binary_data);

/// Decodes a base64 to binary data. Invalid base64 characters are ignored.
/// The returned output is ~ 3 / 4 the size of the valid base64 text.
/// Partial base64 data will have its corrupt tail silently discarded.
/// Padding '=' characters is not required and will be silently discarded.
fstr_mem_t* fstr_base64_decode(fstr_t base64_text);

/// Encodes binary data in base32.
fstr_mem_t* fstr_base32_encode(fstr_t s);

/// Decodes base32 to binary data.
fstr_mem_t* fstr_base32_decode(fstr_t s);

/// Encodes a binary string in valid ASCII.
/// The characters \r, \n and \ is escaped in their standard c escape format.
/// Other control characters and undefined characters are hex escaped.
fstr_mem_t* fstr_ace_encode(fstr_t decoded);

/// Decodes a string escaped with fstr_ace_encode().
/// Throws an io exception is the format is not recognized.
fstr_mem_t* fstr_ace_decode(fstr_t encoded);

/// Computes md5 hash of the specified data.
flstr(16)* fstr_md5(fstr_t data);

/// Computes sha1 hash of the specified data.
flstr(20)* fstr_sha1(fstr_t data);

/// Computes sha256 hash of the specified data.
flstr(32)* fstr_sha256(fstr_t data);

/// Reverses a given string in place
void fstr_reverse_buffer(fstr_t buffer);

/// Returns a new string with the reversed content of the source string
fstr_mem_t* fstr_reverse(const fstr_t source);

/// Generic string conversion functions. Strings, integers, doubles and
/// pointers are supported.
/// (Note: for convenience, this uses __attribute__((overloadable)) which is a
/// clang extension. In theory we could replace it by the standard C11 _Generic,
/// but it would make preprocessor output quite bloated and hard to read.)
__attribute__((overloadable)) static inline fstr_t STR(uint128_t x) { return fss(fstr_from_uint(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(uint64_t x) { return fss(fstr_from_uint(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(uint32_t x) { return fss(fstr_from_uint(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(uint16_t x) { return fss(fstr_from_uint(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(uint8_t x) { return fss(fstr_from_uint(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(int128_t x) { return fss(fstr_from_int(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(int64_t x) { return fss(fstr_from_int(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(int32_t x) { return fss(fstr_from_int(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(int16_t x) { return fss(fstr_from_int(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(int8_t x) { return fss(fstr_from_int(x, 10)); }
__attribute__((overloadable)) static inline fstr_t STR(double x) { return fss(fstr_from_double(x)); }
__attribute__((overloadable)) static inline fstr_t STR(void* x) { return fss(fstr_hex_from_ptr(x)); }
__attribute__((overloadable)) static inline fstr_t STR(fstr_t x) { return x; }

/// Creates a new fixed string circular fifo buffer.
static inline fstr_cfifo_t fstr_cfifo_init(fstr_t buffer) {
    return (fstr_cfifo_t) {
        .buffer = buffer,
        .data_offs = 0,
        .data_len = 0,
    };
}

/// Joins together an fstr_cfifo_slicev_t into a single string.
static inline fstr_t fstr_cfifo_join(fstr_cfifo_slicev_t slices) {
    if (slices.len == 0)
        return fstr("");
    else if (slices.len == 1)
        return slices.vec[0];
    else
        return concs(slices.vec[0], slices.vec[1]);
}

/// Slices the circular fifo to get the memory segments.
/// If free_slice is true the function returns the slices of free memory.
/// If free_slice is false the function returns the slices of used memory.
fstr_cfifo_slicev_t fstr_cfifo_slice(fstr_cfifo_t* cfifo, bool free_slice);

/// Returns the length of the exiting data in the cfifo buffer.
static inline size_t fstr_cfifo_len_data(fstr_cfifo_t* cfifo) {
    return cfifo->data_len;
}

/// Returns the length of the free space in the cfifo buffer.
static inline size_t fstr_cfifo_len_space(fstr_cfifo_t* cfifo) {
    return cfifo->buffer.len - cfifo->data_len;
}

/// Reads from a new fixed string circular fifo buffer.
/// Returns the initial slice of dst that was copied to.
/// If peek is false the cfifo is modified so the read data is consumed.
fstr_t fstr_cfifo_read(fstr_cfifo_t* cfifo, fstr_t dst, bool peek);

/// Writes a to a fixed string circular fifo buffer.
/// Returns the tail of src that was not copied because it did not fit.
/// If overwrite is true an empty string is always returned and the tail of the source buffer that fit was written.
fstr_t fstr_cfifo_write(fstr_cfifo_t* cfifo, fstr_t src, bool overwrite);

/// Returns the base name of the specified path. The base name is the last
/// slice of the path after the last non-trailing forward slash.
/// The base name is also informally known as the file name.
fstr_t fstr_path_base(fstr_t file_path);

/// Counts the length of the UTf-8 string. Takes O(N) time.
/// Invalid character sequences is counted as one character per byte.
size_t fstr_utf8_len(fstr_t str);

/// Validates the specified UTF-8 string. If invalid characters is found the
/// function returns false, otherwise it returns true.
bool fstr_utf8_validate(fstr_t str);

/// Deprecated alias for fstr_utf8_validate().
#define fstr_validate_utf8 fstr_utf8_validate

/// Allocates a new buffer that will contain valid UTF-8, invalid bytes will
/// be replaced with unicode character 0xFFFD (\xEFBFBD).
fstr_mem_t* fstr_utf8_clean(fstr_t str);

/// Deprecated alias for fstr_utf8_clean().
#define fstr_clean_utf8 fstr_utf8_clean

/// Returns a new string that a Unicode lowercased version of the input string.
fstr_mem_t* fstr_utf8_lower(fstr_t str);

/// Returns a new string that a Unicode uppercased version of the input string.
fstr_mem_t* fstr_utf8_upper(fstr_t str);

/// Low level UTF-8 mapping.
/// Decomposes the UTF-8 string and reassembles it into a new allocation
/// based on the specified options. See utf8proc.h and utf8proc_map()
/// for more information.
/// Throws an io exception decompose or re-encode fails.
fstr_mem_t* fstr_utf8_map(fstr_t str, int options);

/// NFC normalizes a UTF-8 string.
/// "NFC is the best form for general text, since it is more compatible
/// with strings converted from legacy encodings."
/// Strings that contains a null byte will be truncated in the output.
fstr_mem_t* fstr_utf8_nrm_nfc(fstr_t str);

/// NFD normalizes a UTF-8 string.
/// "NFD and NFKD are most useful for internal processing."
/// Strings that contains a null byte will be truncated in the output.
fstr_mem_t* fstr_utf8_nrm_nfd(fstr_t str);

/// NFKC normalizes a UTF-8 string.
/// "NFKC is the preferred form for identifiers, especially where there are
/// security concerns (see UTR #36)."
/// Strings that contains a null byte will be truncated in the output.
fstr_mem_t* fstr_utf8_nrm_nfkc(fstr_t str);

/// NFKD normalizes a UTF-8 string.
/// "NFD and NFKD are most useful for internal processing."
/// Strings that contains a null byte will be truncated in the output.
fstr_mem_t* fstr_utf8_nrm_nfkd(fstr_t str);

/// See: http://www.unicode.org/reports/tr39/#Identifier_Characters
///
/// Queries a general security profile for a character for the purpose of
/// being used in an identifier context. Characters are more or less suitable
/// to be used as identifiers. The returned information can be used to filter
/// or reject characters determined to be unsuitable for identifier in a
/// particular application.
utf8_xid_profile_t fstr_utf8_xidmod(uint32_t chr);

/// Creates a new UTF-8 string with characters filtered from the original UTF-8
/// string based on a xidmod profile mask. To not be filtered, the characters
/// must neither match the status_mask NOR match the type_mask.
/// For example, to only use the type_mask, set the status_mask to 0.
/// See fstr_utf8_xidmod() for more information.
fstr_mem_t* fstr_utf8_xidmod_filter(fstr_t str, utf8_xid_status_t status_mask, utf8_xid_type_t type_mask);

/// This function is defined ("skeleton()") by Unicode Technical Standard #39:
/// http://www.unicode.org/reports/tr39/#Confusable_Detection
///
/// It's purpose is to idempotently map a string X to a new skeleton string.
/// The function groups similar characters and picks an arbitrary (but well
/// defined) character to represent that group. This skeleton string should
/// be thought of as an intermediate processing form, similar to a hashcode
/// and is *not* intended for display, storage or transmission.
////
/// The primary purpose of the skeleton string is to allow binary comparision
/// to detect strings which is likely to be confusable. One usecase for this
/// is ensuring that identifiers are equivalent if they look equivalent for
/// security and user experience purposes.
fstr_mem_t* fstr_utf8_skeleton(fstr_t str);

/// Internal UTF-8 function.
bool utf8_confusable_ma(uint32_t in_chr, const uint32_t* out_chrv[], uint8_t* out_len);

/// Internal UTF-8 function.
void utf8_xid_modification(uint32_t in_chr, utf8_xid_status_t* out_status, utf8_xid_type_t* out_type);

#endif /* RCD_FSTRING_H */
