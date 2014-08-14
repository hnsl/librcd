/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */
/* Note that this copyright does not apply to the getdelim() function. */
/* See the copyright notice above it for more information. */

#include "rcd.h"
#include "musl.h"

/* musl <-> librcd compatibility layer */

#pragma librcd

typedef struct stdio_file_state {
    rio_t* rio_h;
    bool is_error;
    bool is_eof;
} stdio_file_state_t;

struct __FILE_s {
    rcd_fid_t stdio_file_fid;
};

struct __FILE_s stdin_mem;
FILE* const stdin = &stdin_mem;

struct __FILE_s stdout_mem;
FILE* const stdout = &stdout_mem;

struct __FILE_s stderr_mem;
FILE* const stderr = &stderr_mem;


join_locked(size_t) stdio_file_fiber_fread(void *ptr, size_t size, size_t nmemb, join_server_params, stdio_file_state_t* state) {
    if (state->is_eof)
        return 0;
    size_t r_value;
    sub_heap {
        try {
            fstr_t buffer = {.str = ptr, .len = size * nmemb};
            fstr_t read_chunk = rio_read_to_end(state->rio_h, buffer);
            r_value = read_chunk.len;
            state->is_eof = (read_chunk.len < buffer.len);
        } catch (exception_io, e) {
            r_value = 0;
            state->is_error = true;
        }
    }
    return r_value;
}

join_locked(size_t) stdio_file_fiber_fwrite(const void *ptr, size_t size, size_t nmemb, join_server_params, stdio_file_state_t* state) {
    size_t r_value;
    sub_heap {
        try {
            fstr_t buffer = {.str = (void*) ptr, .len = size * nmemb};
            rio_write(state->rio_h, buffer);
            r_value = nmemb;
        } catch (exception_io, e) {
            r_value = 0;
            state->is_error = true;
        }
    }
    return r_value;
}

join_locked(int32_t) stdio_file_fiber_fseek(int64_t offset, int32_t whence, join_server_params, stdio_file_state_t* state) {
    int32_t r_value = 0;
    sub_heap {
        try {
            switch (whence) {
            case SEEK_SET:
                rio_set_file_offset(state->rio_h, offset, false);
                break;
            case SEEK_CUR:
                rio_set_file_offset(state->rio_h, offset, true);
                break;
            case SEEK_END:
                rio_set_file_offset_end(state->rio_h, offset);
                break;
            default:
                // "The whence argument to fseek() was not SEEK_SET, SEEK_END, or SEEK_CUR."
                errno = EINVAL;
                r_value = -1;
                break;
            }
        } catch (exception_io, e) {
            errno = e->errno_snapshot;
            r_value = -1;
        }
    }
    return r_value;
}

join_locked(int64_t) stdio_file_fiber_ftell(join_server_params, stdio_file_state_t* state) {
    int64_t r_value;
    sub_heap {
        try {
            r_value = rio_get_file_offset(state->rio_h);
        } catch (exception_io, e) {
            errno = e->errno_snapshot;
            r_value = -1;
        }
    }
    return r_value;
}

join_locked(void) stdio_file_fiber_clearerr(join_server_params, stdio_file_state_t* state) {
    state->is_error = false;
    state->is_eof = false;
}

join_locked(int32_t) stdio_file_fiber_feof(join_server_params, stdio_file_state_t* state) {
    return state->is_eof? 1: 0;
}

join_locked(int32_t) stdio_file_fiber_ferror(join_server_params, stdio_file_state_t* state) {
    return state->is_error? 1: 0;
}

join_locked(int32_t) stdio_file_fiber_fileno(join_server_params, stdio_file_state_t* state) {
    int32_t fd = rio_get_fd_read(state->rio_h);
    return (fd == -1)? fd: rio_get_fd_write(state->rio_h);
}

fiber_main stdio_file_fiber(fiber_main_attr, rio_t* rio_h) {
    stdio_file_state_t state = {
        .rio_h = rio_h,
        .is_error = false,
        .is_eof = false,
    };
    try {
        auto_accept_join(stdio_file_fiber_fread, stdio_file_fiber_fwrite, stdio_file_fiber_fseek, stdio_file_fiber_ftell, \
        stdio_file_fiber_clearerr, stdio_file_fiber_feof, stdio_file_fiber_ferror, stdio_file_fiber_fileno \
        , join_server_params, &state);
    } catch (exception_canceled, e) {}
}

static void stdio_init_file_handle(FILE* ret_fh, rio_t* rio_h) {
    fmitosis {
        lwt_alloc_import(rio_h);
        ret_fh->stdio_file_fid = spawn_static_fiber(stdio_file_fiber("", rio_h));
    }
}

static FILE* stdio_new_file_handle(rio_t* rio_h) {
    FILE* ret_fh = malloc(sizeof(FILE));
    stdio_init_file_handle(ret_fh, rio_h);
    return ret_fh;
}

/// Called once from lwthreads to initialize stdio.
void stdio_init() {
    stdio_init_file_handle(&stdin_mem, rio_stdin());
    stdio_init_file_handle(&stdout_mem, rio_stdout());
    stdio_init_file_handle(&stderr_mem, rio_stderr());
}

FILE* fopen(const char* path, const char* mode) {
    FILE* ret_fh = 0;
    sub_heap {
        try {
            // Check for valid initial mode character
            fstr_t ok_mode = "rwa\0";
            if (!strchr((char*) ok_mode.str, *mode)) {
                errno = EINVAL;
                break;
            }
            // Parse basic flags.
            bool read_only = (*mode == 'r') && !strchr(mode, '+');
            bool create = (*mode != 'r');
            rio_t* rio_h = rio_file_open(fstr_fix_cstr(path), read_only, create);
            if (*mode == 'w') {
                // Truncate.
                rio_file_truncate(rio_h, 0);
            } else if (*mode == 'a') {
                // Append.
                rio_set_file_offset_end(rio_h, 0);
            }
            ret_fh = stdio_new_file_handle(rio_h);
        } catch (exception_io, e) {
            errno = e->errno_snapshot;
        }
    }
    return ret_fh;
}

FILE* fdopen(int32_t fd, const char* mode) {
    return stdio_new_file_handle(rio_new_h(rio_type_file, fd, true, true, 0));
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_fread(ptr, size, nmemb, stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            ret = 0;
        }
    }
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_fwrite(ptr, size, nmemb, stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            ret = 0;
        }
    }
    return ret;
}

int32_t fflush(FILE* stream) {
    return 0;
}

int32_t fclose(FILE* stream) {
    lwt_cancel_fiber_id(stream->stdio_file_fid);
    free(stream);
    return 0;
}

int32_t fseek(FILE *stream, int64_t offset, int32_t whence) {
    size_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_fseek(offset, whence, stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            errno = EBADF;
            ret = -1;
        }
    }
    return ret;
}

int64_t ftell(FILE *stream) {
    size_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_ftell(stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            errno = EBADF;
            ret = -1;
        }
    }
    return ret;
}

void rewind(FILE *stream) {
    return (void) fseek(stream, 0L, SEEK_SET);
}

void clearerr(FILE *stream) {
    sub_heap {
        try {
            stdio_file_fiber_clearerr(stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
        }
    }
}

int32_t feof(FILE *stream) {
    int32_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_feof(stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            errno = EBADF;
            ret = -1;
        }
    }
    return ret;
}

int32_t ferror(FILE *stream) {
    int32_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_ferror(stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            errno = EBADF;
            ret = -1;
        }
    }
    return ret;
}

int32_t fileno(FILE *stream) {
    int32_t ret;
    sub_heap {
        try {
            ret = stdio_file_fiber_fileno(stream->stdio_file_fid);
        } catch (exception_inner_join_fail, e) {
            // Undefined behavior.
            errno = EBADF;
            ret = -1;
        }
    }
    return ret;
}

int fgetc(FILE *stream) {
    char chr;
    return fread(&chr, 1, 1, stream) == 1? chr: EOF;
}

char *fgets(char *s, int size, FILE *stream) {
    if (size == 0)
        return 0;
    for (size_t i = 0;; i++) {
        if (i == size - 1) {
            s[i] = 0;
            break;
        }
        char chr = fgetc(stream);
        if (chr == EOF)
            return 0;
        s[i] = chr;
        if (chr == '\n' || chr == '\0') {
            s[i + 1] = 0;
            break;
        }
    }
    return s;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

int getchar(void) {
    return getc(stdin);
}

// char *gets(char *s) {}

int fputc(int c, FILE* stream) {
    return fwrite(&c, 1, 1, stream) == 1? 1: EOF;
}

int fputs(const char *s, FILE* stream) {
    size_t len = strlen(s);
    return (fwrite(s, 1, len, stream) == len)? len: EOF;
}

int putc(int c, FILE* stream) {
    char chr = c;
    return fputs(&chr, stream);
}

int putchar(int c) {
    return putc(c, stdout);
}

int puts(const char *s) {
    int32_t ret = 0;
    {
        int32_t fputs_r = fputs(s, stdout);
        if (fputs_r == EOF)
            return EOF;
        ret += fputs_r;
    }{
        int32_t fputs_r = fputc('\n', stdout);
        if (fputs_r == EOF)
            return EOF;
        ret += fputs_r;
    }
    return ret;
}

/* Copyright (C) 1994-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

/// Read up to (and including) a DELIMITER from FP into *LINEPTR (and
/// NUL-terminate it). *LINEPTR is a pointer returned from malloc (or
/// NULL), pointing to *N characters of space. It is realloc'ed as
/// necessary.  Returns the number of characters read (not including
/// the null terminator), or -1 on error or EOF.
ssize_t getdelim(char** lineptr, size_t* n, int delimiter, FILE* fp) {
    int result = 0;
    ssize_t cur_len = 0;
    if (lineptr == 0 || n == 0 || fp == 0) {
        errno = EINVAL;
        return -1;
    }
    if (*lineptr == 0 || *n == 0) {
        *n = 120;
        *lineptr = (char *) malloc(*n);
        if (*lineptr == 0) {
            result = -1;
            goto unlock_return;
        }
    }
    for (;;) {
        int i = getc(fp);
        if (i == EOF) {
            result = -1;
            break;
        }
        // Make enough space for len+1 (for final NUL) bytes.
        if (cur_len + 1 >= *n) {
            size_t needed = 2 * (cur_len + 1) + 1; // Be generous.
            char *new_lineptr;
            if (needed < cur_len) {
                result = -1;
                goto unlock_return;
            }
            new_lineptr = (char*) realloc(*lineptr, needed);
            if (new_lineptr == 0) {
                result = -1;
                goto unlock_return;
            }
            *lineptr = new_lineptr;
            *n = needed;
        }
        (*lineptr)[cur_len] = i;
        cur_len++;
        if (i == delimiter)
            break;
    }
    (*lineptr)[cur_len] = '\0';
    result = cur_len? cur_len: result;
unlock_return:
    return result;
}

ssize_t getline(char** lineptr, size_t* n, FILE* fp) {
    return getdelim(lineptr, n, '\n', fp);
}
