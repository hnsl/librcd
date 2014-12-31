/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef SIGNAL_INTERNAL_H
#define	SIGNAL_INTERNAL_H

typedef struct segv_rh segv_rh_t;

typedef void (*segv_rhandler_t)(void* addr, void* arg_ptr);

void rsig_thread_signal_mask_reset();

segv_rh_t* rsig_segv_rhandler_set(segv_rhandler_t segv_rh, void* addr, size_t len, void* arg_ptr);

void rsig_segv_rhandler_unset(segv_rh_t* segv_rhandler_h);

void rsig_segv_rhandler_resize(segv_rh_t* srh, size_t len);

void rsig_init();

#endif	/* SIGNAL_INTERNAL_H */

