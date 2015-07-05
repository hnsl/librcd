/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

join_locked(void) ifc_fiber_sync(join_server_params, rcd_fid_t* out_fiber_id) {
    *out_fiber_id = lwt_get_fiber_id();
}

void ifc_fiber_sync_send(rcd_fid_t remote_fiber_id) {
    ifc_fiber_sync(remote_fiber_id);
}

rcd_fid_t ifc_fiber_sync_recv() {
    rcd_fid_t server_fiber_id;
    accept_join(ifc_fiber_sync, join_server_params, &server_fiber_id);
    return server_fiber_id;
}

void ifc_park() {
    ifc_fiber_sync_send(lwt_get_fiber_id());
    throw("parked fiber woke up unexpectedly", exception_fatal);
}

/// A function that will never be called by the fiber we monitor. Used to provoke a join race when the fiber exits.
join_locked(void) ifc_wait_nop(join_server_params) {}

void ifc_wait(rcd_fid_t fid) {
    try {
        for (;;)
            ifc_wait_nop(fid);
    } catch (exception_inner_join_fail, e) {
        // Explicitly freeing memory instead of using a sub heap so we
        // can fulfill promise to be safe to call from destructors.
        lwt_alloc_free(e->exception_heap);
    }
}

join_locked(uint64_t) event_fiber_trigger(uint64_t trigger_count, join_server_params, uint64_t* current_count) {
    uint64_t current_count_left = (ULONG_MAX - *current_count);
    uint64_t add_count = MIN(trigger_count, current_count_left);
    *current_count += add_count;
    return trigger_count - add_count;
}

join_locked(uint64_t) event_fiber_consume(join_server_params, uint64_t* current_count) {
    uint64_t return_count = *current_count;
    *current_count = 0;
    return return_count;
}

fiber_main event_fiber(fiber_main_attr) { try {
    uint64_t count = 0;
    for (;;) {
        accept_join(
            event_fiber_trigger if (count < ULONG_MAX),
            event_fiber_consume if (count > 0),
            join_server_params,
            &count
        );
    }
} catch (exception_desync, e); }

rcd_fid_t ifc_create_event_fiber() {
    rcd_fid_t event_fid;
    fmitosis {
        event_fid = spawn_static_fiber(event_fiber(""));
    }
    return event_fid;
}

void ifc_event_trigger(rcd_fid_t event_fid, uint64_t count) {
    while (count > 0)
        count = event_fiber_trigger(count, event_fid);
}

uint64_t ifc_event_wait(rcd_fid_t event_fid) {
    return event_fiber_consume(event_fid);
}

join_locked(fstr_t) ifc_pipe_fiber_read(fstr_t dst, join_server_params, fstr_cfifo_t* cfifo) {
    return fstr_cfifo_read(cfifo, dst, false);
}

join_locked(fstr_t) ifc_pipe_fiber_write(fstr_t src, join_server_params, fstr_cfifo_t* cfifo) {
    return fstr_cfifo_write(cfifo, src, false);
}

fiber_main ifc_pipe_fiber(fiber_main_attr, fstr_t buffer) { try {
    if (buffer.len == 0)
        ifc_park();
    fstr_cfifo_t cfifo = fstr_cfifo_init(buffer);
    for (;;) {
        accept_join( \
            ifc_pipe_fiber_read if (fstr_cfifo_len_data(&cfifo) > 0), \
            ifc_pipe_fiber_write if (fstr_cfifo_len_space(&cfifo) > 0), \
            join_server_params, \
            &cfifo \
        );
    }
} catch (exception_canceled, e); }

fstr_t ifc_pipe_read(rcd_fid_t pipe_fid, fstr_t dst) {
    return ifc_pipe_fiber_read(dst, pipe_fid);
}

void ifc_pipe_write(rcd_fid_t pipe_fid, fstr_t src) {
    while (src.len > 0)
        src = ifc_pipe_fiber_write(src, pipe_fid);
}

// Creates a one-directional pipe that can be read or written to.
rcd_sub_fiber_t* ifc_create_pipe(size_t buffer_size) {
    rcd_sub_fiber_t* pipe_fiber;
    fmitosis {
        fstr_t fiber_name = fss(conc("[lwt-pipe(for fid #", rcd_self, ")]"));
        pipe_fiber = spawn_fiber(ifc_pipe_fiber(fiber_name, fss(fstr_alloc_buffer(buffer_size))));
    }
    return pipe_fiber;
}

typedef struct {
    rcd_fid_t remote_fid;
    rcd_fid_t monitor_fid;
    bool auto_cancel;
    fstr_mem_t* data;
} fm_index_entry_t;

join_locked_declare(void) ifc_fiber_map_fiber_remove(fstr_t key, rcd_fid_t validate_monitor_fid, join_server_params, dict(fm_index_entry_t)* fiber_index_ents);

fiber_main ifc_fiber_map_monitor_fiber(fiber_main_attr, fstr_t key, rcd_fid_t remote_fid, rcd_fid_t fm) {
    try {
        ifc_wait(remote_fid);
        ifc_fiber_map_fiber_remove(key, rcd_self, fm);
    } catch (exception_canceled | exception_inner_join_fail, e) {}
}

join_shared(dict(rcd_fid_t)*) ifc_fiber_map_fiber_list(join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    dict(rcd_fid_t)* fibers = new_dict(rcd_fid_t);
    dict_foreach(fiber_index_ents, fm_index_entry_t, key, idx_ent) {
        bool dict_insert_ok = dict_insert(fibers, rcd_fid_t, key, idx_ent.remote_fid);
        assert(dict_insert_ok);
    }
    return fibers;
}

join_shared(dict(fstr_t)*) ifc_fiber_map_fiber_kvlist(join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    dict(fstr_t)* kv_list = new_dict(fstr_t);
    dict_foreach(fiber_index_ents, fm_index_entry_t, key, idx_ent) {
        bool dict_insert_ok = dict_insert(kv_list, fstr_t, key, idx_ent.data != 0? fsc(fss(idx_ent.data)): "");
        assert(dict_insert_ok);
    }
    return kv_list;
}

join_locked(rcd_fid_t) ifc_fiber_map_fiber_put(fstr_t key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel, bool replace, join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    fm_index_entry_t idx_ent;
    server_heap_flip {
        fm_index_entry_t* cur_idx_ent = dict_read(fiber_index_ents, fm_index_entry_t, key);
        if (cur_idx_ent != 0) {
            if (!replace)
                return 0;
            lwt_cancel_fiber_id(cur_idx_ent->monitor_fid);
            if (cur_idx_ent->data != 0)
                lwt_alloc_free(cur_idx_ent->data);
            bool delete_successful = dict_delete(fiber_index_ents, fm_index_entry_t, key);
            assert(delete_successful);
        }
        idx_ent.remote_fid = remote_fid;
        fmitosis {
            idx_ent.monitor_fid = spawn_static_fiber(ifc_fiber_map_monitor_fiber("", fss(fstr_cpy(key)), remote_fid, server_fiber_id));
        }
        idx_ent.auto_cancel = auto_cancel;
        idx_ent.data = (data.len != 0)? fstr_cpy(data): 0;
        bool dict_insert_ok = dict_insert(fiber_index_ents, fm_index_entry_t, key, idx_ent);
        assert(dict_insert_ok);
    }
    return idx_ent.monitor_fid;
}

join_shared(rcd_fid_t) ifc_fiber_map_fiber_read(fstr_t key, fstr_mem_t** out_data, join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    fm_index_entry_t* idx_ent = dict_read(fiber_index_ents, fm_index_entry_t, key);
    if (idx_ent == 0)
        return 0;
    if (out_data != 0)
        *out_data = (idx_ent->data != 0)? fstr_cpy(fss(idx_ent->data)): 0;
    return idx_ent->remote_fid;
}

join_locked(void) ifc_fiber_map_fiber_remove(fstr_t key, rcd_fid_t validate_monitor_fid, join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    server_heap_flip {
        fm_index_entry_t* idx_ent = dict_read(fiber_index_ents, fm_index_entry_t, key);
        if (idx_ent != 0) {
            if (validate_monitor_fid != 0) {
                // Validated remove where cleanup is irrelevant if the key has since been reassigned.
                if (idx_ent->monitor_fid != validate_monitor_fid)
                    return;
            }
            if (rcd_self != idx_ent->monitor_fid) {
                // The user space is removing the key, kill the monitor.
                lwt_cancel_fiber_id(idx_ent->monitor_fid);
            }
            if (idx_ent->data != 0)
                lwt_alloc_free(idx_ent->data);
            bool delete_successful = dict_delete(fiber_index_ents, fm_index_entry_t, key);
            assert(delete_successful);
        }
    }
}

join_locked(rcd_fid_t) ifc_fiber_map_fiber_enqueue(fstr_mem_t** out_key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel, join_server_params, dict(fm_index_entry_t)* fiber_index_ents) { sub_heap {
    fstr_mem_t* last_key = dict_last_key(fiber_index_ents, fm_index_entry_t);
    fstr_t next_last_key;
    fstr_mem_t* ret_last_key;
    if (last_key != 0) {
        fstr_mem_t* next_last_key_mem = fstr_order_next(fss(last_key));
        next_last_key = fss(next_last_key_mem);
        if (out_key != 0)
            ret_last_key = next_last_key_mem;
    } else {
        next_last_key = "";
        if (out_key != 0)
            ret_last_key = fstr_cpy(next_last_key);
    }
    server_heap_flip {
        fm_index_entry_t idx_ent = {
            .remote_fid = remote_fid,
            .auto_cancel = auto_cancel,
            .data = (data.len > 0)? fstr_cpy(data): 0,
        };
        fmitosis {
            idx_ent.monitor_fid = spawn_static_fiber(ifc_fiber_map_monitor_fiber("", fss(fstr_cpy(next_last_key)), remote_fid, server_fiber_id));
        }
        bool dict_insert_ok = dict_insert(fiber_index_ents, fm_index_entry_t, next_last_key, idx_ent);
        assert(dict_insert_ok);
        if (out_key != 0)
            *out_key = escape(ret_last_key);
        return idx_ent.monitor_fid;
    }
}}

join_locked(rcd_fid_t) ifc_fiber_map_fiber_dequeue(fstr_mem_t** out_key, fstr_mem_t** out_data, join_server_params, dict(fm_index_entry_t)* fiber_index_ents) {
    rcd_fid_t fid_r;
    server_heap_flip {
        fstr_mem_t* key = dict_first_key(fiber_index_ents, fm_index_entry_t);
        if (key != 0) {
            if (out_key != 0) server_heap_flip {
                *out_key = fstr_cpy(fss(key));
            }
            fm_index_entry_t* idx_ent = dict_read(fiber_index_ents, fm_index_entry_t, fss(key));
            assert(idx_ent != 0);
            lwt_cancel_fiber_id(idx_ent->monitor_fid);
            fid_r = idx_ent->remote_fid;
            if (out_data != 0) server_heap_flip {
                *out_data = (idx_ent->data != 0)? fstr_cpy(fss(idx_ent->data)): 0;
            }
            if (idx_ent->data != 0)
                lwt_alloc_free(idx_ent->data);
            bool delete_successful = dict_delete(fiber_index_ents, fm_index_entry_t, fss(key));
            assert(delete_successful);
        } else {
            fid_r = 0;
        }
    }
    return fid_r;
}

fiber_main_t(fmap) ifc_fiber_map_fiber(fiber_main_attr) {
    dict(fm_index_entry_t)* fiber_index_ents = new_dict(fm_index_entry_t);
    try {
        auto_accept_join(ifc_fiber_map_fiber_list, ifc_fiber_map_fiber_kvlist, ifc_fiber_map_fiber_put, ifc_fiber_map_fiber_read, ifc_fiber_map_fiber_remove, ifc_fiber_map_fiber_enqueue, ifc_fiber_map_fiber_dequeue, join_server_params, fiber_index_ents);
    } catch (exception_canceled, e) {
        // Cancel all the monitoring fibers and if auto cancel is enabled, also the remote fibers.
        uninterruptible {
            dict_foreach(fiber_index_ents, fm_index_entry_t, key, idx_ent) {
                lwt_cancel_fiber_id(idx_ent.monitor_fid);
                if (idx_ent.auto_cancel)
                    lwt_cancel_fiber_id(idx_ent.remote_fid);
            }
        }
    }
}

sf(fmap)* ifc_fiber_map_create() {
    fmitosis {
        return spawn_fiber(ifc_fiber_map_fiber(""));
    }
}

dict(rcd_fid_t)* ifc_fiber_map_list(fid(fmap) fm) {
    return ifc_fiber_map_fiber_list(fm.fid);
}

dict(fstr_t)* ifc_fiber_map_kvlist(fid(fmap) fm) {
    return ifc_fiber_map_fiber_kvlist(fm.fid);
}

rcd_fid_t ifc_fiber_map_insert(fid(fmap) fm, fstr_t key, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_put(key, "", remote_fid, auto_cancel, false, fm.fid);
}

rcd_fid_t ifc_fiber_map_insert_data(fid(fmap) fm, fstr_t key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_put(key, data, remote_fid, auto_cancel, false, fm.fid);
}

rcd_fid_t ifc_fiber_map_replace(fid(fmap) fm, fstr_t key, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_put(key, "", remote_fid, auto_cancel, true, fm.fid);
}

rcd_fid_t ifc_fiber_map_replace_data(fid(fmap) fm, fstr_t key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_put(key, data, remote_fid, auto_cancel, true, fm.fid);
}

rcd_fid_t ifc_fiber_map_read(fid(fmap) fm, fstr_t key) {
    return ifc_fiber_map_fiber_read(key, 0, fm.fid);
}

rcd_fid_t ifc_fiber_map_read_data(fid(fmap) fm, fstr_t key, fstr_mem_t** out_data) {
    return ifc_fiber_map_fiber_read(key, out_data, fm.fid);
}

void ifc_fiber_map_remove(fid(fmap) fm, fstr_t key) {
    ifc_fiber_map_fiber_remove(key, 0, fm.fid);
}

void ifc_fiber_map_cond_remove(fid(fmap) fm, rcd_fid_t validate_monitor_fid, fstr_t key) {
    ifc_fiber_map_fiber_remove(key, validate_monitor_fid, fm.fid);
}

rcd_fid_t ifc_fiber_map_enqueue(fid(fmap) fm, fstr_mem_t** out_key, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_enqueue(out_key, "", remote_fid, auto_cancel, fm.fid);
}

rcd_fid_t ifc_fiber_map_enqueue_data(fid(fmap) fm, fstr_mem_t** out_key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel) {
    return ifc_fiber_map_fiber_enqueue(out_key, data, remote_fid, auto_cancel, fm.fid);
}

rcd_fid_t ifc_fiber_map_dequeue(fid(fmap) fm, fstr_mem_t** out_key) {
    return ifc_fiber_map_fiber_dequeue(out_key, 0, fm.fid);
}

rcd_fid_t ifc_fiber_map_dequeue_data(fid(fmap) fm, fstr_mem_t** out_key, fstr_mem_t** out_data) {
    return ifc_fiber_map_fiber_dequeue(out_key, out_data, fm.fid);
}

fiber_main ifc_cancel_alarm_fiber(fiber_main_attr, uint128_t alarm_timeout_ns, rcd_fid_t cancel_target_fid) { try {
    lwt_cancellation_point();
    rio_t* timer = rio_timer_create();
    rio_alarm_set(timer, alarm_timeout_ns, false, 0);
    rio_alarm_wait(timer);
    // Need to use exclusive cancellation instead of normal,
    // otherwise we have a race where both fibers can cancel each other.
    lwt_exclusive_cancel_fiber_id(cancel_target_fid);
} catch (exception_canceled, e); }

rcd_sub_fiber_t* ifc_cancel_alarm_arm_fid(uint128_t alarm_timeout_ns, rcd_fid_t cancel_target_fid) {
    rcd_sub_fiber_t* cancel_sf;
    fmitosis {
        cancel_sf = spawn_fiber(ifc_cancel_alarm_fiber("", alarm_timeout_ns, cancel_target_fid));
    }
    return cancel_sf;
}

rcd_sub_fiber_t* ifc_cancel_alarm_arm(uint128_t alarm_timeout_ns) {
    return ifc_cancel_alarm_arm_fid(alarm_timeout_ns, rcd_self);
}

fiber_main ifc_cancel_on_exit_fiber(fiber_main_attr, rcd_fid_t wait_fid, rcd_fid_t cancel_target_fid) { try {
    lwt_cancellation_point();
    ifc_wait(wait_fid);
    // Need to use exclusive cancellation instead of normal,
    // otherwise we have a race where both fibers can cancel each other.
    lwt_exclusive_cancel_fiber_id(cancel_target_fid);
} catch (exception_canceled, e); }

rcd_sub_fiber_t* ifc_cancel_on_exit_arm_fid(rcd_fid_t wait_fid, rcd_fid_t cancel_target_fid) {
    rcd_sub_fiber_t* cancel_sf;
    fmitosis {
        cancel_sf = spawn_fiber(ifc_cancel_on_exit_fiber("", wait_fid, cancel_target_fid));
    }
    return cancel_sf;
}

rcd_sub_fiber_t* ifc_cancel_on_exit_arm(rcd_fid_t wait_fid) {
    return ifc_cancel_on_exit_arm_fid(wait_fid, rcd_self);
}

typedef struct ifc_ipipe_frame {
    fstr_mem_t* mem;
    size_t read_len;
} ifc_ipipe_frame_t;

typedef struct ifc_ipipe_state {
    /// Queue of frames waiting to be read. Is enqueued at the end and dequeued at the front.
    list(ifc_ipipe_frame_t*)* frame_queue;
    /// Next proto frame under construction.
    ifc_ipipe_frame_t* proto_frame;
    /// How far the proto frame has been constructed.
    size_t proto_frame_len;
    /// How large new proto frames should be.
    size_t new_proto_frame_size;
    /// Set by the writer when indicating that there will or will not be immediate additional writes. (Optimization)
    bool more_hint;
    /// Set by writer when the stream has ended and no more data will be available.
    bool end_of_stream;
    /// Number of bytes that is enqueued in the pipe.
    size_t enqueued_len;
    /// Soft maximum number of bytes that is allowed to be enqueued in the pipe.
    /// When zero, infinite amount of data is allowed to be enqueued.
    size_t max_len;
} ifc_ipipe_state_t;

join_locked(fstr_t) ifc_ipipe_fiber_read(fstr_t buffer, bool* more_hint_out, join_server_params, ifc_ipipe_state_t* pipe_state) {
    fstr_t read_data;
    if (list_count(pipe_state->frame_queue, ifc_ipipe_frame_t*) == 0) {
        if (pipe_state->proto_frame_len == 0) {
            assert(pipe_state->end_of_stream);
            throw_eio("end of ipipe stream reached, write side closed gracefully", rio_eos);
        }
        // Read directly from the proto frame.
        ifc_ipipe_frame_t* proto_frame = pipe_state->proto_frame;
        read_data = fstr_cpy_over(buffer, fstr_slice(fss(proto_frame->mem), proto_frame->read_len, pipe_state->proto_frame_len), 0, 0);
        proto_frame->read_len += read_data.len;
        if (proto_frame->read_len == pipe_state->proto_frame_len) {
            // Reset the proto frame as it has been read completely.
            pipe_state->proto_frame_len = 0;
            proto_frame->read_len = 0;
        }
    } else {
        // Read from the first queued frame.
        ifc_ipipe_frame_t* frame = list_peek_start(pipe_state->frame_queue, ifc_ipipe_frame_t*);
        read_data = fstr_cpy_over(buffer, fstr_sslice(fss(frame->mem), frame->read_len, -1), 0, 0);
        frame->read_len += read_data.len;
        if (frame->read_len == frame->mem->len) {
            // Discard the front frame as it has been read completely.
            server_heap_flip {
                ifc_ipipe_frame_t* popd_frame = list_pop_start(pipe_state->frame_queue, ifc_ipipe_frame_t*);
                assert(popd_frame == frame);
                lwt_alloc_free(frame->mem);
                lwt_alloc_free(frame);
            }
        }
    }
    if (more_hint_out != 0) {
        if (list_count(pipe_state->frame_queue, ifc_ipipe_frame_t*) == 0 && pipe_state->proto_frame_len == 0) {
            // There is nothing left in the buffer but more hint could still be true if the writer has indicated this.
            *more_hint_out = pipe_state->more_hint;
        } else {
            // We know that there is more data to be read.
            *more_hint_out = true;
        }
    }
    assert(pipe_state->enqueued_len >= read_data.len);
    pipe_state->enqueued_len -= read_data.len;
    return read_data;
}

static ifc_ipipe_frame_t* ifc_ipipe_new_proto_frame(size_t new_proto_frame_size) {
    ifc_ipipe_frame_t* proto_frame = new(ifc_ipipe_frame_t);
    proto_frame->mem = fstr_alloc_buffer(new_proto_frame_size);
    proto_frame->read_len = 0;
    return proto_frame;
}

join_locked(fstr_t) ifc_ipipe_fiber_write(fstr_t data, bool more_hint, join_server_params, ifc_ipipe_state_t* pipe_state) {
    if (data.len == 0) {
        pipe_state->end_of_stream = true;
        return data;
    }
    ifc_ipipe_frame_t* proto_frame = pipe_state->proto_frame;
    fstr_t dst_buffer = fstr_sslice(fss(proto_frame->mem), pipe_state->proto_frame_len, -1);
    fstr_t unwritten_data_tail;
    fstr_t dst_written = fstr_cpy_over(dst_buffer, data, 0, &unwritten_data_tail);
    pipe_state->proto_frame_len += dst_written.len;
    if (pipe_state->proto_frame_len == proto_frame->mem->len) {
        // Proto frame is complete, enqueue it.
        server_heap_flip {
            list_push_end(pipe_state->frame_queue, ifc_ipipe_frame_t*, proto_frame);
            pipe_state->proto_frame = ifc_ipipe_new_proto_frame(pipe_state->new_proto_frame_size);
            pipe_state->proto_frame_len = 0;
        }
    }
    pipe_state->more_hint = more_hint;
    pipe_state->enqueued_len += dst_written.len;
    return unwritten_data_tail;
}

fiber_main_t(ipipe) ifc_ipipe_fiber(fiber_main_attr, size_t max_buf_len) {
    const size_t default_new_proto_frame_size = PAGE_SIZE;
    try {
        ifc_ipipe_state_t pipe_state;
        pipe_state.frame_queue = new_list(ifc_ipipe_frame_t*);
        pipe_state.proto_frame = ifc_ipipe_new_proto_frame(default_new_proto_frame_size);
        pipe_state.proto_frame_len = 0;
        pipe_state.new_proto_frame_size = default_new_proto_frame_size;
        pipe_state.more_hint = false;
        pipe_state.end_of_stream = false;
        pipe_state.enqueued_len = 0;
        pipe_state.max_len = max_buf_len;
        for (;;) {
            bool read_ready = (list_count(pipe_state.frame_queue, ifc_ipipe_frame_t*) > 0
                || pipe_state.proto_frame_len > 0
                || pipe_state.end_of_stream);
            bool write_ready = (pipe_state.max_len == 0 || pipe_state.enqueued_len < pipe_state.max_len);
            accept_join( \
                ifc_ipipe_fiber_read if read_ready, \
                ifc_ipipe_fiber_write if write_ready, \
                join_server_params, \
                &pipe_state
            );
        }
    } catch (exception_canceled, e) {}
}

static fstr_t ifc_ipipe_read(rcd_fid_t inf_pipe_fid, fstr_t buffer, bool* more_hint_out) {
    try {
        return ifc_ipipe_fiber_read(buffer, more_hint_out, inf_pipe_fid);
    } catch (exception_inner_join_fail, e) {
        throw_fwd("ipipe no longer exists", exception_io, e);
    }
}

static fstr_t ifc_ipipe_write(rcd_fid_t inf_pipe_fid, fstr_t data, bool more_hint) {
    try {
        return ifc_ipipe_fiber_write(data, more_hint, inf_pipe_fid);
    } catch (exception_inner_join_fail, e) {
        throw_fwd("ipipe no longer exists", exception_io, e);
    }
}

sf(ipipe)* ifc_ipipe_create(rio_t** inf_pipe_r_out, rio_t** inf_pipe_w_out, size_t max_buf_len) {
    sf(ipipe)* ipipe_sf;
    fmitosis {
        ipipe_sf = spawn_fiber(ifc_ipipe_fiber("", max_buf_len));
    }
    rcd_fid_t ipipe_fid = sf2id(ipipe, ipipe_sf);
    static const rio_class_t ifc_ipipe_read_class = {
        .read_part_fn = ifc_ipipe_read,
    };
    *inf_pipe_r_out = rio_new_abstract(&ifc_ipipe_read_class, ipipe_fid, 0);
    static const rio_class_t ifc_ipipe_write_class = {
        .write_part_fn = ifc_ipipe_write,
        .notify_wclose = true
    };
    *inf_pipe_w_out = rio_new_abstract(&ifc_ipipe_write_class, ipipe_fid, 0);
    return ipipe_sf;
}

typedef struct ifc_ibpipe_state {
    /// Heap for the allocated frame queue.
    lwt_heap_t* frame_queue_heap;
    /// Queue of frames waiting to be read. Is enqueued at the end and dequeued at the front.
    list(fstr_mem_t*)* frame_queue;
    /// Set by the writer when indicating that there will or will not be immediate additional writes. (Optimization)
    bool more_hint;
} ifc_ibpipe_state_t;

/// A non copying form of write that imports entire memory buckets.
join_locked(void) ifc_ibpipe_fiber_write(fstr_mem_t* bucket, bool more_hint, join_server_params, ifc_ibpipe_state_t* pipe_state) {
    switch_heap(pipe_state->frame_queue_heap) {
        lwt_alloc_import(bucket);
        list_push_end(pipe_state->frame_queue, fstr_mem_t*, bucket);
        pipe_state->more_hint = more_hint;
    }
}

/// Allocates a new frame queue for the ibpipe.
static void ifc_ibpipe_new_frame_queue(ifc_ibpipe_state_t* pipe_state) {
    pipe_state->frame_queue_heap = lwt_alloc_heap();
    switch_heap(pipe_state->frame_queue_heap) {
        pipe_state->frame_queue = new_list(fstr_mem_t*);
    }
}

/// A non copying form of read that returns entire memory buckets.
join_locked(list(fstr_mem_t*)*) ifc_ibpipe_fiber_read(bool* out_more_hint, lwt_heap_t** out_heap, join_server_params, ifc_ibpipe_state_t* pipe_state) {
    // Import the frame queue into the client frame.
    lwt_heap_t* frame_queue_heap = pipe_state->frame_queue_heap;
    lwt_alloc_import(frame_queue_heap);
    list(fstr_mem_t*)* frame_queue = pipe_state->frame_queue;
    // Reset the frame queue.
    server_heap_flip {
        ifc_ibpipe_new_frame_queue(pipe_state);
    }
    if (out_more_hint != 0) {
        // There is nothing left in the buffer but more hint could still be true if the writer has indicated this.
        *out_more_hint = pipe_state->more_hint;
    }
    if (out_heap != 0) {
        // Return the frame queue heap if the callee is interested in it.
        *out_heap = frame_queue_heap;
    }
    return frame_queue;
}

fiber_main_t(ibpipe) ifc_ibpipe_fiber(fiber_main_attr) {
    try {
        ifc_ibpipe_state_t pipe_state;
        ifc_ibpipe_new_frame_queue(&pipe_state);
        pipe_state.more_hint = false;
        for (;;) {
            bool read_ready = (list_count(pipe_state.frame_queue, fstr_mem_t*) > 0);
            accept_join( \
                ifc_ibpipe_fiber_write, \
                ifc_ibpipe_fiber_read if read_ready, \
                join_server_params, \
                &pipe_state
            );
        }
    } catch (exception_canceled, e) {}
}

sf(ibpipe)* ifc_ibpipe_create() {
    fmitosis {
        return spawn_fiber(ifc_ibpipe_fiber(""));
    }
}

void ifc_ibpipe_write(fid(ibpipe) ibp, fstr_mem_t* bucket, bool more_hint) {
    try {
        ifc_ibpipe_fiber_write(bucket, more_hint, ibp.fid);
    } catch (exception_inner_join_fail, e) {
        throw_fwd("ibpipe no longer exists", exception_io, e);
    }
}

list(fstr_mem_t*)* ifc_ibpipe_read(fid(ibpipe) ibp, bool* out_more_hint, lwt_heap_t** out_heap) {
    try {
        return ifc_ibpipe_fiber_read(out_more_hint, out_heap, ibp.fid);
    } catch (exception_inner_join_fail, e) {
        throw_fwd("ibpipe no longer exists", exception_io, e);
    }
}

join_locked(void) semaphore_aquire(join_server_params, uint128_t* sem_v) {
    assert(*sem_v > 0);
    *sem_v = *sem_v - 1;
}

join_locked(void) semaphore_release(join_server_params, uint128_t* sem_v) {
    *sem_v = *sem_v + 1;
}

fiber_main_t(sem) ifc_semaphore(fiber_main_attr, uint128_t sem_v) { try {
    for (;;) {
        accept_join( \
            semaphore_release, \
            semaphore_aquire if (sem_v > 0), \
            join_server_params,
            &sem_v
        );
    }
} catch (exception_canceled, e); }

sf(sem)* ifc_semaphore_create_named(fstr_t name, uint128_t initial_v) {
    fmitosis {
        return spawn_fiber(ifc_semaphore((name.len == 0? "": fsc(name)), initial_v));
    }
}

sf(sem)* ifc_semaphore_create(uint128_t initial_v) {
    return ifc_semaphore_create_named("", initial_v);
}

void ifc_semaphore_aquire(fid(sem) s) {
    semaphore_aquire(s.fid);
}

void ifc_semaphore_release(fid(sem) s) {
    semaphore_release(s.fid);
}
