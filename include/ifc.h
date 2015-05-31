/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef IFC_H
#define	IFC_H

#include "rcd.h"

/// Run synchronizes another fiber. Blocks until it calls ifc_fiber_sync_recv(). Throws a join exception. Cancelation point.
void ifc_fiber_sync_send(rcd_fid_t remote_fiber_id);

/// Run synchronizes another fiber (anyone that calls it). Returns the id of the fiber that was waiting. Blocks until that happens. Cancelation point.
rcd_fid_t ifc_fiber_sync_recv();

/// Indefinitely blocks the current fiber in a deadlock - essentially parking it. Useful for static initialization.
void ifc_park();

/// Waits for the fiber. Returns when the fiber no longer exists.
/// This is sometimes called "joining" in other thread libraries.
/// This function guarantees that it does not use sub heaps making it safe
/// to call from heap destructors.
void ifc_wait(rcd_fid_t fid);

/// Triggers one or more events. A maximum of 2^64 events can be stored.
/// If all events could not be sent the call will block until all events has
/// been sent. Throws an exception_join_race if the requested event fid has
/// been canceled.
void ifc_event_trigger(rcd_fid_t event_fid, uint64_t count);

/// Waits until at least one event is ready and then returns the number
/// of pending events that was ready. This number is always larger than 0.
/// Throws an exception_join_race if the requested event fid has
/// been canceled.
uint64_t ifc_event_wait(rcd_fid_t event_fid);

/// Creates a fiber that can send one or more events. Use the returned fid
/// with ifc_event_trigger and ifc_event_wait. This is much more light-weight
/// than using a pipe. The fid can be canceled at any time.
rcd_fid_t ifc_create_event_fiber();

/// Reads from a pipe created by ifc_create_pipe. Returns a slice that was read
/// as a slice of message_dst. The slice has a length larger than zero.
fstr_t ifc_pipe_read(rcd_fid_t pipe_fid, fstr_t dst);

/// Writes to a pipe created by ifc_create_pipe. Blocks until the full
/// message was written to the pipe.
void ifc_pipe_write(rcd_fid_t pipe_fid, fstr_t src);

/// Creates a pipe with an internal circular buffer that has the specified buffer size.
rcd_sub_fiber_t* ifc_create_pipe(size_t buffer_size);

/// Fiber map fiber reference type declaration.
decl_fid_t(fmap);

/// Spawns a map of fibers. Useful when a set of fibers is required
/// to be referenced. The fiber collection can automatically cancel the
/// inserted fibers if auto_cancel is specified as true when doing
/// ifc_fiber_collection_insert(). When the inserted fibers are canceled they
/// are automatically removed from the collection and dereferenced.
/// The internal ordering of the fibers are non-lexicographical.
sf(fmap)* ifc_fiber_map_create();

/// Creates a list of fiber ids containing those that are still alive.
/// Throws a join race exception if the specified fm is invalid.
dict(rcd_fid_t)* ifc_fiber_map_list(fid(fmap) fm);

/// Creates a dict listing all keys and values in the fiber map.
/// Throws a join race exception if the specified fm is invalid.
dict(fstr_t)* ifc_fiber_map_kvlist(fid(fmap) fm);

/// Inserts a fiber id. Set to auto_cancel to automatically propagate a
/// cancellation when the collection is destroyed. If the specified fiber id
/// doesn't exist the fiber is briefly added before being removed as soon as
/// this is detected. If the specified key already exists the function
/// will return a zero fiber id.
/// Fiber ids are designed to not be synchronized and deal with
/// races "eventually" so this is the correct way to handle this condition.
/// There is no difference between a fiber id that is already dead, one
/// that dies during the duration of this call, one that dies afterwards or
/// one that has not been started yet. Returns the id of the internal
/// monitoring fiber.
/// Throws a join race exception if the specified fm is invalid.
rcd_fid_t ifc_fiber_map_insert(fid(fmap) fm, fstr_t key, rcd_fid_t remote_fid, bool auto_cancel);

/// Like ifc_fiber_map_insert() but also associates a data record.
rcd_fid_t ifc_fiber_map_insert_data(fid(fmap) fm, fstr_t key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel);

/// Like ifc_fiber_map_insert() but deletes the record it contains on collision rather than throwing an exception and then replacing the record.
rcd_fid_t ifc_fiber_map_replace(fid(fmap) fm, fstr_t key, rcd_fid_t remote_fid, bool auto_cancel);

/// Like ifc_fiber_map_insert_data() but deletes the record it contains on collision rather than throwing an exception and then replacing the record and the data.
rcd_fid_t ifc_fiber_map_replace_data(fid(fmap) fm, fstr_t key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel);

/// Returns the fiber id associated with a certain key.
/// Returns 0 if the specified key does not exist.
/// Throws a join race exception if the specified fm is invalid.
rcd_fid_t ifc_fiber_map_read(fid(fmap) fm, fstr_t key);

/// Returns the fiber id and data associated with a certain key.
/// If out_data is 0 the function is the same as ifc_fiber_map_read().
/// Returns 0 if the specified key does not exist.
/// Throws a join race exception if the specified fm is invalid.
rcd_fid_t ifc_fiber_map_read_data(fid(fmap) fm, fstr_t key, fstr_mem_t** out_data);

/// Removes a fiber id from being monitored by the collection. The monitored
/// fiber is not canceled if auto_cancel was specified when it was inserted.
/// If the fiber id does not exist in the collection for various reasons
/// (e.g. it died; it was never added to begin with) the function has no
/// effect and returns without modifying the collection.
/// Throws a join race exception if the specified fm is invalid.
void ifc_fiber_map_remove(fid(fmap) fm, fstr_t key);

/// Conditionally removes a fiber id from being monitored by the collection
/// only if the specified key still has the specified monitor fid else have
/// no effect. Otherwise works exactly like ifc_fiber_map_remove().
/// This function is required to avoid races introduced when cleaning up
/// keys that can concurrently be cleaned up and reused from other contexts.
void ifc_fiber_map_cond_remove(fid(fmap) fm, rcd_fid_t validate_monitor_fid, fstr_t key);

/// Enqueues a fiber with the next available key in non lexicographical order.
/// If out_key is non 0 the generated key is returned in a new string.
/// Throws a join race exception if the specified fm is invalid.
/// Returns the id of the internal monitoring fiber.
/// See ifc_fiber_map_insert() documentation for more information.
rcd_fid_t ifc_fiber_map_enqueue(fid(fmap) fm, fstr_mem_t** out_key, rcd_fid_t remote_fid, bool auto_cancel);

/// Like ifc_fiber_map_enqueue() but allows setting data associated with the entry.
/// If data is "", this function is equivalent to ifc_fiber_map_enqueue().
rcd_fid_t ifc_fiber_map_enqueue_data(fid(fmap) fm, fstr_mem_t** out_key, fstr_t data, rcd_fid_t remote_fid, bool auto_cancel);

/// Dequeues a fiber with the first key in non lexicographical order.
/// The function returns the fid of the fiber that was dequeued or 0 if the
/// internal collection is empty.
/// If out_key is non 0 the key is returned in a new string if the function
/// returns a non 0 fid.
/// See ifc_fiber_map_remove() documentation for more information.
/// Throws a join race exception if the specified fm is invalid.
rcd_fid_t ifc_fiber_map_dequeue(fid(fmap) fm, fstr_mem_t** out_key);

/// Like ifc_fiber_map_dequeue() but also returns the data associated with the entry.
/// If out_data is 0, this function is equivalent to ifc_fiber_map_dequeue().
rcd_fid_t ifc_fiber_map_dequeue_data(fid(fmap) fm, fstr_mem_t** out_key, fstr_mem_t** out_data);

/// Creates a sub fiber that waits the specified time and then cancels the target fiber.
rcd_sub_fiber_t* ifc_cancel_alarm_arm_fid(uint128_t alarm_timeout_ns, rcd_fid_t target_fid);

/// Creates a sub fiber that waits the specified time and then cancels the current fiber.
rcd_sub_fiber_t* ifc_cancel_alarm_arm(uint128_t alarm_timeout_ns);

/// Creates a sub fiber that waits for the specified fiber to exit and then cancels the target fiber.
rcd_sub_fiber_t* ifc_cancel_on_exit_arm_fid(rcd_fid_t wait_fid, rcd_fid_t target_fid);

/// Creates a sub fiber that waits for the specified fiber to exit and then cancels the current fiber.
rcd_sub_fiber_t* ifc_cancel_on_exit_arm(rcd_fid_t wait_fid);

/// Creates an "infinite pipe" that never write blocks. It is optimized for
/// large numbers of small writes and can buffer them efficiently.
/// Internally allocates more memory if writing is faster than reading.
/// Using this pipe with external I/O usually requires some sort of
/// acknowledge in the protocol that prevents out-of-memory conditions.
sf(ipipe)* ifc_ipipe_create(rio_t** inf_pipe_r_out, rio_t** inf_pipe_w_out, size_t max_buf_len);

/// Infinite buffer pipe fiber reference type declaration.
decl_fid_t(ibpipe);

/// Creates an "infinite bucket pipe" that never write blocks.
/// Like an infinite pipe but uses a bucket interface instead to avoid
/// any memory copying.
sf(ibpipe)* ifc_ibpipe_create();

/// Writes a bucket of memory to the infinite pipe. Useful for large writes
/// where an extra copying operation is removed by acquiring ownership of the
/// data (alloc import) rather than copying it.
/// Throws an io exception if pipe no longer exists.
void ifc_ibpipe_write(fid(ibpipe) ibp, fstr_mem_t* bucket, bool more_hint);

/// Reads all pending buckets of memory from the infinite pipe.
/// A hint if more data will be immediately available after the read is
/// returned in the out_more_hint if it's not null. The callee should use this
/// value to optimize transfer speed if the data is forwarded.
/// If out_heap is not null the heap the bucket list is allocated on will be
/// returned through the parameter.
/// Throws an arg exception if out_bucket_fstr is not null.
/// Throws an io exception if pipe no longer exists.
list(fstr_mem_t*)* ifc_ibpipe_read(fid(ibpipe) ibp, bool* out_more_hint, lwt_heap_t** out_heap);

/// Semaphore fiber reference type declaration.
decl_fid_t(sem);

/// Creates a new semaphore. Allows limiting the use of an abstract resource.
sf(sem)* ifc_semaphore_create(uint128_t initial_v);

/// Acquires an abstract semaphore resource. Decreases the internal counter by
/// one. Blocks until the internal counter is non zero.
void ifc_semaphore_aquire(fid(sem) s);

/// Acquires an abstract semaphore resource. Decreases the internal counter by
/// one. Blocks until the internal counter is non zero.
void ifc_semaphore_release(fid(sem) s);

#endif	/* IFC_H */
