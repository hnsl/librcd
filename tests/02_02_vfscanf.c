/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"

void rcd_self_test_vfscanf() {
    {
        int a, b, c;
        int n = sscanf("5 plus 3 is 8", "%d plus %d is %d", &a, &b, &c);
        atest(n == 3);
        atest(a == 5);
        atest(b == 3);
        atest(c == 8);
    }{
        int a, b, c;
        int n = sscanf("5 plu 3 is 8", "%d plus %d is %d", &a, &b, &c);
        atest(n == 1);
        atest(a == 5);
    }{
        int a, b, c;
        int n = sscanf("stuff", "%d plus %d is %d", &a, &b, &c);
        atest(n == 0);
    }{
        char buf[64];
        int n = sscanf("fooo (it-works?)", "fooo %s", buf);
        atest(n == 1);
        atest(strcmp(buf, "(it-works?)") == 0);
    }{
        char buf[64];
        int n = sscanf("fooo", "fooo %s", buf);
        atest(n == EOF);
    }
    // TODO: More tests...
}
