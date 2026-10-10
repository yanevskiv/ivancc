/*
 * C header file for the ivanld linker.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LD_H
#define LD_H

// Standard headers.
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/elf.h"
#include "util/object/link.h"
#include "util/str.h"

// Permission bits for the executable ld writes (rwxr-xr-x).
#define LD_MODE 0755

// Default output name when no -o is given.
#define LD_DEFAULT_OUTPUT "a.out"

// Usage
void  Ld_Usage(const char *prog);

// Placement
char *Ld_PlaceName(const char *spec, size_t len);
void  Ld_ParsePlace(const char *spec, Link_Options *opts);

#endif // LD_H
