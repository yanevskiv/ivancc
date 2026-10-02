/*
 * C source file for ar archives.
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
#include "util/object/arc.h"

// Create an empty archive.
Arc *Arc_New(void)
{
    return calloc(1, sizeof(Arc));
}

// Free an archive and every member it holds.
void Arc_Free(Arc *arc)
{
    if (! arc) {
        return;
    }
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        Arc_MemberFree(arc->arc_members[i]);
    }
    free(arc->arc_members);
    free(arc);
}

// Append a copy of a file as a new member.
Arc_Member *Arc_MemberAdd(Arc *arc, const char *name, const uint8_t *data, size_t size)
{
    Arc_Member *member = calloc(1, sizeof(*member));
    member->am_name = malloc(strlen(name) + 1);
    strcpy(member->am_name, name);
    Arc_MemberReplace(member, data, size);

    if (arc->arc_nmembers == arc->arc_capmembers) {
        arc->arc_capmembers = arc->arc_capmembers != 0 ? arc->arc_capmembers * 2 : ARC_MEMBERS_FIRST;
        arc->arc_members = realloc(arc->arc_members, arc->arc_capmembers * sizeof(*arc->arc_members));
    }
    arc->arc_members[arc->arc_nmembers++] = member;
    return member;
}

// Replace a member's contents with a copy of a file's.
void Arc_MemberReplace(Arc_Member *member, const uint8_t *data, size_t size)
{
    free(member->am_data);
    free(member->am_globals);
    member->am_data = malloc(size != 0 ? size : 1);
    memcpy(member->am_data, data, size);
    member->am_size    = size;
    member->am_globals = Elf_ReadGlobals(member->am_data, size);
}

// Find the first member of a name.
Arc_Member *Arc_MemberFind(const Arc *arc, const char *name)
{
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        if (strcmp(arc->arc_members[i]->am_name, name) == 0) {
            return arc->arc_members[i];
        }
    }
    return NULL;
}

// Return the number of members.
size_t Arc_MemberCount(const Arc *arc)
{
    return arc->arc_nmembers;
}

// Return member i.
Arc_Member *Arc_MemberAt(const Arc *arc, size_t i)
{
    return arc->arc_members[i];
}

// Free a member and everything it owns.
void Arc_MemberFree(Arc_Member *member)
{
    free(member->am_name);
    free(member->am_data);
    free(member->am_globals);
    free(member);
}

// Remove a member from its archive and free it.
void Arc_MemberDelete(Arc *arc, Arc_Member *member)
{
    size_t i = 0;
    while (i < arc->arc_nmembers && arc->arc_members[i] != member) {
        i++;
    }
    if (i == arc->arc_nmembers) {
        return;
    }
    memmove(&arc->arc_members[i], &arc->arc_members[i + 1], (arc->arc_nmembers - i - 1) * sizeof(*arc->arc_members));
    arc->arc_nmembers--;
    Arc_MemberFree(member);
}

// True if the bytes open with the archive magic.
bool Arc_ReadMagic(const uint8_t *data, size_t n)
{
    return n >= ARC_MAGIC_SIZE && memcmp(data, ARC_MAGIC, ARC_MAGIC_SIZE) == 0;
}

// Read a decimal header field padded with spaces.
bool Arc_ReadDecimal(const char *field, size_t width, size_t *value)
{
    size_t i = 0;
    *value = 0;
    while (i < width && isdigit((unsigned char) field[i])) {
        *value = *value * ARC_DECIMAL_BASE + (size_t) (field[i] - '0');
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
bool Arc_IsName(const Arc_Hdr *hdr, const char *name)
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
Arc_Status Arc_ReadName(const Arc_Reader *rd, const Arc_Hdr *hdr, char **name)
{
    size_t off = 0;
    const char *start = hdr->ar_name;
    size_t len = sizeof(hdr->ar_name);
    bool islong = hdr->ar_name[0] == ARC_NAME_END;

    if (islong) {
        if (! Arc_ReadDecimal(hdr->ar_name + 1, sizeof(hdr->ar_name) - 1, &off) || ! rd->ard_longs || off >= rd->ard_nlongs) {
            return ARC_STATUS_NAME_NOT_FOUND;
        }
        start = rd->ard_longs + off;
        len = rd->ard_nlongs - off;
    }

    const char *end = memchr(start, ARC_NAME_END, len);
    if (! end && islong) {
        return ARC_STATUS_NAME_NOT_FOUND;
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
    return ARC_STATUS_OK;
}

// Read the member at the reader's position, advancing past it.
Arc_Status Arc_ReadMember(Arc *arc, Arc_Reader *rd)
{
    size_t size = 0;
    char *name = NULL;

    if (rd->ard_size - rd->ard_pos < sizeof(Arc_Hdr)) {
        return ARC_STATUS_MEMBER_TRUNCATED;
    }
    const Arc_Hdr *hdr = (const Arc_Hdr *) (rd->ard_data + rd->ard_pos);
    if (memcmp(hdr->ar_fmag, ARC_FMAG, sizeof(hdr->ar_fmag)) != 0 || ! Arc_ReadDecimal(hdr->ar_size, sizeof(hdr->ar_size), &size)) {
        return ARC_STATUS_HEADER_MALFORMED;
    }
    if (rd->ard_size - rd->ard_pos - sizeof(Arc_Hdr) < size) {
        return ARC_STATUS_MEMBER_TRUNCATED;
    }
    const uint8_t *body = rd->ard_data + rd->ard_pos + sizeof(Arc_Hdr);
    rd->ard_pos += sizeof(Arc_Hdr) + size;
    if (size % 2 != 0 && rd->ard_pos < rd->ard_size) {
        rd->ard_pos++;
    }

    // Phase: keep the long-name table, skip the symbol index and add the rest.
    if (Arc_IsName(hdr, ARC_NAME_LONGS)) {
        rd->ard_longs  = (const char *) body;
        rd->ard_nlongs = size;
        return ARC_STATUS_OK;
    }
    if (Arc_IsName(hdr, ARC_NAME_INDEX) || Arc_IsName(hdr, ARC_NAME_INDEX64)) {
        return ARC_STATUS_OK;
    }
    Arc_Status status = Arc_ReadName(rd, hdr, &name);
    if (status != ARC_STATUS_OK) {
        return status;
    }
    Arc_MemberAdd(arc, name, body, size);
    free(name);
    return ARC_STATUS_OK;
}

// Parse archive bytes into a new archive.
Arc *Arc_ReadMem(const uint8_t *data, size_t n, Arc_Status *status)
{
    Arc_Reader rd = {
        .ard_data   = data,
        .ard_size   = n,
        .ard_pos    = ARC_MAGIC_SIZE,
        .ard_longs  = NULL,
        .ard_nlongs = 0
    };
    Arc *arc = Arc_New();

    *status = Arc_ReadMagic(data, n) ? ARC_STATUS_OK : ARC_STATUS_NOT_ARCHIVE;
    while (*status == ARC_STATUS_OK && rd.ard_pos < n) {
        *status = Arc_ReadMember(arc, &rd);
    }
    if (*status != ARC_STATUS_OK) {
        Arc_Free(arc);
        return NULL;
    }
    return arc;
}

// Return the size of the symbol index's contents, padded to even.
size_t Arc_WriteIndexSize(const Arc *arc)
{
    size_t size = ARC_INDEX_WORD;
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        for (const char **iter = arc->arc_members[i]->am_globals; *iter; iter++) {
            size += ARC_INDEX_WORD + strlen(*iter) + 1;
        }
    }
    return size + size % 2;
}

// Return the size of the long-name table, padded to even.
size_t Arc_WriteLongsSize(const Arc *arc)
{
    size_t size = 0;
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        size_t len = strlen(arc->arc_members[i]->am_name);
        if (len > ARC_NAME_SHORT_MAX) {
            size += len + strlen(ARC_LONG_END);
        }
    }
    return size + size % 2;
}

// Fill a header field with text padded with spaces.
void Arc_WriteField(char *field, size_t width, const char *text)
{
    size_t len = strlen(text);
    memset(field, ' ', width);
    memcpy(field, text, len < width ? len : width);
}

// Write one member header.
void Arc_WriteHeader(FILE *out, const char *name, const char *owner, const char *mode, size_t size)
{
    Arc_Hdr hdr;
    char text[sizeof(hdr.ar_size) + 1];

    snprintf(text, sizeof(text), "%zu", size);
    Arc_WriteField(hdr.ar_name, sizeof(hdr.ar_name), name);
    Arc_WriteField(hdr.ar_date, sizeof(hdr.ar_date), owner);
    Arc_WriteField(hdr.ar_uid, sizeof(hdr.ar_uid), owner);
    Arc_WriteField(hdr.ar_gid, sizeof(hdr.ar_gid), owner);
    Arc_WriteField(hdr.ar_mode, sizeof(hdr.ar_mode), mode);
    Arc_WriteField(hdr.ar_size, sizeof(hdr.ar_size), text);
    memcpy(hdr.ar_fmag, ARC_FMAG, sizeof(hdr.ar_fmag));
    fwrite(&hdr, sizeof(hdr), 1, out);
}

// Write a big-endian word of the symbol index.
void Arc_WriteWord(FILE *out, uint32_t value)
{
    uint8_t bytes[ARC_INDEX_WORD];
    for (size_t i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (value >> (8 * (sizeof(bytes) - 1 - i))) & 0xFF;
    }
    fwrite(bytes, sizeof(bytes), 1, out);
}

// Write the symbol index for members laid out from offset first.
void Arc_WriteIndex(const Arc *arc, FILE *out, size_t first)
{
    size_t count = 0;
    size_t used = ARC_INDEX_WORD;
    size_t size = Arc_WriteIndexSize(arc);

    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        for (const char **iter = arc->arc_members[i]->am_globals; *iter; iter++) {
            count++;
        }
    }
    Arc_WriteHeader(out, ARC_NAME_INDEX, ARC_FIELD_ZERO, ARC_FIELD_ZERO, size);
    Arc_WriteWord(out, (uint32_t) count);

    // Phase: the offset of the member that defines each symbol.
    size_t pos = first;
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        const Arc_Member *member = arc->arc_members[i];
        for (const char **iter = member->am_globals; *iter; iter++) {
            Arc_WriteWord(out, (uint32_t) pos);
            used += ARC_INDEX_WORD;
        }
        pos += sizeof(Arc_Hdr) + member->am_size + member->am_size % 2;
    }

    // Phase: the symbols' names, in the same order.
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        for (const char **iter = arc->arc_members[i]->am_globals; *iter; iter++) {
            fwrite(*iter, strlen(*iter) + 1, 1, out);
            used += strlen(*iter) + 1;
        }
    }
    if (used < size) {
        fputc(ARC_PAD_INDEX, out);
    }
}

// Write the long-name table.
void Arc_WriteLongs(const Arc *arc, FILE *out, size_t size)
{
    size_t used = 0;

    Arc_WriteHeader(out, ARC_NAME_LONGS, "", "", size);
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        const char *name = arc->arc_members[i]->am_name;
        if (strlen(name) > ARC_NAME_SHORT_MAX) {
            fputs(name, out);
            fputs(ARC_LONG_END, out);
            used += strlen(name) + strlen(ARC_LONG_END);
        }
    }
    if (used < size) {
        fputc(ARC_PAD_MEMBER, out);
    }
}

// Write one member, its long name at *longoff in the long-name table.
void Arc_WriteMember(const Arc_Member *member, FILE *out, size_t *longoff)
{
    char name[ARC_NAME_SHORT_MAX + 2];
    size_t len = strlen(member->am_name);

    if (len > ARC_NAME_SHORT_MAX) {
        snprintf(name, sizeof(name), "%c%zu", ARC_NAME_END, *longoff);
        *longoff += len + strlen(ARC_LONG_END);
    } else {
        snprintf(name, sizeof(name), "%s%c", member->am_name, ARC_NAME_END);
    }
    Arc_WriteHeader(out, name, ARC_FIELD_ZERO, ARC_MODE_MEMBER, member->am_size);
    fwrite(member->am_data, 1, member->am_size, out);
    if (member->am_size % 2 != 0) {
        fputc(ARC_PAD_MEMBER, out);
    }
}

// Serialize an archive to a stream.
bool Arc_WriteFile(const Arc *arc, FILE *out)
{
    size_t longoff = 0;
    size_t index = Arc_WriteIndexSize(arc);
    size_t longs = Arc_WriteLongsSize(arc);
    size_t first = ARC_MAGIC_SIZE + sizeof(Arc_Hdr) + index + (longs != 0 ? sizeof(Arc_Hdr) + longs : 0);

    size_t end = first;
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        end += sizeof(Arc_Hdr) + arc->arc_members[i]->am_size + arc->arc_members[i]->am_size % 2;
    }
    if (end > ARC_INDEX_MAX) {
        return false;
    }

    fwrite(ARC_MAGIC, ARC_MAGIC_SIZE, 1, out);
    if (arc->arc_nmembers == 0) {
        return ! ferror(out);
    }
    Arc_WriteIndex(arc, out, first);
    if (longs != 0) {
        Arc_WriteLongs(arc, out, longs);
    }
    for (size_t i = 0; i < arc->arc_nmembers; i++) {
        Arc_WriteMember(arc->arc_members[i], out, &longoff);
    }
    return ! ferror(out);
}

// Serialize an archive to a file.
bool Arc_WritePath(const Arc *arc, const char *path)
{
    FILE *out = fopen(path, "wb");

    if (! out) {
        return false;
    }

    bool ok = Arc_WriteFile(arc, out);

    return fclose(out) == 0 && ok;
}
