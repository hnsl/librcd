/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef PRNG_H
#define	PRNG_H

typedef struct prng_state prng_state_t;

/// Resets the PRNG state. After calling this the prng must be seeded with
/// entropy to return unpredictable numbers.
void prng_reset(prng_state_t* s);

/// Seeds the PRNG state manually.
/// This applies deterministic entropy on the PRNG.
void prng_seed_manual(prng_state_t* s, uint64_t seed);

/// Seeds the PRNG state with the cycle counter (clock).
/// This applies non-deterministic entropy on the PRNG.
void prng_seed_time(prng_state_t* s);

/// Returns the PRNG thread local state which may not have been appropriately
/// seeded. Only use this reference temporary and never between defer points
/// that might context switch the fiber to another thread.
prng_state_t* prng_get_thread_state();

/// Returns the next deterministic pseudo random number and
/// deterministically modifies the PRNG state.
uint64_t prng_rand_ds(prng_state_t* s);

/// Returns a non-deterministic pseudo random number and modifies the PRNG
/// state. This function automatically seeds the internal state with time when
/// it decides it's appropriate.
uint64_t prng_rand_nds(prng_state_t* s);

/// Returns a non-deterministic random number that is always time seeded
/// automatically. Modifies the thread state returned by
/// prng_get_thread_state().
uint64_t prng_rand();

/// Allocates a new PRNG with a reset state.
/// After calling this the PRNG must be seeded with entropy to return
/// unpredictable numbers.
prng_state_t* prng_alloc_new();

#endif	/* PRNG_H */
