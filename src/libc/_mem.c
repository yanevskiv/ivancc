/*
 * C source file for the heap and its allocator.
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
#include <_mem.h>

// The break, and the error number of an allocation that fails.
#include <_sys.h>

// The error number of the last failed call.
#include <_err.h>

// The loops that copy and fill memory.
#include <_str.h>

// The free blocks, in address order.
_Mem_Block *_Mem_FreeList;

// Compute the block size for size bytes, or 0 past PTRDIFF_MAX, as glibc's.
unsigned long _Mem_ComputeBlockSize(unsigned long size)
{
    unsigned long header = sizeof(_Mem_Block);
    unsigned long block;

    if (size > ((unsigned long) -1 >> 1) - header - _MEM_ALIGN) {
        return 0;
    }
    block = (size + header + _MEM_ALIGN - 1) / _MEM_ALIGN * _MEM_ALIGN;
    return block < _MEM_BLOCK_MIN ? _MEM_BLOCK_MIN : block;
}

// Find the link to the first free block of at least size bytes, or null.
_Mem_Block **_Mem_FindFreeBlock(unsigned long size)
{
    _Mem_Block **link = &_Mem_FreeList;

    while (*link && (*link)->mb_size < size) {
        link = &(*link)->mb_next;
    }
    return *link ? link : _MEM_NULL;
}

// Find the link to the free block that starts where block ends, or null.
_Mem_Block **_Mem_FindNextBlock(const _Mem_Block *block)
{
    const char *end = (const char *) block + block->mb_size;
    _Mem_Block **link = &_Mem_FreeList;

    while (*link && (const char *) *link < end) {
        link = &(*link)->mb_next;
    }
    return *link && (const char *) *link == end ? link : _MEM_NULL;
}

// Take size bytes of the free block at link, and leave the rest of it free.
void *_Mem_TakeBlock(_Mem_Block **link, unsigned long size)
{
    _Mem_Block *block = *link;

    if (block->mb_size - size < _MEM_BLOCK_MIN) {
        *link = block->mb_next;
    } else {
        _Mem_Block *rest = (_Mem_Block *) ((char *) block + size);
        rest->mb_size = block->mb_size - size;
        rest->mb_next = block->mb_next;
        block->mb_size = size;
        *link = rest;
    }
    return block + 1;
}

// Trim a block in use to size bytes, freeing what it holds past them.
void _Mem_TrimBlock(_Mem_Block *block, unsigned long size)
{
    _Mem_Block *rest = (_Mem_Block *) ((char *) block + size);

    if (block->mb_size - size >= _MEM_BLOCK_MIN) {
        rest->mb_size = block->mb_size - size;
        block->mb_size = size;
        _Mem_ReleaseBlock(rest);
    }
}

// Release a block, in address order and merged with its free neighbours.
void _Mem_ReleaseBlock(_Mem_Block *block)
{
    _Mem_Block **link = &_Mem_FreeList;
    _Mem_Block *prev = _MEM_NULL;
    _Mem_Block *next;

    while (*link && *link < block) {
        prev = *link;
        link = &(*link)->mb_next;
    }
    next = *link;
    if (next && (char *) block + block->mb_size == (char *) next) {
        block->mb_size += next->mb_size;
        next = next->mb_next;
    }
    block->mb_next = next;
    if (prev && (char *) prev + prev->mb_size == (char *) block) {
        prev->mb_size += block->mb_size;
        prev->mb_next = next;
    } else {
        *link = block;
    }
}

// Grow the heap by at least size bytes, moving the break, or return 0.
int _Mem_GrowHeap(unsigned long size)
{
    char *start = _Sys_InvokeBrk(_MEM_NULL);
    _Mem_Block *block;
    char *end;

    start += (_MEM_ALIGN - (unsigned long) start % _MEM_ALIGN) % _MEM_ALIGN;
    if (size < _MEM_GROW) {
        size = _MEM_GROW;
    }
    end = start + size;
    if (_Sys_InvokeBrk(end) != end) {
        return 0;
    }
    block = (_Mem_Block *) start;
    block->mb_size = size;
    _Mem_ReleaseBlock(block);
    return 1;
}

// Allocate nmemb objects of size bytes, all bits zero.
void *_Mem_AllocateObjects(unsigned long nmemb, unsigned long size)
{
    void *ptr;

    if (size != 0 && nmemb > (unsigned long) -1 / size) {
        _Err_Errno = _SYS_ENOMEM;
        return _MEM_NULL;
    }
    ptr = _Mem_AllocateMemory(nmemb * size);
    if (ptr) {
        _Str_FillBytes(ptr, 0, nmemb * size);
    }
    return ptr;
}

// Free the memory at ptr, which an allocation gave, or nothing for a null.
void _Mem_FreeMemory(void *ptr)
{
    if (ptr) {
        _Mem_ReleaseBlock((_Mem_Block *) ptr - 1);
    }
}

// Allocate size bytes, or return null with errno ENOMEM, as glibc does.
void *_Mem_AllocateMemory(unsigned long size)
{
    unsigned long need = _Mem_ComputeBlockSize(size);
    _Mem_Block **link = need ? _Mem_FindFreeBlock(need) : _MEM_NULL;

    if (! link && need && _Mem_GrowHeap(need)) {
        link = _Mem_FindFreeBlock(need);
    }
    if (! link) {
        _Err_Errno = _SYS_ENOMEM;
        return _MEM_NULL;
    }
    return _Mem_TakeBlock(link, need);
}

// Resize the memory at ptr to size bytes; a size of 0 frees it, as glibc's.
void *_Mem_ResizeMemory(void *ptr, unsigned long size)
{
    unsigned long need = _Mem_ComputeBlockSize(size);
    _Mem_Block *block;
    _Mem_Block **link;
    void *moved;

    if (! ptr) {
        return _Mem_AllocateMemory(size);
    }
    if (size == 0) {
        _Mem_FreeMemory(ptr);
        return _MEM_NULL;
    }
    if (need == 0) {
        _Err_Errno = _SYS_ENOMEM;
        return _MEM_NULL;
    }
    block = (_Mem_Block *) ptr - 1;
    link = need > block->mb_size ? _Mem_FindNextBlock(block) : _MEM_NULL;
    if (link && block->mb_size + (*link)->mb_size >= need) {
        block->mb_size += (*link)->mb_size;
        *link = (*link)->mb_next;
    }
    if (need <= block->mb_size) {
        _Mem_TrimBlock(block, need);
        return ptr;
    }
    moved = _Mem_AllocateMemory(size);
    if (moved) {
        _Str_CopyBytes(moved, ptr, block->mb_size - sizeof(_Mem_Block));
        _Mem_FreeMemory(ptr);
    }
    return moved;
}
