/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/* Minimal reflection library for debugging and scripting convenience. */

#include "rcd.h"
#include "linux.h"

#pragma librcd

#pragma pack(push, 1)

typedef struct rfl_addr_line_mapping {
    uint32_t addr;
    uint32_t len;
    uint32_t line;
} rfl_addr_line_mapping_t;

typedef struct rfl_addr_fstr_mapping {
    uint32_t addr;
    uint32_t len;
    uint16_t fstr_len;
    uint8_t* fstr_ptr;
} rfl_addr_fstr_mapping_t;

typedef struct rfl_addr_index {
    rfl_addr_line_mapping_t* addr_line_map_ptr;
    uint64_t addr_line_map_n;
    rfl_addr_fstr_mapping_t* addr_file_map_ptr;
    uint64_t addr_file_map_n;
    rfl_addr_fstr_mapping_t* addr_func_map_ptr;
    uint64_t addr_func_map_n;
} rfl_addr_index_t;

#pragma pack(pop)

/// This variable ensures that a special section is created called rfl_raw_line_table that can easily be referenced when post-processing the elf.
/// It should be set to a non-zero value indicating that the rfl_raw_line_table was injected and that it works.
rfl_addr_index_t* rfl_addr_index __attribute__((section("rfl_addr_index"))) = 0;

#define RFL_LOOKUP_ADDR(PTR_NAME, N_NAME, SUCCESS_CODE) { \
    if (rfl_addr_index == 0) \
        return false; \
    typeof(rfl_addr_index->PTR_NAME) addr_map = rfl_addr_index->PTR_NAME; \
    if (i_addr > (void*) 0xffffffff) \
        return false; \
    uint32_t i_addr_32 = (uint32_t) i_addr; \
    uint64_t low_i = 0, high_i = rfl_addr_index->N_NAME; \
    while (high_i > low_i) { \
        uint64_t cmp_i = low_i + (high_i - low_i) / 2; \
        uint32_t addr = addr_map[cmp_i].addr; \
        if (i_addr_32 < addr) { \
            high_i = cmp_i; \
        } else { \
            if (i_addr_32 < addr + addr_map[cmp_i].len) { \
                SUCCESS_CODE \
                return true; \
            } \
            low_i = cmp_i + 1; \
        } \
    } \
    return false; \
}

bool rfl_addr_to_line(void* i_addr, uint32_t* out_line) {
    RFL_LOOKUP_ADDR(addr_line_map_ptr, addr_line_map_n, {*out_line = addr_map[cmp_i].line;});
}

bool rfl_addr_to_file(void* i_addr, fstr_t* out_file) {
    RFL_LOOKUP_ADDR(addr_file_map_ptr, addr_file_map_n, {out_file->len = addr_map[cmp_i].fstr_len; out_file->str = addr_map[cmp_i].fstr_ptr;});
}

bool rfl_addr_to_func(void* i_addr, fstr_t* out_func) {
    RFL_LOOKUP_ADDR(addr_func_map_ptr, addr_func_map_n, {out_func->len = addr_map[cmp_i].fstr_len; out_func->str = addr_map[cmp_i].fstr_ptr;});
}

fstr_mem_t* rfl_addr_to_location(void* i_addr) { sub_heap {
    list(fstr_t)* chunks = new_list(fstr_t);
    list_push_end(chunks, fstr_t, "0x");
    list_push_end(chunks, fstr_t, fss(fstr_from_uint((uint64_t) i_addr, 16)));
    list_push_end(chunks, fstr_t, " ");
    fstr_t func;
    if (!rfl_addr_to_func(i_addr, &func))
        func = "?";
    list_push_end(chunks, fstr_t, func);
    list_push_end(chunks, fstr_t, " in ");
    fstr_t file;
    if (!rfl_addr_to_file(i_addr, &file))
        file = "?";
    list_push_end(chunks, fstr_t, file);
    list_push_end(chunks, fstr_t, ":");
    uint32_t line;
    if (rfl_addr_to_line(i_addr, &line)) {
        list_push_end(chunks, fstr_t, fss(fstr_from_uint(line, 10)));
    } else {
        list_push_end(chunks, fstr_t, "?");
    }
    return escape(fstr_implode(chunks, ""));
}}

fstr_t rfl_addr_to_location_inp(fstr_t buffer, void* i_addr) {
    // Compute address string.
    fstr_t addr_buf;
    FSTR_STACK_DECL(addr_buf, 32);
    fstr_t addr_s = fstr_serial_uint(addr_buf, (size_t) i_addr, 16);
    // Compute line string.
    fstr_t line_buf;
    FSTR_STACK_DECL(line_buf, 32);
    fstr_t line_s;
    uint32_t line;
    if (rfl_addr_to_line(i_addr, &line)) {
        line_s = fstr_serial_uint(line_buf, line, 10);
    } else {
        line_s = "?";
    }
    // Fetch function string.
    fstr_t func;
    if (!rfl_addr_to_func(i_addr, &func))
        func = "?";
    // Fetch file string.
    fstr_t file;
    if (!rfl_addr_to_file(i_addr, &file))
        file = "?";
    // Concatenate full pretty print.
    fstr_t buf_tail = buffer;
    fstr_cpy_over(buf_tail, "0x", &buf_tail, 0);
    fstr_cpy_over(buf_tail, addr_s, &buf_tail, 0);
    fstr_cpy_over(buf_tail, " ", &buf_tail, 0);
    fstr_cpy_over(buf_tail, func, &buf_tail, 0);
    fstr_cpy_over(buf_tail, " in ", &buf_tail, 0);
    fstr_cpy_over(buf_tail, file, &buf_tail, 0);
    fstr_cpy_over(buf_tail, ":", &buf_tail, 0);
    fstr_cpy_over(buf_tail, line_s, &buf_tail, 0);
    return (fstr_t) {.str = buffer.str, .len = buf_tail.str - buffer.str};
}
