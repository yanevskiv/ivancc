/*
 * C header file for the heap and its allocator.
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


#ifndef _MEM_H
#define _MEM_H

// A pointer to nothing, of any type.
#define _MEM_NULL ((void *) 0)

// The alignment of the memory an allocation gives, any object's, as glibc's,
// and the smallest block, a header and that much memory.
#define _MEM_ALIGN     16
#define _MEM_BLOCK_MIN 32

// The least the heap grows by, so that few allocations move the break.
#define _MEM_GROW 0x10000

// A block of the heap, the header before the memory an allocation gives.
typedef struct _Mem_Block _Mem_Block;
struct _Mem_Block {
    unsigned long mb_size; // the block's bytes, this header counted
    _Mem_Block *mb_next;   // the next free block, while this one is free
};

// The free blocks, in address order.
extern _Mem_Block *_Mem_FreeList;

// Heap
unsigned long _Mem_ComputeBlockSize(unsigned long size);
_Mem_Block  **_Mem_FindFreeBlock(unsigned long size);
_Mem_Block  **_Mem_FindNextBlock(const _Mem_Block *block);
void         *_Mem_TakeBlock(_Mem_Block **link, unsigned long size);
void          _Mem_TrimBlock(_Mem_Block *block, unsigned long size);
void          _Mem_ReleaseBlock(_Mem_Block *block);
int           _Mem_GrowHeap(unsigned long size);

// Allocation
void *_Mem_AllocateObjects(unsigned long nmemb, unsigned long size);
void  _Mem_FreeMemory(void *ptr);
void *_Mem_AllocateMemory(unsigned long size);
void *_Mem_ResizeMemory(void *ptr, unsigned long size);

#endif // _MEM_H
