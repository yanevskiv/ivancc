// C header file for whole-file reads and writes.

#ifndef FILE_H
#define FILE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

// An open byte stream.
typedef struct File_Stream File_Stream;
struct File_Stream {
    FILE *fs_file;
    int   fs_owned; // true where File_Close may close it
};

// Whole-file read and write
char *File_GetContents(const char *path, long *len);
int   File_PutContents(const char *path, const void *data, long len);

// Streams
File_Stream *File_Open(const char *path, const char *mode);
File_Stream *File_Out(void);
File_Stream *File_Err(void);
long         File_Size(File_Stream *file);
void         File_Flush(File_Stream *file);
void         File_Close(File_Stream *file);

// Stream reads and writes
size_t File_GetBytes(File_Stream *file, void *data, size_t len);
void   File_PutByte(File_Stream *file, int byte);
void   File_PutText(File_Stream *file, const char *text);
void   File_PutBytes(File_Stream *file, const void *data, size_t len);
void   File_Print(File_Stream *file, const char *fmt, ...);
void   File_PrintVa(File_Stream *file, const char *fmt, va_list ap);

// Diagnostics
void File_ShowError(const char *path);

#endif // FILE_H
