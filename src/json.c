#include "rcd.h"
#include "utf8proc.h"
#include "json.h"

#pragma librcd

#define INDENT_SIZE (2)

typedef struct {
    fstr_t data;
    size_t pos;
} parser_t;

noret static void fail(parser_t* parser, fstr_t message) {
    size_t line = 1, col = 1;
    assert(parser->pos > 0);
    fstr_t data = fstr_slice(parser->data, 0, parser->pos - 1);
    for (size_t i = 0; i < data.len; i++) {
        if (data.str[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    emitosis(json_parse, data) {
        fstr_t emsg = concs("invalid json: ", message, " at line ", line, " column ", col);
        data.message = message;
        data.line = line;
        data.column = col;
        throw_em(emsg, data);
    }
}

static inline bool remaining(parser_t* parser) {
    return (parser->pos < parser->data.len);
}

static inline uint8_t peek(parser_t* parser) {
    if (!remaining(parser)) {
        parser->pos++;
        fail(parser, "unexpected end of data");
    }
    return parser->data.str[parser->pos];
}

static inline uint8_t consume(parser_t* parser) {
    uint8_t ch = peek(parser);
    parser->pos++;
    return ch;
}

static inline void expect(parser_t* parser, uint8_t c, fstr_t message) {
    if (consume(parser) != c)
        fail(parser, message);
}

static inline bool is_ws(uint8_t ch) {
    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t');
}

static inline void skip_ws(parser_t* parser) {
    while (remaining(parser) && is_ws(peek(parser)))
        consume(parser);
}

static bool validate_number(fstr_t str) {
#pragma re2c(str): ^ -? (0|[1-9][0-9]*) (\.[0-9]+)? ([eE][-\+]?[0-9]+)? $ {@match}
    return false;
match:
    return true;
}

static inline bool is_primitive_char(uint8_t c) {
    switch (c) {
        case '\t': case '\r': case '\n': case ' ': case '"':
        case ',': case '[': case '{': case ']': case '}':
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
    uint8_t c = (str.len == 0? '\0': str.str[0]);
    json_value_t ret;
    if (c == '-' || ('0' <= c && c <= '9')) {
        if (!validate_number(str)) {
            parser->pos = start + 1;
            fail(parser, "invalid number literal");
        }
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
        parser->pos = start;
        consume(parser);
        fail(parser, "unexpected character");
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
    consume(parser);
    bool uses_escapes = false;
    for (;;) {
        uint8_t c = consume(parser);
        if (c == '\"')
            break;
        if (c == '\\') {
            uses_escapes = true;
            switch (consume(parser)) {
                // Named escape symbols
                case '\"': case '/': case 'b': case 'f':
                case '\\': case 'r': case 'n': case 't':
                    break;
                // Escape symbols on the form \uXXXX
                case 'u':
                    for (int i = 0; i < 4; i++) {
                        if (!is_hex(consume(parser)))
                            fail(parser, "invalid Unicode escape");
                    }
                    break;
                // Unexpected symbol
                default:
                    fail(parser, "invalid escape character");
            }
        }
        // Control characters are disallowed within strings
        if (c < 0x20)
            fail(parser, "unexpected control character");
    }
    json_value_t ret;
    ret.type = JSON_STRING;
    fstr_t str = fstr_slice(parser->data, start + 1, parser->pos - 1);
    ret.string_value = fss(uses_escapes? decode(str): fstr_cpy(str));
    return ret;
}

static json_value_t parse_array(parser_t* parser) {
    list(json_value_t)* list = new_list(json_value_t);
    size_t start = parser->pos;
    consume(parser);
    bool expect_comma = false;
    for (;;) {
        skip_ws(parser);
        if (peek(parser) == ']')
            break;
        if (expect_comma) {
            expect(parser, ',', "expected ',' or ']' after array element");
            size_t comma_pos = parser->pos;
            skip_ws(parser);
            if (peek(parser) == ']') {
                parser->pos = comma_pos;
                fail(parser, "unexpected trailing comma");
            }
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
    consume(parser);
    bool expect_comma = false;
    for (;;) {
        skip_ws(parser);
        if (peek(parser) == '}')
            break;
        if (expect_comma) {
            expect(parser, ',', "expected ',' or '}' after property value");
            size_t comma_pos = parser->pos;
            skip_ws(parser);
            if (peek(parser) == '}') {
                parser->pos = comma_pos;
                fail(parser, "unexpected trailing comma");
            }
        }
        expect_comma = true;
        if (peek(parser) != '"')
            fail(parser, "expected double-quoted property name");
        json_value_t key_value = parse_string(parser);
        fstr_t key = key_value.string_value;
        skip_ws(parser);
        expect(parser, ':', "expected ':' after property name");
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
        if (remaining(&parser))
            fail(&parser, "unexpected character after JSON data");
        escape_list(heap);
        return tree;
    }
}}

static inline void add_indent(list(fstr_t)* parts, int indent) {
    // To avoid quadratic output size, cap indent to 20 spaces.
    fstr_t ws = fstr_slice("\n                    ", 0, (indent * INDENT_SIZE) + 1);
    list_push_end(parts, fstr_t, ws);
}

static bool should_indent_array(list(json_value_t)* array) {
    list_foreach(array, json_value_t, value) {
        if (value.type == JSON_ARRAY || value.type == JSON_OBJECT)
            return true;
    }
    return false;
}

static void stringify_value(json_value_t value, list(fstr_t)* parts, int indent);

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

static void stringify_array(list(json_value_t)* array, list(fstr_t)* parts, int indent) {
    list_push_end(parts, fstr_t, "[");
    bool first = true;
    bool should_indent = (indent >= 0 && should_indent_array(array));
    fstr_t comma = (indent >= 0 && !should_indent? ", ": ",");
    list_foreach(array, json_value_t, value) {
        if (!first)
            list_push_end(parts, fstr_t, comma);
        first = false;
        if (should_indent)
            add_indent(parts, indent + 1);
        stringify_value(value, parts, indent + 1);
    }
    if (should_indent)
        add_indent(parts, indent);
    list_push_end(parts, fstr_t, "]");
}

static void stringify_object(dict(json_value_t)* object, list(fstr_t)* parts, int indent) {
    list_push_end(parts, fstr_t, "{");
    bool first = true;
    bool should_indent = (indent >= 0);
    dict_foreach(object, json_value_t, key, value) {
        if (!first)
            list_push_end(parts, fstr_t, ",");
        first = false;
        if (should_indent)
            add_indent(parts, indent + 1);
        stringify_string(key, parts);
        list_push_end(parts, fstr_t, should_indent? ": ": ":");
        stringify_value(value, parts, indent + 1);
    }
    if (should_indent)
        add_indent(parts, indent);
    list_push_end(parts, fstr_t, "}");
}

static void stringify_value(json_value_t value, list(fstr_t)* parts, int indent) {
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
            stringify_array(value.array_value, parts, indent);
            break;
        case JSON_OBJECT:
            stringify_object(value.object_value, parts, indent);
            break;
    }
}

fstr_mem_t* json_stringify(json_value_t value) { sub_heap {
    list(fstr_t)* parts = new_list(fstr_t);
    stringify_value(value, parts, INT_MIN);
    return escape(fstr_implode(parts, ""));
}}

fstr_mem_t* json_stringify_pretty(json_value_t value) { sub_heap {
    list(fstr_t)* parts = new_list(fstr_t);
    stringify_value(value, parts, 0);
    return escape(fstr_implode(parts, ""));
}}

fstr_t json_serial_type(json_type_t type) {
    switch (type) {{
    } case JSON_NULL: {
        return "null";
    } case JSON_BOOL: {
        return "bool";
    } case JSON_NUMBER: {
        return "number";
    } case JSON_STRING: {
        return "string";
    } case JSON_ARRAY: {
        return "array";
    } case JSON_OBJECT: {
        return "object";
    }}
}

void _json_fail_invalid_type(json_type_t expected_type, json_type_t got_type) {
    emitosis(json_type, jd) {
        jd.expected = expected_type;
        jd.got = got_type;
        throw_em(concs("invalid json type, expected: ", expected_type, ", got: ", got_type), jd);
    }
}

void _json_fail_missing_property(fstr_t prop_name) {
    emitosis(json_lookup, jd) {
        jd.key = fsc(prop_name);
        throw_em(concs("missing json property: [", prop_name, "]"), jd);
    }
}
