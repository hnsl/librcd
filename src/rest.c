#include "rest.h"

#pragma librcd

#define HTTP_MIN_ACCEPTED_HEADER_SIZE (0x1500)
#define HTTP_MAX_STATUS_LINE (0x1000)

static const fstr_t crlf = "\r\n";

fstr_mem_t* rest_basic_auth_val(fstr_t username, fstr_t password) { sub_heap {
    return escape(conc("Basic ", fss(fstr_base64_encode(concs(username, ":", password)))));
}}

fstr_mem_t* rest_urlencode(fstr_t str, bool plus_enc_sp) {
    fstr_mem_t* buf = fstr_alloc(str.len * 3);
    fstr_t buf_tail = fss(buf);
    for (size_t i = 0; i < str.len; i++) {
        uint8_t ch = str.str[i];
        if ((ch >= 0 && ch <= 9)
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
            fstr_serial_uint(fstr_slice(buf_tail, 0, 2), ch, 16);
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

list(fstr_t)* rest_serialize_request(rest_request_t request){
    list(fstr_t)* req_lines = new_list(fstr_t);
    list_push_end(req_lines, fstr_t, sconc(request.method, " ", request.path, " HTTP/1.1\r\n"));
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
    list_push_end(req_lines, fstr_t, request.body);
    return req_lines;
}

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
    resp.response_code = fs2ui(status_line);
    switch_heap(heap) {
        resp.reason_phrase = fsc(reason_phrase);
    }
    fstr_mem_t* header_buf = fstr_alloc_buffer(HTTP_MIN_ACCEPTED_HEADER_SIZE);
    fstr_t headers = rio_read_to_separator(rio_r, "\r\n\r\n", fss(header_buf));
    fstr_t header_line;
    switch_heap(heap) {
        resp.headers = new_dict(fstr_t);
    }
    while(fstr_iterate(&headers, crlf, &header_line)) {
        fstr_t header_key, header_val;
        if (!fstr_divide(header_line, ":", &header_key, &header_val))
            throw_eio("malformed header", rest);
        fstr_t header_key_lc;
        switch_heap(heap) {
            header_key_lc = fss(fstr_lower(header_key));
            dict_replace(resp.headers, fstr_t, header_key_lc, fstr_trim(header_val));
        }
    }
    return resp;
}}

fstr_mem_t* rest_read_body(rio_t* rio_r, rest_head_t head, size_t max_size){ sub_heap_txn(heap){
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
    fstr_mem_t* body_buffer = 0;
    if (has_chunked) {
        switch_heap(heap) {
            body_buffer = fstr_alloc(max_size);
        }
        size_t body_len = 0;
        fstr_t body_buffer_tail = fss(body_buffer);
        for (;;) {
            fstr_t chunk_size_str = rio_read_to_separator(rio_r, crlf, body_buffer_tail);
            size_t c_size = chunk_size_str.len == 0? 0: fstr_to_uint(chunk_size_str, 16);
            if (c_size == 0) {
                fstr_t end = fstr_slice(body_buffer_tail, 0, crlf.len);
                rio_read_fill(rio_r, end);
                if (!fstr_equal(end, crlf))
                    throw_eio("invalid HTTP response", rest);
                break;
            }
            if ((c_size + crlf.len) > body_buffer_tail.len)
                throw_eio("too large response", rest);
            rio_read_fill(rio_r, fstr_slice(body_buffer_tail, 0, c_size));
            // Keep the body data by moving the tail.
            body_buffer_tail = fstr_slice(body_buffer_tail, c_size, -1);
            fstr_t chunk_break = fstr_slice(body_buffer_tail, 0, crlf.len);
            rio_read_fill(rio_r, chunk_break);
            if (!fstr_equal(chunk_break, crlf))
                throw_eio("invalid chunk", rest);
            body_len += c_size;
        }
        body_buffer->len = body_len;
    } else if (has_content_length) {
        if (content_length > max_size)
            throw_eio("too large response", rest);
        switch_heap(heap) {
            body_buffer = fstr_alloc(content_length);
        }
        rio_read_fill(rio_r, fss(body_buffer));
        body_buffer->len = content_length;
    }
    return body_buffer;
}}
