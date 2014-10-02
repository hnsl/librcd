#ifndef REST_H
#define	REST_H

#include "rcd.h"

define_eio(rest);

dict(fstr_t);

typedef struct http_req {
    fstr_t method;
    fstr_t host;
    fstr_t path;
    fstr_t body;
    dict(fstr_t)* headers;
} rest_req_t;

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

fstr_mem_t* rest_serialize_request(rest_req_t request);

/// Read a response using content-length or chunked transfer encoding.
fstr_t rest_read_response(rio_t* rio_r, size_t max_size);

fstr_t rest_call(rio_t* rio_h, rest_req_t request, size_t max_response_size);

#endif	/* REST_H */

