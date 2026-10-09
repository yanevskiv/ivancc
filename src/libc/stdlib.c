/*
 * C source file for general utilities.
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
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

// Module header.
#include <stdlib.h>

// The error the memory functions report.
#include <errno.h>

// Clearing and copying memory.
#include <string.h>

// The break the heap grows by.
#include <_sys.h>

// Check that a block's header keeps the memory after it aligned.
typedef char _Stdlib_CheckBlock[sizeof(struct _Stdlib_Block) == _STDLIB_ALIGN && _STDLIB_BLOCK_MIN == 2 * _STDLIB_ALIGN ? 1 : -1];

// The free blocks, in address order.
struct _Stdlib_Block *_Stdlib_FreeList;

// Return the block size for size bytes, or 0 past PTRDIFF_MAX, as glibc's.
size_t _Stdlib_BlockSize(size_t size)
{
    size_t header = sizeof(struct _Stdlib_Block);
    size_t block;

    if (size > ((size_t) -1 >> 1) - header - _STDLIB_ALIGN) {
        return 0;
    }
    block = (size + header + _STDLIB_ALIGN - 1) / _STDLIB_ALIGN * _STDLIB_ALIGN;
    return block < _STDLIB_BLOCK_MIN ? _STDLIB_BLOCK_MIN : block;
}

// Return the link to the first free block of at least size bytes, or NULL.
struct _Stdlib_Block **_Stdlib_Fit(size_t size)
{
    struct _Stdlib_Block **link = &_Stdlib_FreeList;

    while (*link && (*link)->sb_size < size) {
        link = &(*link)->sb_next;
    }
    return *link ? link : NULL;
}

// Return the link to the free block that starts where block ends, or NULL.
struct _Stdlib_Block **_Stdlib_Link(const struct _Stdlib_Block *block)
{
    const char *end = (const char *) block + block->sb_size;
    struct _Stdlib_Block **link = &_Stdlib_FreeList;

    while (*link && (const char *) *link < end) {
        link = &(*link)->sb_next;
    }
    return *link && (const char *) *link == end ? link : NULL;
}

// Take size bytes of the free block at link, and leave the rest of it free.
void *_Stdlib_Take(struct _Stdlib_Block **link, size_t size)
{
    struct _Stdlib_Block *block = *link;

    if (block->sb_size - size < _STDLIB_BLOCK_MIN) {
        *link = block->sb_next;
    } else {
        struct _Stdlib_Block *rest = (struct _Stdlib_Block *) ((char *) block + size);
        rest->sb_size = block->sb_size - size;
        rest->sb_next = block->sb_next;
        block->sb_size = size;
        *link = rest;
    }
    return block + 1;
}

// Free what a block in use holds past size bytes.
void _Stdlib_Trim(struct _Stdlib_Block *block, size_t size)
{
    struct _Stdlib_Block *rest = (struct _Stdlib_Block *) ((char *) block + size);

    if (block->sb_size - size >= _STDLIB_BLOCK_MIN) {
        rest->sb_size = block->sb_size - size;
        block->sb_size = size;
        _Stdlib_Release(rest);
    }
}

// Free a block, in address order and merged with its free neighbours.
void _Stdlib_Release(struct _Stdlib_Block *block)
{
    struct _Stdlib_Block **link = &_Stdlib_FreeList;
    struct _Stdlib_Block *prev = NULL;
    struct _Stdlib_Block *next;

    while (*link && *link < block) {
        prev = *link;
        link = &(*link)->sb_next;
    }
    next = *link;
    if (next && (char *) block + block->sb_size == (char *) next) {
        block->sb_size += next->sb_size;
        next = next->sb_next;
    }
    block->sb_next = next;
    if (prev && (char *) prev + prev->sb_size == (char *) block) {
        prev->sb_size += block->sb_size;
        prev->sb_next = next;
    } else {
        *link = block;
    }
}

// Move the break past at least size more bytes and free them, or return 0.
int _Stdlib_Grow(size_t size)
{
    char *start = _Sys_Brk(NULL);
    struct _Stdlib_Block *block;
    char *end;

    start += (_STDLIB_ALIGN - (unsigned long) start % _STDLIB_ALIGN) % _STDLIB_ALIGN;
    if (size < _STDLIB_GROW) {
        size = _STDLIB_GROW;
    }
    end = start + size;
    if (_Sys_Brk(end) != end) {
        return 0;
    }
    block = (struct _Stdlib_Block *) start;
    block->sb_size = size;
    _Stdlib_Release(block);
    return 1;
}

// Allocate nmemb objects of size bytes, all bits zero.
void *calloc(size_t nmemb, size_t size)
{
    void *ptr;

    if (size != 0 && nmemb > (size_t) -1 / size) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    ptr = malloc(nmemb * size);
    if (ptr) {
        memset(ptr, 0, nmemb * size);
    }
    return ptr;
}

// Free the memory at ptr, which malloc, calloc or realloc gave.
void free(void *ptr)
{
    if (ptr) {
        _Stdlib_Release((struct _Stdlib_Block *) ptr - 1);
    }
}

// Allocate size bytes, or return NULL with errno ENOMEM, as glibc does.
void *malloc(size_t size)
{
    size_t need = _Stdlib_BlockSize(size);
    struct _Stdlib_Block **link = need ? _Stdlib_Fit(need) : NULL;

    if (! link && need && _Stdlib_Grow(need)) {
        link = _Stdlib_Fit(need);
    }
    if (! link) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    return _Stdlib_Take(link, need);
}

// Resize the memory at ptr to size bytes; a size of 0 frees it, as glibc's.
void *realloc(void *ptr, size_t size)
{
    size_t need = _Stdlib_BlockSize(size);
    struct _Stdlib_Block *block;
    struct _Stdlib_Block **link;
    void *moved;

    if (! ptr) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    if (need == 0) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    block = (struct _Stdlib_Block *) ptr - 1;
    link = need > block->sb_size ? _Stdlib_Link(block) : NULL;
    if (link && block->sb_size + (*link)->sb_size >= need) {
        block->sb_size += (*link)->sb_size;
        *link = (*link)->sb_next;
    }
    if (need <= block->sb_size) {
        _Stdlib_Trim(block, need);
        return ptr;
    }
    moved = malloc(size);
    if (moved) {
        memcpy(moved, ptr, block->sb_size - sizeof(struct _Stdlib_Block));
        free(ptr);
    }
    return moved;
}
