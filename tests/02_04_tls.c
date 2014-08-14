/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#define TEST_HOST_CNAME "www.ietf.org"

#pragma librcd

void rcd_self_test_tls() {
    {
        list(uint32_t)* itef_ips = rio_resolve_host_ipv4_addr(TEST_HOST_CNAME);
        rio_in_addr4_t itef_addr;
        itef_addr.address = list_peek_start(itef_ips, uint32_t);
        itef_addr.port = 443;
        DBGFN(concs("testing tls by doing https connection to [", fss(rio_serial_in_addr4(itef_addr)), "]"));
        sub_heap {
            rio_t* tls_h;
            rcd_sub_fiber_t* tls_sf = polar_tls_client_open(itef_addr, TEST_HOST_CNAME, &tls_h);
            rio_write(tls_h, "GET /rfc/rfc5246.txt HTTP/1.1\r\nHost: " TEST_HOST_CNAME "\r\nUser-Agent: librcd unit test\r\n\r\n");
            fstr_t buffer = fss(fstr_alloc_buffer(PAGE_SIZE * 10000));
            /* for (;;) DBG(rio_read_part(tls_h, buffer, 0));*/
            try {
                rio_read_to_separator(tls_h, "Please address the information to the IETF", buffer);
            } catch (exception_io, e) {
                throw_fwd("did not find matching string in response", exception_io, e);
            }
        }
    }
}
