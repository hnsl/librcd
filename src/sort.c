/*
 * This file contains source code copied from Linux.
 *
 * Linux is licenced under the GPLv2:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 * For more information see: https://www.kernel.org/
 */
/*
 * A fast, small, non-recursive O(nlog n) sort for the Linux kernel
 *
 * Jan 23 2005  Matt Mackall <mpm@selenic.com>
 */
#include "rcd.h"
#include "musl.h"

#define SWAP_FN(bits) \
static void u##bits##_swap(void* a, void* b, size_t size) { \
    uint##bits##_t *a_v = a, *b_v = b; \
    FLIP(*a_v, *b_v); \
}

SWAP_FN(8);

SWAP_FN(16);

SWAP_FN(32);

SWAP_FN(64);

SWAP_FN(128);

static void generic_swap(void* a, void* b, size_t size) {
    uint8_t swap_mem[size];
    memcpy(swap_mem, a, size);
    memcpy(a, b, size);
    memcpy(b, swap_mem, size);
}

/**
 * sort - sort an array of elements
 * @base: pointer to data to sort
 * @num: number of elements
 * @size: size of each element
 * @cmp_func: pointer to comparison function
 * @swap_func: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap_func function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */
void sort(void* base, size_t num, size_t size, int32_t (*cmp_func)(const void*, const void*), void (*swap_func)(void*, void*, size_t size)) {
    // pre-scale counters for performance
    if (swap_func == 0) {
        switch (size) {
        case 1:
            swap_func = u8_swap;
            break;
        case 2:
            swap_func = u16_swap;
            break;
        case 4:
            swap_func = u32_swap;
            break;
        case 8:
            swap_func = u64_swap;
            break;
        case 16:
            swap_func = u128_swap;
            break;
        default:
            swap_func = generic_swap;
            break;
        }
    }
    // heapify
    int32_t n = num * size;
    for (int32_t i = (num / 2 - 1) * size; i >= 0; i -= size) {
        for (int32_t r = i, c; r * 2 + size < n; r = c) {
            c = r * 2 + size;
            if (c < n - size && cmp_func(base + c, base + c + size) < 0)
                c += size;
            if (cmp_func(base + r, base + c) >= 0)
                break;
            swap_func(base + r, base + c, size);
        }
    }
    // sort
    for (int32_t i = n - size; i > 0; i -= size) {
        swap_func(base, base + i, size);
        for (int32_t r = 0, c; r * 2 + size < i; r = c) {
            c = r * 2 + size;
            if (c < i - size && cmp_func(base + c, base + c + size) < 0)
                c += size;
            if (cmp_func(base + r, base + c) >= 0)
                break;
            swap_func(base + r, base + c, size);
        }
    }
}
