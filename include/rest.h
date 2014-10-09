#ifndef REST_H
#define	REST_H

#include "rcd.h"

define_eio(rest);

typedef struct rest_request {
    fstr_t method;
    fstr_t host;
    fstr_t path;
    fstr_t body;
    dict(fstr_t)* headers;
} rest_request_t;

typedef struct rest_response {
    uint16_t response_code;
    fstr_t reason_phrase;
    dict(fstr_t)* headers;
} rest_head_t;

/// Construct a correct Basic Auth line.
fstr_mem_t* rest_basic_auth_val(fstr_t username, fstr_t password);

/// URL encodes the string. All characters will be encoded as %xx except
/// alphanums and "-._~" as per rfc 3986. If plus_enc_sp is true space will be
/// encoded as "+". This is deprecated and should be avoided.
fstr_mem_t* rest_urlencode(fstr_t str, bool plus_enc_sp);

/// URL decodes the string as per rfc 3986. If plus_dec_sp is true "+" will be
/// decoded as a space. This is deprecated and should be avoided.
fstr_mem_t* rest_urldecode(fstr_t str, bool plus_dec_sp);

/// Encodes parameters into a url query suitable for appending to a URL after
/// the "?" separator.
fstr_mem_t* rest_url_query_encode(dict(fstr_t)* url_params);

/// Decodes a URL query into it's respective parameters. If there are key
/// collisions in the query it is undefined which key/value pair will be
/// returned.
dict(fstr_t)* rest_url_query_decode(fstr_t url_query);

list(fstr_t)* rest_serialize_request(rest_request_t request);

/// Read the status line and headers from rio_r, body not touched.
rest_head_t rest_read_head(rio_t* rio_r);

fstr_mem_t* rest_read_body(rio_t* rio_r, rest_head_t head, size_t max_size);

#endif	/* REST_H */

