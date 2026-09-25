// C source file for filesystem access.

#include <stdio.h>
#include <stdlib.h>

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
