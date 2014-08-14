/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#ifndef DTOA_H
#define DTOA_H

/// dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
char* dtoa(double dd, int mode, int ndigits, int* decpt, int* sign, char** rve);

/// freedtoa(s) must be used to free values s returned by dtoa
void freedtoa(char* s);

#endif /* DTOA_H */
