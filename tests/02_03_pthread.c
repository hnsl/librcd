/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "atomic.h"
#include "linux.h"

void* pthread_test0(void* arg_ptr) {
    return arg_ptr;
}

void* pthread_test1(void* arg_ptr) {
    pthread_t* thread = arg_ptr;
    pthread_t self = pthread_self();
    atest(self == *thread);
    pthread_exit(arg_ptr);
    atest(false);
}

typedef struct phtread_test2_args {
    pthread_mutex_t* test_mutex;
} phtread_test2_args_t;

void* pthread_test2(void* arg_ptr) {
    phtread_test2_args_t* args = arg_ptr;
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000};
    atest(pthread_mutex_timedlock(args->test_mutex, &ts) == ETIMEDOUT);
    return arg_ptr;
}

typedef struct phtread_test3_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
    int32_t* shared_memory;
} phtread_test3_args_t;

void* pthread_test3(void* arg_ptr) {
    phtread_test3_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(*args->shared_memory == 0);
    *args->shared_memory = 42;
    atest(pthread_cond_signal(args->test_cond) == 0);
    atest(pthread_cond_wait(args->test_cond, args->test_mutex) == 0);
    atest(*args->shared_memory == 43);
    *args->shared_memory = 44;
    atest(pthread_cond_signal(args->test_cond) == 0);
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
    return 0;
}

typedef struct phtread_test4_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
} phtread_test4_args_t;

void* pthread_test4(void* arg_ptr) {
    phtread_test4_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
    return 0;
}

int32_t pthread_test5_shared_memory = 0;

void pthread_test5_cleanup(void* arg_ptr) {
    atest(false);
}

void pthread_test5_once0() {
    atest(pthread_test5_shared_memory == 0);
    pthread_test5_shared_memory = 1;
}

void pthread_test5_once1() {
    if (pthread_test5_shared_memory == 1) {
        pthread_test5_shared_memory = 2;
        pthread_cleanup_push(pthread_test5_cleanup, 0); {
            pthread_testcancel();
            pthread_cancel(pthread_self());
        } pthread_cleanup_pop(0);
        pthread_testcancel();
        atest(false);
    } else {
        atest(pthread_test5_shared_memory == 2);
        pthread_test5_shared_memory = 3;
    }
}

void* pthread_test5(void* arg_ptr) {
    static pthread_once_t test_once = PTHREAD_ONCE_INIT;
    pthread_once(&test_once, pthread_test5_once1);
    atest(false);
    return 0;
}

typedef struct phtread_test6_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
    int32_t* shared_memory;
} phtread_test6_args_t;

void pthread_test6_cleanup0(void* arg_ptr) {
    phtread_test6_args_t* args = arg_ptr;
    atest(*args->shared_memory == 3);
    (*args->shared_memory)++;
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
}

void pthread_test6_cleanup1(void* arg_ptr) {
    phtread_test6_args_t* args = arg_ptr;
    (*args->shared_memory)++;
}

void pthread_test6_cleanup_nocall(void* arg_ptr) {
    atest(false);
}

void* pthread_test6(void* arg_ptr) {
    phtread_test6_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(*args->shared_memory == 1);
    pthread_cleanup_push(pthread_test6_cleanup0, args); {
        // Test
        pthread_cleanup_push(pthread_test6_cleanup_nocall, 0); {
            lwt_yield();
        } pthread_cleanup_pop(0);
        pthread_cleanup_push(pthread_test6_cleanup1, args); {
            pthread_cleanup_push(pthread_test6_cleanup1, args); {
                atest(pthread_cond_signal(args->test_cond) == 0);
                atest(pthread_cond_wait(args->test_cond, args->test_mutex) == 0);
                atest(false);
            } pthread_cleanup_pop(1);
            atest(false);
        } pthread_cleanup_pop(0);
        atest(false);
    } pthread_cleanup_pop(1);
    atest(false);
    return 0;
}

typedef struct phtread_test7_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
} phtread_test7_args_t;

void* pthread_test7(void* arg_ptr) {
    phtread_test7_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(pthread_cond_signal(args->test_cond) == 0);
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
    return 0;
}

typedef struct phtread_test8_args {
    pthread_mutex_t* test_mutex;
    int32_t* shared_memory;
} phtread_test8_args_t;

void* pthread_test8(void* arg_ptr) {
    phtread_test8_args_t* args = arg_ptr;
    for (int i = 0; i < 100; i++) {
        atest(pthread_mutex_lock(args->test_mutex) == 0);
        lwt_yield();
        int32_t count = *args->shared_memory;
        lwt_yield();
        *args->shared_memory = (count + 1);
        lwt_yield();
        atest(pthread_mutex_unlock(args->test_mutex) == 0);
        lwt_yield();
    }
    return (void*) pthread_self();
}

typedef struct phtread_test9_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond0;
    pthread_cond_t* test_cond1;
    int32_t* shared_memory;
} phtread_test9_args_t;

void pthread_test9_cleanup1(void* arg_ptr) {
    phtread_test9_args_t* args = arg_ptr;
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
}

void* pthread_test9(void* arg_ptr) {
    phtread_test9_args_t* args = arg_ptr;
    for (;;) {
        lwt_yield();
        atest(pthread_mutex_lock(args->test_mutex) == 0);
        lwt_yield();
        atest(pthread_cond_signal(args->test_cond1) == 0);
        lwt_yield();
        pthread_cleanup_push(pthread_test9_cleanup1, args); {
            for (;;) {
                lwt_yield();
                atest(pthread_cond_wait(args->test_cond0, args->test_mutex) == 0);
                lwt_yield();
                int32_t count = *args->shared_memory;
                lwt_yield();
                *args->shared_memory = (count + 1);
                lwt_yield();
                atest(pthread_cond_signal(args->test_cond1) == 0);
                lwt_yield();
            }
        } pthread_cleanup_pop(0);
        atest(false);
    }
    return 0;
}

typedef struct phtread_test10_args {
    pthread_rwlock_t* test_rwlock;
    int32_t* shared_memory;
} phtread_test10_args_t;

void* pthread_test10(void* arg_ptr) {
    phtread_test10_args_t* args = arg_ptr;
    for (int i = 0; i < 200; i++) {
        if ((i % 2) == 0) {
            switch (i % 6) {{
            } case 0: {
                int32_t tryrdlock_r = pthread_rwlock_tryrdlock(args->test_rwlock);
                if (tryrdlock_r == 0)
                    break;
                atest(tryrdlock_r == EBUSY);
                atest(pthread_rwlock_rdlock(args->test_rwlock) == 0);
                break;
            } case 2: {
                struct timespec ts = {.tv_sec = 0, .tv_nsec = 50000};
                uint128_t start_time = rio_get_time_timer();
                int32_t timedrdlock_r = pthread_rwlock_timedrdlock(args->test_rwlock, &ts);
                if (timedrdlock_r == 0)
                    break;
                atest(timedrdlock_r == ETIMEDOUT);
                uint128_t end_time = rio_get_time_timer();
                atest(end_time >= (start_time + 50000));
                atest(pthread_rwlock_rdlock(args->test_rwlock) == 0);
                break;
            } case 4: {
                atest(pthread_rwlock_rdlock(args->test_rwlock) == 0);
                break;
            }}
            int32_t sm = *args->shared_memory;
            lwt_yield();
            atest(sm == *args->shared_memory);
            for (int j = 0; j < (sm % 3); j++) {
                if ((i % 4) == 0) {
                    atest(pthread_rwlock_tryrdlock(args->test_rwlock) == 0);
                } else {
                    atest(pthread_rwlock_rdlock(args->test_rwlock) == 0);
                }
                lwt_yield();
                atest(sm == *args->shared_memory);
            }
            for (int j = 0; j < (sm % 3); j++) {
                lwt_yield();
                atest(pthread_rwlock_unlock(args->test_rwlock) == 0);
            }
            lwt_yield();
            atest(pthread_rwlock_unlock(args->test_rwlock) == 0);
            lwt_yield();
        } else if (i == 1 || i == 3 || i == 5) {
            switch (i % 6) {{
            } case 1: {
                int32_t tryrdlock_r = pthread_rwlock_trywrlock(args->test_rwlock);
                if (tryrdlock_r == 0)
                    break;
                atest(tryrdlock_r == EBUSY);
                atest(pthread_rwlock_wrlock(args->test_rwlock) == 0);
                break;
            } case 3: {
                struct timespec ts = {.tv_sec = 0, .tv_nsec = 50000};
                uint128_t start_time = rio_get_time_timer();
                int32_t timedwrlock_r = pthread_rwlock_timedwrlock(args->test_rwlock, &ts);
                if (timedwrlock_r == 0)
                    break;
                atest(timedwrlock_r == ETIMEDOUT);
                uint128_t end_time = rio_get_time_timer();
                atest(end_time >= (start_time + 50000));
                atest(pthread_rwlock_wrlock(args->test_rwlock) == 0);
                break;
            } case 5: {
                atest(pthread_rwlock_wrlock(args->test_rwlock) == 0);
                break;
            }}
            int32_t sm = *args->shared_memory;
            lwt_yield();
            atest(sm == *args->shared_memory);
            for (int j = 0; j < (sm % 3); j++) {
                atest(pthread_rwlock_tryrdlock(args->test_rwlock) == EDEADLK);
                lwt_yield();
            }
            (*args->shared_memory)++;
            lwt_yield();
            atest(sm == (*args->shared_memory - 1));
            atest(pthread_rwlock_unlock(args->test_rwlock) == 0);
            lwt_yield();
        }
    }
    return (void*) pthread_self();
}

typedef struct phtread_test11_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
} phtread_test11_args_t;

void* pthread_test11(void* arg_ptr) {
    phtread_test11_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(pthread_cond_signal(args->test_cond) == 0);
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
    return 0;
}

typedef struct phtread_test12_args {
    int32_t total_thread_no;
    int32_t thread_no;
} phtread_test12_args_t;

typedef struct phtread_test12_data {
    pthread_once_t test_onces[100];
    int32_t shared_memory[100];
    int32_t shared_counter;
} pthread_test12_data_t;

pthread_test12_data_t test12_data = {0};

void pthread_test12_once() {
    int32_t count = test12_data.shared_counter;
    lwt_yield();
    atest(count == test12_data.shared_counter);
    lwt_yield();
    atest(test12_data.shared_memory[count] == 0);
    test12_data.shared_memory[count] = test12_data.shared_counter;
    lwt_yield();
    atest(count == test12_data.shared_counter);
}

void* pthread_test12(void* arg_ptr) {
    phtread_test12_args_t* args = arg_ptr;
    for (;;) {
        int32_t count = test12_data.shared_counter;
        if (count == LENGTHOF(test12_data.test_onces))
            break;
        atest(pthread_once(&test12_data.test_onces[count], pthread_test12_once) == 0);
        atest(test12_data.shared_memory[count] == count);
        if ((count % args->total_thread_no) == args->thread_no) {
            // We are the incrementer of the shared counter.
            atest(test12_data.shared_counter == count);
            test12_data.shared_counter = count + 1;
            lwt_yield();
        } else {
            // Wait for the incrementer of the shared counter.
            for (size_t i = 0;; i++) {
                if (test12_data.shared_counter != count)
                    break;
                if (i == 2000) {
                    lwt_yield();
                    i = 0;
                } else {
                    (void) atomic_spin_yield(1);
                }
            }
        }
    }
    return (void*) pthread_self();
}

void pthread_test13_cleanup(void* arg_ptr) {
    atest(false);
}

void* pthread_test13(void* arg_ptr) {
    int oldcancelstate;
    pthread_cleanup_push(pthread_test13_cleanup, 0); {
        atest(pthread_cancel(pthread_self()) == 0);
        atest(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate) == 0);
        for (int i = 0; i < 16; i++) {
            rio_wait(10000);
            atest(pthread_cancel(pthread_self()) == 0);
            pthread_testcancel();
            lwt_yield();
        }
        atest(pthread_setcancelstate(oldcancelstate, 0) == 0);
    } pthread_cleanup_pop(0);
    pthread_testcancel();
    atest(false);
    return 0;
}

typedef struct phtread_test14_args {
    pthread_mutex_t* test_mutex;
    pthread_cond_t* test_cond;
} phtread_test14_args_t;

void* pthread_test14(void* arg_ptr) {
    phtread_test4_args_t* args = arg_ptr;
    atest(pthread_mutex_lock(args->test_mutex) == 0);
    atest(pthread_cond_signal(args->test_cond) == 0);
    atest(pthread_cond_wait(args->test_cond, args->test_mutex) == 0);
    atest(pthread_mutex_unlock(args->test_mutex) == 0);
    return 0;
}

void rcd_self_test_pthread() {
    fprintf(stderr, "[rcd_self_test_pthread]: pthread_create(), pthread_join() and returning pointer\n");
    {
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test0, &thread) == 0);
        void* thread_r;
        atest(pthread_join(thread, &thread_r) == 0);
        atest(thread_r == &thread);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: pthread_exit() and pthread_self()\n");
    {
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test1, &thread) == 0);
        void* thread_r;
        atest(pthread_join(thread, &thread_r) == 0);
        atest(thread_r == &thread);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: pthread_join() NULL retval\n");
    {
        pthread_t thread;
        pthread_attr_t attr;
        atest(pthread_attr_init(&attr) == 0);
        atest(pthread_create(&thread, &attr, pthread_test1, &thread) == 0);
        atest(pthread_join(thread, 0) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: basic mutex locking/unlocking\n");
    for (int32_t i = 0; i < 16; i++) {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: mutex pthread_mutex_init(), pthread_mutex_trylock(), error checking and do some errors where we expect to get certain error numbers\n");
    {
        pthread_mutex_t test_mutex;
        pthread_mutexattr_t attr;
        atest(pthread_mutexattr_init(&attr) == 0);
        atest(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0);
        atest(pthread_mutex_init(&test_mutex, &attr) == 0);
        atest(pthread_mutexattr_destroy(&attr) == 0);
        atest(pthread_mutex_lock(&test_mutex) == 0);
        atest(pthread_mutex_lock(&test_mutex) == EDEADLK);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_mutex_trylock(&test_mutex) == 0);
        atest(pthread_mutex_trylock(&test_mutex) == EBUSY);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == EPERM);
        atest(pthread_mutex_destroy(&test_mutex) == 0);
        // This is undefined behavior in a generic pthread implementation but ours should return EINVAL here.
        atest(pthread_mutex_lock(&test_mutex) == EINVAL);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: recursive locking of mutexes\n");
    {
        pthread_mutex_t test_mutex;
        pthread_mutexattr_t attr;
        atest(pthread_mutexattr_init(&attr) == 0);
        atest(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);
        atest(pthread_mutex_init(&test_mutex, &attr) == 0);
        atest(pthread_mutexattr_destroy(&attr) == 0);
        atest(pthread_mutex_lock(&test_mutex) == 0);
        atest(pthread_mutex_lock(&test_mutex) == 0);
        atest(pthread_mutex_trylock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == EPERM);
        atest(pthread_mutex_destroy(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: default mutexes really deadlocks in an uninterruptable manner like documented\n");
    for (int32_t i = 0; i < 2; i++) {
        static pthread_mutex_t test_mutex_static = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t test_mutex_stack;
        pthread_mutex_t* test_mutex;
        // Both these branches should behave in the exact same way.
        if (i == 0) {
            test_mutex = &test_mutex_static;
        } else {
            test_mutex = &test_mutex_stack;
            pthread_mutexattr_t attr;
            atest(pthread_mutexattr_init(&attr) == 0);
            atest(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT) == 0);
            atest(pthread_mutex_init(test_mutex, &attr) == 0);
            atest(pthread_mutexattr_destroy(&attr) == 0);
        }
        atest(pthread_mutex_lock(test_mutex) == 0);
        phtread_test2_args_t test2_args = {
            .test_mutex = test_mutex,
        };
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test2, &test2_args) == 0);
        void* retval;
        atest(pthread_join(thread, &retval) == 0);
        atest(retval == (void*) &test2_args);
        atest(pthread_mutex_unlock(test_mutex) == 0);
        struct timespec ts = {.tv_sec = 10000000, .tv_nsec = 0};
        atest(pthread_mutex_timedlock(test_mutex, &ts) == 0);
        atest(pthread_mutex_unlock(test_mutex) == 0);
        if (i == 0) {
            // No cleanup
        } else {
            atest(pthread_mutex_destroy(test_mutex) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: basic condition/concurrency\n");
    for (int32_t i = 0; i < 16; i++) {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
        int32_t shared_memory = 0;
        atest(pthread_cond_wait(&test_cond, &test_mutex) == EPERM);
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test3_args_t test3_args = {
            .test_mutex = &test_mutex,
            .test_cond = &test_cond,
            .shared_memory = &shared_memory,
        };
        pthread_t thread;
        pthread_attr_t attr;
        atest(pthread_attr_init(&attr) == 0);
        atest(pthread_attr_setdetachstate(&attr, 1) == 0);
        atest(pthread_create(&thread, &attr, pthread_test3, &test3_args) == 0);
        if ((i % 2) == 0)
            lwt_yield();
        atest(pthread_cond_wait(&test_cond, &test_mutex) == 0);
        atest(shared_memory == 42);
        if ((i % 2) == 0)
            lwt_yield();
        shared_memory = 43;
        atest(pthread_cond_broadcast(&test_cond) == 0);
        atest(pthread_cond_wait(&test_cond, &test_mutex) == 0);
        atest(shared_memory == 44);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: joining with still alive detached thread returns EINVAL or ESRCH (implementation defined UB)\n");
    for (int32_t i = 0; i < 16; i++) {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test4_args_t test4_args = {
            .test_mutex = &test_mutex,
            .test_cond = &test_cond,
        };
        pthread_t thread;
        pthread_attr_t attr;
        atest(pthread_attr_init(&attr) == 0);
        atest(pthread_attr_setdetachstate(&attr, 1) == 0);
        atest(pthread_create(&thread, &attr, pthread_test4, &test4_args) == 0);
        int32_t pthread_join_r = pthread_join(thread, 0);
        atest(pthread_join_r == EINVAL || pthread_join_r == ESRCH);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: pthread_once(); canceling of once routine\n");
    {
        atest(pthread_test5_shared_memory == 0);
        static pthread_once_t test_once1 = PTHREAD_ONCE_INIT;
        for (int32_t i = 0; i < 16; i++) {
            atest(pthread_once(&test_once1, pthread_test5_once0) == 0);
            atest(pthread_test5_shared_memory == 1);
        }
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test5, 0) == 0);
        void* thread_r;
        atest(pthread_join(thread, &thread_r) == 0);
        atest(thread_r == PTHREAD_CANCELED);
        static pthread_once_t test_once2 = PTHREAD_ONCE_INIT;
        for (int32_t i = 0; i < 16; i++) {
            atest(pthread_once(&test_once2, pthread_test5_once1) == 0);
            atest(pthread_test5_shared_memory == 3);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: pthread cancellation of conditional wait; pthread_mutex_lock() is not a cancellation point; cancellation safety; pthread_cond_init(); pthread_cond_init(); pthread_condattr_init() and pthread_cond_destroy()\n");
    for (int32_t i = 0; i < 16; i++) {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_cond_t test_cond_static = PTHREAD_COND_INITIALIZER;
        pthread_cond_t test_cond_stack;
        static pthread_cond_t* test_cond;
        if ((i % 2) == 0) {
            test_cond = &test_cond_static;
        } else {
            test_cond = &test_cond_stack;
            if ((i % 4) == 0) {
                pthread_condattr_t attr;
                atest(pthread_condattr_init(&attr) == 0);
                atest(pthread_cond_init(test_cond, &attr) == 0);
                atest(pthread_condattr_destroy(&attr) == 0);
            } else {
                atest(pthread_cond_init(test_cond, 0) == 0);
            }
        }
        int32_t shared_memory = 0;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test6_args_t test6_args = {
            .test_mutex = &test_mutex,
            .test_cond = test_cond,
            .shared_memory = &shared_memory,
        };
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test6, &test6_args) == 0);
        atest(shared_memory == 0);
        shared_memory = 1;
        atest(pthread_cond_wait(test_cond, &test_mutex) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        atest(pthread_cancel(thread) == 0);
        void* retval;
        atest(pthread_join(thread, &retval) == 0);
        atest(retval == PTHREAD_CANCELED);
        atest(shared_memory == 4);
        if ((i % 2) == 0) {
            // No cleanup.
        } else {
            atest(pthread_cond_destroy(test_cond) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: timed conditional wait\n");
    {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        {
            uint128_t start_time = rio_get_time_timer();
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000};
            atest(pthread_cond_timedwait(&test_cond, &test_mutex, &ts) == ETIMEDOUT);
            uint128_t end_time = rio_get_time_timer();
            atest(end_time >= (start_time + 100000));
        }
        phtread_test7_args_t test7_args = {
            .test_mutex = &test_mutex,
            .test_cond = &test_cond,
        };
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test7, &test7_args) == 0);
        {
            struct timespec ts = {.tv_sec = 100000, .tv_nsec = 0};
            atest(pthread_cond_timedwait(&test_cond, &test_mutex, &ts) == 0);
        }
        atest(pthread_mutex_unlock(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: basic rwlock\n");
    for (int32_t i = 0; i < 16; i++) {
        static pthread_rwlock_t* test_rwlock;
        static pthread_rwlock_t test_rwlock_static = PTHREAD_RWLOCK_INITIALIZER;
        pthread_rwlock_t test_rwlock_stack;
        if ((i % 2) == 0) {
            test_rwlock = &test_rwlock_static;
        } else {
            test_rwlock = &test_rwlock_stack;
            if ((i % 4) == 0) {
                pthread_rwlockattr_t attr;
                atest(pthread_rwlockattr_init(&attr) == 0);
                atest(pthread_rwlock_init(test_rwlock, &attr) == 0);
            } else {
                atest(pthread_rwlock_init(test_rwlock, 0) == 0);
            }
        }{
            // Session A (basic rdlocking)
            atest(pthread_rwlock_rdlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session B (basic try rdlocking)
            atest(pthread_rwlock_tryrdlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            /// Session C (recursive rdlocking with concurrent wrlocking)
            atest(pthread_rwlock_rdlock(test_rwlock) == 0);
            atest(pthread_rwlock_tryrdlock(test_rwlock) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session D (basic wrlocking)
            atest(pthread_rwlock_wrlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session E (basic try wrlocking)
            atest(pthread_rwlock_trywrlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session F (try wrlocking with concurrent locking)
            atest(pthread_rwlock_trywrlock(test_rwlock) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session G (wrlocking with concurrent locking)
            atest(pthread_rwlock_wrlock(test_rwlock) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session H (recursive timed rdlocking with concurrent locking)
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_rwlock_timedrdlock(test_rwlock, &ts) == 0);
            atest(pthread_rwlock_timedrdlock(test_rwlock, &ts) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_trywrlock(test_rwlock) == EDEADLK);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }{
            // Session I (timed wrlocking with concurrent locking)
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_rwlock_timedwrlock(test_rwlock, &ts) == 0);
            struct timespec ts2 = {.tv_sec = 0, .tv_nsec = 10000};
            atest(pthread_rwlock_timedwrlock(test_rwlock, &ts2) == ETIMEDOUT);
            atest(pthread_rwlock_unlock(test_rwlock) == 0);
            atest(pthread_rwlock_unlock(test_rwlock) == EPERM);
        }
        if ((i % 2) == 0) {
            // No cleanup.
        } else {
            atest(pthread_rwlock_destroy(test_rwlock) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: start 100 threads that accesses a shared counter protected by a mutex and count to 10000 by having each iterate it 100 times\n");
    {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        int32_t shared_memory = 0;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test8_args_t test8_args = {
            .test_mutex = &test_mutex,
            .shared_memory = &shared_memory,
        };
        pthread_t threads[100];
        for (int i = 0; i < LENGTHOF(threads); i++) {
            atest(pthread_create(&threads[i], 0, pthread_test8, &test8_args) == 0);
        }
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        for (int i = 0; i < LENGTHOF(threads); i++) {
            void* retval;
            atest(pthread_join(threads[i], &retval) == 0);
            atest(((uint64_t) retval) == threads[i]);
        }
        atest(shared_memory == 10000);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: start 100 threads that accesses a shared counter concurrently with conditions and count to 10000\n");
    {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_cond_t test_cond0 = PTHREAD_COND_INITIALIZER;
        static pthread_cond_t test_cond1 = PTHREAD_COND_INITIALIZER;
        int32_t shared_memory = 0;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test9_args_t test9_args = {
            .test_mutex = &test_mutex,
            .test_cond0 = &test_cond0,
            .test_cond1 = &test_cond1,
            .shared_memory = &shared_memory,
        };
        pthread_t threads[100];
        for (int i = 0; i < LENGTHOF(threads); i++) {
            atest(pthread_create(&threads[i], 0, pthread_test9, &test9_args) == 0);
            atest(pthread_cond_wait(&test_cond1, &test_mutex) == 0);
        }
        for (int i = 0; i < 10000; i++) {
            atest(pthread_cond_signal(&test_cond0) == 0);
            atest(pthread_cond_wait(&test_cond1, &test_mutex) == 0);
            atest(shared_memory == i + 1);
        }
        atest(pthread_mutex_unlock(&test_mutex) == 0);
        for (int i = 0; i < LENGTHOF(threads); i++)
            atest(pthread_cancel(threads[i]) == 0);
        for (int i = 0; i < LENGTHOF(threads); i++) {
            void* retval;
            atest(pthread_join(threads[i], &retval) == 0);
            atest(retval == PTHREAD_CANCELED);
        }
        atest(shared_memory == 10000);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: start 100 threads that accesses a shared counter protected by a rwlock and count to 300 by having each iterate it 3 times and concurrently read lock it a limited but much larger number of times\n");
    {
        static pthread_rwlock_t test_rwlock = PTHREAD_MUTEX_INITIALIZER;
        int32_t shared_memory = 0;
        atest(pthread_rwlock_wrlock(&test_rwlock) == 0);
        phtread_test10_args_t test10_args = {
            .test_rwlock = &test_rwlock,
            .shared_memory = &shared_memory,
        };
        pthread_t threads[100];
        for (int i = 0; i < LENGTHOF(threads); i++) {
            atest(pthread_create(&threads[i], 0, pthread_test10, &test10_args) == 0);
        }
        atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        for (int i = 0; i < LENGTHOF(threads); i++) {
            void* retval;
            atest(pthread_join(threads[i], &retval) == 0);
            atest(((uint64_t) retval) == threads[i]);
        }
        atest(shared_memory == 300);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: mutex static initialization\n");
    for (int i = 0; i < 2; i++) {
        {
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            atest(pthread_mutex_lock(&test_mutex) == 0);
            atest(pthread_mutex_unlock(&test_mutex) == 0);
        }{
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            atest(pthread_mutex_unlock(&test_mutex) == EPERM);
        }{
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            atest(pthread_mutex_trylock(&test_mutex) == 0);
            atest(pthread_mutex_unlock(&test_mutex) == 0);
        }{
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_mutex_timedlock(&test_mutex, &ts) == 0);
            atest(pthread_mutex_unlock(&test_mutex) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: cond static initialization\n");
    for (int i = 0; i < 2; i++) {
        {
            pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
            atest(pthread_cond_signal(&test_cond) == 0);
        }{
            pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
            atest(pthread_cond_broadcast(&test_cond) == 0);
        }{
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
            atest(pthread_mutex_lock(&test_mutex) == 0);
            phtread_test11_args_t test11_args = {
                .test_mutex = &test_mutex,
                .test_cond = &test_cond
            };
            pthread_t thread;
            atest(pthread_create(&thread, 0, pthread_test11, &test11_args) == 0);
            atest(pthread_cond_wait(&test_cond, &test_mutex) == 0);
            atest(pthread_mutex_unlock(&test_mutex) == 0);
        }{
            pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
            atest(pthread_mutex_lock(&test_mutex) == 0);
            phtread_test11_args_t test11_args = {
                .test_mutex = &test_mutex,
                .test_cond = &test_cond
            };
            pthread_t thread;
            atest(pthread_create(&thread, 0, pthread_test11, &test11_args) == 0);
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_cond_timedwait(&test_cond, &test_mutex, &ts) == 0);
            atest(pthread_mutex_unlock(&test_mutex) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: rwlock static initialization\n");
    for (int i = 0; i < 2; i++) {
        {
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            atest(pthread_rwlock_unlock(&test_rwlock) == EPERM);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            atest(pthread_rwlock_rdlock(&test_rwlock) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            atest(pthread_rwlock_tryrdlock(&test_rwlock) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            atest(pthread_rwlock_wrlock(&test_rwlock) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            atest(pthread_rwlock_trywrlock(&test_rwlock) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_rwlock_timedrdlock(&test_rwlock, &ts) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }{
            pthread_rwlock_t test_rwlock = PTHREAD_RWLOCK_INITIALIZER;
            struct timespec ts = {.tv_sec = 1000000, .tv_nsec = 10};
            atest(pthread_rwlock_timedwrlock(&test_rwlock, &ts) == 0);
            atest(pthread_rwlock_unlock(&test_rwlock) == 0);
        }
    }
    fprintf(stderr, "[rcd_self_test_pthread]: stress: start 100 threads that attempts to do contended pthread_once initialization 1000 times\n");
    {
        pthread_t threads[100];
        phtread_test12_args_t args[LENGTHOF(threads)];
        for (int i = 0; i < LENGTHOF(threads); i++) {
            args[i].thread_no = i;
            args[i].total_thread_no = LENGTHOF(threads);
            atest(pthread_create(&threads[i], 0, pthread_test12, &args[i]) == 0);
        }
        for (int i = 0; i < LENGTHOF(threads); i++) {
            void* retval;
            atest(pthread_join(threads[i], &retval) == 0);
            atest(((uint64_t) retval) == threads[i]);
        }
        for (int32_t i = 0; i < LENGTHOF(test12_data.test_onces); i++)
            atest(test12_data.shared_memory[i] == i);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: test disabled cancellation state\n");
    {
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test13, 0) == 0);
        void* thread_r;
        atest(pthread_join(thread, &thread_r) == 0);
        atest(thread_r == PTHREAD_CANCELED);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: test pthread_detach()\n");
    {
        static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;
        atest(pthread_mutex_lock(&test_mutex) == 0);
        phtread_test14_args_t test14_args = {
            .test_mutex = &test_mutex,
            .test_cond = &test_cond,
        };
        pthread_t thread;
        atest(pthread_create(&thread, 0, pthread_test14, &test14_args) == 0);
        atest(pthread_cond_wait(&test_cond, &test_mutex) == 0);
        atest(pthread_detach(thread) == 0);
        // The following is implementation defined UB, the only way to test the side effect of pthread_detach().
        void* thread_r;
        int32_t pthread_join_r = pthread_join(thread, &thread_r);
        atest(pthread_join_r == EINVAL || pthread_join_r == ESRCH);
        atest(pthread_cond_signal(&test_cond) == 0);
        atest(pthread_mutex_unlock(&test_mutex) == 0);
    }
    fprintf(stderr, "[rcd_self_test_pthread]: success\n");
}
