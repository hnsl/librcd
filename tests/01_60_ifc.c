/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

fiber_main ifc_test_run_inf_cancel(fiber_main_attr) {
    try {
        for (;;)
            ifc_fiber_sync_recv();
    } catch (exception_canceled, e) {}
}

fiber_main ifc_test_max_event(fiber_main_attr, rcd_fid_t event_fid) {
    ifc_event_trigger(event_fid, ULONG_MAX);
}

fiber_main ifc_test_read_event(fiber_main_attr, rcd_fid_t event_fid, uint64_t expected_event_count) {
    uint64_t ev_count = ifc_event_wait(event_fid);
    atest(ev_count == expected_event_count);
    ifc_event_trigger(event_fid, ev_count * 2);
}

void rcd_self_test_ifc() {
    sub_heap {
        // Test events.
        rcd_fid_t event_fid = ifc_create_event_fiber();
        ifc_event_trigger(event_fid, 77);
        fmitosis {
            spawn_fiber(ifc_test_read_event("", event_fid, 77));
        }
        rio_wait(5 * RIO_NS_MS);
        // The following could return 77 first if the new fiber didn't get enough cpu time.
        {
            uint64_t ev_count = ifc_event_wait(event_fid);
            atest(ev_count == 77 * 2);
        }
        /// Test triggering max events.
        ifc_event_trigger(event_fid, 3);
        rcd_sub_fiber_t* max_event_sub_fiber;
        fmitosis {
            max_event_sub_fiber = spawn_fiber(ifc_test_max_event("", event_fid));
        }
        rio_wait(5 * RIO_NS_MS);
        // The following could return 3 first if the new fiber didn't get enough cpu time.
        {
            uint64_t ev_count = ifc_event_wait(event_fid);
            atest(ev_count == ULONG_MAX);
        }{
            uint64_t ev_count = ifc_event_wait(event_fid);
            atest(ev_count == 3);
        }
        ifc_wait(lwt_get_sub_fiber_id(max_event_sub_fiber));
    }
    // Test fiber collections.
    sub_heap {
        sf(fmap)* fm_sf = ifc_fiber_map_create();
        fid(fmap) fm_fid = fmap_sf2id(fm_sf);
        rcd_fid_t fiber_ids[5];
        rcd_fid_t monitor_fiber_ids[5];
        for (size_t i = 0; i < 5; i++) {
            fmitosis {
                fiber_ids[i] = spawn_static_fiber(ifc_test_run_inf_cancel(""));
            }
            bool auto_cancel = (i != 0);
            sub_heap {
                monitor_fiber_ids[i] = ifc_fiber_map_insert_data(fm_fid, FSTR_PACK(fiber_ids[i]), ui2fs(i), fiber_ids[i], auto_cancel);
            }
        }
        // Test that canceling a fiber eventually removes it from the collection.
        lwt_cancel_fiber_id(fiber_ids[3]);
        try {
            ifc_fiber_sync_send(monitor_fiber_ids[3]);
            atest(false);
        } catch (exception_inner_join_fail, e) {}
        atest(ifc_fiber_map_read(fm_fid, FSTR_PACK(fiber_ids[3])) == 0);
        // Test that inserting the now dead fiber immediately causes it to be removed.
        monitor_fiber_ids[3] = ifc_fiber_map_insert(fm_fid, FSTR_PACK(fiber_ids[3]), fiber_ids[3], true);
        try {
            ifc_fiber_sync_send(monitor_fiber_ids[3]);
            atest(false);
        } catch (exception_inner_join_fail, e) {}
        atest(ifc_fiber_map_read(fm_fid, FSTR_PACK(fiber_ids[3])) == 0);
        // Test that removing a fiber from the collection cancels the monitor fid but not the monitored fid.
        ifc_fiber_map_remove(fm_fid, FSTR_PACK(fiber_ids[2]));
        lwt_yield();
        try {
            ifc_fiber_sync_send(monitor_fiber_ids[2]);
            atest(false);
        } catch (exception_inner_join_fail, e) {}
        for (size_t i = 0; i < 2; i++) {
            try {
                ifc_fiber_sync_send(fiber_ids[2]);
            } catch (exception_inner_join_fail, e) {
                atest(false);
            }
        }
        // Test count and listing the content.
        sub_heap {
            dict(rcd_fid_t)* fiber_list = ifc_fiber_map_list(fm_fid);
            atest(*dict_read(fiber_list, rcd_fid_t, FSTR_PACK(fiber_ids[0])) == fiber_ids[0]);
            atest(*dict_read(fiber_list, rcd_fid_t, FSTR_PACK(fiber_ids[1])) == fiber_ids[1]);
            atest(*dict_read(fiber_list, rcd_fid_t, FSTR_PACK(fiber_ids[4])) == fiber_ids[4]);
            size_t count = dict_count(fiber_list, rcd_fid_t);
            atest(count == 3);
        }
        // Test reading individual keys directly.
        sub_heap {
            fstr_mem_t* data;
            atest(ifc_fiber_map_read_data(fm_fid, FSTR_PACK(fiber_ids[0]), &data) == fiber_ids[0]);
            atest(fstr_equal(fss(data), "0"));
            atest(ifc_fiber_map_read_data(fm_fid, FSTR_PACK(fiber_ids[2]), &data) == 0);
            atest(ifc_fiber_map_read_data(fm_fid, "", &data) == 0);
            atest(ifc_fiber_map_read_data(fm_fid, "foo", &data) == 0);
            atest(ifc_fiber_map_read_data(fm_fid, FSTR_PACK(fiber_ids[4]), &data) == fiber_ids[4]);
            atest(fstr_equal(fss(data), "4"));
        }
        // Test that enqueuing a fiber adds it last.
        fstr_mem_t* enqueued_last_key;
        monitor_fiber_ids[2] = ifc_fiber_map_enqueue_data(fm_fid, &enqueued_last_key, "2", fiber_ids[2], true);
        sub_heap {
            dict(rcd_fid_t)* fiber_list = ifc_fiber_map_list(fm_fid);
            fstr_mem_t* last_key = dict_last_key(fiber_list, rcd_fid_t);
            atest(last_key != 0);
            atest(fstr_equal(fss(last_key), fss(enqueued_last_key)));
        }
        // Test that destroying the collection cancels all fibers that was specified to be auto canceled (all except i = 0)
        lwt_alloc_free(fm_sf);
        lwt_yield();
        for (size_t i = 0; i < 5; i++) {
            lwt_yield();
            try {
                ifc_fiber_sync_send(monitor_fiber_ids[i]);
                atest(false);
            } catch (exception_inner_join_fail, e) {}
        }
        for (size_t i = 0; i < 5; i++) {
            lwt_yield();
            bool auto_cancel = (i != 0);
            try {
                ifc_fiber_sync_send(fiber_ids[i]);
                if (auto_cancel)
                    atest(false);
                lwt_cancel_fiber_id(fiber_ids[i]);
            } catch (exception_inner_join_fail, e) {
                if (!auto_cancel)
                    atest(false);
            }
        }
        // Test that enqueueing 5 fibers and dequeuing them works.
        fm_sf = ifc_fiber_map_create();
        fm_fid = fmap_sf2id(fm_sf);
        fstr_mem_t* all_enqueued_keys[5];
        for (size_t i = 0; i < 5; i++) {
            fmitosis {
                fiber_ids[i] = spawn_static_fiber(ifc_test_run_inf_cancel(""));
            }
            sub_heap {
                fstr_mem_t* enqueued_key;
                monitor_fiber_ids[i] = ifc_fiber_map_enqueue_data(fm_fid, &enqueued_key, ui2fs(i), fiber_ids[i], true);
                all_enqueued_keys[i] = escape(enqueued_key);
            }
        }
        for (ssize_t i = 4; i >= 0; i--)
            atest(ifc_fiber_map_read(fm_fid, fss(all_enqueued_keys[i])) == fiber_ids[i]);
        for (size_t i = 0; i < 5; i++) {
            fstr_mem_t* key;
            sub_heap {
                fstr_mem_t* data;
                atest(ifc_fiber_map_dequeue_data(fm_fid, &key, &data) == fiber_ids[i]);
                atest(fstr_equal(fss(key), fss(all_enqueued_keys[i])));
                atest(fstr_equal(fss(data), ui2fs(i)));
            }
        }
        // Test that we now have an empty fiber list.
        {
            dict(rcd_fid_t)* fiber_list = ifc_fiber_map_list(fm_fid);
            atest(dict_count(fiber_list, rcd_fid_t) == 0);
        }
        // Test that dequeuing the fibers really canceled the monitor fibers.
        for (size_t i = 0; i < 5; i++) {
            lwt_yield();
            try {
                ifc_fiber_sync_send(monitor_fiber_ids[i]);
                atest(false);
            } catch (exception_inner_join_fail, e) {}
        }
        // Test that after dequeuing, all fibers are still alive even though auto cancel was enabled.
        for (size_t j = 0; j < 2; j++)
        for (size_t i = 0; i < 5; i++) {
            try {
                ifc_fiber_sync_send(fiber_ids[i]);
            } catch (exception_inner_join_fail, e) {
                atest(false);
            }
        }
        lwt_alloc_free(fm_sf);
        // Test that enqueueing 5 fibers and destroying the collection cancels all fibers if auto cancel is enabled.
        for (size_t auto_cancel_i = 0; auto_cancel_i < 2; auto_cancel_i++) {
            fm_sf = ifc_fiber_map_create();
            fm_fid = fmap_sf2id(fm_sf);
            bool auto_cancel = (auto_cancel_i == 1);
            for (size_t i = 0; i < 5; i++) {
                sub_heap {
                    fstr_mem_t* enqueued_key;
                    monitor_fiber_ids[i] = ifc_fiber_map_enqueue_data(fm_fid, &enqueued_key, ui2fs(i), fiber_ids[i], auto_cancel);
                    all_enqueued_keys[i] = escape(enqueued_key);
                }
            }
            for (ssize_t i = 4; i >= 0; i--)
                atest(ifc_fiber_map_read(fm_fid, fss(all_enqueued_keys[i])) == fiber_ids[i]);
            lwt_alloc_free(fm_sf);
            lwt_yield();
            if (auto_cancel) {
                for (size_t i = 0; i < 5; i++) {
                    lwt_yield();
                    try {
                        ifc_fiber_sync_send(monitor_fiber_ids[i]);
                        atest(false);
                    } catch (exception_inner_join_fail, e) {}
                }
                for (size_t i = 0; i < 5; i++) {
                    lwt_yield();
                    try {
                        ifc_fiber_sync_send(fiber_ids[i]);
                        atest(false);
                    } catch (exception_inner_join_fail, e) {}
                }
            } else {
                for (size_t i = 0; i < 5; i++) {
                    lwt_yield();
                    try {
                        ifc_fiber_sync_send(fiber_ids[i]);
                    } catch (exception_inner_join_fail, e) {
                        atest(false);
                    }
                }
            }
        }
    }
    // Test that break works in cancel_timeout_in_ns statement.
    {
        int i = 0;
        sub_heap {
            ifc_cancel_alarm_arm(4 * RIO_NS_SEC);
            atest(i == 0);
            i++;
            break;
        }
    }
    // Test timeout that should be triggered.
    try {
        sub_heap {
            ifc_cancel_alarm_arm(10 * RIO_NS_MS);
            rio_t* timer = rio_timer_create();
            rio_alarm_set(timer, 4 * RIO_NS_SEC, false, 0);
            rio_alarm_wait(timer);
            atest(false);
        }
    } catch (exception_canceled, e) {}
    // Test timeout that should not be triggered.
    try {
        sub_heap {
            ifc_cancel_alarm_arm(4 * RIO_NS_SEC);
            rio_t* timer = rio_timer_create();
            rio_alarm_set(timer, 10 * RIO_NS_MS, false, 0);
            rio_alarm_wait(timer);
        }
    } catch (exception_canceled, e) {
        atest(false);
    }
    // Test that infinite pipe buffering.
    sub_heap {
        rio_t *ipipe_r, *ipipe_w;
        sf(ibpipe)* ibp = ifc_ibpipe_create();
        sf(ipipe)* ipipe_sf = ifc_ipipe_create(&ipipe_r, &ipipe_w);
        for (size_t i = 0; i < 2; i++) {
            bool use_bucket = (i == 1);
            sub_heap {
                // Test a simple read/write.
                sub_heap {
                    if (use_bucket) {
                        for (size_t i = 0; i < 2; i++) {
                            bool more_hint = (i == 1);
                            fstr_mem_t* test_str = fstr_cpy("foo bar");
                            ifc_ibpipe_write(ibpipe_sf2id(ibp), test_str, more_hint);
                            bool out_more_hint;
                            lwt_heap_t* chunk_list_heap;
                            list(fstr_mem_t*)* chunk_list = ifc_ibpipe_read(ibpipe_sf2id(ibp), &out_more_hint, &chunk_list_heap);
                            switch_heap(chunk_list_heap) {
                                atest(out_more_hint == more_hint);
                                atest(list_count(chunk_list, fstr_mem_t*) == 1);
                                atest(list_pop_start(chunk_list, fstr_mem_t*) == test_str);
                            }
                        }
                    } else {
                        rio_write_fstr(ipipe_w, "foo bar");
                        fstr_t test_str = fss(rio_read_fstr(ipipe_r));
                        atest(fstr_equal(test_str, "foo bar"));
                    }
                }
                // Test faster writing than reading.
                size_t write_size = 7;
                size_t read_size = 3;
                const size_t total_bytes = 0x30000;
                for (size_t total_written = 0, total_read = 0, write_count = 0; total_read != total_bytes;) {
                    bool more_hint = ((write_count % 2) == 0);
                    bool no_more = ((write_count % 5) == 1);
                    size_t n_chunks = use_bucket? ((write_count % 4) + 1): 1;
                    if (total_written < total_bytes) {
                        for (size_t n = 0; n < n_chunks; n++) {
                            sub_heap {
                                fstr_mem_t* write_chunk_mem = fstr_alloc(write_size);
                                fstr_t write_chunk = fss(write_chunk_mem);
                                for (size_t i = 0; i < write_size; i++)
                                    write_chunk.str[i] = (0x42 + total_written + i) % 0x100;
                                if (use_bucket) {
                                    // DBG("infinite pipe: writing [", ui2fs(write_size), "] byte bucket chunk");
                                    ifc_ibpipe_write(ibpipe_sf2id(ibp), write_chunk_mem, more_hint);
                                } else {
                                    // DBG("infinite pipe: writing [", ui2fs(write_size), "] byte standard chunk");
                                    rio_write_part(ipipe_w, write_chunk, more_hint);
                                }
                                total_written += write_size;
                                write_size = MIN(total_bytes - total_written, write_size + 5 + write_size / 5);
                            }
                        }
                        write_count++;
                    }
                    sub_heap {
                        bool out_more_hint;
                        if (use_bucket) {
                            list(fstr_mem_t*)* chunks = ifc_ibpipe_read(ibpipe_sf2id(ibp), (no_more? 0: &out_more_hint), 0);
                            atest(list_count(chunks, fstr_mem_t*) == n_chunks);
                            list_foreach(chunks, fstr_mem_t*, chunk) {
                                for (size_t i = 0; i < chunk->len; i++)
                                    atest(chunk->str[i] == ((0x42 + total_read + i) % 0x100));
                                total_read += chunk->len;
                            }
                            atest(no_more || out_more_hint == more_hint);
                        } else {
                            fstr_t read_chunk = fss(fstr_alloc(read_size));
                            // DBG("infinite pipe: reading [", ui2fs(read_size), "] byte chunk");
                            rio_read_fill(ipipe_r, read_chunk);
                            for (size_t i = 0; i < read_size; i++)
                                atest(read_chunk.str[i] == ((0x42 + total_read + i) % 0x100));
                            total_read += read_size;
                            read_size = MIN(total_bytes - total_read, read_size + 5 + read_size / 9);
                        }
                    }
                }
                // Try read one byte and expect to timeout.
                try {
                    sub_heap {
                        ifc_cancel_alarm_arm(4 * RIO_NS_SEC);
                        if (use_bucket) {
                            ifc_ibpipe_read(ibpipe_sf2id(ibp), 0, 0);
                        } else {
                            fstr_t byte = fss(fstr_alloc(1));
                            rio_read(ipipe_r, byte);
                        }
                        atest(false);
                    }
                } catch (exception_canceled, e) {}
            }
        }
    }
}
