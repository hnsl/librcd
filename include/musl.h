// Automatically generated from musl (http://www.musl-libc.org/) by the
// tools/musl-generator script. Do not edit.
//
// Copyright © 2005-2014 Rich Felker, et al.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef MUSL_H
#define MUSL_H

#undef _GNU_SOURCE

#define RCD_SYSCALL_EXCEPTION(name, rcd_exception_type) \
    sub_heap_e(throw(fss(conc(#name "() failed with errno ", fss(fstr_from_int(errno, 10)), " (", ({const char* errno_str = strerror(errno); errno_str != 0? fstr_fix_cstr(errno_str): "";}), ")")), rcd_exception_type))

typedef int wchar_t;
typedef int wint_t;
typedef unsigned long wctype_t;

typedef int8_t int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef int64_t int_fast64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef int pid_t;
typedef int id_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int key_t;

typedef rcd_fid_t pthread_t;
typedef lwt_once_t pthread_once_t;
typedef int pthread_key_t;
typedef int pthread_spinlock_t;

typedef struct { int32_t value; } pthread_attr_t;
typedef struct { int32_t value; } pthread_mutexattr_t;
typedef struct { int32_t value; } pthread_condattr_t;
typedef struct { int32_t value; } pthread_rwlockattr_t;
typedef struct { int32_t value; } pthread_barrierattr_t;

typedef struct { rcd_fid_t mutex_fid; pthread_once_t once_init; } pthread_mutex_t;
typedef struct { rcd_fid_t cond_fid; pthread_once_t once_init; } pthread_cond_t;
typedef struct { rcd_fid_t rwlock_fid; pthread_once_t once_init; } pthread_rwlock_t;
typedef struct { rcd_fid_t barrier_fid; pthread_once_t once_init; } pthread_barrier_t;

typedef long off_t;
typedef unsigned int mode_t;
typedef unsigned long nlink_t;
typedef unsigned long long ino_t;
typedef unsigned long dev_t;
typedef long blksize_t;
typedef long long blkcnt_t;
typedef unsigned long long fsblkcnt_t;
typedef unsigned long long fsfilcnt_t;
typedef void* timer_t;
typedef int clockid_t;
typedef long clock_t;

typedef struct {
    unsigned long __bits[128 / sizeof (long) ];
} sigset_t;

typedef unsigned int socklen_t;
typedef unsigned short sa_family_t;

typedef struct __FILE_s FILE;

typedef struct __locale* locale_t;

struct iovec {void* iov_base; size_t iov_len;};

typedef long time_t;
typedef long suseconds_t;

struct timeval { time_t tv_sec; long tv_usec; };
struct timespec { time_t tv_sec; long tv_nsec; };

#define __isoc_va_list va_list

#define pthread_cleanup_push(routine, arg) do { \
    void (*__cleanup_fn)(void *) = routine; \
    void *__cleanup_arg = arg; \
    try {

#define pthread_cleanup_pop(execute) \
    } catch (exception_any, e) { \
        __cleanup_fn(__cleanup_arg); \
        lwt_throw_exception(e); \
    } \
    if (execute) { \
        __cleanup_fn(__cleanup_arg); \
    } \
} while (false);

#define LOCK(int_ptr) __lock(int_ptr)
#define UNLOCK(int_ptr) __unlock(int_ptr)

#define PTHREAD_ONCE_INIT LWT_ONCE_INIT
#define PTHREAD_MUTEX_INITIALIZER {0}
#define PTHREAD_RWLOCK_INITIALIZER {0}
#define PTHREAD_COND_INITIALIZER {0}

struct sched_param {
    int sched_priority;
    int sched_ss_low_priority;
    struct timespec sched_ss_repl_period;
    struct timespec sched_ss_init_budget;
    int sched_ss_max_repl;
};
/// >>> START include/dirent.h
#ifndef    _DIRENT_H
#define    _DIRENT_H

/// >>> START include/features.h
#ifndef _FEATURES_H
#define _FEATURES_H

#ifdef _ALL_SOURCE
#define _GNU_SOURCE 1
#endif

#if !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE) \
 && !defined(_XOPEN_SOURCE) && !defined(_GNU_SOURCE) \
 && !defined(_BSD_SOURCE) && !defined(__STRICT_ANSI__)
#define _BSD_SOURCE 1
#define _XOPEN_SOURCE 700
#endif

#if __STDC_VERSION__ >= 199901L
#define __restrict restrict
#elif !defined(__GNUC__)
#define __restrict
#endif

#if __STDC_VERSION__ >= 199901L || defined(__cplusplus)
#define __inline inline
#endif

#define _Noreturn __attribute__((__noreturn__))

#endif

/// >>> CONTINUE include/dirent.h

#define __NEED_ino_t
#define __NEED_off_t
#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define __NEED_size_t
#endif

typedef struct __dirstream DIR;

struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};

#define d_fileno d_ino

int            closedir(DIR *);
DIR           *fdopendir(int);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *__restrict, struct dirent *__restrict, struct dirent **__restrict);
void           rewinddir(DIR *);
void           seekdir(DIR *, long);
long           telldir(DIR *);
int            dirfd(DIR *);

int alphasort(const struct dirent **, const struct dirent **);
int scandir(const char *, struct dirent ***, int (*)(const struct dirent *), int (*)(const struct dirent **, const struct dirent **));

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14
#define IFTODT(x) ((x)>>12 & 017)
#define DTTOIF(x) ((x)<<12)
#endif

#ifdef _GNU_SOURCE
int versionsort(const struct dirent **, const struct dirent **);
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define dirent64 dirent
#define readdir64 readdir
#define readdir64_r readdir_r
#define scandir64 scandir
#define alphasort64 alphasort
#define versionsort64 versionsort
#define off64_t off_t
#define ino64_t ino_t
#define getdents64 getdents
#endif

#endif

/// >>> START include/poll.h
#ifndef    _POLL_H
#define    _POLL_H

#define POLLIN     0x001
#define POLLPRI    0x002
#define POLLOUT    0x004
#define POLLERR    0x008
#define POLLHUP    0x010
#define POLLNVAL   0x020
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200
#define POLLMSG    0x400
#define POLLRDHUP  0x2000

typedef unsigned long nfds_t;

struct pollfd {
    int fd;
    short events;
    short revents;
};

int poll (struct pollfd *, nfds_t, int);

#ifdef _GNU_SOURCE
#define __NEED_time_t
#define __NEED_struct_timespec
#define __NEED_sigset_t
int ppoll(struct pollfd *, nfds_t, const struct timespec *, const sigset_t *);
#endif

#endif

/// >>> START include/pthread.h
#ifndef _PTHREAD_H
#define _PTHREAD_H

#define __NEED_time_t
#define __NEED_clockid_t
#define __NEED_struct_timespec
#define __NEED_sigset_t
#define __NEED_pthread_t
#define __NEED_pthread_attr_t
#define __NEED_pthread_mutexattr_t
#define __NEED_pthread_condattr_t
#define __NEED_pthread_rwlockattr_t
#define __NEED_pthread_barrierattr_t
#define __NEED_pthread_mutex_t
#define __NEED_pthread_cond_t
#define __NEED_pthread_rwlock_t
#define __NEED_pthread_barrier_t
#define __NEED_pthread_spinlock_t
#define __NEED_pthread_key_t
#define __NEED_pthread_once_t
#define __NEED_size_t

/// >>> START include/time.h
#ifndef    _TIME_H
#define _TIME_H

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#define __NEED_size_t
#define __NEED_time_t
#define __NEED_clock_t

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
#define __NEED_struct_timespec
#define __NEED_clockid_t
#define __NEED_timer_t
#define __NEED_pid_t
#define __NEED_locale_t
#endif

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define __tm_gmtoff tm_gmtoff
#define __tm_zone tm_zone
#endif

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long __tm_gmtoff;
    const char *__tm_zone;
};

clock_t clock (void);
time_t time (time_t *);
double difftime (time_t, time_t);
time_t mktime (struct tm *);
size_t strftime (char *__restrict, size_t, const char *__restrict, const struct tm *__restrict);
struct tm *gmtime (const time_t *);
struct tm *localtime (const time_t *);
char *asctime (const struct tm *);
char *ctime (const time_t *);

#define CLOCKS_PER_SEC 1000000L

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

size_t strftime_l (char *  __restrict, size_t, const char *  __restrict, const struct tm *  __restrict, locale_t);

struct tm *gmtime_r (const time_t *__restrict, struct tm *__restrict);
struct tm *localtime_r (const time_t *__restrict, struct tm *__restrict);
char *asctime_r (const struct tm *__restrict, char *__restrict);
char *ctime_r (const time_t *, char *);

void tzset (void);

struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};

#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID  3
#define CLOCK_MONOTONIC_RAW      4
#define CLOCK_REALTIME_COARSE    5
#define CLOCK_MONOTONIC_COARSE   6
#define CLOCK_BOOTTIME           7
#define CLOCK_REALTIME_ALARM     8
#define CLOCK_BOOTTIME_ALARM     9
#define CLOCK_SGI_CYCLE         10
#define CLOCK_TAI               11

#define TIMER_ABSTIME 1

int nanosleep (const struct timespec *, struct timespec *);
int clock_getres (clockid_t, struct timespec *);
int clock_gettime (clockid_t, struct timespec *);
int clock_settime (clockid_t, const struct timespec *);
int clock_nanosleep (clockid_t, int, const struct timespec *, struct timespec *);
int clock_getcpuclockid (pid_t, clockid_t *);

struct sigevent;
int timer_create (clockid_t, struct sigevent *__restrict, timer_t *__restrict);
int timer_delete (timer_t);
int timer_settime (timer_t, int, const struct itimerspec *__restrict, struct itimerspec *__restrict);
int timer_gettime (timer_t, struct itimerspec *);
int timer_getoverrun (timer_t);

#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE)
char *strptime (const char *__restrict, const char *__restrict, struct tm *__restrict);
extern int daylight;
extern long timezone;
extern char *tzname[2];
extern int getdate_err;
struct tm *getdate (const char *);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int stime(const time_t *);
time_t timegm(struct tm *);
#endif

#endif

/// >>> CONTINUE include/pthread.h

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2

#define PTHREAD_MUTEX_STALLED 0
#define PTHREAD_MUTEX_ROBUST 1

#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1

#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1

#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

#define PTHREAD_CANCELED ((void *)-1)

#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

int pthread_create(pthread_t *__restrict, const pthread_attr_t *__restrict, void *(*)(void *), void *__restrict);
int pthread_detach(pthread_t);
_Noreturn void pthread_exit(void *);
int pthread_join(pthread_t, void **);

#ifdef __GNUC__
__attribute__((const))
#endif
pthread_t pthread_self(void);

int pthread_equal(pthread_t, pthread_t);
#define pthread_equal(x,y) ((x)==(y))

int pthread_setcancelstate(int, int *);
int pthread_setcanceltype(int, int *);
void pthread_testcancel(void);
int pthread_cancel(pthread_t);

int pthread_getschedparam(pthread_t, int *__restrict, struct sched_param *__restrict);
int pthread_setschedparam(pthread_t, int, const struct sched_param *);
int pthread_setschedprio(pthread_t, int);

int pthread_once(pthread_once_t *, void (*)(void));

int pthread_mutex_init(pthread_mutex_t *__restrict, const pthread_mutexattr_t *__restrict);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);
int pthread_mutex_timedlock(pthread_mutex_t *__restrict, const struct timespec *__restrict);
int pthread_mutex_destroy(pthread_mutex_t *);
int pthread_mutex_consistent(pthread_mutex_t *);

int pthread_mutex_getprioceiling(const pthread_mutex_t *__restrict, int *__restrict);
int pthread_mutex_setprioceiling(pthread_mutex_t *__restrict, int, int *__restrict);

int pthread_cond_init(pthread_cond_t *__restrict, const pthread_condattr_t *__restrict);
int pthread_cond_destroy(pthread_cond_t *);
int pthread_cond_wait(pthread_cond_t *__restrict, pthread_mutex_t *__restrict);
int pthread_cond_timedwait(pthread_cond_t *__restrict, pthread_mutex_t *__restrict, const struct timespec *__restrict);
int pthread_cond_broadcast(pthread_cond_t *);
int pthread_cond_signal(pthread_cond_t *);

int pthread_rwlock_init(pthread_rwlock_t *__restrict, const pthread_rwlockattr_t *__restrict);
int pthread_rwlock_destroy(pthread_rwlock_t *);
int pthread_rwlock_rdlock(pthread_rwlock_t *);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *__restrict, const struct timespec *__restrict);
int pthread_rwlock_wrlock(pthread_rwlock_t *);
int pthread_rwlock_trywrlock(pthread_rwlock_t *);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *__restrict, const struct timespec *__restrict);
int pthread_rwlock_unlock(pthread_rwlock_t *);

int pthread_spin_init(pthread_spinlock_t *, int);
int pthread_spin_destroy(pthread_spinlock_t *);
int pthread_spin_lock(pthread_spinlock_t *);
int pthread_spin_trylock(pthread_spinlock_t *);
int pthread_spin_unlock(pthread_spinlock_t *);

int pthread_barrier_init(pthread_barrier_t *__restrict, const pthread_barrierattr_t *__restrict, unsigned);
int pthread_barrier_destroy(pthread_barrier_t *);
int pthread_barrier_wait(pthread_barrier_t *);

int pthread_key_create(pthread_key_t *, void (*)(void *));
int pthread_key_delete(pthread_key_t);
void *pthread_getspecific(pthread_key_t);
int pthread_setspecific(pthread_key_t, const void *);

int pthread_attr_init(pthread_attr_t *);
int pthread_attr_destroy(pthread_attr_t *);

int pthread_attr_getguardsize(const pthread_attr_t *__restrict, size_t *__restrict);
int pthread_attr_setguardsize(pthread_attr_t *, size_t);
int pthread_attr_getstacksize(const pthread_attr_t *__restrict, size_t *__restrict);
int pthread_attr_setstacksize(pthread_attr_t *, size_t);
int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
int pthread_attr_setdetachstate(pthread_attr_t *, int);
int pthread_attr_getstack(const pthread_attr_t *__restrict, void **__restrict, size_t *__restrict);
int pthread_attr_setstack(pthread_attr_t *, void *, size_t);
int pthread_attr_getscope(const pthread_attr_t *__restrict, int *__restrict);
int pthread_attr_setscope(pthread_attr_t *, int);
int pthread_attr_getschedpolicy(const pthread_attr_t *__restrict, int *__restrict);
int pthread_attr_setschedpolicy(pthread_attr_t *, int);
int pthread_attr_getschedparam(const pthread_attr_t *__restrict, struct sched_param *__restrict);
int pthread_attr_setschedparam(pthread_attr_t *__restrict, const struct sched_param *__restrict);
int pthread_attr_getinheritsched(const pthread_attr_t *__restrict, int *__restrict);
int pthread_attr_setinheritsched(pthread_attr_t *, int);

int pthread_mutexattr_destroy(pthread_mutexattr_t *);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *__restrict, int *__restrict);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *__restrict, int *__restrict);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *__restrict, int *__restrict);
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *__restrict, int *__restrict);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *__restrict, int *__restrict);
int pthread_mutexattr_init(pthread_mutexattr_t *);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
int pthread_mutexattr_setrobust(pthread_mutexattr_t *, int);
int pthread_mutexattr_settype(pthread_mutexattr_t *, int);

int pthread_condattr_init(pthread_condattr_t *);
int pthread_condattr_destroy(pthread_condattr_t *);
int pthread_condattr_setclock(pthread_condattr_t *, clockid_t);
int pthread_condattr_setpshared(pthread_condattr_t *, int);
int pthread_condattr_getclock(const pthread_condattr_t *__restrict, clockid_t *__restrict);
int pthread_condattr_getpshared(const pthread_condattr_t *__restrict, int *__restrict);

int pthread_rwlockattr_init(pthread_rwlockattr_t *);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *);
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *__restrict, int *__restrict);

int pthread_barrierattr_destroy(pthread_barrierattr_t *);
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *__restrict, int *__restrict);
int pthread_barrierattr_init(pthread_barrierattr_t *);
int pthread_barrierattr_setpshared(pthread_barrierattr_t *, int);

int pthread_atfork(void (*)(void), void (*)(void), void (*)(void));

int pthread_getconcurrency(void);
int pthread_setconcurrency(int);

int pthread_getcpuclockid(pthread_t, clockid_t *);

struct __ptcb {
    void (*__f)(void *);
    void *__x;
    struct __ptcb *next;
};

void _pthread_cleanup_push(struct __ptcb *, void (*)(void *), void *);
void _pthread_cleanup_pop(struct __ptcb *, int);

#ifdef _GNU_SOURCE
struct cpu_set_t;
int pthread_getaffinity_np(pthread_t, size_t, struct cpu_set_t *);
int pthread_setaffinity_np(pthread_t, size_t, const struct cpu_set_t *);
int pthread_getattr_np(pthread_t, pthread_attr_t *);
#endif

#endif

/// >>> START include/signal.h
#ifndef _SIGNAL_H
#define _SIGNAL_H

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

#ifdef _GNU_SOURCE
#define __ucontext ucontext
#endif

#define __NEED_size_t
#define __NEED_pid_t
#define __NEED_uid_t
#define __NEED_struct_timespec
#define __NEED_pthread_t
#define __NEED_pthread_attr_t
#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_sigset_t

#define SIG_HOLD ((void (*)(int)) 2)

#define SIG_BLOCK     0
#define SIG_UNBLOCK   1
#define SIG_SETMASK   2

#define SI_ASYNCNL (-60)
#define SI_TKILL (-6)
#define SI_SIGIO (-5)
#define SI_ASYNCIO (-4)
#define SI_MESGQ (-3)
#define SI_TIMER (-2)
#define SI_QUEUE (-1)
#define SI_USER 0
#define SI_KERNEL 128

#define FPE_INTDIV 1
#define FPE_INTOVF 2
#define FPE_FLTDIV 3
#define FPE_FLTOVF 4
#define FPE_FLTUND 5
#define FPE_FLTRES 6
#define FPE_FLTINV 7
#define FPE_FLTSUB 8

#define ILL_ILLOPC 1
#define ILL_ILLOPN 2
#define ILL_ILLADR 3
#define ILL_ILLTRP 4
#define ILL_PRVOPC 5
#define ILL_PRVREG 6
#define ILL_COPROC 7
#define ILL_BADSTK 8

#define SEGV_MAPERR 1
#define SEGV_ACCERR 2

#define BUS_ADRALN 1
#define BUS_ADRERR 2
#define BUS_OBJERR 3
#define BUS_MCEERR_AR 4
#define BUS_MCEERR_AO 5

#define CLD_EXITED 1
#define CLD_KILLED 2
#define CLD_DUMPED 3
#define CLD_TRAPPED 4
#define CLD_STOPPED 5
#define CLD_CONTINUED 6

typedef struct sigaltstack {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
} stack_t;

union sigval {
    int sival_int;
    void *sival_ptr;
};

typedef struct {
    int si_signo, si_errno, si_code;
    union {
        char __pad[128 - 2*sizeof(int) - sizeof(long)];
        struct {
            pid_t si_pid;
            uid_t si_uid;
            union sigval si_sigval;
        } __rt;
        struct {
            unsigned int si_timer1, si_timer2;
        } __timer;
        struct {
            pid_t si_pid;
            uid_t si_uid;
            int si_status;
            clock_t si_utime, si_stime;
        } __sigchld;
        struct {
            void *si_addr;
            short si_addr_lsb;
        } __sigfault;
        struct {
            long si_band;
            int si_fd;
        } __sigpoll;
        struct {
            void *si_call_addr;
            int si_syscall;
            unsigned si_arch;
        } __sigsys;
    } __si_fields;
} siginfo_t;

struct sigaction {
    union {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t *, void *);
    } __sa_handler;
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};
#define sa_handler   __sa_handler.sa_handler
#define sa_sigaction __sa_handler.sa_sigaction

struct sigevent {
    union sigval sigev_value;
    int sigev_signo;
    int sigev_notify;
    void (*sigev_notify_function)(union sigval);
    pthread_attr_t *sigev_notify_attributes;
    char __pad[56-3*sizeof(long)];
};

#define SIGEV_SIGNAL 0
#define SIGEV_NONE 1
#define SIGEV_THREAD 2

int __libc_current_sigrtmin(void);
int __libc_current_sigrtmax(void);

#define SIGRTMIN  (__libc_current_sigrtmin())
#define SIGRTMAX  (__libc_current_sigrtmax())

int kill(pid_t, int);

int sigemptyset(sigset_t *);
int sigfillset(sigset_t *);
int sigaddset(sigset_t *, int);
int sigdelset(sigset_t *, int);
int sigismember(const sigset_t *, int);

int sigprocmask(int, const sigset_t *__restrict, sigset_t *__restrict);
int sigsuspend(const sigset_t *);
int sigaction(int, const struct sigaction *__restrict, struct sigaction *__restrict);
int sigpending(sigset_t *);
int sigwait(const sigset_t *__restrict, int *__restrict);
int sigwaitinfo(const sigset_t *__restrict, siginfo_t *__restrict);
int sigtimedwait(const sigset_t *__restrict, siginfo_t *__restrict, const struct timespec *__restrict);
int sigqueue(pid_t, int, const union sigval);

int pthread_sigmask(int, const sigset_t *__restrict, sigset_t *__restrict);
int pthread_kill(pthread_t, int);

void psiginfo(const siginfo_t *, const char *);
void psignal(int, const char *);

#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE)
int killpg(pid_t, int);
int sigaltstack(const stack_t *__restrict, stack_t *__restrict);
int sighold(int);
int sigignore(int);
int siginterrupt(int, int);
int sigpause(int);
int sigrelse(int);
void (*sigset(int, void (*)(int)))(int);
#define TRAP_BRKPT 1
#define TRAP_TRACE 2
#define POLL_IN 1
#define POLL_OUT 2
#define POLL_MSG 3
#define POLL_ERR 4
#define POLL_PRI 5
#define POLL_HUP 6
#define SS_ONSTACK    1
#define SS_DISABLE    2
#define MINSIGSTKSZ 2048
#define SIGSTKSZ 8192
#endif

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define NSIG _NSIG
typedef void (*sig_t)(int);
#endif

#ifdef _GNU_SOURCE
void (*bsd_signal(int, void (*)(int)))(int);
int sigisemptyset(const sigset_t *);
int sigorset (sigset_t *, const sigset_t *, const sigset_t *);
int sigandset(sigset_t *, const sigset_t *, const sigset_t *);

#define SA_NOMASK SA_NODEFER
#define SA_ONESHOT SA_RESETHAND
#endif

/// >>> START arch/x86_64/bits/signal.h
#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#ifdef _GNU_SOURCE
#define REG_R8          0
#define REG_R9          1
#define REG_R10         2
#define REG_R11         3
#define REG_R12         4
#define REG_R13         5
#define REG_R14         6
#define REG_R15         7
#define REG_RDI         8
#define REG_RSI         9
#define REG_RBP         10
#define REG_RBX         11
#define REG_RDX         12
#define REG_RAX         13
#define REG_RCX         14
#define REG_RSP         15
#define REG_RIP         16
#define REG_EFL         17
#define REG_CSGSFS      18
#define REG_ERR         19
#define REG_TRAPNO      20
#define REG_OLDMASK     21
#define REG_CR2         22
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
typedef long long greg_t, gregset_t[23];
typedef struct _fpstate {
    unsigned short cwd, swd, ftw, fop;
    unsigned long long rip, rdp;
    unsigned mxcsr, mxcr_mask;
    struct {
        unsigned short significand[4], exponent, padding[3];
    } _st[8];
    struct {
        unsigned element[4];
    } _xmm[16];
    unsigned padding[24];
} *fpregset_t;
struct sigcontext {
    unsigned long r8, r9, r10, r11, r12, r13, r14, r15;
    unsigned long rdi, rsi, rbp, rbx, rdx, rax, rcx, rsp, rip, eflags;
    unsigned short cs, gs, fs, __pad0;
    unsigned long err, trapno, oldmask, cr2;
    struct _fpstate *fpstate;
    unsigned long __reserved1[8];
};
typedef struct {
    gregset_t gregs;
    fpregset_t fpregs;
    unsigned long long __reserved1[8];
} mcontext_t;
#else
typedef struct {
    unsigned long __space[32];
} mcontext_t;
#endif

typedef struct __ucontext {
    unsigned long uc_flags;
    struct __ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    unsigned long __fpregs_mem[64];
} ucontext_t;

#define SA_NOCLDSTOP  1
#define SA_NOCLDWAIT  2
#define SA_SIGINFO    4
#define SA_ONSTACK    0x08000000
#define SA_RESTART    0x10000000
#define SA_NODEFER    0x40000000
#define SA_RESETHAND  0x80000000
#define SA_RESTORER   0x04000000

#endif

#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGABRT   6
#define SIGIOT    SIGABRT
#define SIGBUS    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGIO     29
#define SIGPOLL   29
#define SIGPWR    30
#define SIGSYS    31
#define SIGUNUSED SIGSYS

#define _NSIG 65

/// >>> CONTINUE include/signal.h

#define SIG_ERR  ((void (*)(int))-1)
#define SIG_DFL  ((void (*)(int)) 0)
#define SIG_IGN  ((void (*)(int)) 1)

typedef int sig_atomic_t;

void (*signal(int, void (*)(int)))(int);
int raise(int);

#endif

/// >>> START include/utime.h
#ifndef    _UTIME_H
#define    _UTIME_H

#define __NEED_time_t

struct utimbuf {
    time_t actime;
    time_t modtime;
};

int utime (const char *, const struct utimbuf *);

#endif

/// >>> START include/utmp.h
#ifndef _UTMP_H
#define _UTMP_H

/// >>> START include/utmpx.h
#ifndef _UTMPX_H
#define _UTMPX_H

#define __NEED_pid_t
#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval

struct utmpx {
    short ut_type;
    pid_t ut_pid;
    char ut_line[32];
    char ut_id[4];
    char ut_user[32];
    char ut_host[256];
    struct {
        short __e_termination;
        short __e_exit;
    } ut_exit;
    long ut_session;
    struct timeval ut_tv;
    unsigned ut_addr_v6[4];
    char __unused[20];
};

void          endutxent(void);
struct utmpx *getutxent(void);
struct utmpx *getutxid(const struct utmpx *);
struct utmpx *getutxline(const struct utmpx *);
struct utmpx *pututxline(const struct utmpx *);
void          setutxent(void);

#if defined(_BSD_SOURCE) | defined(_GNU_SOURCE)
#define e_exit __e_exit
#define e_termination __e_termination
void updwtmpx(const char *, const struct utmpx *);
#endif

#define EMPTY           0
#define RUN_LVL         1
#define BOOT_TIME       2
#define NEW_TIME        3
#define OLD_TIME        4
#define INIT_PROCESS    5
#define LOGIN_PROCESS   6
#define USER_PROCESS    7
#define DEAD_PROCESS    8

#endif

/// >>> CONTINUE include/utmp.h

#define ACCOUNTING 9
#define UT_NAMESIZE 32
#define UT_HOSTSIZE 256
#define UT_LINESIZE 32

struct lastlog {
    time_t ll_time;
    char ll_line[UT_LINESIZE];
    char ll_host[UT_HOSTSIZE];
};

#define ut_time ut_tv.tv_sec
#define ut_name ut_user
#define ut_addr ut_addr_v6[0]
#define utmp utmpx
#define utmpname(x) (-1)
#define e_exit __e_exit
#define e_termination __e_termination

void         endutent(void);
struct utmp *getutent(void);
struct utmp *getutid(const struct utmp *);
struct utmp *getutline(const struct utmp *);
struct utmp *pututline(const struct utmp *);
void         setutent(void);

void updwtmp(const char *, const struct utmp *);

#define _PATH_UTMP "/dev/null/utmp"
#define _PATH_WTMP "/dev/null/wtmp"

#define UTMP_FILE _PATH_UTMP
#define WTMP_FILE _PATH_WTMP
#define UTMP_FILENAME _PATH_UTMP
#define WTMP_FILENAME _PATH_WTMP

#endif

/// >>> START include/arpa/inet.h
#ifndef _ARPA_INET_H
#define    _ARPA_INET_H

/// >>> START include/netinet/in.h
#ifndef    _NETINET_IN_H
#define    _NETINET_IN_H

/// >>> START include/inttypes.h
#ifndef _INTTYPES_H
#define _INTTYPES_H

#define __NEED_wchar_t

typedef struct { intmax_t quot, rem; } imaxdiv_t;

intmax_t imaxabs(intmax_t);
imaxdiv_t imaxdiv(intmax_t, intmax_t);

intmax_t strtoimax(const char *__restrict, char **__restrict, int);
uintmax_t strtoumax(const char *__restrict, char **__restrict, int);

intmax_t wcstoimax(const wchar_t *__restrict, wchar_t **__restrict, int);
uintmax_t wcstoumax(const wchar_t *__restrict, wchar_t **__restrict, int);

#if UINTPTR_MAX == UINT64_MAX
#define __PRI64  "l"
#define __PRIPTR "l"
#else
#define __PRI64  "ll"
#define __PRIPTR ""
#endif

#define PRId8  "d"
#define PRId16 "d"
#define PRId32 "d"
#define PRId64 __PRI64 "d"

#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 __PRI64 "d"

#define PRIdFAST8  "d"
#define PRIdFAST16 "d"
#define PRIdFAST32 "d"
#define PRIdFAST64 __PRI64 "d"

#define PRIi8  "i"
#define PRIi16 "i"
#define PRIi32 "i"
#define PRIi64 __PRI64 "i"

#define PRIiLEAST8  "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 __PRI64 "i"

#define PRIiFAST8  "i"
#define PRIiFAST16 "i"
#define PRIiFAST32 "i"
#define PRIiFAST64 __PRI64 "i"

#define PRIo8  "o"
#define PRIo16 "o"
#define PRIo32 "o"
#define PRIo64 __PRI64 "o"

#define PRIoLEAST8  "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 __PRI64 "o"

#define PRIoFAST8  "o"
#define PRIoFAST16 "o"
#define PRIoFAST32 "o"
#define PRIoFAST64 __PRI64 "o"

#define PRIu8  "u"
#define PRIu16 "u"
#define PRIu32 "u"
#define PRIu64 __PRI64 "u"

#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 __PRI64 "u"

#define PRIuFAST8  "u"
#define PRIuFAST16 "u"
#define PRIuFAST32 "u"
#define PRIuFAST64 __PRI64 "u"

#define PRIx8  "x"
#define PRIx16 "x"
#define PRIx32 "x"
#define PRIx64 __PRI64 "x"

#define PRIxLEAST8  "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 __PRI64 "x"

#define PRIxFAST8  "x"
#define PRIxFAST16 "x"
#define PRIxFAST32 "x"
#define PRIxFAST64 __PRI64 "x"

#define PRIX8  "X"
#define PRIX16 "X"
#define PRIX32 "X"
#define PRIX64 __PRI64 "X"

#define PRIXLEAST8  "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 __PRI64 "X"

#define PRIXFAST8  "X"
#define PRIXFAST16 "X"
#define PRIXFAST32 "X"
#define PRIXFAST64 __PRI64 "X"

#define PRIdMAX __PRI64 "d"
#define PRIiMAX __PRI64 "i"
#define PRIoMAX __PRI64 "o"
#define PRIuMAX __PRI64 "u"
#define PRIxMAX __PRI64 "x"
#define PRIXMAX __PRI64 "X"

#define PRIdPTR __PRIPTR "d"
#define PRIiPTR __PRIPTR "i"
#define PRIoPTR __PRIPTR "o"
#define PRIuPTR __PRIPTR "u"
#define PRIxPTR __PRIPTR "x"
#define PRIXPTR __PRIPTR "X"

#define SCNd8   "hhd"
#define SCNd16  "hd"
#define SCNd32  "d"
#define SCNd64  __PRI64 "d"

#define SCNdLEAST8  "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 __PRI64 "d"

#define SCNdFAST8  "hhd"
#define SCNdFAST16 "d"
#define SCNdFAST32 "d"
#define SCNdFAST64 __PRI64 "d"

#define SCNi8   "hhi"
#define SCNi16  "hi"
#define SCNi32  "i"
#define SCNi64  __PRI64 "i"

#define SCNiLEAST8  "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 __PRI64 "i"

#define SCNiFAST8  "hhi"
#define SCNiFAST16 "i"
#define SCNiFAST32 "i"
#define SCNiFAST64 __PRI64 "i"

#define SCNu8   "hhu"
#define SCNu16  "hu"
#define SCNu32  "u"
#define SCNu64  __PRI64 "u"

#define SCNuLEAST8  "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 __PRI64 "u"

#define SCNuFAST8 "hhu"
#define SCNuFAST16 "u"
#define SCNuFAST32 "u"
#define SCNuFAST64 __PRI64 "u"

#define SCNo8   "hho"
#define SCNo16  "ho"
#define SCNo32  "o"
#define SCNo64  __PRI64 "o"

#define SCNoLEAST8  "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 __PRI64 "o"

#define SCNoFAST8  "hho"
#define SCNoFAST16 "o"
#define SCNoFAST32 "o"
#define SCNoFAST64 __PRI64 "o"

#define SCNx8   "hhx"
#define SCNx16  "hx"
#define SCNx32  "x"
#define SCNx64  __PRI64 "x"

#define SCNxLEAST8  "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 __PRI64 "x"

#define SCNxFAST8  "hhx"
#define SCNxFAST16 "x"
#define SCNxFAST32 "x"
#define SCNxFAST64 __PRI64 "x"

#define SCNdMAX __PRI64 "d"
#define SCNiMAX __PRI64 "i"
#define SCNoMAX __PRI64 "o"
#define SCNuMAX __PRI64 "u"
#define SCNxMAX __PRI64 "x"

#define SCNdPTR __PRIPTR "d"
#define SCNiPTR __PRIPTR "i"
#define SCNoPTR __PRIPTR "o"
#define SCNuPTR __PRIPTR "u"
#define SCNxPTR __PRIPTR "x"

#endif

/// >>> CONTINUE include/netinet/in.h
/// >>> START include/sys/socket.h
#ifndef    _SYS_SOCKET_H
#define    _SYS_SOCKET_H

#define __NEED_socklen_t
#define __NEED_sa_family_t
#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_uid_t
#define __NEED_pid_t
#define __NEED_gid_t
#define __NEED_struct_iovec

/// >>> START arch/x86_64/bits/socket.h
struct msghdr {
    void *msg_name;
    socklen_t msg_namelen;
    struct iovec *msg_iov;
    int msg_iovlen, __pad1;
    void *msg_control;
    socklen_t msg_controllen, __pad2;
    int msg_flags;
};

struct cmsghdr {
    socklen_t cmsg_len;
    int __pad1;
    int cmsg_level;
    int cmsg_type;
};

/// >>> CONTINUE include/sys/socket.h

#ifdef _GNU_SOURCE
struct ucred {
    pid_t pid;
    uid_t uid;
    gid_t gid;
};
#endif

struct linger {
    int l_onoff;
    int l_linger;
};

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#ifndef SOCK_STREAM
#define SOCK_STREAM    1
#define SOCK_DGRAM     2
#endif

#define SOCK_RAW       3
#define SOCK_RDM       4
#define SOCK_SEQPACKET 5
#define SOCK_DCCP      6
#define SOCK_PACKET    10

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC   02000000
#define SOCK_NONBLOCK  04000
#endif

#define PF_UNSPEC       0
#define PF_LOCAL        1
#define PF_UNIX         PF_LOCAL
#define PF_FILE         PF_LOCAL
#define PF_INET         2
#define PF_AX25         3
#define PF_IPX          4
#define PF_APPLETALK    5
#define PF_NETROM       6
#define PF_BRIDGE       7
#define PF_ATMPVC       8
#define PF_X25          9
#define PF_INET6        10
#define PF_ROSE         11
#define PF_DECnet       12
#define PF_NETBEUI      13
#define PF_SECURITY     14
#define PF_KEY          15
#define PF_NETLINK      16
#define PF_ROUTE        PF_NETLINK
#define PF_PACKET       17
#define PF_ASH          18
#define PF_ECONET       19
#define PF_ATMSVC       20
#define PF_RDS          21
#define PF_SNA          22
#define PF_IRDA         23
#define PF_PPPOX        24
#define PF_WANPIPE      25
#define PF_LLC          26
#define PF_IB           27
#define PF_CAN          29
#define PF_TIPC         30
#define PF_BLUETOOTH    31
#define PF_IUCV         32
#define PF_RXRPC        33
#define PF_ISDN         34
#define PF_PHONET       35
#define PF_IEEE802154   36
#define PF_CAIF         37
#define PF_ALG          38
#define PF_NFC          39
#define PF_VSOCK        40
#define PF_MAX          41

#define AF_UNSPEC       PF_UNSPEC
#define AF_LOCAL        PF_LOCAL
#define AF_UNIX         AF_LOCAL
#define AF_FILE         AF_LOCAL
#define AF_INET         PF_INET
#define AF_AX25         PF_AX25
#define AF_IPX          PF_IPX
#define AF_APPLETALK    PF_APPLETALK
#define AF_NETROM       PF_NETROM
#define AF_BRIDGE       PF_BRIDGE
#define AF_ATMPVC       PF_ATMPVC
#define AF_X25          PF_X25
#define AF_INET6        PF_INET6
#define AF_ROSE         PF_ROSE
#define AF_DECnet       PF_DECnet
#define AF_NETBEUI      PF_NETBEUI
#define AF_SECURITY     PF_SECURITY
#define AF_KEY          PF_KEY
#define AF_NETLINK      PF_NETLINK
#define AF_ROUTE        PF_ROUTE
#define AF_PACKET       PF_PACKET
#define AF_ASH          PF_ASH
#define AF_ECONET       PF_ECONET
#define AF_ATMSVC       PF_ATMSVC
#define AF_RDS          PF_RDS
#define AF_SNA          PF_SNA
#define AF_IRDA         PF_IRDA
#define AF_PPPOX        PF_PPPOX
#define AF_WANPIPE      PF_WANPIPE
#define AF_LLC          PF_LLC
#define AF_IB           PF_IB
#define AF_CAN          PF_CAN
#define AF_TIPC         PF_TIPC
#define AF_BLUETOOTH    PF_BLUETOOTH
#define AF_IUCV         PF_IUCV
#define AF_RXRPC        PF_RXRPC
#define AF_ISDN         PF_ISDN
#define AF_PHONET       PF_PHONET
#define AF_IEEE802154   PF_IEEE802154
#define AF_CAIF         PF_CAIF
#define AF_ALG          PF_ALG
#define AF_NFC          PF_NFC
#define AF_VSOCK        PF_VSOCK
#define AF_MAX          PF_MAX

#ifndef SO_DEBUG
#define SO_DEBUG        1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_DONTROUTE    5
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_KEEPALIVE    9
#define SO_OOBINLINE    10
#define SO_NO_CHECK     11
#define SO_PRIORITY     12
#define SO_LINGER       13
#define SO_BSDCOMPAT    14
#define SO_REUSEPORT    15
#define SO_PASSCRED     16
#define SO_PEERCRED     17
#define SO_RCVLOWAT     18
#define SO_SNDLOWAT     19
#define SO_RCVTIMEO     20
#define SO_SNDTIMEO     21
#define SO_SNDBUFFORCE  32
#define SO_RCVBUFFORCE  33
#endif

#define SO_SECURITY_AUTHENTICATION              22
#define SO_SECURITY_ENCRYPTION_TRANSPORT        23
#define SO_SECURITY_ENCRYPTION_NETWORK          24

#define SO_BINDTODEVICE 25

#define SO_ATTACH_FILTER        26
#define SO_DETACH_FILTER        27
#define SO_GET_FILTER           SO_ATTACH_FILTER

#define SO_PEERNAME             28
#define SO_TIMESTAMP            29
#define SCM_TIMESTAMP           SO_TIMESTAMP

#define SO_ACCEPTCONN           30
#define SO_PEERSEC              31
#define SO_PASSSEC              34
#define SO_TIMESTAMPNS          35
#define SCM_TIMESTAMPNS         SO_TIMESTAMPNS
#define SO_MARK                 36
#define SO_TIMESTAMPING         37
#define SCM_TIMESTAMPING        SO_TIMESTAMPING
#define SO_PROTOCOL             38
#define SO_DOMAIN               39
#define SO_RXQ_OVFL             40
#define SO_WIFI_STATUS          41
#define SCM_WIFI_STATUS         SO_WIFI_STATUS
#define SO_PEEK_OFF             42
#define SO_NOFCS                43
#define SO_LOCK_FILTER          44
#define SO_SELECT_ERR_QUEUE     45
#define SO_BUSY_POLL            46

#ifndef SOL_SOCKET
#define SOL_SOCKET      1
#endif

#define SOL_IP          0
#define SOL_IPV6        41
#define SOL_ICMPV6      58

#define SOL_RAW         255
#define SOL_DECNET      261
#define SOL_X25         262
#define SOL_PACKET      263
#define SOL_ATM         264
#define SOL_AAL         265
#define SOL_IRDA        266

#define SOMAXCONN       128

#define MSG_OOB       0x0001
#define MSG_PEEK      0x0002
#define MSG_DONTROUTE 0x0004
#define MSG_CTRUNC    0x0008
#define MSG_PROXY     0x0010
#define MSG_TRUNC     0x0020
#define MSG_DONTWAIT  0x0040
#define MSG_EOR       0x0080
#define MSG_WAITALL   0x0100
#define MSG_FIN       0x0200
#define MSG_SYN       0x0400
#define MSG_CONFIRM   0x0800
#define MSG_RST       0x1000
#define MSG_ERRQUEUE  0x2000
#define MSG_NOSIGNAL  0x4000
#define MSG_MORE      0x8000
#define MSG_WAITFORONE 0x10000
#define MSG_CMSG_CLOEXEC 0x40000000

#define __CMSG_LEN(cmsg) (((cmsg)->cmsg_len + sizeof(long) - 1) & ~(long)(sizeof(long) - 1))
#define __CMSG_NEXT(cmsg) ((unsigned char *)(cmsg) + __CMSG_LEN(cmsg))
#define __MHDR_END(mhdr) ((unsigned char *)(mhdr)->msg_control + (mhdr)->msg_controllen)

#define CMSG_DATA(cmsg) ((unsigned char *) (((struct cmsghdr *)(cmsg)) + 1))
#define CMSG_NXTHDR(mhdr, cmsg) ((cmsg)->cmsg_len < sizeof (struct cmsghdr) ? (struct cmsghdr *)0 : \
        (__CMSG_NEXT(cmsg) + sizeof (struct cmsghdr) >= __MHDR_END(mhdr) ? (struct cmsghdr *)0 : \
        ((struct cmsghdr *)__CMSG_NEXT(cmsg))))
#define CMSG_FIRSTHDR(mhdr) ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr) ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) 0)

#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

#define SCM_RIGHTS      0x01
#define SCM_CREDENTIALS 0x02

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};

struct sockaddr_storage {
    sa_family_t ss_family;
    unsigned long __ss_align;
    char __ss_padding[128-2*sizeof(unsigned long)];
};

int socket (int, int, int);
int socketpair (int, int, int, int [2]);

int shutdown (int, int);

int bind (int, const struct sockaddr *, socklen_t);
int connect (int, const struct sockaddr *, socklen_t);
int listen (int, int);
int accept (int, struct sockaddr *__restrict, socklen_t *__restrict);
int accept4(int, struct sockaddr *__restrict, socklen_t *__restrict, int);

int getsockname (int, struct sockaddr *__restrict, socklen_t *__restrict);
int getpeername (int, struct sockaddr *__restrict, socklen_t *__restrict);

ssize_t send (int, const void *, size_t, int);
ssize_t recv (int, void *, size_t, int);
ssize_t sendto (int, const void *, size_t, int, const struct sockaddr *, socklen_t);
ssize_t recvfrom (int, void *__restrict, size_t, int, struct sockaddr *__restrict, socklen_t *__restrict);
ssize_t sendmsg (int, const struct msghdr *, int);
ssize_t recvmsg (int, struct msghdr *, int);

int getsockopt (int, int, int, void *__restrict, socklen_t *__restrict);
int setsockopt (int, int, int, const void *, socklen_t);

int sockatmark (int);

#endif

/// >>> CONTINUE include/netinet/in.h

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };

struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    uint8_t sin_zero[8];
};

struct in6_addr {
    union {
        uint8_t __s6_addr[16];
        uint16_t __s6_addr16[8];
        uint32_t __s6_addr32[4];
    } __in6_union;
};
#define s6_addr __in6_union.__s6_addr
#define s6_addr16 __in6_union.__s6_addr16
#define s6_addr32 __in6_union.__s6_addr32

struct sockaddr_in6 {
    sa_family_t     sin6_family;
    in_port_t       sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};

struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr;
    unsigned        ipv6mr_interface;
};

#define INADDR_ANY        ((in_addr_t) 0x00000000)
#define INADDR_BROADCAST  ((in_addr_t) 0xffffffff)
#define INADDR_NONE       ((in_addr_t) 0xffffffff)
#define INADDR_LOOPBACK   ((in_addr_t) 0x7f000001)

#define INADDR_UNSPEC_GROUP     ((in_addr_t) 0xe0000000)
#define INADDR_ALLHOSTS_GROUP   ((in_addr_t) 0xe0000001)
#define INADDR_ALLRTRS_GROUP    ((in_addr_t) 0xe0000002)
#define INADDR_MAX_LOCAL_GROUP  ((in_addr_t) 0xe00000ff)

#define IN6ADDR_ANY_INIT      { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

extern const struct in6_addr in6addr_any, in6addr_loopback;

#undef INET_ADDRSTRLEN
#undef INET6_ADDRSTRLEN
#define INET_ADDRSTRLEN  16
#define INET6_ADDRSTRLEN 46

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

#define IPPROTO_IP       0
#define IPPROTO_HOPOPTS  0
#define IPPROTO_ICMP     1
#define IPPROTO_IGMP     2
#define IPPROTO_IPIP     4
#define IPPROTO_TCP      6
#define IPPROTO_EGP      8
#define IPPROTO_PUP      12
#define IPPROTO_UDP      17
#define IPPROTO_IDP      22
#define IPPROTO_TP       29
#define IPPROTO_DCCP     33
#define IPPROTO_IPV6     41
#define IPPROTO_ROUTING  43
#define IPPROTO_FRAGMENT 44
#define IPPROTO_RSVP     46
#define IPPROTO_GRE      47
#define IPPROTO_ESP      50
#define IPPROTO_AH       51
#define IPPROTO_ICMPV6   58
#define IPPROTO_NONE     59
#define IPPROTO_DSTOPTS  60
#define IPPROTO_MTP      92
#define IPPROTO_BEETPH   94
#define IPPROTO_ENCAP    98
#define IPPROTO_PIM      103
#define IPPROTO_COMP     108
#define IPPROTO_SCTP     132
#define IPPROTO_MH       135
#define IPPROTO_UDPLITE  136
#define IPPROTO_RAW      255
#define IPPROTO_MAX      256

#define IN6_IS_ADDR_UNSPECIFIED(a) \
        (((uint32_t *) (a))[0] == 0 && ((uint32_t *) (a))[1] == 0 && \
         ((uint32_t *) (a))[2] == 0 && ((uint32_t *) (a))[3] == 0)

#define IN6_IS_ADDR_LOOPBACK(a) \
        (((uint32_t *) (a))[0] == 0 && ((uint32_t *) (a))[1] == 0 && \
         ((uint32_t *) (a))[2] == 0 && \
         ((uint8_t *) (a))[12] == 0 && ((uint8_t *) (a))[13] == 0 && \
         ((uint8_t *) (a))[14] == 0 && ((uint8_t *) (a))[15] == 1 )

#define IN6_IS_ADDR_MULTICAST(a) (((uint8_t *) (a))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(a) \
        ((((uint8_t *) (a))[0]) == 0xfe && (((uint8_t *) (a))[1] & 0xc0) == 0x80)

#define IN6_IS_ADDR_SITELOCAL(a) \
        ((((uint8_t *) (a))[0]) == 0xfe && (((uint8_t *) (a))[1] & 0xc0) == 0xc0)

#define IN6_IS_ADDR_V4MAPPED(a) \
        (((uint32_t *) (a))[0] == 0 && ((uint32_t *) (a))[1] == 0 && \
         ((uint8_t *) (a))[8] == 0 && ((uint8_t *) (a))[9] == 0 && \
         ((uint8_t *) (a))[10] == 0xff && ((uint8_t *) (a))[11] == 0xff)

#define IN6_IS_ADDR_V4COMPAT(a) \
        (((uint32_t *) (a))[0] == 0 && ((uint32_t *) (a))[1] == 0 && \
         ((uint32_t *) (a))[2] == 0 && ((uint8_t *) (a))[15] > 1)

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((uint8_t *) (a))[1] & 0xf) == 0x1))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((uint8_t *) (a))[1] & 0xf) == 0x2))

#define IN6_IS_ADDR_MC_SITELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((uint8_t *) (a))[1] & 0xf) == 0x5))

#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((uint8_t *) (a))[1] & 0xf) == 0x8))

#define IN6_IS_ADDR_MC_GLOBAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((uint8_t *) (a))[1] & 0xf) == 0xe))

#define __ARE_4_EQUAL(a,b) \
    (!( 0[a]-0[b] | 1[a]-1[b] | 2[a]-2[b] | 3[a]-3[b] ))
#define IN6_ARE_ADDR_EQUAL(a,b) \
    __ARE_4_EQUAL((const uint32_t *)(a), (const uint32_t *)(b))

#define    IN_CLASSA(a)        ((((in_addr_t)(a)) & 0x80000000) == 0)
#define    IN_CLASSA_NET        0xff000000
#define    IN_CLASSA_NSHIFT    24
#define    IN_CLASSA_HOST        (0xffffffff & ~IN_CLASSA_NET)
#define    IN_CLASSA_MAX        128
#define    IN_CLASSB(a)        ((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
#define    IN_CLASSB_NET        0xffff0000
#define    IN_CLASSB_NSHIFT    16
#define    IN_CLASSB_HOST        (0xffffffff & ~IN_CLASSB_NET)
#define    IN_CLASSB_MAX        65536
#define    IN_CLASSC(a)        ((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
#define    IN_CLASSC_NET        0xffffff00
#define    IN_CLASSC_NSHIFT    8
#define    IN_CLASSC_HOST        (0xffffffff & ~IN_CLASSC_NET)
#define    IN_CLASSD(a)        ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define    IN_MULTICAST(a)        IN_CLASSD(a)
#define    IN_EXPERIMENTAL(a)    ((((in_addr_t)(a)) & 0xe0000000) == 0xe0000000)
#define    IN_BADCLASS(a)        ((((in_addr_t)(a)) & 0xf0000000) == 0xf0000000)

#define IN_LOOPBACKNET 127

#define IP_TOS             1
#define IP_TTL             2
#define IP_HDRINCL         3
#define IP_OPTIONS         4
#define IP_ROUTER_ALERT    5
#define IP_RECVOPTS        6
#define IP_RETOPTS         7
#define IP_PKTINFO         8
#define IP_PKTOPTIONS      9
#define IP_PMTUDISC        10
#define IP_MTU_DISCOVER    10
#define IP_RECVERR         11
#define IP_RECVTTL         12
#define IP_RECVTOS         13
#define IP_MTU             14
#define IP_FREEBIND        15
#define IP_IPSEC_POLICY    16
#define IP_XFRM_POLICY     17
#define IP_PASSSEC         18
#define IP_TRANSPARENT     19
#define IP_ORIGDSTADDR     20
#define IP_RECVORIGDSTADDR IP_ORIGDSTADDR
#define IP_MINTTL          21
#define IP_MULTICAST_IF    32
#define IP_MULTICAST_TTL   33
#define IP_MULTICAST_LOOP  34
#define IP_ADD_MEMBERSHIP  35
#define IP_DROP_MEMBERSHIP 36
#define IP_UNBLOCK_SOURCE  37
#define IP_BLOCK_SOURCE    38
#define IP_ADD_SOURCE_MEMBERSHIP  39
#define IP_DROP_SOURCE_MEMBERSHIP 40
#define IP_MSFILTER        41
#define IP_MULTICAST_ALL   49
#define IP_UNICAST_IF      50

#define IP_RECVRETOPTS IP_RETOPTS

#define IP_PMTUDISC_DONT   0
#define IP_PMTUDISC_WANT   1
#define IP_PMTUDISC_DO     2
#define IP_PMTUDISC_PROBE  3

#define IP_DEFAULT_MULTICAST_TTL        1
#define IP_DEFAULT_MULTICAST_LOOP       1
#define IP_MAX_MEMBERSHIPS              20

struct ip_opts {
    struct in_addr ip_dst;
    char ip_opts[40];
};

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#define MCAST_JOIN_GROUP   42
#define MCAST_BLOCK_SOURCE 43
#define MCAST_UNBLOCK_SOURCE      44
#define MCAST_LEAVE_GROUP  45
#define MCAST_JOIN_SOURCE_GROUP   46
#define MCAST_LEAVE_SOURCE_GROUP  47
#define MCAST_MSFILTER     48

#define MCAST_EXCLUDE 0
#define MCAST_INCLUDE 1

struct ip_mreq {
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
};

struct ip_mreqn {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
};

struct ip_mreq_source {
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
    struct in_addr imr_sourceaddr;
};

struct ip_msfilter {
    struct in_addr imsf_multiaddr;
    struct in_addr imsf_interface;
    uint32_t imsf_fmode;
    uint32_t imsf_numsrc;
    struct in_addr imsf_slist[1];
};
#define IP_MSFILTER_SIZE(numsrc) \
    (sizeof(struct ip_msfilter) - sizeof(struct in_addr) \
    + (numsrc) * sizeof(struct in_addr))

struct group_req {
    uint32_t gr_interface;
    struct sockaddr_storage gr_group;
};

struct group_source_req {
    uint32_t gsr_interface;
    struct sockaddr_storage gsr_group;
    struct sockaddr_storage gsr_source;
};

struct group_filter {
    uint32_t gf_interface;
    struct sockaddr_storage gf_group;
    uint32_t gf_fmode;
    uint32_t gf_numsrc;
    struct sockaddr_storage gf_slist[1];
};
#define GROUP_FILTER_SIZE(numsrc) \
    (sizeof(struct group_filter) - sizeof(struct sockaddr_storage) \
    + (numsrc) * sizeof(struct sockaddr_storage))

struct in_pktinfo {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
};

struct in6_pktinfo {
    struct in6_addr ipi6_addr;
    unsigned ipi6_ifindex;
};

struct ip6_mtuinfo {
    struct sockaddr_in6 ip6m_addr;
    uint32_t ip6m_mtu;
};
#endif

#define IPV6_ADDRFORM           1
#define IPV6_2292PKTINFO        2
#define IPV6_2292HOPOPTS        3
#define IPV6_2292DSTOPTS        4
#define IPV6_2292RTHDR          5
#define IPV6_2292PKTOPTIONS     6
#define IPV6_CHECKSUM           7
#define IPV6_2292HOPLIMIT       8
#define IPV6_NEXTHOP            9
#define IPV6_AUTHHDR            10
#define IPV6_UNICAST_HOPS       16
#define IPV6_MULTICAST_IF       17
#define IPV6_MULTICAST_HOPS     18
#define IPV6_MULTICAST_LOOP     19
#define IPV6_JOIN_GROUP         20
#define IPV6_LEAVE_GROUP        21
#define IPV6_ROUTER_ALERT       22
#define IPV6_MTU_DISCOVER       23
#define IPV6_MTU                24
#define IPV6_RECVERR            25
#define IPV6_V6ONLY             26
#define IPV6_JOIN_ANYCAST       27
#define IPV6_LEAVE_ANYCAST      28
#define IPV6_IPSEC_POLICY       34
#define IPV6_XFRM_POLICY        35

#define IPV6_RECVPKTINFO        49
#define IPV6_PKTINFO            50
#define IPV6_RECVHOPLIMIT       51
#define IPV6_HOPLIMIT           52
#define IPV6_RECVHOPOPTS        53
#define IPV6_HOPOPTS            54
#define IPV6_RTHDRDSTOPTS       55
#define IPV6_RECVRTHDR          56
#define IPV6_RTHDR              57
#define IPV6_RECVDSTOPTS        58
#define IPV6_DSTOPTS            59

#define IPV6_RECVTCLASS         66
#define IPV6_TCLASS             67

#define IPV6_ADD_MEMBERSHIP     IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP    IPV6_LEAVE_GROUP
#define IPV6_RXHOPOPTS          IPV6_HOPOPTS
#define IPV6_RXDSTOPTS          IPV6_DSTOPTS

#define IPV6_PMTUDISC_DONT      0
#define IPV6_PMTUDISC_WANT      1
#define IPV6_PMTUDISC_DO        2
#define IPV6_PMTUDISC_PROBE     3

#define IPV6_RTHDR_LOOSE        0
#define IPV6_RTHDR_STRICT       1

#define IPV6_RTHDR_TYPE_0       0

#endif

/// >>> CONTINUE include/arpa/inet.h

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

in_addr_t inet_addr (const char *);
in_addr_t inet_network (const char *);
char *inet_ntoa (struct in_addr);
int inet_pton (int, const char *__restrict, void *__restrict);
const char *inet_ntop (int, const void *__restrict, char *__restrict, socklen_t);

int inet_aton (const char *, struct in_addr *);
struct in_addr inet_makeaddr(in_addr_t, in_addr_t);
in_addr_t inet_lnaof(struct in_addr);
in_addr_t inet_netof(struct in_addr);

#undef INET_ADDRSTRLEN
#undef INET6_ADDRSTRLEN
#define INET_ADDRSTRLEN  16
#define INET6_ADDRSTRLEN 46

#endif

/// >>> START include/net/ethernet.h
#ifndef _NET_ETHERNET_H
#define _NET_ETHERNET_H

/// >>> START include/sys/types.h
#ifndef    _SYS_TYPES_H
#define    _SYS_TYPES_H

#define __NEED_ino_t
#define __NEED_dev_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_mode_t
#define __NEED_nlink_t
#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_time_t
#define __NEED_timer_t
#define __NEED_clockid_t

#define __NEED_int8_t
#define __NEED_int16_t
#define __NEED_int32_t
#define __NEED_int64_t

#define __NEED_blkcnt_t
#define __NEED_fsblkcnt_t
#define __NEED_fsfilcnt_t

#define __NEED_id_t
#define __NEED_key_t
#define __NEED_clock_t
#define __NEED_suseconds_t
#define __NEED_blksize_t

#define __NEED_pthread_t
#define __NEED_pthread_attr_t
#define __NEED_pthread_mutexattr_t
#define __NEED_pthread_condattr_t
#define __NEED_pthread_rwlockattr_t
#define __NEED_pthread_barrierattr_t
#define __NEED_pthread_mutex_t
#define __NEED_pthread_cond_t
#define __NEED_pthread_rwlock_t
#define __NEED_pthread_barrier_t
#define __NEED_pthread_spinlock_t
#define __NEED_pthread_key_t
#define __NEED_pthread_once_t
#define __NEED_useconds_t
#define __NEED_u_int64_t

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_register_t
#endif

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned u_int32_t;

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
typedef char *caddr_t;
typedef unsigned char u_char;
typedef unsigned short u_short, ushort;
typedef unsigned u_int, uint;
typedef unsigned long u_long, ulong;
typedef long long quad_t;
typedef unsigned long long u_quad_t;
/// >>> START include/endian.h
#ifndef _ENDIAN_H
#define _ENDIAN_H

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412

#if defined(__GNUC__) && defined(__BYTE_ORDER__)
#define __BYTE_ORDER __BYTE_ORDER__
#else
/// >>> START arch/x86_64/bits/endian.h
#define __BYTE_ORDER __LITTLE_ENDIAN

/// >>> CONTINUE include/endian.h
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#define BIG_ENDIAN __BIG_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#define BYTE_ORDER __BYTE_ORDER

static __inline uint16_t __bswap16(uint16_t __x) {
    return __x<<8 | __x>>8;
}

static __inline uint32_t __bswap32(uint32_t __x) {
    return __x>>24 | (__x>>8&0xff00) | (__x<<8&0xff0000) | __x<<24;
}

static __inline uint64_t __bswap64(uint64_t __x) {
    return (__bswap32(__x)+0ULL)<<32 | __bswap32(__x>>32);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobe16(x) __bswap16(x)
#define be16toh(x) __bswap16(x)
#define betoh16(x) __bswap16(x)
#define htobe32(x) __bswap32(x)
#define be32toh(x) __bswap32(x)
#define betoh32(x) __bswap32(x)
#define htobe64(x) __bswap64(x)
#define be64toh(x) __bswap64(x)
#define betoh64(x) __bswap64(x)
#define htole16(x) (uint16_t)(x)
#define le16toh(x) (uint16_t)(x)
#define letoh16(x) (uint16_t)(x)
#define htole32(x) (uint32_t)(x)
#define le32toh(x) (uint32_t)(x)
#define letoh32(x) (uint32_t)(x)
#define htole64(x) (uint64_t)(x)
#define le64toh(x) (uint64_t)(x)
#define letoh64(x) (uint64_t)(x)
#else
#define htobe16(x) (uint16_t)(x)
#define be16toh(x) (uint16_t)(x)
#define betoh16(x) (uint16_t)(x)
#define htobe32(x) (uint32_t)(x)
#define be32toh(x) (uint32_t)(x)
#define betoh32(x) (uint32_t)(x)
#define htobe64(x) (uint64_t)(x)
#define be64toh(x) (uint64_t)(x)
#define betoh64(x) (uint64_t)(x)
#define htole16(x) __bswap16(x)
#define le16toh(x) __bswap16(x)
#define letoh16(x) __bswap16(x)
#define htole32(x) __bswap32(x)
#define le32toh(x) __bswap32(x)
#define letoh32(x) __bswap32(x)
#define htole64(x) __bswap64(x)
#define le64toh(x) __bswap64(x)
#define letoh64(x) __bswap64(x)
#endif

#endif

#endif

/// >>> CONTINUE include/sys/types.h
/// >>> START include/sys/select.h
#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#define __NEED_size_t
#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#define __NEED_struct_timespec
#define __NEED_sigset_t

#define FD_SETSIZE 1024

typedef unsigned long fd_mask;

typedef struct {
    unsigned long fds_bits[FD_SETSIZE / 8 / sizeof(long)];
} fd_set;

#define FD_ZERO(s) do { int __i; unsigned long *__b=(s)->fds_bits; for(__i=sizeof (fd_set)/sizeof (long); __i; __i--) *__b++=0; } while(0)
#define FD_SET(d, s)   ((s)->fds_bits[(d)/(8*sizeof(long))] |= (1UL<<((d)%(8*sizeof(long)))))
#define FD_CLR(d, s)   ((s)->fds_bits[(d)/(8*sizeof(long))] &= ~(1UL<<((d)%(8*sizeof(long)))))
#define FD_ISSET(d, s) !!((s)->fds_bits[(d)/(8*sizeof(long))] & (1UL<<((d)%(8*sizeof(long)))))

int select (int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict, struct timeval *__restrict);
int pselect (int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict, const struct timespec *__restrict, const sigset_t *__restrict);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define NFDBITS (8*(int)sizeof(long))
#endif

#endif

/// >>> CONTINUE include/sys/types.h
/// >>> START include/sys/sysmacros.h
#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

#define major(x) \
    ((unsigned)( (((x)>>31>>1) & 0xfffff000) | (((x)>>8) & 0x00000fff) ))
#define minor(x) \
    ((unsigned)( (((x)>>12) & 0xffffff00) | ((x) & 0x000000ff) ))

#define makedev(x,y) ( \
        (((x)&0xfffff000ULL) << 32) | \
    (((x)&0x00000fffULL) << 8) | \
        (((y)&0xffffff00ULL) << 12) | \
    (((y)&0x000000ffULL)) )

#endif

/// >>> CONTINUE include/sys/types.h
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define blksize64_t blksize_t
#define blkcnt64_t blkcnt_t
#define fsblkcnt64_t fsblkcnt_t
#define fsfilcnt64_t fsfilcnt_t
#define ino64_t ino_t
#define off64_t off_t
#endif

#endif

/// >>> CONTINUE include/net/ethernet.h
/// >>> START include/netinet/if_ether.h
#ifndef _NETINET_IF_ETHER_H
#define _NETINET_IF_ETHER_H

#define ETH_ALEN    6
#define ETH_HLEN    14
#define ETH_ZLEN    60
#define ETH_DATA_LEN    1500
#define ETH_FRAME_LEN    1514
#define ETH_FCS_LEN    4

#define ETH_P_LOOP    0x0060
#define ETH_P_PUP    0x0200
#define ETH_P_PUPAT    0x0201
#define ETH_P_IP    0x0800
#define ETH_P_X25    0x0805
#define ETH_P_ARP    0x0806
#define    ETH_P_BPQ    0x08FF
#define ETH_P_IEEEPUP    0x0a00
#define ETH_P_IEEEPUPAT    0x0a01
#define ETH_P_BATMAN    0x4305
#define ETH_P_DEC       0x6000
#define ETH_P_DNA_DL    0x6001
#define ETH_P_DNA_RC    0x6002
#define ETH_P_DNA_RT    0x6003
#define ETH_P_LAT       0x6004
#define ETH_P_DIAG      0x6005
#define ETH_P_CUST      0x6006
#define ETH_P_SCA       0x6007
#define ETH_P_TEB    0x6558
#define ETH_P_RARP      0x8035
#define ETH_P_ATALK    0x809B
#define ETH_P_AARP    0x80F3
#define ETH_P_8021Q    0x8100
#define ETH_P_IPX    0x8137
#define ETH_P_IPV6    0x86DD
#define ETH_P_PAUSE    0x8808
#define ETH_P_SLOW    0x8809
#define ETH_P_WCCP    0x883E
#define ETH_P_PPP_DISC    0x8863
#define ETH_P_PPP_SES    0x8864
#define ETH_P_MPLS_UC    0x8847
#define ETH_P_MPLS_MC    0x8848
#define ETH_P_ATMMPOA    0x884c
#define ETH_P_LINK_CTL    0x886c
#define ETH_P_ATMFATE    0x8884
#define ETH_P_PAE    0x888E
#define ETH_P_AOE    0x88A2
#define ETH_P_8021AD    0x88A8
#define ETH_P_802_EX1    0x88B5
#define ETH_P_TIPC    0x88CA
#define ETH_P_8021AH    0x88E7
#define ETH_P_MVRP    0x88F5
#define ETH_P_1588    0x88F7
#define ETH_P_FCOE    0x8906
#define ETH_P_TDLS    0x890D
#define ETH_P_FIP    0x8914
#define ETH_P_QINQ1    0x9100
#define ETH_P_QINQ2    0x9200
#define ETH_P_QINQ3    0x9300
#define ETH_P_EDSA    0xDADA
#define ETH_P_AF_IUCV    0xFBFB

#define ETH_P_802_3_MIN    0x0600

#define ETH_P_802_3    0x0001
#define ETH_P_AX25    0x0002
#define ETH_P_ALL    0x0003
#define ETH_P_802_2    0x0004
#define ETH_P_SNAP    0x0005
#define ETH_P_DDCMP     0x0006
#define ETH_P_WAN_PPP   0x0007
#define ETH_P_PPP_MP    0x0008
#define ETH_P_LOCALTALK 0x0009
#define ETH_P_CAN    0x000C
#define ETH_P_CANFD    0x000D
#define ETH_P_PPPTALK    0x0010
#define ETH_P_TR_802_2    0x0011
#define ETH_P_MOBITEX    0x0015
#define ETH_P_CONTROL    0x0016
#define ETH_P_IRDA    0x0017
#define ETH_P_ECONET    0x0018
#define ETH_P_HDLC    0x0019
#define ETH_P_ARCNET    0x001A
#define ETH_P_DSA    0x001B
#define ETH_P_TRAILER    0x001C
#define ETH_P_PHONET    0x00F5
#define ETH_P_IEEE802154 0x00F6
#define ETH_P_CAIF    0x00F7

struct ethhdr {
    uint8_t h_dest[ETH_ALEN];
    uint8_t h_source[ETH_ALEN];
    uint16_t h_proto;
};

/// >>> START include/net/if_arp.h
/* Nonstandard header */
#ifndef _NET_IF_ARP_H
#define _NET_IF_ARP_H

#define MAX_ADDR_LEN    7

#define    ARPOP_REQUEST    1
#define    ARPOP_REPLY    2
#define    ARPOP_RREQUEST    3
#define    ARPOP_RREPLY    4
#define    ARPOP_InREQUEST    8
#define    ARPOP_InREPLY    9
#define    ARPOP_NAK    10

struct arphdr {
    uint16_t ar_hrd;
    uint16_t ar_pro;
    uint8_t ar_hln;
    uint8_t ar_pln;
    uint16_t ar_op;
};

#define ARPHRD_NETROM    0
#define ARPHRD_ETHER     1
#define    ARPHRD_EETHER    2
#define    ARPHRD_AX25    3
#define    ARPHRD_PRONET    4
#define    ARPHRD_CHAOS    5
#define    ARPHRD_IEEE802    6
#define    ARPHRD_ARCNET    7
#define    ARPHRD_APPLETLK    8
#define    ARPHRD_DLCI    15
#define    ARPHRD_ATM    19
#define    ARPHRD_METRICOM    23
#define ARPHRD_IEEE1394    24
#define ARPHRD_EUI64        27
#define ARPHRD_INFINIBAND    32
#define ARPHRD_SLIP    256
#define ARPHRD_CSLIP    257
#define ARPHRD_SLIP6    258
#define ARPHRD_CSLIP6    259
#define ARPHRD_RSRVD    260
#define ARPHRD_ADAPT    264
#define ARPHRD_ROSE    270
#define ARPHRD_X25    271
#define ARPHRD_HWX25    272
#define ARPHRD_CAN    280
#define ARPHRD_PPP    512
#define ARPHRD_CISCO    513
#define ARPHRD_HDLC    ARPHRD_CISCO
#define ARPHRD_LAPB    516
#define ARPHRD_DDCMP    517
#define    ARPHRD_RAWHDLC    518

#define ARPHRD_TUNNEL    768
#define ARPHRD_TUNNEL6    769
#define ARPHRD_FRAD    770
#define ARPHRD_SKIP    771
#define ARPHRD_LOOPBACK    772
#define ARPHRD_LOCALTLK 773
#define ARPHRD_FDDI    774
#define ARPHRD_BIF    775
#define ARPHRD_SIT    776
#define ARPHRD_IPDDP    777
#define ARPHRD_IPGRE    778
#define ARPHRD_PIMREG    779
#define ARPHRD_HIPPI    780
#define ARPHRD_ASH    781
#define ARPHRD_ECONET    782
#define ARPHRD_IRDA    783
#define ARPHRD_FCPP    784
#define ARPHRD_FCAL    785
#define ARPHRD_FCPL    786
#define ARPHRD_FCFABRIC 787
#define ARPHRD_IEEE802_TR 800
#define ARPHRD_IEEE80211 801
#define ARPHRD_IEEE80211_PRISM 802
#define ARPHRD_IEEE80211_RADIOTAP 803
#define ARPHRD_IEEE802154 804
#define ARPHRD_IEEE802154_MONITOR 805
#define ARPHRD_PHONET 820
#define ARPHRD_PHONET_PIPE 821
#define ARPHRD_CAIF 822
#define ARPHRD_IP6GRE 823
#define ARPHRD_NETLINK 824

#define ARPHRD_VOID      0xFFFF
#define ARPHRD_NONE      0xFFFE

struct arpreq {
    struct sockaddr arp_pa;
    struct sockaddr arp_ha;
    int arp_flags;
    struct sockaddr arp_netmask;
    char arp_dev[16];
};

struct arpreq_old {
    struct sockaddr arp_pa;
    struct sockaddr arp_ha;
    int arp_flags;
    struct sockaddr arp_netmask;
};

#define ATF_COM        0x02
#define    ATF_PERM    0x04
#define    ATF_PUBL    0x08
#define    ATF_USETRAILERS    0x10
#define ATF_NETMASK     0x20
#define ATF_DONTPUB    0x40
#define ATF_MAGIC    0x80

#define ARPD_UPDATE    0x01
#define ARPD_LOOKUP    0x02
#define ARPD_FLUSH    0x03

struct arpd_request {
    unsigned short req;
    uint32_t ip;
    unsigned long dev;
    unsigned long stamp;
    unsigned long updated;
    unsigned char ha[MAX_ADDR_LEN];
};

#endif

/// >>> CONTINUE include/netinet/if_ether.h

struct    ether_arp {
    struct    arphdr ea_hdr;
    uint8_t arp_sha[ETH_ALEN];
    uint8_t arp_spa[4];
    uint8_t arp_tha[ETH_ALEN];
    uint8_t arp_tpa[4];
};
#define    arp_hrd    ea_hdr.ar_hrd
#define    arp_pro    ea_hdr.ar_pro
#define    arp_hln    ea_hdr.ar_hln
#define    arp_pln    ea_hdr.ar_pln
#define    arp_op    ea_hdr.ar_op

#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr) \
do { \
    (enaddr)[0] = 0x01; \
    (enaddr)[1] = 0x00; \
    (enaddr)[2] = 0x5e; \
    (enaddr)[3] = ((uint8_t *)ipaddr)[1] & 0x7f; \
    (enaddr)[4] = ((uint8_t *)ipaddr)[2]; \
    (enaddr)[5] = ((uint8_t *)ipaddr)[3]; \
} while(0)

#endif

/// >>> CONTINUE include/net/ethernet.h

struct ether_addr {
    uint8_t ether_addr_octet[ETH_ALEN];
};

struct ether_header {
    uint8_t  ether_dhost[ETH_ALEN];
    uint8_t  ether_shost[ETH_ALEN];
    uint16_t ether_type;
};

#define    ETHERTYPE_PUP        0x0200
#define ETHERTYPE_SPRITE    0x0500
#define    ETHERTYPE_IP        0x0800
#define    ETHERTYPE_ARP        0x0806
#define    ETHERTYPE_REVARP    0x8035
#define ETHERTYPE_AT        0x809B
#define ETHERTYPE_AARP        0x80F3
#define    ETHERTYPE_VLAN        0x8100
#define ETHERTYPE_IPX        0x8137
#define    ETHERTYPE_IPV6        0x86dd
#define ETHERTYPE_LOOPBACK    0x9000

#define    ETHER_ADDR_LEN    ETH_ALEN
#define    ETHER_TYPE_LEN    2
#define    ETHER_CRC_LEN    4
#define    ETHER_HDR_LEN    ETH_HLEN
#define    ETHER_MIN_LEN    (ETH_ZLEN + ETHER_CRC_LEN)
#define    ETHER_MAX_LEN    (ETH_FRAME_LEN + ETHER_CRC_LEN)

#define    ETHER_IS_VALID_LEN(foo)    \
    ((foo) >= ETHER_MIN_LEN && (foo) <= ETHER_MAX_LEN)

#define    ETHERTYPE_TRAIL        0x1000
#define    ETHERTYPE_NTRAILER    16

#define    ETHERMTU    ETH_DATA_LEN
#define    ETHERMIN    (ETHER_MIN_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

#endif

/// >>> START include/net/if.h
#ifndef _NET_IF_H
#define _NET_IF_H

#define IF_NAMESIZE 16

struct if_nameindex {
    unsigned int if_index;
    char *if_name;
};

unsigned int if_nametoindex (const char *);
char *if_indextoname (unsigned int, char *);
struct if_nameindex *if_nameindex (void);
void if_freenameindex (struct if_nameindex *);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#define IFF_UP    0x1
#define IFF_BROADCAST 0x2
#define IFF_DEBUG 0x4
#define IFF_LOOPBACK 0x8
#define IFF_POINTOPOINT 0x10
#define IFF_NOTRAILERS 0x20
#define IFF_RUNNING 0x40
#define IFF_NOARP 0x80
#define IFF_PROMISC 0x100
#define IFF_ALLMULTI 0x200
#define IFF_MASTER 0x400
#define IFF_SLAVE 0x800
#define IFF_MULTICAST 0x1000
#define IFF_PORTSEL 0x2000
#define IFF_AUTOMEDIA 0x4000
#define IFF_DYNAMIC 0x8000
#define IFF_LOWER_UP 0x10000
#define IFF_DORMANT 0x20000
#define IFF_ECHO 0x40000
#define IFF_VOLATILE (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST| \
        IFF_ECHO|IFF_MASTER|IFF_SLAVE|IFF_RUNNING|IFF_LOWER_UP|IFF_DORMANT)

struct ifaddr {
    struct sockaddr ifa_addr;
    union {
        struct sockaddr    ifu_broadaddr;
        struct sockaddr    ifu_dstaddr;
    } ifa_ifu;
    struct iface *ifa_ifp;
    struct ifaddr *ifa_next;
};

#define ifa_broadaddr    ifa_ifu.ifu_broadaddr
#define ifa_dstaddr    ifa_ifu.ifu_dstaddr

struct ifmap {
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
};

#define IFHWADDRLEN    6
#define IFNAMSIZ    IF_NAMESIZE

struct ifreq {
    union {
        char ifrn_name[IFNAMSIZ];
    } ifr_ifrn;
    union {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
        short int ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct ifmap ifru_map;
        char ifru_slave[IFNAMSIZ];
        char ifru_newname[IFNAMSIZ];
        void *ifru_data;
    } ifr_ifru;
};

#define ifr_name    ifr_ifrn.ifrn_name
#define ifr_hwaddr    ifr_ifru.ifru_hwaddr
#define ifr_addr    ifr_ifru.ifru_addr
#define ifr_dstaddr    ifr_ifru.ifru_dstaddr
#define ifr_broadaddr    ifr_ifru.ifru_broadaddr
#define ifr_netmask    ifr_ifru.ifru_netmask
#define ifr_flags    ifr_ifru.ifru_flags
#define ifr_metric    ifr_ifru.ifru_ivalue
#define ifr_mtu        ifr_ifru.ifru_mtu
#define ifr_map        ifr_ifru.ifru_map
#define ifr_slave    ifr_ifru.ifru_slave
#define ifr_data    ifr_ifru.ifru_data
#define ifr_ifindex    ifr_ifru.ifru_ivalue
#define ifr_bandwidth    ifr_ifru.ifru_ivalue
#define ifr_qlen    ifr_ifru.ifru_ivalue
#define ifr_newname    ifr_ifru.ifru_newname
#define _IOT_ifreq    _IOT(_IOTS(char),IFNAMSIZ,_IOTS(char),16,0,0)
#define _IOT_ifreq_short _IOT(_IOTS(char),IFNAMSIZ,_IOTS(short),1,0,0)
#define _IOT_ifreq_int    _IOT(_IOTS(char),IFNAMSIZ,_IOTS(int),1,0,0)

struct ifconf {
    int ifc_len;        
    union {
        void *ifcu_buf;
        struct ifreq *ifcu_req;
    } ifc_ifcu;
};

#define ifc_buf        ifc_ifcu.ifcu_buf
#define ifc_req        ifc_ifcu.ifcu_req
#define _IOT_ifconf _IOT(_IOTS(struct ifconf),1,0,0,0,0)

#endif

#endif

/// >>> START include/net/route.h
#ifndef _NET_ROUTE_H
#define _NET_ROUTE_H

struct rtentry {
    unsigned long int rt_pad1;
    struct sockaddr rt_dst;
    struct sockaddr rt_gateway;
    struct sockaddr rt_genmask;
    unsigned short int rt_flags;
    short int rt_pad2;
    unsigned long int rt_pad3;
    unsigned char rt_tos;
    unsigned char rt_class;
    short int rt_pad4[sizeof(long)/2-1];
    short int rt_metric;
    char *rt_dev;
    unsigned long int rt_mtu;
    unsigned long int rt_window;
    unsigned short int rt_irtt;
};

#define rt_mss    rt_mtu

struct in6_rtmsg {
    struct in6_addr rtmsg_dst;
    struct in6_addr rtmsg_src;
    struct in6_addr rtmsg_gateway;
    uint32_t rtmsg_type;
    uint16_t rtmsg_dst_len;
    uint16_t rtmsg_src_len;
    uint32_t rtmsg_metric;
    unsigned long int rtmsg_info;
    uint32_t rtmsg_flags;
    int rtmsg_ifindex;
};

#define    RTF_UP        0x0001
#define    RTF_GATEWAY    0x0002

#define    RTF_HOST    0x0004
#define RTF_REINSTATE    0x0008
#define    RTF_DYNAMIC    0x0010
#define    RTF_MODIFIED    0x0020
#define RTF_MTU        0x0040
#define RTF_MSS        RTF_MTU
#define RTF_WINDOW    0x0080
#define RTF_IRTT    0x0100
#define RTF_REJECT    0x0200
#define    RTF_STATIC    0x0400
#define    RTF_XRESOLVE    0x0800
#define RTF_NOFORWARD   0x1000
#define RTF_THROW    0x2000
#define RTF_NOPMTUDISC  0x4000

#define RTF_DEFAULT    0x00010000
#define RTF_ALLONLINK    0x00020000
#define RTF_ADDRCONF    0x00040000

#define RTF_LINKRT    0x00100000
#define RTF_NONEXTHOP    0x00200000

#define RTF_CACHE    0x01000000
#define RTF_FLOW    0x02000000
#define RTF_POLICY    0x04000000

#define RTCF_VALVE    0x00200000
#define RTCF_MASQ    0x00400000
#define RTCF_NAT    0x00800000
#define RTCF_DOREDIRECT 0x01000000
#define RTCF_LOG    0x02000000
#define RTCF_DIRECTSRC    0x04000000

#define RTF_LOCAL    0x80000000
#define RTF_INTERFACE    0x40000000
#define RTF_MULTICAST    0x20000000
#define RTF_BROADCAST    0x10000000
#define RTF_NAT        0x08000000

#define RTF_ADDRCLASSMASK    0xF8000000
#define RT_ADDRCLASS(flags)    ((uint32_t) flags >> 23)

#define RT_TOS(tos)        ((tos) & IPTOS_TOS_MASK)

#define RT_LOCALADDR(flags)    ((flags & RTF_ADDRCLASSMASK) \
                 == (RTF_LOCAL|RTF_INTERFACE))

#define RT_CLASS_UNSPEC        0
#define RT_CLASS_DEFAULT    253

#define RT_CLASS_MAIN        254
#define RT_CLASS_LOCAL        255
#define RT_CLASS_MAX        255

#define RTMSG_ACK        NLMSG_ACK
#define RTMSG_OVERRUN        NLMSG_OVERRUN

#define RTMSG_NEWDEVICE        0x11
#define RTMSG_DELDEVICE        0x12
#define RTMSG_NEWROUTE        0x21
#define RTMSG_DELROUTE        0x22
#define RTMSG_NEWRULE        0x31
#define RTMSG_DELRULE        0x32
#define RTMSG_CONTROL        0x40

#define RTMSG_AR_FAILED        0x51

#endif

/// >>> START include/netinet/icmp6.h
#ifndef _NETINET_ICMP6_H
#define _NETINET_ICMP6_H

/// >>> START include/string.h
#ifndef    _STRING_H
#define    _STRING_H

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#define __NEED_size_t
#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
#define __NEED_locale_t
#endif

void *memcpy (void *__restrict, const void *__restrict, size_t);
void *memmove (void *, const void *, size_t);
void *memset (void *, int, size_t);
int memcmp (const void *, const void *, size_t);
void *memchr (const void *, int, size_t);

char *strcpy (char *__restrict, const char *__restrict);
char *strncpy (char *__restrict, const char *__restrict, size_t);

char *strcat (char *__restrict, const char *__restrict);
char *strncat (char *__restrict, const char *__restrict, size_t);

int strcmp (const char *, const char *);
int strncmp (const char *, const char *, size_t);

int strcoll (const char *, const char *);
size_t strxfrm (char *__restrict, const char *__restrict, size_t);

char *strchr (const char *, int);
char *strrchr (const char *, int);

size_t strcspn (const char *, const char *);
size_t strspn (const char *, const char *);
char *strpbrk (const char *, const char *);
char *strstr (const char *, const char *);
char *strtok (char *__restrict, const char *__restrict);

size_t strlen (const char *);

char *strerror (int);

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
/// >>> START include/strings.h
#ifndef    _STRINGS_H
#define    _STRINGS_H

#define __NEED_size_t
#define __NEED_locale_t

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_POSIX_SOURCE) \
 || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 < 200809L) \
 || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700)
int bcmp (const void *, const void *, size_t);
void bcopy (const void *, void *, size_t);
void bzero (void *, size_t);
char *index (const char *, int);
char *rindex (const char *, int);
#endif

int ffs (int);

int strcasecmp (const char *, const char *);
int strncasecmp (const char *, const char *, size_t);

int strcasecmp_l (const char *, const char *, locale_t);
int strncasecmp_l (const char *, const char *, size_t, locale_t);

#endif

/// >>> CONTINUE include/string.h
#endif

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
char *strtok_r (char *__restrict, const char *__restrict, char **__restrict);
int strerror_r (int, char *, size_t);
char *stpcpy(char *__restrict, const char *__restrict);
char *stpncpy(char *__restrict, const char *__restrict, size_t);
size_t strnlen (const char *, size_t);
char *strdup (const char *);
char *strndup (const char *, size_t);
char *strsignal(int);
char *strerror_l (int, locale_t);
int strcoll_l (const char *, const char *, locale_t);
size_t strxfrm_l (char *__restrict, const char *__restrict, size_t, locale_t);
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
void *memccpy (void *__restrict, const void *__restrict, int, size_t);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
char *strsep(char **, const char *);
size_t strlcat (char *, const char *, size_t);
size_t strlcpy (char *, const char *, size_t);
#endif

#ifdef _GNU_SOURCE
#define    strdupa(x)    strcpy(alloca(strlen(x)+1),x)
int strverscmp (const char *, const char *);
int strcasecmp_l (const char *, const char *, locale_t);
int strncasecmp_l (const char *, const char *, size_t, locale_t);
char *strchrnul(const char *, int);
char *strcasestr(const char *, const char *);
void *memmem(const void *, size_t, const void *, size_t);
void *memrchr(const void *, int, size_t);
void *mempcpy(void *, const void *, size_t);
#ifndef __cplusplus
char *basename();
#endif
#endif

#endif

/// >>> CONTINUE include/netinet/icmp6.h

#define ICMP6_FILTER 1

#define ICMP6_FILTER_BLOCK        1
#define ICMP6_FILTER_PASS        2
#define ICMP6_FILTER_BLOCKOTHERS    3
#define ICMP6_FILTER_PASSONLY        4

struct icmp6_filter {
    uint32_t icmp6_filt[8];
};

struct icmp6_hdr {
    uint8_t     icmp6_type;
    uint8_t     icmp6_code;
    uint16_t    icmp6_cksum;
    union {
        uint32_t  icmp6_un_data32[1];
        uint16_t  icmp6_un_data16[2];
        uint8_t   icmp6_un_data8[4];
    } icmp6_dataun;
};

#define icmp6_data32    icmp6_dataun.icmp6_un_data32
#define icmp6_data16    icmp6_dataun.icmp6_un_data16
#define icmp6_data8     icmp6_dataun.icmp6_un_data8
#define icmp6_pptr      icmp6_data32[0]
#define icmp6_mtu       icmp6_data32[0]
#define icmp6_id        icmp6_data16[0]
#define icmp6_seq       icmp6_data16[1]
#define icmp6_maxdelay  icmp6_data16[0]

#define ICMP6_DST_UNREACH             1
#define ICMP6_PACKET_TOO_BIG          2
#define ICMP6_TIME_EXCEEDED           3
#define ICMP6_PARAM_PROB              4

#define ICMP6_INFOMSG_MASK  0x80

#define ICMP6_ECHO_REQUEST          128
#define ICMP6_ECHO_REPLY            129
#define MLD_LISTENER_QUERY          130
#define MLD_LISTENER_REPORT         131
#define MLD_LISTENER_REDUCTION      132

#define ICMP6_DST_UNREACH_NOROUTE     0
#define ICMP6_DST_UNREACH_ADMIN       1
#define ICMP6_DST_UNREACH_BEYONDSCOPE 2
#define ICMP6_DST_UNREACH_ADDR        3
#define ICMP6_DST_UNREACH_NOPORT      4

#define ICMP6_TIME_EXCEED_TRANSIT     0
#define ICMP6_TIME_EXCEED_REASSEMBLY  1

#define ICMP6_PARAMPROB_HEADER        0
#define ICMP6_PARAMPROB_NEXTHEADER    1
#define ICMP6_PARAMPROB_OPTION        2

#define ICMP6_FILTER_WILLPASS(type, filterp) \
    ((((filterp)->icmp6_filt[(type) >> 5]) & (1 << ((type) & 31))) == 0)

#define ICMP6_FILTER_WILLBLOCK(type, filterp) \
    ((((filterp)->icmp6_filt[(type) >> 5]) & (1 << ((type) & 31))) != 0)

#define ICMP6_FILTER_SETPASS(type, filterp) \
    ((((filterp)->icmp6_filt[(type) >> 5]) &= ~(1 << ((type) & 31))))

#define ICMP6_FILTER_SETBLOCK(type, filterp) \
    ((((filterp)->icmp6_filt[(type) >> 5]) |=  (1 << ((type) & 31))))

#define ICMP6_FILTER_SETPASSALL(filterp) \
    memset (filterp, 0, sizeof (struct icmp6_filter));

#define ICMP6_FILTER_SETBLOCKALL(filterp) \
    memset (filterp, 0xFF, sizeof (struct icmp6_filter));

#define ND_ROUTER_SOLICIT           133
#define ND_ROUTER_ADVERT            134
#define ND_NEIGHBOR_SOLICIT         135
#define ND_NEIGHBOR_ADVERT          136
#define ND_REDIRECT                 137

struct nd_router_solicit {
    struct icmp6_hdr  nd_rs_hdr;
};

#define nd_rs_type               nd_rs_hdr.icmp6_type
#define nd_rs_code               nd_rs_hdr.icmp6_code
#define nd_rs_cksum              nd_rs_hdr.icmp6_cksum
#define nd_rs_reserved           nd_rs_hdr.icmp6_data32[0]

struct nd_router_advert {
    struct icmp6_hdr  nd_ra_hdr;
    uint32_t   nd_ra_reachable;
    uint32_t   nd_ra_retransmit;
};

#define nd_ra_type               nd_ra_hdr.icmp6_type
#define nd_ra_code               nd_ra_hdr.icmp6_code
#define nd_ra_cksum              nd_ra_hdr.icmp6_cksum
#define nd_ra_curhoplimit        nd_ra_hdr.icmp6_data8[0]
#define nd_ra_flags_reserved     nd_ra_hdr.icmp6_data8[1]
#define ND_RA_FLAG_MANAGED       0x80
#define ND_RA_FLAG_OTHER         0x40
#define ND_RA_FLAG_HOME_AGENT    0x20
#define nd_ra_router_lifetime    nd_ra_hdr.icmp6_data16[1]

struct nd_neighbor_solicit {
    struct icmp6_hdr  nd_ns_hdr;
    struct in6_addr   nd_ns_target;
};

#define nd_ns_type               nd_ns_hdr.icmp6_type
#define nd_ns_code               nd_ns_hdr.icmp6_code
#define nd_ns_cksum              nd_ns_hdr.icmp6_cksum
#define nd_ns_reserved           nd_ns_hdr.icmp6_data32[0]

struct nd_neighbor_advert {
    struct icmp6_hdr  nd_na_hdr;
    struct in6_addr   nd_na_target;
};

#define nd_na_type               nd_na_hdr.icmp6_type
#define nd_na_code               nd_na_hdr.icmp6_code
#define nd_na_cksum              nd_na_hdr.icmp6_cksum
#define nd_na_flags_reserved     nd_na_hdr.icmp6_data32[0]
#if     __BYTE_ORDER == __BIG_ENDIAN
#define ND_NA_FLAG_ROUTER        0x80000000
#define ND_NA_FLAG_SOLICITED     0x40000000
#define ND_NA_FLAG_OVERRIDE      0x20000000
#else
#define ND_NA_FLAG_ROUTER        0x00000080
#define ND_NA_FLAG_SOLICITED     0x00000040
#define ND_NA_FLAG_OVERRIDE      0x00000020
#endif

struct nd_redirect {
    struct icmp6_hdr  nd_rd_hdr;
    struct in6_addr   nd_rd_target;
    struct in6_addr   nd_rd_dst;
};

#define nd_rd_type               nd_rd_hdr.icmp6_type
#define nd_rd_code               nd_rd_hdr.icmp6_code
#define nd_rd_cksum              nd_rd_hdr.icmp6_cksum
#define nd_rd_reserved           nd_rd_hdr.icmp6_data32[0]

struct nd_opt_hdr {
    uint8_t  nd_opt_type;
    uint8_t  nd_opt_len;
};

#define ND_OPT_SOURCE_LINKADDR        1
#define ND_OPT_TARGET_LINKADDR        2
#define ND_OPT_PREFIX_INFORMATION    3
#define ND_OPT_REDIRECTED_HEADER    4
#define ND_OPT_MTU            5
#define ND_OPT_RTR_ADV_INTERVAL        7
#define ND_OPT_HOME_AGENT_INFO        8

struct nd_opt_prefix_info {
    uint8_t   nd_opt_pi_type;
    uint8_t   nd_opt_pi_len;
    uint8_t   nd_opt_pi_prefix_len;
    uint8_t   nd_opt_pi_flags_reserved;
    uint32_t  nd_opt_pi_valid_time;
    uint32_t  nd_opt_pi_preferred_time;
    uint32_t  nd_opt_pi_reserved2;
    struct in6_addr  nd_opt_pi_prefix;
};

#define ND_OPT_PI_FLAG_ONLINK    0x80
#define ND_OPT_PI_FLAG_AUTO    0x40
#define ND_OPT_PI_FLAG_RADDR    0x20

struct nd_opt_rd_hdr {
    uint8_t   nd_opt_rh_type;
    uint8_t   nd_opt_rh_len;
    uint16_t  nd_opt_rh_reserved1;
    uint32_t  nd_opt_rh_reserved2;
};

struct nd_opt_mtu {
    uint8_t   nd_opt_mtu_type;
    uint8_t   nd_opt_mtu_len;
    uint16_t  nd_opt_mtu_reserved;
    uint32_t  nd_opt_mtu_mtu;
};

struct mld_hdr {
    struct icmp6_hdr    mld_icmp6_hdr;
    struct in6_addr     mld_addr;
};

#define mld_type        mld_icmp6_hdr.icmp6_type
#define mld_code        mld_icmp6_hdr.icmp6_code
#define mld_cksum       mld_icmp6_hdr.icmp6_cksum
#define mld_maxdelay    mld_icmp6_hdr.icmp6_data16[0]
#define mld_reserved    mld_icmp6_hdr.icmp6_data16[1]

#define ICMP6_ROUTER_RENUMBERING    138

struct icmp6_router_renum {
    struct icmp6_hdr    rr_hdr;
    uint8_t             rr_segnum;
    uint8_t             rr_flags;
    uint16_t            rr_maxdelay;
    uint32_t            rr_reserved;
};

#define rr_type        rr_hdr.icmp6_type
#define rr_code         rr_hdr.icmp6_code
#define rr_cksum        rr_hdr.icmp6_cksum
#define rr_seqnum       rr_hdr.icmp6_data32[0]

#define ICMP6_RR_FLAGS_TEST             0x80
#define ICMP6_RR_FLAGS_REQRESULT        0x40
#define ICMP6_RR_FLAGS_FORCEAPPLY       0x20
#define ICMP6_RR_FLAGS_SPECSITE         0x10
#define ICMP6_RR_FLAGS_PREVDONE         0x08

struct rr_pco_match {
    uint8_t             rpm_code;
    uint8_t             rpm_len;
    uint8_t             rpm_ordinal;
    uint8_t             rpm_matchlen;
    uint8_t             rpm_minlen;
    uint8_t             rpm_maxlen;
    uint16_t            rpm_reserved;
    struct in6_addr     rpm_prefix;
};

#define RPM_PCO_ADD             1
#define RPM_PCO_CHANGE          2
#define RPM_PCO_SETGLOBAL       3

struct rr_pco_use {
    uint8_t             rpu_uselen;
    uint8_t             rpu_keeplen;
    uint8_t             rpu_ramask;
    uint8_t             rpu_raflags;
    uint32_t            rpu_vltime;
    uint32_t            rpu_pltime;
    uint32_t            rpu_flags;
    struct in6_addr     rpu_prefix;
};

#define ICMP6_RR_PCOUSE_RAFLAGS_ONLINK  0x20
#define ICMP6_RR_PCOUSE_RAFLAGS_AUTO    0x10

#if __BYTE_ORDER == __BIG_ENDIAN
#define ICMP6_RR_PCOUSE_FLAGS_DECRVLTIME 0x80000000
#define ICMP6_RR_PCOUSE_FLAGS_DECRPLTIME 0x40000000
#else
#define ICMP6_RR_PCOUSE_FLAGS_DECRVLTIME 0x80
#define ICMP6_RR_PCOUSE_FLAGS_DECRPLTIME 0x40
#endif

struct rr_result {
    uint16_t            rrr_flags;
    uint8_t             rrr_ordinal;
    uint8_t             rrr_matchedlen;
    uint32_t            rrr_ifid;
    struct in6_addr     rrr_prefix;
};

#if __BYTE_ORDER == __BIG_ENDIAN
#define ICMP6_RR_RESULT_FLAGS_OOB       0x0002
#define ICMP6_RR_RESULT_FLAGS_FORBIDDEN 0x0001
#else
#define ICMP6_RR_RESULT_FLAGS_OOB       0x0200
#define ICMP6_RR_RESULT_FLAGS_FORBIDDEN 0x0100
#endif

struct nd_opt_adv_interval {
    uint8_t   nd_opt_adv_interval_type;
    uint8_t   nd_opt_adv_interval_len;
    uint16_t  nd_opt_adv_interval_reserved;
    uint32_t  nd_opt_adv_interval_ival;
};

struct nd_opt_home_agent_info {
    uint8_t   nd_opt_home_agent_info_type;
    uint8_t   nd_opt_home_agent_info_len;
    uint16_t  nd_opt_home_agent_info_reserved;
    uint16_t  nd_opt_home_agent_info_preference;
    uint16_t  nd_opt_home_agent_info_lifetime;
};

#endif

/// >>> START include/netinet/in_systm.h
#ifndef _NETINET_IN_SYSTM_H
#define _NETINET_IN_SYSTM_H

typedef uint16_t n_short;
typedef uint32_t n_long, n_time;

#endif

/// >>> START include/netinet/ip.h
#ifndef _NETINET_IP_H
#define _NETINET_IP_H

struct timestamp {
    uint8_t len;
    uint8_t ptr;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int flags:4;
    unsigned int overflow:4;
#else
    unsigned int overflow:4;
    unsigned int flags:4;
#endif
    uint32_t data[9];
  };

struct iphdr {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#else
    unsigned int version:4;
    unsigned int ihl:4;
#endif
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

struct ip {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;
    unsigned int ip_v:4;
#else
    unsigned int ip_v:4;
    unsigned int ip_hl:4;
#endif
    uint8_t ip_tos;
    uint16_t ip_len;
    uint16_t ip_id;
    uint16_t ip_off;
    uint8_t ip_ttl;
    uint8_t ip_p;
    uint16_t ip_sum;
    struct in_addr ip_src, ip_dst;
};

#define    IP_RF 0x8000
#define    IP_DF 0x4000
#define    IP_MF 0x2000
#define    IP_OFFMASK 0x1fff

struct ip_timestamp {
    uint8_t ipt_code;
    uint8_t ipt_len;
    uint8_t ipt_ptr;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ipt_flg:4;
    unsigned int ipt_oflw:4;
#else
    unsigned int ipt_oflw:4;
    unsigned int ipt_flg:4;
#endif
    uint32_t data[9];
};

#define    IPVERSION    4
#define    IP_MAXPACKET    65535

#define    IPTOS_ECN_MASK        0x03
#define    IPTOS_ECN(x)        ((x) & IPTOS_ECN_MASK)
#define    IPTOS_ECN_NOT_ECT    0x00
#define    IPTOS_ECN_ECT1        0x01
#define    IPTOS_ECN_ECT0        0x02
#define    IPTOS_ECN_CE        0x03

#define    IPTOS_DSCP_MASK        0xfc
#define    IPTOS_DSCP(x)        ((x) & IPTOS_DSCP_MASK)
#define    IPTOS_DSCP_AF11        0x28
#define    IPTOS_DSCP_AF12        0x30
#define    IPTOS_DSCP_AF13        0x38
#define    IPTOS_DSCP_AF21        0x48
#define    IPTOS_DSCP_AF22        0x50
#define    IPTOS_DSCP_AF23        0x58
#define    IPTOS_DSCP_AF31        0x68
#define    IPTOS_DSCP_AF32        0x70
#define    IPTOS_DSCP_AF33        0x78
#define    IPTOS_DSCP_AF41        0x88
#define    IPTOS_DSCP_AF42        0x90
#define    IPTOS_DSCP_AF43        0x98
#define    IPTOS_DSCP_EF        0xb8

#define    IPTOS_TOS_MASK        0x1E
#define    IPTOS_TOS(tos)        ((tos) & IPTOS_TOS_MASK)
#define    IPTOS_LOWDELAY        0x10
#define    IPTOS_THROUGHPUT    0x08
#define    IPTOS_RELIABILITY    0x04
#define    IPTOS_LOWCOST        0x02
#define    IPTOS_MINCOST        IPTOS_LOWCOST

#define    IPTOS_PREC_MASK            0xe0
#define    IPTOS_PREC(tos)                ((tos) & IPTOS_PREC_MASK)
#define    IPTOS_PREC_NETCONTROL        0xe0
#define    IPTOS_PREC_INTERNETCONTROL    0xc0
#define    IPTOS_PREC_CRITIC_ECP        0xa0
#define    IPTOS_PREC_FLASHOVERRIDE    0x80
#define    IPTOS_PREC_FLASH        0x60
#define    IPTOS_PREC_IMMEDIATE        0x40
#define    IPTOS_PREC_PRIORITY        0x20
#define    IPTOS_PREC_ROUTINE        0x00

#define    IPOPT_COPY        0x80
#define    IPOPT_CLASS_MASK    0x60
#define    IPOPT_NUMBER_MASK    0x1f

#define    IPOPT_COPIED(o)        ((o) & IPOPT_COPY)
#define    IPOPT_CLASS(o)        ((o) & IPOPT_CLASS_MASK)
#define    IPOPT_NUMBER(o)        ((o) & IPOPT_NUMBER_MASK)

#define    IPOPT_CONTROL        0x00
#define    IPOPT_RESERVED1        0x20
#define    IPOPT_DEBMEAS        0x40
#define    IPOPT_MEASUREMENT       IPOPT_DEBMEAS
#define    IPOPT_RESERVED2        0x60

#define    IPOPT_EOL        0
#define    IPOPT_END        IPOPT_EOL
#define    IPOPT_NOP        1
#define    IPOPT_NOOP        IPOPT_NOP

#define    IPOPT_RR        7
#define    IPOPT_TS        68
#define    IPOPT_TIMESTAMP        IPOPT_TS
#define    IPOPT_SECURITY        130
#define    IPOPT_SEC        IPOPT_SECURITY
#define    IPOPT_LSRR        131
#define    IPOPT_SATID        136
#define    IPOPT_SID        IPOPT_SATID
#define    IPOPT_SSRR        137
#define    IPOPT_RA        148

#define    IPOPT_OPTVAL        0
#define    IPOPT_OLEN        1
#define    IPOPT_OFFSET        2
#define    IPOPT_MINOFF        4

#define    MAX_IPOPTLEN        40

#define    IPOPT_TS_TSONLY        0
#define    IPOPT_TS_TSANDADDR    1
#define    IPOPT_TS_PRESPEC    3

#define    IPOPT_SECUR_UNCLASS    0x0000
#define    IPOPT_SECUR_CONFID    0xf135
#define    IPOPT_SECUR_EFTO    0x789a
#define    IPOPT_SECUR_MMMM    0xbc4d
#define    IPOPT_SECUR_RESTR    0xaf13
#define    IPOPT_SECUR_SECRET    0xd788
#define    IPOPT_SECUR_TOPSECRET    0x6bc5

#define    MAXTTL        255
#define    IPDEFTTL    64
#define    IPFRAGTTL    60
#define    IPTTLDEC    1

#define    IP_MSS        576

#endif

/// >>> START include/netinet/ip6.h
#ifndef _NETINET_IP6_H
#define _NETINET_IP6_H

struct ip6_hdr {
    union {
        struct ip6_hdrctl {
            uint32_t ip6_un1_flow;
            uint16_t ip6_un1_plen;
            uint8_t  ip6_un1_nxt;
            uint8_t  ip6_un1_hlim;
        } ip6_un1;
        uint8_t ip6_un2_vfc;
    } ip6_ctlun;
    struct in6_addr ip6_src;
    struct in6_addr ip6_dst;
};

#define ip6_vfc   ip6_ctlun.ip6_un2_vfc
#define ip6_flow  ip6_ctlun.ip6_un1.ip6_un1_flow
#define ip6_plen  ip6_ctlun.ip6_un1.ip6_un1_plen
#define ip6_nxt   ip6_ctlun.ip6_un1.ip6_un1_nxt
#define ip6_hlim  ip6_ctlun.ip6_un1.ip6_un1_hlim
#define ip6_hops  ip6_ctlun.ip6_un1.ip6_un1_hlim

struct ip6_ext {
    uint8_t  ip6e_nxt;
    uint8_t  ip6e_len;
};

struct ip6_hbh {
    uint8_t  ip6h_nxt;
    uint8_t  ip6h_len;
};

struct ip6_dest {
    uint8_t  ip6d_nxt;
    uint8_t  ip6d_len;
};

struct ip6_rthdr {
    uint8_t  ip6r_nxt;
    uint8_t  ip6r_len;
    uint8_t  ip6r_type;
    uint8_t  ip6r_segleft;
};

struct ip6_rthdr0 {
    uint8_t  ip6r0_nxt;
    uint8_t  ip6r0_len;
    uint8_t  ip6r0_type;
    uint8_t  ip6r0_segleft;
    uint8_t  ip6r0_reserved;
    uint8_t  ip6r0_slmap[3];
    struct in6_addr ip6r0_addr[];
};

struct ip6_frag {
    uint8_t   ip6f_nxt;
    uint8_t   ip6f_reserved;
    uint16_t  ip6f_offlg;
    uint32_t  ip6f_ident;
};

#if __BYTE_ORDER == __BIG_ENDIAN
#define IP6F_OFF_MASK       0xfff8
#define IP6F_RESERVED_MASK  0x0006
#define IP6F_MORE_FRAG      0x0001
#else
#define IP6F_OFF_MASK       0xf8ff
#define IP6F_RESERVED_MASK  0x0600
#define IP6F_MORE_FRAG      0x0100
#endif

struct ip6_opt {
    uint8_t  ip6o_type;
    uint8_t  ip6o_len;
};

#define IP6OPT_TYPE(o)        ((o) & 0xc0)
#define IP6OPT_TYPE_SKIP    0x00
#define IP6OPT_TYPE_DISCARD    0x40
#define IP6OPT_TYPE_FORCEICMP    0x80
#define IP6OPT_TYPE_ICMP    0xc0
#define IP6OPT_TYPE_MUTABLE    0x20

#define IP6OPT_PAD1    0
#define IP6OPT_PADN    1

#define IP6OPT_JUMBO        0xc2
#define IP6OPT_NSAP_ADDR    0xc3
#define IP6OPT_TUNNEL_LIMIT    0x04
#define IP6OPT_ROUTER_ALERT    0x05

struct ip6_opt_jumbo {
    uint8_t  ip6oj_type;
    uint8_t  ip6oj_len;
    uint8_t  ip6oj_jumbo_len[4];
};
#define IP6OPT_JUMBO_LEN    6

struct ip6_opt_nsap {
    uint8_t  ip6on_type;
    uint8_t  ip6on_len;
    uint8_t  ip6on_src_nsap_len;
    uint8_t  ip6on_dst_nsap_len;
};

struct ip6_opt_tunnel {
    uint8_t  ip6ot_type;
    uint8_t  ip6ot_len;
    uint8_t  ip6ot_encap_limit;
};

struct ip6_opt_router {
    uint8_t  ip6or_type;
    uint8_t  ip6or_len;
    uint8_t  ip6or_value[2];
};

#if __BYTE_ORDER == __BIG_ENDIAN
#define IP6_ALERT_MLD    0x0000
#define IP6_ALERT_RSVP    0x0001
#define IP6_ALERT_AN    0x0002
#else
#define IP6_ALERT_MLD    0x0000
#define IP6_ALERT_RSVP    0x0100
#define IP6_ALERT_AN    0x0200
#endif

#endif

/// >>> START include/netinet/ip_icmp.h
#ifndef _NETINET_IP_ICMP_H
#define _NETINET_IP_ICMP_H

struct icmphdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
        struct {
            uint16_t __unused;
            uint16_t mtu;
        } frag;
    } un;
};

#define ICMP_ECHOREPLY        0
#define ICMP_DEST_UNREACH    3
#define ICMP_SOURCE_QUENCH    4
#define ICMP_REDIRECT        5
#define ICMP_ECHO        8
#define ICMP_TIME_EXCEEDED    11
#define ICMP_PARAMETERPROB    12
#define ICMP_TIMESTAMP        13
#define ICMP_TIMESTAMPREPLY    14
#define ICMP_INFO_REQUEST    15
#define ICMP_INFO_REPLY        16
#define ICMP_ADDRESS        17
#define ICMP_ADDRESSREPLY    18
#define NR_ICMP_TYPES        18

#define ICMP_NET_UNREACH    0
#define ICMP_HOST_UNREACH    1
#define ICMP_PROT_UNREACH    2
#define ICMP_PORT_UNREACH    3
#define ICMP_FRAG_NEEDED    4
#define ICMP_SR_FAILED        5
#define ICMP_NET_UNKNOWN    6
#define ICMP_HOST_UNKNOWN    7
#define ICMP_HOST_ISOLATED    8
#define ICMP_NET_ANO        9
#define ICMP_HOST_ANO        10
#define ICMP_NET_UNR_TOS    11
#define ICMP_HOST_UNR_TOS    12
#define ICMP_PKT_FILTERED    13
#define ICMP_PREC_VIOLATION    14
#define ICMP_PREC_CUTOFF    15
#define NR_ICMP_UNREACH        15

#define ICMP_REDIR_NET        0
#define ICMP_REDIR_HOST        1
#define ICMP_REDIR_NETTOS    2
#define ICMP_REDIR_HOSTTOS    3

#define ICMP_EXC_TTL        0
#define ICMP_EXC_FRAGTIME    1

struct icmp_ra_addr {
    uint32_t ira_addr;
    uint32_t ira_preference;
};

struct icmp {
    uint8_t  icmp_type;
    uint8_t  icmp_code;
    uint16_t icmp_cksum;
    union {
        uint8_t ih_pptr;
        struct in_addr ih_gwaddr;
        struct ih_idseq {
            uint16_t icd_id;
            uint16_t icd_seq;
        } ih_idseq;
        uint32_t ih_void;

        struct ih_pmtu {
            uint16_t ipm_void;
            uint16_t ipm_nextmtu;
        } ih_pmtu;

        struct ih_rtradv {
            uint8_t irt_num_addrs;
            uint8_t irt_wpa;
            uint16_t irt_lifetime;
        } ih_rtradv;
    } icmp_hun;
    union {
        struct {
            uint32_t its_otime;
            uint32_t its_rtime;
            uint32_t its_ttime;
        } id_ts;
        struct {
            struct ip idi_ip;
        } id_ip;
        struct icmp_ra_addr id_radv;
        uint32_t   id_mask;
        uint8_t    id_data[1];
    } icmp_dun;
};

#define    icmp_pptr    icmp_hun.ih_pptr
#define    icmp_gwaddr    icmp_hun.ih_gwaddr
#define    icmp_id        icmp_hun.ih_idseq.icd_id
#define    icmp_seq    icmp_hun.ih_idseq.icd_seq
#define    icmp_void    icmp_hun.ih_void
#define    icmp_pmvoid    icmp_hun.ih_pmtu.ipm_void
#define    icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu
#define    icmp_num_addrs    icmp_hun.ih_rtradv.irt_num_addrs
#define    icmp_wpa    icmp_hun.ih_rtradv.irt_wpa
#define    icmp_lifetime    icmp_hun.ih_rtradv.irt_lifetime
#define    icmp_otime    icmp_dun.id_ts.its_otime
#define    icmp_rtime    icmp_dun.id_ts.its_rtime
#define    icmp_ttime    icmp_dun.id_ts.its_ttime
#define    icmp_ip        icmp_dun.id_ip.idi_ip
#define    icmp_radv    icmp_dun.id_radv
#define    icmp_mask    icmp_dun.id_mask
#define    icmp_data    icmp_dun.id_data

#define    ICMP_MINLEN    8
#define    ICMP_TSLEN    (8 + 3 * sizeof (n_time))
#define    ICMP_MASKLEN    12
#define    ICMP_ADVLENMIN    (8 + sizeof (struct ip) + 8)
#define    ICMP_ADVLEN(p)    (8 + ((p)->icmp_ip.ip_hl << 2) + 8)

#define    ICMP_UNREACH        3
#define    ICMP_SOURCEQUENCH    4
#define    ICMP_ROUTERADVERT    9
#define    ICMP_ROUTERSOLICIT    10
#define    ICMP_TIMXCEED        11
#define    ICMP_PARAMPROB        12
#define    ICMP_TSTAMP        13
#define    ICMP_TSTAMPREPLY    14
#define    ICMP_IREQ        15
#define    ICMP_IREQREPLY        16
#define    ICMP_MASKREQ        17
#define    ICMP_MASKREPLY        18
#define    ICMP_MAXTYPE        18

#define    ICMP_UNREACH_NET            0
#define    ICMP_UNREACH_HOST            1
#define    ICMP_UNREACH_PROTOCOL            2
#define    ICMP_UNREACH_PORT            3
#define    ICMP_UNREACH_NEEDFRAG            4
#define    ICMP_UNREACH_SRCFAIL            5
#define    ICMP_UNREACH_NET_UNKNOWN        6
#define    ICMP_UNREACH_HOST_UNKNOWN       7
#define    ICMP_UNREACH_ISOLATED            8
#define    ICMP_UNREACH_NET_PROHIB            9
#define    ICMP_UNREACH_HOST_PROHIB        10
#define    ICMP_UNREACH_TOSNET            11
#define    ICMP_UNREACH_TOSHOST            12
#define    ICMP_UNREACH_FILTER_PROHIB      13
#define    ICMP_UNREACH_HOST_PRECEDENCE    14
#define    ICMP_UNREACH_PRECEDENCE_CUTOFF  15

#define    ICMP_REDIRECT_NET    0
#define    ICMP_REDIRECT_HOST    1
#define    ICMP_REDIRECT_TOSNET    2
#define    ICMP_REDIRECT_TOSHOST    3

#define    ICMP_TIMXCEED_INTRANS    0
#define    ICMP_TIMXCEED_REASS    1

#define    ICMP_PARAMPROB_OPTABSENT 1

#define    ICMP_INFOTYPE(type) \
    ((type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO || \
    (type) == ICMP_ROUTERADVERT || (type) == ICMP_ROUTERSOLICIT || \
    (type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY || \
    (type) == ICMP_IREQ || (type) == ICMP_IREQREPLY || \
    (type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY)

#endif

/// >>> START include/netinet/tcp.h
#ifndef _NETINET_TCP_H
#define _NETINET_TCP_H

#define TCP_NODELAY 1
#define TCP_MAXSEG     2
#define TCP_CORK     3
#define TCP_KEEPIDLE     4
#define TCP_KEEPINTVL     5
#define TCP_KEEPCNT     6
#define TCP_SYNCNT     7
#define TCP_LINGER2     8
#define TCP_DEFER_ACCEPT 9
#define TCP_WINDOW_CLAMP 10
#define TCP_INFO     11
#define    TCP_QUICKACK     12
#define TCP_CONGESTION     13
#define TCP_MD5SIG     14
#define TCP_THIN_LINEAR_TIMEOUTS 16
#define TCP_THIN_DUPACK  17
#define TCP_USER_TIMEOUT 18
#define TCP_REPAIR       19
#define TCP_REPAIR_QUEUE 20
#define TCP_QUEUE_SEQ    21
#define TCP_REPAIR_OPTIONS 22
#define TCP_FASTOPEN     23
#define TCP_TIMESTAMP    24
#define TCP_NOTSENT_LOWAT 25

#define TCP_ESTABLISHED  1
#define TCP_SYN_SENT     2
#define TCP_SYN_RECV     3
#define TCP_FIN_WAIT1    4
#define TCP_FIN_WAIT2    5
#define TCP_TIME_WAIT    6
#define TCP_CLOSE        7
#define TCP_CLOSE_WAIT   8
#define TCP_LAST_ACK     9
#define TCP_LISTEN       10
#define TCP_CLOSING      11

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define SOL_TCP 6
#endif

#ifdef _GNU_SOURCE
struct tcphdr {
    u_int16_t source;
    u_int16_t dest;
    u_int32_t seq;
    u_int32_t ack_seq;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    u_int16_t res1:4;
    u_int16_t doff:4;
    u_int16_t fin:1;
    u_int16_t syn:1;
    u_int16_t rst:1;
    u_int16_t psh:1;
    u_int16_t ack:1;
    u_int16_t urg:1;
    u_int16_t res2:2;
#else
    u_int16_t doff:4;
    u_int16_t res1:4;
    u_int16_t res2:2;
    u_int16_t urg:1;
    u_int16_t ack:1;
    u_int16_t psh:1;
    u_int16_t rst:1;
    u_int16_t syn:1;
    u_int16_t fin:1;
#endif
    u_int16_t window;
    u_int16_t check;
    u_int16_t urg_ptr;
};

#define TCPI_OPT_TIMESTAMPS    1
#define TCPI_OPT_SACK        2
#define TCPI_OPT_WSCALE        4
#define TCPI_OPT_ECN        8

#define TCP_CA_Open        0
#define TCP_CA_Disorder        1
#define TCP_CA_CWR        2
#define TCP_CA_Recovery        3
#define TCP_CA_Loss        4

struct tcp_info {
    u_int8_t tcpi_state;
    u_int8_t tcpi_ca_state;
    u_int8_t tcpi_retransmits;
    u_int8_t tcpi_probes;
    u_int8_t tcpi_backoff;
    u_int8_t tcpi_options;
    u_int8_t tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;
    u_int32_t tcpi_rto;
    u_int32_t tcpi_ato;
    u_int32_t tcpi_snd_mss;
    u_int32_t tcpi_rcv_mss;
    u_int32_t tcpi_unacked;
    u_int32_t tcpi_sacked;
    u_int32_t tcpi_lost;
    u_int32_t tcpi_retrans;
    u_int32_t tcpi_fackets;
    u_int32_t tcpi_last_data_sent;
    u_int32_t tcpi_last_ack_sent;
    u_int32_t tcpi_last_data_recv;
    u_int32_t tcpi_last_ack_recv;
    u_int32_t tcpi_pmtu;
    u_int32_t tcpi_rcv_ssthresh;
    u_int32_t tcpi_rtt;
    u_int32_t tcpi_rttvar;
    u_int32_t tcpi_snd_ssthresh;
    u_int32_t tcpi_snd_cwnd;
    u_int32_t tcpi_advmss;
    u_int32_t tcpi_reordering;
    u_int32_t tcpi_rcv_rtt;
    u_int32_t tcpi_rcv_space;
    u_int32_t tcpi_total_retrans;
};

#define TCP_MD5SIG_MAXKEYLEN    80

struct tcp_md5sig {
    struct sockaddr_storage tcpm_addr;
    u_int16_t __tcpm_pad1;
    u_int16_t tcpm_keylen;
    u_int32_t __tcpm_pad2;
    u_int8_t tcpm_key[TCP_MD5SIG_MAXKEYLEN];
};

#endif

#endif

/// >>> START include/netinet/udp.h
#ifndef _NETINET_UDP_H
#define _NETINET_UDP_H

struct udphdr {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
};

#define uh_sport source
#define uh_dport dest
#define uh_ulen len
#define uh_sum check

#define UDP_CORK    1
#define UDP_ENCAP    100

#define UDP_ENCAP_ESPINUDP_NON_IKE 1
#define UDP_ENCAP_ESPINUDP    2
#define UDP_ENCAP_L2TPINUDP    3

#define SOL_UDP            17

#endif

/// >>> START include/netpacket/packet.h
#ifndef _NETPACKET_PACKET_H
#define _NETPACKET_PACKET_H

struct sockaddr_ll {
    unsigned short sll_family, sll_protocol;
    int sll_ifindex;
    unsigned short sll_hatype;
    unsigned char sll_pkttype, sll_halen;
    unsigned char sll_addr[8];
};

struct packet_mreq {
    int mr_ifindex;
    unsigned short int mr_type,  mr_alen;
    unsigned char mr_address[8];
};

#define PACKET_HOST        0
#define PACKET_BROADCAST    1
#define PACKET_MULTICAST    2
#define PACKET_OTHERHOST    3
#define PACKET_OUTGOING        4
#define PACKET_LOOPBACK        5
#define PACKET_FASTROUTE    6

#define PACKET_ADD_MEMBERSHIP        1
#define PACKET_DROP_MEMBERSHIP        2
#define    PACKET_RECV_OUTPUT        3
#define    PACKET_RX_RING            5
#define    PACKET_STATISTICS        6

#define PACKET_MR_MULTICAST    0
#define PACKET_MR_PROMISC    1
#define PACKET_MR_ALLMULTI    2

#endif

/// >>> START include/sys/epoll.h
#ifndef    _SYS_EPOLL_H
#define    _SYS_EPOLL_H

/// >>> START include/fcntl.h
#ifndef    _FCNTL_H
#define    _FCNTL_H

#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_mode_t

#ifdef _GNU_SOURCE
#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_struct_iovec
#endif

/// >>> START arch/x86_64/bits/fcntl.h
#define O_CREAT        0100
#define O_EXCL         0200
#define O_NOCTTY       0400
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DSYNC      010000
#define O_SYNC     04010000
#define O_RSYNC    04010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW  0400000
#define O_CLOEXEC  02000000

#define O_ASYNC      020000
#define O_DIRECT     040000
#define O_LARGEFILE       0
#define O_NOATIME  01000000
#define O_TMPFILE 020200000
#define O_NDELAY O_NONBLOCK

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4

#define F_SETOWN 8
#define F_GETOWN 9
#define F_SETSIG 10
#define F_GETSIG 11

#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7

#define F_SETOWN_EX 15
#define F_GETOWN_EX 16

#define F_GETOWNER_UIDS 17

/// >>> CONTINUE include/fcntl.h

struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

int creat(const char *, mode_t);
int fcntl(int, int, ...);
int open(const char *, int, ...);
int openat(int, const char *, int, ...);
int posix_fadvise(int, off_t, off_t, int);
int posix_fallocate(int, off_t, off_t);

#define O_SEARCH  010000000
#define O_EXEC    010000000
#define O_PATH    010000000

#define O_ACCMODE (03|O_SEARCH)
#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02

#define F_DUPFD_CLOEXEC 1030

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define FD_CLOEXEC 1

#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200

#define POSIX_FADV_NORMAL     0
#define POSIX_FADV_RANDOM     1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED   3
#define POSIX_FADV_DONTNEED   4
#define POSIX_FADV_NOREUSE    5

#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifndef S_IRUSR
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXU 0700
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IRWXG 0070
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_IRWXO 0007
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define AT_NO_AUTOMOUNT 0x800
#define AT_EMPTY_PATH 0x1000

#define FAPPEND O_APPEND
#define FFSYNC O_FSYNC
#define FASYNC O_ASYNC
#define FNONBLOCK O_NONBLOCK
#define FNDELAY O_NDELAY

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3

#define F_SETLEASE    1024
#define F_GETLEASE    1025
#define F_NOTIFY    1026
#define F_CANCELLK    1029
#define F_SETPIPE_SZ    1031
#define F_GETPIPE_SZ    1032

#define DN_ACCESS    0x00000001
#define DN_MODIFY    0x00000002
#define DN_CREATE    0x00000004
#define DN_DELETE    0x00000008
#define DN_RENAME    0x00000010
#define DN_ATTRIB    0x00000020
#define DN_MULTISHOT    0x80000000

int lockf(int, int, off_t);
#endif

#if defined(_GNU_SOURCE)
#define F_OWNER_TID 0
#define F_OWNER_PID 1
#define F_OWNER_PGRP 2
#define F_OWNER_GID 2
struct f_owner_ex {
    int type;
    pid_t pid;
};
#define FALLOC_FL_KEEP_SIZE 1
#define FALLOC_FL_PUNCH_HOLE 2
#define SYNC_FILE_RANGE_WAIT_BEFORE 1
#define SYNC_FILE_RANGE_WRITE 2
#define SYNC_FILE_RANGE_WAIT_AFTER 4
#define SPLICE_F_MOVE 1
#define SPLICE_F_NONBLOCK 2
#define SPLICE_F_MORE 4
#define SPLICE_F_GIFT 8
int fallocate(int, int, off_t, off_t);
#define fallocate64 fallocate
ssize_t readahead(int, off_t, size_t);
int sync_file_range(int, off_t, off_t, unsigned);
ssize_t vmsplice(int, const struct iovec *, size_t, unsigned);
ssize_t splice(int, off_t *, int, off_t *, size_t, unsigned);
ssize_t tee(int, int, size_t, unsigned);
#define loff_t off_t
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define F_GETLK64 F_GETLK
#define F_SETLK64 F_SETLK
#define F_SETLKW64 F_SETLKW
#define open64 open
#define openat64 openat
#define creat64 creat
#define lockf64 lockf
#define posix_fadvise64 posix_fadvise
#define posix_fallocate64 posix_fallocate
#define off64_t off_t
#endif

#endif

/// >>> CONTINUE include/sys/epoll.h

#define __NEED_sigset_t

#define EPOLL_CLOEXEC O_CLOEXEC
#define EPOLL_NONBLOCK O_NONBLOCK

enum EPOLL_EVENTS { __EPOLL_DUMMY };
#define EPOLLIN 0x001
#define EPOLLPRI 0x002
#define EPOLLOUT 0x004
#define EPOLLRDNORM 0x040
#define EPOLLRDBAND 0x080
#define EPOLLWRNORM 0x100
#define EPOLLWRBAND 0x200
#define EPOLLMSG 0x400
#define EPOLLERR 0x008
#define EPOLLHUP 0x010
#define EPOLLRDHUP 0x2000
#define EPOLLWAKEUP (1U<<29)
#define EPOLLONESHOT (1U<<30)
#define EPOLLET (1U<<31)

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;
    epoll_data_t data;
}
#ifdef __x86_64__
__attribute__ ((__packed__))
#endif
;

int epoll_create(int);
int epoll_create1(int);
int epoll_ctl(int, int, int, struct epoll_event *);
int epoll_wait(int, struct epoll_event *, int, int);
int epoll_pwait(int, struct epoll_event *, int, int, const sigset_t *);

#endif /* sys/epoll.h */

/// >>> START include/sys/eventfd.h
#ifndef _SYS_EVENTFD_H
#define _SYS_EVENTFD_H

typedef uint64_t eventfd_t;

#define EFD_SEMAPHORE 1
#define EFD_CLOEXEC O_CLOEXEC
#define EFD_NONBLOCK O_NONBLOCK

int eventfd(unsigned int, int);
int eventfd_read(int, eventfd_t *);
int eventfd_write(int, eventfd_t);

#endif /* sys/eventfd.h */

/// >>> START include/sys/inotify.h
#ifndef _SYS_INOTIFY_H
#define _SYS_INOTIFY_H

struct inotify_event {
    int wd;
    uint32_t mask, cookie, len;
    char name[];
};

#define IN_CLOEXEC O_CLOEXEC
#define IN_NONBLOCK O_NONBLOCK

#define IN_ACCESS        0x00000001
#define IN_MODIFY        0x00000002
#define IN_ATTRIB        0x00000004
#define IN_CLOSE_WRITE   0x00000008
#define IN_CLOSE_NOWRITE 0x00000010
#define IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
#define IN_OPEN          0x00000020
#define IN_MOVED_FROM    0x00000040
#define IN_MOVED_TO      0x00000080
#define IN_MOVE          (IN_MOVED_FROM | IN_MOVED_TO)
#define IN_CREATE        0x00000100
#define IN_DELETE        0x00000200
#define IN_DELETE_SELF   0x00000400
#define IN_MOVE_SELF     0x00000800
#define IN_ALL_EVENTS    0x00000fff

#define IN_UNMOUNT       0x00002000
#define IN_Q_OVERFLOW    0x00004000
#define IN_IGNORED       0x00008000

#define IN_ONLYDIR       0x01000000
#define IN_DONT_FOLLOW   0x02000000
#define IN_EXCL_UNLINK   0x04000000
#define IN_MASK_ADD      0x20000000

#define IN_ISDIR         0x40000000
#define IN_ONESHOT       0x80000000

int inotify_init(void);
int inotify_init1(int);
int inotify_add_watch(int, const char *, uint32_t);
int inotify_rm_watch(int, int);

#endif

/// >>> START include/sys/file.h
#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#define LOCK_SH    1
#define LOCK_EX    2
#define LOCK_NB    4
#define LOCK_UN    8

#define L_SET 0
#define L_INCR 1
#define L_XTND 2

int flock(int, int);

#endif

/// >>> START include/sys/mman.h
#ifndef    _SYS_MMAN_H
#define    _SYS_MMAN_H

#define __NEED_mode_t
#define __NEED_size_t
#define __NEED_off_t

#if defined(_GNU_SOURCE)
#define __NEED_ssize_t
#endif

/// >>> START arch/x86_64/bits/mman.h
#define MAP_FAILED ((void *) -1)

#define    PROT_NONE      0
#define    PROT_READ      1
#define    PROT_WRITE     2
#define    PROT_EXEC      4
#define PROT_GROWSDOWN  0x01000000
#define PROT_GROWSUP    0x02000000

#define    MAP_SHARED     0x01
#define    MAP_PRIVATE    0x02
#define    MAP_FIXED      0x10

#define MAP_TYPE       0x0f
#define MAP_FILE       0x00
#define MAP_ANON       0x20
#define MAP_ANONYMOUS  MAP_ANON
#define MAP_32BIT      0x40
#define MAP_NORESERVE  0x4000
#define MAP_GROWSDOWN  0x0100
#define MAP_DENYWRITE  0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED     0x2000
#define MAP_POPULATE   0x8000
#define MAP_NONBLOCK   0x10000
#define MAP_STACK      0x20000
#define MAP_HUGETLB    0x40000

#define POSIX_MADV_NORMAL       0
#define POSIX_MADV_RANDOM       1
#define POSIX_MADV_SEQUENTIAL   2
#define POSIX_MADV_WILLNEED     3
#define POSIX_MADV_DONTNEED     0

#define MS_ASYNC        1
#define MS_INVALIDATE   2
#define MS_SYNC         4

#define MCL_CURRENT     1
#define MCL_FUTURE      2

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define MADV_NORMAL      0
#define MADV_RANDOM      1
#define MADV_SEQUENTIAL  2
#define MADV_WILLNEED    3
#define MADV_DONTNEED    4
#define MADV_REMOVE      9
#define MADV_DONTFORK    10
#define MADV_DOFORK      11
#define MADV_MERGEABLE   12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE    14
#define MADV_NOHUGEPAGE  15
#define MADV_DONTDUMP    16
#define MADV_DODUMP      17
#define MADV_HWPOISON    100
#define MADV_SOFT_OFFLINE 101

#define MREMAP_MAYMOVE  1
#define MREMAP_FIXED    2
#endif

/// >>> CONTINUE include/sys/mman.h

void *mmap (void *, size_t, int, int, int, off_t);
int munmap (void *, size_t);

int mprotect (void *, size_t, int);
int msync (void *, size_t, int);

int posix_madvise (void *, size_t, int);

int mlock (const void *, size_t);
int munlock (const void *, size_t);
int mlockall (int);
int munlockall (void);

#ifdef _GNU_SOURCE
int remap_file_pages (void *, size_t, int, size_t, int);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int madvise (void *, size_t, int);
int mincore (void *, size_t, unsigned char *);
#endif

int shm_open (const char *, int, mode_t);
int shm_unlink (const char *);

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define mmap64 mmap
#define off64_t off_t
#endif

#endif

/// >>> START include/sys/mount.h
#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

/// >>> START include/sys/ioctl.h
#ifndef    _SYS_IOCTL_H
#define    _SYS_IOCTL_H

/// >>> START arch/x86_64/bits/ioctl.h
#define _IOC(a,b,c,d) ( ((a)<<30) | ((b)<<8) | (c) | ((d)<<16) )
#define _IOC_NONE  0U
#define _IOC_WRITE 1U
#define _IOC_READ  2U

#define _IO(a,b) _IOC(_IOC_NONE,(a),(b),0)
#define _IOW(a,b,c) _IOC(_IOC_WRITE,(a),(b),sizeof(c))
#define _IOR(a,b,c) _IOC(_IOC_READ,(a),(b),sizeof(c))
#define _IOWR(a,b,c) _IOC(_IOC_READ|_IOC_WRITE,(a),(b),sizeof(c))

#define TCGETS        0x5401
#define TCSETS        0x5402
#define TCSETSW        0x5403
#define TCSETSF        0x5404
#define TCGETA        0x5405
#define TCSETA        0x5406
#define TCSETAW        0x5407
#define TCSETAF        0x5408
#define TCSBRK        0x5409
#define TCXONC        0x540A
#define TCFLSH        0x540B
#define TIOCEXCL    0x540C
#define TIOCNXCL    0x540D
#define TIOCSCTTY    0x540E
#define TIOCGPGRP    0x540F
#define TIOCSPGRP    0x5410
#define TIOCOUTQ    0x5411
#define TIOCSTI        0x5412
#define TIOCGWINSZ    0x5413
#define TIOCSWINSZ    0x5414
#define TIOCMGET    0x5415
#define TIOCMBIS    0x5416
#define TIOCMBIC    0x5417
#define TIOCMSET    0x5418
#define TIOCGSOFTCAR    0x5419
#define TIOCSSOFTCAR    0x541A
#define FIONREAD    0x541B
#define TIOCINQ        FIONREAD
#define TIOCLINUX    0x541C
#define TIOCCONS    0x541D
#define TIOCGSERIAL    0x541E
#define TIOCSSERIAL    0x541F
#define TIOCPKT        0x5420
#define FIONBIO        0x5421
#define TIOCNOTTY    0x5422
#define TIOCSETD    0x5423
#define TIOCGETD    0x5424
#define TCSBRKP        0x5425
#define TIOCTTYGSTRUCT    0x5426
#define TIOCSBRK    0x5427
#define TIOCCBRK    0x5428
#define TIOCGSID    0x5429
#define TIOCGPTN    0x80045430
#define TIOCSPTLCK    0x40045431
#define TCGETX          0x5432
#define TCSETX          0x5433
#define TCSETXF         0x5434
#define TCSETXW         0x5435

#define FIONCLEX    0x5450
#define FIOCLEX        0x5451
#define FIOASYNC    0x5452
#define TIOCSERCONFIG    0x5453
#define TIOCSERGWILD    0x5454
#define TIOCSERSWILD    0x5455
#define TIOCGLCKTRMIOS    0x5456
#define TIOCSLCKTRMIOS    0x5457
#define TIOCSERGSTRUCT    0x5458
#define TIOCSERGETLSR   0x5459
#define TIOCSERGETMULTI 0x545A
#define TIOCSERSETMULTI 0x545B

#define TIOCMIWAIT    0x545C
#define TIOCGICOUNT    0x545D
#define TIOCGHAYESESP   0x545E
#define TIOCSHAYESESP   0x545F
#define FIOQSIZE    0x5460

#define TIOCPKT_DATA         0
#define TIOCPKT_FLUSHREAD     1
#define TIOCPKT_FLUSHWRITE     2
#define TIOCPKT_STOP         4
#define TIOCPKT_START         8
#define TIOCPKT_NOSTOP        16
#define TIOCPKT_DOSTOP        32
#define TIOCPKT_IOCTL           64

#define TIOCSER_TEMT    0x01

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

#define TIOCM_LE        0x001
#define TIOCM_DTR       0x002
#define TIOCM_RTS       0x004
#define TIOCM_ST        0x008
#define TIOCM_SR        0x010
#define TIOCM_CTS       0x020
#define TIOCM_CAR       0x040
#define TIOCM_RNG       0x080
#define TIOCM_DSR       0x100
#define TIOCM_CD        TIOCM_CAR
#define TIOCM_RI        TIOCM_RNG
#define TIOCM_OUT1      0x2000
#define TIOCM_OUT2      0x4000
#define TIOCM_LOOP      0x8000
#define TIOCM_MODEM_BITS TIOCM_OUT2

#define N_TTY           0
#define N_SLIP          1
#define N_MOUSE         2
#define N_PPP           3
#define N_STRIP         4
#define N_AX25          5
#define N_X25           6
#define N_6PACK         7
#define N_MASC          8
#define N_R3964         9
#define N_PROFIBUS_FDL  10
#define N_IRDA          11
#define N_SMSBLOCK      12
#define N_HDLC          13
#define N_SYNC_PPP      14
#define N_HCI           15

#define FIOSETOWN       0x8901
#define SIOCSPGRP       0x8902
#define FIOGETOWN       0x8903
#define SIOCGPGRP       0x8904
#define SIOCATMARK      0x8905
#define SIOCGSTAMP      0x8906

#define SIOCADDRT       0x890B
#define SIOCDELRT       0x890C
#define SIOCRTMSG       0x890D

#define SIOCGIFNAME     0x8910
#define SIOCSIFLINK     0x8911
#define SIOCGIFCONF     0x8912
#define SIOCGIFFLAGS    0x8913
#define SIOCSIFFLAGS    0x8914
#define SIOCGIFADDR     0x8915
#define SIOCSIFADDR     0x8916
#define SIOCGIFDSTADDR  0x8917
#define SIOCSIFDSTADDR  0x8918
#define SIOCGIFBRDADDR  0x8919
#define SIOCSIFBRDADDR  0x891a
#define SIOCGIFNETMASK  0x891b
#define SIOCSIFNETMASK  0x891c
#define SIOCGIFMETRIC   0x891d
#define SIOCSIFMETRIC   0x891e
#define SIOCGIFMEM      0x891f
#define SIOCSIFMEM      0x8920
#define SIOCGIFMTU      0x8921
#define SIOCSIFMTU      0x8922
#define SIOCSIFHWADDR   0x8924
#define SIOCGIFENCAP    0x8925
#define SIOCSIFENCAP    0x8926
#define SIOCGIFHWADDR   0x8927
#define SIOCGIFSLAVE    0x8929
#define SIOCSIFSLAVE    0x8930
#define SIOCADDMULTI    0x8931
#define SIOCDELMULTI    0x8932
#define SIOCGIFINDEX    0x8933
#define SIOGIFINDEX     SIOCGIFINDEX
#define SIOCSIFPFLAGS   0x8934
#define SIOCGIFPFLAGS   0x8935
#define SIOCDIFADDR     0x8936
#define SIOCSIFHWBROADCAST 0x8937
#define SIOCGIFCOUNT    0x8938

#define SIOCGIFBR       0x8940
#define SIOCSIFBR       0x8941

#define SIOCGIFTXQLEN   0x8942
#define SIOCSIFTXQLEN   0x8943

#define SIOCDARP        0x8953
#define SIOCGARP        0x8954
#define SIOCSARP        0x8955

#define SIOCDRARP       0x8960
#define SIOCGRARP       0x8961
#define SIOCSRARP       0x8962

#define SIOCGIFMAP      0x8970
#define SIOCSIFMAP      0x8971

#define SIOCADDDLCI     0x8980
#define SIOCDELDLCI     0x8981

#define SIOCDEVPRIVATE  0x89F0
#define SIOCPROTOPRIVATE 0x89E0

/// >>> CONTINUE include/sys/ioctl.h

int ioctl (int, int, ...);

#endif

/// >>> CONTINUE include/sys/mount.h

#define BLKROSET   _IO(0x12, 93)
#define BLKROGET   _IO(0x12, 94)
#define BLKRRPART  _IO(0x12, 95)
#define BLKGETSIZE _IO(0x12, 96)
#define BLKFLSBUF  _IO(0x12, 97)
#define BLKRASET   _IO(0x12, 98)
#define BLKRAGET   _IO(0x12, 99)
#define BLKFRASET  _IO(0x12,100)
#define BLKFRAGET  _IO(0x12,101)
#define BLKSECTSET _IO(0x12,102)
#define BLKSECTGET _IO(0x12,103)
#define BLKSSZGET  _IO(0x12,104)
#define BLKBSZGET  _IOR(0x12,112,size_t)
#define BLKBSZSET  _IOW(0x12,113,size_t)
#define BLKGETSIZE64 _IOR(0x12,114,size_t)

#define MS_RDONLY      1
#define MS_NOSUID      2
#define MS_NODEV       4
#define MS_NOEXEC      8
#define MS_SYNCHRONOUS 16
#define MS_REMOUNT     32
#define MS_MANDLOCK    64
#define MS_DIRSYNC     128
#define MS_NOATIME     1024
#define MS_NODIRATIME  2048
#define MS_BIND        4096
#define MS_MOVE        8192
#define MS_REC         16384
#define MS_SILENT      32768
#define MS_POSIXACL    (1<<16)
#define MS_UNBINDABLE  (1<<17)
#define MS_PRIVATE     (1<<18)
#define MS_SLAVE       (1<<19)
#define MS_SHARED      (1<<20)
#define MS_RELATIME    (1<<21)
#define MS_KERNMOUNT   (1<<22)
#define MS_I_VERSION   (1<<23)
#define MS_STRICTATIME (1<<24)
#define MS_NOSEC       (1<<28)
#define MS_BORN        (1<<29)
#define MS_ACTIVE      (1<<30)
#define MS_NOUSER      (1U<<31)

#define MS_RMT_MASK (MS_RDONLY|MS_SYNCHRONOUS|MS_MANDLOCK|MS_I_VERSION)

#define MS_MGC_VAL 0xc0ed0000
#define MS_MGC_MSK 0xffff0000

#define MNT_FORCE       1
#define MNT_DETACH      2
#define MNT_EXPIRE      4
#define UMOUNT_NOFOLLOW 8

int mount(const char *, const char *, const char *, unsigned long, const void *);
int umount(const char *);
int umount2(const char *, int);

#endif

/// >>> START include/sys/prctl.h
#ifndef _SYS_PRCTL_H
#define _SYS_PRCTL_H

#define PR_SET_PDEATHSIG  1
#define PR_GET_PDEATHSIG  2
#define PR_GET_DUMPABLE   3
#define PR_SET_DUMPABLE   4
#define PR_GET_UNALIGN   5
#define PR_SET_UNALIGN   6
#define PR_UNALIGN_NOPRINT 1
#define PR_UNALIGN_SIGBUS 2
#define PR_GET_KEEPCAPS   7
#define PR_SET_KEEPCAPS   8
#define PR_GET_FPEMU  9
#define PR_SET_FPEMU 10
#define PR_FPEMU_NOPRINT 1
#define PR_FPEMU_SIGFPE 2
#define PR_GET_FPEXC 11
#define PR_SET_FPEXC 12
#define PR_FP_EXC_SW_ENABLE 0x80
#define PR_FP_EXC_DIV  0x010000
#define PR_FP_EXC_OVF  0x020000
#define PR_FP_EXC_UND  0x040000
#define PR_FP_EXC_RES  0x080000
#define PR_FP_EXC_INV  0x100000
#define PR_FP_EXC_DISABLED 0
#define PR_FP_EXC_NONRECOV 1
#define PR_FP_EXC_ASYNC 2
#define PR_FP_EXC_PRECISE 3
#define PR_GET_TIMING   13
#define PR_SET_TIMING   14
#define PR_TIMING_STATISTICAL  0
#define PR_TIMING_TIMESTAMP    1
#define PR_SET_NAME    15
#define PR_GET_NAME    16
#define PR_GET_ENDIAN 19
#define PR_SET_ENDIAN 20
#define PR_ENDIAN_BIG 0
#define PR_ENDIAN_LITTLE 1
#define PR_ENDIAN_PPC_LITTLE 2
#define PR_GET_SECCOMP 21
#define PR_SET_SECCOMP 22
#define PR_CAPBSET_READ 23
#define PR_CAPBSET_DROP 24
#define PR_GET_TSC 25
#define PR_SET_TSC 26
#define PR_TSC_ENABLE 1
#define PR_TSC_SIGSEGV 2
#define PR_GET_SECUREBITS 27
#define PR_SET_SECUREBITS 28
#define PR_SET_TIMERSLACK 29
#define PR_GET_TIMERSLACK 30

#define PR_TASK_PERF_EVENTS_DISABLE             31
#define PR_TASK_PERF_EVENTS_ENABLE              32

#define PR_MCE_KILL     33
#define PR_MCE_KILL_CLEAR   0
#define PR_MCE_KILL_SET     1
#define PR_MCE_KILL_LATE    0
#define PR_MCE_KILL_EARLY   1
#define PR_MCE_KILL_DEFAULT 2
#define PR_MCE_KILL_GET 34

#define PR_SET_MM               35
#define PR_SET_MM_START_CODE           1
#define PR_SET_MM_END_CODE             2
#define PR_SET_MM_START_DATA           3
#define PR_SET_MM_END_DATA             4
#define PR_SET_MM_START_STACK          5
#define PR_SET_MM_START_BRK            6
#define PR_SET_MM_BRK                  7
#define PR_SET_MM_ARG_START            8
#define PR_SET_MM_ARG_END              9
#define PR_SET_MM_ENV_START            10
#define PR_SET_MM_ENV_END              11
#define PR_SET_MM_AUXV                 12
#define PR_SET_MM_EXE_FILE             13

#define PR_SET_PTRACER 0x59616d61
#define PR_SET_PTRACER_ANY (-1UL)

#define PR_SET_CHILD_SUBREAPER  36
#define PR_GET_CHILD_SUBREAPER  37

#define PR_SET_NO_NEW_PRIVS     38
#define PR_GET_NO_NEW_PRIVS     39

#define PR_GET_TID_ADDRESS      40

int prctl (int, ...);

#endif

/// >>> START include/sys/shm.h
#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#define __NEED_time_t
#define __NEED_size_t
#define __NEED_pid_t

/// >>> START include/sys/ipc.h
#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_mode_t
#define __NEED_key_t

#define __ipc_perm_key __key
#define __ipc_perm_seq __seq

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __key key
#define __seq seq
#endif

/// >>> START arch/x86_64/bits/ipc.h
struct ipc_perm {
    key_t __ipc_perm_key;
    uid_t uid;
    gid_t gid;
    uid_t cuid;
    gid_t cgid;
    mode_t mode;
    int __ipc_perm_seq;
    long __pad1;
    long __pad2;
};

#define IPC_64 0

/// >>> CONTINUE include/sys/ipc.h

#define IPC_CREAT  01000
#define IPC_EXCL   02000
#define IPC_NOWAIT 04000

#define IPC_RMID 0
#define IPC_SET  1
#define IPC_STAT 2
#define IPC_INFO 3

#define IPC_PRIVATE ((key_t) 0)

key_t ftok (const char *, int);

#endif

/// >>> CONTINUE include/sys/shm.h
/// >>> START arch/x86_64/bits/shm.h
#define SHMLBA 4096

struct shmid_ds {
    struct ipc_perm shm_perm;
    size_t shm_segsz;
    time_t shm_atime;
    time_t shm_dtime;
    time_t shm_ctime;
    pid_t shm_cpid;
    pid_t shm_lpid;
    unsigned long shm_nattch;
    unsigned long __pad1;
    unsigned long __pad2;
};

/// >>> CONTINUE include/sys/shm.h

#define SHM_R 0400
#define SHM_W 0200

#define SHM_RDONLY 010000
#define SHM_RND    020000
#define SHM_REMAP  040000
#define SHM_EXEC   0100000

#define SHM_LOCK 11
#define SHM_UNLOCK 12
#define SHM_STAT 13
#define SHM_INFO 14
#define SHM_DEST 01000
#define SHM_LOCKED 02000
#define SHM_HUGETLB 04000
#define SHM_NORESERVE 010000

struct shminfo {
    unsigned long shmmax, shmmin, shmmni, shmseg, shmall, __unused[4];
};

#ifdef _GNU_SOURCE
#define __used_ids used_ids
#define __swap_attempts swap_attempts
#define __swap_successes swap_successes
#endif

struct shm_info {
    int __used_ids;
    unsigned long shm_tot, shm_rss, shm_swp;
    unsigned long __swap_attempts, __swap_successes;
};

typedef unsigned long shmatt_t;

void *shmat(int, const void *, int);
int shmctl(int, int, struct shmid_ds *);
int shmdt(const void *);
int shmget(key_t, size_t, int);

#endif

/// >>> START include/sys/signalfd.h
#ifndef _SYS_SIGNALFD_H
#define _SYS_SIGNALFD_H

#define __NEED_sigset_t

#define SFD_CLOEXEC O_CLOEXEC
#define SFD_NONBLOCK O_NONBLOCK

int signalfd(int, const sigset_t *, int);

struct signalfd_siginfo {
    uint32_t  ssi_signo;
    int32_t   ssi_errno;
    int32_t   ssi_code;
    uint32_t  ssi_pid;
    uint32_t  ssi_uid;
    int32_t   ssi_fd;
    uint32_t  ssi_tid;
    uint32_t  ssi_band;
    uint32_t  ssi_overrun;
    uint32_t  ssi_trapno;
    int32_t   ssi_status;
    int32_t   ssi_int;
    uint64_t  ssi_ptr;
    uint64_t  ssi_utime;
    uint64_t  ssi_stime;
    uint64_t  ssi_addr;
    uint16_t  ssi_addr_lsb;
    uint8_t   pad[128-12*4-4*8-2];
};

#endif

/// >>> START include/sys/statfs.h
#ifndef    _SYS_STATFS_H
#define    _SYS_STATFS_H

/// >>> START include/sys/statvfs.h
#ifndef    _SYS_STATVFS_H
#define    _SYS_STATVFS_H

#define __NEED_fsblkcnt_t
#define __NEED_fsfilcnt_t

struct statvfs {
    unsigned long f_bsize, f_frsize;
    fsblkcnt_t f_blocks, f_bfree, f_bavail;
    fsfilcnt_t f_files, f_ffree, f_favail;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned long f_fsid;
    unsigned :8*(2*sizeof(int)-sizeof(long));
#else
    unsigned :8*(2*sizeof(int)-sizeof(long));
    unsigned long f_fsid;
#endif
    unsigned long f_flag, f_namemax;
    int __reserved[6];
};

int statvfs (const char *__restrict, struct statvfs *__restrict);
int fstatvfs (int, struct statvfs *);

#define ST_RDONLY 1
#define ST_NOSUID 2
#define ST_NODEV  4
#define ST_NOEXEC 8
#define ST_SYNCHRONOUS 16
#define ST_MANDLOCK    64
#define ST_WRITE       128
#define ST_APPEND      256
#define ST_IMMUTABLE   512
#define ST_NOATIME     1024
#define ST_NODIRATIME  2048

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define statvfs64 statvfs
#define fstatvfs64 fstatvfs
#define fsblkcnt64_t fsblkcnt_t
#define fsfilcnt64_t fsfilcnt_t
#endif

#endif

/// >>> CONTINUE include/sys/statfs.h

typedef struct __fsid_t {
    int __val[2];
} fsid_t;

/// >>> START arch/x86_64/bits/statfs.h
struct statfs {
    unsigned long f_type, f_bsize;
    fsblkcnt_t f_blocks, f_bfree, f_bavail;
    fsfilcnt_t f_files, f_ffree;
    fsid_t f_fsid;
    unsigned long f_namelen, f_frsize, f_flags, f_spare[4];
};

/// >>> CONTINUE include/sys/statfs.h

int statfs (const char *, struct statfs *);
int fstatfs (int, struct statfs *);

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define statfs64 statfs
#define fstatfs64 fstatfs
#define fsblkcnt64_t fsblkcnt_t
#define fsfilcnt64_t fsfilcnt_t
#endif

#endif

/// >>> START include/sys/sysinfo.h
#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H

#define SI_LOAD_SHIFT 16

struct sysinfo {
    unsigned long uptime;
    unsigned long loads[3];
    unsigned long totalram;
    unsigned long freeram;
    unsigned long sharedram;
    unsigned long bufferram;
    unsigned long totalswap;
    unsigned long freeswap;
    unsigned short procs, pad;
    unsigned long totalhigh;
    unsigned long freehigh;
    unsigned mem_unit;
    char __reserved[256];
};

int sysinfo (struct sysinfo *);
int get_nprocs_conf (void);
int get_nprocs (void);
long get_phys_pages (void);
long get_avphys_pages (void);

#endif

/// >>> START include/sys/time.h
#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};

int getitimer (int, struct itimerval *);
int setitimer (int, const struct itimerval *__restrict, struct itimerval *__restrict);
int utimes (const char *, const struct timeval [2]);

#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
int futimes(int, const struct timeval [2]);
int futimesat(int, const char *, const struct timeval [2]);
int lutimes(const char *, const struct timeval [2]);
int settimeofday(const struct timeval *, const struct timezone *);
int adjtime (const struct timeval *, struct timeval *);
#define timerisset(t) ((t)->tv_sec || (t)->tv_usec)
#define timerclear(t) ((t)->tv_sec = (t)->tv_usec = 0)
#define timercmp(s,t,op) ((s)->tv_sec == (t)->tv_sec ? \
    (s)->tv_usec op (t)->tv_usec : (s)->tv_sec op (t)->tv_sec)
#define timeradd(s,t,a) (void) ( (a)->tv_sec = (s)->tv_sec + (t)->tv_sec, \
    ((a)->tv_usec = (s)->tv_usec + (t)->tv_usec) >= 1000000 && \
    ((a)->tv_usec -= 1000000, (a)->tv_sec++) )
#define timersub(s,t,a) (void) ( (a)->tv_sec = (s)->tv_sec - (t)->tv_sec, \
    ((a)->tv_usec = (s)->tv_usec - (t)->tv_usec) < 0 && \
    ((a)->tv_usec += 1000000, (a)->tv_sec--) )
#endif

#endif

/// >>> START include/sys/timerfd.h
#ifndef _SYS_TIMERFD_H
#define _SYS_TIMERFD_H

#define TFD_NONBLOCK O_NONBLOCK
#define TFD_CLOEXEC O_CLOEXEC

#define TFD_TIMER_ABSTIME 1

int timerfd_create(int, int);
int timerfd_settime(int, int, const struct itimerspec *, struct itimerspec *);
int timerfd_gettime(int, struct itimerspec *);

#endif

/// >>> START include/sys/times.h
#ifndef    _SYS_TIMES_H
#define    _SYS_TIMES_H

#define __NEED_clock_t

struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

clock_t times (struct tms *);

#endif

/// >>> START include/sys/un.h
#ifndef    _SYS_UN_H
#define    _SYS_UN_H

#define __NEED_sa_family_t
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_size_t
#endif

struct sockaddr_un {
    sa_family_t sun_family;
    char sun_path[108];
};

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
size_t strlen(const char *);
#define SUN_LEN(s) (2+strlen((s)->sun_path))
#endif

#endif

/// >>> START include/sys/utsname.h
#ifndef    _SYS_UTSNAME_H
#define    _SYS_UTSNAME_H

struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
#ifdef _GNU_SOURCE
    char domainname[65];
#else
    char __domainname[65];
#endif
};

int uname (struct utsname *);

#endif

/// >>> START include/sys/wait.h
#ifndef    _SYS_WAIT_H
#define    _SYS_WAIT_H

#define __NEED_pid_t
#define __NEED_id_t

typedef enum {
    P_ALL = 0,
    P_PID = 1,
    P_PGID = 2
} idtype_t;

pid_t wait (int *);
pid_t waitpid (pid_t, int *, int );

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
/// >>> START include/sys/resource.h
#ifndef    _SYS_RESOURCE_H
#define    _SYS_RESOURCE_H

#define __NEED_id_t

#ifdef _GNU_SOURCE
#define __NEED_pid_t
#endif

typedef unsigned long long rlim_t;

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
    /* linux extentions, but useful */
    long    ru_maxrss;
    long    ru_ixrss;
    long    ru_idrss;
    long    ru_isrss;
    long    ru_minflt;
    long    ru_majflt;
    long    ru_nswap;
    long    ru_inblock;
    long    ru_oublock;
    long    ru_msgsnd;
    long    ru_msgrcv;
    long    ru_nsignals;
    long    ru_nvcsw;
    long    ru_nivcsw;
    /* room for more... */
    long    __reserved[16];
};

int getrlimit (int, struct rlimit *);
int setrlimit (int, const struct rlimit *);
int getrusage (int, struct rusage *);

int getpriority (int, id_t);
int setpriority (int, id_t, int);

#ifdef _GNU_SOURCE
int prlimit(pid_t, int, const struct rlimit *, struct rlimit *);
#define prlimit64 prlimit
#endif

#define PRIO_MIN (-20)
#define PRIO_MAX 20

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN 1

#define RLIM_INFINITY (~0ULL)
#define RLIM_SAVED_CUR RLIM_INFINITY
#define RLIM_SAVED_MAX RLIM_INFINITY

#define RLIMIT_CPU     0
#define RLIMIT_FSIZE   1
#define RLIMIT_DATA    2
#define RLIMIT_STACK   3
#define RLIMIT_CORE    4
#define RLIMIT_RSS     5
#define RLIMIT_NOFILE  7
#define RLIMIT_AS      9
#define RLIMIT_NPROC   6
#define RLIMIT_MEMLOCK 8
#define RLIMIT_LOCKS   10
#define RLIMIT_SIGPENDING 11
#define RLIMIT_MSGQUEUE 12
#define RLIMIT_NICE    13
#define RLIMIT_RTPRIO  14
#define RLIMIT_NLIMITS 15

#define RLIM_NLIMITS RLIMIT_NLIMITS

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define getrlimit64 getrlimit
#define setrlimit64 setrlimit
#define rlimit64 rlimit
#define rlim64_t rlim_t
#endif

#endif

/// >>> CONTINUE include/sys/wait.h
pid_t wait3 (int *, int, struct rusage *);
pid_t wait4 (pid_t, int *, int, struct rusage *);
#endif

#define WNOHANG    1
#define WUNTRACED  2

#define WSTOPPED   2
#define WEXITED    4
#define WCONTINUED 8
#define WNOWAIT    0x1000000

#define __WNOTHREAD 0x20000000
#define __WALL      0x40000000
#define __WCLONE    0x80000000

#define WEXITSTATUS(s) (((s) & 0xff00) >> 8)
#define WTERMSIG(s) ((s) & 0x7f)
#define WSTOPSIG(s) WEXITSTATUS(s)
#define WCOREDUMP(s) ((s) & 0x80)
#define WIFEXITED(s) (!WTERMSIG(s))
#define WIFSTOPPED(s) ((short)((((s)&0xffff)*0x10001)>>8) > 0x7f00)
#define WIFSIGNALED(s) (((s)&0xffff)-1 < 0xffu)
#define WIFCONTINUED(s) ((s) == 0xffff)

#endif

/// >>> START include/sys/xattr.h
#ifndef    _SYS_XATTR_H
#define    _SYS_XATTR_H

#define __NEED_ssize_t
#define __NEED_size_t

#define XATTR_CREATE 1
#define XATTR_REPLACE 2

ssize_t getxattr(const char *, const char *, void *, size_t);
ssize_t lgetxattr(const char *, const char *, void *, size_t);
ssize_t fgetxattr(int filedes, const char *, void *, size_t);
ssize_t listxattr(const char *, char *, size_t);
ssize_t llistxattr(const char *, char *, size_t);
ssize_t flistxattr(int filedes, char *, size_t);
int setxattr(const char *, const char *, const void *, size_t, int);
int lsetxattr(const char *, const char *, const void *, size_t, int);
int fsetxattr(int, const char *, const void *, size_t, int);
int removexattr(const char *, const char *);
int lremovexattr(const char *, const char *);
int fremovexattr(int, const char *);

#endif

/// >>> START src/complex/__cexp.c
/// >>> START src/internal/libm.h
/// >>> START include/float.h
#ifndef _FLOAT_H
#define _FLOAT_H

#define FLT_RADIX 2

#define FLT_TRUE_MIN 1.40129846432481707092e-45F
#define FLT_MIN 1.17549435082228750797e-38F
#define FLT_MAX 3.40282346638528859812e+38F
#define FLT_EPSILON 1.1920928955078125e-07F

#define FLT_MANT_DIG 24
#define FLT_MIN_EXP (-125)
#define FLT_MAX_EXP 128

#define FLT_DIG 6
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_10_EXP 38

#define DBL_TRUE_MIN 4.94065645841246544177e-324
#define DBL_MIN 2.22507385850720138309e-308
#define DBL_MAX 1.79769313486231570815e+308
#define DBL_EPSILON 2.22044604925031308085e-16

#define DBL_MANT_DIG 53
#define DBL_MIN_EXP (-1021)
#define DBL_MAX_EXP 1024

#define DBL_DIG 15
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_10_EXP 308

/// >>> START arch/x86_64/bits/float.h
#define FLT_ROUNDS 1
#ifdef __FLT_EVAL_METHOD__
#define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#else
#define FLT_EVAL_METHOD 0
#endif

#define LDBL_TRUE_MIN 3.6451995318824746025e-4951L
#define LDBL_MIN     3.3621031431120935063e-4932L
#define LDBL_MAX     1.1897314953572317650e+4932L
#define LDBL_EPSILON 1.0842021724855044340e-19L

#define LDBL_MANT_DIG 64
#define LDBL_MIN_EXP (-16381)
#define LDBL_MAX_EXP 16384

#define LDBL_DIG 18
#define LDBL_MIN_10_EXP (-4931)
#define LDBL_MAX_10_EXP 4932

#define DECIMAL_DIG 21

/// >>> CONTINUE include/float.h

#endif

/// >>> CONTINUE src/internal/libm.h
/// >>> START include/math.h
#ifndef _MATH_H
#define _MATH_H

#define __NEED_float_t
#define __NEED_double_t

#if 100*__GNUC__+__GNUC_MINOR__ >= 303
#else
#endif

#define HUGE_VALF INFINITY
#define HUGE_VAL  ((double)INFINITY)
#define HUGE_VALL ((long double)INFINITY)

#define MATH_ERRNO  1
#define MATH_ERREXCEPT 2
#define math_errhandling 2

#define FP_ILOGBNAN (-1-(int)(((unsigned)-1)>>1))
#define FP_ILOGB0 FP_ILOGBNAN

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

int __fpclassify(double);
int __fpclassifyf(float);
int __fpclassifyl(long double);

static __inline unsigned __FLOAT_BITS(float __f) {
    union {float __f; unsigned __i;} __u = {__f};
    return __u.__i;
}
static __inline unsigned long long __DOUBLE_BITS(double __f) {
    union {double __f; unsigned long long __i;} __u = {__f};
    return __u.__i;
}

#define fpclassify(x) ( \
    sizeof(x) == sizeof(float) ? __fpclassifyf(x) : \
    sizeof(x) == sizeof(double) ? __fpclassify(x) : \
    __fpclassifyl(x) )

#define isinf(x) ( \
    sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) == 0x7f800000 : \
    sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) == 0x7ffULL<<52 : \
    __fpclassifyl(x) == FP_INFINITE)

#define isnan(x) ( \
    sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000 : \
    sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) > 0x7ffULL<<52 : \
    __fpclassifyl(x) == FP_NAN)

#define isnormal(x) ( \
    sizeof(x) == sizeof(float) ? ((__FLOAT_BITS(x)+0x00800000) & 0x7fffffff) >= 0x01000000 : \
    sizeof(x) == sizeof(double) ? ((__DOUBLE_BITS(x)+(1ULL<<52)) & -1ULL>>1) >= 1ULL<<53 : \
    __fpclassifyl(x) == FP_NORMAL)

#define isfinite(x) ( \
    sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000 : \
    sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) < 0x7ffULL<<52 : \
    __fpclassifyl(x) > FP_INFINITE)

int __signbit(double);
int __signbitf(float);
int __signbitl(long double);

#define signbit(x) ( \
    sizeof(x) == sizeof(float) ? (int)(__FLOAT_BITS(x)>>31) : \
    sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x)>>63) : \
    __signbitl(x) )

#define isunordered(x,y) (isnan((x)) ? ((void)(y),1) : isnan((y)))

#define __ISREL_DEF(rel, op, type) \
static __inline int __is##rel(type __x, type __y) \
{ return !isunordered(__x,__y) && __x op __y; }

__ISREL_DEF(lessf, <, float_t)
__ISREL_DEF(less, <, double_t)
__ISREL_DEF(lessl, <, long double)
__ISREL_DEF(lessequalf, <=, float_t)
__ISREL_DEF(lessequal, <=, double_t)
__ISREL_DEF(lessequall, <=, long double)
__ISREL_DEF(lessgreaterf, !=, float_t)
__ISREL_DEF(lessgreater, !=, double_t)
__ISREL_DEF(lessgreaterl, !=, long double)
__ISREL_DEF(greaterf, >, float_t)
__ISREL_DEF(greater, >, double_t)
__ISREL_DEF(greaterl, >, long double)
__ISREL_DEF(greaterequalf, >=, float_t)
__ISREL_DEF(greaterequal, >=, double_t)
__ISREL_DEF(greaterequall, >=, long double)

#define __tg_pred_2(x, y, p) ( \
    sizeof((x)+(y)) == sizeof(float) ? p##f(x, y) : \
    sizeof((x)+(y)) == sizeof(double) ? p(x, y) : \
    p##l(x, y) )

#define isless(x, y)            __tg_pred_2(x, y, __isless)
#define islessequal(x, y)       __tg_pred_2(x, y, __islessequal)
#define islessgreater(x, y)     __tg_pred_2(x, y, __islessgreater)
#define isgreater(x, y)         __tg_pred_2(x, y, __isgreater)
#define isgreaterequal(x, y)    __tg_pred_2(x, y, __isgreaterequal)

double      acos(double);
float       acosf(float);
long double acosl(long double);

double      acosh(double);
float       acoshf(float);
long double acoshl(long double);

double      asin(double);
float       asinf(float);
long double asinl(long double);

double      asinh(double);
float       asinhf(float);
long double asinhl(long double);

double      atan(double);
float       atanf(float);
long double atanl(long double);

double      atan2(double, double);
float       atan2f(float, float);
long double atan2l(long double, long double);

double      atanh(double);
float       atanhf(float);
long double atanhl(long double);

double      cbrt(double);
float       cbrtf(float);
long double cbrtl(long double);

double      ceil(double);
float       ceilf(float);
long double ceill(long double);

double      copysign(double, double);
float       copysignf(float, float);
long double copysignl(long double, long double);

double      cos(double);
float       cosf(float);
long double cosl(long double);

double      cosh(double);
float       coshf(float);
long double coshl(long double);

double      erf(double);
float       erff(float);
long double erfl(long double);

double      erfc(double);
float       erfcf(float);
long double erfcl(long double);

double      exp(double);
float       expf(float);
long double expl(long double);

double      exp2(double);
float       exp2f(float);
long double exp2l(long double);

double      expm1(double);
float       expm1f(float);
long double expm1l(long double);

double      fabs(double);
float       fabsf(float);
long double fabsl(long double);

double      fdim(double, double);
float       fdimf(float, float);
long double fdiml(long double, long double);

double      floor(double);
float       floorf(float);
long double floorl(long double);

double      fma(double, double, double);
float       fmaf(float, float, float);
long double fmal(long double, long double, long double);

double      fmax(double, double);
float       fmaxf(float, float);
long double fmaxl(long double, long double);

double      fmin(double, double);
float       fminf(float, float);
long double fminl(long double, long double);

double      fmod(double, double);
float       fmodf(float, float);
long double fmodl(long double, long double);

double      frexp(double, int *);
float       frexpf(float, int *);
long double frexpl(long double, int *);

double      hypot(double, double);
float       hypotf(float, float);
long double hypotl(long double, long double);

int         ilogb(double);
int         ilogbf(float);
int         ilogbl(long double);

double      ldexp(double, int);
float       ldexpf(float, int);
long double ldexpl(long double, int);

double      lgamma(double);
float       lgammaf(float);
long double lgammal(long double);

long long   llrint(double);
long long   llrintf(float);
long long   llrintl(long double);

long long   llround(double);
long long   llroundf(float);
long long   llroundl(long double);

double      log(double);
float       logf(float);
long double logl(long double);

double      log10(double);
float       log10f(float);
long double log10l(long double);

double      log1p(double);
float       log1pf(float);
long double log1pl(long double);

double      log2(double);
float       log2f(float);
long double log2l(long double);

double      logb(double);
float       logbf(float);
long double logbl(long double);

long        lrint(double);
long        lrintf(float);
long        lrintl(long double);

long        lround(double);
long        lroundf(float);
long        lroundl(long double);

double      modf(double, double *);
float       modff(float, float *);
long double modfl(long double, long double *);

double      nan(const char *);
float       nanf(const char *);
long double nanl(const char *);

double      nearbyint(double);
float       nearbyintf(float);
long double nearbyintl(long double);

double      nextafter(double, double);
float       nextafterf(float, float);
long double nextafterl(long double, long double);

double      nexttoward(double, long double);
float       nexttowardf(float, long double);
long double nexttowardl(long double, long double);

double      pow(double, double);
float       powf(float, float);
long double powl(long double, long double);

double      remainder(double, double);
float       remainderf(float, float);
long double remainderl(long double, long double);

double      remquo(double, double, int *);
float       remquof(float, float, int *);
long double remquol(long double, long double, int *);

double      rint(double);
float       rintf(float);
long double rintl(long double);

double      round(double);
float       roundf(float);
long double roundl(long double);

double      scalbln(double, long);
float       scalblnf(float, long);
long double scalblnl(long double, long);

double      scalbn(double, int);
float       scalbnf(float, int);
long double scalbnl(long double, int);

double      sin(double);
float       sinf(float);
long double sinl(long double);

double      sinh(double);
float       sinhf(float);
long double sinhl(long double);

double      sqrt(double);
float       sqrtf(float);
long double sqrtl(long double);

double      tan(double);
float       tanf(float);
long double tanl(long double);

double      tanh(double);
float       tanhf(float);
long double tanhl(long double);

double      tgamma(double);
float       tgammaf(float);
long double tgammal(long double);

double      trunc(double);
float       truncf(float);
long double truncl(long double);

#if defined(_XOPEN_SOURCE) || defined(_BSD_SOURCE)
#undef  MAXFLOAT
#define MAXFLOAT        3.40282346638528859812e+38F
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define M_E             2.7182818284590452354   /* e */
#define M_LOG2E         1.4426950408889634074   /* log_2 e */
#define M_LOG10E        0.43429448190325182765  /* log_10 e */
#define M_LN2           0.69314718055994530942  /* log_e 2 */
#define M_LN10          2.30258509299404568402  /* log_e 10 */
#define M_PI            3.14159265358979323846  /* pi */
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#define M_PI_4          0.78539816339744830962  /* pi/4 */
#define M_1_PI          0.31830988618379067154  /* 1/pi */
#define M_2_PI          0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI      1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */

extern int signgam;

double      j0(double);
double      j1(double);
double      jn(int, double);

double      y0(double);
double      y1(double);
double      yn(int, double);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define HUGE            3.40282346638528859812e+38F

double      drem(double, double);
float       dremf(float, float);

int         finite(double);
int         finitef(float);

double      scalb(double, double);
float       scalbf(float, float);

double      significand(double);
float       significandf(float);

double      lgamma_r(double, int*);
float       lgammaf_r(float, int*);

float       j0f(float);
float       j1f(float);
float       jnf(int, float);

float       y0f(float);
float       y1f(float);
float       ynf(int, float);
#endif

#ifdef _GNU_SOURCE
long double lgammal_r(long double, int*);

void        sincos(double, double*, double*);
void        sincosf(float, float*, float*);
void        sincosl(long double, long double*, long double*);

double      exp10(double);
float       exp10f(float);
long double exp10l(long double);

double      pow10(double);
float       pow10f(float);
long double pow10l(long double);
#endif

#endif

/// >>> CONTINUE src/internal/libm.h
/// >>> START include/complex.h
#ifndef _COMPLEX_H
#define _COMPLEX_H

#define complex _Complex
#ifdef __GNUC__
#define _Complex_I (__extension__ 1.0fi)
#else
#define _Complex_I 1.0fi
#endif
#define I _Complex_I

double complex cacos(double complex);
float complex cacosf(float complex);
long double complex cacosl(long double complex);

double complex casin(double complex);
float complex casinf(float complex);
long double complex casinl(long double complex);

double complex catan(double complex);
float complex catanf(float complex);
long double complex catanl(long double complex);

double complex ccos(double complex);
float complex ccosf(float complex);
long double complex ccosl(long double complex);

double complex csin(double complex);
float complex csinf(float complex);
long double complex csinl(long double complex);

double complex ctan(double complex);
float complex ctanf(float complex);
long double complex ctanl(long double complex);

double complex cacosh(double complex);
float complex cacoshf(float complex);
long double complex cacoshl(long double complex);

double complex casinh(double complex);
float complex casinhf(float complex);
long double complex casinhl(long double complex);

double complex catanh(double complex);
float complex catanhf(float complex);
long double complex catanhl(long double complex);

double complex ccosh(double complex);
float complex ccoshf(float complex);
long double complex ccoshl(long double complex);

double complex csinh(double complex);
float complex csinhf(float complex);
long double complex csinhl(long double complex);

double complex ctanh(double complex);
float complex ctanhf(float complex);
long double complex ctanhl(long double complex);

double complex cexp(double complex);
float complex cexpf(float complex);
long double complex cexpl(long double complex);

double complex clog(double complex);
float complex clogf(float complex);
long double complex clogl(long double complex);

double cabs(double complex);
float cabsf(float complex);
long double cabsl(long double complex);

double complex cpow(double complex, double complex);
float complex cpowf(float complex, float complex);
long double complex cpowl(long double complex, long double complex);

double complex csqrt(double complex);
float complex csqrtf(float complex);
long double complex csqrtl(long double complex);

double carg(double complex);
float cargf(float complex);
long double cargl(long double complex);

double cimag(double complex);
float cimagf(float complex);
long double cimagl(long double complex);

double complex conj(double complex);
float complex conjf(float complex);
long double complex conjl(long double complex);

double complex cproj(double complex);
float complex cprojf(float complex);
long double complex cprojl(long double complex);

double creal(double complex);
float crealf(float complex);
long double creall(long double complex);

#define __CIMAG(x, t) \
    ((union { _Complex t __z; t __xy[2]; }){(_Complex t)(x)}.__xy[1])

#define creal(x) ((double)(x))
#define crealf(x) ((float)(x))
#define creall(x) ((long double)(x))

#define cimag(x) __CIMAG(x, double)
#define cimagf(x) __CIMAG(x, float)
#define cimagl(x) __CIMAG(x, long double)

#define __CMPLX(x, y, t) \
    ((union { _Complex t __z; t __xy[2]; }){.__xy = {(x),(y)}}.__z)

#define CMPLX(x, y) __CMPLX(x, y, double)
#define CMPLXF(x, y) __CMPLX(x, y, float)
#define CMPLXL(x, y) __CMPLX(x, y, long double)

#endif

/// >>> CONTINUE src/internal/libm.h

/// >>> CONTINUE src/complex/__cexp.c

/// >>> START src/conf/confstr.c
/// >>> START include/unistd.h
#ifndef    _UNISTD_H
#define    _UNISTD_H

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_intptr_t
#define __NEED_useconds_t

int pipe(int [2]);
int pipe2(int [2], int);
int close(int);
int posix_close(int, int);
int dup(int);
int dup2(int, int);
int dup3(int, int, int);
off_t lseek(int, off_t, int);
int fsync(int);
int fdatasync(int);

ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);
ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);

int chown(const char *, uid_t, gid_t);
int fchown(int, uid_t, gid_t);
int lchown(const char *, uid_t, gid_t);
int fchownat(int, const char *, uid_t, gid_t, int);

int link(const char *, const char *);
int linkat(int, const char *, int, const char *, int);
int symlink(const char *, const char *);
int symlinkat(const char *, int, const char *);
ssize_t readlink(const char *__restrict, char *__restrict, size_t);
ssize_t readlinkat(int, const char *__restrict, char *__restrict, size_t);
int unlink(const char *);
int unlinkat(int, const char *, int);
int rmdir(const char *);
int truncate(const char *, off_t);
int ftruncate(int, off_t);

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

int access(const char *, int);
int faccessat(int, const char *, int, int);

int chdir(const char *);
int fchdir(int);
char *getcwd(char *, size_t);

unsigned alarm(unsigned);
unsigned sleep(unsigned);
int pause(void);

pid_t fork(void);
int execve(const char *, char *const [], char *const []);
int execv(const char *, char *const []);
int execle(const char *, const char *, ...);
int execl(const char *, const char *, ...);
int execvp(const char *, char *const []);
int execlp(const char *, const char *, ...);
int fexecve(int, char *const [], char *const []);
_Noreturn void _exit(int);

pid_t getpid(void);
pid_t getppid(void);
pid_t getpgrp(void);
pid_t getpgid(pid_t);
int setpgid(pid_t, pid_t);
pid_t setsid(void);
pid_t getsid(pid_t);
char *ttyname(int);
int ttyname_r(int, char *, size_t);
int isatty(int);
pid_t tcgetpgrp(int);
int tcsetpgrp(int, pid_t);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int getgroups(int, gid_t []);
int setuid(uid_t);
int setreuid(uid_t, uid_t);
int seteuid(uid_t);
int setgid(gid_t);
int setregid(gid_t, gid_t);
int setegid(gid_t);

char *getlogin(void);
int getlogin_r(char *, size_t);
int gethostname(char *, size_t);
char *ctermid(char *);

int getopt(int, char * const [], const char *);
extern char *optarg;
extern int optind, opterr, optopt;

long pathconf(const char *, int);
long fpathconf(int, int);
long sysconf(int);
size_t confstr(int, char *, size_t);

#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int lockf(int, int, off_t);
long gethostid(void);
int nice(int);
void sync(void);
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE)
pid_t setpgrp(void);
char *crypt(const char *, const char *);
void encrypt(char *, int);
void swab(const void *__restrict, void *__restrict, ssize_t);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) \
 || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700)
int usleep(unsigned);
unsigned ualarm(unsigned, unsigned);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define L_SET 0
#define L_INCR 1
#define L_XTND 2
int brk(void *);
void *sbrk(intptr_t);
pid_t vfork(void);
int vhangup(void);
int chroot(const char *);
int getpagesize(void);
int getdtablesize(void);
int sethostname(const char *, size_t);
int getdomainname(char *, size_t);
int setdomainname(const char *, size_t);
int setgroups(size_t, const gid_t *);
char *getpass(const char *);
int daemon(int, int);
void setusershell(void);
void endusershell(void);
char *getusershell(void);
int acct(const char *);
#endif

#ifdef _GNU_SOURCE
extern char **environ;
int setresuid(uid_t, uid_t, uid_t);
int setresgid(gid_t, gid_t, gid_t);
int getresuid(uid_t *, uid_t *, uid_t *);
int getresgid(gid_t *, gid_t *, gid_t *);
char *get_current_dir_name(void);
int euidaccess(const char *, int);
int eaccess(const char *, int);
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define lseek64 lseek
#define pread64 pread
#define pwrite64 pwrite
#define truncate64 truncate
#define ftruncate64 ftruncate
#define lockf64 lockf
#define off64_t off_t
#endif

#define POSIX_CLOSE_RESTART     0

#define _XOPEN_VERSION          700
#define _XOPEN_UNIX             1
#define _XOPEN_ENH_I18N         1

#define _POSIX_VERSION          200809L
#define _POSIX2_VERSION         _POSIX_VERSION

#define _POSIX_ADVISORY_INFO    _POSIX_VERSION
#define _POSIX_CHOWN_RESTRICTED 1
#define _POSIX_IPV6             _POSIX_VERSION
#define _POSIX_JOB_CONTROL      1
#define _POSIX_MAPPED_FILES     _POSIX_VERSION
#define _POSIX_MEMLOCK          _POSIX_VERSION
#define _POSIX_MEMLOCK_RANGE    _POSIX_VERSION
#define _POSIX_MEMORY_PROTECTION _POSIX_VERSION
#define _POSIX_MESSAGE_PASSING  _POSIX_VERSION
#define _POSIX_FSYNC            _POSIX_VERSION
#define _POSIX_NO_TRUNC         1
#define _POSIX_RAW_SOCKETS      _POSIX_VERSION
#define _POSIX_REALTIME_SIGNALS _POSIX_VERSION
#define _POSIX_REGEXP           1
#define _POSIX_SAVED_IDS        1
#define _POSIX_SHELL            1
#define _POSIX_SPAWN            _POSIX_VERSION
#define _POSIX_VDISABLE         0

#define _POSIX_THREADS          _POSIX_VERSION
#define _POSIX_THREAD_PROCESS_SHARED _POSIX_VERSION
#define _POSIX_THREAD_SAFE_FUNCTIONS _POSIX_VERSION
#define _POSIX_THREAD_ATTR_STACKADDR _POSIX_VERSION
#define _POSIX_THREAD_ATTR_STACKSIZE _POSIX_VERSION
#define _POSIX_THREAD_PRIORITY_SCHEDULING _POSIX_VERSION
#define _POSIX_THREAD_CPUTIME   _POSIX_VERSION
#define _POSIX_TIMERS           _POSIX_VERSION
#define _POSIX_TIMEOUTS         _POSIX_VERSION
#define _POSIX_MONOTONIC_CLOCK  _POSIX_VERSION
#define _POSIX_CPUTIME          _POSIX_VERSION
#define _POSIX_CLOCK_SELECTION  _POSIX_VERSION
#define _POSIX_BARRIERS         _POSIX_VERSION
#define _POSIX_SPIN_LOCKS       _POSIX_VERSION
#define _POSIX_READER_WRITER_LOCKS _POSIX_VERSION
#define _POSIX_ASYNCHRONOUS_IO  _POSIX_VERSION
#define _POSIX_SEMAPHORES       _POSIX_VERSION
#define _POSIX_SHARED_MEMORY_OBJECTS _POSIX_VERSION

#define _POSIX2_C_BIND          _POSIX_VERSION

/// >>> START arch/x86_64/bits/posix.h
#define _POSIX_V6_LP64_OFF64  1
#define _POSIX_V7_LP64_OFF64  1

/// >>> CONTINUE include/unistd.h

#define _PC_LINK_MAX    0
#define _PC_MAX_CANON    1
#define _PC_MAX_INPUT    2
#define _PC_NAME_MAX    3
#define _PC_PATH_MAX    4
#define _PC_PIPE_BUF    5
#define _PC_CHOWN_RESTRICTED    6
#define _PC_NO_TRUNC    7
#define _PC_VDISABLE    8
#define _PC_SYNC_IO    9
#define _PC_ASYNC_IO    10
#define _PC_PRIO_IO    11
#define _PC_SOCK_MAXBUF    12
#define _PC_FILESIZEBITS    13
#define _PC_REC_INCR_XFER_SIZE    14
#define _PC_REC_MAX_XFER_SIZE    15
#define _PC_REC_MIN_XFER_SIZE    16
#define _PC_REC_XFER_ALIGN    17
#define _PC_ALLOC_SIZE_MIN    18
#define _PC_SYMLINK_MAX    19
#define _PC_2_SYMLINKS    20

#define _SC_ARG_MAX    0
#define _SC_CHILD_MAX    1
#define _SC_CLK_TCK    2
#define _SC_NGROUPS_MAX    3
#define _SC_OPEN_MAX    4
#define _SC_STREAM_MAX    5
#define _SC_TZNAME_MAX    6
#define _SC_JOB_CONTROL    7
#define _SC_SAVED_IDS    8
#define _SC_REALTIME_SIGNALS    9
#define _SC_PRIORITY_SCHEDULING    10
#define _SC_TIMERS    11
#define _SC_ASYNCHRONOUS_IO    12
#define _SC_PRIORITIZED_IO    13
#define _SC_SYNCHRONIZED_IO    14
#define _SC_FSYNC    15
#define _SC_MAPPED_FILES    16
#define _SC_MEMLOCK    17
#define _SC_MEMLOCK_RANGE    18
#define _SC_MEMORY_PROTECTION    19
#define _SC_MESSAGE_PASSING    20
#define _SC_SEMAPHORES    21
#define _SC_SHARED_MEMORY_OBJECTS    22
#define _SC_AIO_LISTIO_MAX    23
#define _SC_AIO_MAX    24
#define _SC_AIO_PRIO_DELTA_MAX    25
#define _SC_DELAYTIMER_MAX    26
#define _SC_MQ_OPEN_MAX    27
#define _SC_MQ_PRIO_MAX    28
#define _SC_VERSION    29
#define _SC_PAGE_SIZE    30
#define _SC_PAGESIZE    30 /* !! */
#define _SC_RTSIG_MAX    31
#define _SC_SEM_NSEMS_MAX    32
#define _SC_SEM_VALUE_MAX    33
#define _SC_SIGQUEUE_MAX    34
#define _SC_TIMER_MAX    35
#define _SC_BC_BASE_MAX    36
#define _SC_BC_DIM_MAX    37
#define _SC_BC_SCALE_MAX    38
#define _SC_BC_STRING_MAX    39
#define _SC_COLL_WEIGHTS_MAX    40
#define _SC_EQUIV_CLASS_MAX    41
#define _SC_EXPR_NEST_MAX    42
#define _SC_LINE_MAX    43
#define _SC_RE_DUP_MAX    44
#define _SC_CHARCLASS_NAME_MAX    45
#define _SC_2_VERSION    46
#define _SC_2_C_BIND    47
#define _SC_2_C_DEV    48
#define _SC_2_FORT_DEV    49
#define _SC_2_FORT_RUN    50
#define _SC_2_SW_DEV    51
#define _SC_2_LOCALEDEF    52
#define _SC_PII    53
#define _SC_PII_XTI    54
#define _SC_PII_SOCKET    55
#define _SC_PII_INTERNET    56
#define _SC_PII_OSI    57
#define _SC_POLL    58
#define _SC_SELECT    59
#define _SC_UIO_MAXIOV    60 /* !! */
#define _SC_IOV_MAX    60
#define _SC_PII_INTERNET_STREAM    61
#define _SC_PII_INTERNET_DGRAM    62
#define _SC_PII_OSI_COTS    63
#define _SC_PII_OSI_CLTS    64
#define _SC_PII_OSI_M    65
#define _SC_T_IOV_MAX    66
#define _SC_THREADS    67
#define _SC_THREAD_SAFE_FUNCTIONS    68
#define _SC_GETGR_R_SIZE_MAX    69
#define _SC_GETPW_R_SIZE_MAX    70
#define _SC_LOGIN_NAME_MAX    71
#define _SC_TTY_NAME_MAX    72
#define _SC_THREAD_DESTRUCTOR_ITERATIONS    73
#define _SC_THREAD_KEYS_MAX    74
#define _SC_THREAD_STACK_MIN    75
#define _SC_THREAD_THREADS_MAX    76
#define _SC_THREAD_ATTR_STACKADDR    77
#define _SC_THREAD_ATTR_STACKSIZE    78
#define _SC_THREAD_PRIORITY_SCHEDULING    79
#define _SC_THREAD_PRIO_INHERIT    80
#define _SC_THREAD_PRIO_PROTECT    81
#define _SC_THREAD_PROCESS_SHARED    82
#define _SC_NPROCESSORS_CONF    83
#define _SC_NPROCESSORS_ONLN    84
#define _SC_PHYS_PAGES    85
#define _SC_AVPHYS_PAGES    86
#define _SC_ATEXIT_MAX    87
#define _SC_PASS_MAX    88
#define _SC_XOPEN_VERSION    89
#define _SC_XOPEN_XCU_VERSION    90
#define _SC_XOPEN_UNIX    91
#define _SC_XOPEN_CRYPT    92
#define _SC_XOPEN_ENH_I18N    93
#define _SC_XOPEN_SHM    94
#define _SC_2_CHAR_TERM    95
#define _SC_2_C_VERSION    96
#define _SC_2_UPE    97
#define _SC_XOPEN_XPG2    98
#define _SC_XOPEN_XPG3    99
#define _SC_XOPEN_XPG4    100
#define _SC_CHAR_BIT    101
#define _SC_CHAR_MAX    102
#define _SC_CHAR_MIN    103
#define _SC_INT_MAX    104
#define _SC_INT_MIN    105
#define _SC_LONG_BIT    106
#define _SC_WORD_BIT    107
#define _SC_MB_LEN_MAX    108
#define _SC_NZERO    109
#define _SC_SSIZE_MAX    110
#define _SC_SCHAR_MAX    111
#define _SC_SCHAR_MIN    112
#define _SC_SHRT_MAX    113
#define _SC_SHRT_MIN    114
#define _SC_UCHAR_MAX    115
#define _SC_UINT_MAX    116
#define _SC_ULONG_MAX    117
#define _SC_USHRT_MAX    118
#define _SC_NL_ARGMAX    119
#define _SC_NL_LANGMAX    120
#define _SC_NL_MSGMAX    121
#define _SC_NL_NMAX    122
#define _SC_NL_SETMAX    123
#define _SC_NL_TEXTMAX    124
#define _SC_XBS5_ILP32_OFF32    125
#define _SC_XBS5_ILP32_OFFBIG    126
#define _SC_XBS5_LP64_OFF64    127
#define _SC_XBS5_LPBIG_OFFBIG    128
#define _SC_XOPEN_LEGACY    129
#define _SC_XOPEN_REALTIME    130
#define _SC_XOPEN_REALTIME_THREADS    131
#define _SC_ADVISORY_INFO    132
#define _SC_BARRIERS    133
#define _SC_BASE    134
#define _SC_C_LANG_SUPPORT    135
#define _SC_C_LANG_SUPPORT_R    136
#define _SC_CLOCK_SELECTION    137
#define _SC_CPUTIME    138
#define _SC_THREAD_CPUTIME    139
#define _SC_DEVICE_IO    140
#define _SC_DEVICE_SPECIFIC    141
#define _SC_DEVICE_SPECIFIC_R    142
#define _SC_FD_MGMT    143
#define _SC_FIFO    144
#define _SC_PIPE    145
#define _SC_FILE_ATTRIBUTES    146
#define _SC_FILE_LOCKING    147
#define _SC_FILE_SYSTEM    148
#define _SC_MONOTONIC_CLOCK    149
#define _SC_MULTI_PROCESS    150
#define _SC_SINGLE_PROCESS    151
#define _SC_NETWORKING    152
#define _SC_READER_WRITER_LOCKS    153
#define _SC_SPIN_LOCKS    154
#define _SC_REGEXP    155
#define _SC_REGEX_VERSION    156
#define _SC_SHELL    157
#define _SC_SIGNALS    158
#define _SC_SPAWN    159
#define _SC_SPORADIC_SERVER    160
#define _SC_THREAD_SPORADIC_SERVER    161
#define _SC_SYSTEM_DATABASE    162
#define _SC_SYSTEM_DATABASE_R    163
#define _SC_TIMEOUTS    164
#define _SC_TYPED_MEMORY_OBJECTS    165
#define _SC_USER_GROUPS    166
#define _SC_USER_GROUPS_R    167
#define _SC_2_PBS    168
#define _SC_2_PBS_ACCOUNTING    169
#define _SC_2_PBS_LOCATE    170
#define _SC_2_PBS_MESSAGE    171
#define _SC_2_PBS_TRACK    172
#define _SC_SYMLOOP_MAX    173
#define _SC_STREAMS    174
#define _SC_2_PBS_CHECKPOINT    175
#define _SC_V6_ILP32_OFF32    176
#define _SC_V6_ILP32_OFFBIG    177
#define _SC_V6_LP64_OFF64    178
#define _SC_V6_LPBIG_OFFBIG    179
#define _SC_HOST_NAME_MAX    180
#define _SC_TRACE    181
#define _SC_TRACE_EVENT_FILTER    182
#define _SC_TRACE_INHERIT    183
#define _SC_TRACE_LOG    184

#define _SC_IPV6    235
#define _SC_RAW_SOCKETS    236
#define _SC_V7_ILP32_OFF32    237
#define _SC_V7_ILP32_OFFBIG    238
#define _SC_V7_LP64_OFF64    239
#define _SC_V7_LPBIG_OFFBIG    240
#define _SC_SS_REPL_MAX    241
#define _SC_TRACE_EVENT_NAME_MAX    242
#define _SC_TRACE_NAME_MAX    243
#define _SC_TRACE_SYS_MAX    244
#define _SC_TRACE_USER_EVENT_MAX    245
#define _SC_XOPEN_STREAMS    246
#define _SC_THREAD_ROBUST_PRIO_INHERIT    247
#define _SC_THREAD_ROBUST_PRIO_PROTECT    248

#define _CS_PATH    0
#define _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS    1
#define _CS_GNU_LIBC_VERSION    2
#define _CS_GNU_LIBPTHREAD_VERSION    3
#define _CS_POSIX_V5_WIDTH_RESTRICTED_ENVS    4
#define _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS    5

#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS    1116
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS    1117
#define _CS_POSIX_V6_ILP32_OFF32_LIBS    1118
#define _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS    1119
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS    1120
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS    1121
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS    1122
#define _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS    1123
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS    1124
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS    1125
#define _CS_POSIX_V6_LP64_OFF64_LIBS    1126
#define _CS_POSIX_V6_LP64_OFF64_LINTFLAGS    1127
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS    1128
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS    1129
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS    1130
#define _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS    1131
#define _CS_POSIX_V7_ILP32_OFF32_CFLAGS    1132
#define _CS_POSIX_V7_ILP32_OFF32_LDFLAGS    1133
#define _CS_POSIX_V7_ILP32_OFF32_LIBS    1134
#define _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS    1135
#define _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS    1136
#define _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS    1137
#define _CS_POSIX_V7_ILP32_OFFBIG_LIBS    1138
#define _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS    1139
#define _CS_POSIX_V7_LP64_OFF64_CFLAGS    1140
#define _CS_POSIX_V7_LP64_OFF64_LDFLAGS    1141
#define _CS_POSIX_V7_LP64_OFF64_LIBS    1142
#define _CS_POSIX_V7_LP64_OFF64_LINTFLAGS    1143
#define _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS    1144
#define _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS    1145
#define _CS_POSIX_V7_LPBIG_OFFBIG_LIBS    1146
#define _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS    1147

#endif

/// >>> CONTINUE src/conf/confstr.c
/// >>> START include/stdio.h
#ifndef _STDIO_H
#define _STDIO_H

#define __NEED_FILE
#define __NEED___isoc_va_list
#define __NEED_size_t

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
#define __NEED_ssize_t
#define __NEED_off_t
#define __NEED_va_list
#endif

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#undef EOF
#define EOF (-1)

#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 1024
#define FILENAME_MAX 4096
#define FOPEN_MAX 1000
#define TMP_MAX 10000
#define L_tmpnam 20

typedef union _G_fpos64_t {
    char __opaque[16];
    double __align;
} fpos_t;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;

#define stdin  (stdin)
#define stdout (stdout)
#define stderr (stderr)

FILE *fopen(const char *__restrict, const char *__restrict);
FILE *freopen(const char *__restrict, const char *__restrict, FILE *__restrict);
int fclose(FILE *);

int remove(const char *);
int rename(const char *, const char *);

int feof(FILE *);
int ferror(FILE *);
int fflush(FILE *);
void clearerr(FILE *);

int fseek(FILE *, long, int);
long ftell(FILE *);
void rewind(FILE *);

int fgetpos(FILE *__restrict, fpos_t *__restrict);
int fsetpos(FILE *, const fpos_t *);

size_t fread(void *__restrict, size_t, size_t, FILE *__restrict);
size_t fwrite(const void *__restrict, size_t, size_t, FILE *__restrict);

int fgetc(FILE *);
int getc(FILE *);
int getchar(void);
int ungetc(int, FILE *);

int fputc(int, FILE *);
int putc(int, FILE *);
int putchar(int);

char *fgets(char *__restrict, int, FILE *__restrict);
#if __STDC_VERSION__ < 201112L
char *gets(char *);
#endif

int fputs(const char *__restrict, FILE *__restrict);
int puts(const char *);

int printf(const char *__restrict, ...);
int fprintf(FILE *__restrict, const char *__restrict, ...);
int sprintf(char *__restrict, const char *__restrict, ...);
int snprintf(char *__restrict, size_t, const char *__restrict, ...);

int vprintf(const char *__restrict, __isoc_va_list);
int vfprintf(FILE *__restrict, const char *__restrict, __isoc_va_list);
int vsprintf(char *__restrict, const char *__restrict, __isoc_va_list);
int vsnprintf(char *__restrict, size_t, const char *__restrict, __isoc_va_list);

int scanf(const char *__restrict, ...);
int fscanf(FILE *__restrict, const char *__restrict, ...);
int sscanf(const char *__restrict, const char *__restrict, ...);
int vscanf(const char *__restrict, __isoc_va_list);
int vfscanf(FILE *__restrict, const char *__restrict, __isoc_va_list);
int vsscanf(const char *__restrict, const char *__restrict, __isoc_va_list);

void perror(const char *);

int setvbuf(FILE *__restrict, char *__restrict, int, size_t);
void setbuf(FILE *__restrict, char *__restrict);

char *tmpnam(char *);
FILE *tmpfile(void);

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
FILE *fmemopen(void *__restrict, size_t, const char *__restrict);
FILE *open_memstream(char **, size_t *);
FILE *fdopen(int, const char *);
FILE *popen(const char *, const char *);
int pclose(FILE *);
int fileno(FILE *);
int fseeko(FILE *, off_t, int);
off_t ftello(FILE *);
int dprintf(int, const char *__restrict, ...);
int vdprintf(int, const char *__restrict, __isoc_va_list);
void flockfile(FILE *);
int ftrylockfile(FILE *);
void funlockfile(FILE *);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);
ssize_t getdelim(char **__restrict, size_t *__restrict, int, FILE *__restrict);
ssize_t getline(char **__restrict, size_t *__restrict, FILE *__restrict);
int renameat(int, const char *, int, const char *);
char *ctermid(char *);
#define L_ctermid 20
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
#define P_tmpdir "/tmp"
char *tempnam(const char *, const char *);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define L_cuserid 20
char *cuserid(char *);
void setlinebuf(FILE *);
void setbuffer(FILE *, char *, size_t);
int fgetc_unlocked(FILE *);
int fputc_unlocked(int, FILE *);
int fflush_unlocked(FILE *);
size_t fread_unlocked(void *, size_t, size_t, FILE *);
size_t fwrite_unlocked(const void *, size_t, size_t, FILE *);
void clearerr_unlocked(FILE *);
int feof_unlocked(FILE *);
int ferror_unlocked(FILE *);
int fileno_unlocked(FILE *);
int getw(FILE *);
int putw(int, FILE *);
char *fgetln(FILE *, size_t *);
int asprintf(char **, const char *, ...);
int vasprintf(char **, const char *, __isoc_va_list);
#endif

#ifdef _GNU_SOURCE
char *fgets_unlocked(char *, int, FILE *);
int fputs_unlocked(const char *, FILE *);
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define tmpfile64 tmpfile
#define fopen64 fopen
#define freopen64 freopen
#define fseeko64 fseeko
#define ftello64 ftello
#define fgetpos64 fgetpos
#define fsetpos64 fsetpos
#define fpos64_t fpos_t
#define off64_t off_t
#endif

#endif

/// >>> CONTINUE src/conf/confstr.c
/// >>> START include/errno.h
#ifndef    _ERRNO_H
#define _ERRNO_H

/// >>> START arch/x86_64/bits/errno.h
#define EPERM            1
#define ENOENT           2
#define ESRCH            3
#define EINTR            4
#define EIO              5
#define ENXIO            6
#define E2BIG            7
#define ENOEXEC          8
#define EBADF            9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define ENOTBLK         15
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define ETXTBSY         26
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         35
#define ENAMETOOLONG    36
#define ENOLCK          37
#define ENOSYS          38
#define ENOTEMPTY       39
#define ELOOP           40
#define EWOULDBLOCK     EAGAIN
#define ENOMSG          42
#define EIDRM           43
#define ECHRNG          44
#define EL2NSYNC        45
#define EL3HLT          46
#define EL3RST          47
#define ELNRNG          48
#define EUNATCH         49
#define ENOCSI          50
#define EL2HLT          51
#define EBADE           52
#define EBADR           53
#define EXFULL          54
#define ENOANO          55
#define EBADRQC         56
#define EBADSLT         57
#define EDEADLOCK       EDEADLK
#define EBFONT          59
#define ENOSTR          60
#define ENODATA         61
#define ETIME           62
#define ENOSR           63
#define ENONET          64
#define ENOPKG          65
#define EREMOTE         66
#define ENOLINK         67
#define EADV            68
#define ESRMNT          69
#define ECOMM           70
#define EPROTO          71
#define EMULTIHOP       72
#define EDOTDOT         73
#define EBADMSG         74
#define EOVERFLOW       75
#define ENOTUNIQ        76
#define EBADFD          77
#define EREMCHG         78
#define ELIBACC         79
#define ELIBBAD         80
#define ELIBSCN         81
#define ELIBMAX         82
#define ELIBEXEC        83
#define EILSEQ          84
#define ERESTART        85
#define ESTRPIPE        86
#define EUSERS          87
#define ENOTSOCK        88
#define EDESTADDRREQ    89
#define EMSGSIZE        90
#define EPROTOTYPE      91
#define ENOPROTOOPT     92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP      95
#define ENOTSUP         EOPNOTSUPP
#define EPFNOSUPPORT    96
#define EAFNOSUPPORT    97
#define EADDRINUSE      98
#define EADDRNOTAVAIL   99
#define ENETDOWN        100
#define ENETUNREACH     101
#define ENETRESET       102
#define ECONNABORTED    103
#define ECONNRESET      104
#define ENOBUFS         105
#define EISCONN         106
#define ENOTCONN        107
#define ESHUTDOWN       108
#define ETOOMANYREFS    109
#define ETIMEDOUT       110
#define ECONNREFUSED    111
#define EHOSTDOWN       112
#define EHOSTUNREACH    113
#define EALREADY        114
#define EINPROGRESS     115
#define ESTALE          116
#define EUCLEAN         117
#define ENOTNAM         118
#define ENAVAIL         119
#define EISNAM          120
#define EREMOTEIO       121
#define EDQUOT          122
#define ENOMEDIUM       123
#define EMEDIUMTYPE     124
#define ECANCELED       125
#define ENOKEY          126
#define EKEYEXPIRED     127
#define EKEYREVOKED     128
#define EKEYREJECTED    129
#define EOWNERDEAD      130
#define ENOTRECOVERABLE 131
#define ERFKILL         132
#define EHWPOISON       133

/// >>> CONTINUE include/errno.h

#ifdef __GNUC__
__attribute__((const))
#endif
int *__errno_location(void);
#define errno (*__errno_location())

#ifdef _GNU_SOURCE
extern char *program_invocation_short_name, *program_invocation_name;
#endif

#endif

/// >>> CONTINUE src/conf/confstr.c

/// >>> START src/conf/sysconf.c
/// >>> START src/internal/syscall.h
/// >>> START include/sys/syscall.h
#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

/// >>> START arch/x86_64/bits/syscall.h
#define __NR_read                0
#define __NR_write                1
#define __NR_open                2
#define __NR_close                3
#define __NR_stat                4
#define __NR_fstat                5
#define __NR_lstat                6
#define __NR_poll                7
#define __NR_lseek                8
#define __NR_mmap                9
#define __NR_mprotect            10
#define __NR_munmap                11
#define __NR_brk                12
#define __NR_rt_sigaction        13
#define __NR_rt_sigprocmask        14
#define __NR_rt_sigreturn        15
#define __NR_ioctl                16
#define __NR_pread64            17
#define __NR_pwrite64            18
#define __NR_readv                19
#define __NR_writev                20
#define __NR_access                21
#define __NR_pipe                22
#define __NR_select                23
#define __NR_sched_yield        24
#define __NR_mremap                25
#define __NR_msync                26
#define __NR_mincore            27
#define __NR_madvise            28
#define __NR_shmget                29
#define __NR_shmat                30
#define __NR_shmctl                31
#define __NR_dup                32
#define __NR_dup2                33
#define __NR_pause                34
#define __NR_nanosleep            35
#define __NR_getitimer            36
#define __NR_alarm                37
#define __NR_setitimer            38
#define __NR_getpid                39
#define __NR_sendfile            40
#define __NR_socket                41
#define __NR_connect            42
#define __NR_accept                43
#define __NR_sendto                44
#define __NR_recvfrom            45
#define __NR_sendmsg            46
#define __NR_recvmsg            47
#define __NR_shutdown            48
#define __NR_bind                49
#define __NR_listen                50
#define __NR_getsockname        51
#define __NR_getpeername        52
#define __NR_socketpair            53
#define __NR_setsockopt            54
#define __NR_getsockopt            55
#define __NR_clone                56
#define __NR_fork                57
#define __NR_vfork                58
#define __NR_execve                59
#define __NR_exit                60
#define __NR_wait4                61
#define __NR_kill                62
#define __NR_uname                63
#define __NR_semget                64
#define __NR_semop                65
#define __NR_semctl                66
#define __NR_shmdt                67
#define __NR_msgget                68
#define __NR_msgsnd                69
#define __NR_msgrcv                70
#define __NR_msgctl                71
#define __NR_fcntl                72
#define __NR_flock                73
#define __NR_fsync                74
#define __NR_fdatasync            75
#define __NR_truncate            76
#define __NR_ftruncate            77
#define __NR_getdents            78
#define __NR_getcwd                79
#define __NR_chdir                80
#define __NR_fchdir                81
#define __NR_rename                82
#define __NR_mkdir                83
#define __NR_rmdir                84
#define __NR_creat                85
#define __NR_link                86
#define __NR_unlink                87
#define __NR_symlink            88
#define __NR_readlink            89
#define __NR_chmod                90
#define __NR_fchmod                91
#define __NR_chown                92
#define __NR_fchown                93
#define __NR_lchown                94
#define __NR_umask                95
#define __NR_gettimeofday        96
#define __NR_getrlimit            97
#define __NR_getrusage            98
#define __NR_sysinfo            99
#define __NR_times                100
#define __NR_ptrace                101
#define __NR_getuid                102
#define __NR_syslog                103
#define __NR_getgid                104
#define __NR_setuid                105
#define __NR_setgid                106
#define __NR_geteuid            107
#define __NR_getegid            108
#define __NR_setpgid            109
#define __NR_getppid            110
#define __NR_getpgrp            111
#define __NR_setsid                112
#define __NR_setreuid            113
#define __NR_setregid            114
#define __NR_getgroups            115
#define __NR_setgroups            116
#define __NR_setresuid            117
#define __NR_getresuid            118
#define __NR_setresgid            119
#define __NR_getresgid            120
#define __NR_getpgid            121
#define __NR_setfsuid            122
#define __NR_setfsgid            123
#define __NR_getsid                124
#define __NR_capget                125
#define __NR_capset                126
#define __NR_rt_sigpending        127
#define __NR_rt_sigtimedwait    128
#define __NR_rt_sigqueueinfo    129
#define __NR_rt_sigsuspend        130
#define __NR_sigaltstack        131
#define __NR_utime                132
#define __NR_mknod                133
#define __NR_uselib                134
#define __NR_personality        135
#define __NR_ustat                136
#define __NR_statfs                137
#define __NR_fstatfs            138
#define __NR_sysfs                139
#define __NR_getpriority            140
#define __NR_setpriority            141
#define __NR_sched_setparam            142
#define __NR_sched_getparam            143
#define __NR_sched_setscheduler        144
#define __NR_sched_getscheduler        145
#define __NR_sched_get_priority_max    146
#define __NR_sched_get_priority_min    147
#define __NR_sched_rr_get_interval    148
#define __NR_mlock                    149
#define __NR_munlock                150
#define __NR_mlockall                151
#define __NR_munlockall                152
#define __NR_vhangup                153
#define __NR_modify_ldt                154
#define __NR_pivot_root                155
#define __NR__sysctl                156
#define __NR_prctl                    157
#define __NR_arch_prctl                158
#define __NR_adjtimex                159
#define __NR_setrlimit                160
#define __NR_chroot                    161
#define __NR_sync                    162
#define __NR_acct                    163
#define __NR_settimeofday            164
#define __NR_mount                    165
#define __NR_umount2                166
#define __NR_swapon                    167
#define __NR_swapoff                168
#define __NR_reboot                    169
#define __NR_sethostname            170
#define __NR_setdomainname            171
#define __NR_iopl                    172
#define __NR_ioperm                    173
#define __NR_create_module            174
#define __NR_init_module            175
#define __NR_delete_module            176
#define __NR_get_kernel_syms        177
#define __NR_query_module            178
#define __NR_quotactl                179
#define __NR_nfsservctl                180
#define __NR_getpmsg                181
#define __NR_putpmsg                182
#define __NR_afs_syscall            183
#define __NR_tuxcall                184
#define __NR_security                185
#define __NR_gettid                    186
#define __NR_readahead                187
#define __NR_setxattr                188
#define __NR_lsetxattr                189
#define __NR_fsetxattr                190
#define __NR_getxattr                191
#define __NR_lgetxattr                192
#define __NR_fgetxattr                193
#define __NR_listxattr                194
#define __NR_llistxattr                195
#define __NR_flistxattr                196
#define __NR_removexattr            197
#define __NR_lremovexattr            198
#define __NR_fremovexattr            199
#define __NR_tkill                    200
#define __NR_time                    201
#define __NR_futex                    202
#define __NR_sched_setaffinity        203
#define __NR_sched_getaffinity        204
#define __NR_set_thread_area        205
#define __NR_io_setup                206
#define __NR_io_destroy                207
#define __NR_io_getevents            208
#define __NR_io_submit                209
#define __NR_io_cancel                210
#define __NR_get_thread_area        211
#define __NR_lookup_dcookie            212
#define __NR_epoll_create            213
#define __NR_epoll_ctl_old            214
#define __NR_epoll_wait_old            215
#define __NR_remap_file_pages        216
#define __NR_getdents64                217
#define __NR_set_tid_address        218
#define __NR_restart_syscall        219
#define __NR_semtimedop                220
#define __NR_fadvise64                221
#define __NR_timer_create            222
#define __NR_timer_settime            223
#define __NR_timer_gettime            224
#define __NR_timer_getoverrun        225
#define __NR_timer_delete            226
#define __NR_clock_settime            227
#define __NR_clock_gettime            228
#define __NR_clock_getres            229
#define __NR_clock_nanosleep        230
#define __NR_exit_group                231
#define __NR_epoll_wait                232
#define __NR_epoll_ctl                233
#define __NR_tgkill                    234
#define __NR_utimes                    235
#define __NR_vserver                236
#define __NR_mbind                    237
#define __NR_set_mempolicy            238
#define __NR_get_mempolicy            239
#define __NR_mq_open                240
#define __NR_mq_unlink                241
#define __NR_mq_timedsend            242
#define __NR_mq_timedreceive        243
#define __NR_mq_notify                244
#define __NR_mq_getsetattr            245
#define __NR_kexec_load                246
#define __NR_waitid                    247
#define __NR_add_key                248
#define __NR_request_key            249
#define __NR_keyctl                    250
#define __NR_ioprio_set                251
#define __NR_ioprio_get                252
#define __NR_inotify_init            253
#define __NR_inotify_add_watch        254
#define __NR_inotify_rm_watch        255
#define __NR_migrate_pages            256
#define __NR_openat                    257
#define __NR_mkdirat                258
#define __NR_mknodat                259
#define __NR_fchownat                260
#define __NR_futimesat                261
#define __NR_newfstatat                262
#define __NR_unlinkat                263
#define __NR_renameat                264
#define __NR_linkat                    265
#define __NR_symlinkat                266
#define __NR_readlinkat                267
#define __NR_fchmodat                268
#define __NR_faccessat                269
#define __NR_pselect6                270
#define __NR_ppoll                    271
#define __NR_unshare                272
#define __NR_set_robust_list        273
#define __NR_get_robust_list        274
#define __NR_splice                    275
#define __NR_tee                    276
#define __NR_sync_file_range        277
#define __NR_vmsplice                278
#define __NR_move_pages                279
#define __NR_utimensat                280
#define __NR_epoll_pwait            281
#define __NR_signalfd                282
#define __NR_timerfd_create            283
#define __NR_eventfd                284
#define __NR_fallocate                285
#define __NR_timerfd_settime        286
#define __NR_timerfd_gettime        287
#define __NR_accept4                288
#define __NR_signalfd4                289
#define __NR_eventfd2                290
#define __NR_epoll_create1            291
#define __NR_dup3                    292
#define __NR_pipe2                    293
#define __NR_inotify_init1            294
#define __NR_preadv                    295
#define __NR_pwritev                296
#define __NR_rt_tgsigqueueinfo        297
#define __NR_perf_event_open        298
#define __NR_recvmmsg                299
#define __NR_fanotify_init            300
#define __NR_fanotify_mark            301
#define __NR_prlimit64                302
#define __NR_name_to_handle_at            303
#define __NR_open_by_handle_at            304
#define __NR_clock_adjtime            305
#define __NR_syncfs                306
#define __NR_sendmmsg                307
#define __NR_setns                308
#define __NR_getcpu                309
#define __NR_process_vm_readv            310
#define __NR_process_vm_writev            311
#define __NR_kcmp                312
#define __NR_finit_module            313

#undef __NR_fstatat
#undef __NR_pread
#undef __NR_pwrite
#undef __NR_getdents
#define __NR_fstatat __NR_newfstatat
#define __NR_pread __NR_pread64
#define __NR_pwrite __NR_pwrite64
#define __NR_getdents __NR_getdents64
#define __NR_fadvise __NR_fadvise64

/* Repeat with SYS_ prefix */

#define SYS_read                0
#define SYS_write                1
#define SYS_open                2
#define SYS_close                3
#define SYS_stat                4
#define SYS_fstat                5
#define SYS_lstat                6
#define SYS_poll                7
#define SYS_lseek                8
#define SYS_mmap                9
#define SYS_mprotect            10
#define SYS_munmap                11
#define SYS_brk                12
#define SYS_rt_sigaction        13
#define SYS_rt_sigprocmask        14
#define SYS_rt_sigreturn        15
#define SYS_ioctl                16
#define SYS_pread64            17
#define SYS_pwrite64            18
#define SYS_readv                19
#define SYS_writev                20
#define SYS_access                21
#define SYS_pipe                22
#define SYS_select                23
#define SYS_sched_yield        24
#define SYS_mremap                25
#define SYS_msync                26
#define SYS_mincore            27
#define SYS_madvise            28
#define SYS_shmget                29
#define SYS_shmat                30
#define SYS_shmctl                31
#define SYS_dup                32
#define SYS_dup2                33
#define SYS_pause                34
#define SYS_nanosleep            35
#define SYS_getitimer            36
#define SYS_alarm                37
#define SYS_setitimer            38
#define SYS_getpid                39
#define SYS_sendfile            40
#define SYS_socket                41
#define SYS_connect            42
#define SYS_accept                43
#define SYS_sendto                44
#define SYS_recvfrom            45
#define SYS_sendmsg            46
#define SYS_recvmsg            47
#define SYS_shutdown            48
#define SYS_bind                49
#define SYS_listen                50
#define SYS_getsockname        51
#define SYS_getpeername        52
#define SYS_socketpair            53
#define SYS_setsockopt            54
#define SYS_getsockopt            55
#define SYS_clone                56
#define SYS_fork                57
#define SYS_vfork                58
#define SYS_execve                59
#define SYS_exit                60
#define SYS_wait4                61
#define SYS_kill                62
#define SYS_uname                63
#define SYS_semget                64
#define SYS_semop                65
#define SYS_semctl                66
#define SYS_shmdt                67
#define SYS_msgget                68
#define SYS_msgsnd                69
#define SYS_msgrcv                70
#define SYS_msgctl                71
#define SYS_fcntl                72
#define SYS_flock                73
#define SYS_fsync                74
#define SYS_fdatasync            75
#define SYS_truncate            76
#define SYS_ftruncate            77
#define SYS_getdents            78
#define SYS_getcwd                79
#define SYS_chdir                80
#define SYS_fchdir                81
#define SYS_rename                82
#define SYS_mkdir                83
#define SYS_rmdir                84
#define SYS_creat                85
#define SYS_link                86
#define SYS_unlink                87
#define SYS_symlink            88
#define SYS_readlink            89
#define SYS_chmod                90
#define SYS_fchmod                91
#define SYS_chown                92
#define SYS_fchown                93
#define SYS_lchown                94
#define SYS_umask                95
#define SYS_gettimeofday        96
#define SYS_getrlimit            97
#define SYS_getrusage            98
#define SYS_sysinfo            99
#define SYS_times                100
#define SYS_ptrace                101
#define SYS_getuid                102
#define SYS_syslog                103
#define SYS_getgid                104
#define SYS_setuid                105
#define SYS_setgid                106
#define SYS_geteuid            107
#define SYS_getegid            108
#define SYS_setpgid            109
#define SYS_getppid            110
#define SYS_getpgrp            111
#define SYS_setsid                112
#define SYS_setreuid            113
#define SYS_setregid            114
#define SYS_getgroups            115
#define SYS_setgroups            116
#define SYS_setresuid            117
#define SYS_getresuid            118
#define SYS_setresgid            119
#define SYS_getresgid            120
#define SYS_getpgid            121
#define SYS_setfsuid            122
#define SYS_setfsgid            123
#define SYS_getsid                124
#define SYS_capget                125
#define SYS_capset                126
#define SYS_rt_sigpending        127
#define SYS_rt_sigtimedwait    128
#define SYS_rt_sigqueueinfo    129
#define SYS_rt_sigsuspend        130
#define SYS_sigaltstack        131
#define SYS_utime                132
#define SYS_mknod                133
#define SYS_uselib                134
#define SYS_personality        135
#define SYS_ustat                136
#define SYS_statfs                137
#define SYS_fstatfs            138
#define SYS_sysfs                139
#define SYS_getpriority            140
#define SYS_setpriority            141
#define SYS_sched_setparam            142
#define SYS_sched_getparam            143
#define SYS_sched_setscheduler        144
#define SYS_sched_getscheduler        145
#define SYS_sched_get_priority_max    146
#define SYS_sched_get_priority_min    147
#define SYS_sched_rr_get_interval    148
#define SYS_mlock                    149
#define SYS_munlock                150
#define SYS_mlockall                151
#define SYS_munlockall                152
#define SYS_vhangup                153
#define SYS_modify_ldt                154
#define SYS_pivot_root                155
#define SYS__sysctl                156
#define SYS_prctl                    157
#define SYS_arch_prctl                158
#define SYS_adjtimex                159
#define SYS_setrlimit                160
#define SYS_chroot                    161
#define SYS_sync                    162
#define SYS_acct                    163
#define SYS_settimeofday            164
#define SYS_mount                    165
#define SYS_umount2                166
#define SYS_swapon                    167
#define SYS_swapoff                168
#define SYS_reboot                    169
#define SYS_sethostname            170
#define SYS_setdomainname            171
#define SYS_iopl                    172
#define SYS_ioperm                    173
#define SYS_create_module            174
#define SYS_init_module            175
#define SYS_delete_module            176
#define SYS_get_kernel_syms        177
#define SYS_query_module            178
#define SYS_quotactl                179
#define SYS_nfsservctl                180
#define SYS_getpmsg                181
#define SYS_putpmsg                182
#define SYS_afs_syscall            183
#define SYS_tuxcall                184
#define SYS_security                185
#define SYS_gettid                    186
#define SYS_readahead                187
#define SYS_setxattr                188
#define SYS_lsetxattr                189
#define SYS_fsetxattr                190
#define SYS_getxattr                191
#define SYS_lgetxattr                192
#define SYS_fgetxattr                193
#define SYS_listxattr                194
#define SYS_llistxattr                195
#define SYS_flistxattr                196
#define SYS_removexattr            197
#define SYS_lremovexattr            198
#define SYS_fremovexattr            199
#define SYS_tkill                    200
#define SYS_time                    201
#define SYS_futex                    202
#define SYS_sched_setaffinity        203
#define SYS_sched_getaffinity        204
#define SYS_set_thread_area        205
#define SYS_io_setup                206
#define SYS_io_destroy                207
#define SYS_io_getevents            208
#define SYS_io_submit                209
#define SYS_io_cancel                210
#define SYS_get_thread_area        211
#define SYS_lookup_dcookie            212
#define SYS_epoll_create            213
#define SYS_epoll_ctl_old            214
#define SYS_epoll_wait_old            215
#define SYS_remap_file_pages        216
#define SYS_getdents64                217
#define SYS_set_tid_address        218
#define SYS_restart_syscall        219
#define SYS_semtimedop                220
#define SYS_fadvise64                221
#define SYS_timer_create            222
#define SYS_timer_settime            223
#define SYS_timer_gettime            224
#define SYS_timer_getoverrun        225
#define SYS_timer_delete            226
#define SYS_clock_settime            227
#define SYS_clock_gettime            228
#define SYS_clock_getres            229
#define SYS_clock_nanosleep        230
#define SYS_exit_group                231
#define SYS_epoll_wait                232
#define SYS_epoll_ctl                233
#define SYS_tgkill                    234
#define SYS_utimes                    235
#define SYS_vserver                236
#define SYS_mbind                    237
#define SYS_set_mempolicy            238
#define SYS_get_mempolicy            239
#define SYS_mq_open                240
#define SYS_mq_unlink                241
#define SYS_mq_timedsend            242
#define SYS_mq_timedreceive        243
#define SYS_mq_notify                244
#define SYS_mq_getsetattr            245
#define SYS_kexec_load                246
#define SYS_waitid                    247
#define SYS_add_key                248
#define SYS_request_key            249
#define SYS_keyctl                    250
#define SYS_ioprio_set                251
#define SYS_ioprio_get                252
#define SYS_inotify_init            253
#define SYS_inotify_add_watch        254
#define SYS_inotify_rm_watch        255
#define SYS_migrate_pages            256
#define SYS_openat                    257
#define SYS_mkdirat                258
#define SYS_mknodat                259
#define SYS_fchownat                260
#define SYS_futimesat                261
#define SYS_newfstatat                262
#define SYS_unlinkat                263
#define SYS_renameat                264
#define SYS_linkat                    265
#define SYS_symlinkat                266
#define SYS_readlinkat                267
#define SYS_fchmodat                268
#define SYS_faccessat                269
#define SYS_pselect6                270
#define SYS_ppoll                    271
#define SYS_unshare                272
#define SYS_set_robust_list        273
#define SYS_get_robust_list        274
#define SYS_splice                    275
#define SYS_tee                    276
#define SYS_sync_file_range        277
#define SYS_vmsplice                278
#define SYS_move_pages                279
#define SYS_utimensat                280
#define SYS_epoll_pwait            281
#define SYS_signalfd                282
#define SYS_timerfd_create            283
#define SYS_eventfd                284
#define SYS_fallocate                285
#define SYS_timerfd_settime        286
#define SYS_timerfd_gettime        287
#define SYS_accept4                288
#define SYS_signalfd4                289
#define SYS_eventfd2                290
#define SYS_epoll_create1            291
#define SYS_dup3                    292
#define SYS_pipe2                    293
#define SYS_inotify_init1            294
#define SYS_preadv                    295
#define SYS_pwritev                296
#define SYS_rt_tgsigqueueinfo        297
#define SYS_perf_event_open        298
#define SYS_recvmmsg                299
#define SYS_fanotify_init            300
#define SYS_fanotify_mark            301
#define SYS_prlimit64                302
#define SYS_name_to_handle_at            303
#define SYS_open_by_handle_at            304
#define SYS_clock_adjtime            305
#define SYS_syncfs                306
#define SYS_sendmmsg                307
#define SYS_setns                308
#define SYS_getcpu                309
#define SYS_process_vm_readv            310
#define SYS_process_vm_writev            311
#define SYS_kcmp                312
#define SYS_finit_module            313

#undef SYS_fstatat
#undef SYS_pread
#undef SYS_pwrite
#undef SYS_getdents
#define SYS_fstatat SYS_newfstatat
#define SYS_pread SYS_pread64
#define SYS_pwrite SYS_pwrite64
#define SYS_getdents SYS_getdents64
#define SYS_fadvise SYS_fadvise64

/// >>> CONTINUE include/sys/syscall.h

#endif

/// >>> CONTINUE src/internal/syscall.h
/// >>> START arch/x86_64/syscall_arch.h
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

static __inline long __syscall0(long n) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall1(long n, long a1) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall2(long n, long a1, long a2) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
                          : "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall3(long n, long a1, long a2, long a3) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
                          "d"(a3) : "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4) {
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
                          "d"(a3), "r"(r10): "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5) {
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
                          "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
                          "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

/// >>> CONTINUE src/internal/syscall.h

/// >>> CONTINUE src/conf/sysconf.c
/// >>> START src/internal/libc.h
/// >>> START include/stdlib.h
#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#define __NEED_size_t
#define __NEED_wchar_t

int atoi (const char *);
long atol (const char *);
long long atoll (const char *);
double atof (const char *);

float strtof (const char *__restrict, char **__restrict);
double strtod (const char *__restrict, char **__restrict);
long double strtold (const char *__restrict, char **__restrict);

long strtol (const char *__restrict, char **__restrict, int);
unsigned long strtoul (const char *__restrict, char **__restrict, int);
long long strtoll (const char *__restrict, char **__restrict, int);
unsigned long long strtoull (const char *__restrict, char **__restrict, int);

int rand (void);
void srand (unsigned);

void *malloc (size_t);
void *calloc (size_t, size_t);
void *realloc (void *, size_t);
void free (void *);
void *aligned_alloc(size_t alignment, size_t size);

_Noreturn void abort (void);
int atexit (void (*) (void));
_Noreturn void exit (int);
_Noreturn void _Exit (int);
int at_quick_exit (void (*) (void));
_Noreturn void quick_exit (int);

char *getenv (const char *);

int system (const char *);

void *bsearch (const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
void qsort (void *, size_t, size_t, int (*)(const void *, const void *));

int abs (int);
long labs (long);
long long llabs (long long);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div (int, int);
ldiv_t ldiv (long, long);
lldiv_t lldiv (long long, long long);

int mblen (const char *, size_t);
int mbtowc (wchar_t *__restrict, const char *__restrict, size_t);
int wctomb (char *, wchar_t);
size_t mbstowcs (wchar_t *__restrict, const char *__restrict, size_t);
size_t wcstombs (char *__restrict, const wchar_t *__restrict, size_t);

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define MB_CUR_MAX ((size_t)+4)

#define RAND_MAX (0x7fffffff)

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

#define WNOHANG    1
#define WUNTRACED  2

#define WEXITSTATUS(s) (((s) & 0xff00) >> 8)
#define WTERMSIG(s) ((s) & 0x7f)
#define WSTOPSIG(s) WEXITSTATUS(s)
#define WIFEXITED(s) (!WTERMSIG(s))
#define WIFSTOPPED(s) ((short)((((s)&0xffff)*0x10001)>>8) > 0x7f00)
#define WIFSIGNALED(s) (((s)&0xffff)-1 < 0xffu)

int posix_memalign (void **, size_t, size_t);
int setenv (const char *, const char *, int);
int unsetenv (const char *);
int mkstemp (char *);
int mkostemp (char *, int);
char *mkdtemp (char *);
int getsubopt (char **, char *const *, char **);
int rand_r (unsigned *);

#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)
char *realpath (const char *__restrict, char *__restrict);
long int random (void);
void srandom (unsigned int);
char *initstate (unsigned int, char *, size_t);
char *setstate (char *);
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE)
int putenv (char *);
int posix_openpt (int);
int grantpt (int);
int unlockpt (int);
char *ptsname (int);
char *l64a (long);
long a64l (const char *);
void setkey (const char *);
double drand48 (void);
double erand48 (unsigned short [3]);
long int lrand48 (void);
long int nrand48 (unsigned short [3]);
long mrand48 (void);
long jrand48 (unsigned short [3]);
void srand48 (long);
unsigned short *seed48 (unsigned short [3]);
void lcong48 (unsigned short [7]);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
/// >>> START include/alloca.h
#ifndef    _ALLOCA_H
#define    _ALLOCA_H

#define    __NEED_size_t

void *alloca(size_t);

#ifdef __GNUC__
#define alloca __builtin_alloca
#endif

#endif

/// >>> CONTINUE include/stdlib.h
char *mktemp (char *);
int mkstemps (char *, int);
int mkostemps (char *, int, int);
void *valloc (size_t);
void *memalign(size_t, size_t);
int getloadavg(double *, int);
#define WCOREDUMP(s) ((s) & 0x80)
#define WIFCONTINUED(s) ((s) == 0xffff)
#endif

#ifdef _GNU_SOURCE
int clearenv(void);
int ptsname_r(int, char *, size_t);
char *ecvt(double, int, int *, int *);
char *fcvt(double, int, int *, int *);
char *gcvt(double, int, char *);
struct __locale_struct;
float strtof_l(const char *__restrict, char **__restrict, struct __locale_struct *);
double strtod_l(const char *__restrict, char **__restrict, struct __locale_struct *);
long double strtold_l(const char *__restrict, char **__restrict, struct __locale_struct *);
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define mkstemp64 mkstemp
#define mkostemp64 mkostemp
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define mkstemps64 mkstemps
#define mkostemps64 mkostemps
#endif
#endif

#endif

/// >>> CONTINUE src/internal/libc.h

/// >>> CONTINUE src/conf/sysconf.c

/// >>> START src/ctype/isalnum.c
/// >>> START include/ctype.h
#ifndef    _CTYPE_H
#define    _CTYPE_H

int   isalnum(int);
int   isalpha(int);
int   isblank(int);
int   iscntrl(int);
int   isdigit(int);
int   isgraph(int);
int   islower(int);
int   isprint(int);
int   ispunct(int);
int   isspace(int);
int   isupper(int);
int   isxdigit(int);
int   tolower(int);
int   toupper(int);

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) \
 || defined(_BSD_SOURCE)

#define __NEED_locale_t

int   isalnum_l(int, locale_t);
int   isalpha_l(int, locale_t);
int   isblank_l(int, locale_t);
int   iscntrl_l(int, locale_t);
int   isdigit_l(int, locale_t);
int   isgraph_l(int, locale_t);
int   islower_l(int, locale_t);
int   isprint_l(int, locale_t);
int   ispunct_l(int, locale_t);
int   isspace_l(int, locale_t);
int   isupper_l(int, locale_t);
int   isxdigit_l(int, locale_t);
int   tolower_l(int, locale_t);
int   toupper_l(int, locale_t);

int   isascii(int);
int   toascii(int);
#define _tolower(a) ((a)|0x20)
#define _toupper(a) ((a)&0x5f)

#endif

#endif

/// >>> CONTINUE src/ctype/isalnum.c

/// >>> START src/ctype/iswalnum.c
/// >>> START include/wchar.h
#ifndef _WCHAR_H
#define _WCHAR_H

#define __NEED_FILE
#define __NEED___isoc_va_list
#define __NEED_size_t
#define __NEED_wchar_t
#define __NEED_wint_t

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_locale_t
#define __NEED_va_list
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_wctype_t
#endif

#if L'\0'-1 > 0
#define WCHAR_MAX (0xffffffffu+L'\0')
#define WCHAR_MIN (0+L'\0')
#else
#define WCHAR_MAX (0x7fffffff+L'\0')
#define WCHAR_MIN (-1-0x7fffffff+L'\0')
#endif

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#undef WEOF
#define WEOF 0xffffffffU

typedef struct __mbstate_t {
    unsigned __opaque1, __opaque2;
} mbstate_t;

wchar_t *wcscpy (wchar_t *__restrict, const wchar_t *__restrict);
wchar_t *wcsncpy (wchar_t *__restrict, const wchar_t *__restrict, size_t);

wchar_t *wcscat (wchar_t *__restrict, const wchar_t *__restrict);
wchar_t *wcsncat (wchar_t *__restrict, const wchar_t *__restrict, size_t);

int wcscmp (const wchar_t *, const wchar_t *);
int wcsncmp (const wchar_t *, const wchar_t *, size_t);

int wcscoll(const wchar_t *, const wchar_t *);
size_t wcsxfrm (wchar_t *__restrict, const wchar_t *__restrict, size_t n);

wchar_t *wcschr (const wchar_t *, wchar_t);
wchar_t *wcsrchr (const wchar_t *, wchar_t);

size_t wcscspn (const wchar_t *, const wchar_t *);
size_t wcsspn (const wchar_t *, const wchar_t *);
wchar_t *wcspbrk (const wchar_t *, const wchar_t *);

wchar_t *wcstok (wchar_t *__restrict, const wchar_t *__restrict, wchar_t **__restrict);

size_t wcslen (const wchar_t *);

wchar_t *wcsstr (const wchar_t *__restrict, const wchar_t *__restrict);
wchar_t *wcswcs (const wchar_t *, const wchar_t *);

wchar_t *wmemchr (const wchar_t *, wchar_t, size_t);
int wmemcmp (const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemcpy (wchar_t *__restrict, const wchar_t *__restrict, size_t);
wchar_t *wmemmove (wchar_t *, const wchar_t *, size_t);
wchar_t *wmemset (wchar_t *, wchar_t, size_t);

wint_t btowc (int);
int wctob (wint_t);

int mbsinit (const mbstate_t *);
size_t mbrtowc (wchar_t *__restrict, const char *__restrict, size_t, mbstate_t *__restrict);
size_t wcrtomb (char *__restrict, wchar_t, mbstate_t *__restrict);

size_t mbrlen (const char *__restrict, size_t, mbstate_t *__restrict);

size_t mbsrtowcs (wchar_t *__restrict, const char **__restrict, size_t, mbstate_t *__restrict);
size_t wcsrtombs (char *__restrict, const wchar_t **__restrict, size_t, mbstate_t *__restrict);

float wcstof (const wchar_t *__restrict, wchar_t **__restrict);
double wcstod (const wchar_t *__restrict, wchar_t **__restrict);
long double wcstold (const wchar_t *__restrict, wchar_t **__restrict);

long wcstol (const wchar_t *__restrict, wchar_t **__restrict, int);
unsigned long wcstoul (const wchar_t *__restrict, wchar_t **__restrict, int);

long long wcstoll (const wchar_t *__restrict, wchar_t **__restrict, int);
unsigned long long wcstoull (const wchar_t *__restrict, wchar_t **__restrict, int);

int fwide (FILE *, int);

int wprintf (const wchar_t *__restrict, ...);
int fwprintf (FILE *__restrict, const wchar_t *__restrict, ...);
int swprintf (wchar_t *__restrict, size_t, const wchar_t *__restrict, ...);

int vwprintf (const wchar_t *__restrict, __isoc_va_list);
int vfwprintf (FILE *__restrict, const wchar_t *__restrict, __isoc_va_list);
int vswprintf (wchar_t *__restrict, size_t, const wchar_t *__restrict, __isoc_va_list);

int wscanf (const wchar_t *__restrict, ...);
int fwscanf (FILE *__restrict, const wchar_t *__restrict, ...);
int swscanf (const wchar_t *__restrict, const wchar_t *__restrict, ...);

int vwscanf (const wchar_t *__restrict, __isoc_va_list);
int vfwscanf (FILE *__restrict, const wchar_t *__restrict, __isoc_va_list);
int vswscanf (const wchar_t *__restrict, const wchar_t *__restrict, __isoc_va_list);

wint_t fgetwc (FILE *);
wint_t getwc (FILE *);
wint_t getwchar (void);

wint_t fputwc (wchar_t, FILE *);
wint_t putwc (wchar_t, FILE *);
wint_t putwchar (wchar_t);

wchar_t *fgetws (wchar_t *__restrict, int, FILE *__restrict);
int fputws (const wchar_t *__restrict, FILE *__restrict);

wint_t ungetwc (wint_t, FILE *);

struct tm;
size_t wcsftime (wchar_t *__restrict, size_t, const wchar_t *__restrict, const struct tm *__restrict);

#undef iswdigit

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE)  || defined(_BSD_SOURCE)
FILE *open_wmemstream(wchar_t **, size_t *);
size_t mbsnrtowcs(wchar_t *__restrict, const char **__restrict, size_t, size_t, mbstate_t *__restrict);
size_t wcsnrtombs(char *__restrict, const wchar_t **__restrict, size_t, size_t, mbstate_t *__restrict);
wchar_t *wcsdup(const wchar_t *);
size_t wcsnlen (const wchar_t *, size_t);
wchar_t *wcpcpy (wchar_t *__restrict, const wchar_t *__restrict);
wchar_t *wcpncpy (wchar_t *__restrict, const wchar_t *__restrict, size_t);
int wcscasecmp(const wchar_t *, const wchar_t *);
int wcscasecmp_l(const wchar_t *, const wchar_t *, locale_t);
int wcsncasecmp(const wchar_t *, const wchar_t *, size_t);
int wcsncasecmp_l(const wchar_t *, const wchar_t *, size_t, locale_t);
int wcscoll_l(const wchar_t *, const wchar_t *, locale_t);
size_t wcsxfrm_l(wchar_t *__restrict, const wchar_t *__restrict, size_t n, locale_t);
#endif

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int wcwidth (wchar_t);
int wcswidth (const wchar_t *, size_t);
int       iswalnum(wint_t);
int       iswalpha(wint_t);
int       iswblank(wint_t);
int       iswcntrl(wint_t);
int       iswdigit(wint_t);
int       iswgraph(wint_t);
int       iswlower(wint_t);
int       iswprint(wint_t);
int       iswpunct(wint_t);
int       iswspace(wint_t);
int       iswupper(wint_t);
int       iswxdigit(wint_t);
int       iswctype(wint_t, wctype_t);
wint_t    towlower(wint_t);
wint_t    towupper(wint_t);
wctype_t  wctype(const char *);
#undef iswdigit
#define iswdigit(a) ((unsigned)(a)-'0' < 10)
#endif

#endif

/// >>> CONTINUE src/ctype/iswalnum.c
/// >>> START include/wctype.h
#ifndef _WCTYPE_H
#define _WCTYPE_H

#define __NEED_wint_t
#define __NEED_wctype_t

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_locale_t
#endif

typedef const int * wctrans_t;

#undef WEOF
#define WEOF 0xffffffffU

#undef iswdigit

int       iswalnum(wint_t);
int       iswalpha(wint_t);
int       iswblank(wint_t);
int       iswcntrl(wint_t);
int       iswdigit(wint_t);
int       iswgraph(wint_t);
int       iswlower(wint_t);
int       iswprint(wint_t);
int       iswpunct(wint_t);
int       iswspace(wint_t);
int       iswupper(wint_t);
int       iswxdigit(wint_t);
int       iswctype(wint_t, wctype_t);
wint_t    towctrans(wint_t, wctrans_t);
wint_t    towlower(wint_t);
wint_t    towupper(wint_t);
wctrans_t wctrans(const char *);
wctype_t  wctype(const char *);

#undef iswdigit
#define iswdigit(a) (((unsigned)(a)-L'0') < 10)

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

int iswalnum_l(wint_t, locale_t);
int iswalpha_l(wint_t, locale_t);
int iswblank_l(wint_t, locale_t);
int iswcntrl_l(wint_t, locale_t);
int iswdigit_l(wint_t, locale_t);
int iswgraph_l(wint_t, locale_t);
int iswlower_l(wint_t, locale_t);
int iswprint_l(wint_t, locale_t);
int iswpunct_l(wint_t, locale_t);
int iswspace_l(wint_t, locale_t);
int iswupper_l(wint_t, locale_t);
int iswxdigit_l(wint_t, locale_t);
int iswctype_l(wint_t, wctype_t, locale_t);
wint_t towlower_l(wint_t, locale_t);
wint_t towupper_l(wint_t, locale_t);
wint_t towctrans_l(wint_t, wctrans_t, locale_t);
wctrans_t wctrans_l(const char *, locale_t);
wctype_t  wctype_l(const char *, locale_t);

#endif

#endif

/// >>> CONTINUE src/ctype/iswalnum.c

/// >>> START src/dirent/fdopendir.c
/// >>> START include/sys/stat.h
#ifndef    _SYS_STAT_H
#define    _SYS_STAT_H

#define __NEED_dev_t
#define __NEED_ino_t
#define __NEED_mode_t
#define __NEED_nlink_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_time_t
#define __NEED_blksize_t
#define __NEED_blkcnt_t
#define __NEED_struct_timespec

/// >>> START arch/x86_64/bits/stat.h
/* copied from kernel definition, but with padding replaced
 * by the corresponding correctly-sized userspace types. */

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;

    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    unsigned int    __pad0;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;

    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    long __unused[3];
};

/// >>> CONTINUE include/sys/stat.h

#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec

#define S_IFMT  0170000

#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_TYPEISMQ(buf)  0
#define S_TYPEISSEM(buf) 0
#define S_TYPEISSHM(buf) 0
#define S_TYPEISTMO(buf) 0

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#ifndef S_IRUSR
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXU 0700
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IRWXG 0070
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_IRWXO 0007
#endif

#define UTIME_NOW  0x3fffffff
#define UTIME_OMIT 0x3ffffffe

int stat(const char *__restrict, struct stat *__restrict);
int fstat(int, struct stat *);
int lstat(const char *__restrict, struct stat *__restrict);
int fstatat(int, const char *__restrict, struct stat *__restrict, int);
int chmod(const char *, mode_t);
int fchmod(int, mode_t);
int fchmodat(int, const char *, mode_t, int);
mode_t umask(mode_t);
int mkdir(const char *, mode_t);
int mknod(const char *, mode_t, dev_t);
int mkfifo(const char *, mode_t);
int mkdirat(int, const char *, mode_t);
int mknodat(int, const char *, mode_t, dev_t);
int mkfifoat(int, const char *, mode_t);

int futimens(int, const struct timespec [2]);
int utimensat(int, const char *, const struct timespec [2], int);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int lchmod(const char *, mode_t);
#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR
#endif

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define stat64 stat
#define fstat64 fstat
#define lstat64 lstat
#define fstatat64 fstatat
#define blksize64_t blksize_t
#define blkcnt64_t blkcnt_t
#define ino64_t ino_t
#define off64_t off_t
#endif

#endif

/// >>> CONTINUE src/dirent/fdopendir.c

/// >>> START src/fenv/fegetexceptflag.c
/// >>> START include/fenv.h
#ifndef _FENV_H
#define _FENV_H

/// >>> START arch/x86_64/bits/fenv.h
#define FE_INVALID    1
#define __FE_DENORM   2
#define FE_DIVBYZERO  4
#define FE_OVERFLOW   8
#define FE_UNDERFLOW  16
#define FE_INEXACT    32

#define FE_ALL_EXCEPT 63

#define FE_TONEAREST  0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

typedef unsigned short fexcept_t;

typedef struct {
    unsigned short __control_word;
    unsigned short __unused1;
    unsigned short __status_word;
    unsigned short __unused2;
    unsigned short __tags;
    unsigned short __unused3;
    unsigned int __eip;
    unsigned short __cs_selector;
    unsigned int __opcode:11;
    unsigned int __unused4:5;
    unsigned int __data_offset;
    unsigned short __data_selector;
    unsigned short __unused5;
    unsigned int __mxcsr;
} fenv_t;

#define FE_DFL_ENV      ((const fenv_t *) -1)

/// >>> CONTINUE include/fenv.h

int feclearexcept(int);
int fegetexceptflag(fexcept_t *, int);
int feraiseexcept(int);
int fesetexceptflag(const fexcept_t *, int);
int fetestexcept(int);

int fegetround(void);
int fesetround(int);

int fegetenv(fenv_t *);
int feholdexcept(fenv_t *);
int fesetenv(const fenv_t *);
int feupdateenv(const fenv_t *);

#endif

/// >>> CONTINUE src/fenv/fegetexceptflag.c

/// >>> START src/locale/catclose.c
/// >>> START include/nl_types.h
#ifndef _NL_TYPES_H
#define _NL_TYPES_H

#define NL_SETD 1
#define NL_CAT_LOCALE 1

typedef int nl_item;
typedef void *nl_catd;

nl_catd catopen (const char *, int);
char *catgets (nl_catd, int, int, const char *);
int catclose (nl_catd);

#endif

/// >>> CONTINUE src/locale/catclose.c

/// >>> START src/locale/intl.c
/// >>> START include/libintl.h
#ifndef _LIBINTL_H
#define _LIBINTL_H

#define __USE_GNU_GETTEXT 1
#define __GNU_GETTEXT_SUPPORTED_REVISION(major) ((major) == 0 ? 1 : -1)

char *gettext(const char *);
char *dgettext(const char *, const char *);
char *dcgettext(const char *, const char *, int);
char *ngettext(const char *, const char *, unsigned long);
char *dngettext(const char *, const char *, const char *, unsigned long);
char *dcngettext(const char *, const char *, const char *, unsigned long, int);
char *textdomain(const char *);
char *bindtextdomain (const char *, const char *);
char *bind_textdomain_codeset(const char *, const char *);

#endif

/// >>> CONTINUE src/locale/intl.c

/// >>> START src/locale/langinfo.c
/// >>> START include/locale.h
#ifndef    _LOCALE_H
#define    _LOCALE_H

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif

#define LC_CTYPE    0
#define LC_NUMERIC  1
#define LC_TIME     2
#define LC_COLLATE  3
#define LC_MONETARY 4
#define LC_MESSAGES 5
#define LC_ALL      6

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;

    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    char int_p_cs_precedes;
    char int_p_sep_by_space;
    char int_n_cs_precedes;
    char int_n_sep_by_space;
    char int_p_sign_posn;
    char int_n_sign_posn;
};

char *setlocale (int, const char *);
struct lconv *localeconv(void);

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#define __NEED_locale_t

#define LC_GLOBAL_LOCALE ((locale_t)-1)

#define LC_CTYPE_MASK    (1<<LC_CTYPE)
#define LC_NUMERIC_MASK  (1<<LC_NUMERIC)
#define LC_TIME_MASK     (1<<LC_TIME)
#define LC_COLLATE_MASK  (1<<LC_COLLATE)
#define LC_MONETARY_MASK (1<<LC_MONETARY)
#define LC_MESSAGES_MASK (1<<LC_MESSAGES)
#define LC_ALL_MASK      0x7fffffff

locale_t duplocale(locale_t);
void freelocale(locale_t);
locale_t newlocale(int, const char *, locale_t);
locale_t uselocale(locale_t);

#endif

#endif

/// >>> CONTINUE src/locale/langinfo.c
/// >>> START include/langinfo.h
#ifndef _LANGINFO_H
#define _LANGINFO_H

#define __NEED_locale_t

#define ABDAY_1 0x20000
#define ABDAY_2 0x20001
#define ABDAY_3 0x20002
#define ABDAY_4 0x20003
#define ABDAY_5 0x20004
#define ABDAY_6 0x20005
#define ABDAY_7 0x20006

#define DAY_1 0x20007
#define DAY_2 0x20008
#define DAY_3 0x20009
#define DAY_4 0x2000A
#define DAY_5 0x2000B
#define DAY_6 0x2000C
#define DAY_7 0x2000D

#define ABMON_1 0x2000E
#define ABMON_2 0x2000F
#define ABMON_3 0x20010
#define ABMON_4 0x20011
#define ABMON_5 0x20012
#define ABMON_6 0x20013
#define ABMON_7 0x20014
#define ABMON_8 0x20015
#define ABMON_9 0x20016
#define ABMON_10 0x20017
#define ABMON_11 0x20018
#define ABMON_12 0x20019

#define MON_1 0x2001A
#define MON_2 0x2001B
#define MON_3 0x2001C
#define MON_4 0x2001D
#define MON_5 0x2001E
#define MON_6 0x2001F
#define MON_7 0x20020
#define MON_8 0x20021
#define MON_9 0x20022
#define MON_10 0x20023
#define MON_11 0x20024
#define MON_12 0x20025

#define AM_STR 0x20026
#define PM_STR 0x20027

#define D_T_FMT 0x20028
#define D_FMT 0x20029
#define T_FMT 0x2002A
#define T_FMT_AMPM 0x2002B

#define ERA 0x2002C
#define ERA_D_FMT 0x2002E
#define ALT_DIGITS 0x2002F
#define ERA_D_T_FMT 0x20030
#define ERA_T_FMT 0x20031

#define CODESET 14

#define CRNCYSTR 0x4000F

#define RADIXCHAR 0x10000
#define THOUSEP 0x10001
#define YESEXPR 0x50000
#define NOEXPR 0x50001

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define YESSTR 0x50002
#define NOSTR 0x50003
#endif

char *nl_langinfo(nl_item);
char *nl_langinfo_l(nl_item, locale_t);

#endif

/// >>> CONTINUE src/locale/langinfo.c

/// >>> START src/locale/strfmon.c
/// >>> START include/monetary.h
#ifndef _MONETARY_H
#define _MONETARY_H

#define __NEED_ssize_t
#define __NEED_size_t
#define __NEED_locale_t

ssize_t strfmon(char *__restrict, size_t, const char *__restrict, ...);
ssize_t strfmon_l(char *__restrict, size_t, locale_t, const char *__restrict, ...);

#endif

/// >>> CONTINUE src/locale/strfmon.c

/// >>> START src/misc/basename.c
/// >>> START include/libgen.h
#ifndef _LIBGEN_H
#define _LIBGEN_H

char *dirname(char *);
char *basename(char *);

#endif

/// >>> CONTINUE src/misc/basename.c

/// >>> START src/misc/ffs.c
/// >>> START arch/x86_64/atomic.h
#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

static inline int a_ctz_64(uint64_t x) {
    __asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
    return x;
}

static inline int a_ctz_l(unsigned long x) {
    __asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
    return x;
}

static inline void a_and_64(volatile uint64_t *p, uint64_t v) {
    __asm__( "lock ; and %1, %0"
             : "=m"(*p) : "r"(v) : "memory" );
}

static inline void a_or_64(volatile uint64_t *p, uint64_t v) {
    __asm__( "lock ; or %1, %0"
             : "=m"(*p) : "r"(v) : "memory" );
}

static inline void a_store_l(volatile void *p, long x) {
    __asm__( "mov %1, %0" : "=m"(*(long *)p) : "r"(x) : "memory" );
}

static inline void a_or_l(volatile void *p, long v) {
    __asm__( "lock ; or %1, %0"
        : "=m"(*(long *)p) : "r"(v) : "memory" );
}

static inline void *a_cas_p(volatile void *p, void *t, void *s) {
    __asm__( "lock ; cmpxchg %3, %1"
        : "=a"(t), "=m"(*(long *)p) : "a"(t), "r"(s) : "memory" );
    return t;
}

static inline long a_cas_l(volatile void *p, long t, long s) {
    __asm__( "lock ; cmpxchg %3, %1"
        : "=a"(t), "=m"(*(long *)p) : "a"(t), "r"(s) : "memory" );
    return t;
}

static inline int a_cas(volatile int *p, int t, int s) {
    __asm__( "lock ; cmpxchg %3, %1"
        : "=a"(t), "=m"(*p) : "a"(t), "r"(s) : "memory" );
    return t;
}

static inline void *a_swap_p(void *volatile *x, void *v) {
    __asm__( "xchg %0, %1" : "=r"(v), "=m"(*(void **)x) : "0"(v) : "memory" );
    return v;
}
static inline long a_swap_l(volatile void *x, long v) {
    __asm__( "xchg %0, %1" : "=r"(v), "=m"(*(long *)x) : "0"(v) : "memory" );
    return v;
}

static inline void a_or(volatile void *p, int v) {
    __asm__( "lock ; or %1, %0"
        : "=m"(*(int *)p) : "r"(v) : "memory" );
}

static inline void a_and(volatile void *p, int v) {
    __asm__( "lock ; and %1, %0"
        : "=m"(*(int *)p) : "r"(v) : "memory" );
}

static inline int a_swap(volatile int *x, int v) {
    __asm__( "xchg %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory" );
    return v;
}

#define a_xchg a_swap

static inline int a_fetch_add(volatile int *x, int v) {
    __asm__( "lock ; xadd %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory" );
    return v;
}

static inline void a_inc(volatile int *x) {
    __asm__( "lock ; incl %0" : "=m"(*x) : "m"(*x) : "memory" );
}

static inline void a_dec(volatile int *x) {
    __asm__( "lock ; decl %0" : "=m"(*x) : "m"(*x) : "memory" );
}

static inline void a_store(volatile int *p, int x) {
    __asm__( "mov %1, %0" : "=m"(*p) : "r"(x) : "memory" );
}

static inline void a_spin() {
    __asm__ __volatile__( "pause" : : : "memory" );
}

static inline void a_crash() {
    __asm__ __volatile__( "hlt" : : : "memory" );
}

#endif

/// >>> CONTINUE src/misc/ffs.c

/// >>> START src/misc/getgrouplist.c
/// >>> START include/grp.h
#ifndef    _GRP_H
#define    _GRP_H

#define __NEED_size_t
#define __NEED_gid_t

#ifdef _GNU_SOURCE
#define __NEED_FILE
#endif

struct group {
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

struct group  *getgrgid(gid_t);
struct group  *getgrnam(const char *);

int getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
int getgrnam_r(const char *, struct group *, char *, size_t, struct group **);

struct group  *getgrent(void);
void           endgrent(void);
void           setgrent(void);

#ifdef _GNU_SOURCE
struct group  *fgetgrent(FILE *stream);
int putgrent(const struct group *, FILE *);
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int getgrouplist(const char *, gid_t, gid_t *, int *);
int setgroups(size_t, const gid_t *);
int initgroups(const char *, gid_t);
#endif

#endif

/// >>> CONTINUE src/misc/getgrouplist.c

/// >>> START src/misc/getopt_long.c
/// >>> START include/getopt.h
#ifndef _GETOPT_H
#define _GETOPT_H

int getopt(int, char * const [], const char *);
extern char *optarg;
extern int optind, opterr, optopt, optreset;

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

int getopt_long(int, char *const *, const char *, const struct option *, int *);
int getopt_long_only(int, char *const *, const char *, const struct option *, int *);

#define no_argument        0
#define required_argument  1
#define optional_argument  2

#endif

/// >>> CONTINUE src/misc/getopt_long.c

/// >>> START src/misc/nftw.c
/// >>> START include/ftw.h
#ifndef _FTW_H
#define    _FTW_H

#define FTW_F   1
#define FTW_D   2
#define FTW_DNR 3
#define FTW_NS  4
#define FTW_SL  5
#define FTW_DP  6
#define FTW_SLN 7

#define FTW_PHYS  1
#define FTW_MOUNT 2
#define FTW_CHDIR 4
#define FTW_DEPTH 8

struct FTW {
    int base;
    int level;
};

int ftw(const char *, int (*)(const char *, const struct stat *, int), int);
int nftw(const char *, int (*)(const char *, const struct stat *, int, struct FTW *), int, int);

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define ftw64 ftw
#define nftw64 nftw
#endif

#endif

/// >>> CONTINUE src/misc/nftw.c

/// >>> START src/misc/openpty.c
/// >>> START include/pty.h
#ifndef    _PTY_H
#define    _PTY_H

/// >>> START include/termios.h
#ifndef    _TERMIOS_H
#define    _TERMIOS_H

#define __NEED_pid_t

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

/// >>> START arch/x86_64/bits/termios.h
struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t c_line;
    cc_t c_cc[NCCS];
    speed_t __c_ispeed;
    speed_t __c_ospeed;
};

#define VINTR     0
#define VQUIT     1
#define VERASE    2
#define VKILL     3
#define VEOF      4
#define VTIME     5
#define VMIN      6
#define VSWTC     7
#define VSTART    8
#define VSTOP     9
#define VSUSP    10
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16

#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IUCLC   0001000
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IMAXBEL 0020000
#define IUTF8   0040000

#define OPOST  0000001
#define OLCUC  0000002
#define ONLCR  0000004
#define OCRNL  0000010
#define ONOCR  0000020
#define ONLRET 0000040
#define OFILL  0000100
#define OFDEL  0000200
#define NLDLY  0000400
#define NL0    0000000
#define NL1    0000400
#define CRDLY  0003000
#define CR0    0000000
#define CR1    0001000
#define CR2    0002000
#define CR3    0003000
#define TABDLY 0014000
#define TAB0   0000000
#define TAB1   0004000
#define TAB2   0010000
#define TAB3   0014000
#define BSDLY  0020000
#define BS0    0000000
#define BS1    0020000
#define FFDLY  0100000
#define FF0    0000000
#define FF1    0100000

#define VTDLY  0040000
#define VT0    0000000
#define VT1    0040000

#define B0       0000000
#define B50      0000001
#define B75      0000002
#define B110     0000003
#define B134     0000004
#define B150     0000005
#define B200     0000006
#define B300     0000007
#define B600     0000010
#define B1200    0000011
#define B1800    0000012
#define B2400    0000013
#define B4800    0000014
#define B9600    0000015
#define B19200   0000016
#define B38400   0000017

#define B57600   0010001
#define B115200  0010002
#define B230400  0010003
#define B460800  0010004
#define B500000  0010005
#define B576000  0010006
#define B921600  0010007
#define B1000000 0010010
#define B1152000 0010011
#define B1500000 0010012
#define B2000000 0010013
#define B2500000 0010014
#define B3000000 0010015
#define B3500000 0010016
#define B4000000 0010017

#define CBAUD    0010017

#define CSIZE  0000060
#define CS5    0000000
#define CS6    0000020
#define CS7    0000040
#define CS8    0000060
#define CSTOPB 0000100
#define CREAD  0000200
#define PARENB 0000400
#define PARODD 0001000
#define HUPCL  0002000
#define CLOCAL 0004000

#define ISIG   0000001
#define ICANON 0000002
#define ECHO   0000010
#define ECHOE  0000020
#define ECHOK  0000040
#define ECHONL 0000100
#define NOFLSH 0000200
#define TOSTOP 0000400
#define IEXTEN 0100000

#define ECHOCTL 0001000
#define ECHOPRT 0002000
#define ECHOKE 0004000
#define FLUSHO 0010000
#define PENDIN 0040000

#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3

#define TCIFLUSH  0
#define TCOFLUSH  1
#define TCIOFLUSH 2

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define CBAUDEX 0010000
#define CRTSCTS  020000000000
#define EXTPROC 0200000
#define XTABS  0014000
#endif

/// >>> CONTINUE include/termios.h

speed_t cfgetospeed (const struct termios *);
speed_t cfgetispeed (const struct termios *);
int cfsetospeed (struct termios *, speed_t);
int cfsetispeed (struct termios *, speed_t);

int tcgetattr (int, struct termios *);
int tcsetattr (int, int, const struct termios *);

int tcsendbreak (int, int);
int tcdrain (int);
int tcflush (int, int);
int tcflow (int, int);

pid_t tcgetsid (int);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
void cfmakeraw(struct termios *);
int cfsetspeed(struct termios *, speed_t);
#endif

#endif

/// >>> CONTINUE include/pty.h

int openpty(int *, int *, char *, const struct termios *, const struct winsize *);
int forkpty(int *, char *, const struct termios *, const struct winsize *);

#endif

/// >>> CONTINUE src/misc/openpty.c

/// >>> START src/multibyte/mbrtowc.c
#undef N

/// >>> START src/network/ether.c
/// >>> START include/netinet/ether.h
#ifndef _NETINET_ETHER_H
#define _NETINET_ETHER_H

char *ether_ntoa (const struct ether_addr *);
struct ether_addr *ether_aton (const char *);
char *ether_ntoa_r (const struct ether_addr *, char *);
struct ether_addr *ether_aton_r (const char *, struct ether_addr *);
int ether_line(const char *, struct ether_addr *, char *);
int ether_ntohost(char *, const struct ether_addr *);
int ether_hostton(const char *, struct ether_addr *);

#endif

/// >>> CONTINUE src/network/ether.c

/// >>> START src/network/freeaddrinfo.c
/// >>> START include/netdb.h
#ifndef    _NETDB_H
#define    _NETDB_H

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_size_t
#endif

struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};

#define IPPORT_RESERVED 1024

#define AI_PASSIVE      0x01
#define AI_CANONNAME    0x02
#define AI_NUMERICHOST  0x04
#define AI_V4MAPPED     0x08
#define AI_ALL          0x10
#define AI_ADDRCONFIG   0x20
#define AI_NUMERICSERV  0x400

#define NI_NUMERICHOST  0x01
#define NI_NUMERICSERV  0x02
#define NI_NOFQDN       0x04
#define NI_NAMEREQD     0x08
#define NI_DGRAM        0x10
/*#define NI_NUMERICSCOPE */

#define EAI_BADFLAGS   -1
#define EAI_NONAME     -2
#define EAI_AGAIN      -3
#define EAI_FAIL       -4
#define EAI_FAMILY     -6
#define EAI_SOCKTYPE   -7
#define EAI_SERVICE    -8
#define EAI_MEMORY     -10
#define EAI_SYSTEM     -11
#define EAI_OVERFLOW   -12

int getaddrinfo (const char *__restrict, const char *__restrict, const struct addrinfo *__restrict, struct addrinfo **__restrict);
void freeaddrinfo (struct addrinfo *);
int getnameinfo (const struct sockaddr *__restrict, socklen_t, char *__restrict, socklen_t, char *__restrict, socklen_t, int);
const char *gai_strerror(int);

/* Legacy functions follow (marked OBsolete in SUS) */

struct netent {
    char *n_name;
    char **n_aliases;
    int n_addrtype;
    uint32_t n_net;
};

struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};
#define h_addr h_addr_list[0]

struct servent {
    char *s_name;
    char **s_aliases;
    int s_port;
    char *s_proto;
};

struct protoent {
    char *p_name;
    char **p_aliases;
    int p_proto;
};

void sethostent (int);
void endhostent (void);
struct hostent *gethostent (void);

void setnetent (int);
void endnetent (void);
struct netent *getnetent (void);
struct netent *getnetbyaddr (uint32_t, int);
struct netent *getnetbyname (const char *);

void setservent (int);
void endservent (void);
struct servent *getservent (void);
struct servent *getservbyname (const char *, const char *);
struct servent *getservbyport (int, const char *);

void setprotoent (int);
void endprotoent (void);
struct protoent *getprotoent (void);
struct protoent *getprotobyname (const char *);
struct protoent *getprotobynumber (int);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_POSIX_SOURCE) \
 || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 < 200809L) \
 || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700)
struct hostent *gethostbyname (const char *);
struct hostent *gethostbyaddr (const void *, socklen_t, int);
#ifdef __GNUC__
__attribute__((const))
#endif
int *__h_errno_location(void);
#define h_errno (*__h_errno_location())
#define HOST_NOT_FOUND 1
#define TRY_AGAIN      2
#define NO_RECOVERY    3
#define NO_DATA        4
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
void herror(const char *);
const char *hstrerror(int);
int gethostbyname_r(const char *, struct hostent *, char *, size_t, struct hostent **, int *);
int gethostbyname2_r(const char *, int, struct hostent *, char *, size_t, struct hostent **, int *);
struct hostent *gethostbyname2(const char *, int);
int gethostbyaddr_r(const void *, socklen_t, int, struct hostent *, char *, size_t, struct hostent **, int *);
int getservbyport_r(int, const char *, struct servent *, char *, size_t, struct servent **);
int getservbyname_r(const char *, const char *, struct servent *, char *, size_t, struct servent **);
#define EAI_NODATA     -5
#define EAI_ADDRFAMILY -9
#define EAI_INPROGRESS -100
#define EAI_CANCELED   -101
#define EAI_NOTCANCELED -102
#define EAI_ALLDONE    -103
#define EAI_INTR       -104
#define EAI_IDN_ENCODE -105
#define NI_MAXHOST 255
#define NI_MAXSERV 32
#endif

#endif

/// >>> CONTINUE src/network/freeaddrinfo.c

/// >>> START src/network/getifaddrs.c
/// >>> START include/ifaddrs.h
#ifndef _IFADDRS_H
#define _IFADDRS_H

struct ifaddrs {
    struct ifaddrs *ifa_next;
    char *ifa_name;
    unsigned ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    union {
        struct sockaddr *ifu_broadaddr;
        struct sockaddr *ifu_dstaddr;
    } ifa_ifu;
    void *ifa_data;
};
#define ifa_broadaddr ifa_ifu.ifu_broadaddr
#define ifa_dstaddr ifa_ifu.ifu_dstaddr

void freeifaddrs(struct ifaddrs *ifp);
int getifaddrs(struct ifaddrs **ifap);

#endif

/// >>> CONTINUE src/network/getifaddrs.c
#undef next

/// >>> START src/network/htonl.c
/// >>> START include/byteswap.h
#ifndef _BYTESWAP_H
#define _BYTESWAP_H

static __inline uint16_t __bswap_16(uint16_t __x) {
    return __x<<8 | __x>>8;
}

static __inline uint32_t __bswap_32(uint32_t __x) {
    return __x>>24 | (__x>>8&0xff00) | (__x<<8&0xff0000) | __x<<24;
}

static __inline uint64_t __bswap_64(uint64_t __x) {
    return (__bswap_32(__x)+0ULL)<<32 | __bswap_32(__x>>32);
}

#define bswap_16(x) __bswap_16(x)
#define bswap_32(x) __bswap_32(x)
#define bswap_64(x) __bswap_64(x)

#endif

/// >>> CONTINUE src/network/htonl.c

/// >>> START src/passwd/fgetgrent.c
/// >>> START src/passwd/pwf.h
/// >>> START include/pwd.h
#ifndef _PWD_H
#define _PWD_H

#define __NEED_size_t
#define __NEED_uid_t
#define __NEED_gid_t

#ifdef _GNU_SOURCE
#define __NEED_FILE
#endif

struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

void setpwent (void);
void endpwent (void);
struct passwd *getpwent (void);

struct passwd *getpwuid (uid_t);
struct passwd *getpwnam (const char *);
int getpwuid_r (uid_t, struct passwd *, char *, size_t, struct passwd **);
int getpwnam_r (const char *, struct passwd *, char *, size_t, struct passwd **);

#ifdef _GNU_SOURCE
struct passwd *fgetpwent(FILE *);
int putpwent(const struct passwd *, FILE *);
#endif

#endif

/// >>> CONTINUE src/passwd/pwf.h
/// >>> START include/shadow.h
#ifndef _SHADOW_H
#define _SHADOW_H

#define    __NEED_FILE
#define __NEED_size_t

#define    SHADOW "/etc/shadow"

struct spwd {
    char *sp_namp;
    char *sp_pwdp;
    long sp_lstchg;
    long sp_min;
    long sp_max;
    long sp_warn;
    long sp_inact;
    long sp_expire;
    unsigned long sp_flag;
};

void setspent(void);
void endspent(void);
struct spwd *getspent(void);
struct spwd *fgetspent(FILE *);
struct spwd *sgetspent(const char *);
int putspent(const struct spwd *, FILE *);

struct spwd *getspnam(const char *);
int getspnam_r(const char *, struct spwd *, char *, size_t, struct spwd **);

int lckpwdf(void);
int ulckpwdf(void);

#endif

/// >>> CONTINUE src/passwd/pwf.h

/// >>> CONTINUE src/passwd/fgetgrent.c

/// >>> START src/search/hsearch.c
/// >>> START include/search.h
#ifndef _SEARCH_H
#define _SEARCH_H

#define __NEED_size_t

typedef enum { FIND, ENTER } ACTION;
typedef enum { preorder, postorder, endorder, leaf } VISIT;

typedef struct entry {
    char *key;
    void *data;
} ENTRY;

int hcreate(size_t);
void hdestroy(void);
ENTRY *hsearch(ENTRY, ACTION);

void insque(void *, void *);
void remque(void *);

void *lsearch(const void *, void *, size_t *, size_t,
    int (*)(const void *, const void *));
void *lfind(const void *, const void *, size_t *, size_t,
    int (*)(const void *, const void *));

void *tdelete(const void *__restrict, void **__restrict, int(*)(const void *, const void *));
void *tfind(const void *, void *const *, int(*)(const void *, const void *));
void *tsearch(const void *, void **, int (*)(const void *, const void *));
void twalk(const void *, void (*)(const void *, VISIT, int));

#ifdef _GNU_SOURCE
struct qelem {
    struct qelem *q_forw, *q_back;
    char q_data[1];
};

void tdestroy(void *, void (*)(void *));
#endif

#endif

/// >>> CONTINUE src/search/hsearch.c

/// >>> START src/signal/block.c
/// >>> START src/internal/pthread_impl.h
/// >>> START src/internal/futex.h
#ifndef _INTERNAL_FUTEX_H
#define _INTERNAL_FUTEX_H

#define FUTEX_WAIT        0
#define FUTEX_WAKE        1
#define FUTEX_FD        2
#define FUTEX_REQUEUE        3
#define FUTEX_CMP_REQUEUE    4
#define FUTEX_WAKE_OP        5
#define FUTEX_LOCK_PI        6
#define FUTEX_UNLOCK_PI        7
#define FUTEX_TRYLOCK_PI    8
#define FUTEX_WAIT_BITSET    9

#define FUTEX_CLOCK_REALTIME 256

int __futex(volatile int *, int, int, void *);

#endif

/// >>> CONTINUE src/internal/pthread_impl.h
/// >>> START arch/x86_64/pthread_arch.h
static inline struct pthread *__pthread_self() {
    struct pthread *self;
    __asm__ __volatile__ ("mov %%fs:0,%0" : "=r" (self) );
    return self;
}

#define TP_ADJ(p) (p)

#define CANCEL_REG_IP 16

/// >>> CONTINUE src/internal/pthread_impl.h

/// >>> CONTINUE src/signal/block.c

/// >>> START src/signal/sigaction.c
/// >>> START src/internal/ksigaction.h
/* This is the structure used for the rt_sigaction syscall on most archs,
 * but it can be overridden by a file with the same name in the top-level
 * arch dir for a given arch, if necessary. */
struct k_sigaction {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned mask[2];
};

/// >>> CONTINUE src/signal/sigaction.c

/// >>> START src/time/ftime.c
/// >>> START include/sys/timeb.h
#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H

#define __NEED_time_t

struct timeb {
    time_t time;
    unsigned short millitm;
    short timezone, dstflag;
};

int ftime(struct timeb *);

#endif

/// >>> CONTINUE src/time/ftime.c

#endif    /* MUSL_H */
