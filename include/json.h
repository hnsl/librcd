#ifndef JSON_H
#define JSON_H

#include "rcd.h"

#define json_null_v ((json_value_t){.type = JSON_NULL})
#define json_bool_v(x)   ((json_value_t){.type = JSON_BOOL,   .bool_value   = x})
#define json_number_v(x) ((json_value_t){.type = JSON_NUMBER, .number_value = x})
#define json_string_v(x) ((json_value_t){.type = JSON_STRING, .string_value = x})
#define json_array_v(x)  ((json_value_t){.type = JSON_ARRAY,  .array_value  = x})
#define json_object_v(x) ((json_value_t){.type = JSON_OBJECT, .object_value = x})

/// Enter a scope with 'this' assigned to a particular JSON object value. This can be
/// be helpful in making creation of JSON structures feel more natural.
#define json_for_obj(obj) \
    for (uint8_t __i = 0; __i == 0;) \
    for (json_value_t this = (obj); (__i++) == 0;)

/// Create a new object as a property of 'this', and enter a scope with it set as 'this'.
/// Example usage:
///
/// json_value_t val = json_new_object();
/// json_for_obj(val) {
///     json_for_new_obj("property") {
///         JSON_SET(this, "leaf", json_string_v("value"));
///     }
/// }
#define json_for_new_obj(key) \
    for (uint8_t __i = 0; __i == 0;) \
    for (json_value_t new_obj = json_new_object_in(this, (key)); __i == 0;) \
    for (json_value_t this = new_obj; (__i++) == 0;)

/// Traverse a chain of JSON properties in a lenient manner, returning a null JSON value
/// if any link in the chain does not exist. Example usage:
///
/// json_tree_t* tree = json_parse("{\"a\": {\"b\": 1}}");
/// json_value_t val = JSON_LREF(tree->value, "a", "b");
/// if (!json_is_null(val)) {
///     do_something(json_get_number(val));
/// }
#define JSON_LREF(value, ...) ({ \
    json_value_t __value = value; \
    fstr_t __path[] = {__VA_ARGS__}; \
    for (int64_t __i = 0; __i < LENGTHOF(__path); __i++) { \
        json_value_t* __next_value = (__value.type == JSON_OBJECT? \
            dict_read(__value.object_value, json_value_t, __path[__i]): 0); \
        __value = (__next_value == 0? json_null_v: *__next_value); \
    } \
    __value; \
})

/// Traverse a chain of JSON properties in a strict manner, throwing an exception if some
/// property traversed does not exist or is null.
#define JSON_REF(value, ...) ({ \
    json_value_t __value = value; \
    fstr_t __path[] = {__VA_ARGS__}; \
    for (int64_t __i = 0; __i < LENGTHOF(__path); __i++) { \
        json_value_t* __next_value = (__value.type == JSON_OBJECT? \
            dict_read(__value.object_value, json_value_t, __path[__i]): 0); \
        __value = (__next_value == 0? json_null_v: *__next_value); \
        if (json_is_null(__value)) \
            json_fail_missing_property(__path[__i]); \
    } \
    __value; \
})

/// Set a property of a JSON object to some value. Example usage:
///
/// json_value_t obj = json_new_object();
/// JSON_SET(obj, "property", json_string_v("value"));
#define JSON_SET(parent, prop, value) ({ \
    assert(parent.type == JSON_OBJECT); \
    dict_replace(parent.object_value, json_value_t, prop, value); \
})

/// JSON type identifier.
typedef enum json_type {
    JSON_NULL = 0,
    JSON_BOOL = 1,
    JSON_NUMBER = 2,
    JSON_STRING = 3,
    JSON_ARRAY = 4,
    JSON_OBJECT = 5,
} json_type_t;

typedef struct json_value {
    json_type_t type;
    union {
        bool bool_value;
        double number_value;
        fstr_t string_value;
        list(json_value_t)* array_value;
        dict(json_value_t)* object_value;
    };
} json_value_t;

/// A JSON value, together with a heap which owns all values and strings contained
/// in that value.
typedef struct json_tree {
    json_value_t value;
    lwt_heap_t* heap;
} json_tree_t;

define_eio(json_parse);

typedef struct {
    /// The property name that was missing.
    fstr_t key;
} json_lookup_eio_t;
define_eio_complex(json_lookup, key);

typedef struct {
    /// The expected type.
    json_type_t expected;
    /// The actual type.
    json_type_t got;
} json_type_eio_t;
define_eio_complex(json_type, expected, got);

/// Parse a string into a json_tree_t. Throws exception_io on failure.
json_tree_t* json_parse(fstr_t str);

/// Stringify a JSON tree structure.
fstr_mem_t* json_stringify(json_value_t value);

/// Stringify a JSON tree structure in a human-readable manner.
fstr_mem_t* json_stringify_pretty(json_value_t value);

/// Serializes a JSON type to a human readable string.
fstr_t json_serial_type(json_type_t type);

void json_fail_invalid_type(json_type_t expected_type, json_type_t got_type);

void json_fail_missing_property(fstr_t prop_name);

static inline fstr_t __attribute__((overloadable)) STR(json_value_t x) { return fss(json_stringify(x)); }
static inline fstr_t __attribute__((overloadable)) STR(json_type_t x) { return json_serial_type(x); }

inline json_value_t json_new_object() {
    return json_object_v(new_dict(json_value_t));
}

/// Create a new object and assign it as a property of another object.
inline json_value_t json_new_object_in(json_value_t parent, fstr_t key) {
    json_value_t obj = json_new_object();
    JSON_SET(parent, key, obj);
    return obj;
}

inline void json_type_expect(json_value_t value, json_type_t expected_type) {
    if (value.type != expected_type)
        json_fail_invalid_type(expected_type, value.type);
}

/// Returns a number from a JSON value, throwing exception_io if the type is wrong.
inline double json_get_number(json_value_t value) {
    json_type_expect(value, JSON_NUMBER);
    return value.number_value;
}

/// Returns a string from a JSON value, throwing exception_io if the type is wrong.
inline fstr_t json_get_string(json_value_t value) {
    json_type_expect(value, JSON_STRING);
    return value.string_value;
}

/// Returns a boolean from a JSON value, throwing exception_io if the type is wrong.
inline bool json_get_bool(json_value_t value) {
    json_type_expect(value, JSON_BOOL);
    return value.bool_value;
}

/// Returns an list (array) from a JSON value, throwing exception_io if the type is wrong.
inline list(json_value_t)* json_get_array(json_value_t value) {
    json_type_expect(value, JSON_ARRAY);
    return value.array_value;
}

/// Returns a dict (object) from a JSON value, throwing exception_io if the type is wrong.
inline dict(json_value_t)* json_get_object(json_value_t value) {
    json_type_expect(value, JSON_OBJECT);
    return value.object_value;
}

inline bool json_is_null(json_value_t value) {
    return value.type == JSON_NULL;
}

#endif  /* JSON_H */
