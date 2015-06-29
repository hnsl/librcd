#include "rest.h"
#include "polarssl/sha1.h"

#pragma librcd

#define HTTP_MIN_ACCEPTED_HEADER_SIZE (0x1500)
#define HTTP_MAX_STATUS_LINE (0x1000)
#define HTTP_MAX_CHUNK_DEF_LINE (0x100)
#define HTTP_CHUNK_STREAM_BUFFER (128 * 1024 * 1024)

static const fstr_t crlf = "\r\n";

fstr_mem_t* rest_basic_auth_val(fstr_t username, fstr_t password) { sub_heap {
    return escape(conc("Basic ", fss(fstr_base64_encode(concs(username, ":", password)))));
}}

fstr_mem_t* rest_urlencode(fstr_t str, bool plus_enc_sp) {
    return rest_urlencodec(str, plus_enc_sp, false);
}

fstr_mem_t* rest_urlencodec(fstr_t str, bool plus_enc_sp, bool ucase) {
    fstr_mem_t* buf = fstr_alloc(str.len * 3);
    fstr_t buf_tail = fss(buf);
    for (size_t i = 0; i < str.len; i++) {
        uint8_t ch = str.str[i];
        if ((ch >= '0' && ch <= '9')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= 'a' && ch <= 'z')
        || (ch == '-')
        || (ch == '.')
        || (ch == '_')
        || (ch == '~')) {
            // Copy over raw character.
            fstr_putc(&buf_tail, ch);
        } else if (plus_enc_sp && ch == ' ') {
            // Plus encode space.
            fstr_putc(&buf_tail, '+');
        } else {
            // Hexencode the character.
            fstr_putc(&buf_tail, '%');
            fstr_t dst = fstr_slice(buf_tail, 0, 2);
            fstr_serial_uint(dst, ch, 16);
            if (ucase)
                fstr_toupper(dst);
            buf_tail = fstr_slice(buf_tail, 2, -1);
        }
    }
    // Throw away the unwritten length of the buffer and return it.
    buf->len = fstr_detail(fss(buf), buf_tail).len;
    return buf;
}

static inline bool is_hex(uint8_t ch) {
    return
        ('0' <= ch && ch <= '9') ||
        ('A' <= ch && ch <= 'F') ||
        ('a' <= ch && ch <= 'f');
}

static inline uint32_t hex_to_int(uint8_t ch) {
    if ('0' <= ch && ch <= '9') return ch - '0';
    if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
    if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
    unreachable();
}

fstr_mem_t* rest_urldecode(fstr_t str, bool plus_dec_sp) {
    fstr_mem_t* out = fstr_alloc(str.len);
    size_t out_i = 0;
    for (size_t i = 0; i < str.len; i++) {
        uint8_t ch;
        if (str.str[i] == '%' && i + 2 < str.len && is_hex(str.str[i + 1]) && is_hex(str.str[i + 2])) {
            ch = hex_to_int(str.str[i + 1]) * 16 + hex_to_int(str.str[i + 2]);
            i += 2;
        } else if (plus_dec_sp && str.str[i] == '+') {
            ch = ' ';
        } else {
            ch = str.str[i];
        }
        out->str[out_i] = ch;
        out_i++;
    }
    out->len = out_i;
    return out;
}

fstr_mem_t* rest_url_query_encode(dict(fstr_t)* url_params) {
    if (dict_count(url_params, fstr_t) == 0)
        return fstr_cpy("");
    sub_heap {
        list(fstr_t)* parts = new_list(fstr_t);
        bool first = true;
        dict_foreach(url_params, fstr_t, key, value) {
            if (first) {
                first = false;
            } else {
                list_push_end(parts, fstr_t, "&");
            }
            list_push_end(parts, fstr_t, fss(rest_urlencode(key, false)));
            list_push_end(parts, fstr_t, "=");
            list_push_end(parts, fstr_t, fss(rest_urlencode(value, false)));
        }
        return escape(fstr_implode(parts, ""));
    }
}

dict(fstr_t)* rest_url_query_decode(fstr_t url_query) {
    dict(fstr_t)* url_params = new_dict(fstr_t);
    for (fstr_t param; fstr_iterate_trim(&url_query, "&", &param);) {
        fstr_t enc_key, enc_value;
        if (!fstr_divide(param, "=", &enc_key, &enc_value)) {
            enc_key = param;
            enc_value = "";
        }
        fstr_t key = fss(rest_urldecode(enc_key, true));
        fstr_t value = fss(rest_urldecode(enc_value, true));
        dict_replace(url_params, fstr_t, key, value);
    }
    return url_params;
}

static fstr_mem_t* header_line(fstr_t key, fstr_t val) {
    return conc(key, ": ", val, crlf);
}

static void parse_headers(fstr_t headers, dict(fstr_t)* headers_io, lwt_heap_t* heap) {
    fstr_t header_line;
    while (fstr_iterate(&headers, crlf, &header_line)) {
        fstr_t header_key, header_val;
        if (!fstr_divide(header_line, ":", &header_key, &header_val))
            throw_eio("malformed header", rest);
        fstr_t header_key_lc;
        switch_heap(heap) {
            header_key_lc = fss(fstr_lower(header_key));
            (void) dict_insert(headers_io, fstr_t, header_key_lc, fsc(fstr_trim(header_val)));
        }
    }
}

void rest_write_request(rio_t* rio_h, rest_request_t request) { sub_heap {
    list(fstr_t)* req_lines = new_list(fstr_t);
    list_push_end(req_lines, fstr_t, concs(request.method, " ", request.path, " HTTP/1.1\r\n"));
    bool host_set = false;
    if (request.headers != 0) {
        dict_foreach(request.headers, fstr_t, key, val) {
            if (fstr_equal_case(key, "host"))
                host_set = true;
            list_push_end(req_lines, fstr_t, fss(header_line(key, val)));
        }
    }
    if (!host_set)
        list_push_end(req_lines, fstr_t, fss(header_line("host", request.host)));
    if (request.body.len > 0)
        list_push_end(req_lines, fstr_t, fss(header_line("content-length", ui2fs(request.body.len))));
    list_push_end(req_lines, fstr_t, crlf);
    rio_write(rio_h, fss(fstr_implode(req_lines, "")));
    rio_write(rio_h, request.body);
}}

rest_head_t rest_read_head(rio_t* rio_r) { sub_heap_txn(heap) {
    rest_head_t resp;
    fstr_mem_t* status_line_buffer = fstr_alloc_buffer(HTTP_MAX_STATUS_LINE);
    fstr_t status_line = rio_read_to_separator(rio_r, crlf, fss(status_line_buffer));
    fstr_t protocol, statuscode;
    if (!fstr_iterate(&status_line, " ", &protocol) || !fstr_iterate(&status_line, " ", &statuscode))
        throw_eio("invalid status line", rest);
    fstr_t reason_phrase = status_line;
    if (!fstr_equal(protocol, "HTTP/1.1"))
        throw_eio("server not http 1.1", rest);
    resp.response_code = fs2ui(statuscode);
    switch_heap(heap) {
        resp.reason_phrase = fsc(reason_phrase);
    }
    switch_heap(heap) {
        resp.headers = new_dict(fstr_t);
    }
    fstr_mem_t* header_buf = fstr_alloc_buffer(HTTP_MIN_ACCEPTED_HEADER_SIZE);
    fstr_t headers = rio_read_to_separator(rio_r, "\r\n\r\n", fss(header_buf));
    parse_headers(headers, resp.headers, heap);
    return resp;
}}

static vstr_t* read_body(rio_t* rio_r, rio_t* rio_w, rest_head_t head, size_t limit_size) { sub_heap_txn(heap) {
    bool has_chunked = false;
    bool has_content_length = false;
    size_t content_length;
    fstr_t* cl_ptr = dict_read(head.headers, fstr_t, "content-length");
    if (cl_ptr != 0) {
        has_content_length = true;
        content_length = fs2ui(*cl_ptr);
    }
    fstr_t* te_ptr = dict_read(head.headers, fstr_t, "transfer-encoding");
    if ((te_ptr != 0) && fstr_equal("chunked", *te_ptr)) {
        has_chunked = true;
    }
    vstr_t* body = 0;
    if (rio_w == 0) {
        body = vstr_new();
        if (limit_size > 0) {
            vstr_limit_set(body, true, limit_size);
        }
    }
    if (has_chunked) {
        // chunk-def is "chunk-size [ chunk-extension ] CRLF"
        fstr_mem_t* chunk_def_buffer = fstr_alloc_buffer(HTTP_MAX_CHUNK_DEF_LINE);
        fstr_t crlf_buf = fstr_slice(fss(chunk_def_buffer), 0, crlf.len);
        for (;;) {
            // Read next chunk size.
            fstr_t chunk_size_str = rio_read_to_separator(rio_r, crlf, fss(chunk_def_buffer));
            // There might be chunk-extensions, they will be ignored.
            fstr_divide(chunk_size_str, ";", &chunk_size_str, 0);
            size_t c_size = fstr_to_uint(chunk_size_str, 16);
            if (c_size == 0) {
                // End reached, read trailing headers.
                fstr_mem_t* header_buf = fstr_alloc_buffer(HTTP_MIN_ACCEPTED_HEADER_SIZE);
                fstr_t trail_headers = fstr_trim(rio_read_to_separator(rio_r, crlf, fss(header_buf)));
                if (trail_headers.len > 0) {
                    switch_heap (heap) {
                        import_list(header_buf);
                        parse_headers(trail_headers, head.headers, heap);
                    }
                }
                // Parsing chunked body complete.
                break;
            }
            if (rio_w == 0) {
                // Read chunk into buffer.
                rio_read_fill(rio_r, vstr_extend(body, c_size));
            } else {
                // Forward chunk to output.
                rio_forward(rio_r, rio_w, c_size);
            }
            // Read trailing crlf.
            rio_read_fill(rio_r, crlf_buf);
            if (!fstr_equal(crlf_buf, crlf))
                throw_eio("invalid chunk, bad trailing crlf", rest);
        }
    } else if (has_content_length) {
        // Read entire body.
        if (rio_w == 0) {
            rio_read_fill(rio_r, vstr_extend(body, content_length));
        } else {
            rio_forward(rio_r, rio_w, content_length);
        }
    }
    switch_heap (heap) {
        return import(body);
    }
}}

vstr_t* rest_read_body(rio_t* rio_r, rest_head_t head, size_t limit_size) {
    return read_body(rio_r, 0, head, limit_size);
}

void rest_stream_body(rio_t* rio_r, rio_t* rio_w, rest_head_t head) {
    read_body(rio_r, rio_w, head, 0);
}

static fstr_mem_t* oa10a_uri_host(oa10a_cfg_t* oac) { sub_heap {
    uint16_t port = oac->port;
    bool no_tls = oac->no_tls;
    fstr_t spec_port = (port == 0 || (no_tls && port == 80) || (!no_tls && port == 443))? "": concs(":", oac->port);
    fstr_mem_t* spec_host = conc(oac->host, spec_port);
    fstr_tolower(fss(spec_host));
    return escape(spec_host);
}}

static fstr_mem_t* oa10a_uri_base(oa10a_cfg_t* oac, fstr_t uri_host) { sub_heap {
    fstr_t protocol = oac->no_tls? "http": "https";
    return escape(conc(protocol, "://", uri_host, oac->path));
}}

typedef struct {
    fstr_t key;
    fstr_t value;
} kv_t;

vec(kv_t);

static int32_t cmp_kv(const void* a_ptr, const void* b_ptr) {
    kv_t *a = (void*) a_ptr, *b = (void*) b_ptr;
    int64_t cmp = fstr_cmp_lexical(a->key, b->key);
    if (cmp != 0) {
        return cmp;
    } else {
        return fstr_cmp_lexical(a->value, b->value);
    }
}

/// OAuth 1.0a, rfc5849 3.6. Percent Encoding. Input string is UTF-8.
static inline fstr_t oa10a_penc(fstr_t in) {
    return fss(rest_urlencodec(in, false, true));
}

static void oa10a_param_apec(vec(kv_t)* dst, dict(fstr_t)* src) {
    dict_foreach(src, fstr_t, key, value) {
        kv_t kv = {.key = oa10a_penc(key), .value = oa10a_penc(value)};
        vec_append(dst, kv_t, kv);
    }
}

/// Parameter normalization according to rfc5849 3.4.1.3.2.
static fstr_mem_t* oa10a_normal_req_param(oa10a_cfg_t* oac, dict(fstr_t)* auth_param) { sub_heap {
    // Append all parameters to vector and encode them.
    vec(kv_t)* par = new_vec(kv_t);
    oa10a_param_apec(par, oac->params);
    oa10a_param_apec(par, auth_param);
    // Sort the parameters.
    size_t n_par = vec_count(par, kv_t);
    kv_t* a_par = vec_array(par, kv_t);
    sort(a_par, n_par, sizeof(*a_par), cmp_kv, 0);
    // Concatenate the parameters.
    vec(fstr_t)* strv = new_vec(fstr_t);
    vec_foreach(par, kv_t, i, kv) {
        if (i > 0) {
            vec_append(strv, fstr_t, "&");
        }
        vec_append(strv, fstr_t, kv.key);
        vec_append(strv, fstr_t, "=");
        vec_append(strv, fstr_t, kv.value);
    }
    return escape(fstr_concatv(strv, ""));
}}

rest_request_t rest_oa10a_req(oa10a_cfg_t* oac) {
    assert(oac->path.len > 0);
    fstr_t ucmethod = fss(fstr_upper(oac->method));
    fstr_t uri_host = fss(oa10a_uri_host(oac));
    fstr_t uri_base = fss(oa10a_uri_base(oac, uri_host));
    // Build auth parameter list.
    fstr_t timestamp = str(rio_epoch_ns_now() / RIO_NS_SEC);
    fstr_t raw_nonce = fss(fstr_alloc(16));
    polar_secure_drbg_fill(raw_nonce);
    fstr_t nonce = fss(fstr_hexencode(raw_nonce));
    dict(fstr_t)* auth_param = new_dict(fstr_t,
        {"oauth_consumer_key", oac->consumer_key},
        {"oauth_token", oac->token},
        {"oauth_signature_method", "HMAC-SHA1"},
        {"oauth_timestamp", timestamp},
        {"oauth_nonce", nonce},
        {"oauth_version", "1.0"},
    );
    fstr_t normal_req_param = fss(oa10a_normal_req_param(oac, auth_param));
    //x-dbg/ DBGFN("ucmethod:", ucmethod);
    //x-dbg/ DBGFN("uri_base:", uri_base);
    //x-dbg/ DBGFN("normal_req_param:", normal_req_param);
    fstr_t sig_base = concs(
        oa10a_penc(ucmethod),
        "&",
        oa10a_penc(uri_base),
        "&",
        oa10a_penc(normal_req_param)
    );
    // Generate hmac-sha1 signature using secrets as key.
    fstr_t secret_key = concs(
        oa10a_penc(oac->consumer_secret),
        "&",
        oa10a_penc(oac->token_secret)
    );
    //x-dbg/ DBGFN("sig_base:", sig_base);
    //x-dbg/ DBGFN("secret_key:", secret_key);
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_hmac_starts(&ctx, secret_key.str, secret_key.len);
    sha1_hmac_update(&ctx, sig_base.str, sig_base.len);
    uint8_t raw_sig[20];
    sha1_hmac_finish(&ctx, raw_sig);
    fstr_t raw_sig_fs = {.str = raw_sig, .len = sizeof(raw_sig)};
    fstr_t signature = fss(fstr_base64_encode(raw_sig_fs));
    dict_inserta(auth_param, fstr_t, "oauth_signature", signature);
    // Build authorization header.
    vec(fstr_t)* authv = new_vec(fstr_t, "OAuth ");
    bool afirst = true;
    dict_foreach(auth_param, fstr_t, akey, aval) {
        if (!afirst)
            vec_append(authv, fstr_t, ", ");
        afirst = false;
        vec_append_n(authv, fstr_t, akey, "=\"", oa10a_penc(aval), "\"");
    }
    // Compile rest request.
    rest_request_t req = {
        .path = oac->path,
        .method = ucmethod,
        .host = oac->host,
        .headers = new_dict(fstr_t,
            {"host", uri_host},
            {"authorization", fss(fstr_concatv(authv, ""))},
        ),
    };
    if (dict_count(oac->params, fstr_t) > 0) {
        fstr_t urlenc_params = fss(rest_url_query_encode(oac->params));
        if (fstr_equal(ucmethod, "POST")) {
            dict_inserta(req.headers, fstr_t, "content-type", "application/x-www-form-urlencoded");
            req.body = urlenc_params;
        } else {
            req.path = concs(req.path, "?", urlenc_params);
        }
    }
    return req;
}

rio_t* rest_oa10a_exec(oa10a_cfg_t* oac) {
    rest_request_t req = rest_oa10a_req(oac);
    rio_in_addr4_t addr = {
        .address = list_peek_start(rio_resolve_host_ipv4_addr(oac->host), uint32_t),
        .port = (oac->port == 0? (oac->no_tls? 80: 443): oac->port)
    };
    rio_t* sock_h;
    if (oac->no_tls) {
        sock_h = rio_tcp_client(addr);
    } else {
        // Start tls client, leak sub fiber to parent.
        polar_tls_client_open(addr, oac->host, &sock_h);
    }
    // Write request.
    rest_write_request(sock_h, req);
    // Return socket.
    return sock_h;
}
