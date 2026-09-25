// C header file for filesystem access.

#ifndef FS_H
#define FS_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Whole-file read and write
char *Fs_FileGetContents(const char *path, size_t *len);
bool  Fs_FilePutContents(const char *path, const void *data, size_t len);

// Queries
bool Fs_FileExists(const char *path);

#endif // FS_H
