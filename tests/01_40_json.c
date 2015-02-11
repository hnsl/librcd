/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

void rcd_self_test_json() {
    // Create a json object and do some mutations.
    sub_heap {
        json_value_t v = json_null_v;
        try {
            json_value_t v2 = JSON_TOUCH(v, jnum(1), "foo", "bar");
            atest(false);
        } catch_eio (json_type, e) {}
    }
    sub_heap {
        json_value_t v = jobj_new();
        json_value_t v2 = JSON_TOUCH(v, jnum(33), "foo", "bar");
        atest(v2.type == JSON_NUMBER);
        atest(v2.number_value == 33);
        json_value_t v3 = JSON_TOUCH(v, jstr("xyz"), "foo", "bar");
        atest(v3.type == JSON_NUMBER);
        atest(v3.number_value == 33);
        json_value_t v4 = JSON_TOUCH(v, jobj_new({"baz", jstr("biz")}), "foo", "bar");
        atest(v4.type == JSON_NUMBER);
        atest(v4.number_value == 33);
        JSON_REF_SET(v, jobj_new({"baz", jstr("biz")}), "foo", "bar");
        json_value_t v7 = JSON_REF(v, "foo", "bar", "baz");
        atest(json_cmp(v7, jstr("biz")));
        json_value_t v8 = JSON_LREF(v, "foo", "bad", "baz");
        atest(!json_cmp(v8, jstr("biz")));
    }
}
