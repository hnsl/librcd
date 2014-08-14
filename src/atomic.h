/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef CAS_H
#define	CAS_H

#include "linux.h"

#define ATOMIC_RWLOCK_IS_WLOCKED(rwlock) (rwlock != 0)
#define ATOMIC_RWLOCK_IS_RLOCKED(rwlock) (rwlock > 0)

typedef int32_t rwspinlock_t;

static inline bool atomic_cas_uint8(uint8_t*, uint8_t, uint8_t) __attribute__((const, always_inline));

static inline bool atomic_cas_uint8(uint8_t* ptr, uint8_t oldval, uint8_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_int8(int8_t*, int8_t, int8_t) __attribute__((const, always_inline));

static inline bool atomic_cas_int8(int8_t* ptr, int8_t oldval, int8_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_uint16(uint16_t*, uint16_t, uint16_t) __attribute__((const, always_inline));

static inline bool atomic_cas_uint16(uint16_t* ptr, uint16_t oldval, uint16_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_int16(int16_t*, int16_t, int16_t) __attribute__((const, always_inline));

static inline bool atomic_cas_int16(int16_t* ptr, int16_t oldval, int16_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_uint32(uint32_t*, uint32_t, uint32_t) __attribute__((const, always_inline));

static inline bool atomic_cas_uint32(uint32_t* ptr, uint32_t oldval, uint32_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_int32(int32_t*, int32_t, int32_t) __attribute__((const, always_inline));

static inline bool atomic_cas_int32(int32_t* ptr, int32_t oldval, int32_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_uint64(uint64_t*, uint64_t, uint64_t) __attribute__((const, always_inline));

static inline bool atomic_cas_uint64(uint64_t* ptr, uint64_t oldval, uint64_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_int64(int64_t*, int64_t, int64_t) __attribute__((const, always_inline));

static inline bool atomic_cas_int64(int64_t* ptr, int64_t oldval, int64_t newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

static inline bool atomic_cas_uint128(uint128_t* ptr, uint128_t oldval, uint128_t newval) {
    uint64_t old_low = oldval & 0xffffffffffffffff;
    uint64_t old_high = oldval >> 64;
    uint64_t new_low = newval & 0xffffffffffffffff;
    uint64_t new_high = newval >> 64;
    bool result;
    __asm__(
        "lock cmpxchg16b %1\n\t"
        "setz %0\n\t"
        : "=q" (result)
        , "+m" (*ptr)
        , "+d" (old_high)
        , "+a" (old_low)
        : "c" (new_high)
        , "b" (new_low)
        : "cc"
    );
    return result;
}

static inline bool atomic_cas_int128(int128_t* ptr, int128_t oldval, int128_t newval) {
    return atomic_cas_uint128((uint128_t*) ptr, (uint128_t) oldval, (uint128_t) newval);
}

/* Heads up: Only suitable for static pointers due to ABA problem. */
static inline bool atomic_cas_ptr(void** ptr, void* oldval, void* newval) {
    return sync_bool_compare_and_swap(ptr, oldval, newval);
}

__attribute__((always_inline))
static inline int32_t atomic_spin_yield(uint32_t unyielded_spins) {
    const int atomic_unyielded_spins_max = 0x1000;
    __asm__("pause"::: "memory");
    uint32_t next_unyielded_spins;
    if (unyielded_spins == 0) {
        next_unyielded_spins = atomic_unyielded_spins_max;
    } else {
        next_unyielded_spins = unyielded_spins - 1;
        if (next_unyielded_spins == 0)
            sched_yield();
    }
    return next_unyielded_spins;
}

__attribute__((always_inline))
static inline void atomic_spinlock_rlock(rwspinlock_t* rwspinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        rwspinlock_t rwspinlock_v = *rwspinlock;
        if (rwspinlock_v >= 0 && atomic_cas_int32(rwspinlock, rwspinlock_v, rwspinlock_v + 1))
            return;
    }
}

__attribute__((always_inline))
static inline void atomic_spinlock_wlock(rwspinlock_t* rwspinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        if (atomic_cas_int32(rwspinlock, 0, -1))
            break;
    }
}

__attribute__((always_inline))
static inline void atomic_spinlock_urlock(rwspinlock_t* rwspinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        rwspinlock_t rwspinlock_v = *rwspinlock;
        if (atomic_cas_int32(rwspinlock, rwspinlock_v, rwspinlock_v - 1))
            break;
        sync_synchronize();
    }
}

__attribute__((always_inline))
static inline void atomic_spinlock_uwlock(rwspinlock_t* rwspinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        // This CAS should never fail but we loop anyway in case of aggressive
        // hardware false-negative, and infinite loop is the best form of UB as
        // the program does not appear to work but by coincidence.
        if (atomic_cas_int32(rwspinlock, -1, 0))
            break;
    }
}

__attribute__((always_inline))
static inline bool atomic_spinlock_trylock(int8_t* spinlock) {
    return atomic_cas_int8(spinlock, 0, -1);
}

__attribute__((always_inline))
static inline void atomic_spinlock_lock(int8_t* spinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        if (atomic_spinlock_trylock(spinlock))
            break;
    }
}

__attribute__((always_inline))
static inline void atomic_spinlock_unlock(int8_t* spinlock) {
    for (register uint32_t n_spins = 0;; n_spins = atomic_spin_yield(n_spins)) {
        if (atomic_cas_int8(spinlock, -1, 0))
            break;
    }
}

#endif	/* CAS_H */
