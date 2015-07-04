/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"

#pragma librcd

/// Check if data is chi square random.
/// Original implementation at: https://en.wikibooks.org/wiki/Algorithm_Implementation/Pseudorandom_Numbers/Chi-Square_Test
/// This is a very simple frequency analysis, it does not check if the entropy is cryptographic grade (impossible).
/// A good rng will fail this test about 10% of the time.
static bool is_chi_square_random(fstr_t data) {
    // Basic test parameters.
    double n = data.len;
    double r = 0x100;
    // Check if too little data to proceed.
    if (n <= 10 * r)
        return false;
    // Part A: Get frequency of random numbers.
    size_t freq[0x100];
    memset(freq, 0, sizeof(freq));
    for (size_t i = 0; i < data.len; i++) {
        freq[data.str[i]]++;
    }
    // Part B: Calculate chi-square.
    double n_r = n / r;
    double chi_square = 0;
    for (size_t i = 0; i < LENGTHOF(freq); i++) {
        double f = ((double) freq[i]) - n_r;
        chi_square += f * f;
    }
    chi_square /= n_r;
    // Part C: The statistic should be within 2(r)^1/2 of r.
    double max_d = 2 * sqrt(r);
    return chi_square >= r - max_d && chi_square <= r + max_d;
}

static void self_test_rdrand() {
    for (size_t i = 1;; i++) sub_heap {
        // Test random data generator.
        vstr_t* random_data = vstr_new();
        for (size_t i = 0; i < 1000; i++) {
            uint64_t x = lwt_rdrand64();
            vstr_write(random_data, FSTR_PACK(x));
        }
        for (size_t i = 1; i < PAGE_SIZE * 10; i += (i / 8 + 1)) sub_heap {
            fstr_t buf = fss(fstr_alloc(i));
            lwt_rdrand(buf);
            vstr_write(random_data, buf);
        }
        if (is_chi_square_random(vstr_str(random_data)))
            return;
        if (i == 4) {
            // The probability of this is about 10%^4 = 0.01%.
            improb_atest(false, "the random data had poor entropy, broken rng?");
            return;
        }
    }
}

void rcd_self_test_misc() {
    self_test_rdrand();
}
