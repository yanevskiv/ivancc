// C source file for whole-file reads and writes.

#include <stdarg.h>
#include <stdlib.h>

#include "util/file.h"
#include "util/str.h"

// The streams the process is given rather than opens.
static File_Stream File_OutStream;
static File_Stream File_ErrStream;

// Read the whole file at path.
char *File_GetContents(const char *path, long *len)
{
    File_Stream *file = File_Open(path, "rb");
    if (! file) {
        return NULL;
    }

    long size = File_Size(file);
    char *buf = malloc(size + 1);

    if (File_GetBytes(file, buf, (size_t) size) != (size_t) size) {
        Str_Free(buf);
        File_Close(file);
        return NULL;
    }
    File_Close(file);

    buf[size] = '\0';
    if (len) {
        *len = size;
    }
    return buf;
}

// Write len bytes to path, replacing it.
int File_PutContents(const char *path, const void *data, long len)
{
    File_Stream *file = File_Open(path, "wb");
    if (! file) {
        return -1;
    }
    int ok = fwrite(data, 1, len, file->fs_file) == (size_t) len;
    File_Close(file);
    return ok ? 0 : -1;
}

// Open path, or return NULL where it cannot be opened.
File_Stream *File_Open(const char *path, const char *mode)
{
    FILE *raw = fopen(path, mode);
    if (! raw) {
        return NULL;
    }

    File_Stream *file = malloc(sizeof(*file));

    file->fs_file = raw;
    file->fs_owned = 1;
    return file;
}

// Return the stream this process writes its output to.
File_Stream *File_Out(void)
{
    File_OutStream.fs_file = stdout;
    return &File_OutStream;
}

// Return the stream this process writes its diagnostics to.
File_Stream *File_Err(void)
{
    File_ErrStream.fs_file = stderr;
    return &File_ErrStream;
}

// Return the number of bytes in an open stream.
long File_Size(File_Stream *file)
{
    fseek(file->fs_file, 0, SEEK_END);
    long size = ftell(file->fs_file);
    fseek(file->fs_file, 0, SEEK_SET);
    return size;
}

// Push whatever a stream is still holding.
void File_Flush(File_Stream *file)
{
    fflush(file->fs_file);
}

// Close a stream, flushing one this process was given instead.
void File_Close(File_Stream *file)
{
    if (! file->fs_owned) {
        File_Flush(file);
        return;
    }
    fclose(file->fs_file);
    free(file);
}

// Read up to len bytes, returning how many arrived.
size_t File_GetBytes(File_Stream *file, void *data, size_t len)
{
    return fread(data, 1, len, file->fs_file);
}

// Write one byte.
void File_PutByte(File_Stream *file, int byte)
{
    fputc(byte, file->fs_file);
}

// Write a NUL-terminated string.
void File_PutText(File_Stream *file, const char *text)
{
    fputs(text, file->fs_file);
}

// Write len bytes.
void File_PutBytes(File_Stream *file, const void *data, size_t len)
{
    fwrite(data, 1, len, file->fs_file);
}

// Write a formatted string.
void File_Print(File_Stream *file, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    File_PrintVa(file, fmt, ap);
    va_end(ap);
}

// Write a formatted string from an argument list.
void File_PrintVa(File_Stream *file, const char *fmt, va_list ap)
{
    vfprintf(file->fs_file, fmt, ap);
}

// Report why the last call naming path failed.
void File_ShowError(const char *path)
{
    perror(path);
}
