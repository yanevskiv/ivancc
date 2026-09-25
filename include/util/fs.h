// C header file for filesystem access.

#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stddef.h>

// Whole-file read and write
char *Fs_FileGetContents(const char *path, size_t *len);
bool  Fs_FilePutContents(const char *path, const void *data, size_t len);

#endif // FS_H
