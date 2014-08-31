/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/// TODO: Support specifying a callback function that returns the memory range
/// from the key that the hash should be calculated for (to support dynamic
/// length strings etc).

/// TODO: Support a lookup function that returns the key pointer as well so
/// complex keys can be freed by the caller.

#ifndef HMAP_H
#define	HMAP_H

#include "fstring.h"

/// Good default value for HMAP_N_PEAK_EST in applications that do not wish to fine tune this.
#define HMAP_DFL_N_PEAK_EST (10)

/// Good default value for MAX_FILL_R in applications that do not wish to fine tune this.
#define HMAP_DFL_MAX_FILL_R (0.4f)

/// Defines a new hmap type and interface functions with a complex key that are compared and hashed with a specified callback function.
#define HMAP_DEFINE_ABSTRACT_TYPE(NAME, KEY_TYPE, KEY_HASH_FN, KEY_CMP_FN, VALUE_TYPE, USE_HEAP, N_PEAK_EST, MAX_FILL_R, SECURE) \
static const hmap_cfg_t hmap_##NAME##_cfg = {                                           \
    .key_len = sizeof(KEY_TYPE),                                                        \
    .key_hash_fn = KEY_HASH_FN,                                                         \
    .key_cmp_fn = KEY_CMP_FN,                                                           \
    .value_len = sizeof(VALUE_TYPE),                                                    \
    .use_heap = USE_HEAP,                                                               \
    .n_peak_est = N_PEAK_EST,                                                           \
    .max_fill_r = MAX_FILL_R,                                                           \
    .secure = SECURE,                                                                   \
};                                                                                      \
                                                                                        \
typedef struct hmap_##NAME {                                                            \
    hmap_t hm;                                                                          \
} hmap_##NAME##_t;                                                                      \
                                                                                        \
typedef struct hmap_##NAME##_lookup {                                                   \
    hmap_lookup_t lu;                                                                   \
} hmap_##NAME##_lookup_t;                                                               \
                                                                                        \
static void hmap_##NAME##_init(hmap_##NAME##_t* thm) {                                  \
    hmap_init(&thm->hm, hmap_##NAME##_cfg);                                             \
}                                                                                       \
                                                                                        \
static hmap_##NAME##_lookup_t hmap_##NAME##_lookup(                                     \
hmap_##NAME##_t* thm, KEY_TYPE key, bool may_exist) {                                   \
    return (hmap_##NAME##_lookup_t) {                                                   \
        .lu = hmap_lookup(&thm->hm, hmap_##NAME##_cfg, &key, may_exist),                \
    };                                                                                  \
}                                                                                       \
                                                                                        \
static void hmap_##NAME##_insert(                                                       \
hmap_##NAME##_t* thm, hmap_##NAME##_lookup_t tlu, KEY_TYPE key, VALUE_TYPE value) {     \
    return hmap_insert(&thm->hm, hmap_##NAME##_cfg, tlu.lu, &key, &value);              \
}                                                                                       \
                                                                                        \
static void hmap_##NAME##_delete(                                                       \
hmap_##NAME##_t* thm, hmap_##NAME##_lookup_t tlu) {                                     \
    return hmap_delete(&thm->hm, hmap_##NAME##_cfg, tlu.lu);                            \
}                                                                                       \
                                                                                        \
static void hmap_##NAME##_replace(                                                      \
hmap_##NAME##_t* thm, hmap_##NAME##_lookup_t tlu, VALUE_TYPE value) {                   \
    return hmap_replace(&thm->hm, hmap_##NAME##_cfg, tlu.lu, &value);                   \
}                                                                                       \
                                                                                        \
static void hmap_##NAME##_replace_kv(                                                   \
hmap_##NAME##_t* thm, hmap_##NAME##_lookup_t tlu, KEY_TYPE key, VALUE_TYPE value) {     \
    return hmap_replace_kv(&thm->hm, hmap_##NAME##_cfg, tlu.lu, &key, &value);          \
}                                                                                       \
                                                                                        \
static KEY_TYPE hmap_##NAME##_key(hmap_##NAME##_lookup_t tlu) {                         \
    return *((KEY_TYPE*) &tlu.lu.slot->elem_data[0]);                                   \
}                                                                                       \
                                                                                        \
static VALUE_TYPE hmap_##NAME##_value(hmap_##NAME##_lookup_t tlu) {                     \
    return *((VALUE_TYPE*) &tlu.lu.slot->elem_data[sizeof(KEY_TYPE)]);                  \
}                                                                                       \
                                                                                        \
static bool hmap_##NAME##_found(hmap_##NAME##_lookup_t tlu) {                           \
    return tlu.lu.found_element;                                                        \
}                                                                                       \
                                                                                        \
static size_t hmap_##NAME##_count(hmap_##NAME##_t* thm) {                               \
    return thm->hm.count;                                                               \
}                                                                                       \

/// Defines a new hmap type and interface functions that assume that the key is a primitive range of memory.
#define HMAP_DEFINE_TYPE(NAME, KEY_TYPE, VALUE_TYPE, USE_HEAP, N_PEAK_EST, MAX_FILL_R, SECURE) \
    HMAP_DEFINE_ABSTRACT_TYPE(NAME, KEY_TYPE, 0, 0, VALUE_TYPE, USE_HEAP, N_PEAK_EST, MAX_FILL_R, SECURE)

/// Defines a new hmap type with a key that is a range of memory.
#define HMAP_DEFINE_STRK_TYPE(NAME, VALUE_TYPE, N_PEAK_EST, MAX_FILL_R, SECURE) \
    HMAP_DEFINE_ABSTRACT_TYPE(NAME, fstr_t, hmap_fstr_key_hash, hmap_fstr_key_cmp, VALUE_TYPE, true, N_PEAK_EST, MAX_FILL_R, SECURE)

/// Like HMAP_DEFINE_TYPE but with good default values for n_peak_est and max_fill_r.
#define HMAP_DEFINE_DFL_TYPE(NAME, KEY_TYPE, VALUE_TYPE, USE_HEAP, SECURE) \
    HMAP_DEFINE_ABSTRACT_TYPE(NAME, KEY_TYPE, 0, 0, VALUE_TYPE, USE_HEAP, HMAP_DFL_N_PEAK_EST, HMAP_DFL_MAX_FILL_R, SECURE)

/// Like HMAP_DEFINE_STRK_TYPE but with good default values for n_peak_est and max_fill_r.
#define HMAP_DEFINE_DFL_STRK_TYPE(NAME, VALUE_TYPE, SECURE) \
    HMAP_DEFINE_STRK_TYPE(NAME, VALUE_TYPE, HMAP_DFL_N_PEAK_EST, HMAP_DFL_MAX_FILL_R, SECURE)

typedef struct hmap {
    /// Count of elements in list.
    size_t count;
    /// Maximum number of elements allowed in list before it should be expanded.
    size_t max_count;
    // The pointer of the first invalid slot after the last valid slot.
    void* end_slot_ptr;
    /// The length of all slots in the memory segment in bytes.
    size_t slots_len;
    /// The memory segment used by the hash map.
    fstr_t mem;
    /// Randomly generated hash map salt.
    uint64_t salt;
} hmap_t;

typedef uint64_t (*hmap_key_hash_fn_t)(void* key, uint64_t salt);

typedef bool (*hmap_key_cmp_fn_t)(void* key1, void* key2);

typedef struct hmap_cfg {
    size_t key_len;
    hmap_key_hash_fn_t key_hash_fn;
    hmap_key_cmp_fn_t key_cmp_fn;
    size_t value_len;
    bool use_heap;
    size_t n_peak_est;
    /// Maximum % full the hash map is allowed to become before expanding it.
    /// This is a limit on the global density in the hash map.
    float max_fill_r;
    bool secure;
} hmap_cfg_t;

#pragma pack(push, 1)

typedef struct hmap_slot {
    /// The offset of the previous slot in the circular linked list. If zero the slot is empty/available.
    /// The msb is used to signify that this slot has the appropriate hash for the slot location and is the root of the circular list.
    struct hmap_slot* prev;
    /// The offset of the next slot in the circular linked list.
    struct hmap_slot* next;
    /// Element data.
    uint8_t elem_data[];
} hmap_slot_t;

#pragma pack(pop)

typedef struct hmap_lookup {
    /// Computed hash of the key we attempted to find.
    uint64_t hash;
    /// True if the key was found. False if the key does not exist in the hash map.
    bool found_element;
    /// The destination slot. Where the element is located if key was found, otherwise where the element should be inserted.
    hmap_slot_t* slot;
} hmap_lookup_t;

static inline uint64_t hmap_murmurhash_64a(void* key, uint64_t len, uint64_t seed) {
    uint64_t m = 0xc6a4a7935bd1e995;
    int r = 47;
    uint64_t h = seed ^ len;
    uint64_t* data = (uint64_t*) key;
    uint64_t* end = data + (len / 8);
    while (data != end) {
        uint64_t k = *data++;
        k *= m;
        k ^= k >> r;
        k *= m;
        h ^= k;
        h *= m;
    }
    unsigned char* data2 = (unsigned char*) data;
    switch (len & 7) {
    case 7:
        h ^= (uint64_t)(data2[6]) << 48;
    case 6:
        h ^= (uint64_t)(data2[5]) << 40;
    case 5:
        h ^= (uint64_t)(data2[4]) << 32;
    case 4:
        h ^= (uint64_t)(data2[3]) << 24;
    case 3:
        h ^= (uint64_t)(data2[2]) << 16;
    case 2:
        h ^= (uint64_t)(data2[1]) << 8;
    case 1:
        h ^= (uint64_t)(data2[0]);
        h *= m;
    }
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h;
}

static inline size_t hmap_elem_len(size_t key_len, size_t value_len) {
    return (sizeof(hmap_slot_t) + key_len + value_len + 0x7UL) & ~0x7UL;
}

static inline float hmap_max_fill_r(float max_fill_r) {
    return MAX(0.0, MIN(1.0, max_fill_r));
}

static inline void hmap_init_len(hmap_t* hm, const hmap_cfg_t cfg, size_t min_len) {
    hm->count = 0;
    size_t final_len;
    void* mem_ptr = cfg.use_heap? lwt_alloc_buffer(min_len, &final_len): vm_mmap_reserve(min_len, &final_len);
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    size_t n_slots = (final_len / elem_len);
    hm->max_count = MIN(n_slots * cfg.max_fill_r, n_slots - 1);
    size_t slots_len = n_slots * elem_len;
    hm->end_slot_ptr = mem_ptr + slots_len;
    hm->slots_len = slots_len;
    fstr_t mem = {.len = final_len, .str = mem_ptr};
    fstr_fill(mem, 0);
    hm->mem = mem;
    if (cfg.secure) {
        // Generate a cryptographic safe salt to make collision exploits less practical.
        polar_secure_drbg_fill(FSTR_PACK(hm->salt));
    } else {
        // No security - better for testing as the hash map behaves in a deterministic manner.
        hm->salt = 0;
    }
}

static inline void hmap_init(hmap_t* hm, const hmap_cfg_t cfg) {
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    size_t min_len = elem_len * ((1.0 + cfg.n_peak_est) * (1.0 / hmap_max_fill_r(cfg.max_fill_r)) * 1.5);
    hmap_init_len(hm, cfg, min_len);
}

static inline void hmap_free(hmap_t* hm, const hmap_cfg_t cfg) {
    if (cfg.use_heap)
        lwt_alloc_free(hm->mem.str);
    else
        vm_mmap_unreserve(hm->mem.str, hm->mem.len);
}

static inline void* hmap_hash_to_slot_ptr(hmap_t* hm, const hmap_cfg_t cfg, uint64_t hash) {
    void* start_slot_ptr = hm->mem.str;
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    return start_slot_ptr + ((hash % hm->slots_len) / elem_len) * elem_len;
}

static inline bool hmap_slot_ptr_is_root(hmap_slot_t* slot_ptr) {
    return (((uint64_t) slot_ptr) & 0x8000000000000000ULL) != 0;
}

static inline hmap_slot_t* hmap_slot_ptr_set_root(hmap_slot_t* slot_ptr) {
    return (hmap_slot_t*) (((uint64_t) slot_ptr) | 0x8000000000000000ULL);
}

static inline hmap_slot_t* hmap_slot_ptr_clear_root(hmap_slot_t* slot_ptr) {
    return (hmap_slot_t*) (((uint64_t) slot_ptr) & ~0x8000000000000000ULL);
}

static inline bool hmap_cmp_key(const hmap_cfg_t cfg, hmap_slot_t* slot, void* key) {
    if (cfg.key_cmp_fn == 0) {
        uint8_t* cmp_ptr_a = slot->elem_data;
        for (uint8_t *cmp_ptr_b = key, *end_ptr = key + cfg.key_len; cmp_ptr_b < end_ptr; cmp_ptr_a++, cmp_ptr_b++) {
            if (*cmp_ptr_a != *cmp_ptr_b)
                return false;
        }
        return true;
    } else {
        return cfg.key_cmp_fn(slot->elem_data, key);
    }
}

static inline void hmap_write_v(const hmap_cfg_t cfg, hmap_slot_t* dst_slot, void* value) {
    uint8_t* dst_ptr = &dst_slot->elem_data[cfg.key_len];
    for (uint8_t *src_ptr = value, *end_ptr = value + cfg.value_len; src_ptr < end_ptr; dst_ptr++, src_ptr++)
        *dst_ptr = *src_ptr;
}

static inline void hmap_write_kv(const hmap_cfg_t cfg, hmap_slot_t* dst_slot, void* key, void* value) {
    uint8_t* dst_ptr = dst_slot->elem_data;
    for (uint8_t *src_ptr = key, *end_ptr = key + cfg.key_len; src_ptr < end_ptr; dst_ptr++, src_ptr++)
        *dst_ptr = *src_ptr;
    for (uint8_t *src_ptr = value, *end_ptr = value + cfg.value_len; src_ptr < end_ptr; dst_ptr++, src_ptr++)
        *dst_ptr = *src_ptr;
}

/// Attempts to find a key and return true if a slot with the key is found.
/// Returns the slot where the key should be inserted if the key is not found.
/// If may_exist is false the function will assume that the key is not already
/// added to the list and be optimized to only find the right empty slot.
static inline hmap_lookup_t hmap_lookup(hmap_t* hm, const hmap_cfg_t cfg, void* key, bool may_exist) {
    // Calculate the hash.
    uint64_t hash = (cfg.key_hash_fn == 0? hmap_murmurhash_64a(key, cfg.key_len, hm->salt): cfg.key_hash_fn(key, hm->salt));
    // Transform vector from hash-space into pointer-space.
    hmap_slot_t* slot = hmap_hash_to_slot_ptr(hm, cfg, hash);
    // If the element may exist and the slot is not empty and a proper root we need to look in the bucket.
    if (may_exist && slot->prev != 0 && hmap_slot_ptr_is_root(slot->prev)) {
        // Follow the circular linked list around and do actual real key compare on the hash collisions.
        for (hmap_slot_t* cur_slot = slot;;) {
            assert(cur_slot->prev != 0);
            if (hmap_cmp_key(cfg, cur_slot, key)) {
                // We have a match at this location.
                return (hmap_lookup_t) {.hash = hash, .found_element = true, .slot = cur_slot};
            }
            // Iterate to next slot. The slot should be nearby making this operation very cache effective.
            cur_slot = cur_slot->next;
            if (cur_slot == slot)
                break;
        }
    }
    // Element does not exist and would be inserted at this slot.
    return (hmap_lookup_t) {.hash = hash, .found_element = false, .slot = slot};
}

/// Attempts to find a free slot in as close proximity to the reference slot as possible.
/// The reference slot is assumed to be occupied.
static inline hmap_slot_t* hmap_scan_free_slot(hmap_t* hm, const hmap_cfg_t cfg, hmap_slot_t* ref_slot) {
    size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
    void *start_slot_ptr = hm->mem.str, *end_slot_ptr = hm->end_slot_ptr;
    void *fwd_slot_ptr = ref_slot, *rev_slot_ptr = ref_slot;
    for (;;) {
        // Iterate to the next slot.
        fwd_slot_ptr += elem_len;
        if (fwd_slot_ptr >= end_slot_ptr)
            fwd_slot_ptr = hm->mem.str;
        // If the slot is empty we can use it.
        hmap_slot_t* fwd_slot = fwd_slot_ptr;
        if (fwd_slot->prev == 0)
            return fwd_slot;
        // Iterate to the previous slot.
        rev_slot_ptr = ((rev_slot_ptr > start_slot_ptr)? rev_slot_ptr: end_slot_ptr) - elem_len;
        // If the slot is empty we can use it.
        hmap_slot_t* rev_slot = rev_slot_ptr;
        if (rev_slot->prev == 0)
            return rev_slot;
    }
}

/// Makes an unsafe insert, without checking if we should expand the hash map.
/// This would hang in hmap_scan_free_slot() if the hash map was full and
/// perform horribly if the hash map was over 50% full.
static inline void hmap_insert_unsafe(hmap_t* hm, const hmap_cfg_t cfg, hmap_lookup_t lu, void* key, void* value) {
    assert(!lu.found_element);
    hmap_slot_t* slot = lu.slot;
    hmap_slot_t* dst_slot;
    if (slot->prev == 0) {
        // The slot is empty, this is the trivial and most likely case.
        slot->prev = hmap_slot_ptr_set_root(slot);
        slot->next = slot;
        dst_slot = slot;
    } else {
        // Find the nearest free slot.
        hmap_slot_t* free_slot = hmap_scan_free_slot(hm, cfg, slot);
        if (hmap_slot_ptr_is_root(slot->prev)) {
            // The slot is a root, insert the element into the free slot and append it to the list.
            free_slot->prev = slot;
            free_slot->next = slot->next;
            slot->next->prev = free_slot;
            slot->next = free_slot;
            // Setting slot->next->prev might have overwritten slot->prev and the root flag if the list only had a single item.
            slot->prev = hmap_slot_ptr_set_root(slot->prev);
            dst_slot = free_slot;
        } else {
            // The slot is not a root, move it to the free slot and re-link it.
            free_slot->prev = slot->prev;
            free_slot->next = slot->next;
            slot->prev->next = free_slot;
            slot->next->prev = hmap_slot_ptr_is_root(slot->next->prev)? hmap_slot_ptr_set_root(free_slot): free_slot;
            hmap_write_kv(cfg, free_slot, &slot->elem_data[0], &slot->elem_data[cfg.key_len]);
            // The slot is a root, link it to itself.
            slot->prev = hmap_slot_ptr_set_root(slot);
            slot->next = slot;
            dst_slot = slot;
        }
    }
    // Write the actual key and value to the final destination slot.
    hmap_write_kv(cfg, dst_slot, key, value);
    hm->count++;
}

/// Inserts a looked up item into the hash map.
static inline void hmap_insert(hmap_t* hm, const hmap_cfg_t cfg, hmap_lookup_t lu, void* key, void* value) {
    assert(!lu.found_element);
    if (hm->count >= hm->max_count) {
        // The list is overflowing, we need to expand it and re-index all the elements.
        size_t new_mem_len = hm->mem.len * 2;
        // Allocate a new hash map.
        hmap_t new_hm;
        hmap_init_len(&new_hm, cfg, new_mem_len);
        // Iterate over all old slots and copy them to the new hash map memory.
        size_t elem_len = hmap_elem_len(cfg.key_len, cfg.value_len);
        void* slot_ptr = (void*) hm->mem.str;
        void* end_slot_ptr = hm->end_slot_ptr;
        for (;;) {
            // Check if the slot is used.
            hmap_slot_t* slot = slot_ptr;
            if (slot->prev != 0) {
                // Make a lookup in the new hash map. We know that the element must not exist there.
                hmap_lookup_t new_lu = hmap_lookup(&new_hm, cfg, slot->elem_data, false);
                // Insert the element into the new hash map.
                assert(!new_lu.found_element);
                hmap_insert_unsafe(&new_hm, cfg, new_lu, &slot->elem_data[0], &slot->elem_data[cfg.key_len]);
            }
            // Increment to the next slot.
            slot_ptr += elem_len;
            if (slot_ptr >= end_slot_ptr)
                break;
        }
        // Free the old hash map and replace it.
        hmap_free(hm, cfg);
        *hm = new_hm;
        // Replace the lookup data with a new lookup in the new hash map and retry insert.
        lu = hmap_lookup(hm, cfg, key, false);
    }
    hmap_insert_unsafe(hm, cfg, lu, key, value);
}

/// Deletes a looked up item from the hash map.
static inline void hmap_delete(hmap_t* hm, const hmap_cfg_t cfg, hmap_lookup_t lu) {
    assert(lu.found_element);
    hmap_slot_t* slot = lu.slot;
    if (slot->next == slot) {
        // This is the only element in the bucket. Clear prev, indicating that the slot is free.
        slot->prev = 0;
    } else if (hmap_slot_ptr_is_root(slot->prev)) {
        // We are root and have other items in the list so we must move another item to the root location.
        // Let's pick the nextmost item as it has the highest chance of being the most furthest away in memory as it is the most recent.
        hmap_slot_t* next_slot = slot->next;
        slot->next = next_slot->next;
        if (next_slot->next == slot)
            slot->prev = hmap_slot_ptr_set_root(slot);
        else
            next_slot->next->prev = slot;
        // Copy the keys and the values from next_slot to slot.
        hmap_write_kv(cfg, slot, &next_slot->elem_data[0], &next_slot->elem_data[cfg.key_len]);
        // Clear prev for the slot we moved to the root, indicating that it is free.
        next_slot->prev = 0;
    } else {
        // Unlink the slot from the list as we are not the only item in it.
        // Since we are not root this is a simple re-link procedure.
        slot->prev->next = slot->next;
        slot->next->prev = hmap_slot_ptr_is_root(slot->next->prev)? hmap_slot_ptr_set_root(slot->prev): slot->prev;
        // Clear prev, indicating that the slot is free.
        slot->prev = 0;
    }
    hm->count--;
}

/// Replace an already existing value of a looked up item into the hash map.
static inline void hmap_replace(hmap_t* hm, const hmap_cfg_t cfg, hmap_lookup_t lu, void* value) {
    assert(lu.found_element);
    hmap_slot_t* slot = lu.slot;
    hmap_write_v(cfg, slot, value);
}

/// Replace an already existing key and value of a looked up item into the hash map.
static inline void hmap_replace_kv(hmap_t* hm, const hmap_cfg_t cfg, hmap_lookup_t lu, void* key, void* value) {
    assert(lu.found_element);
    hmap_slot_t* slot = lu.slot;
    assert(hmap_cmp_key(cfg, slot, key));
    hmap_write_kv(cfg, slot, key, value);
}

static inline uint64_t hmap_fstr_key_hash(void* key, uint64_t salt) {
    fstr_t* fstr_key = key;
    return hmap_murmurhash_64a(fstr_key->str, fstr_key->len, salt);
}

static inline bool hmap_fstr_key_cmp(void* key1, void* key2) {
    fstr_t* fstr_key1 = key1;
    fstr_t* fstr_key2 = key2;
    return fstr_equal(*fstr_key1, *fstr_key2);
}

#endif	/* HMAP_H */
