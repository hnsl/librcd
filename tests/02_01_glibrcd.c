/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"
#include "vm-internal.h"

void rcd_self_test_glibrcd() {
    // Test malloc/free/calloc/realloc.
    {
        const int power = 4;
        char* data[5];
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            data[i] = malloc(size);
            atest((((size_t) data[i] / VM_ALLOC_ALIGN) * VM_ALLOC_ALIGN) == (size_t) data[i]);
            memset(data[i], i, size);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            for (int j = 0; j < size; j++)
                atest(data[i][j] == i);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            free(data[i]);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            data[i] = calloc(1, size);
            atest((((size_t) data[i] / VM_ALLOC_ALIGN) * VM_ALLOC_ALIGN) == (size_t) data[i]);
            for (int j = 0; j < size; j++)
                atest(data[i][j] == 0);
            memset(data[i], i, size);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            for (int j = 0; j < size; j++)
                atest(data[i][j] == i);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            data[i] = realloc(data[i], size / 2);
            atest((((size_t) data[i] / VM_ALLOC_ALIGN) * VM_ALLOC_ALIGN) == (size_t) data[i]);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            for (int j = 0; j < size / 2; j++)
                atest(data[i][j] == i);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            data[i] = realloc(data[i], size * 2);
            atest((((size_t) data[i] / VM_ALLOC_ALIGN) * VM_ALLOC_ALIGN) == (size_t) data[i]);
            memset(data[i] + size / 2, i, size / 2 + size);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            for (int j = 0; j < size * 2; j++)
                atest(data[i][j] == i);
        }
        for (int i = 0, size = power; i < 5; i++, size *= power) {
            free(data[i]);
        }
    }
    // Test strerror.
    {
        const char* str = strerror(ENOEXEC);
        atest(strcmp(str, "Exec format error") == 0);
    }{
        const char* str = strerror(ERANGE);
        atest(strcmp(str, "Result not representable") == 0);
    }{
        const char* str = strerror(31231);
        atest(str == 0);
    }
    // Test strtoul.
    {
        unsigned long number = strtoul("123", 0, 10);
        atest(number == 123);
    }{
        unsigned long number = strtoul("0x123", 0, 16);
        atest(number == 291);
    }{
        unsigned long number = strtoul("       +234", 0, 10);
        atest(number == 234);
    }{
        const char test[] = "    3244aag";
        char* end;
        unsigned long number = strtoul(test, &end, 16);
        atest(number == 3294378);
        atest(end == &test[sizeof(test) - 2]);
    }{
        unsigned long number = strtoul("FFFFFFFFFFFFFFFEq", 0, 16);
        atest(number == 0xFFFFFFFFFFFFFFFEUL);
    }{
        /// FIXME: The current strtoul() implementation does not pass this
        /// test case as it is not implemented to overflow at all.
        /*
        unsigned long number = strtoul("10000000000000000q", 0, 16);
        atest(number == ULONG_MAX);
        atest(errno == ERANGE);
        */
    }{
        long number = strtol("   -29  ", 0, 10);
        atest(number == -29);
    }{
        long number = strtol("-0x7FFFFFFFFFFFFFFEq", 0, 16);
        atest(number == -0x7FFFFFFFFFFFFFFE);
    }
    // Test strtod.
    {
        char szOrbits[] = "365.24 29.53";
        char* pEnd;
        double d1, d2;
        d1 = strtod(szOrbits, &pEnd);
        d2 = strtod(pEnd, NULL);
        atest(d1 == 365.24);
        atest(d2 == 29.53);
    }{
        /// FIXME: More strtod() tests.
    }
    // Test getaddrinfo().
    {
        printf("[getaddrinfo-test]: starting\n");
        struct addrinfo *result;
        int error;
        {
            char* hostname = "www.google.com";
            error = getaddrinfo(hostname, "http", NULL, &result);
            if (error != 0)
                atest(false);
            atest(result != 0);
            /* loop over all returned results and do inverse lookup */
            for (struct addrinfo* res = result; res != NULL; res = res->ai_next) {
                char str[INET_ADDRSTRLEN];
                struct sockaddr_in* addr = (void*) res->ai_addr;
                inet_ntop(AF_INET, &addr->sin_addr, str, INET_ADDRSTRLEN);
                printf("[getaddrinfo-test]: [%s] -> [%s]\n", hostname, str);
                atest(htons(addr->sin_port) == 80);
            }
            freeaddrinfo(result);
        }{
            char hostname[100];
            sprintf(hostname, "a%p%p.com", hostname, hostname);
            error = getaddrinfo(hostname, NULL, NULL, &result);
            atest(error == EAI_NONAME);
            printf("[getaddrinfo-test]: [%s] -> [EAI_NONAME]\n", hostname);
        }
    }
}
