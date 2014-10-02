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

/// Convenience function for application/x-www-form-urlencoded data.
fstr_mem_t* rest_urlencode_dict(dict(fstr_t)* kv);

fstr_mem_t* rest_path_conc_args(fstr_t path, dict(fstr_t)* kv);

fstr_mem_t* rest_serialize_request(rest_req_t request);

/// Read a response using content-length or chunked transfer encoding.
fstr_t rest_read_response(rio_t* rio_r, size_t max_size);

fstr_t rest_call(rio_t* rio_h, rest_req_t request, size_t max_response_size);

#endif	/* REST_H */

