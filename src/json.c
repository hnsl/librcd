#include "rcd.h"
#include "utf8proc.h"
#include "json.h"

#pragma librcd

typedef struct {
    fstr_t data;
    unsigned int pos;
} parser_t;

static inline void fail() {
    throw("invalid json", exception_io);
}

static inline void assume(bool b) {
    if (!b) fail();
}

static inline bool remaining(parser_t* parser) {
    return (parser->pos < parser->data.len);
}

static inline uint8_t peek(parser_t* parser) {
    assume(remaining(parser));
    return parser->data.str[parser->pos];
}

static inline uint8_t consume(parser_t* parser) {
    uint8_t ch = peek(parser);
    parser->pos++;
    return ch;
}

static inline bool is_ws(uint8_t ch) {
    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t');
}

static inline void skip_ws(parser_t* parser) {
    while (remaining(parser) && is_ws(peek(parser)))
        consume(parser);
}

static void validate_number(fstr_t str) {
#pragma re2c(str): ^ -? (0|[1-9][0-9]*) (\.[0-9]+)? ([eE][-\+]?[0-9]+)? $ {@match}
    fail();
match:;
}

static inline bool is_primitive_char(uint8_t c) {
    switch (c) {
        case '\t': case '\r': case '\n': case ' ':
        case ',': case ']': case '}':
            return false;
        default:
            return true;
    }
}

static json_value_t parse_primitive(parser_t* parser) {
    size_t start = parser->pos;
    while (remaining(parser) && is_primitive_char(peek(parser)))
        consume(parser);
    fstr_t str = fstr_slice(parser->data, start, parser->pos);
    assume(str.len > 0);
    uint8_t c = str.str[0];
    json_value_t ret;
    if (c == '-' || ('0' <= c && c <= '9')) {
        validate_number(str);
        ret.type = JSON_NUMBER;
        ret.number_value = fstr_to_double(str);
    } else if (fstr_equal(str, "true")) {
        ret.type = JSON_BOOL;
        ret.bool_value = true;
    } else if (fstr_equal(str, "false")) {
        ret.type = JSON_BOOL;
        ret.bool_value = false;
    } else if (fstr_equal(str, "null")) {
        ret.type = JSON_NULL;
    } else {
        fail();
    }
    return ret;
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

static fstr_mem_t* decode(fstr_t encoded) {
    // The pre-parse done by parse_string guarantees that this will never read
    // out of bounds. Because no escape sequence is shorter than what it escapes,
    // we won't write out of bounds, either.
    fstr_mem_t* ret = fstr_alloc(encoded.len);
    uint8_t *enc = encoded.str, *out = ret->str;
    size_t index = 0, out_index = 0;
    while (index < encoded.len) {
        uint8_t c1 = enc[index];
        if (c1 != '\\') {
            out[out_index] = c1;
            out_index++;
            index++;
            continue;
        }
        uint8_t c2 = enc[index+1];
        index += 2;
        if (c2 == '\\') out[out_index++] = '\\';
        else if (c2 == '"') out[out_index++] = '"';
        else if (c2 == '/') out[out_index++] = '/';
        else if (c2 == 'b') out[out_index++] = '\b';
        else if (c2 == 'f') out[out_index++] = '\f';
        else if (c2 == 't') out[out_index++] = '\t';
        else if (c2 == 'r') out[out_index++] = '\r';
        else if (c2 == 'n') out[out_index++] = '\n';
        else {
            // Escape sequence of the form \uXXXX. Encode it into UTF-8.
            assert(c2 == 'u');
            int32_t ch = 0;
            for (int i = 0; i < 4; i++)
                ch += hex_to_int(enc[index + 3 - i]) << (i * 4);
            index += 4;
            if (0xd800 <= ch && ch < 0xdbff && index < encoded.len &&
                    enc[index] == '\\' && enc[index+1] == 'u') {
                // Potentially UTF-16
                int32_t ch2 = 0;
                for (int i = 0; i < 4; i++)
                    ch2 += hex_to_int(enc[index + 2 + 3 - i]) << (i * 4);
                if (0xdc00 <= ch2 && ch2 < 0xdfff) {
                    int32_t lead = ch - 0xd800;
                    int32_t trail = ch2 - 0xdc00;
                    ch = (lead << 10) + trail + 0x10000;
                    index += 6;
                }
            }
            out_index += utf8proc_encode_char(ch, &out[out_index]);
        }
    }
    ret->len = out_index;
    return ret;
}

static json_value_t parse_value(parser_t* parser);

static json_value_t parse_string(parser_t *parser) {
    size_t start = parser->pos;
    assume(consume(parser) == '"');

    bool uses_escapes = false;
    for (;;) {
        uint8_t c = consume(parser);

        // Quote: end of string
        if (c == '\"') {
            json_value_t ret;
            ret.type = JSON_STRING;
            fstr_t str = fstr_slice(parser->data, start + 1, parser->pos - 1);
            ret.string_value = fss(uses_escapes? decode(str): fstr_cpy(str));
            return ret;
        }

        // Backslash: Quoted symbol expected
        if (c == '\\') {
            switch (consume(parser)) {
                // Named escape symbols
                case '\"': case '/': case 'b': case 'f':
                case '\\': case 'r': case 'n': case 't':
                    uses_escapes = true;
                    break;
                // Escape symbols on the form \uXXXX
                case 'u':
                    for (int i = 0; i < 4; i++) {
                        assume(is_hex(consume(parser)));
                    }
                    uses_escapes = true;
                    break;
                // Unexpected symbol
                default:
                    fail();
            }
        }

        // White-space is disallowed within strings
        if (c == '\t' || c == '\n' || c == '\r')
            fail();
    }
}

static json_value_t parse_array(parser_t* parser) {
    list(json_value_t)* list = new_list(json_value_t);
    size_t start = parser->pos;
    assume(consume(parser) == '[');
    bool expect_comma = false;
    for (;;) {
        skip_ws(parser);
        if (peek(parser) == ']')
            break;
        if (expect_comma) {
            assume(consume(parser) == ',');
        }
        expect_comma = true;
        json_value_t val = parse_value(parser);
        list_push_end(list, json_value_t, val);
    }
    consume(parser);
    json_value_t ret;
    ret.type = JSON_ARRAY;
    ret.array_value = list;
    return ret;
}

static json_value_t parse_object(parser_t* parser) {
    dict(json_value_t)* dict = new_dict(json_value_t);
    size_t start = parser->pos;
    assume(consume(parser) == '{');
    bool expect_comma = false;
    for (;;) {
        skip_ws(parser);
        if (peek(parser) == '}')
            break;
        if (expect_comma) {
            assume(consume(parser) == ',');
            skip_ws(parser);
        }
        expect_comma = true;
        json_value_t key_value = parse_string(parser);
        fstr_t key = key_value.string_value;
        skip_ws(parser);
        assume(consume(parser) == ':');
        json_value_t val = parse_value(parser);
        dict_replace(dict, json_value_t, key, val);
    }
    consume(parser);
    json_value_t ret;
    ret.type = JSON_OBJECT;
    ret.object_value = dict;
    return ret;
}

static json_value_t parse_value(parser_t* parser) {
    skip_ws(parser);
    switch (peek(parser)) {
        case '"':
            return parse_string(parser);
        case '[':
            return parse_array(parser);
        case '{':
            return parse_object(parser);
        default:
            return parse_primitive(parser);
    }
}

json_tree_t* json_parse(fstr_t str) { sub_heap {
    lwt_heap_t* heap = lwt_alloc_heap();
    switch_heap (heap) {
        json_tree_t* tree = new(json_tree_t);
        tree->heap = heap;
        parser_t parser = {
            .data = str,
            .pos = 0,
        };
        tree->value = parse_value(&parser);
        skip_ws(&parser);
        assume(!remaining(&parser));
        escape_list(heap);
        return tree;
    }
}}

static void stringify_value(json_value_t value, list(fstr_t)* parts);

static void stringify_string(fstr_t value, list(fstr_t)* parts) {
    list_push_end(parts, fstr_t, "\"");
    size_t start = 0;
    for (size_t i = 0; i < value.len; i++) {
        fstr_t part;
        switch (value.str[i]) {
            case '\b': part = "\\b"; break;
            case '\f': part = "\\f"; break;
            case '\t': part = "\\t"; break;
            case '\r': part = "\\r"; break;
            case '\n': part = "\\n"; break;
            case '\\': part = "\\\\"; break;
            case '\"': part = "\\\""; break;
            default: goto next;
        }
        if (start != i)
            list_push_end(parts, fstr_t, fstr_slice(value, start, i));
        list_push_end(parts, fstr_t, part);
        start = i + 1;
next:;
    }
    if (start != value.len)
        list_push_end(parts, fstr_t, fstr_slice(value, start, value.len));
    list_push_end(parts, fstr_t, "\"");
}

static void stringify_array(list(json_value_t)* array, list(fstr_t)* parts) {
    list_push_end(parts, fstr_t, "[");
    bool first = true;
    list_foreach(array, json_value_t, value) {
        if (!first)
            list_push_end(parts, fstr_t, ",");
        first = false;
        stringify_value(value, parts);
    }
    list_push_end(parts, fstr_t, "]");
}

static void stringify_object(dict(json_value_t)* object, list(fstr_t)* parts) {
    list_push_end(parts, fstr_t, "{");
    bool first = true;
    dict_foreach(object, json_value_t, key, value) {
        if (!first)
            list_push_end(parts, fstr_t, ",");
        first = false;
        stringify_string(key, parts);
        list_push_end(parts, fstr_t, ":");
        stringify_value(value, parts);
    }
    list_push_end(parts, fstr_t, "}");
}

static void stringify_value(json_value_t value, list(fstr_t)* parts) {
    switch (value.type) {
        case JSON_NULL:
            list_push_end(parts, fstr_t, "null");
            break;
        case JSON_BOOL:
            if (value.bool_value == true)
                list_push_end(parts, fstr_t, "true");
            else
                list_push_end(parts, fstr_t, "false");
            break;
        case JSON_NUMBER:
            list_push_end(parts, fstr_t, fss(fstr_from_double(value.number_value)));
            break;
        case JSON_STRING:
            stringify_string(value.string_value, parts);
            break;
        case JSON_ARRAY:
            stringify_array(value.array_value, parts);
            break;
        case JSON_OBJECT:
            stringify_object(value.object_value, parts);
            break;
    }
}

fstr_mem_t* json_stringify(json_value_t value) { sub_heap {
    list(fstr_t)* parts = new_list(fstr_t);
    stringify_value(value, parts);
    return escape(fstr_implode(parts, ""));
}}

void json_fail_missing_property(fstr_t prop_name) {
    sub_heap_e(throw(concs("missing JSON property: ", prop_name), exception_io));
}
