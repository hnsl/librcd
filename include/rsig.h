/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef RSIG_H
#define	RSIG_H

typedef struct rsig_signal_cfg {
    /// If the signal shall be passed through in the mask.
    bool pass;
    /// If the signal shall be ignored. When true, handler_fn is ignored.
    bool ignore;
    /// handler_fn should be null, writing safe signal handlers is not supported by librcd. When it's null the default action will be used.
    void* handler_fn;
} rsig_signal_cfg_t;

typedef struct rsig_full_signal_cfg {
    /// All signal configurations indexed by signal number.
    rsig_signal_cfg_t sig_cfgs[65];
} rsig_full_signal_cfg_t;

/// Resets the per-thread signal mask to the default.
void rsig_thread_signal_mask_reset();

/// This function is overridden in librcd programs that require their own
/// specific signal configuration. The default signal configuration is passed
/// in signal_cfgs.
/// Custom signal handlers should be avoided as they are unsafe.
void rsig_init_user_signal_cfg(rsig_full_signal_cfg_t* signal_cfgs);

#endif	/* RSIG_H */
