/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef REFLECT_H
#define	REFLECT_H

/// Looks up the line of an instruction address in librcd debug reflection tables.
/// If the function returns true the address was found and out_line is set.
bool rfl_addr_to_line(void* i_addr, uint32_t* out_line);

/// Looks up the file of an instruction address in librcd debug reflection tables.
/// If the function returns true the address was found and out_file is set.
bool rfl_addr_to_file(void* i_addr, fstr_t* out_file);

/// Looks up the line of an instruction address in librcd debug reflection tables.
/// If the function returns true the address was found and out_func is set.
bool rfl_addr_to_func(void* i_addr, fstr_t* out_func);

/// Generates a pretty but compact human readable source location of an instruction address.
fstr_mem_t* rfl_addr_to_location(void* i_addr);

/// Archaic in-place pretty printing of a location that fills the specified buffer without doing any allocation.
fstr_t rfl_addr_to_location_inp(fstr_t buffer, void* i_addr);

#endif	/* REFLECT_H */
