/*
 * C source file for libraries.
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

// Module header.
#include "util/object/lib.h"

// Create an empty archive.
Lib_Ar *Lib_ArNew(void)
{
    return calloc(1, sizeof(Lib_Ar));
}

// Free an archive and every member it holds.
void Lib_ArFree(Lib_Ar *ar)
{
    if (! ar) {
        return;
    }
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        Lib_ArMemberFree(ar->la_members[i]);
    }
    free(ar->la_members);
    free(ar);
}

// Append a copy of a file as a new member.
Lib_ArMember *Lib_ArMemberAdd(Lib_Ar *ar, const char *name, const uint8_t *data, size_t size)
{
    Lib_ArMember *member = calloc(1, sizeof(*member));
    member->lam_name = malloc(strlen(name) + 1);
    strcpy(member->lam_name, name);
    Lib_ArMemberReplace(member, data, size);

    if (ar->la_nmembers == ar->la_capmembers) {
        ar->la_capmembers = ar->la_capmembers != 0 ? ar->la_capmembers * 2 : LIB_AR_MEMBERS_FIRST;
        ar->la_members = realloc(ar->la_members, ar->la_capmembers * sizeof(*ar->la_members));
    }
    ar->la_members[ar->la_nmembers++] = member;
    return member;
}

// Replace a member's contents with a copy of a file's.
void Lib_ArMemberReplace(Lib_ArMember *member, const uint8_t *data, size_t size)
{
    free(member->lam_data);
    free(member->lam_globals);
    member->lam_data = malloc(size != 0 ? size : 1);
    memcpy(member->lam_data, data, size);
    member->lam_size    = size;
    member->lam_globals = Elf_ReadGlobals(member->lam_data, size);
}

// Find the first member of a name.
Lib_ArMember *Lib_ArMemberFind(const Lib_Ar *ar, const char *name)
{
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        if (strcmp(ar->la_members[i]->lam_name, name) == 0) {
            return ar->la_members[i];
        }
    }
    return NULL;
}

// Return the number of members.
size_t Lib_ArMemberCount(const Lib_Ar *ar)
{
    return ar->la_nmembers;
}

// Return member i.
Lib_ArMember *Lib_ArMemberAt(const Lib_Ar *ar, size_t i)
{
    return ar->la_members[i];
}

// Free a member and everything it owns.
void Lib_ArMemberFree(Lib_ArMember *member)
{
    free(member->lam_name);
    free(member->lam_data);
    free(member->lam_globals);
    free(member);
}

// Remove a member from its archive and free it.
void Lib_ArMemberDelete(Lib_Ar *ar, Lib_ArMember *member)
{
    size_t i = 0;
    while (i < ar->la_nmembers && ar->la_members[i] != member) {
        i++;
    }
    if (i == ar->la_nmembers) {
        return;
    }
    memmove(&ar->la_members[i], &ar->la_members[i + 1], (ar->la_nmembers - i - 1) * sizeof(*ar->la_members));
    ar->la_nmembers--;
    Lib_ArMemberFree(member);
}

// True if the bytes open with the archive magic.
bool Lib_ArReadMagic(const uint8_t *data, size_t n)
{
    return n >= LIB_AR_MAGIC_SIZE && memcmp(data, LIB_AR_MAGIC, LIB_AR_MAGIC_SIZE) == 0;
}

// Read a decimal header field padded with spaces.
bool Lib_ArReadDecimal(const char *field, size_t width, size_t *value)
{
    size_t i = 0;
    *value = 0;
    while (i < width && isdigit((unsigned char) field[i])) {
        *value = *value * LIB_AR_DECIMAL_BASE + (size_t) (field[i] - '0');
        i++;
    }
    if (i == 0) {
        return false;
    }
    while (i < width && field[i] == ' ') {
        i++;
    }
    return i == width;
}

// True if a header's name field holds exactly name.
bool Lib_ArIsName(const Lib_ArHdr *hdr, const char *name)
{
    size_t len = strlen(name);
    if (memcmp(hdr->ar_name, name, len) != 0) {
        return false;
    }
    for (size_t i = len; i < sizeof(hdr->ar_name); i++) {
        if (hdr->ar_name[i] != ' ') {
            return false;
        }
    }
    return true;
}

// Read a member's name from its header or the long-name table.
Lib_ArStatus Lib_ArReadName(const Lib_ArReader *rd, const Lib_ArHdr *hdr, char **name)
{
    size_t off = 0;
    const char *start = hdr->ar_name;
    size_t len = sizeof(hdr->ar_name);
    bool islong = hdr->ar_name[0] == LIB_AR_NAME_END;

    if (islong) {
        if (! Lib_ArReadDecimal(hdr->ar_name + 1, sizeof(hdr->ar_name) - 1, &off) || ! rd->lar_longs || off >= rd->lar_nlongs) {
            return LIB_AR_STATUS_NAME_NOT_FOUND;
        }
        start = rd->lar_longs + off;
        len = rd->lar_nlongs - off;
    }

    const char *end = memchr(start, LIB_AR_NAME_END, len);
    if (! end && islong) {
        return LIB_AR_STATUS_NAME_NOT_FOUND;
    }
    if (! end) {
        end = start + len;
        while (end > start && end[-1] == ' ') {
            end--;
        }
    }

    *name = malloc((size_t) (end - start) + 1);
    memcpy(*name, start, (size_t) (end - start));
    (*name)[end - start] = '\0';
    return LIB_AR_STATUS_OK;
}

// Read the member at the reader's position, advancing past it.
Lib_ArStatus Lib_ArReadMember(Lib_Ar *ar, Lib_ArReader *rd)
{
    size_t size = 0;
    char *name = NULL;

    if (rd->lar_size - rd->lar_pos < sizeof(Lib_ArHdr)) {
        return LIB_AR_STATUS_MEMBER_TRUNCATED;
    }
    const Lib_ArHdr *hdr = (const Lib_ArHdr *) (rd->lar_data + rd->lar_pos);
    if (memcmp(hdr->ar_fmag, LIB_AR_FMAG, sizeof(hdr->ar_fmag)) != 0 || ! Lib_ArReadDecimal(hdr->ar_size, sizeof(hdr->ar_size), &size)) {
        return LIB_AR_STATUS_HEADER_MALFORMED;
    }
    if (rd->lar_size - rd->lar_pos - sizeof(Lib_ArHdr) < size) {
        return LIB_AR_STATUS_MEMBER_TRUNCATED;
    }
    const uint8_t *body = rd->lar_data + rd->lar_pos + sizeof(Lib_ArHdr);
    rd->lar_pos += sizeof(Lib_ArHdr) + size;
    if (size % 2 != 0 && rd->lar_pos < rd->lar_size) {
        rd->lar_pos++;
    }

    // Phase: keep the long-name table, skip the symbol index and add the rest.
    if (Lib_ArIsName(hdr, LIB_AR_NAME_LONGS)) {
        rd->lar_longs  = (const char *) body;
        rd->lar_nlongs = size;
        return LIB_AR_STATUS_OK;
    }
    if (Lib_ArIsName(hdr, LIB_AR_NAME_INDEX) || Lib_ArIsName(hdr, LIB_AR_NAME_INDEX64)) {
        return LIB_AR_STATUS_OK;
    }
    Lib_ArStatus status = Lib_ArReadName(rd, hdr, &name);
    if (status != LIB_AR_STATUS_OK) {
        return status;
    }
    Lib_ArMemberAdd(ar, name, body, size);
    free(name);
    return LIB_AR_STATUS_OK;
}

// Parse archive bytes into a new archive.
Lib_Ar *Lib_ArReadMem(const uint8_t *data, size_t n, Lib_ArStatus *status)
{
    Lib_ArReader rd = {
        .lar_data   = data,
        .lar_size   = n,
        .lar_pos    = LIB_AR_MAGIC_SIZE,
        .lar_longs  = NULL,
        .lar_nlongs = 0
    };
    Lib_Ar *ar = Lib_ArNew();

    *status = Lib_ArReadMagic(data, n) ? LIB_AR_STATUS_OK : LIB_AR_STATUS_NOT_ARCHIVE;
    while (*status == LIB_AR_STATUS_OK && rd.lar_pos < n) {
        *status = Lib_ArReadMember(ar, &rd);
    }
    if (*status != LIB_AR_STATUS_OK) {
        Lib_ArFree(ar);
        return NULL;
    }
    return ar;
}

// Return the size of the symbol index's contents, padded to even.
size_t Lib_ArWriteIndexSize(const Lib_Ar *ar)
{
    size_t size = LIB_AR_INDEX_WORD;
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        for (const char **iter = ar->la_members[i]->lam_globals; *iter; iter++) {
            size += LIB_AR_INDEX_WORD + strlen(*iter) + 1;
        }
    }
    return size + size % 2;
}

// Return the size of the long-name table, padded to even.
size_t Lib_ArWriteLongsSize(const Lib_Ar *ar)
{
    size_t size = 0;
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        size_t len = strlen(ar->la_members[i]->lam_name);
        if (len > LIB_AR_NAME_SHORT_MAX) {
            size += len + strlen(LIB_AR_LONG_END);
        }
    }
    return size + size % 2;
}

// Fill a header field with text padded with spaces.
void Lib_ArWriteField(char *field, size_t width, const char *text)
{
    size_t len = strlen(text);
    memset(field, ' ', width);
    memcpy(field, text, len < width ? len : width);
}

// Write one member header.
void Lib_ArWriteHeader(FILE *out, const char *name, const char *owner, const char *mode, size_t size)
{
    Lib_ArHdr hdr;
    char text[sizeof(hdr.ar_size) + 1];

    snprintf(text, sizeof(text), "%zu", size);
    Lib_ArWriteField(hdr.ar_name, sizeof(hdr.ar_name), name);
    Lib_ArWriteField(hdr.ar_date, sizeof(hdr.ar_date), owner);
    Lib_ArWriteField(hdr.ar_uid, sizeof(hdr.ar_uid), owner);
    Lib_ArWriteField(hdr.ar_gid, sizeof(hdr.ar_gid), owner);
    Lib_ArWriteField(hdr.ar_mode, sizeof(hdr.ar_mode), mode);
    Lib_ArWriteField(hdr.ar_size, sizeof(hdr.ar_size), text);
    memcpy(hdr.ar_fmag, LIB_AR_FMAG, sizeof(hdr.ar_fmag));
    fwrite(&hdr, sizeof(hdr), 1, out);
}

// Write a big-endian word of the symbol index.
void Lib_ArWriteWord(FILE *out, uint32_t value)
{
    uint8_t bytes[LIB_AR_INDEX_WORD];
    for (size_t i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (value >> (8 * (sizeof(bytes) - 1 - i))) & 0xFF;
    }
    fwrite(bytes, sizeof(bytes), 1, out);
}

// Write the symbol index for members laid out from offset first.
void Lib_ArWriteIndex(const Lib_Ar *ar, FILE *out, size_t first)
{
    size_t count = 0;
    size_t used = LIB_AR_INDEX_WORD;
    size_t size = Lib_ArWriteIndexSize(ar);

    for (size_t i = 0; i < ar->la_nmembers; i++) {
        for (const char **iter = ar->la_members[i]->lam_globals; *iter; iter++) {
            count++;
        }
    }
    Lib_ArWriteHeader(out, LIB_AR_NAME_INDEX, LIB_AR_FIELD_ZERO, LIB_AR_FIELD_ZERO, size);
    Lib_ArWriteWord(out, (uint32_t) count);

    // Phase: the offset of the member that defines each symbol.
    size_t pos = first;
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        const Lib_ArMember *member = ar->la_members[i];
        for (const char **iter = member->lam_globals; *iter; iter++) {
            Lib_ArWriteWord(out, (uint32_t) pos);
            used += LIB_AR_INDEX_WORD;
        }
        pos += sizeof(Lib_ArHdr) + member->lam_size + member->lam_size % 2;
    }

    // Phase: the symbols' names, in the same order.
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        for (const char **iter = ar->la_members[i]->lam_globals; *iter; iter++) {
            fwrite(*iter, strlen(*iter) + 1, 1, out);
            used += strlen(*iter) + 1;
        }
    }
    if (used < size) {
        fputc(LIB_AR_PAD_INDEX, out);
    }
}

// Write the long-name table.
void Lib_ArWriteLongs(const Lib_Ar *ar, FILE *out, size_t size)
{
    size_t used = 0;

    Lib_ArWriteHeader(out, LIB_AR_NAME_LONGS, "", "", size);
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        const char *name = ar->la_members[i]->lam_name;
        if (strlen(name) > LIB_AR_NAME_SHORT_MAX) {
            fputs(name, out);
            fputs(LIB_AR_LONG_END, out);
            used += strlen(name) + strlen(LIB_AR_LONG_END);
        }
    }
    if (used < size) {
        fputc(LIB_AR_PAD_MEMBER, out);
    }
}

// Write one member, its long name at *longoff in the long-name table.
void Lib_ArWriteMember(const Lib_ArMember *member, FILE *out, size_t *longoff)
{
    char name[LIB_AR_NAME_SHORT_MAX + 2];
    size_t len = strlen(member->lam_name);

    if (len > LIB_AR_NAME_SHORT_MAX) {
        snprintf(name, sizeof(name), "%c%zu", LIB_AR_NAME_END, *longoff);
        *longoff += len + strlen(LIB_AR_LONG_END);
    } else {
        snprintf(name, sizeof(name), "%s%c", member->lam_name, LIB_AR_NAME_END);
    }
    Lib_ArWriteHeader(out, name, LIB_AR_FIELD_ZERO, LIB_AR_MODE_MEMBER, member->lam_size);
    fwrite(member->lam_data, 1, member->lam_size, out);
    if (member->lam_size % 2 != 0) {
        fputc(LIB_AR_PAD_MEMBER, out);
    }
}

// Serialize an archive to a stream.
bool Lib_ArWriteFile(const Lib_Ar *ar, FILE *out)
{
    size_t longoff = 0;
    size_t index = Lib_ArWriteIndexSize(ar);
    size_t longs = Lib_ArWriteLongsSize(ar);
    size_t first = LIB_AR_MAGIC_SIZE + sizeof(Lib_ArHdr) + index + (longs != 0 ? sizeof(Lib_ArHdr) + longs : 0);

    size_t end = first;
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        end += sizeof(Lib_ArHdr) + ar->la_members[i]->lam_size + ar->la_members[i]->lam_size % 2;
    }
    if (end > LIB_AR_INDEX_MAX) {
        return false;
    }

    fwrite(LIB_AR_MAGIC, LIB_AR_MAGIC_SIZE, 1, out);
    if (ar->la_nmembers == 0) {
        return ! ferror(out);
    }
    Lib_ArWriteIndex(ar, out, first);
    if (longs != 0) {
        Lib_ArWriteLongs(ar, out, longs);
    }
    for (size_t i = 0; i < ar->la_nmembers; i++) {
        Lib_ArWriteMember(ar->la_members[i], out, &longoff);
    }
    return ! ferror(out);
}

// Serialize an archive to a file.
bool Lib_ArWritePath(const Lib_Ar *ar, const char *path)
{
    FILE *out = fopen(path, "wb");

    if (! out) {
        return false;
    }

    bool ok = Lib_ArWriteFile(ar, out);

    return fclose(out) == 0 && ok;
}
