/*
 * C source file for the ivanar archiver.
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

// Standard headers.
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/arc.h"
#include "util/object/elf.h"

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

// Show usage information and exit.
static void Ar_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [-]{d|r|t|x}[cDs] ARCHIVE [FILE...]\n"
        "       %s [-]s[cD] ARCHIVE\n"
        "  d   delete each FILE from ARCHIVE\n"
        "  r   add each FILE to ARCHIVE, replacing the member of its name\n"
        "  t   list the members of ARCHIVE, or each FILE among them\n"
        "  x   extract the members of ARCHIVE, or each FILE among them\n"
        "  c   create ARCHIVE without saying so\n"
        "  s   write the symbol index, which every change rewrites anyway\n"
        "  D   write zero dates, owners and modes, as ivanar always does\n",
        prog, prog);
    exit(1);
}

// Set the operation the letters name, refusing a second one.
static void Ar_SetOp(const char *prog, Ar_Options *opts, Ar_Op op)
{
    if (opts->ao_op != AR_OP_NONE) {
        Ar_Usage(prog);
    }
    opts->ao_op = op;
}

// Parse the letters bundled in the first argument.
static Ar_Options Ar_Parse(const char *prog, const char *letters)
{
    Ar_Options opts = {
        .ao_op    = AR_OP_NONE,
        .ao_quiet = false,
        .ao_index = false
    };

    const char *ptr = letters[0] == AR_DASH ? letters + 1 : letters;
    for (; *ptr != '\0'; ptr++) {
        switch (*ptr) {
            case 'd': {
                Ar_SetOp(prog, &opts, AR_OP_DELETE);
            } break;
            case 'r': {
                Ar_SetOp(prog, &opts, AR_OP_REPLACE);
            } break;
            case 't': {
                Ar_SetOp(prog, &opts, AR_OP_LIST);
            } break;
            case 'x': {
                Ar_SetOp(prog, &opts, AR_OP_EXTRACT);
            } break;
            case 'c': {
                opts.ao_quiet = true;
            } break;
            case 's': {
                opts.ao_index = true;
            } break;
            case 'D': {
                // empty
            } break;
            default: {
                Ar_Usage(prog);
            } break;
        }
    }

    if (opts.ao_op == AR_OP_NONE && opts.ao_index) {
        opts.ao_op = AR_OP_INDEX;
    }
    if (opts.ao_op == AR_OP_NONE) {
        Ar_Usage(prog);
    }
    return opts;
}

// Return the file name a path ends in.
static const char *Ar_Basename(const char *path)
{
    const char *sep = strrchr(path, AR_PATH_SEP);
    return sep != NULL ? sep + 1 : path;
}

// Read the archive at path, or start an empty one where r may create it.
static Arc *Ar_Open(const char *path, const Ar_Options *opts)
{
    size_t len = 0;
    Arc_Status status = ARC_STATUS_OK;
    uint8_t *bytes = Elf_ReadBytes(path, &len);

    if (! bytes && errno == ENOENT && opts->ao_op == AR_OP_REPLACE) {
        if (! opts->ao_quiet) {
            Log_ShowInfo("creating %s", path);
        }
        return Arc_New();
    }
    Err_Assert(bytes, ERR_AR_ARCHIVE_NOT_READABLE, path, strerror(errno));

    Arc *arc = Arc_ReadMem(bytes, len, &status);
    free(bytes);
    Err_Assert(status != ARC_STATUS_NOT_ARCHIVE, ERR_AR_NOT_ARCHIVE, path);
    Err_Assert(status != ARC_STATUS_MEMBER_TRUNCATED, ERR_AR_MEMBER_TRUNCATED, path);
    Err_Assert(status != ARC_STATUS_HEADER_MALFORMED, ERR_AR_HEADER_MALFORMED, path);
    Err_Assert(status != ARC_STATUS_NAME_NOT_FOUND, ERR_AR_NAME_NOT_FOUND, path);
    return arc;
}

// Find the member a file names, refusing one the archive lacks.
static Arc_Member *Ar_Find(const Arc *arc, const char *path, const char *file)
{
    Arc_Member *member = Arc_MemberFind(arc, Ar_Basename(file));
    Err_Assert(member, ERR_AR_MEMBER_NOT_FOUND, file, path);
    return member;
}

// Add each file, replacing a member of its name the archive held before.
static void Ar_Replace(Arc *arc, const char *const *files, size_t nfiles)
{
    size_t nheld = Arc_MemberCount(arc);
    bool *replaced = calloc(nheld + 1, sizeof(*replaced));

    for (size_t i = 0; i < nfiles; i++) {
        size_t len = 0;
        size_t held = 0;
        const char *name = Ar_Basename(files[i]);
        uint8_t *bytes = Elf_ReadBytes(files[i], &len);
        Err_Assert(bytes, ERR_AR_INPUT_NOT_READABLE, files[i], strerror(errno));
        while (held < nheld && (replaced[held] || strcmp(Arc_MemberAt(arc, held)->am_name, name) != 0)) {
            held++;
        }
        if (held < nheld) {
            Arc_MemberReplace(Arc_MemberAt(arc, held), bytes, len);
            replaced[held] = true;
        } else {
            Arc_MemberAdd(arc, name, bytes, len);
        }
        free(bytes);
    }
    free(replaced);
}

// Delete the member each file names.
static void Ar_Delete(Arc *arc, const char *path, const char *const *files, size_t nfiles)
{
    for (size_t i = 0; i < nfiles; i++) {
        Arc_MemberDelete(arc, Ar_Find(arc, path, files[i]));
    }
}

// Print the name of every member, or of each one a file names.
static void Ar_List(const Arc *arc, const char *path, const char *const *files, size_t nfiles)
{
    if (nfiles == 0) {
        for (size_t i = 0; i < Arc_MemberCount(arc); i++) {
            printf("%s\n", Arc_MemberAt(arc, i)->am_name);
        }
        return;
    }
    for (size_t i = 0; i < nfiles; i++) {
        printf("%s\n", Ar_Find(arc, path, files[i])->am_name);
    }
}

// Write a member to the file of its name in the current directory.
static void Ar_ExtractOne(const Arc_Member *member)
{
    Err_Assert(! strchr(member->am_name, AR_PATH_SEP), ERR_AR_EXTRACT_NAME_NOT_PLAIN, member->am_name);

    FILE *out = fopen(member->am_name, "wb");
    bool written = out != NULL && fwrite(member->am_data, 1, member->am_size, out) == member->am_size;
    bool closed = out != NULL && fclose(out) == 0;
    Err_Assert(written && closed, ERR_AR_EXTRACT_NOT_WRITEABLE, member->am_name, strerror(errno));
}

// Extract every member, or each one a file names.
static void Ar_Extract(const Arc *arc, const char *path, const char *const *files, size_t nfiles)
{
    if (nfiles == 0) {
        for (size_t i = 0; i < Arc_MemberCount(arc); i++) {
            Ar_ExtractOne(Arc_MemberAt(arc, i));
        }
        return;
    }
    for (size_t i = 0; i < nfiles; i++) {
        Ar_ExtractOne(Ar_Find(arc, path, files[i]));
    }
}

// Write the archive back to path.
static void Ar_Save(const Arc *arc, const char *path)
{
    Err_Assert(Arc_WritePath(arc, path), ERR_AR_OUTPUT_NOT_WRITEABLE, path, strerror(errno));
}

// Main function
int main(int argc, char **argv)
{
    Log_SetProgramName(argv[0]);

    if (argc < AR_ARG_FILES) {
        Ar_Usage(argv[0]);
    }

    Ar_Options opts = Ar_Parse(argv[0], argv[AR_ARG_LETTERS]);
    const char *path = argv[AR_ARG_ARCHIVE];
    const char *const *files = (const char *const *) argv + AR_ARG_FILES;
    size_t nfiles = (size_t) (argc - AR_ARG_FILES);

    if (opts.ao_op == AR_OP_INDEX && nfiles != 0) {
        Ar_Usage(argv[0]);
    }

    Arc *arc = Ar_Open(path, &opts);

    switch (opts.ao_op) {
        case AR_OP_INDEX: {
            Ar_Save(arc, path);
        } break;
        case AR_OP_DELETE: {
            Ar_Delete(arc, path, files, nfiles);
            Ar_Save(arc, path);
        } break;
        case AR_OP_REPLACE: {
            Ar_Replace(arc, files, nfiles);
            Ar_Save(arc, path);
        } break;
        case AR_OP_LIST: {
            Ar_List(arc, path, files, nfiles);
        } break;
        case AR_OP_EXTRACT: {
            Ar_Extract(arc, path, files, nfiles);
        } break;
        case AR_OP_NONE:
        case AR_OP_COUNT: {
            // empty
        } break;
    }

    Arc_Free(arc);
    return 0;
}
