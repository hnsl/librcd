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

/// Like rest_urlencode() but allows control of the case of the escaped symbols.
fstr_mem_t* rest_urlencodec(fstr_t str, bool plus_enc_sp, bool ucase);

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

void rest_write_request(rio_t* rio_h, rest_request_t request);

/// Read the status line and headers from rio_r, body not touched.
rest_head_t rest_read_head(rio_t* rio_r);

/// Reads a rest body.
fstr_mem_t* rest_read_body(rio_t* rio_r, rest_head_t head, size_t max_size);

/// Streams a chunked rest body and parses the raw data into another rio stream.
/// When the chunked stream ends gracefully (application level) the trailing
/// headers is parsed and returned.
dict(fstr_t)* rest_stream_chunked_body(rio_t* rio_r, rio_t* rio_w);

/// OAuth 1.0a configuration.
typedef struct oa10a_cfg {
    // Method of the HTTP request. Case insensitive.
    // When "POST" is specified, the parameters are added in the body encoded
    // with the application/x-www-form-urlencoded format per the OAuth 1.0a spec.
    fstr_t method;
    // Host of the HTTP request without port. Case insensitive.
    fstr_t host;
    // Optional: Port of the HTTP request. Zero means default.
    uint16_t port;
    // Optional: True to use HTTP instead of HTTPS.
    bool no_tls;
    // Path of the HTTP request. Should NOT contain any query or fragment parts.
    fstr_t path;
    // Parameters of the oauth request. These are added to the request depending on the method type.
    dict(fstr_t)* params;
    // Public ID that identifies the 3rd party application. (app username)
    fstr_t consumer_key;
    // Secret key that authenticates a request made by the 3rd party application. (app password)
    fstr_t consumer_secret;
    // Token that identifies the user. (user username)
    fstr_t token;
    // Token that identifies the user. (user password)
    fstr_t token_secret;
} oa10a_cfg_t;

/// Builds a complete OAuth 1.0a request based on the specified parameters.
/// The request contains a timestamp and will only remain valid for a certain time period.
/// It is assumed the rest request is done over http.
/// @dirty Leaks a lot of memory needed to support the complex rest_request_t into callers heap.
rest_request_t rest_oa10a_req(oa10a_cfg_t* oac);

/// Executes a OAuth 1.0a rest request by opening a socket and writing it.
/// The returned socket can be used with rest_read_head() and rest_read_body().
/// @dirty Leaks the sub fiber started for a tls connection into callers heap.
rio_t* rest_oa10a_exec(oa10a_cfg_t* oac);

#endif	/* REST_H */
