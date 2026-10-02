/*
 * C header file for loading executables.
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
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOAD_H
#define LOAD_H

// Standard headers.
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/object/elf.h"

// A loaded program: one flat buffer holding every PT_LOAD and a stack.
typedef struct Load_Image Load_Image;
struct Load_Image {
    uint8_t  *li_mem;      // li_size bytes, zeroed and then filled
    uint64_t  li_base;     // virtual address li_mem[0] stands for
    uint64_t  li_size;     // bytes li_mem holds
    uint64_t  li_entry;    // e_entry
    uint64_t  li_stack;    // initial %rsp, 16-byte aligned
    uint16_t  li_machine;  // e_machine, for the caller to accept or reject
};

// Loading
uint64_t Load_AlignDown(uint64_t addr, uint64_t align);
uint64_t Load_AlignUp(uint64_t addr, uint64_t align);
bool     Load_ReadExec(const char *path, Load_Image *img);
void    *Load_At(const Load_Image *img, uint64_t vaddr, uint64_t size);
void     Load_Free(Load_Image *img);

#endif // LOAD_H
