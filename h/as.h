/*
 * C header file for the ivanas assembler.
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

#ifndef AS_H
#define AS_H

// Standard headers.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/str.h"
#include "arch/x86_64/enc.h"
#include "arch/x86_64/txt.h"

// Usage
void  As_Usage(const char *prog);

// Assembling
char *As_ReadSource(const char *path);
void  As_Assemble(const char *input, const char *output);

#endif // AS_H
