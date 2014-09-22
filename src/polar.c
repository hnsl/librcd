/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#include "polarssl/error.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/ssl.h"

#define POLAR_TLS_MAX_CLOSE_WAIT_NS (4 * RIO_NS_SEC)

#define POLAR_TLS_CA_PATHS {"/etc/ssl/certs", "/etc/ssl/private"}

#pragma librcd

typedef struct sec_prng_state {
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
} sec_drbg_state_t;

typedef struct tls_session {
    rcd_exception_t* pending_exception;
    ssl_context ssl_ctx;
    rio_t* rio_h;
    bool* read_hint;
    bool write_hint;
    bool is_read_ready;
    bool is_write_ready;
    bool is_handshake_over;
 //   fstr_t write_buffer;
} tls_session_t;

join_locked(int32_t) sec_drgb_fill(unsigned char* output, size_t output_len, join_server_params, sec_drbg_state_t* state) {
    for (;;) {
        size_t chunk_size = MIN(output_len, CTR_DRBG_MAX_REQUEST);
        RCD_POLAR_ECE(ctr_drbg_random(&state->ctr_drbg, output, output_len), exception_fatal);
        output_len -= chunk_size;
        if (output_len == 0)
            break;
        output += chunk_size;
    }
    return 0;
}

fiber_main sec_drgb_fiber(fiber_main_attr, sec_drbg_state_t* state) {
    auto_accept_join(sec_drgb_fill, join_server_params, state);
}

static void init_sec_drbg_fid(void* arg_ptr) {
    rcd_fid_t* fid_ptr = arg_ptr;
    try uninterruptible {
        fmitosis {
            sec_drbg_state_t* state = new(sec_drbg_state_t);
            entropy_init(&state->entropy);
            fstr_t ctr_drbg_pers = "librcd-ctr-drbg";
            RCD_POLAR_ECE(ctr_drbg_init(&state->ctr_drbg, entropy_func, &state->entropy, ctr_drbg_pers.str, ctr_drbg_pers.len), exception_fatal);
            *fid_ptr = spawn_static_fiber(sec_drgb_fiber("", state));
        }
    } catch (exception_any, e) {
        throw_fwd("initializing rng failed", exception_fatal, e);
    }
}

int32_t polar_secure_drbg_random(void* __unused, unsigned char* output, size_t output_len) {
    static rcd_fid_t sec_drgb_fid = 0;
    static lwt_once_t sec_drgb_once = LWT_ONCE_INIT;
    lwt_once(&sec_drgb_once, init_sec_drbg_fid, &sec_drgb_fid);
    try {
        return sec_drgb_fill(output, output_len, sec_drgb_fid);
    } catch (exception_inner_join_fail, e) {
        throw_fwd(e->message, exception_fatal, e);
    }
}

void polar_secure_drbg_fill(fstr_t buffer) {
    polar_secure_drbg_random(0, buffer.str, buffer.len);
}

void polar_error(int32_t r_code, fstr_t expr_str, rcd_exception_type_t etype) {
    char error_str[0x80];
    error_strerror(r_code, error_str, sizeof(error_str) - 1);
    sub_heap_e(throw(concs("polarssl error @ [", expr_str, "]: got return code [", fss(fstr_from_int(r_code, 16)), "h]: [", fstr_fix_cstr(error_str), "]"), etype));
}

static int tls_bio_recv(void* arg_ptr, unsigned char* buffer_ptr, size_t len) {
    tls_session_t* session = (tls_session_t*) arg_ptr;
    fstr_t buffer = {.str = (uint8_t*) buffer_ptr, .len = len};
    fstr_t chunk = rio_read_part(session->rio_h, buffer, session->read_hint);
    return chunk.len;
}

static int tls_bio_send(void* arg_ptr, const unsigned char* buffer_ptr, size_t len) {
    tls_session_t* session = (tls_session_t*) arg_ptr;
    fstr_t buffer = {.str = (uint8_t*) buffer_ptr, .len = len};
    if (session->is_handshake_over) {
        // Stop sending now if writing would block.
        if (!rio_poll(session->rio_h, false, false))
            return POLARSSL_ERR_NET_WANT_WRITE;
    }
    fstr_t chunk = fstr_detail(buffer, rio_write_chunk(session->rio_h, buffer, session->write_hint));
    return chunk.len;
}

static void tls_session_dbg(void * __unused, int code, const char* cstr) {
    DBGFN("code:[", i2fs(code), "] ", fstr_fix_cstr(cstr));
}

join_locked(fstr_t) tls_read(fstr_t buffer, bool* more_hint_out, join_server_params, tls_session_t* session) {
    if (buffer.len == 0)
        return buffer;
    if (session->pending_exception != 0)
        throw_fwd("tls io error", exception_io, lwt_copy_exception(session->pending_exception));
    session->read_hint = more_hint_out;
    int32_t ssl_read_ret = ssl_read(&session->ssl_ctx, buffer.str, buffer.len);
    if (ssl_read_ret == 0) {
        throw("tls stream ended", exception_io);
    } else if (ssl_read_ret < 0) {
        if (ssl_read_ret == POLARSSL_ERR_NET_WANT_WRITE) {
            // Write buffer became full while replying with alert.
            return fstr_slice(buffer, 0, 0);
        }
        RCD_POLAR_E(ssl_read_ret, exception_io);
    } else {
        return fstr_slice(buffer, 0, ssl_read_ret);
    }
}

join_locked(fstr_t) tls_write(fstr_t buffer, bool more_hint, join_server_params, tls_session_t* session) {
    if (buffer.len == 0)
        return buffer;
    if (session->pending_exception != 0)
        throw_fwd("tls io error", exception_io, lwt_copy_exception(session->pending_exception));
    session->write_hint = more_hint;
    int32_t ssl_write_ret = ssl_write(&session->ssl_ctx, buffer.str, buffer.len);
    if (ssl_write_ret < 0) {
        if (ssl_write_ret == POLARSSL_ERR_NET_WANT_WRITE)
            return buffer;
        RCD_POLAR_ECE(ssl_write_ret, exception_io);
    }
    return fstr_slice(buffer, ssl_write_ret, -1);
}

join_locked(bool) tls_poll(bool read, join_server_params, tls_session_t* session) {
    if (session->pending_exception != 0)
        return true;
    return read? session->is_read_ready: session->is_write_ready;
}

join_locked(void) tls_epollr_notify(join_server_params, tls_session_t* session) {}

join_locked(void) tls_epollw_notify(join_server_params, tls_session_t* session) {}

static void init_ca_chain(void* arg_ptr) { uninterruptible sub_heap {
    x509_cert* ca_chain = arg_ptr;
    fstr_t ca_paths[] = POLAR_TLS_CA_PATHS;
    for (size_t i = 0; i < LENGTHOF(ca_paths); i++) {
        fstr_t ca_path = ca_paths[i];
        try {
            list_foreach(rio_file_list(ca_path), fstr_mem_t*, fname) {
                fstr_t ca_file_path = concs(ca_path, "/", fss(fname));
                try {
                    if (rio_file_stat(ca_file_path).file_type != rio_file_type_regular)
                        break;
                    /* DBG("reading cert [", ca_file_path, "]"); */
                    RCD_POLAR_ECE(x509parse_crtfile(ca_chain, fstr_to_cstr(ca_file_path)), exception_io);
                } catch (exception_io, e) {
                    /* rio_debug(concs("tls init ca error: failed to read cert [", ca_file_path, "], certificate validation might fail:\n")); */
                    /* rio_debug(fss(lwt_get_exception_dump(e))); */
                }
            }
        } catch (exception_io, e) {
            rio_debug(concs("tls init ca error: failed to open cert path [", ca_path, "], certificate validation might fail:\n"));
            rio_debug(fss(lwt_get_exception_dump(e)));
        }
    }
}}

static x509_cert* tls_get_ca_chain() {
    static lwt_once_t once = LWT_ONCE_INIT;
    static x509_cert ca_chain = {0};
    lwt_once(&once, init_ca_chain, &ca_chain);
    return &ca_chain;
}

typedef void (*notify_join_fn_t)(rcd_fid_t);

fiber_main tls_epoll_fiber(fiber_main_attr, notify_join_fn_t notify_join_fn, rio_epoll_t* epoll_h, rcd_fid_t tls_fid) { try {
    for (;;) {
        rio_epoll_poll(epoll_h, true);
        notify_join_fn(tls_fid);
    }
} catch (exception_canceled, e); }

static void tls_session(tls_session_t session) {
    try {
        for (bool handshake_done = false; !handshake_done;) {
            int32_t handshake_r = ssl_handshake(&session.ssl_ctx);
            switch (handshake_r) {{
            } case 0: {
                handshake_done = true;
                break;
            } case POLARSSL_ERR_NET_WANT_READ: {
            } case POLARSSL_ERR_NET_WANT_WRITE: {
                break;
            } default: {
                polar_error(handshake_r, "ssl_handshake", exception_io);
            }};
        }
        session.is_handshake_over = true;
        // Create a sub fiber that monitors for kernel read epoll events and notifies to allow the set of
        // accepted functions to be refreshed based on the new volatile return value of rio_poll().
        fmitosis {
            rio_epoll_t* epoll_h = rio_epoll_create(session.rio_h, rio_epoll_event_inlvl);
            spawn_fiber(tls_epoll_fiber("r", tls_epollr_notify, epoll_h, rcd_self));
        }
        // Create a sub fiber that monitors for kernel write epoll events and notifies to allow the set of
        // accepted functions to be refreshed based on the new volatile return value of ssl_flush_output().
        fmitosis {
            rio_epoll_t* epoll_h = rio_epoll_create(session.rio_h, rio_epoll_event_outlvl);
            spawn_fiber(tls_epoll_fiber("w", tls_epollw_notify, epoll_h, rcd_self));
        }
        for (;;) {
            // Determine if read is ready.
            bool is_read_ready = (ssl_get_bytes_avail(&session.ssl_ctx) > 0 || rio_poll(session.rio_h, true, false));
            // Flush any pending output buffer while determining if write is ready.
            bool is_write_ready;
            int32_t flush_r = ssl_flush_output(&session.ssl_ctx);
            if (flush_r == 0) {
                is_write_ready = true;
            } else if (flush_r == POLARSSL_ERR_NET_WANT_WRITE) {
                is_write_ready = false;
            } else {
                polar_error(flush_r, "ssl_flush_output", exception_io);
            }
            // Accept joins depending on whether we are ready to read/write.
            session.is_read_ready = is_read_ready;
            session.is_write_ready = is_write_ready;
            accept_join( \
                tls_poll, \
                tls_read if (is_read_ready), \
                tls_write if (is_write_ready), \
                tls_epollr_notify if (!is_read_ready), \
                tls_epollw_notify if (!is_write_ready), \
                join_server_params, \
                &session \
            );
        }
    } finally uninterruptible {
        session.is_handshake_over = true;
        try {
            ssl_close_notify(&session.ssl_ctx);
        } catch (exception_io, e);
        ssl_free(&session.ssl_ctx);
    } catch (exception_io, e) {
        session.pending_exception = e;
        auto_accept_join(tls_poll, tls_read, tls_write, join_server_params, &session);
    }
}

static int tls_server_sni_cb(void* parameter, ssl_context* ssl, const unsigned char* hostname_ptr, size_t len) {
    polar_tls_sni_cb_t* sni_cb = parameter;
    fstr_t hostname = {.str = (void*) hostname_ptr, .len = len};
    polar_sck_t sck;
    if (!sni_cb->fn(hostname, &sck, sni_cb->arg_ptr))
        return 1;
    ssl_set_own_cert(ssl, sck.own_cert, sck.rsa_key);
    return 0;
}

fiber_main tls_server_session_fiber(fiber_main_attr, rio_t* socket, polar_sck_t* sck, polar_tls_sni_cb_t* sni_cb) { try {
    tls_session_t session = {
        .rio_h = socket,
    };
    RCD_POLAR_ECE(ssl_init(&session.ssl_ctx), exception_fatal);
    /* ssl_set_dbg(&session.ssl_ctx, tls_session_dbg, 0); */
    ssl_set_endpoint(&session.ssl_ctx, SSL_IS_SERVER);
    ssl_set_authmode(&session.ssl_ctx, SSL_VERIFY_NONE);
    ssl_set_bio(&session.ssl_ctx, tls_bio_recv, &session, tls_bio_send, &session);
    if (sck != 0)
        ssl_set_own_cert(&session.ssl_ctx, sck->own_cert, sck->rsa_key);
    if (sni_cb != 0)
        ssl_set_sni(&session.ssl_ctx, tls_server_sni_cb, sni_cb);
    // TODO: We disable renegotiation. We should fix this for protection against leaking entropy.
    ssl_set_renegotiation(&session.ssl_ctx, SSL_RENEGOTIATION_DISABLED);
    ssl_legacy_renegotiation(&session.ssl_ctx, SSL_LEGACY_NO_RENEGOTIATION);
    ssl_set_rng(&session.ssl_ctx, polar_secure_drbg_random, 0);
    // Enter tls session.
    tls_session(session);
} catch (exception_desync, e); }

fiber_main tls_client_session_fiber(fiber_main_attr, rio_t* socket, fstr_t host_cname) { try {
    tls_session_t session = {
        .rio_h = socket,
    };
    char* host_cname_cstr = (host_cname.len > 0)? fstr_to_cstr(host_cname): 0;
    RCD_POLAR_ECE(ssl_init(&session.ssl_ctx), exception_fatal);
    /* ssl_set_dbg(&session.ssl_ctx, tls_session_dbg, 0); */
    ssl_set_endpoint(&session.ssl_ctx, SSL_IS_CLIENT);
    ssl_set_authmode(&session.ssl_ctx, SSL_VERIFY_REQUIRED);
    ssl_set_ca_chain(&session.ssl_ctx, tls_get_ca_chain(), 0, host_cname_cstr);
    ssl_set_bio(&session.ssl_ctx, tls_bio_recv, &session, tls_bio_send, &session);
    // TODO: We disable renegotiation. We should fix this for protection against leaking entropy.
    ssl_set_renegotiation(&session.ssl_ctx, SSL_RENEGOTIATION_DISABLED);
    ssl_legacy_renegotiation(&session.ssl_ctx, SSL_LEGACY_NO_RENEGOTIATION);
    ssl_set_rng(&session.ssl_ctx, polar_secure_drbg_random, 0);
    // This is a modern TLS lib so we should support host name extension.
    ssl_set_hostname(&session.ssl_ctx, host_cname_cstr);
    // Enter tls session.
    tls_session(session);
} catch (exception_desync, e); }

static fstr_t tls_rio_read(rcd_fid_t fid_arg, fstr_t buffer, bool* more_hint_out) {
    try {
        for (;;) {
            fstr_t chunk = tls_read(buffer, more_hint_out, fid_arg);
            if (buffer.len == 0 || chunk.len > 0)
                return chunk;
        }
    } catch (exception_inner_join_fail, e) {
        throw_fwd("tls read failure", exception_io, e);
    }
}

static fstr_t tls_rio_write(rcd_fid_t fid_arg, fstr_t buffer, bool more_hint) {
    try {
        return tls_write(buffer, more_hint, fid_arg);
    } catch (exception_inner_join_fail, e) {
        throw_fwd("tls write failure", exception_io, e);
    }
}

static bool tls_rio_poll(rcd_fid_t fid_arg, bool read, bool wait) {
    try {
        if (wait) {
            // Block on read or write until the join is accepted.
            if (read) {
                tls_read("", 0, fid_arg);
            } else {
                tls_write("", false, fid_arg);
            }
            return true;
        } else {
            // Non blocking poll.
            return tls_poll(read, fid_arg);
        }
    } catch (exception_inner_join_fail, e) {
        return true;
    }
}

const static rio_class_t tls_impl = {
    .read_part_fn = tls_rio_read,
    .write_part_fn = tls_rio_write,
    .poll_fn = tls_rio_poll,
};

rcd_sub_fiber_t* polar_tls_server(rio_t* socket, polar_sck_t* sck, polar_tls_sni_cb_t* sni_cb, rio_t** rio_h_out) {
    if (sck == 0 && sni_cb == 0)
        throw("must specify either sck or sni_cb", exception_arg);
    rcd_sub_fiber_t* tls_sf;
    fmitosis {
        rio_t* raw_socket = rio_realloc(socket);
        tls_sf = spawn_fiber(tls_server_session_fiber("", raw_socket, cln(sck), cln(sni_cb)));
    }
    *rio_h_out = rio_new_abstract(&tls_impl, sfid(tls_sf), 0);
    return tls_sf;
}

rcd_sub_fiber_t* polar_tls_client(rio_t* socket, fstr_t host_cname, rio_t** rio_h_out) {
    rcd_sub_fiber_t* tls_sf;
    fmitosis {
        rio_t* raw_socket = rio_realloc(socket);
        tls_sf = spawn_fiber(tls_client_session_fiber("", raw_socket, fsc(host_cname)));
    }
    *rio_h_out = rio_new_abstract(&tls_impl, sfid(tls_sf), 0);
    return tls_sf;
}

rcd_sub_fiber_t* polar_tls_client_open_ka(rio_in_addr4_t addr, fstr_t host_cname, rio_tcp_ka_t ka, rio_t** rio_h_out) {
    rio_t* socket = rio_tcp_client(addr);
    if (ka.idle_before_ping_s > 0 && ka.ping_interval_s > 0 && ka.count_before_timeout > 0)
        rio_tcp_set_keepalive(socket, ka);
    rcd_sub_fiber_t* sf = polar_tls_client(socket, host_cname, rio_h_out);
    lwt_alloc_free(socket);
    return sf;
}

rcd_sub_fiber_t* polar_tls_client_open(rio_in_addr4_t addr, fstr_t host_cname, rio_t** rio_h_out) {
    rio_tcp_ka_t ka = {0};
    return polar_tls_client_open_ka(addr, host_cname, ka, rio_h_out);
}
