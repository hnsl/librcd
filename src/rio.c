/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"
#include "vm-internal.h"
#include "lwthreads-internal.h"
#include "atomic.h"

#pragma librcd

#define RIO_THROW_TYPE_ERROR(rio_type_str) \
    throw("invalid rio type, expected " rio_type_str, exception_arg);

#define RIO_CHECK_TYPE(rio_h, rio_type) \
    if (rio_h->type != rio_type) \
        RIO_THROW_TYPE_ERROR(#rio_type);

struct rio_handle {
    rio_type_t type;
    union {
        // Generic file descriptor that support full duplex.
        struct {
            int32_t fd;
            bool is_readable;
            bool is_writable;
        } duplex;
        // Properties when (.type == rio_type_pipe).
        // Unlike a generic fd a pipe have different fds for each direction.
        struct {
            int32_t fd_read;
            int32_t fd_write;
        } pipe;
        // Properties when (.type == rio_type_abstract).
        // An implementation of read, write and poll that is not defined by rio.
        struct {
            const rio_class_t* impl;
            rcd_fid_t fid_arg;
            bool is_readable;
            bool is_writable;
        } abstract;
    } xfer;
    fstr_t peek_unconsumed;
    fstr_mem_t peek_buf;
};

struct rio_proc {
    /// Fiber that waits for the subprocess to exit and then sets exit_code.
    rcd_fid_t wait_fid;
    /// Race free process id, is has the same life time as the rio handle itself.
    /// The zombie process is garbage collected in the rio_proc destructor.
    int32_t pid;
    /// Set to the exit code of the program when it quits.
    int32_t exit_code;
};

struct rio_epoll {
    int32_t lt_fd;
    int32_t et_fd;
    uint32_t events;
};

const int32_t std_stream_numbers[] = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};

fstr_t rio_self_path = "/proc/self/exe";

int32_t rio_get_fd_read(rio_t* rio) {
    return (rio->type == rio_type_pipe)? rio->xfer.pipe.fd_read: (rio->type != rio_type_abstract && rio->xfer.duplex.is_readable? rio->xfer.duplex.fd: -1);
}

int32_t rio_get_fd_write(rio_t* rio) {
    return (rio->type == rio_type_pipe)? rio->xfer.pipe.fd_write: (rio->type != rio_type_abstract && rio->xfer.duplex.is_writable? rio->xfer.duplex.fd: -1);
}

static void rio_strict_close(int32_t fd) {
    int32_t close_r = close(fd);
    if (close_r == -1)
        RCD_SYSCALL_EXCEPTION(close, exception_fatal);
}

fiber_main rio_end_abstract_write(fiber_main_attr, const rio_class_t* impl, rcd_fid_t fid_arg) { try {
    impl->write_part_fn(fid_arg, "", false);
} catch (exception_io | exception_desync, e); }

static void rio_destruct_h(void* arg_ptr) {
    rio_t* rio = arg_ptr;
    if (rio->type == rio_type_pipe) {
        if (rio->xfer.pipe.fd_read != -1) {
            lwt_io_free_fd_tracking(rio->xfer.pipe.fd_read);
            rio_strict_close(rio->xfer.pipe.fd_read);
        }
        if (rio->xfer.pipe.fd_write != -1 && rio->xfer.pipe.fd_write != rio->xfer.pipe.fd_read) {
            lwt_io_free_fd_tracking(rio->xfer.pipe.fd_write);
            rio_strict_close(rio->xfer.pipe.fd_write);
        }
    } else if (rio->type == rio_type_abstract) {
        const rio_class_t* impl = rio->xfer.abstract.impl;
        if (rio->xfer.abstract.is_writable && impl->write_part_fn != 0 && impl->notify_wclose) {
            // Spawn end abstract write notify fiber.
            fmitosis {
                spawn_static_fiber(rio_end_abstract_write("", impl, rio->xfer.abstract.fid_arg));
            }
        }
    } else {
        if (rio->xfer.duplex.fd != -1) {
            lwt_io_free_fd_tracking(rio->xfer.duplex.fd);
            rio_strict_close(rio->xfer.duplex.fd);
        }
    }
#ifdef DEBUG
    memset(rio, 255, sizeof(*rio));
#endif
}

static rio_t* rio_alloc_new(rio_type_t type, bool is_readable, size_t peek_buf_min_len) {
    const size_t min_peek_buf_min_len = 0x40;
    size_t final_size;
    rio_t* rio = lwt_alloc_buffer_destructable(sizeof(rio_t) + (is_readable? MAX(peek_buf_min_len, min_peek_buf_min_len): 0), &final_size, rio_destruct_h);
    rio->type = type;
    rio->peek_unconsumed = (fstr_t) {0};
    rio->peek_buf.len = final_size - sizeof(rio_t);
    return rio;
}

rio_t* rio_new_h(rio_type_t type, int32_t fd, bool is_readable, bool is_writable, size_t peek_buf_min_len) {
    if (type == rio_type_pipe || type == rio_type_abstract)
        throw("unsupported function initialization type", exception_arg);
    rio_t* rio = rio_alloc_new(type, is_readable, peek_buf_min_len);
    rio->xfer.duplex.fd = fd;
    rio->xfer.duplex.is_readable = is_readable;
    rio->xfer.duplex.is_writable = is_writable;
    return rio;
}

rio_t* rio_new_abstract(const rio_class_t* impl, rcd_fid_t fid_arg, size_t peek_buf_min_len) {
    rio_t* rio = rio_alloc_new(rio_type_abstract, (impl->read_part_fn != 0), peek_buf_min_len);
    rio->xfer.abstract.impl = impl;
    rio->xfer.abstract.fid_arg = fid_arg;
    rio->xfer.abstract.is_readable = (impl->read_part_fn != 0);
    rio->xfer.abstract.is_writable = (impl->write_part_fn != 0);
    return rio;
}

rio_t* rio_new_pipe_h(int32_t fd_read, int32_t fd_write, size_t peek_buf_min_len) {
    rio_t* rio = rio_alloc_new(rio_type_pipe, (fd_read != -1), peek_buf_min_len);
    rio->xfer.pipe.fd_read = fd_read;
    rio->xfer.pipe.fd_write = fd_write;
    return rio;
}

fstr_mem_t* rio_serial_addr4(uint32_t ipv4_addr) { sub_heap {
    uint8_t a3, a2, a1, a0;
    RIO_IPV4_ADDR_UNPACK(ipv4_addr, a3, a2, a1, a0);
    return escape(conc(ui2fs(a3), ".", ui2fs(a2), ".", ui2fs(a1), ".", ui2fs(a0)));
}}

static void rio_unserial4_failed(fstr_t serial) {
    sub_heap_e(throw(concs("invalid ipv4 serial [", serial, "]"), exception_io));
}

static uint32_t rio_unserial_octs4(fstr_t c_4oct[], fstr_t ipv4_addr_serial) {
    uint8_t ipv4_octs[4];
    for (uint8_t i = 0; i < 4; i++) {
        uint128_t oct;
        if (!fstr_unserial_uint(c_4oct[i], 10, &oct))
            rio_unserial4_failed(ipv4_addr_serial);
        if (oct > 0xff)
            rio_unserial4_failed(ipv4_addr_serial);
        ipv4_octs[i] = oct;
    }
    return RIO_IPV4_ADDR_PACK(ipv4_octs[0], ipv4_octs[1], ipv4_octs[2], ipv4_octs[3]);
}

uint32_t rio_unserial_addr4(fstr_t ipv4_addr_serial) {
    fstr_t c_4oct[4];
    {
        fstr_t match_s = ipv4_addr_serial;
#pragma re2c(match_s): ^ (\d{1,3}){c_4oct[0]} \. (\d{1,3}){c_4oct[1]} \. (\d{1,3}){c_4oct[2]} \. (\d{1,3}){c_4oct[3]} $ {@match}
        rio_unserial4_failed(ipv4_addr_serial);
    } match:; {
        return rio_unserial_octs4(c_4oct, ipv4_addr_serial);
    }
}

fstr_mem_t* rio_serial_in_addr4(rio_in_addr4_t addr) { sub_heap {
    return escape(conc(fss(rio_serial_addr4(addr.address)), ":", ui2fs(addr.port)));
}}

rio_in_addr4_t rio_unserial_in_addr4(fstr_t in_addr4_serial) {
    fstr_t c_4oct[4];
    fstr_t c_port;
    {
        fstr_t match_s = in_addr4_serial;
#pragma re2c(match_s): ^ (\d{1,3}){c_4oct[0]} \. (\d{1,3}){c_4oct[1]} \. (\d{1,3}){c_4oct[2]} \. (\d{1,3}){c_4oct[3]} \: (\d{1,5}){c_port} $ {@match}
        rio_unserial4_failed(in_addr4_serial);
    } match:; {
        uint128_t port;
        if (!fstr_unserial_uint(c_port, 10, &port))
            rio_unserial4_failed(in_addr4_serial);
        return (rio_in_addr4_t) {
            .address = rio_unserial_octs4(c_4oct, in_addr4_serial),
            .port = port,
        };
    }
}

rio_type_t rio_get_type(rio_t* rio) {
    return rio->type;
}

bool rio_is_network(rio_t* rio) {
    return rio->type == rio_type_tcp || rio->type == rio_type_udp;
}

bool rio_is_stream(rio_t* rio) {
    return rio->type == rio_type_abstract || rio->type == rio_type_pipe || rio->type == rio_type_tcp || rio->type == rio_type_unix_stream;
}

static void rio_copy_peek_buffer(rio_t* rio_dst, rio_t* rio_src) {
    rio_dst->peek_unconsumed = fstr_cpy_over(fss(&rio_dst->peek_buf), rio_src->peek_unconsumed, 0, 0);
}

static void rio_disable_abstract_h(rio_t* rio) {
    assert(rio->type == rio_type_abstract);
    static const rio_class_t disabled_impl = {0};
    rio->xfer.abstract.impl = &disabled_impl;
    rio->xfer.abstract.fid_arg = 0;
    rio->xfer.abstract.is_readable = false;
    rio->xfer.abstract.is_writable = false;
}

rio_t* rio_realloc_peek_buffer(rio_t* rio, size_t peek_buf_min_len) {
    // Allocate new rio handle copy.
    peek_buf_min_len = MAX(rio->peek_unconsumed.len, peek_buf_min_len);
    rio_t* new_rio;
    if (rio->type == rio_type_pipe) {
        new_rio = rio_new_pipe_h(rio->xfer.pipe.fd_read, rio->xfer.pipe.fd_write, peek_buf_min_len);
    } else if (rio->type == rio_type_abstract) {
        new_rio = rio_new_abstract(rio->xfer.abstract.impl, rio->xfer.abstract.fid_arg, peek_buf_min_len);
    } else {
        new_rio = rio_new_h(rio->type, rio->xfer.duplex.fd, rio->xfer.duplex.is_readable, rio->xfer.duplex.is_writable, peek_buf_min_len);
    }
    // Copy over peek buffer.
    rio_copy_peek_buffer(new_rio, rio);
    // Disable old handle.
    if (rio->type == rio_type_pipe) {
        rio->xfer.pipe.fd_read = -1;
        rio->xfer.pipe.fd_write = -1;
    } else if (rio->type == rio_type_abstract) {
        rio_disable_abstract_h(rio);
    } else {
        rio->xfer.duplex.fd = -1;
        rio->xfer.duplex.is_readable = false;
        rio->xfer.duplex.is_writable = false;
    }
    // Return new buffer.
    return new_rio;
}

rio_t* rio_realloc(rio_t* rio) {
    return rio_realloc_peek_buffer(rio, rio->peek_buf.len);
}

bool rio_direct_write(int32_t write_fd, fstr_t data, int32_t* out_errno) {
    while (data.len > 0) {
        ssize_t n_write = write(write_fd, data.str, data.len);
        if (n_write > 0) {
            data = fstr_sslice(data, n_write, -1);
        } else if (n_write == -1) {
            if (errno == EWOULDBLOCK) {
                struct pollfd pfd = {.fd = write_fd, .events = POLLOUT};
                for (;;) {
                    int32_t poll_r = poll(&pfd, 1, -1);
                    if (poll_r >= 0)
                        break;
                    if (errno != EINTR) {
                        if (out_errno != 0)
                            *out_errno = errno;
                        return false;
                    }
                }
            } else if (errno == EINTR) {
                continue;
            } else {
                if (out_errno != 0)
                    *out_errno = errno;
                return false;
            }
        } else {
            if (out_errno != 0)
                *out_errno = EIO;
            return false;
        }
    }
    return true;
}

void rio_debug(fstr_t str) {
    rio_direct_write(STDERR_FILENO, str, 0);
}

void rio_debug_chunks(fstr_t* chunks, size_t n_chunks) {
    // This could be replaced with writev instead to allow the kernel to
    // provide the atomic properties of the write. This would allow the write
    // to be atomic even across concurrent processes. The benefit of this is
    // however negligible and it would require new write logic.
    static int8_t lock = 0;
    atomic_spinlock_lock(&lock); {
        for (size_t i = 0; i < n_chunks; i++)
            rio_direct_write(STDERR_FILENO, chunks[i], 0);
    } atomic_spinlock_unlock(&lock);
}

bool rio_file_exists(fstr_t file_path) {
    bool file_exists;
    sub_heap {
        int32_t access_r = access(fstr_to_cstr(file_path), F_OK);
        if (access_r == -1) {
            if (errno != ENOENT)
                RCD_SYSCALL_EXCEPTION(access, exception_io);
            file_exists = false;
        } else {
            file_exists = true;
        }
    }
    return file_exists;
}

fstr_mem_t* rio_get_cwd() {
    return rio_file_read_link("/proc/self/cwd");
}

static inline rio_stat_t rio_read_sys_stat(struct stat fs) {
    rio_stat_t rs;
    if (S_ISREG(fs.st_mode))
        rs.file_type = rio_file_type_regular;
    else if (S_ISDIR(fs.st_mode))
        rs.file_type = rio_file_type_directory;
    else if (S_ISCHR(fs.st_mode))
        rs.file_type = rio_file_type_character_device;
    else if (S_ISBLK(fs.st_mode))
        rs.file_type = rio_file_type_block_device;
    else if (S_ISFIFO(fs.st_mode))
        rs.file_type = rio_file_type_fifo;
    else if (S_ISLNK(fs.st_mode))
        rs.file_type = rio_file_type_symlink;
    else if (S_ISSOCK(fs.st_mode))
        rs.file_type = rio_file_type_socket;
    else
        rs.file_type = rio_file_type_unknown;
    rs.access_mode = (fs.st_mode & 0777);
    rs.user_id = fs.st_uid;
    rs.group_id = fs.st_gid;
    rs.size = fs.st_size;
    rs.time_accessed = fs.st_atim.tv_sec * RIO_NS_SEC + fs.st_atim.tv_nsec;
    rs.time_modified = fs.st_mtim.tv_sec * RIO_NS_SEC + fs.st_mtim.tv_nsec;
    rs.time_changed = fs.st_ctim.tv_sec * RIO_NS_SEC + fs.st_ctim.tv_nsec;
    rs.inode = fs.st_ino;
    rs.device = fs.st_dev;
    rs.rdevice_id = fs.st_rdev;
    rs.n_hard_links = fs.st_nlink;
    return rs;
}

rio_stat_t rio_file_stat(fstr_t file_path) { sub_heap {
    struct stat fs;
    int32_t stat_r = stat(fstr_to_cstr(file_path), &fs);
    if (stat_r < 0)
        RCD_SYSCALL_EXCEPTION(stat, exception_io);
    return rio_read_sys_stat(fs);
}}

rio_stat_t rio_file_lstat(fstr_t file_path) { sub_heap {
    struct stat fs;
    int32_t stat_r = lstat(fstr_to_cstr(file_path), &fs);
    if (stat_r < 0)
        RCD_SYSCALL_EXCEPTION(lstat, exception_io);
    return rio_read_sys_stat(fs);
}}

rio_stat_t rio_file_fstat(rio_t* file_h) { sub_heap {
    RIO_CHECK_TYPE(file_h, rio_type_file);
    struct stat fs;
    int32_t stat_r = fstat(rio_get_fd_read(file_h), &fs);
    if (stat_r < 0)
        RCD_SYSCALL_EXCEPTION(fstat, exception_io);
    return rio_read_sys_stat(fs);
}}

bool rio_file_access(fstr_t file_path, bool read, bool write, bool execute) {
    bool can_access;
    sub_heap {
        int32_t access_r = access(fstr_to_cstr(file_path), (read? R_OK: 0) | (write? W_OK: 0) | (execute? X_OK: 0));
        if (access_r == -1) {
            if (errno != EACCES && errno != ENOENT && errno != EROFS)
                RCD_SYSCALL_EXCEPTION(access, exception_io);
            can_access = false;
        } else {
            can_access = true;
        }
    }
    return can_access;
}

void rio_file_chmod(fstr_t file_path, uint32_t mode) {
    sub_heap {
        int32_t chmod_r = chmod(fstr_to_cstr(file_path), mode);
        if (chmod_r < 0)
            RCD_SYSCALL_EXCEPTION(chmod, exception_io);
    }
}

void rio_file_chown(fstr_t file_path, uint32_t uid, uint32_t gid) {
    sub_heap {
        int32_t chownd_r = chown(fstr_to_cstr(file_path), uid, gid);
        if (chownd_r < 0)
            RCD_SYSCALL_EXCEPTION(chown, exception_io);
    }
}

static inline struct timespec rio_file_chtime_get_timespec(uint128_t time_value) {
    struct timespec r_ts;
    if (time_value == RIO_FILE_CHTIME_NOW) {
        r_ts.tv_sec = 0;
        r_ts.tv_nsec = UTIME_NOW;
    } else if (time_value == RIO_FILE_CHTIME_OMIT) {
        r_ts.tv_sec = 0;
        r_ts.tv_nsec = UTIME_OMIT;
    } else {
        // tv_nsec cannot be UTIME_NOW or UTIME_OMIT in this case as these constants are larger than RIO_NS_SEC.
        r_ts.tv_sec = MAX(1, time_value / RIO_NS_SEC);
        r_ts.tv_nsec = time_value % RIO_NS_SEC;
    }
    return r_ts;
}

void rio_file_chtime(fstr_t file_path, uint128_t time_accessed, uint128_t time_modified) {
    sub_heap {
        struct timespec times[] = {
            rio_file_chtime_get_timespec(time_accessed),
            rio_file_chtime_get_timespec(time_modified),
        };
        int32_t utimensat_r = utimensat(0, fstr_to_cstr(file_path), times, 0);
        if (utimensat_r == -1)
            RCD_SYSCALL_EXCEPTION(utimensat, exception_io);
    }
}

void rio_file_mkreg(fstr_t file_path, uint32_t mode) {
    sub_heap {
        int32_t mknod_r = mknod(fstr_to_cstr(file_path), S_IFREG | (mode & 0777), 0);
        if (mknod_r == -1)
            RCD_SYSCALL_EXCEPTION(mknod, exception_io);
    }
}

void rio_file_mkdir(fstr_t file_path) {
    sub_heap {
        int32_t mkdir_r = mkdir(fstr_to_cstr(file_path), 0750);
        if (mkdir_r == -1)
            RCD_SYSCALL_EXCEPTION(mkdir, exception_io);
    }
}

void rio_file_mkchr(fstr_t file_path, uint32_t mode, uint64_t rdev_id) {
    sub_heap {
        int32_t mknod_r = mknod(fstr_to_cstr(file_path), S_IFCHR | (mode & 0777), rdev_id);
        if (mknod_r == -1)
            RCD_SYSCALL_EXCEPTION(mknod, exception_io);
    }
}

void rio_file_mkblk(fstr_t file_path, uint32_t mode, uint64_t rdev_id) {
    sub_heap {
        int32_t mknod_r = mknod(fstr_to_cstr(file_path), S_IFBLK | (mode & 0777), rdev_id);
        if (mknod_r == -1)
            RCD_SYSCALL_EXCEPTION(mknod, exception_io);
    }
}

void rio_file_mkfifo(fstr_t file_path, uint32_t mode) {
    sub_heap {
        int32_t mknod_r = mknod(fstr_to_cstr(file_path), S_IFIFO | (mode & 0777), 0);
        if (mknod_r == -1)
            RCD_SYSCALL_EXCEPTION(mknod, exception_io);
    }
}

void rio_file_mksock(fstr_t file_path, uint32_t mode) {
    sub_heap {
        int32_t mknod_r = mknod(fstr_to_cstr(file_path), S_IFSOCK | (mode & 0777), 0);
        if (mknod_r == -1)
            RCD_SYSCALL_EXCEPTION(mknod, exception_io);
    }
}

void rio_file_rename(fstr_t old_file_path, fstr_t new_file_path) {
    sub_heap {
        int32_t rename_r = rename(fstr_to_cstr(old_file_path), fstr_to_cstr(new_file_path));
        if (rename_r == -1)
            RCD_SYSCALL_EXCEPTION(rename, exception_io);
    }
}

void rio_file_rmdir(fstr_t file_path) {
    sub_heap {
        int32_t rmdir_r = rmdir(fstr_to_cstr(file_path));
        if (rmdir_r == -1)
            RCD_SYSCALL_EXCEPTION(rmdir, exception_io);
    }
}

void rio_file_link(fstr_t dst_path, fstr_t src_path) {
    sub_heap {
        int32_t link_r = link(fstr_to_cstr(dst_path), fstr_to_cstr(src_path));
        if (link_r == -1)
            RCD_SYSCALL_EXCEPTION(link, exception_io);
    }
}

void rio_file_unlink(fstr_t file_path) {
    sub_heap {
        int32_t unlink_r = unlink(fstr_to_cstr(file_path));
        if (unlink_r == -1)
            RCD_SYSCALL_EXCEPTION(unlink, exception_io);
    }
}

void rio_file_symlink(fstr_t dst_path, fstr_t src_path) {
    sub_heap {
        int32_t symlink_r = symlink(fstr_to_cstr(dst_path), fstr_to_cstr(src_path));
        if (symlink_r == -1)
            RCD_SYSCALL_EXCEPTION(symlink, exception_io);
    }
}

fstr_mem_t* rio_file_read_link(fstr_t file_path) { sub_heap {
    fstr_mem_t* buffer = fstr_alloc(PAGE_SIZE * 4);
    int32_t readlink_r = readlink(fstr_to_cstr(file_path), (char*) buffer->str, buffer->len);
    if (readlink_r == -1)
        RCD_SYSCALL_EXCEPTION(readlink, exception_io);
    return escape(fstr_cpy(fstr_slice(fss(buffer), 0, readlink_r)));
}}

fstr_mem_t* rio_file_get_path(rio_t* file_h) { sub_heap {
    return escape(rio_file_read_link(concs("/proc/self/fd/", i2fs(rio_get_fd_read(file_h)))));
}}

fstr_mem_t* rio_file_real_path(fstr_t file_path) { sub_heap {
    rio_t* file_h = rio_file_open(file_path, true, false);
    return escape(rio_file_get_path(file_h));
}}

rio_t* rio_file_open(fstr_t file_path, bool read_only, bool create) {
    int32_t fd;
    sub_heap {
        fd = open(fstr_to_cstr(file_path), (read_only? O_RDONLY: O_RDWR) | (create? O_CREAT: 0) | O_NONBLOCK | O_CLOEXEC, 0644);
        if (fd == -1)
            RCD_SYSCALL_EXCEPTION(open, exception_io);
    }
    return rio_new_h(rio_type_file, fd, true, read_only? false: true, 0);
}

void rio_file_lock(rio_t* file_h, bool exclusive) {
    RIO_CHECK_TYPE(file_h, rio_type_file);
    rio_file_lock_raw(file_h->xfer.duplex.fd, exclusive, false);
}

bool rio_file_try_lock(rio_t* file_h, bool exclusive) {
    RIO_CHECK_TYPE(file_h, rio_type_file);
    return rio_file_try_lock_raw(file_h->xfer.duplex.fd, exclusive);
}

void rio_file_unlock(rio_t* file_h) {
    RIO_CHECK_TYPE(file_h, rio_type_file);
    rio_file_unlock_raw(file_h->xfer.duplex.fd);
}

void rio_file_lock_raw(int32_t fd, bool exclusive, bool mutex) {
    int32_t operation = (exclusive? LOCK_EX: LOCK_SH);
    for (bool optimistic_nb_attempt = true;;) {
        int32_t flock_r;
        if (optimistic_nb_attempt) {
            // First attempting to do an optimistic normal non-blocking flock.
            flock_r = flock(fd, operation | (mutex? 0: LOCK_NB));
        } else {
            // Since linux does not provide an asynchronous facility that allows us to wait for the file to
            // unlock we use a lwt wrapper for the syscall that out sources the call to another thread.
            flock_r = lwt_flock(fd, operation);
        }
        if (flock_r == 0)
            return;
        int32_t errno_v = errno;
        if (errno_v == EINTR) {
            continue;
        } else if (!mutex && errno_v == EWOULDBLOCK) {
            optimistic_nb_attempt = false;
        } else {
            RCD_SYSCALL_EXCEPTION(flock, exception_io);
        }
    }
}

bool rio_file_try_lock_raw(int32_t fd, bool exclusive) {
    int32_t operation = (exclusive? LOCK_EX: LOCK_SH);
    for (;;) {
        int32_t flock_r = flock(fd, operation | LOCK_NB);
        if (flock_r == 0)
            return true;
        int32_t errno_v = errno;
        if (errno_v == EINTR) {
            continue;
        } else if (errno_v == EWOULDBLOCK) {
            return false;
        } else {
            RCD_SYSCALL_EXCEPTION(flock, exception_io);
        }
    }
}

void rio_file_unlock_raw(int32_t fd) {
    int32_t flock_r = flock(fd, LOCK_UN);
    if (flock_r == -1)
        RCD_SYSCALL_EXCEPTION(flock, exception_io);
}

void rio_file_fsync(rio_t* file_h) {
    RIO_CHECK_TYPE(file_h, rio_type_file);
    int32_t fd = file_h->xfer.duplex.fd;
    int32_t fsync_r = fsync(fd);
    if (fsync_r == -1)
        RCD_SYSCALL_EXCEPTION(fsync, exception_io);
}

list(fstr_mem_t*)* rio_file_list(fstr_t file_path) {
    list(fstr_mem_t*)* files;
    sub_heap_txn(heap) {
        // Open directory and wrap it in rio_h for cleanup.
        int32_t dir_fd = open(fstr_to_cstr(file_path), O_RDONLY | O_DIRECTORY | O_NONBLOCK | O_CLOEXEC, 0);
        if (dir_fd == -1)
            RCD_SYSCALL_EXCEPTION(open, exception_io);
        rio_t* dir_fd_h = rio_new_h(rio_type_file, dir_fd, false, false, 0);
        // Allocate buffer and start reading directories. Since paths can be quite long in linux, allocation is
        // extremely fast and to avoid doing lots of iterations we make sure to allocate a quite big buffer.
        const size_t dirp_buf_len = PAGE_SIZE * 8;
        void* dirp_buf = lwt_alloc_new(dirp_buf_len);
        switch_heap(heap) {
            files = new_list(fstr_mem_t*);
            for (;;) {
                int32_t getdents_r = getdents(dir_fd, dirp_buf, dirp_buf_len);
                if (getdents_r == -1) {
                    if (errno == EINTR)
                        continue;
                    RCD_SYSCALL_EXCEPTION(getdents, exception_io);
                }
                if (getdents_r == 0)
                    break;
                void* dirp_buf_end = dirp_buf + getdents_r;
                for (struct dirent* dirp = dirp_buf; ((void*) dirp) < dirp_buf_end; dirp = ((void*) dirp) + dirp->d_reclen) {
                    fstr_mem_t* file_name = fstr_from_cstr(dirp->d_name);
                    if (fstr_equal(fss(file_name), ".") || fstr_equal(fss(file_name), ".."))
                        continue;
                    list_push_end(files, fstr_mem_t*, file_name);
                }
            }
        }
    }
    return files;
}

int32_t rio_raw_fcntl_toggle_cloexec(int fd, bool enable) {
    int32_t fcntl_r = fcntl(fd, F_GETFD, 0);
    if (fcntl_r == -1)
        return -1;
    int32_t fd_flags = fcntl_r;
    int32_t fcntl_r2 = fcntl(fd, F_SETFD, (enable? fd_flags | O_CLOEXEC: fd_flags & ~O_CLOEXEC));
    if (fcntl_r2 == -1)
        return -1;
    return 0;
}

int32_t rio_raw_fcntl_toggle_nonblocking(int fd, bool enable) {
    int32_t fcntl_r = fcntl(fd, F_GETFL, 0);
    if (fcntl_r == -1)
        return -1;
    int32_t status_flags = fcntl_r;
    int32_t fcntl_r2 = fcntl(fd, F_SETFL, (enable? status_flags | O_NONBLOCK: status_flags & ~O_NONBLOCK));
    if (fcntl_r2 == -1)
        return -1;
    return 0;
}

int32_t rio_raw_dup(int32_t old_fd) {
    int32_t dup_fd;
    {
        // Need to do a dup() here with O_CLOEXEC - but flags are
        // only provided for dup2 (dup3). This is the UNIX facepalm workaround.
        int32_t fcntl_r = fcntl(old_fd, F_DUPFD_CLOEXEC, 0);
        if (fcntl_r == -1)
            RCD_SYSCALL_EXCEPTION(fcntl, exception_io);
        dup_fd = fcntl_r;
    }
    int fcntl_r = rio_raw_fcntl_toggle_nonblocking(dup_fd, true);
    if (fcntl_r == -1)
        RCD_SYSCALL_EXCEPTION(fcntl, exception_io);
    return dup_fd;
}

static rio_t* rio_dup_std_fd(int32_t old_fd, bool read) {
    int32_t dup_fd = rio_raw_dup(old_fd);
    return rio_new_pipe_h(read? dup_fd: -1, read? -1: dup_fd, 0);
}

// We implement standard streams by duplicating fds although this is
// theoretically not necessary. The duplicated file descriptors function is
// to allow separation of different fibers when more than one are writing
// to or reading from the standard streams at the same time to not break
// the scheduler assertion that more than one fiber are not using the same fd.

rio_t* rio_stdin() {
    return rio_dup_std_fd(STDIN_FILENO, true);
}

rio_t* rio_stdout() {
    return rio_dup_std_fd(STDOUT_FILENO, false);
}

rio_t* rio_stderr() {
    return rio_dup_std_fd(STDERR_FILENO, false);
}

rio_t* rio_open_pipe() {
    int32_t pipe_fds[2];
    int32_t pipe2_r = pipe2(pipe_fds, O_NONBLOCK | O_CLOEXEC);
    if (pipe2_r == -1)
        RCD_SYSCALL_EXCEPTION(pipe2, exception_io);
    return rio_new_pipe_h(pipe_fds[0], pipe_fds[1], 0);
}

void rio_pipe_close_end(rio_t* rio_pipe, bool read_end) {
    RIO_CHECK_TYPE(rio_pipe, rio_type_pipe);
    int32_t fd = read_end? rio_pipe->xfer.pipe.fd_read: rio_pipe->xfer.pipe.fd_write;
    if (fd == -1)
        return;
    rio_strict_close(fd);
    *(read_end? &rio_pipe->xfer.pipe.fd_read: &rio_pipe->xfer.pipe.fd_write) = -1;
}

void rio_realloc_split(rio_t* rio_combined, rio_t** out_rio_reader, rio_t** out_rio_writer) { sub_heap {
    rio_t *rio_reader, *rio_writer;
    if (rio_combined->type == rio_type_pipe) {
        if (rio_combined->xfer.pipe.fd_read == -1)
            throw("given pipe has no read end", exception_arg);
        if (rio_combined->xfer.pipe.fd_write == -1)
            throw("given pipe has no write end", exception_arg);
        // Pick out pipe read and write ends.
        int32_t fd_read = rio_combined->xfer.pipe.fd_read;
        int32_t fd_write = rio_combined->xfer.pipe.fd_write;
        rio_combined->xfer.pipe.fd_read = -1;
        rio_combined->xfer.pipe.fd_write = -1;
        rio_reader = rio_new_pipe_h(fd_read, -1, rio_combined->peek_buf.len);
        rio_writer = rio_new_pipe_h(-1, fd_write, 0);
    } else if (rio_combined->type == rio_type_abstract) {
        if (rio_combined->xfer.abstract.impl->read_part_fn == 0 || rio_combined->xfer.abstract.impl->write_part_fn == 0)
            throw("given rio handle does not support read/write", exception_arg);
        if (!rio_combined->xfer.abstract.is_readable)
            throw("given rio handle is not readable", exception_arg);
        if (!rio_combined->xfer.abstract.is_writable)
            throw("given rio handle is not writable", exception_arg);
        // Create two new handles that have uni-directional support only.
        rio_reader = rio_new_abstract(rio_combined->xfer.abstract.impl, rio_combined->xfer.abstract.fid_arg, rio_combined->peek_buf.len);
        rio_reader->xfer.abstract.is_writable = false;
        rio_writer = rio_new_abstract(rio_combined->xfer.abstract.impl, rio_combined->xfer.abstract.fid_arg, 0);
        rio_writer->xfer.abstract.is_readable = false;
        // Disable the abstract handle.
        rio_disable_abstract_h(rio_combined);
    } else {
        if (rio_combined->xfer.duplex.fd == -1)
            throw("given rio handle does not support read/write", exception_arg);
        if (!rio_combined->xfer.duplex.is_readable)
            throw("given rio handle is not readable", exception_arg);
        if (!rio_combined->xfer.duplex.is_writable)
            throw("given rio handle is not writable", exception_arg);
        // Disable the combined handle and split it into a read and write only handle.
        rio_type_t rio_type = rio_combined->type;
        int32_t duplex_fd = rio_combined->xfer.duplex.fd;
        rio_combined->xfer.duplex.fd = -1;
        rio_combined->xfer.duplex.is_readable = false;
        rio_combined->xfer.duplex.is_writable = false;
        rio_reader = rio_new_h(rio_type, duplex_fd, true, false, rio_combined->peek_buf.len);
        // The dup we do here could cause an io exception so we do it last.
        rio_writer = rio_new_h(rio_type, rio_raw_dup(duplex_fd), false, true, 0);
    }
    // Preserve the unconsumed peek buffer.
    if (rio_combined->peek_buf.len > 0)
        rio_copy_peek_buffer(rio_reader, rio_combined);
    // Return split ends.
    *out_rio_reader = escape(rio_reader);
    *out_rio_writer = escape(rio_writer);
}}

rio_t* rio_realloc_combined(rio_t* read_pipe, rio_t* write_pipe) {
    RIO_CHECK_TYPE(read_pipe, rio_type_pipe);
    RIO_CHECK_TYPE(write_pipe, rio_type_pipe);
    if (read_pipe->xfer.pipe.fd_read == -1)
        throw("given read pipe has no read end", exception_arg);
    if (write_pipe->xfer.pipe.fd_write == -1)
        throw("given write pipe has no write end", exception_arg);
    rio_pipe_close_end(read_pipe, false);
    rio_pipe_close_end(write_pipe, true);
    rio_t* combined_pipe = rio_new_pipe_h(read_pipe->xfer.pipe.fd_read, write_pipe->xfer.pipe.fd_write, 0);
    read_pipe->xfer.pipe.fd_read = -1;
    write_pipe->xfer.pipe.fd_write = -1;
    return combined_pipe;
}

rio_t* rio_open_dev_null() {
    int32_t open_r = open(fstr_to_cstr("/dev/null"), O_RDWR | O_NONBLOCK | O_CLOEXEC, 0);
    if (open_r == -1)
        RCD_SYSCALL_EXCEPTION(open, exception_fatal);
    return rio_new_h(rio_type_file, open_r, true, true, 0);
}

ssize_t rio_get_file_offset(rio_t* rio) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    off_t lseek_r = lseek(rio->xfer.duplex.fd, 0, SEEK_CUR);
    if (lseek_r == -1)
        RCD_SYSCALL_EXCEPTION(lseek, exception_io);
    return lseek_r;
}

void rio_set_file_offset(rio_t* rio, ssize_t offs, bool relative) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    off_t lseek_r = lseek(rio->xfer.duplex.fd, offs, relative? SEEK_CUR: SEEK_SET);
    if (lseek_r == -1)
        RCD_SYSCALL_EXCEPTION(lseek, exception_io);
}

void rio_set_file_offset_end(rio_t* rio, ssize_t offs) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    off_t lseek_r = lseek(rio->xfer.duplex.fd, offs, SEEK_END);
    if (lseek_r == -1)
        RCD_SYSCALL_EXCEPTION(lseek, exception_io);
}

bool rio_is_eof(rio_t* rio) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    // Check if the current offset equals the file size and thus is at end of file.
    ssize_t offset = rio_get_file_offset(rio);
    rio_set_file_offset_end(rio, 0);
    ssize_t file_size = rio_get_file_offset(rio);
    bool is_eof = (offset >= file_size);
    // Restore offset if it changed.
    if (offset != file_size)
        rio_set_file_offset(rio, offset, false);
    return is_eof;
}

void rio_file_truncate(rio_t* rio, size_t new_length) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    int32_t ftruncate_r = ftruncate(rio_get_fd_write(rio), new_length);
    if (ftruncate_r == -1)
        RCD_SYSCALL_EXCEPTION(ftruncate, exception_io);
}

size_t rio_get_file_size(rio_t* rio) {
    RIO_CHECK_TYPE(rio, rio_type_file);
    ssize_t offset = rio_get_file_offset(rio);
    off_t lseek_r = lseek(rio->xfer.duplex.fd, 0, SEEK_END);
    if (lseek_r == -1)
        RCD_SYSCALL_EXCEPTION(lseek, exception_io);
    size_t file_size = (size_t) lseek_r;
    // Restore offset and return file size.
    rio_set_file_offset(rio, offset, false);
    return file_size;
}

fstr_mem_t* rio_read_file_contents(fstr_t file_path) { sub_heap {
    rio_t* rio = rio_file_open(file_path, true, false);
    size_t file_size = rio_get_file_size(rio);
    fstr_mem_t* buffer = fstr_alloc(file_size);
    fstr_t buffer_tail = fss(buffer);
    buffer_tail.len = MIN(buffer_tail.len, file_size);
    while (buffer_tail.len > 0) {
        fstr_t chunk = rio_read(rio, buffer_tail);
        assert(chunk.len > 0);
        buffer_tail = fstr_sslice(buffer_tail, (ssize_t) chunk.len, -1);
    }
    buffer->len -= buffer_tail.len;
    return escape(buffer);
}}

fstr_t rio_read_virtual_file_contents(fstr_t file_path, fstr_t buffer) {
    sub_heap {
        // Virtual files are impossible to seek or understand the size of. They
        // are supposed to be read until EOF is reached, however EOF is an
        // error condition in rio so we just call read once and expect the
        // buffer to be filled.
        rio_t* rio = rio_file_open(file_path, true, false);
        fstr_t buffer_tail = buffer;
        while (buffer_tail.len > 0) {
            ssize_t n_read = read(rio->xfer.duplex.fd, buffer_tail.str, buffer_tail.len);
            if (n_read == -1)
                RCD_SYSCALL_EXCEPTION(read, exception_io);
            buffer_tail = fstr_sslice(buffer_tail, n_read, -1);
            if (n_read == 0)
                break;
        }
        buffer = fstr_sslice(buffer, 0, -buffer_tail.len - 1);
    }
    return buffer;
}

rio_t* rio_signal_chan_open(size_t n_signals, int32_t* signals) {
    sigset_t set;
    sigemptyset(&set);
    for (size_t i = 0; i < n_signals; i++)
        sigaddset(&set, signals[i]);
    int32_t signalfd4_r = signalfd4(-1, &set, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signalfd4_r == -1)
        RCD_SYSCALL_EXCEPTION(signalfd4, exception_io);
    return rio_new_h(rio_type_signal, signalfd4_r, true, false, 0);
}

static void rio_reset_sigprocmask() {
    {
        // Unblock all signals as this mask is inherited.
        sigset_t sigset;
        sigemptyset(&sigset);
        int32_t sigprocmask_r = sigprocmask(SIG_SETMASK, &sigset, 0);
        if (sigprocmask_r == -1)
            RCD_SYSCALL_EXCEPTION(sigprocmask, exception_io);
    }{
        // Reset all signal disposition handlers as this mask is inherited.
        struct k_sigaction sa = {.handler = SIG_DFL};
        for (int32_t signo = SIGHUP; signo <= SIGSYS; signo++) {
            if (signo == SIGKILL || signo == SIGSTOP)
                continue;
            int r_rt_sigaction = rt_sigaction(signo, &sa, 0);
            if (r_rt_sigaction == -1)
                RCD_SYSCALL_EXCEPTION(rt_sigaction, exception_fatal);
        }
    }
}

static char** rio_compile_argv(fstr_t* path, list(fstr_t)* args) {
    char** argv = lwt_alloc_new((list_count(args, fstr_t) + 2) * sizeof(char*));
    int32_t i = 0;
    if (path != 0) {
        argv[0] = fstr_to_cstr(*path);
        i++;
    }
    list_foreach(args, fstr_t, arg) {
        argv[i] = fstr_to_cstr(arg);
        i++;
    }
    argv[i] = 0;
    return argv;
}

uint32_t rio_process_getuid() {
    return getuid();
}

uint32_t rio_process_getgid() {
    return getgid();
}

void rio_process_setuid(uint32_t uid) {
    int32_t setuid_r = setuid(uid);
    if (setuid_r == -1)
        RCD_SYSCALL_EXCEPTION(setuid, exception_io);
}

void rio_process_setgid(uint32_t gid) {
    int32_t setgid_r = setgid(gid);
    if (setgid_r == -1)
        RCD_SYSCALL_EXCEPTION(setgid, exception_io);
}

void rio_process_setid(rio_id_t id) {
    if (id.gid != 0)
        rio_process_setgid(id.gid);
    if (id.uid != 0)
        rio_process_setuid(id.uid);
}

/// This function irreversibly changes standard streams and resets signal mask and so represents a point of no return for the process.
static void rio_pre_execve(int32_t stdin_fd, int32_t stdout_fd, int32_t stderr_fd, rio_id_t set_id) {
    // Dup-over standard streams, also disable CLOEXEC flags on standard file
    // descriptors to ensure that only they survive the exec. All other file
    // descriptors should be open with O_CLOEXEC and if they leak the bug is
    // not here but in the routine that opens the descriptor.
    int32_t std_stream_replacements[] = {stdin_fd, stdout_fd, stderr_fd};
    for (int i = 0; i < LENGTHOF(std_stream_numbers); i++) {
        int32_t src_fd = std_stream_replacements[i];
        int32_t std_fd = std_stream_numbers[i];
        if (src_fd == -1)
            continue;
        if (src_fd == std_fd) {
            // Allow standard file descriptor to leak to subprocess.
            rio_raw_fcntl_toggle_cloexec(std_fd, false);
        } else {
            // dup2 has O_CLOEXEC disabled for the new file descriptor as it should be.
            // Note that we must use syscall() directly as the wrapper for dup2() automatically creates the new fd with CLOEXEC.
            // This is pretty much the only place where we want to allow creating file descriptors without CLOEXEC so using raw syscall() here is legitimate.
            int32_t dup2_r = (int32_t) syscall(SYS_dup2, src_fd, std_fd);
            if (dup2_r == -1)
                RCD_SYSCALL_EXCEPTION(dup2, exception_io);
            // Make sure nonblocking is disabled, dup2 does not disable this flag if set.
            rio_raw_fcntl_toggle_nonblocking(std_fd, false);
        }
    }
    // Prevent signal procmask state from leaking into subprocess.
    rio_reset_sigprocmask();
    // Set new unix user id.
    rio_process_setid(set_id);
}

void rio_process_execve(rio_exec_t e) {
    // If there is contention on calling this function we simply avoid multiple threads messing with the executable state.
    static int8_t lock = 0;
    atomic_spinlock_lock(&lock);
    // Make irreversible changes to the process state.
    int32_t stdin_fd = e.io_in != 0? rio_get_fd_read(e.io_in): STDIN_FILENO;
    int32_t stdout_fd = e.io_out != 0? rio_get_fd_write(e.io_out): STDOUT_FILENO;
    int32_t stderr_fd = e.io_err != 0? rio_get_fd_write(e.io_err): STDERR_FILENO;
    rio_pre_execve(stdin_fd, stdout_fd, stderr_fd, e.set_id);
    execve(fstr_to_cstr(e.path), rio_compile_argv(&e.path, e.args), 0);
    // If the execve fails we throw a fatal exception since we have already called rio_pre_execve() so the process is no longer in a catchable state.
    RCD_SYSCALL_EXCEPTION(execve, exception_fatal);
}

typedef struct rio_post_clone_args {
    char* path;
    char** argv;
    char** env;
    int32_t stdin_fd;
    int32_t stdout_fd;
    int32_t stderr_fd;
    rio_id_t set_id;
    int32_t exit_pipe_fd;
    volatile int32_t execve_errno;
} rio_post_clone_args_t;

static void rio_execute_post_clone(void* arg_ptr) {
    // After the clone we're in a temporary really primitive context where we can
    // basically do no librcd magic at all - just plain ansi c and syscalls.
    // We don't have a heap, however we must have errno so setup a primitive context for this.
    //-x-execute/ DBG("[rio_execute_post_clone]: called");
    rio_post_clone_args_t* args = arg_ptr;
    lwt_setup_archaic_physical_thread();
    // This is a subprocess so we'd like the child to exit asap if the parent does.
    int32_t prctl_r = prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
    if (prctl_r < 0)
        RCD_SYSCALL_EXCEPTION(prctl, exception_io);
    // Prevent signal procmask state from leaking into subprocess.
    rio_pre_execve(args->stdin_fd, args->stdout_fd, args->stderr_fd, args->set_id);
    //-x-execute/ DBG("[rio_execute_post_clone]: calling execve(", fss(fstr_from_cstr(args->path)), ")");
    int32_t execve_r = execve(args->path, args->argv, args->env);
    //-x-execute/ DBG("[rio_execute_post_clone]: execve failed");
    // Execve failed - return errno. Because the memory is shared we can
    // communicate this in the shared errno integer. We also share the errno
    args->execve_errno = errno;
    exit_group(execve_r);
    unreachable();
}

fiber_main rio_proc_wait_fiber(fiber_main_attr, rio_proc_t* proc_h) {
    // Note that lwt_waitpid can throw an io exception if waitid fails, but the kernel promises that
    // waitid should work regardless of execve or the behavior of the subprocess so we just crash the
    // program by leaking the exception as we cannot possibly know what's broken and deal with it.
    proc_h->exit_code = lwt_waitpid(proc_h->pid);
}

static void rio_proc_destruct(void* arg_ptr) { uninterruptible {
    rio_proc_t* proc_h = arg_ptr;
    // Forcefully clean up the subprocess.
    int32_t kill_r = kill(proc_h->pid, SIGKILL);
    if (kill_r == -1)
        RCD_SYSCALL_EXCEPTION(kill, exception_fatal);
    // Wait for the process to exit which is equivalent to waiting for the
    // avatar fiber to exit that we are responsible for cleaning up anyway.
    // The SIGKILL ensures that we are only waiting for kernel CPU time here.
    // (Note: proc_h->wait_fid could be 0 here if there was an execve error, which is fine.)
    ifc_wait(proc_h->wait_fid);
    // This should always succeed. If something goes wrong it's just as bad as close() or munmap() not working and we throw a fatal.
    for (;;) {
        int32_t wait4_r = wait4(proc_h->pid, 0, 0, 0);
        if (wait4_r >= 0)
            break;
        if (errno != EINTR)
            RCD_SYSCALL_EXCEPTION(wait4, exception_fatal);
    }
}}

rio_proc_t* rio_proc_execute(rio_sub_exec_t se) { sub_heap {
    // Create proc handle for subprocess.
    rio_proc_t* proc_h = lwt_alloc_destructable(sizeof(rio_proc_t), rio_proc_destruct);
    *proc_h = (rio_proc_t) {0};
    // Enter trampoline memory that we only need briefly in the pre execve phase.
    sub_heap {
        // Because we fork the file descriptor table after the clone we don't have a race between closing the file
        // descriptors we are given as standard in/out/err after returning and the execve() we're not waiting for.
        int32_t stdin_fd = se.exec.io_in != 0? rio_get_fd_read(se.exec.io_in): STDIN_FILENO;
        int32_t stdout_fd = se.exec.io_out != 0? rio_get_fd_write(se.exec.io_out): STDOUT_FILENO;
        int32_t stderr_fd = se.exec.io_err != 0? rio_get_fd_write(se.exec.io_err): STDERR_FILENO;
        // Allocate some memory for the stack.
        size_t fast_execve_stack_size = PAGE_SIZE * 0x20;
        void* clone_stack = lwt_alloc_new(fast_execve_stack_size);
        void* aligned_stack_entry = (void*) vm_align_floor((uintptr_t) clone_stack + fast_execve_stack_size, 16) - 8;
        // Process all arguments into their raw c form allowing the new process to do as little work as possible before calling execve.
        rio_post_clone_args_t* pc_args = new(rio_post_clone_args_t);
        pc_args->path = fstr_to_cstr(se.exec.path);
        pc_args->argv = rio_compile_argv(&se.exec.path, se.exec.args);
        pc_args->env = se.exec.env != 0? rio_compile_argv(0, se.exec.env): 0;
        pc_args->stdin_fd = stdin_fd;
        pc_args->stdout_fd = stdout_fd;
        pc_args->stderr_fd = stderr_fd;
        pc_args->set_id = se.exec.set_id;
        pc_args->execve_errno = 0;
        // Create a pipe which has the only purpose of allowing us to waiting for
        // the forked child process to no longer be a thread in the current process image.
        rio_t* pre_wait_pipe = rio_open_pipe();
        // Create the new process which share our memory so we can clone quickly. This is the fast execve method.
        int32_t flags = CLONE_VM | SIGCHLD
        | (se.new_kernel_ns? CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWNET: 0)
        | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
        struct lwt_physical_thread* phys_thread = 0;
        int32_t clone_r = lwt_start_new_thread(rio_execute_post_clone, aligned_stack_entry, flags, pc_args, &phys_thread);
        if (clone_r == -1)
            RCD_SYSCALL_EXCEPTION(clone, exception_io);
        proc_h->pid = clone_r;
        assert(proc_h->pid > 0);
        // Wait for the pre-execve phase to end so we can free the trampoline memory.
        // We do this by enabling blocking mode and reading one byte.
        rio_pipe_close_end(pre_wait_pipe, false);
        int32_t read_fd = pre_wait_pipe->xfer.pipe.fd_read;
        int32_t fcntl_r = rio_raw_fcntl_toggle_nonblocking(read_fd, false);
        if (fcntl_r == -1)
            RCD_SYSCALL_EXCEPTION(fcntl, exception_io);
        // Read should indicate EOF by returning 0 (see man 7 pipe) since the write end of the pipe is never used.
        // If we get an unexpected value from read here we must crash as we are no longer in a
        // position where we can say that the memory is safe to free or will eventually be so.
        for (uint8_t buf;;) {
            int32_t read_r = read(read_fd, &buf, 1);
            if (read_r == 0)
                break;
            if (read_r == -1) {
                if (errno == EINTR)
                    continue;
                RCD_SYSCALL_EXCEPTION(read, exception_fatal);
            }
            throw(concs("read() failed: returned unexpected value [", ui2fs(read_r), "]"), exception_fatal);
        }
        // Forward errno from execve as a proper exception. It is safe to check it after the wait pipe returned eof.
        if (pc_args->execve_errno != 0) {
            errno = pc_args->execve_errno;
            RCD_SYSCALL_EXCEPTION(execve, exception_io);
        }
    }
    // Create fiber that waits for the subprocess to exit.
    // This allows us to transform the act of waiting for the subprocess into the act of waiting
    // for a fiber, which enables performing interprocess concurrency as librcd concurrency.
    fmitosis {
        fstr_t fiber_name = sconc(se.exec.path, list_count(se.exec.args, fstr_t) > 0? " ": "", fss(fstr_implode(se.exec.args, " ")));
        proc_h->wait_fid = spawn_static_fiber(rio_proc_wait_fiber(fiber_name, proc_h));
    }
    return escape(proc_h);
}}

int32_t rio_proc_get_pid(rio_proc_t* proc_h) {
    return proc_h->pid;
}

int32_t rio_proc_wait(rio_proc_t* proc_h) {
    ifc_wait(proc_h->wait_fid);
    return proc_h->exit_code;
}

void rio_proc_signal(rio_proc_t* proc_h, int32_t posix_signal) {
    int32_t kill_r = kill(proc_h->pid, posix_signal);
    if (kill_r == -1)
        RCD_SYSCALL_EXCEPTION(kill, exception_io);
}

int32_t rio_proc_execute_and_wait(fstr_t path, list(fstr_t)* args, bool keep_stderr) {
    int32_t exit_code;
    sub_heap {
        rio_t* dev_null = rio_open_dev_null();
        rio_sub_exec_t se = {
            .exec = {
                .path = path,
                .args = args,
                .io_in = dev_null,
                .io_out = dev_null,
                .io_err = (keep_stderr? 0: dev_null),
            },
        };
        rio_proc_t* proc_h = rio_proc_execute(se);
        lwt_alloc_free(dev_null);
        exit_code = rio_proc_wait(proc_h);
    }
    return exit_code;
}

void rio_exec(fstr_t unix_name, list(fstr_t)* args) {
    sub_heap {
        fstr_t path = fss(rio_which(unix_name));
        int32_t exit_code = rio_proc_execute_and_wait(path, args, true);
        if (exit_code != 0)
            throw(fss(conc("executing the subprocess [", unix_name ,"] failed, returned non zero exit code [", fss(fstr_from_int(exit_code, 10)), "]")), exception_io);
    }
}

void rio_shell(fstr_t command) {
    sub_heap {
        rio_exec("bash", new_list(fstr_t, "-c", command));
    }
}

void rio_proc_execute_and_pipe(fstr_t path, list(fstr_t)* args, bool keep_stderr, rio_proc_t** out_proc_h, rio_t** out_rio_pipe) { sub_heap {
    rio_t* stdin_pipe = rio_open_pipe();
    rio_t* stdout_pipe = rio_open_pipe();
        rio_sub_exec_t se = {
            .exec = {
                .path = path,
                .args = args,
                .io_in = stdin_pipe,
                .io_out = stdout_pipe,
                .io_err = (keep_stderr? 0: rio_open_dev_null()),
            },
        };
    rio_proc_t* proc_h = rio_proc_execute(se);
    // We combine the stdin_fd pipe and stdout_fd pipe so we read from the childs stdout_fd and write to the childs stdin_fd.
    rio_t* rio_pipe = rio_realloc_combined(stdout_pipe, stdin_pipe);
    *out_proc_h = escape(proc_h);
    *out_rio_pipe = escape(rio_pipe);
}}

fstr_mem_t* rio_which(fstr_t unix_name) {
    fstr_t unix_exec_bin_paths[] = {
        "/usr/local/sbin/",
        "/usr/local/bin/",
        "/usr/sbin/",
        "/usr/bin/",
        "/sbin/",
        "/bin/",
    };
    // The following code is hard to read because of the imperative looping
    // mixed with the functional lambda style memory blocks and null
    // termination enforced by access().
    fstr_mem_t* full_path;
    for (size_t i = 0;; i++) {
        sub_heap {
            if (i >= LENGTHOF(unix_exec_bin_paths))
                throw(fss(conc("failed to find the binary [", unix_name ,"] in the local environment")), exception_io);
            fstr_mem_t* test_path = conc(unix_exec_bin_paths[i], unix_name, "\0");
            if (access((char*) test_path->str, X_OK) == 0) {
                full_path = escape(test_path);
                goto found_full_path;
            }
        }
    }
    found_full_path:;
    // Remove null terminator used when passing fixed string to access (2).
    full_path->len--;
    return full_path;
}

void rio_write_file_contents(fstr_t file_path, fstr_t data) {
    sub_heap {
        rio_t* rio = rio_file_open(file_path, false, true);
        rio_file_truncate(rio, 0);
        rio_write(rio, data);
    }
}

static fstr_t rio_read_direct(rio_t* rio, fstr_t buffer, bool* out_more_hint) {
    if (rio->type == rio_type_abstract) {
        if (rio->xfer.abstract.impl->read_part_fn == 0 || !rio->xfer.abstract.is_readable)
            throw("the specified rio handle does not support the operation read", exception_arg);
        return rio->xfer.abstract.impl->read_part_fn(rio->xfer.abstract.fid_arg, buffer, out_more_hint);
    }
    ssize_t n_read;
    int32_t read_fd = rio_get_fd_read(rio);
    if (read_fd == -1)
        throw("the specified rio handle does not support the operation read", exception_arg);
    for (;;) {
        n_read = read(read_fd, buffer.str, buffer.len);
        if (n_read == 0)
            throw_eio("read() failed: end of stream reached", rio_eos);
        if (n_read > 0)
            break;
        if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(read_fd, lwt_fd_event_read);
        else if (errno == EPIPE)
            throw_eio("read() failed: end of pipe stream reached", rio_eos);
        else if (errno != EINTR)
            RCD_SYSCALL_EXCEPTION(read, exception_io);
    }
    fstr_t slice = fstr_slice(buffer, 0, n_read);
    if (out_more_hint != 0) {
        // There may be ways to figure out the size of the kernel buffer in the underlying file descriptor
        // however, since this is a performance hint we are not able to use those methods as we may end up losing performance from syscall overhead.
        *out_more_hint = false;
    }
    return slice;
}

fstr_t rio_read(rio_t* rio, fstr_t buffer) {
    return rio_read_part(rio, buffer, 0);
}

fstr_t rio_read_part(rio_t* rio, fstr_t buffer, bool* out_more_hint) {
    if (buffer.len == 0)
        return fstr_slice(buffer, 0, 0);
    fstr_t r_buffer;
    if (rio->peek_unconsumed.len == 0) {
        // Optimization heuristics: we use the peek buffer as long as it as least twice the size of the passed buffer.
        // This is memory we have already allocated and it's always better to read as large chunks to the user space as possible.
        if (rio->peek_buf.len > buffer.len * 2) {
            rio->peek_unconsumed = rio_read_direct(rio, fss(&rio->peek_buf), 0);
            goto use_peek_buffer;
        } else {
            r_buffer = rio_read_direct(rio, buffer, out_more_hint);
        }
    } else {
        use_peek_buffer:;
        r_buffer = fstr_cpy_over(buffer, rio->peek_unconsumed, 0, &rio->peek_unconsumed);
        if (out_more_hint != 0)
            *out_more_hint = (rio->peek_unconsumed.len > 0);
    }
    return r_buffer;
}

void rio_read_fill(rio_t* rio, fstr_t buffer) {
    fstr_t tail_left = buffer;
    while (tail_left.len > 0) {
        fstr_t head_done = rio_read(rio, tail_left);
        tail_left = fstr_slice(tail_left, head_done.len, tail_left.len);
    }
}

fstr_t rio_read_to_end(rio_t* rio, fstr_t buffer) {
    // If we get an exception we want to preserve the state of tail_left so we declare it as volatile.
    volatile fstr_t tail_left = buffer;
    try {
        while (tail_left.len > 0) {
            fstr_t head_done = rio_read(rio, tail_left);
            tail_left = fstr_slice(tail_left, head_done.len, tail_left.len);
        }
    } catch_eio (rio_eos, e);
    return fstr_sslice(buffer, 0, -tail_left.len - 1);
}

fstr_t rio_read_all(rio_t* rio, vstr_t* buffer) {
    size_t vbuf_slen = vec_count(buffer, uint8_t);
    try {
        for (;;) {
            // Attempt to read a chunk that is 25% as large as the buffer.
            // We set a minimum of 64 bytes and up to 128 pages of data.
            size_t blen = vec_count(buffer, uint8_t);
            size_t max_chunk_len = MIN(MAX(64, blen / 4), 128 * PAGE_SIZE);
            fstr_t dst = vstr_extend(buffer, max_chunk_len);
            fstr_t chunk = "";
            try {
                chunk = rio_read(rio, dst);
            } finally {
                // Shrink the buffer to get rid of the unwritten tail.
                vec_resize(buffer, uint8_t, blen + chunk.len);
            }
        }
    } catch_eio (rio_eos, e);
    return fstr_slice(vstr_str(buffer), vbuf_slen, -1);
}

join_locked(fstr_t) read_to_end_tout_result(join_server_params, rcd_exception_t* ex, fstr_t result) {
    if (ex != 0)
        lwt_throw_exception(ex);
    return result;
}

fiber_main read_to_end_tout_fiber(fiber_main_attr, rio_t* rio, fstr_t buffer, uint128_t timeout_ns) { try {
    // Cancel fiber after this time.
    ifc_cancel_alarm_arm(timeout_ns);
    // Read to end.
    rcd_exception_t* ex = 0;
    fstr_t result;
    try {
        result = rio_read_to_end(rio, buffer);
    } catch (exception_io, e) {
        ex = e;
    }
    // Return result.
    accept_join(read_to_end_tout_result, join_server_params, ex, result);
} catch (exception_desync, e); }

fstr_t rio_read_to_end_timeout(rio_t* rio, fstr_t buffer, uint128_t timeout_ns) { sub_heap {
    rcd_sub_fiber_t* sf;
    fmitosis {
        sf = spawn_fiber(read_to_end_tout_fiber("", rio_realloc(rio), buffer, timeout_ns));
    }
    try {
        return read_to_end_tout_result(sfid(sf));
    } catch (exception_inner_join_fail, e) {
        throw_eio_fwd("timed out while reading to end of stream", rio_tout, e);
    }
}}

void rio_skip(rio_t* rio, size_t length) {
    if (length == 0)
        return;
    if (rio->peek_unconsumed.len < length) {
        // TODO
        throw("skipping bytes read directly from byte stream is not implemented yet - length must be lower than the last peeked buffer", exception_io);
    }
    rio->peek_unconsumed = fstr_sslice(rio->peek_unconsumed, (ssize_t) length, -1);
}

bool rio_poll(rio_t* rio, bool read, bool wait) {
    if (read && rio->peek_unconsumed.len > 0)
        return true;
    if (rio->type == rio_type_abstract) {
        // Forward poll to abstract rio implementation.
        if (rio->xfer.abstract.impl->poll_fn == 0)
            throw("the specified rio handle does not support the operation poll", exception_arg);
        if ((read && !rio->xfer.abstract.is_readable) || (!read && !rio->xfer.abstract.is_writable))
            throw("the specified rio handle does not support the polled operation", exception_arg);
        return rio->xfer.abstract.impl->poll_fn(rio->xfer.abstract.fid_arg, read, wait);
    }
    return rio_poll_raw(read? rio_get_fd_read(rio): rio_get_fd_write(rio), read, wait);
}

bool rio_poll_raw(int32_t target_fd, bool read, bool wait) {
    // Check if we have data in the buffer right now with a single poll first.
    struct pollfd fds[] = {{.fd = target_fd, .events = (read? POLLIN: POLLOUT)}};
    for (;;) {
        int32_t poll_r = poll(fds, LENGTHOF(fds), 0);
        if (poll_r == 0)
            break;
        else if (poll_r > 0)
            return true;
        else if (errno == EINTR)
            continue;
        else
            RCD_SYSCALL_EXCEPTION(poll, exception_io);
    }
    // If we shouldn't be waiting we return false here, polling is complete.
    if (!wait)
        return false;
    // We need to do a long wait for external I/O. Since we're context
    // switching anyway the overhead from doing these multiple syscalls should
    // not increase the already existing overhead magnitude.
    sub_heap {
        rio_epoll_t* epoll_h = rio_epoll_create_raw(target_fd, read? rio_epoll_event_inlvl: rio_epoll_event_outlvl);
        rio_epoll_poll(epoll_h, true);
    }
    // Waiting is complete, the rio handle should be ready
    // for the requested event now.
    return true;
}

fstr_t rio_peek(rio_t* rio) {
    if (rio->peek_buf.len == 0)
        throw("cannot peek, no peek buffer allocated for rio handle", exception_io);
    if (rio->peek_unconsumed.len == 0)
        rio->peek_unconsumed = rio_read_direct(rio, fss(&rio->peek_buf), 0);
    return rio->peek_unconsumed;
}

static inline fstr_t read_to_separator(rio_t* rio, fstr_t separator, bool with_vbuf, vstr_t* vbuf, fstr_t max_buf) {
    if (separator.len == 0)
        return fstr_slice(max_buf, 0, 0);
    // The buf_len is the length of the buffer that has been filled by copying over peaked chunks.
    size_t buf_len = 0;
    // Track offset where last chunk peek started in peek_offs.
    uint64_t peek_offs = 0;
    for (size_t i = 0;; i++) {
        for (size_t j = 0;; j++) {
            size_t buf_offs = (i + j);
            if (j == separator.len) {
                // Separator matched, skip up to current offset.
                rio_skip(rio, buf_offs - peek_offs);
                // Slice the non-consummed and non-skipped data from the buffer and
                // return the matched chunk up to i offset (before separator match).
                if (with_vbuf) {
                    vec_resize(vbuf, uint8_t, i);
                    return vstr_str(vbuf);
                } else {
                    return fstr_slice(max_buf, 0, i);
                }
            }
            while (buf_offs == buf_len) {
                // End of buffer reached. Extend buffer content by peeking next chunk.
                rio_skip(rio, buf_offs - peek_offs);
                fstr_t chunk = rio_peek(rio);
                peek_offs = buf_offs;
                // Copy chunk to buffer.
                if (with_vbuf) {
                    buf_len = buf_len + chunk.len;
                    vstr_write(vbuf, chunk);
                } else {
                    fstr_t mb_tail = fstr_sslice(max_buf, buf_len, -1);
                    if (mb_tail.len == 0)
                        throw("reached end of max buffer without matching separator", exception_io);
                    fstr_cpy_over(mb_tail, chunk, 0, 0);
                    buf_len = MIN(buf_len + chunk.len, max_buf.len);
                }
            }
            // Match character with separator.
            fstr_t buf = with_vbuf? vstr_str(vbuf): max_buf;
            if (separator.str[j] != buf.str[buf_offs])
                break;
        }
    }
}

fstr_t rio_read_to_separator(rio_t* rio, fstr_t separator, fstr_t max_buffer) {
    return read_to_separator(rio, separator, false, 0, max_buffer);
}

fstr_t rio_read_until(rio_t* rio, vstr_t* buf, fstr_t separator) {
    return read_to_separator(rio, separator, true, buf, "");
}

fstr_t rio_write_chunk(rio_t* rio, fstr_t chunk, bool more_hint) {
    if (rio->type == rio_type_abstract) {
        if (rio->xfer.abstract.impl->write_part_fn == 0 || !rio->xfer.abstract.is_writable)
            throw("the specified rio handle does not support the operation write", exception_arg);
        if (chunk.len == 0)
            return chunk;
        return rio->xfer.abstract.impl->write_part_fn(rio->xfer.abstract.fid_arg, chunk, more_hint);
    }
    int32_t write_fd = rio_get_fd_write(rio);
    if (write_fd == -1)
        throw("the specified rio handle does not support the operation write", exception_arg);
    bool send_with_msg_more = (more_hint && rio->type == rio_type_tcp);
    if (chunk.len == 0)
        return chunk;
    for (;;) {
        ssize_t n_sent;
        if (send_with_msg_more) {
            n_sent = send(write_fd, chunk.str, chunk.len, MSG_DONTWAIT | MSG_MORE);
        } else {
            n_sent = write(write_fd, chunk.str, chunk.len);
        }
        if (n_sent > 0) {
            return fstr_sslice(chunk, (ssize_t) n_sent, -1);
        } else if (n_sent == -1) {
            if (errno == EWOULDBLOCK) {
                lwt_block_until_edge_level_io_event(write_fd, lwt_fd_event_write);
            } else if (errno == EINTR) {
                continue;
            } else {
                if (send_with_msg_more) {
                    RCD_SYSCALL_EXCEPTION(send, exception_io);
                } else {
                    RCD_SYSCALL_EXCEPTION(write, exception_io);
                }
            }
        } else {
            sub_heap_e(throw(concs((send_with_msg_more? "send": "write"), "() failed: no data was written (abnormal return code)"), exception_io));
        }
    }
}

void rio_write(rio_t* rio, fstr_t buffer) {
    rio_write_part(rio, buffer, false);
}

void rio_write_part(rio_t* rio, fstr_t buffer, bool more_hint) {
    while (buffer.len > 0) {
        // Write next chunk to the rio stream.
        buffer = rio_write_chunk(rio, buffer, more_hint);
    }
}

void rio_forward(rio_t* in, rio_t* out, size_t len) { sub_heap {
    fstr_t buf = fss(fstr_alloc_buffer(MIN(len, 10 * PAGE_SIZE)));
    for (size_t i = 0; i < len;) {
        fstr_t chunk = rio_read(in, fstr_slice(buf, 0, len - i));
        rio_write(out, chunk);
        i += chunk.len;
    }
}}

fstr_mem_t* rio_read_fstr_max(rio_t* rio, size_t max_len) { sub_heap {
    uint64_t nbo_size;
    rio_read_fill(rio, FSTR_PACK(nbo_size));
    uint64_t size = RIO_NBO_SWAP64(nbo_size);
    if (max_len != 0 && size > max_len)
        sub_heap_e(throw(concs("the string was too large to be read [", ui2fs(size), "] > [", ui2fs(max_len), "]"), exception_io));
    fstr_mem_t* fstr = fstr_alloc(size);
    rio_read_fill(rio, fss(fstr));
    return escape(fstr);
}}

fstr_mem_t* rio_read_fstr(rio_t* rio) {
    return rio_read_fstr_max(rio, 0);
}

void rio_write_fstr(rio_t* rio, fstr_t buffer) {
    uint64_t nbo_size = RIO_NBO_SWAP64(buffer.len);
    rio_write(rio, FSTR_PACK(nbo_size));
    rio_write(rio, buffer);
}

void rio_iov_write_fstr(vec(fstr_t)* iov, fstr_t buffer) {
    rio_iov_write_u64(iov, buffer.len);
    vec_append(iov, fstr_t, buffer);
}

fstr_t rio_msg_recv(rio_t* rio, fstr_t buffer) {
    int32_t read_fd = rio_get_fd_read(rio);
    if (read_fd == -1)
        throw("the specified rio handle does not support the operation read", exception_arg);
    ssize_t recv_r;
    for (;;) {
        recv_r = recv(read_fd, buffer.str, buffer.len, 0);
        if (recv_r >= 0)
            break;
        else if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(read_fd, lwt_fd_event_read);
        else if (errno == EINTR)
            continue;
        else
            RCD_SYSCALL_EXCEPTION(recv, exception_io);
    }
    return fstr_slice(buffer, 0, recv_r);
}

static size_t rio_msg_send_raw(rio_t* rio, fstr_t buffer, bool no_wait) {
    int32_t write_fd = rio_get_fd_write(rio);
    if (write_fd == -1)
        throw("the specified rio handle does not support the operation write", exception_arg);
    for (;;) {
        ssize_t send_r = send(write_fd, buffer.str, buffer.len, 0);
        if (send_r >= 0) {
            return send_r;
        } else if (errno == EWOULDBLOCK) {
            if (no_wait) {
                return 0;
            } else {
                lwt_block_until_edge_level_io_event(write_fd, lwt_fd_event_write);
            }
        } else if (errno == EINTR) {
            continue;
        } else {
            RCD_SYSCALL_EXCEPTION(send, exception_io);
        }
    }
}

size_t rio_msg_send(rio_t* rio, fstr_t buffer) {
    return rio_msg_send_raw(rio, buffer, false);
}

size_t rio_msg_try_send(rio_t* rio, fstr_t buffer) {
    return rio_msg_send_raw(rio, buffer, true);
}

fstr_t rio_msg_recv_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t* out_src_addr) {
    RIO_CHECK_TYPE(rio, rio_type_udp);
    int32_t read_fd = rio_get_fd_read(rio);
    if (read_fd == -1)
        throw("the specified rio handle does not support the operation read", exception_arg);
    struct sockaddr_in s_addr;
    ssize_t recvfrom_r;
    for (;;) {
        socklen_t addrlen = sizeof(s_addr);
        recvfrom_r = recvfrom(read_fd, buffer.str, buffer.len, 0, (void*) &s_addr, &addrlen);
        if (recvfrom_r >= 0)
            break;
        else if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(read_fd, lwt_fd_event_read);
        else if (errno == EINTR)
            continue;
        else
            RCD_SYSCALL_EXCEPTION(recvfrom, exception_io);
    }
    if (out_src_addr != 0) {
        out_src_addr->address = RIO_NBO_SWAP32(s_addr.sin_addr.s_addr);
        out_src_addr->port = RIO_NBO_SWAP16(s_addr.sin_port);
    }
    return fstr_slice(buffer, 0, recvfrom_r);
}

static size_t rio_msg_send_udp_raw(rio_t* rio, fstr_t buffer, rio_in_addr4_t dest_addr, bool no_wait) {
    RIO_CHECK_TYPE(rio, rio_type_udp);
    int32_t write_fd = rio_get_fd_write(rio);
    if (write_fd == -1)
        throw("the specified rio handle does not support the operation write", exception_arg);
    struct sockaddr_in s_addr = {
        .sin_family = AF_INET,
        .sin_port = RIO_NBO_SWAP16(dest_addr.port),
        .sin_addr = RIO_NBO_SWAP32(dest_addr.address)
    };
    for (;;) {
        ssize_t sendto_r = sendto(write_fd, buffer.str, buffer.len, 0, (void*) &s_addr, sizeof(s_addr));
        if (sendto_r >= 0) {
            return sendto_r;
        } else if (errno == EWOULDBLOCK) {
            if (no_wait) {
                return 0;
            } else {
                lwt_block_until_edge_level_io_event(write_fd, lwt_fd_event_write);
            }
        } else if (errno == EINTR) {
            continue;
        } else {
            RCD_SYSCALL_EXCEPTION(sendto, exception_io);
        }
    }
}

size_t rio_msg_send_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t dest_addr) {
    return rio_msg_send_udp_raw(rio, buffer, dest_addr, false);
}

size_t rio_msg_try_send_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t dest_addr) {
    return rio_msg_send_udp_raw(rio, buffer, dest_addr, true);
}

rio_t* rio_tcp_client(rio_in_addr4_t remote_addr) { sub_heap {
    int32_t fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(socket, exception_io);
    rio_t* rio = rio_new_h(rio_type_tcp, fd, true, true, 0);
    struct sockaddr_in s_addr = {
        .sin_family = AF_INET,
        .sin_port = RIO_NBO_SWAP16(remote_addr.port),
        .sin_addr = RIO_NBO_SWAP32(remote_addr.address),
    };
    int32_t connect_r = connect(fd, (void*) &s_addr, sizeof(s_addr));
    if ((connect_r == -1) && (errno != EINPROGRESS))
        RCD_SYSCALL_EXCEPTION(connect, exception_io);
    return escape(rio);
}}

rio_t* rio_tcp_server(rio_in_addr4_t bind_addr, int32_t backlog) { sub_heap {
    int32_t fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(socket, exception_io);
    int32_t opt_val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1)
        RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
    rio_t* rio = rio_new_h(rio_type_tcp, fd, false, false, 0);
    struct sockaddr_in s_addr = {
        .sin_family = AF_INET,
        .sin_port = RIO_NBO_SWAP16(bind_addr.port),
        .sin_addr = RIO_NBO_SWAP32(bind_addr.address),
    };
    int32_t bind_r = bind(fd, (void*) &s_addr, sizeof(s_addr));
    if (bind_r == -1)
        RCD_SYSCALL_EXCEPTION(bind, exception_io);
    int32_t listen_r = listen(fd, backlog);
    if (listen_r == -1)
        RCD_SYSCALL_EXCEPTION(listen, exception_io);
    return escape(rio);
}}

rio_t* rio_tcp_accept(rio_t* rio_tcp_server, rio_in_addr4_t* out_remote_addr) {
    RIO_CHECK_TYPE(rio_tcp_server, rio_type_tcp);
    struct sockaddr_in s_addr;
    int32_t fd;
    for (;;) {
        socklen_t addrlen = sizeof(s_addr);
        fd = accept4(rio_tcp_server->xfer.duplex.fd, (void*) &s_addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd >= 0)
            break;
        else if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(rio_tcp_server->xfer.duplex.fd, lwt_fd_event_read);
        else if (errno == EINTR)
            continue;
        else
            RCD_SYSCALL_EXCEPTION(accept4, exception_io);
    }
    if (out_remote_addr != 0) {
        out_remote_addr->address = RIO_NBO_SWAP32(s_addr.sin_addr.s_addr);
        out_remote_addr->port = RIO_NBO_SWAP16(s_addr.sin_port);
    }
    return rio_new_h(rio_type_tcp, fd, true, true, 0);
}

void rio_tcp_set_keepalive(rio_t* rio, rio_tcp_ka_t ka) {
    RIO_CHECK_TYPE(rio, rio_type_tcp);
    {
        int32_t setsockopt_r = setsockopt(rio->xfer.duplex.fd, SOL_TCP, TCP_KEEPIDLE, (char*) &ka.idle_before_ping_s, sizeof(ka.idle_before_ping_s));
        if (setsockopt_r == -1)
            RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
    }{
        int32_t setsockopt_r = setsockopt(rio->xfer.duplex.fd, SOL_TCP, TCP_KEEPINTVL, (char*) &ka.ping_interval_s, sizeof(ka.ping_interval_s));
        if (setsockopt_r == -1)
            RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
    }{
        int32_t setsockopt_r = setsockopt(rio->xfer.duplex.fd, SOL_TCP, TCP_KEEPCNT, (char*) &ka.count_before_timeout, sizeof(ka.count_before_timeout));
        if (setsockopt_r == -1)
            RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
    }
}

bool rio_tcp_conn_wait(rio_t* rio, int32_t* out_so_error) {
    RIO_CHECK_TYPE(rio, rio_type_tcp);
    // Wait until socket is writable.
    rio_poll(rio, false, true);
    // Get pending error with SO_ERROR.
    int32_t so_error;
    socklen_t so_error_len = sizeof(so_error);
    int32_t getsockopt_r = getsockopt(rio->xfer.duplex.fd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len);
    if (getsockopt_r == -1)
        RCD_SYSCALL_EXCEPTION(getsockopt, exception_io);
    if (so_error == 0)
        return true;
    if (out_so_error != 0)
        *out_so_error = so_error;
    return false;
}

rio_t* rio_udp_client() {
    int32_t fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(socket, exception_io);
    return rio_new_h(rio_type_udp, fd, true, true, 0);
}

rio_t* rio_udp_server(rio_in_addr4_t* bind_addr) {
    int32_t fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(socket, exception_io);
    sub_heap {
        struct sockaddr_in s_addr = {
            .sin_family = AF_INET,
            .sin_port = RIO_NBO_SWAP16(bind_addr->port),
            .sin_addr = RIO_NBO_SWAP32(bind_addr->address),
        };
        int32_t bind_r = bind(fd, (void*) &s_addr, sizeof(s_addr));
        if (bind_r == -1)
            RCD_SYSCALL_EXCEPTION(bind, exception_io);
    }
    return rio_new_h(rio_type_udp, fd, true, true, 0);
}

void rio_udp_toggle_broadcast(rio_t* rio, bool enable) {
    RIO_CHECK_TYPE(rio, rio_type_udp);
    int32_t so_broadcast = (enable? 1: 0);
    int32_t setsockopt_r = setsockopt(rio->xfer.duplex.fd, SOL_SOCKET, SO_BROADCAST, (char*) &so_broadcast, sizeof(so_broadcast));
    if (setsockopt_r == -1)
        RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
}

rio_in_addr4_t rio_get_socket_address(rio_t* rio, bool peer) {
    if (rio->type != rio_type_tcp && rio->type != rio_type_udp)
        RIO_THROW_TYPE_ERROR("rio_type_tcp or rio_type_udp");
    struct sockaddr_in s_addr;
    socklen_t s_len = sizeof(s_addr);
    if (peer) {
        int32_t getpeername_r = getpeername(rio->xfer.duplex.fd, (void*) &s_addr, &s_len);
        if (getpeername_r == -1)
            RCD_SYSCALL_EXCEPTION(getpeername, exception_io);
    } else {
        int32_t getsockname_r = getsockname(rio->xfer.duplex.fd, (void*) &s_addr, &s_len);
        if (getsockname_r == -1)
            RCD_SYSCALL_EXCEPTION(getsockname, exception_io);
    }
    rio_in_addr4_t out_addr = {.address = RIO_NBO_SWAP32(s_addr.sin_addr.s_addr), .port = RIO_NBO_SWAP16(s_addr.sin_port)};
    return out_addr;
}

void rio_bind_to_device(rio_t* rio, fstr_t dev_name) {
    if (rio->type != rio_type_tcp && rio->type != rio_type_udp)
        RIO_THROW_TYPE_ERROR("rio_type_tcp or rio_type_udp");
    int32_t setsockopt_r = setsockopt(rio->xfer.duplex.fd, SOL_SOCKET, SO_BINDTODEVICE, dev_name.str, dev_name.len);
    if (setsockopt_r == -1)
        RCD_SYSCALL_EXCEPTION(setsockopt, exception_io);
}

uint128_t rio_get_time_timer() {
    struct timespec tp;
    int32_t clock_gettime_r = clock_gettime(CLOCK_MONOTONIC, &tp);
    if (clock_gettime_r == -1)
        RCD_SYSCALL_EXCEPTION(clock_gettime, exception_io);
    return tp.tv_nsec + (uint128_t) tp.tv_sec * 1000000000;
}


uint128_t rio_clock_time_to_epoch(rio_clock_time_t clock_time) {
    const size_t epoch_year = 1970;
    const uint128_t sec_ns = RIO_NS_SEC;
    const uint128_t min_ns = sec_ns * 60;
    const uint128_t hour_ns = min_ns * 60;
    const uint128_t day_ns = hour_ns *24;
    size_t days = rio_days_year(clock_time.year)
        - rio_days_year(epoch_year)
        + rio_year_day(rio_is_leap_year(clock_time.year), clock_time.month, clock_time.month_day);
    return days * day_ns
        + clock_time.hour * hour_ns
        + clock_time.minute * min_ns
        + clock_time.second * sec_ns
        + clock_time.nanosecond;
}

rio_clock_time_t rio_epoch_to_clock_time(uint128_t epoch_ns) {
    const size_t epoch_year = 1970;
    const size_t sec_per_day = (24 * 60 * 60);
    const uint8_t dpm[2][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    // Do main division.
    uint128_t epoch_s = (epoch_ns / RIO_NS_SEC);
    uint32_t day_sec = epoch_s % sec_per_day;
    uint32_t day_n = epoch_s / sec_per_day;
    // Calculate time.
    rio_clock_time_t clock_tt;
    clock_tt.nanosecond = epoch_ns % RIO_NS_SEC;
    clock_tt.second = day_sec % 60;
    clock_tt.minute = (day_sec % 3600) / 60;
    clock_tt.hour = day_sec / 3600;
    // Scan over years to calculate year + year day.
    uint32_t year = epoch_year;
    while (day_n >= rio_year_days(year)) {
        day_n -= rio_year_days(year);
        year++;
    }
    clock_tt.year = year;
    // Scan over months to calculate month + month day.
    clock_tt.month = 0;
    uint8_t is_leap_yr = rio_is_leap_year(year);
    while (day_n >= dpm[is_leap_yr][clock_tt.month]) {
        day_n -= dpm[is_leap_yr][clock_tt.month];
        clock_tt.month++;
    }
    clock_tt.month++;
    clock_tt.month_day = day_n + 1;
    return clock_tt;
}

uint128_t rio_epoch_ns_now() {
    struct timespec tp;
    int32_t clock_gettime_r = clock_gettime(CLOCK_REALTIME, &tp);
    if (clock_gettime_r == -1)
        RCD_SYSCALL_EXCEPTION(clock_gettime, exception_io);
    return tp.tv_nsec + (uint128_t) tp.tv_sec * 1000000000;
}

size_t rio_year_day(bool leap_year, size_t month, size_t month_day) {
    const uint128_t days_before_month[2][12] = {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
    };
    // Month and month_day are 1 indexed.
    return days_before_month[leap_year? 1: 0][month -1] + (month_day - 1);
}

rio_date_time_t rio_clock_to_date_time(rio_clock_time_t clock) {
    size_t year_day = rio_year_day(rio_is_leap_year(clock.year), clock.month, clock.month_day);
    size_t epoch_day = rio_clock_time_to_epoch(clock) / RIO_NS_SEC / 60 / 60 / 24;
    return (rio_date_time_t) {
        .second = clock.second,
        .minute = clock.minute,
        .hour = clock.hour,
        .month_day = clock.month_day,
        .month = clock.month,
        .year = clock.year,
        .year_day = year_day,
        .week_day = (epoch_day + 3) % 7
    };
}

fstr_mem_t* rio_clock_to_rfc3339(rio_clock_time_t clock, size_t n_sec_frac) { sub_heap {
    n_sec_frac = MIN(n_sec_frac, 9);
    fstr_t date_fullyear, date_month, date_mday, time_hour, time_minute, time_second;
    FSTR_STACK_DECL(date_fullyear, 4);
    FSTR_STACK_DECL(date_month, 2);
    FSTR_STACK_DECL(date_mday, 2);
    FSTR_STACK_DECL(time_hour, 2);
    FSTR_STACK_DECL(time_minute, 2);
    FSTR_STACK_DECL(time_second, 2);
    fstr_serial_uint(date_fullyear, clock.year, 10);
    fstr_serial_uint(date_month, clock.month, 10);
    fstr_serial_uint(date_mday, clock.month_day, 10);
    fstr_serial_uint(time_hour, clock.hour, 10);
    fstr_serial_uint(time_minute, clock.minute, 10);
    fstr_serial_uint(time_second, clock.second, 10);
    // Serialize fraction.
    fstr_t frac_final_buf;
    FSTR_STACK_DECL(frac_final_buf, 16);
    fstr_t time_sec_frac;
    if (n_sec_frac > 0) {
        fstr_t ns_slice = fss(fstr_from_int_pad(clock.nanosecond, 10, 9));
        assert(ns_slice.len == 9);
        frac_final_buf.str[0] = '.';
        size_t i = 0;
        for (; i < n_sec_frac; i++)
            frac_final_buf.str[1 + i] = ns_slice.str[i];
        time_sec_frac.str = frac_final_buf.str;
        time_sec_frac.len = 1 + n_sec_frac;
    } else {
        time_sec_frac = "";
    }
    return escape(conc(date_fullyear, "-", date_month, "-", date_mday, "T", time_hour, ":", time_minute, ":", time_second, time_sec_frac, "Z"));
}}

fstr_mem_t* rio_clock_to_rfc1123(rio_clock_time_t clock_time) { sub_heap {
    rio_date_time_t date_time = rio_clock_to_date_time(clock_time);
    static fstr_t wdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    static fstr_t months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    fstr_t wday = wdays[date_time.week_day];
    fstr_t month = months[date_time.month - 1];
    fstr_t date_mday, date_fullyear, time_hour, time_minute, time_second;
    FSTR_STACK_DECL(date_mday, 2);
    FSTR_STACK_DECL(date_fullyear, 4);
    FSTR_STACK_DECL(time_hour, 2);
    FSTR_STACK_DECL(time_minute, 2);
    FSTR_STACK_DECL(time_second, 2);
    fstr_serial_uint(date_mday, date_time.month_day, 10);
    fstr_serial_uint(date_fullyear, date_time.year, 10);
    fstr_serial_uint(time_hour, date_time.hour, 10);
    fstr_serial_uint(time_minute, date_time.minute, 10);
    fstr_serial_uint(time_second, date_time.second, 10);
    return escape(conc(wday, ", ", date_mday, " ", month, " ", date_fullyear, " ", time_hour, ":", time_minute, ":", time_second, " GMT"));
}}

rio_clock_time_t rio_rfc3339_to_clock(fstr_t clock_str) {
    const size_t ns_frac_digits = 9;
    fstr_t year_s, month_s, day_s, hour_s, minute_s, second_s, sec_frac_s;
    {
        #pragma ocre2c(clock_str):  \
        ^ (\d{4,4}){year_s} - (\d{2,2}){month_s} - (\d{2,2}){day_s} \
        T (\d{2,2}){hour_s} : (\d{2,2}){minute_s} : (\d{2,2}){second_s} \
        ((\.\d+)?){sec_frac_s} (Z | \+00:00) $ {@match}
        throw("not valid rfc3339 time", exception_io);
    } match: {
        rio_clock_time_t clock_time = {
            .year = fs2ui(year_s),
            .month = fs2ui(month_s),
            .month_day = fs2ui(day_s),
            .hour = fs2ui(hour_s),
            .minute = fs2ui(minute_s),
            .second = fs2ui(second_s)
        };
        if (sec_frac_s.len > 1) {
            if (!fstr_equal(fstr_slice(sec_frac_s, 0, 1), "."))
                throw("rfc3339 time-secfrac", exception_io);
            // Cut off the dot and digits after 9.
            fstr_t sec_frac_part = fstr_slice(sec_frac_s, 1, MIN(1 + ns_frac_digits, sec_frac_s.len));
            // Pad the fraction string with zeroes to be 9 digits
            fstr_t sec_frac_ns_buf;
            FSTR_STACK_DECL(sec_frac_ns_buf, ns_frac_digits);
            fstr_fill(sec_frac_ns_buf, '0');
            fstr_cpy_over(fstr_slice(sec_frac_ns_buf, 0, sec_frac_part.len), sec_frac_part, 0, 0);
            clock_time.nanosecond = fs2ui(sec_frac_ns_buf);
        }
        return clock_time;
    }
}

fstr_mem_t* rio_clock_to_iso8601_date(rio_clock_time_t clock_time, bool no_dash, bool no_day) { sub_heap {
    if (no_dash && no_day)
        throw("no dash and no day is not valid iso8601 format", exception_io);
    fstr_t fullyear, month, month_day;
    FSTR_STACK_DECL(fullyear, 4);
    FSTR_STACK_DECL(month, 2);
    FSTR_STACK_DECL(month_day, 2);
    fstr_serial_uint(fullyear, clock_time.year, 10);
    fstr_serial_uint(month, clock_time.month, 10);
    if (!no_day)
        fstr_serial_uint(month_day, clock_time.month_day, 10);
    fstr_t tokens[] = {
        fullyear,
        month,
        month_day,
    };
    return escape(fstr_concat(tokens, LENGTHOF(tokens) - (no_day? 1: 0), no_dash? "": "-"));
}}

rio_clock_time_t rio_iso8601_date_to_clock(fstr_t clock_str) {
    rio_clock_time_t clock_time = {0};
    fstr_t year_s, month_s, day_s;
    {
        #pragma re2c(clock_str): \
              ^ (\d{4,4}){year_s} - (\d{2,2}){month_s} - (\d{2,2}){day_s} $ {@match_ymd} \
            | ^ (\d{4,4}){year_s} (\d{2,2}){month_s} (\d{2,2}){day_s} $ {@match_ymd} \
            | ^ (\d{4,4}){year_s} - (\d{2,2}){month_s} $ {@match_ym} \
            | ^ (\d{4,4}){year_s} $ {@match_y}
        // Invalid format or a non supported iso 8601 format like "Ordinal dates".
        throw("not valid iso8601 syntax or not implemented format", exception_io);
    } match_ymd: {
        clock_time.month_day = fs2ui(day_s);
    } match_ym: {
        clock_time.month = fs2ui(month_s);
    } match_y: {
        clock_time.year = fs2ui(year_s);
    }
    return clock_time;
}

rio_t* rio_timer_create() {
    int32_t fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(timerfd_create, exception_io);
    return rio_new_h(rio_type_timer, fd, true, false, 0);
}

rio_t* rio_clock_create() {
    int32_t fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(timerfd_create, exception_io);
    return rio_new_h(rio_type_clock, fd, true, false, 0);
}

rio_t* rio_alarm_create(uint128_t alarm_ns, bool repeat) {
    rio_t* timer_h = rio_timer_create();
    rio_alarm_set(timer_h, alarm_ns, false, repeat? alarm_ns: 0);
    return timer_h;
}

void rio_alarm_set(rio_t* rio, uint128_t value_ns, bool value_absolute, uint128_t repeat_ns) {
    if (rio->type != rio_type_timer && rio->type != rio_type_clock)
        RIO_THROW_TYPE_ERROR("rio_type_timer or rio_type_clock");
    struct itimerspec time_spec = {.it_interval = {.tv_nsec = repeat_ns % 1000000000, .tv_sec = repeat_ns / 1000000000}
    , .it_value = {.tv_nsec = value_ns % 1000000000, .tv_sec = value_ns / 1000000000}};
    int32_t settime_r = timerfd_settime(rio->xfer.duplex.fd, value_absolute? TFD_TIMER_ABSTIME: 0, &time_spec, 0);
    if (settime_r == -1)
        RCD_SYSCALL_EXCEPTION(timerfd_settime, exception_io);
}

uint64_t rio_alarm_wait(rio_t* rio) {
    if (rio->type != rio_type_timer && rio->type != rio_type_clock)
        RIO_THROW_TYPE_ERROR("rio_type_timer or rio_type_clock");
    fstr_t n_timeouts_str;
    FSTR_STACK_DECL(n_timeouts_str, sizeof(uint64_t));
    rio_read_fill(rio, n_timeouts_str);
    uint64_t n_timeouts = *((uint64_t*) n_timeouts_str.str);
    return n_timeouts;
}

void rio_wait(uint128_t wait_ns) {
    if (wait_ns == 0)
        return;
    sub_heap {
        rio_t* timer_h = rio_timer_create();
        rio_alarm_set(timer_h, wait_ns, false, 0);
        rio_alarm_wait(timer_h);
    }
}

rio_t* rio_eventfd_create(int64_t init_value, bool semaphore) {
    int32_t fd = eventfd2(init_value, EFD_CLOEXEC | EFD_NONBLOCK | (semaphore? EFD_SEMAPHORE: 0));
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(eventfd2, exception_io);
    return rio_new_h(rio_type_eventfd, fd, true, true, 0);
}

void rio_eventfd_trigger(rio_t* rio, uint64_t count) {
    RIO_CHECK_TYPE(rio, rio_type_eventfd);
    rio_write(rio, FSTR_PACK(count));
}

uint64_t rio_eventfd_wait(rio_t* rio) {
    RIO_CHECK_TYPE(rio, rio_type_eventfd);
    uint64_t count;
    rio_read_fill(rio, FSTR_PACK(count));
    return count;
}

static void rio_epoll_destruct(void* arg_ptr) { uninterruptible {
    rio_epoll_t* epoll_h = arg_ptr;
    if (epoll_h->et_fd != -1) {
        lwt_io_free_fd_tracking(epoll_h->et_fd);
        rio_strict_close(epoll_h->et_fd);
    }
    if (epoll_h->lt_fd != -1) {
        lwt_io_free_fd_tracking(epoll_h->lt_fd);
        rio_strict_close(epoll_h->lt_fd);
    }
}}

static rio_epoll_t* rio_epoll_create_internal(int32_t target_fd, uint32_t epoll_events) {
    int32_t epoll_et_fd;
    {
        int32_t epoll_create1_r = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_create1_r == -1)
            RCD_SYSCALL_EXCEPTION(epoll_create1, exception_io);
        epoll_et_fd = epoll_create1_r;
    }
    int32_t epoll_lt_fd;
    {
        int32_t epoll_create1_r = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_create1_r == -1) {
            rio_strict_close(epoll_et_fd);
            RCD_SYSCALL_EXCEPTION(epoll_create1, exception_io);
        }
        epoll_lt_fd = epoll_create1_r;
    }
    rio_epoll_t* epoll_h = lwt_alloc_destructable(sizeof(rio_epoll_t), rio_epoll_destruct);
    epoll_h->et_fd = epoll_et_fd;
    epoll_h->lt_fd = epoll_lt_fd;
    epoll_h->events = epoll_events;
    {
        struct epoll_event eevent = {.events = epoll_events};
        int32_t epoll_ctl_r = epoll_ctl(epoll_lt_fd, EPOLL_CTL_ADD, target_fd, &eevent);
        if (epoll_ctl_r == -1)
            RCD_SYSCALL_EXCEPTION(epoll_ctl, exception_io);
    }{
        struct epoll_event eevent = {.events = epoll_events | EPOLLET};
        int32_t epoll_ctl_r = epoll_ctl(epoll_et_fd, EPOLL_CTL_ADD, target_fd, &eevent);
        if (epoll_ctl_r == -1)
            RCD_SYSCALL_EXCEPTION(epoll_ctl, exception_io);
    }
    return epoll_h;
}

rio_epoll_t* rio_epoll_create(rio_t* rio_target, rio_epoll_event_t epoll_event) {
    int32_t target_fd;
    uint32_t eevent;
    switch (epoll_event) {
    case rio_epoll_event_inlvl:
        eevent = EPOLLIN;
        target_fd = rio_get_fd_read(rio_target);
        break;
    case rio_epoll_event_outlvl:
        eevent = EPOLLOUT;
        target_fd = rio_get_fd_write(rio_target);
        break;
    case rio_epoll_event_hup:
        eevent = EPOLLRDHUP;
        target_fd = rio_get_fd_read(rio_target);
        if (target_fd == -1)
            target_fd = rio_get_fd_write(rio_target);
        break;
    default:
        throw("unknown epoll_event specified", exception_arg);
    }
    if (target_fd == -1)
        throw("the specified target rio handle does not support the requested poll wait operation", exception_arg);
    return rio_epoll_create_internal(target_fd, eevent);
}

rio_epoll_t* rio_epoll_create_raw(int32_t target_fd, rio_epoll_event_t epoll_event) {
    uint32_t eevent;
    switch (epoll_event) {
    case rio_epoll_event_inlvl:
        eevent = EPOLLIN;
        break;
    case rio_epoll_event_outlvl:
        eevent = EPOLLOUT;
        break;
    case rio_epoll_event_hup:
        eevent = EPOLLRDHUP;
        break;
    default:
        throw("unknown epoll_event specified", exception_arg);
    }
    return rio_epoll_create_internal(target_fd, eevent);
}

static bool rio_epoll_poll_internal(int32_t epoll_fd) {
    struct epoll_event events[1];
    int32_t epoll_wait_r = epoll_wait(epoll_fd, events, LENGTHOF(events), 0);
    if (epoll_wait_r == -1 && errno != EINTR)
        RCD_SYSCALL_EXCEPTION(epoll_wait, exception_io);
    return (epoll_wait_r != 0);
}

bool rio_epoll_poll(rio_epoll_t* epoll_h, bool wait) {
    for (;;) {
        bool level_ready = rio_epoll_poll_internal(epoll_h->lt_fd);
        if (level_ready || !wait)
            return level_ready;
       lwt_block_until_epoll_ready(epoll_h->et_fd, lwt_fd_event_read);
    }
}

static void rio_open_unix_socket_pair(int32_t type, rio_t** out_socket_1, rio_t** out_socket_2) {
    int32_t socket_fds[2];
    int32_t socketpair_r = socketpair(AF_UNIX, type | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, socket_fds);
    if (socketpair_r == -1)
        RCD_SYSCALL_EXCEPTION(socketpair, exception_io);
    *out_socket_1 = rio_new_h(type == SOCK_STREAM? rio_type_unix_stream: rio_type_unix_dgram, socket_fds[0], true, true, 0);
    *out_socket_2 = rio_new_h(type == SOCK_STREAM? rio_type_unix_stream: rio_type_unix_dgram, socket_fds[1], true, true, 0);
}

void rio_open_unix_socket_stream_pair(rio_t** out_socket_1, rio_t** out_socket_2) {
    rio_open_unix_socket_pair(SOCK_STREAM, out_socket_1, out_socket_2);
}

void rio_open_unix_socket_dgram_pair(rio_t** out_socket_1, rio_t** out_socket_2) {
    rio_open_unix_socket_pair(SOCK_DGRAM, out_socket_1, out_socket_2);
}

rio_t* rio_open_unix_socket_client(fstr_t socket_path) { sub_heap {
    int32_t fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        RCD_SYSCALL_EXCEPTION(socket, exception_io);
    rio_t* rio = rio_new_h(rio_type_unix_stream, fd, true, true, 0);
    struct sockaddr_un* s_addr;
    size_t s_addr_len = sizeof(struct sockaddr_un) - sizeof(s_addr->sun_path) + socket_path.len + 1;
    s_addr = lwt_alloc_new(s_addr_len);
    s_addr->sun_family = AF_UNIX;
    memcpy(s_addr->sun_path, socket_path.str, socket_path.len);
    s_addr->sun_path[socket_path.len] = 0;
    int32_t connect_r = connect(fd, (void*) s_addr, s_addr_len);
    if ((connect_r == -1)&&(errno != EINPROGRESS))
        RCD_SYSCALL_EXCEPTION(connect, exception_io);
    return escape(rio);
}}

rio_t* rio_ipc_fd_recv(rio_t* unix_dgram_h, rio_type_t type, bool is_readable, bool is_writable) {
    RIO_CHECK_TYPE(unix_dgram_h, rio_type_unix_dgram);
    // Receive structures.
    struct iovec iov = {0};
    struct msghdr msg = {0};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    uint8_t control[0x400];
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    // Receive fd.
    for (int32_t fd = rio_get_fd_read(unix_dgram_h);;) {
        int32_t recvmsg_r = recvmsg(fd, &msg, 0);
        if (recvmsg_r != -1)
            break;
        if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(fd, lwt_fd_event_read);
        else
            RCD_SYSCALL_EXCEPTION(recvmsg, exception_io);
    }
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    rio_t* r_rio_h = 0;
    while (cmsg != 0) {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
            int32_t new_fd = *((int*) CMSG_DATA(cmsg));
            // Enable non blocking as this is usually disabled by convention.
            // We just expect the received file descriptor to have O_CLOEXEC
            // enabled as that is the only way to avoid the race of receiving
            // the fd and preventing it from leaking when calling execve.
            int32_t fcntl_r = rio_raw_fcntl_toggle_nonblocking(new_fd, true);
            if (fcntl_r == -1)
                RCD_SYSCALL_EXCEPTION(fcntl, exception_io);
            if (type == rio_type_pipe) {
                r_rio_h = rio_new_pipe_h((is_readable? new_fd: -1), (is_writable? new_fd: -1), 0);
            } else {
                r_rio_h = rio_new_h(type, new_fd, is_readable, is_writable, 0);
            }
            break;
        }
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
    if (r_rio_h == 0)
        throw("could not locate file descriptor in the received message", exception_io);
    return r_rio_h;
}

void rio_ipc_fd_send(rio_t* unix_dgram_h, rio_t* rio) {
    RIO_CHECK_TYPE(unix_dgram_h, rio_type_unix_dgram);
    if (rio->type == rio_type_pipe && rio->xfer.pipe.fd_read != -1 && rio->xfer.pipe.fd_write != -1)
        RIO_THROW_TYPE_ERROR("non combined rio handle");
    int32_t fd_to_send = (rio->type == rio_type_pipe? (rio->xfer.pipe.fd_read != -1? rio->xfer.pipe.fd_read: rio->xfer.pipe.fd_write): rio->xfer.duplex.fd);
    // Disable non-blocking if this was enabled as this is usually disabled by convention.
    int32_t fcntl_r = rio_raw_fcntl_toggle_nonblocking(fd_to_send, false);
    if (fcntl_r == -1)
        RCD_SYSCALL_EXCEPTION(fcntl, exception_io);
    // Reference data.
    struct iovec iov = {0};
    // Compose message.
    struct msghdr msg = {0};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    uint8_t control[0x400];
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    struct cmsghdr* cmsg;
    cmsg = CMSG_FIRSTHDR(&msg);
    // Need to clear the struct so padding is zeroed, otherwise we get EINVAL from sendmsg().
    *cmsg = (struct cmsghdr) {0};
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));
    *((int*) CMSG_DATA(cmsg)) = fd_to_send;
    msg.msg_controllen = cmsg->cmsg_len;
    // DBG("rio_ipc_fd_send: *** calling sendmsg() with sockfd: ", DBG_INT(unix_dgram_h->fds.duplex.fd));
    // DBG("rio_ipc_fd_send: *** calling sendmsg() with msg:\n", fss(fstr_hexdump(FSTR_PACK(msg))));
    // DBG("rio_ipc_fd_send: *** calling sendmsg() with control:\n", fss(fstr_hexdump((fstr_t) {.str = msg.msg_control, .len = msg.msg_controllen})));
    for (int32_t fd = rio_get_fd_write(unix_dgram_h);;) {
        int32_t sendmsg_r = sendmsg(fd, &msg, 0);
        if (sendmsg_r != -1)
            break;
        if (errno == EWOULDBLOCK)
            lwt_block_until_edge_level_io_event(fd, lwt_fd_event_write);
        else
            RCD_SYSCALL_EXCEPTION(sendmsg, exception_io);
    }
}

void rio_ipc_main_injection_handler(list(fstr_t)* main_args) {
    fstr_t arg0, arg1;
    if (list_unpack(main_args, fstr_t, &arg0, &arg1)) {
        if (fstr_equal(arg0, "main-inject")) {
            void (*main_fn_ptr)() = (void*) fstr_to_uint(arg1, 16);
            main_fn_ptr();
            exit_group(0);
        }
    }
}

list(fstr_t)* rio_ipc_main_injection_get_args(void (*main_fn_ptr)()) {
    return new_list(fstr_t, "main-inject", fss(fstr_from_uint((uint64_t) main_fn_ptr, 16)));
}

list(uint32_t)* rio_resolve_host_ipv4_addr(fstr_t host_name) {
    list(uint32_t)* addr_list_r = 0;
    sub_heap_txn(addr_list_r_heap) {
        switch_heap(addr_list_r_heap) {
            addr_list_r = new_list(uint32_t);
        }
        // Check /etc/hosts first.
        sub_heap {
            fstr_t etc_host_content;
            bool etc_host_ok = false;
            try {
                etc_host_content = fss(rio_read_file_contents("/etc/hosts"));
                etc_host_ok = true;
            } catch (exception_io, e) {}
            if (!etc_host_ok)
                break;
            fstr_t host_addr_c, host_names_c;
#pragma re2c(etc_host_content): ^ [\s]* ([\d\.]+){host_addr_c} ([\s]+ (([^\s]+))+){host_names_c} [\s]* \n {@matched_host_line}
            break;
            matched_host_line:;
            for (;;) {
                fstr_t host_name_c;
#pragma re2c(host_names_c): ^ [\s]* ([^\s]+){host_name_c} (\s|$) {@matched_host_name}
                break;
                matched_host_name:;
                if (fstr_equal(host_name_c, host_name)) {
                    bool addr_ok = false;
                    uint32_t addr;
                    try {
                        addr = rio_unserial_addr4(host_addr_c);
                        addr_ok = true;
                    } catch (exception_io, e);
                    if (!addr_ok)
                        continue;
                    switch_heap(addr_list_r_heap) {
                        list_push_end(addr_list_r, uint32_t, addr);
                    }
                    break;
                }
            }
        }
        if (list_count(addr_list_r, uint32_t) > 0)
            break;
        // No /etc/hosts match, do some DNS querying instead.
        rio_proc_t* dig_proc_h;
        rio_t* dig_pipe_h;
        rio_proc_execute_and_pipe(fss(rio_which("dig")), new_list(fstr_t, "+short", host_name), false, &dig_proc_h, &dig_pipe_h);
        fstr_t buffer = fss(fstr_alloc_buffer(0x10000));
        fstr_t dig_out = rio_read_to_end(dig_pipe_h, buffer);
        list_foreach(fstr_explode(dig_out, "\n"), fstr_t, serial_addr) {
            bool addr_ok = false;
            uint32_t addr;
            try {
                addr = rio_unserial_addr4(serial_addr);
                addr_ok = true;
            } catch (exception_io, e);
            if (!addr_ok)
                continue;
            switch_heap(addr_list_r_heap) {
                list_push_end(addr_list_r, uint32_t, addr);
            }
        }
        if (list_count(addr_list_r, uint32_t) == 0)
            throw(concs("failed to resolve the host name [", host_name, "], dig returned no results"), exception_io);
    }
    return addr_list_r;
}
