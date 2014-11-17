/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef RIO_H
#define	RIO_H

#include "rcd.h"

#define RIO_O32_HOST_ORDER (o32_host_order.value)

/// Swap a uint16_t value between native and network byte order.
#define RIO_NBO_SWAP16(a) ((uint16_t)(RIO_O32_HOST_ORDER != O32_LITTLE_ENDIAN? (uint16_t)(a): \
    (((uint16_t)(a) & 0x00FF) << 8) | (((uint16_t)(a) & 0xFF00) >> 8)))

/// Swap a uint32_t value between native and network byte order.
#define RIO_NBO_SWAP32(a) ((uint32_t)(RIO_O32_HOST_ORDER != O32_LITTLE_ENDIAN? (uint32_t)(a): \
				    (((uint32_t)(a) & 0x000000FF) << 24) | \
					(((uint32_t)(a) & 0x0000FF00) << 8) | \
				    (((uint32_t)(a) & 0x00FF0000) >> 8) | \
				    (((uint32_t)(a) & 0xFF000000) >> 24)))

/// Swap a uint64_t value between native and network byte order.
#define RIO_NBO_SWAP64(a) ((uint64_t)(RIO_O32_HOST_ORDER != O32_LITTLE_ENDIAN? (uint64_t)(a): \
    (((uint64_t)(a) & 0x00000000000000FFULL) << 56) | \
    (((uint64_t)(a) & 0x000000000000FF00ULL) << 40) | \
    (((uint64_t)(a) & 0x0000000000FF0000ULL) << 24) | \
    (((uint64_t)(a) & 0x00000000FF000000ULL) << 8) | \
    (((uint64_t)(a) & 0x000000FF00000000ULL) >> 8) | \
    (((uint64_t)(a) & 0x0000FF0000000000ULL) >> 24) | \
    (((uint64_t)(a) & 0x00FF000000000000ULL) >> 40) | \
    (((uint64_t)(a) & 0xFF00000000000000ULL) >> 56)))

/// Swap a uint128_t value between native and network byte order.
#define RIO_NBO_SWAP128(a) ((uint128_t)(RIO_O32_HOST_ORDER != O32_LITTLE_ENDIAN? (uint128_t)(a): \
    (uint128_t) RIO_NBO_SWAP64(a >> 64) | ((uint128_t) RIO_NBO_SWAP64(a & 0xFFFFFFFFFFFFFFFFULL) << 64)))

/// Converts an IPv4 address from it's dotted largest-byte-first notation to the machine ordered 32 bit integer format they are stored as in rio.
#define RIO_IPV4_ADDR_PACK(a3, a2, a1, a0) (((uint32_t) (a3 & 0xff) << 24) | ((uint32_t) (a2 & 0xff) << 16) | ((uint32_t) (a1 & 0xff) << 8) | ((uint32_t) (a0 & 0xff) << 0))

/// Unpacks an IPv4 address stored as machine ordered 32 bit integer into it's dotted format number components. The components should preferably be of type uint8_t.
#define RIO_IPV4_ADDR_UNPACK(src_uint32_addr, a3, a2, a1, a0) { \
    a3 = (src_uint32_addr >> 24) & 0xff; \
    a2 = (src_uint32_addr >> 16) & 0xff; \
    a1 = (src_uint32_addr >> 8) & 0xff; \
    a0 = (src_uint32_addr >> 0) & 0xff; \
}

/// One second in nanoseconds. (Nanosecond second)
#define RIO_NS_SEC ((uint128_t)(1000ULL * 1000ULL * 1000ULL))

/// One millisecond in nanoseconds. (Nanosecond millisecond)
#define RIO_NS_MS ((uint128_t)(1000ULL * 1000ULL))

/// When calling rio_file_chtime, specify this to indicate time now.
#define RIO_FILE_CHTIME_NOW ((uint128_t) -1)

/// When calling rio_file_chtime, specify this to omit change.
#define RIO_FILE_CHTIME_OMIT ((uint128_t) -2)

typedef enum {
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul,
} rio_o32_byte_order_t;

static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order = { { 0, 1, 2, 3 } };

typedef enum rio_in_version {
    rio_in_version_ipv4,
} rio_in_version_t;

typedef enum rio_type {
    rio_type_abstract,
    rio_type_file,
    rio_type_signal,
    rio_type_pipe,
    rio_type_tcp,
    rio_type_udp,
    rio_type_timer,
    rio_type_clock,
    rio_type_eventfd,
    /// unix stream.
    rio_type_unix_stream,
    /// unix datagram socket. currently only used for inter-process file
    /// descriptor transfer.
    rio_type_unix_dgram,
} rio_type_t;

/// An epoll handle.
typedef struct rio_epoll rio_epoll_t;

/// A subprocess handle. A subprocess have the same life cycle as the memory
/// of this struct. The memory of this struct is only read accessed when the
/// reference is used from the various rio_proc_* functions, so it is safe
/// to share the reference between fibers if it can be otherwise guaranteed
/// that the reference is not cleaned up while those fibers are not cleaned up.
typedef struct rio_proc rio_proc_t;

typedef enum rio_epoll_event {
    rio_epoll_event_inlvl,
    rio_epoll_event_outlvl,
    rio_epoll_event_hup,
} rio_epoll_event_t;

/// Abstract rio class interface.
typedef struct rio_class {
    /// Shall read to the buffer and return the head slice of the buffer that
    /// was read to. Shall throw an io exception if reading failed.
    /// If more hint is not null the stream should set it to indicate that it
    /// will be possible to read more data immediately after the call and the
    /// implementation should use this information to reduce any overhead
    /// caused by acting on consuming the read immediately.
    fstr_t (*read_part_fn)(rcd_fid_t fid_arg, fstr_t buffer, bool* out_more_hint);
    /// Shall write the buffer and return the tail slice of the buffer that was
    /// written. Shall throw an io exception if writing failed.
    /// If more hint is set the caller indicates that it will make one or more
    /// writes immediately after the call and the implementation should use
    /// this information to reduce any overhead caused by acting on the write
    /// immediately.
    fstr_t (*write_part_fn)(rcd_fid_t fid_arg, fstr_t buffer, bool more_hint);
    /// Shall wait until [reading does not block] if read is true or
    /// otherwise [writing does not block] and then return true.
    /// If wait is false the function should return immediately return false
    /// instead of waiting if the condition has not occurred yet.
    bool (*poll_fn)(rcd_fid_t fid_arg, bool read, bool wait);
} rio_class_t;

typedef struct rio_handle rio_t;

typedef struct rio_in_addr4 {
    /// Numerical IPv4 address stored in native machine format.
    uint32_t address;
    /// Numerical IPv4 port stored in native machine format.
    uint16_t port;
} rio_in_addr4_t;

typedef struct rio_in_addr6 {
    /// Numerical IPv6 address stored in native machine format.
    uint128_t address;
    /// Numerical IPv6 port stored in native machine format.
    uint16_t port;
} rio_in_addr6_t;

typedef enum rio_file_type {
    rio_file_type_unknown,
    rio_file_type_regular,
    rio_file_type_directory,
    rio_file_type_character_device,
    rio_file_type_block_device,
    rio_file_type_fifo,
    rio_file_type_symlink,
    rio_file_type_socket,
} rio_file_type_t;

typedef struct rio_stat {
    /// The type of file.
    rio_file_type_t file_type;
    /// The access mode of the file. (security)
    uint32_t access_mode;
    /// The user ID of the file's owner.
    uint32_t user_id;
    /// The group ID of the file.
    uint32_t group_id;
    /// The size of a regular file in bytes.
    size_t size;
    /// The last access time for the file, in nano seconds. Some file systems has this record disabled for performance reasons.
    uint128_t time_accessed;
    /// The last modification time for the file, in nano seconds.
    uint128_t time_modified;
    /// The last time the file meta data was changed, in nano seconds.
    uint128_t time_changed;
    /// The file serial number, which distinguishes this file from all other files on the same device.
    uint64_t inode;
    /// The device serial number, which distinguishes the device that the file is stored from other devices where files are stored.
    uint64_t device;
    /// Device ID; this entry is defined only for character or block special files.
    uint64_t rdevice_id;
    /// The number of hard links to the file.
    uint64_t n_hard_links;
} rio_stat_t;

typedef struct rio_date_time {
    /// Seconds. [0-60] (1 leap second)
    int32_t second;
    /// Minutes. [0-59]
    int32_t minute;
    /// Hours. [0-23]
    int32_t hour;
    /// Day. [1-31]
    int32_t month_day;
    /// Month. [1-12]
    int32_t month;
    /// Year.
    int32_t year;
    /// Day of week starting at monday. [0-6]
    int32_t week_day;
    /// Days in year.[0-365]
    int32_t year_day;
} rio_date_time_t;

typedef struct rio_clock_time {
    /// Nanosecond. [0-999,999,999]
    int32_t nanosecond;
    /// Seconds. [0-60] (1 leap second)
    int32_t second;
    /// Minutes. [0-59]
    int32_t minute;
    /// Hours. [0-23]
    int32_t hour;
    /// Day. [1-31]
    int32_t month_day;
    /// Month. [1-12]
    int32_t month;
    /// Year.
    int32_t year;
} rio_clock_time_t;

/// Keep alive configuration for a tcp stream.
typedef struct rio_tcp_ka {
    int32_t idle_before_ping_s;
    int32_t ping_interval_s;
    int32_t count_before_timeout;
} rio_tcp_ka_t;

/// User/group id.
typedef struct rio_id {
    int32_t uid;
    int32_t gid;
} rio_id_t;

typedef struct rio_exec {
    /// Path to executable to execute.
    fstr_t path;
    /// List of arguments passed to the executable.
    list(fstr_t)* args;
    /// List of environment variables passed to the executable.
    /// Each item should have the format KEY=VALUE.
    /// If 0 no environment is passed to the new executable.
    list(fstr_t)* env;
    /// Rio stream to use as stdin descriptor. If 0 stdin is inherited.
    rio_t* io_in;
    /// Rio stream to use as stdout descriptor. If 0 stdout is inherited.
    rio_t* io_out;
    /// Rio stream to use as stderr descriptor. If 0 stderr is inherited.
    rio_t* io_err;
    /// The uid/gid that will be set before executing.
    /// See rio_process_setid() for more information.
    rio_id_t set_id;
} rio_exec_t;

/// Arguments passed to rio_process_execve() when executing subprocesses.
typedef struct rio_sub_exec {
    rio_exec_t exec;
    /// If true the subprocess will run in it's own kernel namespace and have
    /// its own file descriptor table, pid list, mounts etc.
    bool new_kernel_ns;
} rio_sub_exec_t;

/// End of stream. Thrown when reading and a a stream is gracefully ended.
/// For files this is the end of file. For TCP this is a graceful connection close.
define_eio(rio_eos);

/// INTERNAL RIO FUNCTION that returns the read file descriptor associated with
/// a rio handle. Only useful when dealing with external file descriptors.
/// Normally you should not call this function directly.
int32_t rio_get_fd_read(rio_t* rio);

/// INTERNAL RIO FUNCTION that returns the write file descriptor associated
/// with a rio handle. Only useful when dealing with external file descriptors.
/// Normally you should not call this function directly.
int32_t rio_get_fd_write(rio_t* rio);

/// INTERNAL RIO FUNCTION that allocates a new auto-destructing rio handle.
/// Only useful when dealing with external file descriptors.
/// Normally you should not call this function directly.
rio_t* rio_new_h(rio_type_t type, int32_t fd, bool is_readable, bool is_writable, size_t peek_buf_min_len);

/// Creates a new instance of an abstract rio class with the specified
/// implementation. The impl struct memory will not be copied and will be
/// constantly referred to so it should be a constant struct declared in static
/// memory.
rio_t* rio_new_abstract(const rio_class_t* impl, rcd_fid_t fid_arg, size_t peek_buf_min_len);

/// INTERNAL RIO FUNCTION that allocates a new auto-destructing rio handle.
/// for a pipe. Only useful when dealing with external file descriptors.
/// Normally you should not call this function directly.
rio_t* rio_new_pipe_h(int32_t fd_read, int32_t fd_write, size_t peek_buf_min_len);

/// Serializes the numerical ipv4 address stored in native machine format into
/// standard ipv4 address decimal dot notation (b3.b2.b1.b0).
fstr_mem_t* rio_serial_addr4(uint32_t ipv4_addr);

/// Unserializes the numerical ipv4 address stored in standard ipv4 address
/// decimal dot notation (b3.b2.b1.b0) into native machine format.
/// Throws an io exception if the syntax of the ipv4 address serial is
/// invalid (a string that rio_serial_addr4() would never return).
uint32_t rio_unserial_addr4(fstr_t ipv4_addr_serial);

/// Serializes the addr exactly like rio_serial_addr4() + ":" + port.
fstr_mem_t* rio_serial_in_addr4(rio_in_addr4_t addr);

/// Unserializes the numerical ipv4 address stored in standard ipv4 address
/// decimal dot notation (b3.b2.b1.b0) into native machine format.
/// Throws an io exception if the syntax of the ipv4 address serial is
/// invalid (a string that rio_serial_ipv4_addr() would never return).
rio_in_addr4_t rio_unserial_in_addr4(fstr_t in_addr_serial);

/// Returns the underlying rio type.
rio_type_t rio_get_type(rio_t* rio) NO_NULL_ARGS;

/// Returns true if the underlying rio type is of network type (udp or tcp).
bool rio_is_network(rio_t* rio) NO_NULL_ARGS;

/// Returns true if the underlying rio type is of stream type (pipe or tcp).
bool rio_is_stream(rio_t* rio) NO_NULL_ARGS;

/// Reallocates a rio stream with a peek buffer of specified length so
/// rio_peek can be used to stream protocols that requires look-ahead to know
/// how much data to consume. Peek+skip allows transparently switching between
/// look-ahead read to separator and normal length fill. The unconsumed peek
/// buffer is preserved. The passed rio stream is deactivated and a new
/// is allocated. The passed rio stream becomes useless after this call.
rio_t* rio_realloc_peek_buffer(rio_t* rio, size_t peek_buf_len) NO_NULL_ARGS;

/// Realloactes a rio stream in the current heap context. Useful for passing
/// a rio stream to another thread. The passed rio stream is deactivated and a
/// new is allocated. The passed rio stream becomes useless after this call.
rio_t* rio_realloc(rio_t* rio) NO_NULL_ARGS;

/// INTERNAL RIO FUNCTION that writes data directly to a file descriptor
/// without scheduling the thread or allocating memory. If the file descriptor
/// blocks, so will the thread. This is comparable to direct I/O to a file.
/// If a non temporary error is encountered while writing the function will
/// return false. If out_errno is not 0 it will be set to the encountered
/// errno in this case. If the entire buffer is written without any errors the
/// function returns true.
bool rio_direct_write(int32_t write_fd, fstr_t data, int32_t* out_errno);

/// Writes data to stderr. Use DBG() instead.
void rio_debug(fstr_t str);

/// Writes data to stderr. Use DBG() instead.
void rio_debug_chunks(fstr_t* chunks, size_t n_chunks);

/// Returns current working directory.
fstr_mem_t* rio_get_cwd();

/// Checks if the given path exists and returns true on success and false
/// if the file system returns ENOENT (A component of path does not exist,
/// or path is an empty string.). Other failures causes an io exception to
/// be thown, e.g. if a component of the path prefix of path is not a
/// directory.
bool rio_file_exists(fstr_t file_path);

/// Attempts to stat the given path and returns the related information.
/// Throws an io exception if the syscall failed for any reason.
rio_stat_t rio_file_stat(fstr_t file_path);

/// Attempts to stat the given path, but does not follow symbolic links, and
/// returns the related exception.
/// Throws an io exception if the syscall failed for any reason.
rio_stat_t rio_file_lstat(fstr_t file_path);

/// Attempts to stat an open file.
/// Throws an io exception if the syscall failed for any reason.
rio_stat_t rio_file_fstat(rio_t* file_h);

/// Tests if the security mode of the file allow the path to accessed
/// with the specified intent. Returns true if it is so. Returns false
/// if the file system reports EACCES, ENOENT or EROFS. Other failures causes
/// an io exception to be thown, e.g. if a component of the path prefix of path
/// is not a directory.
bool rio_file_access(fstr_t file_path, bool read, bool write, bool execute);

/// Attempts to change access rights for the specified file.
/// Throws an io exception if any failure was reported by the system.
void rio_file_chmod(fstr_t file_path, uint32_t mode);

/// Attempts to change ownership for the specified file.
/// Throws an io exception if any failure was reported by the system.
void rio_file_chown(fstr_t file_path, uint32_t uid, uint32_t gid);

/// Attempts to change the last access and modification times of a file.
/// If the time component is RIO_FILE_CHTIME_NOW the time is changed to the
/// current timestamp.
/// If the time component is RIO_FILE_CHTIME_OMIT the time is not changed.
/// Throws an io exception if any failure was reported by the system.
void rio_file_chtime(fstr_t file_path, uint128_t time_accessed, uint128_t time_modified);

/// Creates a file in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mkreg(fstr_t file_path, uint32_t mode);

/// Creates a directory in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mkdir(fstr_t file_path);

/// Creates a character device in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mkchr(fstr_t file_path, uint32_t mode, uint64_t rdev_id);

/// Creates a block device in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mkblk(fstr_t file_path, uint32_t mode, uint64_t rdev_id);

/// Creates a fifo in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mkfifo(fstr_t file_path, uint32_t mode);

/// Creates a socket in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_mksock(fstr_t file_path, uint32_t mode);

/// Renames a file in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_rename(fstr_t old_file_path, fstr_t new_file_path);

/// Removes a directory from the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_rmdir(fstr_t file_path);

/// Creates a hard link in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_link(fstr_t dst_path, fstr_t src_path);

/// Removes a non-directory file from the system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_unlink(fstr_t file_path);

/// Creates a symbolic link in the file system.
/// Throws an io exception if any failure was reported by the system.
void rio_file_symlink(fstr_t dst_path, fstr_t src_path);

/// Reads a symbolic link in the file system.
/// Throws an io exception if any failure was reported by the system.
fstr_mem_t* rio_file_read_link(fstr_t file_path);

/// Returns the absolute path of the opened file. This is done by reading the
/// symlink generated by the kernel at the location /proc/self/fd/$fd.
fstr_mem_t* rio_file_get_path(rio_t* file_h);

/// Expands all symbolic links and resolves references to /./, /../ and extra
/// '/' characters in the string named by path to produce a canonicalized
/// absolute pathname. This is done by opening the file in read only mode
/// and reading the path with rio_file_get_path().
/// Throws an io exception if any failure was reported by the system, either
/// when opening the file or reading the link.
fstr_mem_t* rio_file_real_path(fstr_t file_path);

/// Opens a file for reading or writing.
/// If create is true it will be created if it does not exist.
rio_t* rio_file_open(fstr_t file_path, bool read_only, bool create);

/// Locks a file using unix advisory file locking.
void rio_file_lock(rio_t* file_h, bool exclusive);

/// Attempts to lock a file using unix advisory file locking.
/// Returns true on success, otherwise false.
bool rio_file_try_lock(rio_t* file_h, bool exclusive);

/// Unlocks a file locked with unix advisory file locking.
void rio_file_unlock(rio_t* file_h);

/// Locks a raw file using unix advisory file locking.
/// If mutex is true the function will block the physical working thread while
/// waiting to aquire the lock. This is a more light weight form of locking
/// as it avoids a thread clone when waiting and is thus more suitable for
/// synchronizing small cpu blocking transactions. The downside is that a stale
/// lock will block other fibers from running. It is recommended to only use
/// mutex locking in system programming contexts.
void rio_file_lock_raw(int32_t fd, bool exclusive, bool mutex);

/// Attempts to lock a raw file descriptor using unix advisory file locking.
/// Returns true on success, otherwise false.
bool rio_file_try_lock_raw(int32_t fd, bool exclusive);

/// Unlocks a raw file locked with unix advisory file locking.
void rio_file_unlock_raw(int32_t fd);

/// Syncs pending writes to a file with the disk.
void rio_file_fsync(rio_t* file_h);

/// Lists all file entities in a directory. Returns the list and the
/// alternative heap it was allocated on.
list(fstr_mem_t*)* rio_file_list(fstr_t file_path);

/// INTERNAL RIO FUNCTION, dealing with file descriptors directly is an anti pattern in librcd.
int32_t rio_raw_fcntl_toggle_cloexec(int fd, bool enable);

/// INTERNAL RIO FUNCTION, dealing with file descriptors directly is an anti pattern in librcd.
int32_t rio_raw_fcntl_toggle_nonblocking(int fd, bool enable);

/// INTERNAL RIO FUNCTION, dealing with file descriptors directly is an anti pattern in librcd.
int32_t rio_raw_dup(int32_t old_fd);

/// Opens a new handle to stdin.
rio_t* rio_stdin();

/// Opens a new handle to stdout.
rio_t* rio_stdout();

/// Opens a new handle to stderr. NOTE: You probably want to use DBG() instead.
rio_t* rio_stderr();

/// Creates a new system pipe. See pipe(2) for details.
rio_t* rio_open_pipe();

/// Close one end of a pipe. If the end was never open in the first place
/// nothing happens. If both ends are closed neither reading or writing is
/// possible but the rio handle is still allocated (but unusable).
void rio_pipe_close_end(rio_t* rio_pipe, bool read_end) NO_NULL_ARGS;

/// Closes the write end of the read pipe and read end of the write pipe and
/// combines the open ends into a single new rio handle that supports both
/// reading and writing. The read and write rio handles are free'd.
rio_t* rio_realloc_combined(rio_t* read_pipe, rio_t* write_pipe) NO_NULL_ARGS;

/// Deallocates a rio handle and splits it into it's reader and writer pieces.
/// The unconsumed peek buffer is preserved.
/// Both pipes and other rio handles are supported although
/// the given rio type must both support reading and writing. This allows
/// reading in one fiber and writing in another which is unsafe to do
/// asynchronously if they share a rio handle.
/// Abstract rio handles cannot be split by this function as the backend is
/// undefined. The underlying library must either create a separate reader and
/// writer or define an operation that can split a combined reader and writer
/// into two separate abstract rio handles.
/// The rio_combined rio handle becomes useless after this operation and can
/// be deallocated.
void rio_realloc_split(rio_t* rio_combined, rio_t** out_rio_reader, rio_t** out_rio_writer) NO_NULL_ARGS;

/// Opens /dev/null for reading and writing and returns a file rio handle to it.
rio_t* rio_open_dev_null();

/// Returns the file read/write offset relative to the beginning of the file.
/// Throws an io exception if any failure was reported by the system. If an io
/// exception is thrown the file handle has an undefined read/write offset.
ssize_t rio_get_file_offset(rio_t* rio);

/// Sets the file read/write offset to the specified value.
/// Throws an io exception if any failure was reported by the system. If an io
/// exception is thrown the file handle has an undefined read/write offset.
void rio_set_file_offset(rio_t* rio, ssize_t offs, bool relative);

/// Sets the file read/write offset relative to the file end to the specified
/// value.
/// Throws an io exception if any failure was reported by the system. If an io
/// exception is thrown the file handle has an undefined read/write offset.
void rio_set_file_offset_end(rio_t* rio, ssize_t offs);

/// Returns true if the file read/write offset is currently at or beyond the
/// end of file position.
/// Throws an io exception if any failure was reported by the system. If an io
/// exception is thrown the file handle has an undefined read/write offset.
bool rio_is_eof(rio_t* rio);

/// Truncates a file to specified size of of precisely new_length bytes.
/// If the file previously was larger than this size, the extra data is lost.
/// If the file previously was shorter, it is extended, and the extended part
/// reads as null bytes ('\0'). The file offset is not changed.
/// The file must be opened as writable.
/// Throws an io exception if any failure was reported by the system.
void rio_file_truncate(rio_t* rio, size_t new_length);

/// Returns the size of an open file.
/// Throws an io exception if any failure was reported by the system. If an io
/// exception is thrown the file handle has an undefined read/write offset.
size_t rio_get_file_size(rio_t* rio);

/// Read the full content of a file.
fstr_mem_t* rio_read_file_contents(fstr_t file_path);

/// Read the full content of a virtual file.
fstr_t rio_read_virtual_file_contents(fstr_t file_path, fstr_t buffer);

/// Opens a channel for the specific signals. Accepts signals that are sent to
/// the whole thread group and/or signals sent to the specific thread.
/// See man 2 signalfd for more information.
rio_t* rio_signal_chan_open(size_t n_signals, int32_t* signals);

/// Returns the user id of the process. Never throws exceptions.
uint32_t rio_process_getuid();

/// Returns the group id of the process. Never throws exceptions.
uint32_t rio_process_getgid();

/// Sets the user id of the process. See setuid(2).
/// Throws an io exception if any failure was reported by the system.
void rio_process_setuid(uint32_t uid);

/// Sets the group id of the process. See setuid(2).
/// Throws an io exception if any failure was reported by the system.
void rio_process_setgid(uint32_t gid);

/// Sets the user/group id of the process. If id.uid or id.gid is 0 the set
/// will be skipped.
/// The uid/gid is set in gid, uid order. This order is important as setting
/// the uid usually drops permission of setting the gid. The operation is not
/// atomic. If the last setuid fails the process could be running with a
/// different gid.
/// Throws an io exception if any failure was reported by the system.
/// See setuid(2)/setgid(2) for more information.
void rio_process_setid(rio_id_t id);

/// Replaces the executable image by doing an execve(). Similar to
/// rio_proc_execute() but completely replaces the process image instead
/// of launching a subprocess. If the execve() fails this function will throw
/// a non-catchable fatal error since it corrupts the global process state by
/// modifying the file descriptor and signal table. The function does not
/// return. To prevent the execve from failing due to path or access errors
/// it is recommended to use rio_file_access() first to check if the path
/// is valid and executable.
void rio_process_execve(rio_exec_t e);

/// ** YOU PROBABLY WANT TO USE A HIGHER LEVEL FUNCTION THAN THIS, SEE
/// rio_proc_execute_and_wait() AND rio_proc_execute_and_pipe(). **
/// Wrapper for subprocess execution. See rio_exec_t for argument documentation.
/// Starts an internal avatar fiber for the subprocess that continously waits
/// for it and cleans it up with SIGKILL if it's free'd.
/// Returns a rio_type_subprocess type rio handle for the process which is not
/// possible to read or write from, only to wait for using
/// rio_proc_wait() and get child pid (rio_proc_get_pid) for
/// direct unix operations. To communicate with the process the specified
/// standard stream parameters are used which if set to 0 is inherited
/// to be the same as the parent process (not recommended except for stderr).
/// Throws io exception if clone() fails or execve() fails.
rio_proc_t* rio_proc_execute(rio_sub_exec_t se);

/// Returns the child process id of the subprocess. Only useful for libraries
/// that wish to perform direct linux interaction with the process.
/// Warning: Using the pid directly is prone to race conditions.
/// Ensure that the proc_h struct is not free'd while the fd is meddled with.
int32_t rio_proc_get_pid(rio_proc_t* proc_h) NO_NULL_ARGS;

/// Waits for a subprocess to exit and returns the status code of specified process.
int32_t rio_proc_wait(rio_proc_t* proc_h) NO_NULL_ARGS;

/// Sends a posix signal to a subprocess in a race free manner.
/// Include linux.h to get a list of signal definitions.
/// Throws an io exception if any failure was reported by the system including
/// if the subprocess no longer exists.
void rio_proc_signal(rio_proc_t* proc_h, int32_t posix_signal) NO_NULL_ARGS;

/// Executes a subprocess and waits for it to exit without performing any
/// communication. Stdin/out are replaced with /dev/null to prevent the
/// subprocess to mess up the general stdin/out and optionally stderr as
/// well if keep_stderr is set to false.
int32_t rio_proc_execute_and_wait(fstr_t path, list(fstr_t)* args, bool keep_stderr);

/// Subprocess execution that is as simple as normal script execution.
/// Shortcut for rio_proc_execute_and_wait() that also runs a rio_witch()
/// to determine the path of the unix name. In addition the function checks
/// the return code and throws an io exception if the return code is not zero.
/// See rio_which() for additional limitations.
void rio_exec(fstr_t unix_name, list(fstr_t)* args);

/// Like rio_exec() but runs command in a bash shell.
/// SECURITY WARNING: NEVER CALL THIS FUNCTION WITH UNTRUSTED INPUT.
/// See rio_which() for additional limitations.
void rio_shell(fstr_t command);

/// Executes a subprocess and communicates with it like rio_proc_execute_and_wait()
/// however, returns the rio_type_subprocess rio handle in out_rio_proc
/// and rio_type_pipe in out_rio_pipe so userspace can interact with the
/// process directly and either wait for normal stdin/out I/O or process exit.
/// The process is killed automatically with SIGKILL when the out_rio_proc
/// destructor runs to make sure that you store it safely until you're done.
void rio_proc_execute_and_pipe(fstr_t path, list(fstr_t)* args, bool keep_stderr, rio_proc_t** out_proc_h, rio_t** out_rio_pipe) NO_NULL_ARGS;

/// Looks in common unix binary locations for a binary with the specified name
/// that is executable and returns the full binary path. E.g. cat -> /bin/cat.
/// Throws an io exception if no such path exists.
/// This function is designed to be simple, robust and fast instead of faithful
/// to unix conventions. It uses a hard coded list of common binary locations
/// and ignores the environment. You should use a which function instead that
/// determines a location based on the current environment PATH variable
/// if you need a more generic and unix compatible which. The rio library
/// does not provide this.
fstr_mem_t* rio_which(fstr_t unix_name);

/// Overwrites the content of a file.
void rio_write_file_contents(fstr_t file_path, fstr_t data);

/// Fills the buffer and returns a slice of it that was filled. The returned
/// slice is always larger than zero. If the underlying file descriptor returns
/// zero it is assumed to be dead and an I/O exception is thrown as it cannot
/// satisfy the request to read more bytes.
fstr_t rio_read(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Like rio_read but also returns a hint that indicates whether more data will
/// be available for reading immediately. This allows the caller to make
/// optimizations that prevents overhead from consuming the data immediately.
/// If this is not known out_more_hint will be set to false.
/// If out_more_hint is 0 this function is equivalent to rio_read().
fstr_t rio_read_part(rio_t* rio, fstr_t buffer, bool* out_more_hint) NOT_NULL_ARGS(1);

/// Fills the given buffer by reading from rio stream. Does not return until
/// the entire buffer has been filled. Throws I/O exception if there is no
/// more bytes to read.
void rio_read_fill(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Reads until either the buffer or the stream ends. If an rio_eos io
/// exception is encountered while the stream is being read it's discarded
/// and the function returns whatever data was read.
/// Useful when the data source is trusted and the message has an undefined
/// length (e.g. when reading subprocess output).
/// Throws other io exceptions on read error.
fstr_t rio_read_to_end(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Skips over this many bytes in the rio stream. Can currently only skip over
/// the internal peek buffer and cannot directly skip I/O on underlying fd.
/// Currently throws an IO Exception if given a length larger than the buffer
/// returned by rio_peek() in the last call.
void rio_skip(rio_t* rio, size_t length) NO_NULL_ARGS;

/// Wait until [reading in rio does not block] if read is true or otherwise
/// [writing in rio does not block] and then return true.
/// If wait is false the function should return immediately return false
/// instead of waiting if the condition has not occurred yet.
/// This is significantly different from the epoll interface since it is not
/// directly querying the internal kernel buffer. Instead it queries the rio
/// handle, checking the rio buffers first. Abstract rio classes can also
/// implement their own form of polling that should avoid lying about whether
/// read/write blocks or not.
bool rio_poll(rio_t* rio, bool read, bool wait);

/// Polls a raw file descriptor. Polls the kernel directly in a way that has
/// minimal overhead if the event is already ready. Never use this for file
/// descriptors managed by rio as it does not check the internal rio struct
/// buffers. See rio_poll() for doc on arguments and return value as it
/// otherwise has the same behavior.
bool rio_poll_raw(int32_t target_fd, bool read, bool wait);

/// If no data exists in the internal peek buffer, tries to read as much data
/// as possible from the underlying file descriptor and fill it in one call
/// limited by the size of the internal peek buffer set by
/// rio_realloc_peek_buffer(). Otherwise just returns the existing data in
/// the peek buffer. To consume data in the peek buffer, either read it
/// using rio_read() or call rio_skip() to skip it without reading.
/// Throws an exception if a peek buffer has not been allocated for the
/// underlying rio.
fstr_t rio_peek(rio_t* rio) NO_NULL_ARGS;

/// Reads until a separator is reached in the stream. The rio stream must be
/// allocated with a peek buffer. Throws an io exception if the rio stream is
/// not allocated with a peek buffer. Throws an io exception if the separator
/// is not found after filling the entire max buffer. When using this function
/// it is crucial that a sufficiently large peek buffer is used, otherwise
/// the reading can be very inefficient.
fstr_t rio_read_to_separator(rio_t* rio, fstr_t separator, fstr_t max_buffer);

/// Writes a chunk of data to the rio handle, returning the tail slice of the
/// chunk that was not written because the underlying transport only accepted
/// a limited number of bytes without blocking at this time. Will block until
/// at least one byte has been written.
fstr_t rio_write_chunk(rio_t* rio, fstr_t chunk, bool more_hint) NO_NULL_ARGS;

/// Writes data to the rio handle. If the internal buffer of the fd is full
/// will block until the entire buffer has been written to the descriptor.
/// Returning only indicates that the data is in transit and not that it has
/// been received.
void rio_write(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Functionally equivalent to rio_write() except if more_hint is true it hints
/// to the caller that there will be immediate additional writes so it should
/// avoid the overhead of acting on the write immediately if this would reduce
/// efficiency of the transfer.
void rio_write_part(rio_t* rio, fstr_t buffer, bool more_hint) NO_NULL_ARGS;

/// Like rio_read_fstr() but with a maximum length.
/// If the slice of memory read is larger than max_len it throws an io exception.
/// If max_len is zero it has the same function as rio_read_fstr().
fstr_mem_t* rio_read_fstr_max(rio_t* rio, size_t max_len);

/// Reads variable length data with the encoding specified by rio_write_fstr().
fstr_mem_t* rio_read_fstr(rio_t* rio);

/// Life is to short to manually encode variable length data for streaming.
/// This function writes the length as a 64 bit network byte encoded
/// fixed string and then writes the actual fixed string.
void rio_write_fstr(rio_t* rio, fstr_t buffer);

/// Reads the next message from the rio handle using recv(2).
/// If the buffer is smaller than the received message the message is truncated to the buffer size.
fstr_t rio_msg_recv(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Writes the next message to the rio handle using send(2).
/// Returns the number of bytes written as a UDP datagram.
/// The returned length might be smaller than buffer.len if the UDP socket does
/// not support datagrams of that size.
size_t rio_msg_send(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Like rio_msg_send() but returns immediately with zero instead of waiting
/// if the underlying kernel buffer is full.
size_t rio_msg_try_send(rio_t* rio, fstr_t buffer) NO_NULL_ARGS;

/// Reads the next datagram from the specified udp rio handle.
/// If the buffer is smaller than the received message the message is truncated to the buffer size.
/// The remote address is returned on out_src_addr unless out_src_addr is null.
fstr_t rio_msg_recv_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t* out_src_addr) NOT_NULL_ARGS(1);

/// Writes a datagram to the specified udp rio handle.
/// Returns the number of bytes written as a UDP datagram.
/// The returned length might be smaller than buffer.len if the UDP socket does
/// not support datagrams of that size.
size_t rio_msg_send_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t dest_addr) NO_NULL_ARGS;

/// Like rio_msg_send_udp() but returns immediately with zero instead of waiting
/// if the underlying kernel buffer is full.
size_t rio_msg_try_send_udp(rio_t* rio, fstr_t buffer, rio_in_addr4_t dest_addr) NO_NULL_ARGS;

/// Creates a new TCP client that asynchronously attempts to connect to the
/// remote address. If connection fails the next read or write will fail.
/// Until the connection is established the next read or write will block.
rio_t* rio_tcp_client(rio_in_addr4_t remote_addr);

/// Creates a new TCP server that binds to the specified address and begins
/// listening for new connections.
rio_t* rio_tcp_server(rio_in_addr4_t bind_addr, int backlog);

/// Blocks until a client connects on the specified TCP server in which case
/// a new TCP client stream is created. The remote address is returned on
/// out_remote_addr unless out_remote_addr is null.
rio_t* rio_tcp_accept(rio_t* rio_tcp_server, rio_in_addr4_t* out_remote_addr) NOT_NULL_ARGS(1);

/// Configures TCP keep alive on the specified TCP client stream.
void rio_tcp_set_keepalive(rio_t* rio, rio_tcp_ka_t cfg);

/// Waits until the tcp connection phase is complete.
/// On successfully established connection the function returns true.
/// If connection was not successfully established the function returns
/// false. If out_so_error is not 0 it returns the error status from the
/// underlying socket SO_ERROR option on failure. It has no well defined
/// meaning to call this function after reading or writing to a tcp stream or
/// after the function has already returned a result for a tcp stream.
bool rio_tcp_conn_wait(rio_t* rio, int32_t* out_so_error);

/// Creates a UDP client socket that can be used to send messages with.
rio_t* rio_udp_client();

/// Creates a UDP server socket that binds to the specified address and
/// receives messages on it.
rio_t* rio_udp_server(rio_in_addr4_t* bind_addr) NO_NULL_ARGS;

/// Toggles broadcast on the specified UDP server/client.
/// Broadcast must be enabled to send or receive broadcast messages.
void rio_udp_toggle_broadcast(rio_t* rio, bool enable);

/// Returns the address a udp or tcp socket is bound to or if peer is true, the peer its connected to.
rio_in_addr4_t rio_get_socket_address(rio_t* rio, bool peer);

/// Binds a TCP or UDP client/server to a specific device.
void rio_bind_to_device(rio_t* rio, fstr_t dev_name);

/// Returns the current time in nanoseconds since an unspecified point in the
/// past (e.g. system startup). The system guarantees that this measurement is
/// monotonic and not affected by configuration, leap seconds etc.
/// Throws an io exception if reading the time value failed.
uint128_t rio_get_time_timer();

/// Returns the current time in nanoseconds since the Epoch. This value
/// can change unpredictably due to politics, setting system clock, leap
/// etc. It's probably not correct and should not be used for anything time
/// critical or interval measurement.
/// Throws an io exception if reading the time value failed.
uint128_t rio_get_time_clock();

/// Returns true if specified year is a leap year.
static inline bool rio_is_leap_year(size_t year) {
    return (!((year) % 4) && (((year) % 100) || !((year) % 400)));
}

/// Returns the number of days in the specified year.
static inline uint32_t rio_year_days(size_t year) {
    return (rio_is_leap_year(year)? 366: 365);
}

/// Returns the day of the year, begins at zero.
size_t rio_year_day(bool leap_year, size_t month, size_t month_day);

/// Converts a clock time to date time.
rio_date_time_t rio_epoch_to_date_time(uint128_t epoch_ns);

/// Converts a clock time to rfc3339 format. The n_sec_frac is the fixed
/// number of second fractions to include. If n_sec_frac is ceiled at 9.
fstr_mem_t* rio_epoch_to_rfc3339(uint128_t epoch_ns, size_t n_sec_frac);

/// Converts a clock time to rfc1123 format.
fstr_mem_t* rio_epoch_to_rfc1123(uint128_t epoch_ns);

/// Returns the days since past since year 0 at the beginning of year.
static inline size_t rio_days_year(size_t year) {
    return year * 365 + year / 4 - year / 100 + year / 400;
}

/// Converts timestamps in the rfc3339 format. Second fractions are
/// truncated to nanoseconds.
rio_clock_time_t rio_rfc3339_to_clock(fstr_t clock_str);

/// Converts clock_time to nanoseconds since epoch.
uint128_t rio_clock_time_deflate(rio_clock_time_t clock_time);
/// Inverse of rio_clock_time_deflate.
rio_clock_time_t rio_clock_time_inflate(uint128_t epoch_ns);

/// Creates a new timer I/O based on timer measurement. See rio_get_time_timer()
/// for more information.
rio_t* rio_timer_create();

/// Creates a new timer I/O based on clock measurement. See rio_get_time_clock()
/// for more information.
rio_t* rio_clock_create();

/// Arms the timer with the specified timeout and repeat interval relative to
/// the current time. If the repeat interval is zero the timer will only
/// timeout once.
void rio_alarm_set(rio_t* rio, uint128_t value_ns, bool value_absolute, uint128_t repeat_ns) NO_NULL_ARGS;

/// Waits for a timeout and returns the number of timeouts that occurred since
/// the last wait.
uint64_t rio_alarm_wait(rio_t* rio) NO_NULL_ARGS;

/// Uses rio timers to wait a specified time before continuing execution.
/// If wait_ns is zero the function will return immediately.
void rio_wait(uint128_t wait_ns);

/// Creates an event file descriptor. See eventfd(2).
rio_t* rio_eventfd_create(int64_t init_value, bool semaphore);

/// Triggers an event file descriptor. Blocks if the number of events would
/// cause an overflow. A maximum of 0xfffffffffffffffe events can be triggered.
/// Throws an io exception if any failure was reported by the system.
void rio_eventfd_trigger(rio_t* rio, uint64_t count);

/// Waits for events to be triggered for the specified eventfd.
/// Returns the number of events that was triggered.
/// Throws an io exception if any failure was reported by the system.
uint64_t rio_eventfd_wait(rio_t* rio);

/// Creates an event polling file descriptor that can be used to check and wait
/// (poll) for an os-level event to occur on the underlying file descriptor.
/// Note that the created epoll handle is associated with the kernel file
/// descriptor object so it will remain valid even if the specified rio_target
/// becomes invalid through reallocation, splitting or other forms of duplication.
rio_epoll_t* rio_epoll_create(rio_t* rio_target, rio_epoll_event_t epoll_event);

/// Alternative to rio_epoll_create() in situations when dealing with a
/// raw file descriptor managed by an external library (in global memory)
/// rather than an rio handle with automatic cleanup.
rio_epoll_t* rio_epoll_create_raw(int32_t target_fd, rio_epoll_event_t epoll_event);

/// Polls the specified epoll handle. If wait is true, waits indefinitely
/// until the event occurs, otherwise returns immediately indicating the event
/// ready status.
/// Note that it polls the kernel, not rio. To poll rio, use rio_poll which
/// uses an internal epoll if it has to query the kernel.
bool rio_epoll_poll(rio_epoll_t* epoll_h, bool wait);

/// Creates a new inotify handle.
rio_t* rio_inotify_create();

/// Opens a connected unix socket stream pair.
void rio_open_unix_socket_stream_pair(rio_t** out_socket_1, rio_t** out_socket_2) NO_NULL_ARGS;

/// Opens a connected unix socket datagram pair. Can be used to send messages
/// that have special functions in the kernel.
void rio_open_unix_socket_dgram_pair(rio_t** out_socket_1, rio_t** out_socket_2) NO_NULL_ARGS;

/// Connects to a unix socket at path.
rio_t* rio_open_unix_socket_client(fstr_t socket_path);

/// Receives a file descriptor on a unix datagram socket from another process.
/// Since this call deals with internal file descriptors you need to enter
/// information that rio is unaware about like file descriptor type, etc.
rio_t* rio_ipc_fd_recv(rio_t* unix_dgram_h, rio_type_t type, bool is_readable, bool is_writable) NO_NULL_ARGS;

/// Sends a file descriptor on a unix datagram socket. The specified rio handle
/// is automatically free'd if the send is successful.
void rio_ipc_fd_send(rio_t* unix_dgram_h, rio_t* rio) NO_NULL_ARGS;

/// Handles a main function injected by command line arguments.
void rio_ipc_main_injection_handler(list(fstr_t)* main_args);

/// Generates an argument list that injects a main function that can be
/// handled when the current program starts if it calls
/// rcd_ipc_main_injection_handler().
list(fstr_t)* rio_ipc_main_injection_get_args(void (*main_fn_ptr)());

/// Resolves a list of ipv4 addresses from the specified host name.
/// Uses /etc/hosts and dig internally for resolving.
/// The returned list is guaranteed to have more than zero elements.
/// Throws io exception for multiple reasons (unknown name, dig not installed
/// on system, etc).
list(uint32_t)* rio_resolve_host_ipv4_addr(fstr_t host_name);

#endif	/* RIO_H */
