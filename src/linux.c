/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"
#include "linux.h"

ssize_t read(int fd, void* buf, size_t count) {
    return (ssize_t) syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return (ssize_t) syscall(SYS_write, fd, buf, count);
}

/*int open(const char* pathname, int flags, mode_t mode) {
    return (int) syscall(SYS_open, pathname, flags, mode);
}*/

int close(int fd) {
    return (int) syscall(SYS_close, fd);
}

int stat(const char* path, struct stat* buf) {
    return (int) syscall(SYS_stat, path, buf);
}

int fstat(int fd, struct stat* buf) {
    return (int) syscall(SYS_fstat, fd, buf);
}

int lstat(const char* path, struct stat* buf) {
    return (int) syscall(SYS_lstat, path, buf);
}

int poll(struct pollfd* fds, nfds_t nfds, int timeout) {
    return (int) syscall(SYS_poll, fds, nfds, timeout);
}

off_t lseek(int fd, off_t offset, int whence) {
    return (off_t) syscall(SYS_lseek, fd, offset, whence);
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return (void*) syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

int mprotect(void* addr, size_t length, int prot) {
    return (int) syscall(SYS_mprotect, addr, length, prot);
}

int munmap(void* addr, size_t length) {
    return (int) syscall(SYS_munmap, addr, length);
}

int brk(void* addr) {
    return (int) syscall(SYS_brk, addr);
}

int rt_sigaction(int signum, const struct k_sigaction *act, struct k_sigaction *oldact) {
    return (int) syscall(SYS_rt_sigaction, signum, act, oldact, _NSIG/8);
}

int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    return (int) syscall(SYS_rt_sigprocmask, how, set, oldset, _NSIG/8);
}

int rt_sigreturn() {
    return (int) syscall(SYS_rt_sigreturn);
}

/*int ioctl(int d, int request, void* argp) {
    return (int) syscall(SYS_ioctl, d, request, argp);
}*/

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
    return (ssize_t) syscall(SYS_pread, fd, buf, count, offset);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
    return (ssize_t) syscall(SYS_pwrite, fd, buf, count, offset);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
    return (ssize_t) syscall(SYS_readv, fd, iov, iovcnt);
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
    return (ssize_t) syscall(SYS_writev, fd, iov, iovcnt);
}

int access(const char* pathname, int mode) {
    return (int) syscall(SYS_access, pathname, mode);
}

int pipe(int pipefd[2]) {
    // Redirect to pipe2() for CLOEXEC.
    return pipe2(pipefd, 0);
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) {
    return (int) syscall(SYS_select, nfds, readfds, writefds, exceptfds, timeout);
}

int sched_yield() {
    return (int) syscall(SYS_sched_yield);
}

void* mremap(void* old_address, size_t old_size, size_t new_size, int flags, void* new_address) {
    return (void*) syscall(SYS_mremap, old_address, old_size, new_size, flags, new_address);
}

int msync(void* addr, size_t length, int flags) {
    return (int) syscall(SYS_msync, addr, length, flags);
}

int mincore(void* addr, size_t length, unsigned char* vec) {
    return (int) syscall(SYS_mincore, addr, length, vec);
}

int madvise(void* addr, size_t length, int advice) {
    return (int) syscall(SYS_madvise, addr, length, advice);
}

int shmget(key_t key, size_t size, int shmflg) {
    return (int) syscall(SYS_shmget, key, size, shmflg);
}

void* shmat(int shmid, const void* shmaddr, int shmflg) {
    return (void*) syscall(SYS_shmat, shmid, shmaddr, shmflg);
}

int shmctl(int shmid, int cmd, struct shmid_ds* buf) {
    return (int) syscall(SYS_shmctl, shmid, cmd, buf);
}

int dup(int oldfd) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    // Need to do a dup() here with O_CLOEXEC - but flags are
    // only provided for dup2 (dup3). This is the UNIX facepalm workaround.
    return (int) syscall(SYS_fcntl, oldfd, F_DUPFD_CLOEXEC, 0);
}

int dup2(int oldfd, int newfd) {
    // Redirecting to dup3() to get CLOEXEC.
    return dup3(oldfd, newfd, 0);
}

int nanosleep(const struct timespec* req, struct timespec* rem) {
    return (int) syscall(SYS_nanosleep, req, rem);
}

pid_t getpid() {
    return (pid_t) syscall(SYS_getpid);
}

ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count) {
    return (ssize_t) syscall(SYS_sendfile, out_fd, in_fd, offset, count);
}

int socket(int domain, int type, int protocol) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_socket, domain, type | SOCK_CLOEXEC, protocol);
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return (int) syscall(SYS_connect, sockfd, addr, addrlen);
}

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
    return (int) syscall(SYS_sendto, sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
    return (ssize_t) syscall(SYS_recvfrom, sockfd, buf, len, flags, src_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags) {
    return (ssize_t) syscall(SYS_sendmsg, sockfd, msg, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return recvfrom(sockfd, buf, len, flags, 0, 0);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return sendto(sockfd, buf, len, flags, 0, 0);
}

ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags) {
    return (ssize_t) syscall(SYS_recvmsg, sockfd, msg, flags);
}

int shutdown(int sockfd, int how) {
    return (int) syscall(SYS_shutdown, sockfd, how);
}

int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return (int) syscall(SYS_bind, sockfd, addr, addrlen);
}

int listen(int sockfd, int backlog) {
    return (int) syscall(SYS_listen, sockfd, backlog);
}

int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return (int) syscall(SYS_getsockname, sockfd, addr, addrlen);
}

int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return (int) syscall(SYS_getpeername, sockfd, addr, addrlen);
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
    return (int) syscall(SYS_socketpair, domain, type, protocol, sv);
}

int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
    return (int) syscall(SYS_setsockopt, sockfd, level, optname, optval, optlen);
}

int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) {
    return (int) syscall(SYS_getsockopt, sockfd, level, optname, optval, optlen);
}

int execve(const char* filename, char* const argv[], char* const envp[]) {
    return (int) syscall(SYS_execve, filename, argv, envp);
}

noret void _exit(int status) {
    syscall(SYS_exit, status);
    __asm("hlt");
    unreachable();
}

int wait4(pid_t pid, int* status, int options, struct rusage* rusage) {
    return (int) syscall(SYS_wait4, pid, status, options, rusage);
}

int kill(pid_t pid, int sig) {
    return (int) syscall(SYS_kill, pid, sig);
}

int uname(struct utsname* buf) {
    return (int) syscall(SYS_uname, buf);
}

int shmdt(const void *shmaddr) {
    return (int) syscall(SYS_shmdt, shmaddr);
}

/*int fcntl(int fd, int cmd arg) {
    return (int) syscall(SYS_fcntl, fd, cmd, arg);
}*/

int flock(int fd, int operation) {
    return (int) syscall(SYS_flock, fd, operation);
}

int fsync(int fd) {
    return (int) syscall(SYS_fsync, fd);
}

int fdatasync(int fd) {
    return (int) syscall(SYS_fdatasync, fd);
}

int truncate(const char* path, off_t length) {
    return (int) syscall(SYS_truncate, path, length);
}

int ftruncate(int fd, off_t length) {
    return (int) syscall(SYS_ftruncate, fd, length);
}

int getdents(unsigned int fd, struct dirent* dirp, unsigned int count) {
    return (int) syscall(SYS_getdents, fd, dirp, count);
}

char* getcwd(char* buf, size_t size) {
    return (char*) syscall(SYS_getcwd, buf, size);
}

int chdir(const char* path) {
    return (int) syscall(SYS_chdir, path);
}

int fchdir(int fd) {
    return (int) syscall(SYS_fchdir, fd);
}

int rename(const char* oldpath, const char* newpath) {
    return (int) syscall(SYS_rename, oldpath, newpath);
}

int mkdir(const char* pathname, mode_t mode) {
    return (int) syscall(SYS_mkdir, pathname, mode);
}

int rmdir(const char* pathname) {
    return (int) syscall(SYS_rmdir, pathname);
}

/*int creat(const char* pathname, mode_t mode) {
    return (int) syscall(SYS_creat, pathname, mode);
}*/

int link(const char* oldpath, const char* newpath) {
    return (int) syscall(SYS_link, oldpath, newpath);
}

int unlink(const char* pathname) {
    return (int) syscall(SYS_unlink, pathname);
}

int symlink(const char* oldpath, const char* newpath) {
    return (int) syscall(SYS_symlink, oldpath, newpath);
}

ssize_t readlink(const char* path, char* buf, size_t bufsiz) {
    return (ssize_t) syscall(SYS_readlink, path, buf, bufsiz);
}

int chmod(const char* path, mode_t mode) {
    return (int) syscall(SYS_chmod, path, mode);
}

int fchmod(int fd, mode_t mode) {
    return (int) syscall(SYS_fchmod, fd, mode);
}

int chown(const char* path, uid_t owner, gid_t group) {
    return (int) syscall(SYS_chown, path, owner, group);
}

int fchown(int fd, uid_t owner, gid_t group) {
    return (int) syscall(SYS_fchown, fd, owner, group);
}

int lchown(const char* path, uid_t owner, gid_t group) {
    return (int) syscall(SYS_lchown, path, owner, group);
}

mode_t umask(mode_t mask) {
    return (mode_t) syscall(SYS_umask, mask);
}

int gettimeofday(struct timeval* tv, struct timezone* tz) {
    return (int) syscall(SYS_gettimeofday, tv, tz);
}

int getrlimit(int resource, struct rlimit* rlim) {
    return (int) syscall(SYS_getrlimit, resource, rlim);
}

int getrusage(int who, struct rusage* usage) {
    return (int) syscall(SYS_getrusage, who, usage);
}

int sysinfo(struct sysinfo* info) {
    return (int) syscall(SYS_sysinfo, info);
}

clock_t times(struct tms* buf) {
    return (clock_t) syscall(SYS_times, buf);
}

uid_t getuid() {
    return (uid_t) syscall(SYS_getuid);
}

gid_t getgid() {
    return (gid_t) syscall(SYS_getgid);
}

int setuid(uid_t uid) {
    return (int) syscall(SYS_setuid, uid);
}

int setgid(gid_t gid) {
    return (int) syscall(SYS_setgid, gid);
}

uid_t geteuid() {
    return (uid_t) syscall(SYS_geteuid);
}

gid_t getegid() {
    return (gid_t) syscall(SYS_getegid);
}

int setpgid(pid_t pid, pid_t pgid) {
    return (int) syscall(SYS_setpgid, pid, pgid);
}

pid_t getppid() {
    return (pid_t) syscall(SYS_getppid);
}

pid_t getpgrp() {
    return (pid_t) syscall(SYS_getpgrp);
}

pid_t setsid() {
    return (pid_t) syscall(SYS_setsid);
}

int setreuid(uid_t ruid, uid_t euid) {
    return (int) syscall(SYS_setreuid, ruid, euid);
}

int setregid(gid_t rgid, gid_t egid) {
    return (int) syscall(SYS_setregid, rgid, egid);
}

int getgroups(int size, gid_t list[]) {
    return (int) syscall(SYS_getgroups, size, list);
}

int setgroups(size_t size, const gid_t* list) {
    return (int) syscall(SYS_setgroups, size, list);
}

int setresuid(uid_t ruid, uid_t euid, uid_t suid) {
    return (int) syscall(SYS_setresuid, ruid, euid, suid);
}

int getresuid(uid_t* ruid, uid_t* euid, uid_t* suid) {
    return (int) syscall(SYS_getresuid, ruid, euid, suid);
}

int setresgid(gid_t rgid, gid_t egid, gid_t sgid) {
    return (int) syscall(SYS_setresgid, rgid, egid, sgid);
}

int getresgid(gid_t* rgid, gid_t* egid, gid_t* sgid) {
    return (int) syscall(SYS_getresgid, rgid, egid, sgid);
}

pid_t getpgid(pid_t pid) {
    return (pid_t) syscall(SYS_getpgid, pid);
}

int setfsuid(uid_t fsuid) {
    return (int) syscall(SYS_setfsuid, fsuid);
}

int setfsgid(uid_t fsgid) {
    return (int) syscall(SYS_setfsgid, fsgid);
}

pid_t getsid(pid_t pid) {
    return (pid_t) syscall(SYS_getsid, pid);
}

int rt_sigpending(sigset_t* set) {
    return (int) syscall(SYS_rt_sigpending, set);
}

int rt_sigtimedwait(const sigset_t* set, siginfo_t* info, const struct timespec* timeout) {
    return (int) syscall(SYS_rt_sigtimedwait, set, info, timeout, _NSIG/8);
}

int rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t* uinfo) {
    return (int) syscall(SYS_rt_sigqueueinfo, tgid, sig, uinfo);
}

int rt_sigsuspend(const sigset_t* mask) {
    return (int) syscall(SYS_rt_sigsuspend, mask);
}

int utime(const char* filename, const struct utimbuf* times) {
    return (int) syscall(SYS_utime, filename, times);
}

int mknod(const char* pathname, mode_t mode, dev_t dev) {
    return (int) syscall(SYS_mknod, pathname, mode, dev);
}

int uselib(const char* library) {
    return (int) syscall(SYS_uselib, library);
}

int personality(unsigned long persona) {
    return (int) syscall(SYS_personality, persona);
}

int ustat(dev_t dev, struct ustat* ubuf) {
    return (int) syscall(SYS_ustat, dev, ubuf);
}

int statfs(const char* path, struct statfs* buf) {
    return (int) syscall(SYS_statfs, path, buf);
}

int fstatfs(int fd, struct statfs* buf) {
    return (int) syscall(SYS_fstatfs, fd, buf);
}

int getpriority(int which, int who) {
    return (int) syscall(SYS_getpriority, which, who);
}

int setpriority(int which, int who, int prio) {
    return (int) syscall(SYS_setpriority, which, who, prio);
}

int sched_setparam(pid_t pid, const struct sched_param* param) {
    return (int) syscall(SYS_sched_setparam, pid, param);
}

int sched_getparam(pid_t pid, struct sched_param* param) {
    return (int) syscall(SYS_sched_getparam, pid, param);
}

int sched_setscheduler(pid_t pid, int policy, const struct sched_param* param) {
    return (int) syscall(SYS_sched_setscheduler, pid, policy, param);
}

int sched_getscheduler(pid_t pid) {
    return (int) syscall(SYS_sched_getscheduler, pid);
}

int sched_get_priority_max(int policy) {
    return (int) syscall(SYS_sched_get_priority_max, policy);
}

int sched_get_priority_min(int policy) {
    return (int) syscall(SYS_sched_get_priority_min, policy);
}

int sched_rr_get_interval(pid_t pid, struct timespec* tp) {
    return (int) syscall(SYS_sched_rr_get_interval, pid, tp);
}

int mlock(const void* addr, size_t len) {
    return (int) syscall(SYS_mlock, addr, len);
}

int munlock(const void* addr, size_t len) {
    return (int) syscall(SYS_munlock, addr, len);
}

int mlockall(int flags) {
    return (int) syscall(SYS_mlockall, flags);
}

int munlockall() {
    return (int) syscall(SYS_munlockall);
}

int vhangup() {
    return (int) syscall(SYS_vhangup);
}

int pivot_root(const char* new_root, const char* put_old) {
    return (int) syscall(SYS_pivot_root, new_root, put_old);
}

/*int prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) {
    return (int) syscall(SYS_prctl, option, arg2, arg3, arg4, arg5);
}*/

int arch_prctl(int code, unsigned long* addr) {
    return (int) syscall(SYS_arch_prctl, code, addr);
}

int setrlimit(int resource, const struct rlimit* rlim) {
    return (int) syscall(SYS_setrlimit, resource, rlim);
}

int chroot(const char* path) {
    return (int) syscall(SYS_chroot, path);
}

void sync() {
    return (void) syscall(SYS_sync);
}

int acct(const char* filename) {
    return (int) syscall(SYS_acct, filename);
}

int settimeofday(const struct timeval* tv, const struct timezone* tz) {
    return (int) syscall(SYS_settimeofday, tv, tz);
}

int mount(const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
    return (int) syscall(SYS_mount, source, target, filesystemtype, mountflags, data);
}

int umount2(__const char* __special_file, int __flags) {
    return (int) syscall(SYS_umount2, __special_file, __flags);
}

int swapon(const char* path, int swapflags) {
    return (int) syscall(SYS_swapon, path, swapflags);
}

int swapoff(const char* path) {
    return (int) syscall(SYS_swapoff, path);
}

int reboot(int magic, int magic2, int cmd, void* arg) {
    return (int) syscall(SYS_reboot, magic, magic2, cmd, arg);
}

int sethostname(const char* name, size_t len) {
    return (int) syscall(SYS_sethostname, name, len);
}

int setdomainname(const char* name, size_t len) {
    return (int) syscall(SYS_setdomainname, name, len);
}

int iopl(int level) {
    return (int) syscall(SYS_iopl, level);
}

pid_t gettid() {
    return (pid_t) syscall(SYS_gettid);
}

int setxattr(const char* path, const char* name, const void* value, size_t size, int flags) {
    return (int) syscall(SYS_setxattr, path, name, value, size, flags);
}

int lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags) {
    return (int) syscall(SYS_lsetxattr, path, name, value, size, flags);
}

int fsetxattr(int fd, const char* name, const void* value, size_t size, int flags) {
    return (int) syscall(SYS_fsetxattr, fd, name, value, size, flags);
}

ssize_t getxattr(const char* path, const char* name, void* value, size_t size) {
    return (ssize_t) syscall(SYS_getxattr, path, name, value, size);
}

ssize_t lgetxattr(const char* path, const char* name, void* value, size_t size) {
    return (ssize_t) syscall(SYS_lgetxattr, path, name, value, size);
}

ssize_t fgetxattr(int fd, const char* name, void* value, size_t size) {
    return (ssize_t) syscall(SYS_fgetxattr, fd, name, value, size);
}

ssize_t listxattr(const char* path, char* list, size_t size) {
    return (ssize_t) syscall(SYS_listxattr, path, list, size);
}

ssize_t llistxattr(const char* path, char* list, size_t size) {
    return (ssize_t) syscall(SYS_llistxattr, path, list, size);
}

ssize_t flistxattr(int fd, char* list, size_t size) {
    return (ssize_t) syscall(SYS_flistxattr, fd, list, size);
}

int removexattr(const char* path, const char* name) {
    return (int) syscall(SYS_removexattr, path, name);
}

int lremovexattr(const char* path, const char* name) {
    return (int) syscall(SYS_lremovexattr, path, name);
}

int fremovexattr(int fd, const char* name) {
    return (int) syscall(SYS_fremovexattr, fd, name);
}

int tkill(int tid, int sig) {
    return (int) syscall(SYS_tkill, tid, sig);
}

time_t time(time_t* t) {
    return (time_t) syscall(SYS_time, t);
}

int futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3) {
    return (int) syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}

int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
    return (int) syscall(SYS_sched_setaffinity, pid, cpusetsize, mask);
}

int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
    return (int) syscall(SYS_sched_getaffinity, pid, cpusetsize, mask);
}

int remap_file_pages(void* addr, size_t size, int prot, size_t pgoff, int flags) {
    return (int) syscall(SYS_remap_file_pages, addr, size, prot, pgoff, flags);
}

long set_tid_address(int* tidptr) {
    return (long) syscall(SYS_set_tid_address, tidptr);
}

long restart_syscall() {
    return (long) syscall(SYS_restart_syscall);
}

int fadvise64(int fd, off_t offset, off_t len, int advice) {
    return (int) syscall(SYS_fadvise64, fd, offset, len, advice);
}

int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid) {
    return (int) syscall(SYS_timer_create, clockid, sevp, timerid);
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec*  old_value) {
    return (int) syscall(SYS_timer_settime, timerid, flags, new_value, old_value);
}

int timer_gettime(timer_t timerid, struct itimerspec* curr_value) {
    return (int) syscall(SYS_timer_gettime, timerid, curr_value);
}

int timer_getoverrun(timer_t timerid) {
    return (int) syscall(SYS_timer_getoverrun, timerid);
}

int timer_delete(timer_t timerid) {
    return (int) syscall(SYS_timer_delete, timerid);
}

int clock_settime(clockid_t clk_id, const struct timespec* tp) {
    return (int) syscall(SYS_clock_settime, clk_id, tp);
}

int clock_gettime(clockid_t clk_id, struct timespec* tp) {
    return (int) syscall(SYS_clock_gettime, clk_id, tp);
}

int clock_getres(clockid_t clk_id, struct timespec* res) {
    return (int) syscall(SYS_clock_getres, clk_id, res);
}

int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec* request, struct timespec* remain) {
    return (int) syscall(SYS_clock_nanosleep, clock_id, flags, request, remain);
}

void exit_group(int status) {
    return (void) syscall(SYS_exit_group, status);
}

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout) {
    return (int) syscall(SYS_epoll_wait, epfd, events, maxevents, timeout);
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) {
    return (int) syscall(SYS_epoll_ctl, epfd, op, fd, event);
}

int tgkill(int tgid, int tid, int sig) {
    return (int) syscall(SYS_tgkill, tgid, tid, sig);
}

int utimes(const char* filename, const struct timeval times[2]) {
    return (int) syscall(SYS_utimes, filename, times);
}

int waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, struct rusage* rusage) {
    return (int) syscall(SYS_waitid, idtype, id, infop, options, rusage);
}

int ioprio_set(int which, int who, int ioprio) {
    return (int) syscall(SYS_ioprio_set, which, who, ioprio);
}

int ioprio_get(int which, int who) {
    return (int) syscall(SYS_ioprio_get, which, who);
}

int inotify_add_watch(int fd, const char* pathname, uint32_t mask) {
    return (int) syscall(SYS_inotify_add_watch, fd, pathname, mask);
}

int inotify_rm_watch(int fd, int wd) {
    return (int) syscall(SYS_inotify_rm_watch, fd, wd);
}

int mkdirat(int dirfd, const char* pathname, mode_t mode) {
    return (int) syscall(SYS_mkdirat, dirfd, pathname, mode);
}

int mknodat(int dirfd, const char* pathname, mode_t mode, dev_t dev) {
    return (int) syscall(SYS_mknodat, dirfd, pathname, mode, dev);
}

int fchownat(int dirfd, const char* pathname, uid_t owner, gid_t group, int flags) {
    return (int) syscall(SYS_fchownat, dirfd, pathname, owner, group, flags);
}

int futimesat(int dirfd, const char* pathname, const struct timeval times[2]) {
    return (int) syscall(SYS_futimesat, dirfd, pathname, times);
}

int fstatat(int dirfd, const char* pathname, struct stat* buf, int flags) {
    return (int) syscall(SYS_fstatat, dirfd, pathname, buf, flags);
}

int unlinkat(int dirfd, const char* pathname, int flags) {
    return (int) syscall(SYS_unlinkat, dirfd, pathname, flags);
}

int renameat(int olddirfd, const char* oldpath, int newdirfd, const char* newpath) {
    return (int) syscall(SYS_renameat, olddirfd, oldpath, newdirfd, newpath);
}

int linkat(int olddirfd, const char* oldpath, int newdirfd, const char* newpath, int flags) {
    return (int) syscall(SYS_linkat, olddirfd, oldpath, newdirfd, newpath, flags);
}

int symlinkat(const char* oldpath, int newdirfd, const char* newpath) {
    return (int) syscall(SYS_symlinkat, oldpath, newdirfd, newpath);
}

ssize_t readlinkat(int dirfd, const char* pathname, char* buf, size_t bufsiz) {
    return (ssize_t) syscall(SYS_readlinkat, dirfd, pathname, buf, bufsiz);
}

int fchmodat(int dirfd, const char* pathname, mode_t mode, int flags) {
    return (int) syscall(SYS_fchmodat, dirfd, pathname, mode, flags);
}

int faccessat(int dirfd, const char* pathname, int mode, int flags) {
    return (int) syscall(SYS_faccessat, dirfd, pathname, mode, flags);
}

int pselect6(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timespec* timeout, const sigset_t* sigmask) {
    long data[2] = {(long) sigmask, _NSIG/8};
    return (int) syscall(SYS_pselect6, nfds, readfds, writefds, exceptfds, timeout, data);
}

int ppoll(struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask) {
    return (int) syscall(SYS_ppoll, fds, nfds, timeout_ts, sigmask, _NSIG/8);
}

int unshare(int flags) {
    return (int) syscall(SYS_unshare, flags);
}

ssize_t splice(int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len, unsigned int flags) {
    return (ssize_t) syscall(SYS_splice, fd_in, off_in, fd_out, off_out, len, flags);
}

ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags) {
    return (ssize_t) syscall(SYS_tee, fd_in, fd_out, len, flags);
}

int sync_file_range(int fd, off64_t offset, off64_t nbytes, unsigned int flags) {
    return (int) syscall(SYS_sync_file_range, fd, offset, nbytes, flags);
}

ssize_t vmsplice(int fd, const struct iovec* iov, unsigned long nr_segs, unsigned int flags) {
    return (ssize_t) syscall(SYS_vmsplice, fd, iov, nr_segs, flags);
}

int utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags) {
    return (int) syscall(SYS_utimensat, dirfd, pathname, times, flags);
}

int epoll_pwait(int epfd, struct epoll_event* events, int maxevents, int timeout, const sigset_t* sigmask) {
    return (int) syscall(SYS_epoll_pwait, epfd, events, maxevents, timeout, sigmask);
}

int timerfd_create(int clockid, int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_timerfd_create, clockid, flags | TFD_CLOEXEC);
}

int fallocate(int fd, int mode, off_t offset, off_t len) {
    return (int) syscall(SYS_fallocate, fd, mode, offset, len);
}

int timerfd_settime(int fd, int flags, const struct itimerspec* new_value, struct itimerspec* old_value) {
    return (int) syscall(SYS_timerfd_settime, fd, flags, new_value, old_value);
}

int timerfd_gettime(int fd, struct itimerspec* curr_value) {
    return (int) syscall(SYS_timerfd_gettime, fd, curr_value);
}

int accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_accept4, sockfd, addr, addrlen, flags | SOCK_CLOEXEC);
}

int signalfd4(int ufd, const sigset_t* user_mask, int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_signalfd4, ufd, user_mask, _NSIG/8, flags | SFD_CLOEXEC);
}

int eventfd2(unsigned int initval, int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_eventfd2, initval, flags | EFD_CLOEXEC);
}

int epoll_create1(int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_epoll_create1, flags | EPOLL_CLOEXEC);
}

int dup3(int oldfd, int newfd, int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_dup3, oldfd, newfd, flags | O_CLOEXEC);
}

int pipe2(int pipefd[2], int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_pipe2, pipefd, flags | O_CLOEXEC);
}

int inotify_init1(int flags) {
    // This syscall creates a file descriptor. Force CLOEXEC to prevent fd leak.
    // We need to force this on this level as libraries usually don't specify this flag
    // and we can only do it here as doing it later creates a race.
    return (int) syscall(SYS_inotify_init1, flags | IN_CLOEXEC);
}

ssize_t preadv(int fd, const struct  iovec* iov, int iovcnt, off_t offset) {
    return (ssize_t) syscall(SYS_preadv, fd, iov, iovcnt, offset);
}

ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) {
    return (ssize_t) syscall(SYS_pwritev, fd, iov, iovcnt, offset);
}

int rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t* uinfo) {
    return (int) syscall(SYS_rt_tgsigqueueinfo, tgid, tid, sig, uinfo);
}

int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags, struct timespec *timeout) {
    return (int) syscall(SYS_recvmmsg, sockfd, msgvec, vlen, flags, timeout);
}

int syncfs(int fd) {
    return (int) syscall(SYS_syncfs, fd);
}

// Make sigprocmask alias for rt_sigprocmask.
int sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
__attribute__ ((weak, alias ("rt_sigprocmask")));

#pragma librcd

void linux_mount(fstr_t source, fstr_t mnt_path, fstr_t fs_type, uint64_t mountflags, fstr_t data) { sub_heap {
    try {
        char* source_cstr = fstr_to_cstr(source);
        char* mnt_path_cstr = fstr_to_cstr(mnt_path);
        char* fs_type_cstr = fstr_to_cstr(fs_type);
        char* data_cstr = data.len > 0? fstr_to_cstr(data): 0;
        int32_t mkdir_r = mkdir(mnt_path_cstr, 0755);
        if (mkdir_r == -1 && errno != EEXIST && errno != EROFS)
            RCD_SYSCALL_EXCEPTION(mkdir, exception_io);
        int32_t mount_r = mount(source_cstr, mnt_path_cstr, fs_type_cstr, mountflags, data_cstr);
        if (mount_r == -1)
            RCD_SYSCALL_EXCEPTION(mount, exception_io);
    } catch (exception_io, e) {
        throw_fwd(concs("mount failed: [", source, "] [", mnt_path, "] [", fs_type, "] [", ui2fs(mountflags), "] [", data, "]"), exception_io, e);
    }
}}

void _rcd_syscall_exception(fstr_t msg_start, rcd_exception_type_t type) { sub_heap {
    const char* errno_cstr = strerror(errno);
    fstr_t errno_str = errno_cstr != 0? fstr_fix_cstr(errno_cstr): fstr("");
    fstr_t msg = concs(msg_start, errno, fstr(" ("), errno_str, fstr(")"));
    if (type == exception_io) {
        emitosis(syscall, data) {
            data.errno_v = errno;
            throw_em(msg, data);
        }
    } else {
        throw(msg, type);
    }
}}
