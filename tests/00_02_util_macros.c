/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

void rcd_self_test_util_macros() {
    // Test LENGTHOF.
    {
        int foo[] = {3, 4, 5};
        atest(LENGTHOF(foo) == 3);
        int bar[] = {};
        atest(LENGTHOF(bar) == 0);
        char baz[] = {99, 99, 99, 99, 99, 99, 99};
        atest(LENGTHOF(baz) == 7);
    }

    // Test FOR_EACH_ARG.
    {
        int counter = 0, a, b, c;
#define INCREMENT(which) which = counter++
        FOR_EACH_ARG(INCREMENT);
        atest(counter == 0);
        FOR_EACH_ARG(INCREMENT, a);
        atest(a == 0);
        atest(counter == 1);
        FOR_EACH_ARG(INCREMENT, b, c);
        atest(b == 1);
        atest(c == 2);
        atest(counter == 3);
        (FOR_EACH_ARG(INCREMENT, a, b, c));
    }

    // Test VA_NARGS.
    {
        atest(VA_NARGS() == 0);
        atest(VA_NARGS(x) == 1);
        atest(VA_NARGS(x, y) == 2);
        atest(VA_NARGS(, ) == 2);
        atest(VA_NARGS((x, y, z), ()) == 2);
    }
}
