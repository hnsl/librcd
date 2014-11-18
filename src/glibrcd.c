/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/* musl <-> librcd compatibility layer */

#include "rcd.h"
#include "atomic.h"
#include "musl.h"
#include "asmlib.h"
#include "vm-internal.h"

#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES*  (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

#define BITOP(a,b,op) ((a)[(size_t)(b)/(8*sizeof(*(a)))] op (size_t)1<<((size_t)(b)%(8*sizeof(*(a)))))

union sa {
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
};

struct aibuf {
    struct addrinfo ai;
    union sa sa;
};

const char __gmt[] = "GMT";

void __lock(volatile int* lock) {
    atomic_spinlock_lock((int8_t*) lock);
}

void __unlock(volatile int* lock) {
    atomic_spinlock_unlock((int8_t*) lock);
}

int memcmp(const void* vl, const void* vr, size_t n) {
    const uint8_t* l = vl,* r = vr;
    for (; n && *l == *r; n--, l++, r++);
    return n? *l - *r: 0;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    return A_memcpy(dest, src, n);
}

void* memmove(void* dest, const void* src, size_t n) {
    return A_memmove(dest, src, n);
}

void* mempcpy(void* dest, const void* src, size_t n) {
    return memcpy(dest, src, n) + n;
}

void* memset(void* dest, int c, size_t n) {
    return A_memset(dest, c, n);
}

void* malloc(size_t size) {
    if (size == 0)
        return 0;
    size_t total_size = size + sizeof(size_t);
    void* primary_ptr = vm_mmap_reserve(total_size, 0);
    *((size_t*) primary_ptr) = total_size;
    void* ptr = primary_ptr + sizeof(size_t);
    return ptr;
}

void* calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    if (total_size == 0)
        return 0;
    void* ptr = malloc(total_size);
    memset(ptr, 0, total_size);
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (ptr == 0)
        return 0;
    void* primary_ptr = ptr - sizeof(size_t);
    size_t old_total_size = *((size_t*) primary_ptr);
    if (size == 0) {
        vm_mmap_unreserve(primary_ptr, old_total_size);
        return 0;
    } else {
        size_t old_size = (old_total_size - sizeof(size_t));
        void* new_ptr = malloc(size);
        memcpy(new_ptr, ptr, MIN(old_size, size));
        vm_mmap_unreserve(primary_ptr, old_total_size);
        return new_ptr;
    }
}

void free(void* ptr) {
    if (ptr == 0)
        return;
    void* primary_ptr = ptr - sizeof(size_t);
    size_t total_size = *((size_t*) primary_ptr);
    vm_mmap_unreserve(primary_ptr, total_size);
}

#define ERRSTR(a, b) {a, b},

const struct {const int err; const char* str;} error_strings[] = {
    ERRSTR(EILSEQ, "Illegal byte sequence")
    ERRSTR(EDOM, "Argument outside domain")
    ERRSTR(ERANGE, "Result not representable")
    ERRSTR(ENOTTY, "Not a tty")
    ERRSTR(EACCES, "Permission denied")
    ERRSTR(EPERM, "Operation not permitted")
    ERRSTR(ENOENT, "No such file or directory")
    ERRSTR(ESRCH, "No such process")
    ERRSTR(EEXIST, "File exists")
    ERRSTR(EOVERFLOW, "Value too large for defined data type")
    ERRSTR(ENOSPC, "No space left on device")
    ERRSTR(ENOMEM, "Out of memory")
    ERRSTR(EBUSY, "Device or resource busy")
    ERRSTR(EINTR, "Interrupted system call")
    ERRSTR(EAGAIN, "Operation would block")
    ERRSTR(ESPIPE, "Illegal seek")
    ERRSTR(EXDEV, "Cross-device link")
    ERRSTR(EROFS, "Read-only file system")
    ERRSTR(ENOTEMPTY, "Directory not empty")
    ERRSTR(ECONNRESET, "Connection reset by peer")
    ERRSTR(ETIMEDOUT, "Connection timed out")
    ERRSTR(ECONNREFUSED, "Connection refused")
    ERRSTR(EHOSTDOWN, "Host is down")
    ERRSTR(EHOSTUNREACH, "No route to host")
    ERRSTR(EADDRINUSE, "Address already in use")
    ERRSTR(EPIPE, "Broken pipe")
    ERRSTR(EIO, "I/O error")
    ERRSTR(ENXIO, "No such device or address")
    ERRSTR(ENOTBLK, "Block device required")
    ERRSTR(ENODEV, "No such device")
    ERRSTR(ENOTDIR, "Not a directory")
    ERRSTR(EISDIR, "Is a directory")
    ERRSTR(ETXTBSY, "Text file busy")
    ERRSTR(ENOEXEC, "Exec format error")
    ERRSTR(EINVAL, "Invalid argument")
    ERRSTR(E2BIG, "Argument list too long")
    ERRSTR(ELOOP, "Too many levels of symbolic links")
    ERRSTR(ENAMETOOLONG, "Filename too long")
    ERRSTR(ENFILE, "File table overflow")
    ERRSTR(EMFILE, "Too many open files")
    ERRSTR(EBADF, "Bad file number")
    ERRSTR(ECHILD, "No child processes")
    ERRSTR(EFAULT, "Bad address")
    ERRSTR(EFBIG, "File too large")
    ERRSTR(EMLINK, "Too many links")
    ERRSTR(ENOLCK, "No record locks available")
    ERRSTR(EDEADLK, "Resource deadlock would occur")
    ERRSTR(ENOSYS, "Function not supported")
    ERRSTR(ENOMSG, "No message of desired type")
    ERRSTR(EIDRM, "Identifier removed")
    ERRSTR(ENOSTR, "Device not a stream")
    ERRSTR(ENODATA, "No data available")
    ERRSTR(ETIME, "Timer expired")
    ERRSTR(ENOSR, "Out of streams resources")
    ERRSTR(ENOLINK, "Link has been severed")
    ERRSTR(EPROTO, "Protocol error")
    ERRSTR(EBADMSG, "Not a data message")
    ERRSTR(EBADFD, "File descriptor in bad state")
    ERRSTR(ENOTSOCK, "Socket operation on non-socket")
    ERRSTR(EDESTADDRREQ, "Destination address required")
    ERRSTR(EMSGSIZE, "Message too long")
    ERRSTR(EPROTOTYPE, "Protocol wrong type for socket")
    ERRSTR(ENOPROTOOPT, "Protocol not available")
    ERRSTR(EPROTONOSUPPORT,"Protocol not supported")
    ERRSTR(ESOCKTNOSUPPORT,"Socket type not supported")
    ERRSTR(EOPNOTSUPP, "Operation not supported on socket")
    ERRSTR(EPFNOSUPPORT, "Protocol family not supported")
    ERRSTR(EAFNOSUPPORT, "Address family not supported by protocol")
    ERRSTR(EADDRNOTAVAIL,"Cannot assign requested address")
    ERRSTR(ENETDOWN, "Network is down")
    ERRSTR(ENETUNREACH, "Network is unreachable")
    ERRSTR(ENETRESET, "Network dropped connection because of reset")
    ERRSTR(ECONNABORTED, "Software caused connection abort")
    ERRSTR(ENOBUFS, "No buffer space available")
    ERRSTR(EISCONN, "Socket is connected")
    ERRSTR(ENOTCONN, "Socket is not connected")
    ERRSTR(ESHUTDOWN, "Cannot send after socket shutdown")
    ERRSTR(EALREADY, "Operation already in progress")
    ERRSTR(EINPROGRESS, "Operation now in progress")
    ERRSTR(ESTALE, "Stale NFS file handle")
    ERRSTR(EREMOTEIO, "Remote I/O error")
    ERRSTR(EDQUOT, "Quota exceeded")
    ERRSTR(ENOMEDIUM, "No medium found")
    ERRSTR(EMEDIUMTYPE, "Wrong medium type")
    ERRSTR(0, "No error information")
};

char* strerror(int err) {
    for (int i = 0; i < LENGTHOF(error_strings); i++) {
        if (error_strings[i].err == err)
            return (char*) error_strings[i].str;
    }
    return 0;
}

static const char signal_strings[] =
	"Unknown signal\0"
	"Hangup\0"
	"Interrupt\0"
	"Quit\0"
	"Illegal instruction\0"
	"Trace/breakpoint trap\0"
	"Aborted\0"
	"Bus error\0"
	"Floating point exception\0"
	"Killed\0"
	"User defined signal 1\0"
	"Segmentation fault\0"
	"User defined signal 2\0"
	"Broken pipe\0"
	"Alarm clock\0"
	"Terminated\0"
	"Stack fault\0"
	"Child exited\0"
	"Continued\0"
	"Stopped (signal)\0"
	"Stopped\0"
	"Stopped (tty input)\0"
	"Stopped (tty output)\0"
	"Urgent I/O condition\0"
	"CPU time limit exceeded\0"
	"File size limit exceeded\0"
	"Virtual timer expired\0"
	"Profiling timer expired\0"
	"Window changed\0"
	"I/O possible\0"
	"Power failure\0"
	"Bad system call";

char *getenv(const char *name) {
    // NOT IMPLEMENTED: USING "ENVIRONMENT FEATURE" IS LEGACY AND DISCOURAGED
    return 0;
}

int setenv(const char *name, const char *value, int overwrite) {
    // NOT IMPLEMENTED: USING "ENVIRONMENT FEATURE" IS LEGACY AND DISCOURAGED
    return 0;
}

int unsetenv(const char *name) {
    // NOT IMPLEMENTED: USING "ENVIRONMENT FEATURE" IS LEGACY AND DISCOURAGED
    return 0;
}

unsigned long int strtoul(const char* nptr, char** endptr, int base) {
    static char lower[] = "abcdefghijklmnopqrstuvwxyz";
    unsigned long result = 0UL;
    char *nptr_orig = (char *) nptr;
    int neg = false;
    char *cp, c;
    int val;
    int sawdigs = false;
    /*
     * The strtoul() function converts the initial part of the
     * string in nptr to an unsigned long integer value according
     * to the given base, which must be between 2 and 36 inclusive,
     * or be the special value 0.
     */
    if ((base != 0 && (base < 2 || base > 36)) || nptr == NULL) {
        if (endptr != NULL)
            *endptr = nptr_orig;
        errno = EINVAL;
        return 0;
    }
    /*
     * The string must [sic] begin with an arbitrary amount of white space
     * (as determined by isspace(3)) followed by a single optional
     * `+' or `-' sign.
     */
    while (isspace(*nptr))
        nptr++;
    if (*nptr == '+') {
        nptr++;
    } else if (*nptr == '-') {
        nptr++;
        neg = true;
    }
    /*
     * If base is zero or 16, the string may then include a `0x' prefix,
     * and the number will be read in base 16; otherwise, a zero base is
     * taken as 10 (decimal) unless the next character is `0', in which
     * case it is taken as 8 (octal).
     */
    if ((base == 0 || base == 16) && nptr[0] == '0' && (nptr[1] == 'x' || nptr[1] == 'X')) {
        base = 16; /* force it */
        nptr += 2; /* skip 0x */
    } else if ((base == 0 || base == 8) && nptr[0] == '0') {
        base = 8;
        nptr++;
        /* We cant distinguish 0 in base 10 from octal prefix, this is expected behavior from _ipparse */
        sawdigs = true;
    } else if (base == 0) {
        base = 10;
    }
    /*
     * The remainder of the string is converted to an unsigned long int
     * value in the obvious manner, stopping at the first character
     * which is not a valid digit in the given base. (In bases above 10,
     * the letter `A' in either upper or lower case represents 10,
     * `B' represents 11, and so forth, with `Z' representing 35.)
     */
    for (; *nptr != '\0'; nptr++) {
        c = *nptr;
        switch (c) {
            case '0': case '1': case '2':
            case '3': case '4': case '5':
            case '6': case '7': case '8':
            case '9':
                val = c - '0';
                if (val >= base) /* even base 2 allowed ... */
                    goto out;
                result *= base;
                result += val;
                sawdigs = true;
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
                c += 'a' - 'A'; /* downcase */
                /* fall through */
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
                cp = strchr(lower, c);
                val = cp - lower;
                val += 10; /* 'a' == 10 */
                if (val >= base)
                    goto out;
                result *= base;
                result += val;
                sawdigs = true;
                break;
            default:
                goto out;
        }
    }
out:
    /*
     * If endptr is not NULL, strtoul() stores the address of the
     * first invalid character in *endptr. If there were no digits
     * at all, strtoul() stores the original value of nptr in *endptr
     * (and returns 0).  In particular, if *nptr is not `\0' but
     * **endptr is `\0' on return, the entire string is valid.
     */
    if (endptr != NULL) {
        if (!sawdigs) {
            *endptr = nptr_orig;
            return 0;
        } else
            *endptr = (char *) nptr;
    }
    /*
     * RETURN VALUE
     * The strtoul() function returns either the result of the
     * conversion or, if there was a leading minus sign, the
     * negation of the result of the conversion, unless the original
     * (non-negated) value would overflow; in the latter case,
     * strtoul() returns ULONG_MAX and sets the global variable errno
     * to ERANGE.
     */
    /*
     * ADR: This computation is probably bogus.  If it's a
     * problem, upgrade to a modern system.
     */
    if (neg && result == ULONG_MAX) {
        errno = ERANGE;
        return ULONG_MAX;
    } else if (neg)
        result = -result;

    return result;
}

unsigned long long int strtoull(const char* nptr, char** endptr, int base) {
    return strtoul(nptr, endptr, base);
}

long strtol(const char* restrict nptr, char** restrict endptr, int base) {
    return (long) strtoul(nptr, endptr, base);
}

int vfprintf(FILE* stream, const char* format, va_list ap) {
    int32_t ret_value;
    sub_heap {
        // Get required size of the buffer.
        va_list len_ap;
        va_copy(len_ap, ap);
        int32_t min_buffer_len = vsnprintf(0, 0, format, len_ap);
        if (min_buffer_len < 0) {
            ret_value = min_buffer_len;
            break;
        }
        // Allocate the required buffer and start formatting including space for null terminator.
        fstr_t buffer = fss(fstr_alloc_buffer(min_buffer_len + 1));
        int32_t max_str_len = vsnprintf((void*) buffer.str, buffer.len, format, ap);
        if (max_str_len < 0) {
            ret_value = max_str_len;
            break;
        }
        int32_t write_len = MIN(max_str_len, buffer.len);
        ret_value = fwrite(buffer.str, 1, write_len, stream);
    }
    return ret_value;
}

_Noreturn void exit(int code) {
    lwt_exit(code);
}

int __ipparse(void *dest, int family, const char *__ipparse_c__s0);

int32_t getaddrinfo(const char* restrict host, const char* restrict serv, const struct addrinfo* restrict hint, struct addrinfo** restrict res) {
    if (host && strlen(host) > 255)
        return EAI_NONAME;
    if (serv && strlen(serv) > 32)
        return EAI_SERVICE;
    int type = hint? hint->ai_socktype: 0;
    int proto = hint? hint->ai_protocol: 0;
    if (type && !proto)
        proto = (type == SOCK_DGRAM? IPPROTO_UDP: IPPROTO_TCP);
    if (!type && proto)
        type = (proto == IPPROTO_UDP? SOCK_DGRAM: SOCK_STREAM);
    int flags = hint? hint->ai_flags: 0;
    int family = hint? hint->ai_family: AF_UNSPEC;
    unsigned long port = 0;
    union sa sa = {{0}};
    char line[512];
    if (serv) {
        if (!*serv)
            return EAI_SERVICE;
        char *z;
        port = strtoul(serv, &z, 10);
        if (*z) {
            size_t servlen = strlen(serv);
            char *end = line;
            if (flags & AI_NUMERICSERV)
                return EAI_SERVICE;
            bool is_eof;
            // Uninterruptibly reading /etc/services.
            uninterruptible {
                FILE* f = fopen("/etc/services", "r");
                if (!f) {
                    is_eof = true;
                    break;
                }
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, serv, servlen) || !isspace(line[servlen]))
                        continue;
                    port = strtoul(line + servlen, &end, 10);
                    if (strncmp(end, proto == IPPROTO_UDP? "/udp": "/tcp", 4))
                        continue;
                    break;
                }
                is_eof = (feof(f) != 0);
                fclose(f);
            }
            if (is_eof)
                return EAI_SERVICE;
        }
        if (port > 65535)
            return EAI_SERVICE;
        port = htons(port);
    }
    if (!host) {
        if (family == AF_UNSPEC)
            family = AF_INET;
        struct aibuf* buf = calloc(sizeof(*buf), 1);
        if (!buf)
            return EAI_MEMORY;
        buf->ai.ai_protocol = proto;
        buf->ai.ai_socktype = type;
        buf->ai.ai_addr = (void *) &buf->sa.sin;
        buf->ai.ai_addrlen = family == AF_INET6? sizeof(sa.sin6): sizeof(sa.sin);
        buf->ai.ai_family = family;
        buf->sa.sin.sin_family = family;
        buf->sa.sin.sin_port = port;
        if (!(flags & AI_PASSIVE)) {
            if (family == AF_INET) {
                0[(uint8_t*) & buf->sa.sin.sin_addr.s_addr] = 127;
                3[(uint8_t*) & buf->sa.sin.sin_addr.s_addr] = 1;
            } else buf[0].sa.sin6.sin6_addr.s6_addr[15] = 1;
        }
        *res = &buf->ai;
        return 0;
    }
    if (!*host)
        return EAI_NONAME;
    // Try as a numeric address
    if (__ipparse(&sa, family, host) >= 0) {
        struct aibuf* buf = calloc(sizeof(*buf), 1);
        if (!buf)
            return EAI_MEMORY;
        family = sa.sin.sin_family;
        buf->ai.ai_protocol = proto;
        buf->ai.ai_socktype = type;
        buf->ai.ai_addr = (void *) &buf->sa.sin;
        buf->ai.ai_addrlen = family == AF_INET6? sizeof(sa.sin6): sizeof(sa.sin);
        buf->ai.ai_family = family;
        buf->ai.ai_canonname = (char *) host;
        buf->sa = sa;
        buf->sa.sin.sin_port = port;
        *res = &buf->ai;
        return 0;
    }
    if (flags & AI_NUMERICHOST)
        return EAI_NONAME;
    // Use the built-in host name resolver in rio.
    bool resolve_ok = false;
    sub_heap {
        list(uint32_t)* ipv4_list;
        try {
            ipv4_list = rio_resolve_host_ipv4_addr(fstr_fix_cstr(host));
            resolve_ok = true;
        } catch (exception_io, e) {}
        if (!resolve_ok)
            break;
        size_t cnt = list_count(ipv4_list, uint32_t);
        struct aibuf* buf = calloc(sizeof(*buf), cnt);
        size_t i = 0;
        list_foreach(ipv4_list, uint32_t, ipv4_addr) {
            buf[i].ai.ai_protocol = proto;
            buf[i].ai.ai_socktype = type;
            buf[i].ai.ai_addr = (void *) &buf[i].sa.sin;
            buf[i].ai.ai_addrlen = sizeof(sa.sin);
            buf[i].ai.ai_family = AF_INET;
            buf[i].sa.sin.sin_family = AF_INET;
            buf[i].sa.sin.sin_port = port;
            buf[i].sa.sin.sin_addr.s_addr = htonl(ipv4_addr);
            buf[i].ai.ai_next = &buf[i + 1].ai;
            buf[i].ai.ai_canonname = (char *) host;
            i++;
        }
        buf[cnt - 1].ai.ai_next = 0;
        *res = &buf->ai;
    }
    if (!resolve_ok)
        return EAI_NONAME;
    return 0;
}

struct tm* localtime_r(const time_t* restrict t, struct tm* restrict tm) {
    uint128_t clock_ns = (*t) * RIO_NS_SEC;
    rio_date_time_t dt = rio_clock_to_date_time(rio_epoch_to_clock_time(clock_ns));
    tm->tm_sec = dt.second;
    tm->tm_min = dt.minute;
    tm->tm_hour = dt.hour;
    tm->tm_mday = dt.month_day;
    tm->tm_mon = dt.month - 1;
    tm->tm_year = dt.year - 1900;
    tm->tm_wday = (dt.week_day + 1) % 7;
    tm->tm_yday = dt.year_day;
    tm->tm_isdst = 0;
    tm->tm_gmtoff = 0;
    tm->tm_zone = __gmt;
    return tm;
}

// Make __localtime_r alias for localtime_r.
struct tm* __localtime_r(const time_t* restrict t, struct tm* restrict tm)
__attribute__ ((weak, alias ("localtime_r")));
