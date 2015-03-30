/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef POLAR_H
#define	POLAR_H

/// Like polar_error, but induces the exception string from the name of the
/// error code variable.
#define RCD_POLAR_E(r_code_expr, exception_type) \
    polar_error(r_code_expr, fstr(#r_code_expr), exception_type)

/// Like RCD_POLAR_E, but only throws an exception if the code is non-zero.
#define RCD_POLAR_ECE(r_code_expr, exception_type) ({ \
    int32_t __r_code = r_code_expr; \
    if (__r_code != 0) \
        polar_error(__r_code, fstr(#r_code_expr), exception_type); \
})

#define POLARSSL_ERROR_STRERROR_BC

#define RCD_POLAR_EC(r_code_expr) RCD_POLAR_ECE(r_code_expr, exception_io)

/// Polarssl server certificate and key.
typedef struct polar_sck {
    struct _x509_crt* own_cert;
    struct _pk_context* private_key;
} polar_sck_t;

typedef struct polar_tls_sni_cb {
    /// Callback from TLS handshake where client has presented the hostname it is
    /// attempting to connect to. Should return the certificate and rsa key that
    /// will be used from this point. If no cert or rsa key exist for the
    /// specified host the function should return false so the handshake can
    /// fail gracefully.
    bool (*fn)(fstr_t hostname, polar_sck_t* out_sck, void* arg_ptr);
    /// Third argument of callback fn.
    void* arg_ptr;
} polar_tls_sni_cb_t;

/// Like polar_secure_drbg_fill() but the polarssl prng callback compatible interface.
int32_t polar_secure_drbg_random(void* __unused, unsigned char* output, size_t output_len);

/// Fills a buffer with cryptographically secure random data from a persistent
/// thread local state to save memory and initialization cpu time.
/// Succeeds or throws a fatal exception.
void polar_secure_drbg_fill(fstr_t buffer);

/// Throws an exception with a descriptive error message of the specified type.
/// Assumes r_code is a polarssl return value.
noret void polar_error(int32_t r_code, fstr_t expr_str, rcd_exception_type_t etype);

/// Starts a new tls server session on the specified socket. The session is
/// executed on an internal session sub fiber which the socket is passed to.
/// The socket becomes useless after this call and should be thrown away.
/// The server should either have a static sck (cert + key) or a sni callback,
/// an arg exception will be thrown otherwise. The sck and sni_cb memory
/// must not be valid after the call returns but any internal passed cert or
/// key memory must be valid for the life time of tls session fiber.
/// A new rio stream is returned to rio_h_out which should be used for I/O.
/// The session sub fiber is returned.
rcd_sub_fiber_t* polar_tls_server(rio_t* socket, polar_sck_t* sck, polar_tls_sni_cb_t* sni_cb, rio_t** rio_h_out);

/// Starts a new tls client session on the specified socket. The session is
/// executed on an internal session fiber which the socket is passed to. The
/// socket becomes useless after this call and should be thrown away.
/// A new rio stream is returned to rio_h_out which should be used for I/O.
/// The session sub fiber is returned.
rcd_sub_fiber_t* polar_tls_client(rio_t* socket, fstr_t host_cname, rio_t** rio_h_out);

/// Same function as polar_tls_client_open() but allows specifying the tcp
/// keep alive configuration for the underlying tcp connection.
/// If any value in tt is equal or less than 0 no timeout is configured.
rcd_sub_fiber_t* polar_tls_client_open_ka(rio_in_addr4_t addr, fstr_t host_cname, rio_tcp_ka_t ka, rio_t** rio_h_out) NO_NULL_ARGS;

/// Opens a tls client connection. You SHOULD supply the canonical name of the
/// host you're connecting to (e.g. example.org) for certificate validation and
/// host name extension. Leaving host_cname blank will disable active security.
rcd_sub_fiber_t* polar_tls_client_open(rio_in_addr4_t addr, fstr_t host_cname, rio_t** rio_h_out) NO_NULL_ARGS;

#endif	/* POLAR_H */
