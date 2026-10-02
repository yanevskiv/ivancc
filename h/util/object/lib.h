/*
 * C header file for libraries.
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

#ifndef LIB_H
#define LIB_H

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
#define LIB_AR_MAGIC      "!<arch>\n"
#define LIB_AR_MAGIC_SIZE (sizeof(LIB_AR_MAGIC) - 1)

// The two bytes that close every member header.
#define LIB_AR_FMAG "`\n"

// Names of the special members: the symbol index, its 64-bit form and the long-name table.
#define LIB_AR_NAME_INDEX   "/"
#define LIB_AR_NAME_INDEX64 "/SYM64/"
#define LIB_AR_NAME_LONGS   "//"

// Character that ends a member's name.
#define LIB_AR_NAME_END '/'

// What ends a name in the long-name table.
#define LIB_AR_LONG_END "/\n"

// Longest name a member header holds itself.
#define LIB_AR_NAME_SHORT_MAX 15

// Byte that pads a member's contents to an even length.
#define LIB_AR_PAD_MEMBER '\n'

// Byte that pads the symbol index to an even length.
#define LIB_AR_PAD_INDEX '\0'

// Date, owner and group of every member.
#define LIB_AR_FIELD_ZERO "0"

// Mode of every ordinary member.
#define LIB_AR_MODE_MEMBER "644"

// Bytes in one word of the symbol index.
#define LIB_AR_INDEX_WORD 4

// Largest offset the symbol index can hold.
#define LIB_AR_INDEX_MAX UINT32_MAX

// Base of the decimal fields in a member header.
#define LIB_AR_DECIMAL_BASE 10

// Members an archive first has room for.
#define LIB_AR_MEMBERS_FIRST 16

// Why an archive could not be read.
typedef enum Lib_ArStatus Lib_ArStatus;
enum Lib_ArStatus {
    LIB_AR_STATUS_OK,
    LIB_AR_STATUS_NOT_ARCHIVE,      // no `!<arch>` magic
    LIB_AR_STATUS_MEMBER_TRUNCATED, // a header or its contents run past the end
    LIB_AR_STATUS_HEADER_MALFORMED, // a size or terminator that is not one
    LIB_AR_STATUS_NAME_NOT_FOUND,   // a long name the long-name table does not hold
    LIB_AR_STATUS_COUNT
};

// One member header, on disk.
typedef struct Lib_ArHdr Lib_ArHdr;
struct Lib_ArHdr {
    char ar_name[16];
    char ar_date[12];
    char ar_uid[6];
    char ar_gid[6];
    char ar_mode[8];
    char ar_size[10];
    char ar_fmag[2];
};

// One member: a file's name and contents.
typedef struct Lib_ArMember Lib_ArMember;
struct Lib_ArMember {
    char         *lam_name;    // without the slash that ends it on disk
    uint8_t      *lam_data;
    size_t        lam_size;    // bytes lam_data holds
    const char  **lam_globals; // symbols the member defines, NULL-terminated
};

// An archive: its members in order.
typedef struct Lib_Ar Lib_Ar;
struct Lib_Ar {
    Lib_ArMember **la_members;    // members, in archive order
    size_t         la_nmembers;   // members la_members holds
    size_t         la_capmembers; // members la_members has room for
};

// Where a read of an archive stands.
typedef struct Lib_ArReader Lib_ArReader;
struct Lib_ArReader {
    const uint8_t *lar_data;
    size_t         lar_size;   // bytes lar_data holds
    size_t         lar_pos;    // offset of the next member header
    const char    *lar_longs;  // the long-name table, or NULL
    size_t         lar_nlongs; // bytes lar_longs holds
};

// Archives
Lib_Ar *Lib_ArNew(void);
void    Lib_ArFree(Lib_Ar *ar);

// Members
Lib_ArMember *Lib_ArMemberAdd(Lib_Ar *ar, const char *name, const uint8_t *data, size_t size);
void          Lib_ArMemberReplace(Lib_ArMember *member, const uint8_t *data, size_t size);
Lib_ArMember *Lib_ArMemberFind(const Lib_Ar *ar, const char *name);
size_t        Lib_ArMemberCount(const Lib_Ar *ar);
Lib_ArMember *Lib_ArMemberAt(const Lib_Ar *ar, size_t i);
void          Lib_ArMemberFree(Lib_ArMember *member);
void          Lib_ArMemberDelete(Lib_Ar *ar, Lib_ArMember *member);

// Reading
bool          Lib_ArReadMagic(const uint8_t *data, size_t n);
bool          Lib_ArReadDecimal(const char *field, size_t width, size_t *value);
bool          Lib_ArIsName(const Lib_ArHdr *hdr, const char *name);
Lib_ArStatus  Lib_ArReadName(const Lib_ArReader *rd, const Lib_ArHdr *hdr, char **name);
Lib_ArStatus  Lib_ArReadMember(Lib_Ar *ar, Lib_ArReader *rd);
Lib_Ar       *Lib_ArReadMem(const uint8_t *data, size_t n, Lib_ArStatus *status);

// Writing
size_t Lib_ArWriteIndexSize(const Lib_Ar *ar);
size_t Lib_ArWriteLongsSize(const Lib_Ar *ar);
void   Lib_ArWriteField(char *field, size_t width, const char *text);
void   Lib_ArWriteHeader(FILE *out, const char *name, const char *owner, const char *mode, size_t size);
void   Lib_ArWriteWord(FILE *out, uint32_t value);
void   Lib_ArWriteIndex(const Lib_Ar *ar, FILE *out, size_t first);
void   Lib_ArWriteLongs(const Lib_Ar *ar, FILE *out, size_t size);
void   Lib_ArWriteMember(const Lib_ArMember *member, FILE *out, size_t *longoff);
bool   Lib_ArWriteFile(const Lib_Ar *ar, FILE *out);
bool   Lib_ArWritePath(const Lib_Ar *ar, const char *path);

#endif // LIB_H
