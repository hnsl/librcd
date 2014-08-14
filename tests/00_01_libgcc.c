/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "musl.h"

void rcd_self_test_libgcc() {
    {
        // Test __floatuntidf. The memory barrier enforces run time calculation.
        uint128_t a = 19, b = 5;
        sync_synchronize();
        double foo = (a / b);
        atest(foo == 3.0f);
    }{
        // Test __fixunssfti. The memory barrier enforces run time cast.
        float foo = 123456.78901f;
        sync_synchronize();
        uint128_t bar = foo;
        atest(bar == 123456);
    }
}
