/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"

void rcd_self_test_vsnprintf() {
    {
        char buffer[50];
        int n, a = 5, b = 3;
        n = sprintf(buffer, "%d plus %d is %d", a, b, a + b);
        atest(strlen(buffer) == n);
        atest(strcmp(buffer, "5 plus 3 is 8") == 0);
    }
    // TODO: Support floating point in vsnprintf().
    /*{
        float x = 0.75;
        char buf[50];
        int n = sprintf(buf, "Test=%.2f %s", x, "(it works?)");
        atest(strlen(buf) == n);
        atest(strcmp(buf, "Test=0.75 (it works?)") == 0);
    }{
        int raw_pipes[2];
        pipe(raw_pipes);
        FILE* pipes[2];
        pipes[0] = fdopen(raw_pipes[0], "r");
        pipes[1] = fdopen(raw_pipes[1], "w");
        float x = 2.5;
        const char expect[]  = "[x = 2.5]";
        int n = fprintf(pipes[1], "[x = %.1f]", x);
        atest(n == sizeof(expect) - 1);
        char buf[50];
        int n2 = fread(buf, n, 1, pipes[0]);
        atest(n2 == sizeof(expect) - 1);
        atest(strncmp(buf, expect, n2) == 0);
        fclose(pipes[0]);
        fclose(pipes[1]);
    }*/
    // TODO: More tests...
}
