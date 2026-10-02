/*
 * C header file for ar archives.
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

#ifndef ARC_H
#define ARC_H

// Standard headers.
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/object/elf.h"

// Format
// The string that opens every archive.
#define ARC_MAGIC      "!<arch>\n"
#define ARC_MAGIC_SIZE (sizeof(ARC_MAGIC) - 1)

// The two bytes that close every member header.
#define ARC_FMAG "`\n"

// Names of the special members: the symbol index, its 64-bit form and the long-name table.
#define ARC_NAME_INDEX   "/"
#define ARC_NAME_INDEX64 "/SYM64/"
#define ARC_NAME_LONGS   "//"

// Character that ends a member's name.
#define ARC_NAME_END '/'

// What ends a name in the long-name table.
#define ARC_LONG_END "/\n"

// Longest name a member header holds itself.
#define ARC_NAME_SHORT_MAX 15

// Byte that pads a member's contents to an even length.
#define ARC_PAD_MEMBER '\n'

// Byte that pads the symbol index to an even length.
#define ARC_PAD_INDEX '\0'

// Date, owner and group of every member.
#define ARC_FIELD_ZERO "0"

// Mode of every ordinary member.
#define ARC_MODE_MEMBER "644"

// Bytes in one word of the symbol index.
#define ARC_INDEX_WORD 4

// Largest offset the symbol index can hold.
#define ARC_INDEX_MAX UINT32_MAX

// Base of the decimal fields in a member header.
#define ARC_DECIMAL_BASE 10

// Members an archive first has room for.
#define ARC_MEMBERS_FIRST 16

// Why an archive could not be read.
typedef enum Arc_Status Arc_Status;
enum Arc_Status {
    ARC_STATUS_OK,
    ARC_STATUS_NOT_ARCHIVE,      // no `!<arch>` magic
    ARC_STATUS_MEMBER_TRUNCATED, // a header or its contents run past the end
    ARC_STATUS_HEADER_MALFORMED, // a size or terminator that is not one
    ARC_STATUS_NAME_NOT_FOUND,   // a long name the long-name table does not hold
    ARC_STATUS_COUNT
};

// One member header, on disk.
typedef struct Arc_Hdr Arc_Hdr;
struct Arc_Hdr {
    char ar_name[16];
    char ar_date[12];
    char ar_uid[6];
    char ar_gid[6];
    char ar_mode[8];
    char ar_size[10];
    char ar_fmag[2];
};

// One member: a file's name and contents.
typedef struct Arc_Member Arc_Member;
struct Arc_Member {
    char         *am_name;    // without the slash that ends it on disk
    uint8_t      *am_data;
    size_t        am_size;    // bytes am_data holds
    const char  **am_globals; // symbols the member defines, NULL-terminated
};

// An archive: its members in order.
typedef struct Arc Arc;
struct Arc {
    Arc_Member **arc_members;    // members, in archive order
    size_t       arc_nmembers;   // members arc_members holds
    size_t       arc_capmembers; // members arc_members has room for
};

// Where a read of an archive stands.
typedef struct Arc_Reader Arc_Reader;
struct Arc_Reader {
    const uint8_t *ard_data;
    size_t         ard_size;   // bytes ard_data holds
    size_t         ard_pos;    // offset of the next member header
    const char    *ard_longs;  // the long-name table, or NULL
    size_t         ard_nlongs; // bytes ard_longs holds
};

// Archives
Arc  *Arc_New(void);
void  Arc_Free(Arc *arc);

// Members
Arc_Member *Arc_MemberAdd(Arc *arc, const char *name, const uint8_t *data, size_t size);
void        Arc_MemberReplace(Arc_Member *member, const uint8_t *data, size_t size);
Arc_Member *Arc_MemberFind(const Arc *arc, const char *name);
size_t      Arc_MemberCount(const Arc *arc);
Arc_Member *Arc_MemberAt(const Arc *arc, size_t i);
void        Arc_MemberFree(Arc_Member *member);
void        Arc_MemberDelete(Arc *arc, Arc_Member *member);

// Reading
bool       Arc_ReadMagic(const uint8_t *data, size_t n);
bool       Arc_ReadDecimal(const char *field, size_t width, size_t *value);
bool       Arc_IsName(const Arc_Hdr *hdr, const char *name);
Arc_Status Arc_ReadName(const Arc_Reader *rd, const Arc_Hdr *hdr, char **name);
Arc_Status Arc_ReadMember(Arc *arc, Arc_Reader *rd);
Arc       *Arc_ReadMem(const uint8_t *data, size_t n, Arc_Status *status);

// Writing
size_t Arc_WriteIndexSize(const Arc *arc);
size_t Arc_WriteLongsSize(const Arc *arc);
void   Arc_WriteField(char *field, size_t width, const char *text);
void   Arc_WriteHeader(FILE *out, const char *name, const char *owner, const char *mode, size_t size);
void   Arc_WriteWord(FILE *out, uint32_t value);
void   Arc_WriteIndex(const Arc *arc, FILE *out, size_t first);
void   Arc_WriteLongs(const Arc *arc, FILE *out, size_t size);
void   Arc_WriteMember(const Arc_Member *member, FILE *out, size_t *longoff);
bool   Arc_WriteFile(const Arc *arc, FILE *out);
bool   Arc_WritePath(const Arc *arc, const char *path);

#endif // ARC_H
