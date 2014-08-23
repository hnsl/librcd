/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef POLAR_H
#define	POLAR_H

#define RCD_POLAR_E(r_code_expr, exception_type) \
    polar_error(r_code_expr, fstr(#r_code_expr), exception_type)

#define RCD_POLAR_ECE(r_code_expr, exception_type) ({ \
    int32_t __r_code = r_code_expr; \
    if (__r_code != 0) \
        polar_error(__r_code, fstr(#r_code_expr), exception_type); \
})

#define RCD_POLAR_EC(r_code_expr) RCD_POLAR_ECE(r_code_expr, exception_io)

/// Like polar_secure_drbg_fill() but the polarssl prng callback compatible interface.
int32_t polar_secure_drbg_random(void* __unused, unsigned char* output, size_t output_len);

/// Fills a buffer with cryptographically secure random data from a persistent
/// thread local state to save memory and initialization cpu time.
/// Succeeds or throws a fatal exception.
void polar_secure_drbg_fill(fstr_t buffer);

/// Throws an exception with a descriptive error message of the specified type.
/// Assumes r_code is a polarssl return value.
__attribute__((noreturn))
void polar_error(int32_t r_code, fstr_t expr_str, rcd_exception_type_t etype);

/// Same function as polar_tls_client_open() but allows specifying the tcp
/// keep alive configuration for the underlying tcp connection.
/// If any value in tt is equal or less than 0 no timeout is configured.
rcd_sub_fiber_t* polar_tls_client_open_ka(rio_in_addr4_t addr, fstr_t host_cname, rio_tcp_ka_t ka, rio_t** rio_h_out) NO_NULL_ARGS;

/// Opens a tls client connection. You SHOULD supply the cannotical name of the
/// host you're connecting to (e.g. example.org) for certificate validation and
/// host name extension. Leaving host_cname blank will disable active security.
rcd_sub_fiber_t* polar_tls_client_open(rio_in_addr4_t addr, fstr_t host_cname, rio_t** rio_h_out) NO_NULL_ARGS;

#endif	/* POLAR_H */
