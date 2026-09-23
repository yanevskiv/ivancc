#include <stdio.h>
#include <stdlib.h>

#include "util/file.h"
#include "util/str.h"

// Read the whole file at path.
char *File_GetContents(const char *path, long *len)
{
    FILE *file = fopen(path, "rb");
    if (! file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (fread(buf, 1, size, file) != (size_t) size) {
        Str_Free(buf);
        fclose(file);
        return NULL;
    }
    fclose(file);

    buf[size] = '\0';
    if (len) {
        *len = size;
    }
    return buf;
}

// Write len bytes to path, replacing it.
int File_PutContents(const char *path, const void *data, long len)
{
    FILE *file = fopen(path, "wb");
    if (! file) {
        return -1;
    }
    int ok = fwrite(data, 1, len, file) == (size_t) len;
    fclose(file);
    return ok ? 0 : -1;
}
