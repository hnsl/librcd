/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/* Legacy pthread synchronization primitive support. */

#include "rcd.h"
#include "atomic.h"
#include "musl.h"
#include "asmlib.h"
#include "vm-internal.h"

#pragma librcd

#define PTHREAD_ATTR_MASK_MUTEX_TYPE      (0x00000006)
#define PTHREAD_ATTR_MASK_DETACH_STATE    (0x00000008)

#define TZ(x) __builtin_ctz(x)

typedef void* (*pthread_start_routine_t)(void*);

typedef struct pthread_mutex_state {
    /// Fiber that is locking the mutex.
    rcd_fid_t locked_fiber;
    /// Number of locks the locking fiber has aquired.
    size_t lock_count;
    /// Type of mutex.
    int32_t mutex_type;
} pthread_mutex_state_t;

typedef struct pthread_cond_state {
    // The ifc fiber map of event fibers that wait to be conditionally signaled.
    sf(fmap)* cond_event_map;
} pthread_cond_state_t;

typedef struct pthread_rwlock_state {
    /// Fiber that is write locking the rwlock or zero if no write lock is held. A non-zero value of this field is mutually exclusive with a read locks count greater than zero.
    rcd_fid_t write_locking_fiber;
    /// A dict of fiber ids mapped to the number of read locks they hold. This count must be greater than zero or the element shall be removed from the dict.
    dict(size_t)* read_locking_fibers;
} pthread_rwlock_state_t;

typedef enum {
    pthread_recursive_lock_status_successful,
    pthread_recursive_lock_status_not_available,
    pthread_recursive_lock_status_deadlock,
} pthread_lock_recursive_status_t;

static void pthread_fiber_map_init(void* arg_ptr) {
    fid(fmap)* fiber_map_fid = arg_ptr;
    global_heap {
        *fiber_map_fid = fmap_sf2id(ifc_fiber_map_create());
    }
}

static fid(fmap) pthread_get_attached_fiber_map_fid() {
    static lwt_once_t once = LWT_ONCE_INIT;
    static fid(fmap) fiber_map_fid;
    lwt_once(&once, pthread_fiber_map_init, &fiber_map_fid);
    return fiber_map_fid;
}

join_locked(void*) pthread_fiber_join(join_server_params, void* retval) {
    return retval;
}

join_locked(void) pthread_fiber_attach(join_server_params) {
    fid(fmap) fm_fid = pthread_get_attached_fiber_map_fid();
    ifc_fiber_map_insert(fm_fid, FSTR_PACK(server_fiber_id), server_fiber_id, false);
}

fiber_main pthread_fiber_main(fiber_main_attr, pthread_start_routine_t start_routine, void* restrict arg, bool attached) {
    void* retval;
    if (attached) {
        // Attach the fiber to the attached fiber map.
        accept_join(pthread_fiber_attach, join_server_params);
    }
    try {
        retval = start_routine(arg);
    } catch (exception_canceled, e) {
        retval = PTHREAD_CANCELED;
    }
    try {
        pthread_exit(retval);
    } catch (exception_canceled, e) {}
}

int32_t pthread_create(pthread_t* restrict thread, const pthread_attr_t* restrict attr, pthread_start_routine_t start_routine, void* restrict arg) {
    bool attached = (attr != 0)? ((attr->value & PTHREAD_ATTR_MASK_DETACH_STATE) == 0): true;
    rcd_fid_t fid;
    fmitosis {
        fid = spawn_static_fiber(pthread_fiber_main("", start_routine, arg, attached));
    }
    *thread = fid;
    if (attached) {
        // Attach the fiber to the attached fiber map.
        pthread_fiber_attach(fid);
    }
    return 0;
}

int32_t pthread_detach(pthread_t thread) {
    fid(fmap) fm_fid = pthread_get_attached_fiber_map_fid();
    ifc_fiber_map_remove(fm_fid, FSTR_PACK(thread));
    return 0;
}

void pthread_exit(void* retval) {
    rcd_fid_t self_fid = lwt_get_fiber_id();
    fid(fmap) fm_fid = pthread_get_attached_fiber_map_fid();
    rcd_fid_t fid = ifc_fiber_map_read(fm_fid, FSTR_PACK(self_fid));
    if (fid != 0) {
        // We are attached, join with return value.
        accept_join(pthread_fiber_join, join_server_params, retval);
    }
    throw("pthread_exit() canceling pthread", exception_canceled);
}

int32_t pthread_join(pthread_t thread, void** retval) {
    fid(fmap) fm_fid = pthread_get_attached_fiber_map_fid();
    rcd_fid_t fid = ifc_fiber_map_read(fm_fid, FSTR_PACK(thread));
    int32_t r_code;
    lwt_cancellation_point();
    if (fid != 0) {
        try {
            void* join_retval = pthread_fiber_join(thread);
            if (retval != 0)
                *retval = join_retval;
            r_code = 0;
        } catch (exception_inner_join_fail, e) {
            // "No thread with the ID thread could be found."
            r_code = ESRCH;
        }
    } else {
        // "No thread with the ID thread could be found."
        r_code = ESRCH;
    }
    return r_code;
}

pthread_t pthread_self() {
    return lwt_get_fiber_id();
}

int32_t pthread_setcancelstate(int32_t state, int32_t* oldstate) {
    bool oldstate_b;
    if (state == PTHREAD_CANCEL_ENABLE) {
        __lwt_fiber_set_uninterruptible(false, &oldstate_b);
    } else if (state == PTHREAD_CANCEL_DISABLE) {
        __lwt_fiber_set_uninterruptible(true, &oldstate_b);
    } else {
        return EINVAL;
    }
    if (oldstate != 0)
        *oldstate = (oldstate_b? PTHREAD_CANCEL_DISABLE: PTHREAD_CANCEL_ENABLE);
    return 0;
}

int32_t pthread_setcanceltype(int32_t type, int32_t* oldtype) {
    if (type == PTHREAD_CANCEL_DEFERRED) {
        *oldtype = PTHREAD_CANCEL_DEFERRED;
        return 0;
    } else {
        // No other cancellation type is currently supported.
        // This feature is rarely used and not important anyway.
        return EINVAL;
    }
}

void pthread_testcancel() {
    lwt_cancellation_point();
}

int32_t pthread_cancel(pthread_t thread) {
    lwt_cancel_fiber_id(thread);
    return 0;
}


int32_t pthread_getschedparam(pthread_t thread, int32_t* restrict policy, struct sched_param* restrict param) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_setschedparam(pthread_t thread, int32_t policy, const struct sched_param* param) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_setschedprio(pthread_t thread, int32_t prio) {
    // Not implemented/applicable.
    return ENOTSUP;
}

void phtread_once_callback(void* arg_ptr) {
    void (*init_routine)() = arg_ptr;
    init_routine();
}

int32_t pthread_once(pthread_once_t* once_ctrl, void (*init_routine)()) {
    lwt_once(once_ctrl, phtread_once_callback, init_routine);
    return 0;
}

/// Returns true if the mutex is already locked by the calling thread .
join_locked(pthread_lock_recursive_status_t) pthread_mutex_fiber_lock_recursive(rcd_fid_t fiber_id, bool try_lock, join_server_params, pthread_mutex_state_t* state) {
    if (state->lock_count > 0 && state->locked_fiber == fiber_id) {
        if (state->mutex_type == PTHREAD_MUTEX_RECURSIVE) {
            state->lock_count++;
            return pthread_recursive_lock_status_successful;
        } else {
            if (!try_lock && state->mutex_type != PTHREAD_MUTEX_ERRORCHECK) {
                // Never allow the thread to return as specified in the pthread_mutex_lock definition.
                ifc_park();
            }
            return pthread_recursive_lock_status_deadlock;
        }
    } else {
        return pthread_recursive_lock_status_not_available;
    }
}

/// Returns when the mutex was successfully locked.
join_locked(int32_t) pthread_mutex_fiber_lock_exclusive(rcd_fid_t fiber_id, bool* aquire_ok, join_server_params, pthread_mutex_state_t* state) {
    assert(state->lock_count == 0);
    state->lock_count = 1;
    state->locked_fiber = fiber_id;
    if (aquire_ok != 0)
        *aquire_ok = true;
    return 0;
}

join_locked(int32_t) pthread_mutex_fiber_unlock(rcd_fid_t fiber_id, join_server_params, pthread_mutex_state_t* state) {
    if (state->lock_count > 0 && state->locked_fiber == fiber_id) {
        state->lock_count--;
        return 0;
    } else {
        /* The current thread does not own the mutex. */
        return EPERM;
    }
}

join_locked(int32_t) pthread_mutex_fiber_trylock(rcd_fid_t fiber_id, join_server_params, pthread_mutex_state_t* state) {
    if (state->lock_count == 0) {
        state->lock_count = 1;
        state->locked_fiber = fiber_id;
        return 0;
    } else {
        // "The mutex could not be acquired because it was already locked."
        return EBUSY;
    }
}

join_locked(bool) pthread_mutex_fiber_lock_test(rcd_fid_t fiber_id, join_server_params, pthread_mutex_state_t* state) {
    return (state->lock_count > 0 && state->locked_fiber == fiber_id);
}

fiber_main pthread_mutex_fiber(fiber_main_attr, int32_t mutex_type) {
    try {
        pthread_mutex_state_t state = {
            .locked_fiber = 0,
            .lock_count = 0,
            .mutex_type = mutex_type,
        };
        for (;;) {
            if (state.lock_count > 0) {
                accept_join(pthread_mutex_fiber_lock_recursive, pthread_mutex_fiber_unlock, pthread_mutex_fiber_trylock, pthread_mutex_fiber_lock_test, join_server_params, &state);
            } else {
                accept_join(pthread_mutex_fiber_lock_recursive, pthread_mutex_fiber_unlock, pthread_mutex_fiber_trylock, pthread_mutex_fiber_lock_test, pthread_mutex_fiber_lock_exclusive, join_server_params, &state);
            }
        }
    } catch (exception_canceled, e) {}
}

static void pthread_mutex_init_raw(pthread_mutex_t* restrict mutex, int32_t mutex_type) {
    fmitosis {
        mutex->mutex_fid = spawn_static_fiber(pthread_mutex_fiber("", mutex_type));
    }
}

int32_t pthread_mutex_init(pthread_mutex_t* restrict mutex, const pthread_mutexattr_t* restrict attr) {
    int32_t mutex_type = PTHREAD_MUTEX_NORMAL;
    if (attr != 0) {
        int32_t mutexattr_gettype_r = pthread_mutexattr_gettype(attr, &mutex_type);
        if (mutexattr_gettype_r != 0) {
            // "The value specified by attr is invalid."
            return EINVAL;
        }
    }
    pthread_mutex_init_raw(mutex, mutex_type);
    mutex->once_init = LWT_ONCE_INIT_TRIPPED;
    return 0;
}

static void pthread_mutex_once_init_now(void* arg_ptr) {
    pthread_mutex_t* mutex = arg_ptr;
    pthread_mutex_init_raw(mutex, PTHREAD_MUTEX_NORMAL);
}

static void pthread_mutex_once_init(pthread_mutex_t* restrict mutex) {
    lwt_once(&mutex->once_init, pthread_mutex_once_init_now, mutex);
}

int32_t pthread_mutex_lock(pthread_mutex_t* mutex) {
    pthread_mutex_once_init(mutex);
    int32_t r_code;
    sub_heap {
        // pthread_mutex_lock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                switch (pthread_mutex_fiber_lock_recursive(lwt_get_fiber_id(), false, mutex->mutex_fid)) {{
                } case pthread_recursive_lock_status_successful: {
                    r_code = 0;
                    break;
                } case pthread_recursive_lock_status_not_available: {
                    r_code = pthread_mutex_fiber_lock_exclusive(lwt_get_fiber_id(), 0, mutex->mutex_fid);
                    break;
                } case pthread_recursive_lock_status_deadlock: {
                    // "The current thread already owns the mutex."
                    r_code = EDEADLK;
                    break;
                }}
            } catch (exception_no_such_fiber, e) {
                // "The value specified by mutex does not refer to an initialized mutex object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_mutex_unlock(pthread_mutex_t* mutex) {
    pthread_mutex_once_init(mutex);
    int32_t r_code;
    sub_heap {
        // pthread_mutex_unlock is not allowed to be a cancellation point
        uninterruptible {
            try {
                r_code = pthread_mutex_fiber_unlock(lwt_get_fiber_id(), mutex->mutex_fid);
            } catch (exception_no_such_fiber, e) {
                // "The value specified by mutex does not refer to an initialized mutex object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_mutex_trylock(pthread_mutex_t* mutex) {
    pthread_mutex_once_init(mutex);
    int32_t r_code;
    sub_heap {
        // pthread_mutex_trylock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                switch (pthread_mutex_fiber_lock_recursive(lwt_get_fiber_id(), true, mutex->mutex_fid)) {{
                } case pthread_recursive_lock_status_successful: {
                    r_code = 0;
                    break;
                } case pthread_recursive_lock_status_not_available: {
                    r_code = pthread_mutex_fiber_trylock(lwt_get_fiber_id(), mutex->mutex_fid);
                    break;
                } case pthread_recursive_lock_status_deadlock: {
                    // "The mutex could not be acquired because it was already locked."
                    r_code = EBUSY;
                    break;
                }}
            } catch (exception_no_such_fiber, e) {
                // "The value specified by mutex does not refer to an initialized mutex object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

fiber_main pthread_mutex_timedlock_fiber(fiber_main_attr, rcd_fid_t fiber_id, bool* aquire_ok, pthread_mutex_t* restrict mutex, const struct timespec* restrict abs_timeout) {
    try {
        ifc_cancel_alarm_arm(abs_timeout->tv_nsec + abs_timeout->tv_sec * RIO_NS_SEC);
        pthread_mutex_fiber_lock_exclusive(fiber_id, aquire_ok, mutex->mutex_fid);
    } catch (exception_canceled | exception_inner_join_fail, e) {}
}

int32_t pthread_mutex_timedlock(pthread_mutex_t* restrict mutex, const struct timespec* restrict abs_timeout) {
    pthread_mutex_once_init(mutex);
    int32_t r_code;
    sub_heap {
        // pthread_mutex_timedlock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                switch (pthread_mutex_fiber_lock_recursive(lwt_get_fiber_id(), true, mutex->mutex_fid)) {{
                } case pthread_recursive_lock_status_successful: {
                    r_code = 0;
                    break;
                } case pthread_recursive_lock_status_not_available: {
                    bool aquire_ok = false;
                    rcd_sub_fiber_t* pthread_mutex_timedlock_sf;
                    fmitosis {
                        pthread_mutex_timedlock_sf = spawn_fiber(pthread_mutex_timedlock_fiber("", lwt_get_fiber_id(), &aquire_ok, mutex, abs_timeout));
                    }
                    // We must uninterruptedly wait here for two separate reasons:
                    // 1) pthread_mutex_timedlock() is not allowed to be a cancellation point
                    // 2) the sub fiber has a reference to this stack frame (bool aquire_ok)
                    ifc_wait(sfid(pthread_mutex_timedlock_sf));
                    // Now check if we have acquired the lock or if we timed out.
                    if (aquire_ok) {
                        r_code = 0;
                    } else {
                        // "The mutex could not be locked before the specified timeout expired."
                        r_code = ETIMEDOUT;
                    }
                    break;
                } case pthread_recursive_lock_status_deadlock: {
                    // We are deadlocked, instead of parking forever we just uninterruptibly wait until we timeout like documented.
                    rio_wait(abs_timeout->tv_nsec + abs_timeout->tv_sec * RIO_NS_SEC);
                    // "The mutex could not be locked before the specified timeout expired."
                    r_code = ETIMEDOUT;
                    break;
                }}
            } catch (exception_no_such_fiber, e) {
                // "The value specified by mutex does not refer to an initialized mutex object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_mutex_destroy(pthread_mutex_t* mutex) {
    lwt_cancel_fiber_id(mutex->mutex_fid);
    return 0;
}

join_locked(rcd_fid_t) pthread_cond_fiber_register(rcd_fid_t event_fid, fstr_mem_t** out_key, join_server_params, pthread_cond_state_t* state) {
    return ifc_fiber_map_enqueue(fmap_sf2id(state->cond_event_map), out_key, event_fid, true);
}

join_locked(void) pthread_cond_fiber_unregister(rcd_fid_t event_monitor_fid, fstr_t key, join_server_params, pthread_cond_state_t* state) {
    ifc_fiber_map_cond_remove(fmap_sf2id(state->cond_event_map), event_monitor_fid, key);
}

join_locked(void) pthread_cond_fiber_signal(join_server_params, pthread_cond_state_t* state) {
    rcd_fid_t event_fid = ifc_fiber_map_dequeue(fmap_sf2id(state->cond_event_map), 0);
    if (event_fid != 0)
        lwt_cancel_fiber_id(event_fid);
}

join_locked(int32_t) pthread_cond_fiber_broadcast(join_server_params, pthread_cond_state_t* state) { server_heap_flip {
    lwt_alloc_free(state->cond_event_map);
    state->cond_event_map = ifc_fiber_map_create();
    return 0;
}}

fiber_main pthread_cond_fiber(fiber_main_attr) { try {
    pthread_cond_state_t state = {
        .cond_event_map = ifc_fiber_map_create()
    };
    auto_accept_join(pthread_cond_fiber_register, pthread_cond_fiber_unregister, pthread_cond_fiber_signal, pthread_cond_fiber_broadcast, join_server_params, &state);
} catch (exception_canceled, e); }

/// A fiber that is triggered by a conditional wait by being canceled or timed out.
fiber_main pthread_cond_event_fiber(fiber_main_attr, const struct timespec* restrict abstime, bool* timed_out) {
    try {
        if (abstime == 0) {
            ifc_park();
        } else {
            rio_wait(abstime->tv_sec * RIO_NS_SEC + abstime->tv_nsec);
            *timed_out = true;
        }
    } catch (exception_inner_join_fail | exception_canceled, e) {}
}

static void pthread_cond_init_raw(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr) {
    fmitosis {
        cond->cond_fid = spawn_static_fiber(pthread_cond_fiber(""));
    }
}

int32_t pthread_cond_init(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr) {
    pthread_cond_init_raw(cond, attr);
    cond->once_init = LWT_ONCE_INIT_TRIPPED;
    return 0;
}

static void pthread_cond_once_init_now(void* arg_ptr) {
    pthread_cond_t* cond = arg_ptr;
    pthread_cond_init_raw(cond, 0);
}

static void pthread_cond_once_init(pthread_cond_t* restrict cond) {
    lwt_once(&cond->once_init, pthread_cond_once_init_now, cond);
}

int32_t pthread_cond_destroy(pthread_cond_t* cond) {
    lwt_cancel_fiber_id(cond->cond_fid);
    return 0;
}

int32_t pthread_cond_wait(pthread_cond_t* restrict cond, pthread_mutex_t* restrict mutex) {
    return pthread_cond_timedwait(cond, mutex, 0);
}

int32_t pthread_cond_timedwait(pthread_cond_t* restrict cond, pthread_mutex_t* restrict mutex, const struct timespec* restrict abstime) {
    pthread_cond_once_init(cond);
    pthread_mutex_once_init(mutex);
    int32_t r_code;
    sub_heap {
        try {
            if (!pthread_mutex_fiber_lock_test(lwt_get_fiber_id(), mutex->mutex_fid)) {
                // "The mutex was not owned by the current thread at the time of the call."
                r_code = EPERM;
            } else {
                // This is a cancellation point so testing here to save some cycles.
                lwt_cancellation_point();
                // Create a pthread cond event sub fiber and register it.
                bool timed_out = false;
                rcd_fid_t cond_event_fid;
                fmitosis {
                    cond_event_fid = sfid(spawn_fiber(pthread_cond_event_fiber("", abstime, &timed_out)));
                }
                // Register the condition event, unlock the mutex, wait for the sub fiber and lock it again.
                rcd_fid_t cond_event_monitor_fid;
                fstr_mem_t* cond_event_key;
                try {
                    cond_event_monitor_fid = pthread_cond_fiber_register(cond_event_fid, &cond_event_key, cond->cond_fid);
                    pthread_mutex_fiber_unlock(lwt_get_fiber_id(), mutex->mutex_fid);
                    ifc_wait(cond_event_fid);
                    pthread_mutex_fiber_lock_exclusive(lwt_get_fiber_id(), 0, mutex->mutex_fid);
                } catch (exception_any, e) {
                    // We must uninterruptedly re-aquire the mutex before forwarding the exception due to the specification:
                    // "A condition wait (whether timed or not) is a cancellation point. When the cancelability enable state of a thread is set to
                    // PTHREAD_CANCEL_DEFERRED, a side effect of acting upon a cancellation request while in a condition wait is that the mutex is
                    // (in effect) re-acquired before calling the first cancellation cleanup handler. The effect is as if the thread were unblocked,
                    // allowed to execute up to the point of returning from the call to pthread_cond_timedwait() or pthread_cond_wait(), but at that
                    // point notices the cancellation request and instead of returning to the caller of pthread_cond_timedwait() or pthread_cond_wait(),
                    // starts the thread cancellation activities, which includes calling cancellation cleanup handlers."
                    uninterruptible {
                        try {
                            if (!pthread_mutex_fiber_lock_test(lwt_get_fiber_id(), mutex->mutex_fid)) {
                                // We only need to re-aquire the mutex if it was unlocked.
                                pthread_mutex_fiber_lock_exclusive(lwt_get_fiber_id(), 0, mutex->mutex_fid);
                            }
                        } catch (exception_no_such_fiber, e) {}
                        // We must uninterruptedly wait for the pthread cond event fiber to exit before
                        // we continue as it has a memory reference to this stack frame (bool timed_out).
                        lwt_cancel_fiber_id(cond_event_fid);
                        ifc_wait(cond_event_fid);
                        try {
                            // We must also make sure that the cond event is removed in a race-free manner
                            // so it does not remain indexed and possibly consumes an additional cond signal.
                            pthread_cond_fiber_unregister(cond_event_monitor_fid, fss(cond_event_key), cond->cond_fid);
                        } catch (exception_no_such_fiber, e) {}
                    }
                    lwt_throw_exception(e);
                }
                if (timed_out) {
                    // "The time specified by abstime to pthread_cond_timedwait() has passed."
                    r_code = ETIMEDOUT;
                } else {
                    r_code = 0;
                }
            }
        } catch (exception_inner_join_fail, e) {
            // "The value specified by cond, mutex, or abstime is invalid."
            r_code = EINVAL;
        }
    }
    return r_code;
}

int32_t pthread_cond_broadcast(pthread_cond_t* cond) {
    pthread_cond_once_init(cond);
    int32_t r_code;
    sub_heap {
        // pthread_cond_broadcast is not allowed to be a cancellation point.
        uninterruptible {
            try {
                pthread_cond_fiber_broadcast(cond->cond_fid);
                r_code = 0;
            } catch (exception_no_such_fiber, e) {
                // "The value cond does not refer to an initialized condition variable."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_cond_signal(pthread_cond_t* cond) {
    pthread_cond_once_init(cond);
    int32_t r_code;
    sub_heap {
        // pthread_cond_signal is not allowed to be a cancellation point.
        uninterruptible {
            try {
                pthread_cond_fiber_signal(cond->cond_fid);
                r_code = 0;
            } catch (exception_no_such_fiber, e) {
                // "The value cond does not refer to an initialized condition variable."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

join_locked(int32_t) pthread_rwlock_fiber_wrlock(rcd_fid_t fiber_id, bool* aquire_ok, join_server_params, pthread_rwlock_state_t* state) {
    assert(state->write_locking_fiber == 0 && dict_count(state->read_locking_fibers, size_t) == 0);
    state->write_locking_fiber = fiber_id;
    if (aquire_ok != 0)
        *aquire_ok = true;
    return 0;
}

join_locked(int32_t) pthread_rwlock_fiber_try_wrlock(rcd_fid_t fiber_id, join_server_params, pthread_rwlock_state_t* state) {
    int32_t r_code;
    if (state->write_locking_fiber != 0 || dict_count(state->read_locking_fibers, size_t) > 0) {
        if (state->write_locking_fiber == fiber_id || dict_read(state->read_locking_fibers, size_t, FSTR_PACK(fiber_id)) != 0) {
            // "The current thread already owns the read-write lock for writing or reading."
            r_code = EDEADLK;
        } else {
            // "The read-write lock could not be acquired for writing because it was already locked for reading or writing."
            r_code = EBUSY;
        }
    } else {
        state->write_locking_fiber = fiber_id;
        r_code = 0;
    }
    return r_code;
}

static void pthread_rwlock_fiber_increment_reader(rcd_fid_t fiber_id, pthread_rwlock_state_t* state) {
    size_t* lock_count = dict_read(state->read_locking_fibers, size_t, FSTR_PACK(fiber_id));
    if (lock_count != 0) {
        (*lock_count)++;
    } else {
        bool insert_ok = dict_insert(state->read_locking_fibers, size_t, FSTR_PACK(fiber_id), 1);
        assert(insert_ok);
    }
}

join_locked(int32_t) pthread_rwlock_fiber_rdlock(rcd_fid_t fiber_id, bool* aquire_ok, join_server_params, pthread_rwlock_state_t* state) {
    assert(state->write_locking_fiber == 0);
    server_heap_flip {
        pthread_rwlock_fiber_increment_reader(fiber_id, state);
    }
    if (aquire_ok != 0)
        *aquire_ok = true;
    return 0;
}

join_locked(int32_t) pthread_rwlock_fiber_try_rdlock(rcd_fid_t fiber_id, join_server_params, pthread_rwlock_state_t* state) {
    int32_t r_code;
    if (state->write_locking_fiber != 0) {
        if (state->write_locking_fiber == fiber_id) {
            // "The current thread already owns the read-write lock for writing."
            r_code = EDEADLK;
        } else {
            // "The read-write lock could not be acquired for reading because a writer holds the lock or a writer with the appropriate priority was blocked on it."
            r_code = EBUSY;
        }
    } else {
        server_heap_flip {
            pthread_rwlock_fiber_increment_reader(fiber_id, state);
        }
        r_code = 0;
    }
    return r_code;
}

join_locked(int32_t) pthread_rwlock_fiber_unlock(rcd_fid_t fiber_id, join_server_params, pthread_rwlock_state_t* state) {
    int32_t r_code;
    if (state->write_locking_fiber != 0) {
        if (state->write_locking_fiber == fiber_id) {
            state->write_locking_fiber = 0;
            r_code = 0;
        } else {
            // "The current thread does not hold a lock on the read-write lock."
            r_code = EPERM;
        }
    } else if (dict_count(state->read_locking_fibers, size_t) > 0) {
        server_heap_flip {
            fstr_t fiber_id_key = FSTR_PACK(fiber_id);
            size_t* lock_count = dict_read(state->read_locking_fibers, size_t, fiber_id_key);
            if (lock_count != 0) {
                (*lock_count)--;
                if (*lock_count == 0) {
                    bool delete_ok = dict_delete(state->read_locking_fibers, size_t, fiber_id_key);
                    assert(delete_ok);
                }
                r_code = 0;
            } else {
                // "The current thread does not hold a lock on the read-write lock."
                r_code = EPERM;
            }
        }
    } else {
        // "The current thread does not hold a lock on the read-write lock."
        r_code = EPERM;
    }
    return r_code;
}

fiber_main pthread_rwlock_fiber(fiber_main_attr) {
    try {
        pthread_rwlock_state_t state;
        state.write_locking_fiber = 0;
        state.read_locking_fibers = new_dict(size_t);
        for (;;) {
            if (state.write_locking_fiber != 0) {
                accept_join(pthread_rwlock_fiber_unlock, pthread_rwlock_fiber_try_wrlock, pthread_rwlock_fiber_try_rdlock, join_server_params, &state);
            } else if (dict_count(state.read_locking_fibers, size_t) > 0) {
                accept_join(pthread_rwlock_fiber_unlock, pthread_rwlock_fiber_try_wrlock, pthread_rwlock_fiber_try_rdlock, pthread_rwlock_fiber_rdlock, join_server_params, &state);
            } else {
                accept_join(pthread_rwlock_fiber_unlock, pthread_rwlock_fiber_try_wrlock, pthread_rwlock_fiber_try_rdlock, pthread_rwlock_fiber_rdlock, pthread_rwlock_fiber_wrlock, join_server_params, &state);
            }
        }
    } catch (exception_canceled, e) {}
}

static void pthread_rwlock_init_raw(pthread_rwlock_t* restrict rwlock, const pthread_rwlockattr_t* restrict attr) {
    fmitosis {
        rwlock->rwlock_fid = spawn_static_fiber(pthread_rwlock_fiber(""));
    }
}

int32_t pthread_rwlock_init(pthread_rwlock_t* restrict rwlock, const pthread_rwlockattr_t* restrict attr) {
    pthread_rwlock_init_raw(rwlock, attr);
    rwlock->once_init = LWT_ONCE_INIT_TRIPPED;
    return 0;
}

static void pthread_rwlock_once_init_now(void* arg_ptr) {
    pthread_rwlock_t* rwlock = arg_ptr;
    pthread_rwlock_init_raw(rwlock, 0);
}

static void pthread_rwlock_once_init(pthread_rwlock_t* restrict rwlock) {
    lwt_once(&rwlock->once_init, pthread_rwlock_once_init_now, rwlock);
}

int32_t pthread_rwlock_destroy(pthread_rwlock_t* rwlock) {
    lwt_cancel_fiber_id(rwlock->rwlock_fid);
    return 0;
}

int32_t pthread_rwlock_rdlock(pthread_rwlock_t* rwlock) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        // pthread_rwlock_rdlock is allowed to be a cancellation point but cannot be one because the documentation does
        // not define what effect a cancellation has on the rwlock state, causing undefined behavior.
        uninterruptible {
            try {
                r_code = pthread_rwlock_fiber_rdlock(lwt_get_fiber_id(), 0, rwlock->rwlock_fid);
            } catch (exception_no_such_fiber, e) {
                // "The value specified by rwlock does not refer to an initialized read-write lock object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        // pthread_rwlock_tryrdlock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                r_code = pthread_rwlock_fiber_try_rdlock(lwt_get_fiber_id(), rwlock->rwlock_fid);
            } catch (exception_no_such_fiber, e) {
                // "The value specified by rwlock does not refer to an initialized read-write lock object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

fiber_main pthread_rwlock_timedrdlock_fiber(fiber_main_attr, rcd_fid_t fiber_id, bool* aquire_ok, pthread_rwlock_t* restrict rwlock, const struct timespec* restrict abs_timeout) {
    try {
        ifc_cancel_alarm_arm(abs_timeout->tv_nsec + abs_timeout->tv_sec * RIO_NS_SEC);
        pthread_rwlock_fiber_rdlock(fiber_id, aquire_ok, rwlock->rwlock_fid);
    } catch (exception_canceled | exception_inner_join_fail, e) {}
}

int32_t pthread_rwlock_timedrdlock(pthread_rwlock_t* restrict rwlock, const struct timespec* restrict abs_timeout) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        bool aquire_ok = false;
        rcd_fid_t pthread_rwlock_timedrdlock_fid;
        fmitosis {
            pthread_rwlock_timedrdlock_fid = sfid(spawn_fiber(pthread_rwlock_timedrdlock_fiber("", lwt_get_fiber_id(), &aquire_ok, rwlock, abs_timeout)));
        }
        // pthread_rwlock_timedrdlock is allowed to be a cancellation point but cannot be one because the documentation does
        // not define what effect a cancellation has on the rwlock state, causing undefined behavior.
        // In addition we must wait for the sub fiber to exit since it has a memory reference to this stack frame. (bool aquire_ok)
        uninterruptible {
            ifc_wait(pthread_rwlock_timedrdlock_fid);
        }
        // We can check this variable to verify if we got the intended side effect or if the cancellation was triggered before it.
        if (aquire_ok) {
            r_code = 0;
        } else {
            // "The lock could not be acquired before the specified timeout expired."
            r_code = ETIMEDOUT;
            // aquire_ok could also be false if the condition is invalid however this is undefined behavior so we skip checking it and return ETIMEDOUT instead.
        }
    }
    return r_code;
}

int32_t pthread_rwlock_wrlock(pthread_rwlock_t* rwlock) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        // pthread_rwlock_wrlock is allowed to be a cancellation point but cannot be one because the documentation does
        // not define what effect a cancellation has on the rwlock state, causing undefined behavior.
        uninterruptible {
            try {
                r_code = pthread_rwlock_fiber_wrlock(lwt_get_fiber_id(), 0, rwlock->rwlock_fid);
            } catch (exception_no_such_fiber, e) {
                // "The value specified by rwlock does not refer to an initialized read-write lock object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

int32_t pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        // pthread_rwlock_trywrlock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                r_code = pthread_rwlock_fiber_try_wrlock(lwt_get_fiber_id(), rwlock->rwlock_fid);
            } catch (exception_inner_join_fail, e) {
                // "The value specified by rwlock does not refer to an initialized read-write lock object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

fiber_main pthread_rwlock_timedwrlock_fiber(fiber_main_attr, rcd_fid_t fiber_id, bool* aquire_ok, pthread_rwlock_t* restrict rwlock, const struct timespec* restrict abs_timeout) {
    try {
        ifc_cancel_alarm_arm(abs_timeout->tv_nsec + abs_timeout->tv_sec * RIO_NS_SEC);
        pthread_rwlock_fiber_wrlock(fiber_id, aquire_ok, rwlock->rwlock_fid);
    } catch (exception_canceled | exception_inner_join_fail, e) {}
}

int32_t pthread_rwlock_timedwrlock(pthread_rwlock_t* restrict rwlock, const struct timespec* restrict abs_timeout) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        bool aquire_ok = false;
        rcd_fid_t pthread_rwlock_timedwrlock_fid;
        fmitosis {
            pthread_rwlock_timedwrlock_fid = sfid(spawn_fiber(pthread_rwlock_timedwrlock_fiber("", lwt_get_fiber_id(), &aquire_ok, rwlock, abs_timeout)));
        }
        // pthread_rwlock_timedwrlock is allowed to be a cancellation point but cannot be one because the documentation does
        // not define what effect a cancellation has on the rwlock state, causing undefined behavior.
        // In addition we must wait for the sub fiber to exit since it has a memory reference to this stack frame. (bool aquire_ok)
        uninterruptible {
            ifc_wait(pthread_rwlock_timedwrlock_fid);
        }
        // We can check this variable to verify if we got the intended side effect or if the cancellation was triggered before it.
        if (aquire_ok) {
            r_code = 0;
        } else {
            // "The lock could not be acquired before the specified timeout expired."
            r_code = ETIMEDOUT;
            // aquire_ok could also be false if the condition is invalid however this is undefined behavior so we skip checking it and return ETIMEDOUT instead.
        }
    }
    return r_code;
}

int32_t pthread_rwlock_unlock(pthread_rwlock_t* rwlock) {
    pthread_rwlock_once_init(rwlock);
    int32_t r_code;
    sub_heap {
        // pthread_rwlock_fiber_unlock is not allowed to be a cancellation point.
        uninterruptible {
            try {
                r_code = pthread_rwlock_fiber_unlock(lwt_get_fiber_id(), rwlock->rwlock_fid);
            } catch (exception_inner_join_fail, e) {
                // "The value specified by rwlock does not refer to an initialized read-write lock object."
                r_code = EINVAL;
            }
        }
    }
    return r_code;
}

// TODO: Implement these rarely used pthread functions. Some libraries may use them but YAGNI for the moment.

/*int pthread_spin_init(pthread_spinlock_t *, int);
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
int pthread_setspecific(pthread_key_t, const void *);*/

static int32_t pthread_attr_getpshared(const int32_t* attr, int32_t* pshared) {
    // Process shared pthread objects are not supported as we use librcd scheduling and not linux scheduling.
    return ENOTSUP;
}

static int32_t pthread_attr_setpshared(int32_t* attr, int32_t pshared) {
    // Process shared pthread objects are not supported as we use librcd scheduling and not linux scheduling.
    return ENOTSUP;
}

int32_t pthread_attr_init(pthread_attr_t* attr) {
    attr->value = 0;
    return 0;
}

int32_t pthread_attr_destroy(pthread_attr_t* attr) {
    return 0;
}

int32_t pthread_attr_getguardsize(const pthread_attr_t* restrict attr, size_t* restrict guardsize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_setguardsize(pthread_attr_t* attr, size_t guardsize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_getstacksize(const pthread_attr_t* restrict attr, size_t* restrict stacksize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_getdetachstate(const pthread_attr_t* restrict attr, int32_t* detachstate) {
    *detachstate = ((attr->value & PTHREAD_ATTR_MASK_DETACH_STATE) != 0)? PTHREAD_CREATE_DETACHED: PTHREAD_CREATE_JOINABLE;
    return 0;
}

int32_t pthread_attr_setdetachstate(pthread_attr_t* attr, int32_t detachstate) {
    if (detachstate == PTHREAD_CREATE_JOINABLE) {
        attr->value = (attr->value & ~PTHREAD_ATTR_MASK_DETACH_STATE);
        return 0;
    } else if (detachstate == PTHREAD_CREATE_DETACHED) {
        attr->value = (attr->value | PTHREAD_ATTR_MASK_DETACH_STATE);
        return 0;
    } else {
        // "An invalid value was specified in detachstate."
        return EINVAL;
    }
}

int32_t pthread_attr_getstack(const pthread_attr_t* restrict attr, void** restrict stackaddr, size_t* restrict stacksize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_setstack(pthread_attr_t* attr, void* stackaddr, size_t stacksize) {
    // Not applicable. Librcd may use split/spaghetti stacks.
    return ENOTSUP;
}

int32_t pthread_attr_getscope(const pthread_attr_t* restrict attr, int32_t* restrict scope) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_setscope(pthread_attr_t* attr, int32_t scope) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_getschedpolicy(const pthread_attr_t* restrict attr, int32_t* restrict schedpolicy) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_setschedpolicy(pthread_attr_t* attr, int32_t schedpolicy) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_getschedparam(const pthread_attr_t* restrict attr, struct sched_param* restrict param) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_setschedparam(pthread_attr_t* restrict attr, const struct sched_param* restrict param) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_getinheritsched(const pthread_attr_t* restrict attr, int32_t* restrict inheritsched) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_attr_setinheritsched(pthread_attr_t* attr, int32_t inheritsched) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_mutexattr_destroy(pthread_mutexattr_t* attr) {
    return 0;
}

int32_t pthread_mutexattr_getprioceiling(const pthread_mutexattr_t* restrict attr, int32_t* restrict prioceiling) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_getprotocol(const pthread_mutexattr_t* restrict attr, int32_t* restrict protocol) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_getpshared(const pthread_mutexattr_t* restrict attr, int32_t* restrict pshared) {
    return pthread_attr_getpshared(&attr->value, pshared);
}

int32_t pthread_mutexattr_getrobust(const pthread_mutexattr_t* restrict attr, int32_t* restrict robust) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_gettype(const pthread_mutexattr_t* restrict attr, int32_t* restrict type) {
    *type = (attr->value & PTHREAD_ATTR_MASK_MUTEX_TYPE) >> TZ(PTHREAD_ATTR_MASK_MUTEX_TYPE);
    return 0;
}

int32_t pthread_mutexattr_init(pthread_mutexattr_t* attr) {
    attr->value = 0;
    return 0;
}

int32_t pthread_mutexattr_setprioceiling(pthread_mutexattr_t* attr, int32_t prioceiling) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int32_t protocol) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int32_t pshared) {
    return pthread_attr_setpshared(&attr->value, pshared);
}

int32_t pthread_mutexattr_setrobust(pthread_mutexattr_t* attr, int32_t robust) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_mutexattr_settype(pthread_mutexattr_t* attr, int32_t type) {
    switch (type) {{
    } case PTHREAD_MUTEX_NORMAL:; {
    } case PTHREAD_MUTEX_RECURSIVE:; {
    } case PTHREAD_MUTEX_ERRORCHECK:; {
        attr->value = (attr->value & ~PTHREAD_ATTR_MASK_MUTEX_TYPE) | (type << TZ(PTHREAD_ATTR_MASK_MUTEX_TYPE));
        return 0;
    } default:; {
        // "The value type is invalid."
        return EINVAL;
    }}
}

int32_t pthread_condattr_init(pthread_condattr_t* attr) {
    attr->value = 0;
    return 0;
}

int32_t pthread_condattr_destroy(pthread_condattr_t* attr) {
    return 0;
}

int32_t pthread_condattr_setclock(pthread_condattr_t* attr, clockid_t clock_id) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_condattr_setpshared(pthread_condattr_t* attr, int32_t pshared) {
    return pthread_attr_setpshared(&attr->value, pshared);
}

int32_t pthread_condattr_getclock(const pthread_condattr_t* restrict attr, clockid_t* restrict clock_id) {
    // Not implemented.
    return ENOTSUP;
}

int32_t pthread_condattr_getpshared(const pthread_condattr_t* restrict attr, int32_t* restrict pshared) {
    return pthread_attr_getpshared(&attr->value, pshared);
}

int32_t pthread_rwlockattr_init(pthread_rwlockattr_t* attr) {
    attr->value = 0;
    return 0;
}

int32_t pthread_rwlockattr_destroy(pthread_rwlockattr_t* attr) {
    return 0;
}

int32_t pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int32_t pshared) {
    return pthread_attr_setpshared(&attr->value, pshared);
}

int32_t pthread_rwlockattr_getpshared(const pthread_rwlockattr_t* restrict attr, int32_t* restrict pshared) {
    return pthread_attr_getpshared(&attr->value, pshared);
}

int32_t pthread_getconcurrency(void) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_setconcurrency(int32_t val) {
    // Not implemented/applicable.
    return ENOTSUP;
}

int32_t pthread_getcpuclockid(pthread_t thread, clockid_t* clock_id) {
    // Not supported.
    return ENOTSUP;
}

int pthread_sigmask(int how, const sigset_t* restrict set, sigset_t* restrict oset) {
    // Not supported. Librcd fibers should not attempt to modify the signal mask of the worker thread.
    return ENOTSUP;
}
