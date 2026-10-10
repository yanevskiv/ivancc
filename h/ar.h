/*
 * C header file for the ivanar archiver.
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

#ifndef AR_H
#define AR_H

// Standard headers.
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/elf.h"
#include "util/object/lib.h"

// Positions of the letters, the archive and the first file on the command line.
#define AR_ARG_LETTERS 1
#define AR_ARG_ARCHIVE 2
#define AR_ARG_FILES   3

// Character that may open the letters.
#define AR_DASH '-'

// Character that separates the directories of a path.
#define AR_PATH_SEP '/'

// Operations the letters name.
typedef enum Ar_Op Ar_Op;
enum Ar_Op {
    AR_OP_NONE,
    AR_OP_INDEX,   // s alone, as ranlib
    AR_OP_DELETE,
    AR_OP_REPLACE,
    AR_OP_LIST,
    AR_OP_EXTRACT,
    AR_OP_COUNT
};

// What the letters ask for.
typedef struct Ar_Options Ar_Options;
struct Ar_Options {
    Ar_Op ao_op;
    bool  ao_quiet; // c: create the archive without saying so
    bool  ao_index; // s: write the symbol index
};

// Usage
void Ar_Usage(const char *prog);

// Options
void       Ar_SetOp(const char *prog, Ar_Options *opts, Ar_Op op);
Ar_Options Ar_Parse(const char *prog, const char *letters);

// Paths
const char *Ar_Basename(const char *path);

// Archives
Lib_Ar       *Ar_Open(const char *path, const Ar_Options *opts);
Lib_ArMember *Ar_Find(const Lib_Ar *ar, const char *path, const char *file);
void          Ar_Replace(Lib_Ar *ar, const char *const *files, size_t nfiles);
void          Ar_Delete(Lib_Ar *ar, const char *path, const char *const *files, size_t nfiles);
void          Ar_List(const Lib_Ar *ar, const char *path, const char *const *files, size_t nfiles);
void          Ar_ExtractOne(const Lib_ArMember *member);
void          Ar_Extract(const Lib_Ar *ar, const char *path, const char *const *files, size_t nfiles);
void          Ar_Save(const Lib_Ar *ar, const char *path);

#endif // AR_H
