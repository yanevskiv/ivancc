/*
 * C header file for general utilities.
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

#ifndef __STDLIB_H__
#define __STDLIB_H__

// The alignment of the memory malloc gives, any object's, as glibc's,
// and the smallest block, a header and that much memory.
#define _STDLIB_ALIGN     16
#define _STDLIB_BLOCK_MIN 32

// The least the heap grows by, so that few allocations move the break.
#define _STDLIB_GROW 0x10000

// (S7.20) General utilities
#define NULL ((void *) 0)

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

// A block of the heap: its size with this header, and while free the next free block.
struct _Stdlib_Block {
    size_t sb_size;
    struct _Stdlib_Block *sb_next;
};

// The free blocks, in address order.
extern struct _Stdlib_Block *_Stdlib_FreeList;

// Heap
size_t _Stdlib_BlockSize(size_t size);
struct _Stdlib_Block **_Stdlib_Fit(size_t size);
struct _Stdlib_Block **_Stdlib_Link(const struct _Stdlib_Block *block);
void *_Stdlib_Take(struct _Stdlib_Block **link, size_t size);
void _Stdlib_Trim(struct _Stdlib_Block *block, size_t size);
void _Stdlib_Release(struct _Stdlib_Block *block);
int _Stdlib_Grow(size_t size);

// (S7.20.3) Memory management functions
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

#endif // __STDLIB_H__
