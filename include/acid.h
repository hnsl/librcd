/*
 * File:   acid.h
 * Author: hannes
 *
 * Created on August 10, 2014, 5:35 PM
 */

#ifndef ACID_H
#define	ACID_H

/// ACID recommended "origin" address that is safe-ish to map to
/// reserve memory from. Going lower than this address is unsafe-ish due
/// to possible collision with binary/elf mappings.
#define ACID_ADDR_0 ((void*) 0x100000000000)

/// The top of the recommended address space. First non recommended address.
/// The address just below this is still recommended and has the highest risk
/// of colliding with the heap, although the heap has to be terrabyte sized
/// for that to be likely.
#define ACID_ADDR_T ((void*) 0x700000000000)

typedef struct acid acid_h;

/// Closes the acid handle and frees it. Before closing the handle the caller
/// must ensure that no concurrent threads or fibers are accessing the acid
/// memory or the handle itself as this is undefined behavior and will likely
/// crash the program. The moment this function is called the acid memory
/// range is no longer valid. This function automatically commits and fsyncs
/// before closing the acid handle.
void acid_close(acid_h* ah);

/// Opens an acid handle. The handle is low-level process global and is not
/// allocated on the heap. The mapping will be forced on the specified address
/// or the call will fail. The call is atomic, it either returns a valid acid
/// handle or it throws an exception without any side effects. On successful
/// open a sync thread is started for the acid handle. If new_length is
/// specified as non-zero the data file will be resized to the specified length
/// rounded up to the nearest page size. This is the only safe way to shrink
/// an acid data file, by opening it with a lower new_length. If the data or
/// journal file does not exist it will be created. The data file is truncated
/// to PAGE_SIZE if its smaller as that is the smallest allow data file.
/// If the acid data handle was not gracefully closed the last session the
/// journal will be automatically committed and a complete fsync is executed
/// before the call returns. If the journal is corrupt the data is removed.
/// This is opaque and not visible to the caller. Both the data and journal
/// file is locked on open. If locking fails the function throws an exception.
acid_h* acid_open(fstr_t data_path, fstr_t journal_path, void* base_addr, size_t new_length);

/// Expands the acid data file while it's open. It is completely safe to
/// use the data map in parallel while calling this function.
void acid_expand(acid_h* ah, size_t new_length);

/// Returns the complete memory range of the acid map as a fixed string.
fstr_t acid_memory(acid_h* ah);

/// Waits for compelte fsync of both journal and database to disk.
/// This function is a cancellation point and will abort wait and throw
/// cancellation/join race.
void acid_fsync(acid_h* ah);

/// Triggers a new commit of the journal unless one is already pending and
/// waits until all previous changes are synced to journal before proceeding.
/// This function is a cancellation point and will abort wait and throw
/// cancellation/join race.
void acid_fsync_journal(acid_h* ah);

/// Triggers a new commit of the journal unless one is already pending and
/// waits until all dirty pages before the call has been snapshotted and is
/// ready to be committed to journal. When snapshot is taken it returns
/// a fiber that will be cancelled when commit is complete. Waiting for
/// this fiber allows asynchronous fsync().
/// Returns 0 when snapshot could not be immediately taken.
/// This function is a cancellation point and will abort wait and throw
/// cancellation/join race.
rcd_fid_t acid_fsync_async(acid_h* ah);

/// Triggers a new commit of the journal unless one is already pending and
/// waits until all dirty pages before the call has been snapshotted and is
/// ready to be committed to journal. This call only waits for snapshot (CPU),
/// never on fsync (disk). If the sync thread is busy flushing to disk the
/// function returns false without having any effect. If the function returns
/// true it is guaranteed that any previous changes is snapshotted and queued
/// to be asynchronously committed to disk.
/// This function is a cancellation point and will abort wait and throw
/// cancellation/join race.
bool acid_snapshot(acid_h* ah);

#endif	/* ACID_H */
