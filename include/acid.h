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

acid_h* acid_open(fstr_t data_path, fstr_t journal_path, void* base_addr, size_t new_length);

void acid_expand(acid_h* ah, size_t new_length);

fstr_t acid_memory(acid_h* ah);

void acid_fsync(acid_h* ah);

bool acid_commit_hint(acid_h* ah);

#endif	/* ACID_H */
