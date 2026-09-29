/*
 * C source file for filesystem access.
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
#include "util/fs.h"

// Read the whole file at path.
char *Fs_FileGetContents(const char *path, size_t *len)
{
    FILE *file = fopen(path, "rb");
    if (! file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 0) {
        fclose(file);
        return NULL;
    }

    char *buf = malloc((size_t) size + 1);

    if (fread(buf, 1, (size_t) size, file) != (size_t) size) {
        free(buf);
        fclose(file);
        return NULL;
    }
    fclose(file);

    buf[size] = '\0';
    if (len) {
        *len = (size_t) size;
    }
    return buf;
}

// Write len bytes to path, replacing it.
bool Fs_FilePutContents(const char *path, const void *data, size_t len)
{
    FILE *file = fopen(path, "wb");
    if (! file) {
        return false;
    }

    bool ok = fwrite(data, 1, len, file) == len;

    return fclose(file) == 0 && ok;
}

// True if the file at path can be read.
bool Fs_FileExists(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (! file) {
        return false;
    }
    fclose(file);
    return true;
}
