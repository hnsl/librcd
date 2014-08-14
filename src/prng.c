/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#define QSIZE 0x1000
#define CNG(state) (state->cng = 6906969069ULL * state->cng + 13579)
#define XS(state) (state->xs ^= (state->xs << 13), state->xs ^= (state->xs >> 17), state->xs ^= (state->xs << 43))

struct prng_state {
    uint64_t qary[QSIZE];
    uint64_t j;
    uint64_t carry;
    uint64_t xs;
    uint64_t cng;
};

/// Every thread has a cached prng state for fast random number generation.
RCD_DEFINE_THREAD_STATIC_MEMORY(prng_state_t prng_thread_state) = {0};
RCD_DEFINE_THREAD_STATIC_MEMORY(bool prng_thread_is_initialized) = false;

void prng_reset(prng_state_t* s) {
    fstr_fill(FSTR_PACK(s->qary), 0);
    s->j = QSIZE - 1;
    s->carry = 0;
    s->xs = 362436069362436069ULL;
    s->cng = 123456789987654321ULL; /* use this as the seed */
}

static uint64_t prng_b64mwc(prng_state_t* s) {
    uint64_t t, x;
    s->j = (s->j + 1) & (QSIZE - 1);
    x = s->qary[s->j];
    t = (x << 28) + s->carry;
    s->carry = (x >> 36) - (t < x);
    return (s->qary[s->j] = t - x);
}

/* Initialize PRNG with default seed */
static void prng_seed(prng_state_t* s) {
    /* Seed qary[] with CNG+XS: */
    for (size_t i = 0; i < QSIZE; i++)
        s->qary[i] = CNG(s) + XS(s);
}

void prng_seed_manual(prng_state_t* s, uint64_t seed) {
    s->cng ^= seed;
    s->xs ^= s->cng;
    prng_seed(s);
}

uint64_t prng_rand_ds(prng_state_t* s) {
    return (prng_b64mwc(s) + CNG(s) + XS(s));
}

static void prng_warmup(prng_state_t* s, size_t n_rounds) {
    /* Run through several rounds to warm up the state */
    for (size_t i = 0; i < n_rounds; i++)
        prng_rand_ds(s);
}

void prng_seed_time(prng_state_t* s) {
    uint128_t t = readcyclecounter();
    prng_seed_manual(s, t & UINT64_MAX);
    prng_seed_manual(s, t >> 64);
}

prng_state_t* prng_get_thread_state() {
    prng_state_t* state = lwt_get_thread_static_ptr(&prng_thread_state);
    bool* is_initialized = lwt_get_thread_static_ptr(&prng_thread_is_initialized);
    if (!*is_initialized) {
        // Initialize the the MT state for the thread on demand.
        prng_reset(state);
        prng_seed_time(state);
        *is_initialized = true;
    }
    return state;
}

uint64_t prng_rand_nds(prng_state_t* state) {
    // Return the next random number.
    uint64_t r = prng_rand_ds(state);
    // Seed with more entropy when the internal state wraps around.
    if (state->j == (QSIZE - 1))
        prng_seed_time(state);
    return r;
}

uint64_t prng_rand() {
    return prng_rand_nds(prng_get_thread_state());
}

prng_state_t* prng_alloc_new() {
    prng_state_t* state = new(prng_state_t);
    prng_reset(state);
    return state;
}
