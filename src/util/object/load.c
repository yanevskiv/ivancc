/*
 * C source file for loading executables.
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

// Module header.
#include "util/object/load.h"

// Bytes of stack reserved above the image.
#define LOAD_STACK_SIZE 0x100000

// Alignment the SysV ABI requires of %rsp at a call boundary.
#define LOAD_STACK_ALIGN 16

// Most bytes the arguments and environment take, a quarter as on Linux.
#define LOAD_ARGS_MAX (LOAD_STACK_SIZE / 4)

// Bytes in one word of the argument block.
#define LOAD_WORD_SIZE 8

// Bits in one byte of a word.
#define LOAD_BYTE_BITS 8

// Words of the argument block that are not pointers to strings.
#define LOAD_ARGS_FIXED_WORDS 5

// The type that ends the auxiliary vector.
#define LOAD_AT_NULL 0

// Round addr down to a multiple of align.
uint64_t Load_AlignDown(uint64_t addr, uint64_t align)
{
    return addr - addr % align;
}

// Round addr up to a multiple of align.
uint64_t Load_AlignUp(uint64_t addr, uint64_t align)
{
    return Load_AlignDown(addr + align - 1, align);
}

// Read an ET_EXEC file into a flat image, with a stack above it.
bool Load_ReadExec(const char *path, Load_Image *img)
{
    size_t len = 0;
    uint8_t *file = Elf_ReadBytes(path, &len);
    if (! file) {
        return false;
    }

    const uint8_t *data = file;
    const Elf64_Ehdr *eh = Elf_ReadEhdr(data, len);
    Err_Assert(eh, ERR_LOAD_NOT_ELF, path);
    Err_Assert(eh->e_ident[4] == ELF_CLASS64 && eh->e_ident[5] == ELF_DATA2LSB, ERR_LOAD_NOT_ELF64_LSB, path);
    Err_Assert(eh->e_type == ELF_ET_EXEC, ERR_LOAD_NOT_EXECUTABLE, path);

    // Phase: the extent of every PT_LOAD.
    uint64_t lo = UINT64_MAX;
    uint64_t hi = 0;
    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *) (data + eh->e_phoff + (uint64_t) i * eh->e_phentsize);
        if (ph->p_type != ELF_PT_LOAD) {
            continue;
        }
        if (ph->p_vaddr < lo) {
            lo = ph->p_vaddr;
        }
        if (ph->p_vaddr + ph->p_memsz > hi) {
            hi = ph->p_vaddr + ph->p_memsz;
        }
    }
    Err_Assert(lo <= hi, ERR_LOAD_NO_SEGMENTS, path);

    img->li_base    = Load_AlignDown(lo, ELF_PAGE);
    img->li_size    = Load_AlignUp(hi, ELF_PAGE) - img->li_base + LOAD_STACK_SIZE;
    img->li_entry   = eh->e_entry;
    img->li_machine = eh->e_machine;
    img->li_stack   = Load_AlignDown(img->li_base + img->li_size, LOAD_STACK_ALIGN);
    img->li_mem     = calloc(img->li_size, 1);

    // Phase: the bytes themselves.
    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *) (data + eh->e_phoff + (uint64_t) i * eh->e_phentsize);
        if (ph->p_type != ELF_PT_LOAD || ph->p_filesz == 0) {
            continue;
        }
        Err_Assert(ph->p_offset + ph->p_filesz <= len, ERR_LOAD_SEGMENT_TRUNCATED, path);
        memcpy(img->li_mem + (ph->p_vaddr - img->li_base), data + ph->p_offset, ph->p_filesz);
    }

    free(file);
    return true;
}

// Store a little-endian word.
void Load_PutWord(uint8_t *ptr, uint64_t value)
{
    for (size_t i = 0; i < LOAD_WORD_SIZE; i++) {
        ptr[i] = (uint8_t) (value >> (i * LOAD_BYTE_BITS));
    }
}

// Lay out argc, argv, envp and an empty auxiliary vector as Linux does.
void Load_PushArgs(Load_Image *img, const char *const *argv, size_t argc, const char *const *envp, size_t nenv)
{
    // Phase: the room the block takes.
    uint64_t strings = 0;
    for (size_t i = 0; i < argc; i++) {
        strings += strlen(argv[i]) + 1;
    }
    for (size_t i = 0; i < nenv; i++) {
        strings += strlen(envp[i]) + 1;
    }
    uint64_t words = argc + nenv + LOAD_ARGS_FIXED_WORDS;
    uint64_t total = strings + words * LOAD_WORD_SIZE + LOAD_STACK_ALIGN;
    Err_Assert(total <= LOAD_ARGS_MAX, ERR_LOAD_ARGS_TOO_LARGE, (unsigned long long) total, (unsigned long long) LOAD_ARGS_MAX);

    // Phase: the strings at the top, the words below them.
    uint64_t text = img->li_stack - strings;
    uint64_t sp = Load_AlignDown(text - words * LOAD_WORD_SIZE, LOAD_STACK_ALIGN);
    uint8_t *word = Load_At(img, sp, words * LOAD_WORD_SIZE);

    Load_PutWord(word, argc);
    word = Load_PutVector(img, word + LOAD_WORD_SIZE, &text, argv, argc);
    word = Load_PutVector(img, word, &text, envp, nenv);
    Load_PutWord(word, LOAD_AT_NULL);
    Load_PutWord(word + LOAD_WORD_SIZE, 0);
    img->li_stack = sp;
}

// Store a vector of strings and the null that ends it.
uint8_t *Load_PutVector(Load_Image *img, uint8_t *word, uint64_t *text, const char *const *strs, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(strs[i]) + 1;
        memcpy(Load_At(img, *text, len), strs[i], len);
        Load_PutWord(word, *text);
        word += LOAD_WORD_SIZE;
        *text += len;
    }
    Load_PutWord(word, 0);
    return word + LOAD_WORD_SIZE;
}

// Return a pointer to size bytes of the image at vaddr.
void *Load_At(const Load_Image *img, uint64_t vaddr, uint64_t size)
{
    if (vaddr < img->li_base || size > img->li_size) {
        return NULL;
    }
    uint64_t off = vaddr - img->li_base;
    if (off > img->li_size - size) {
        return NULL;
    }
    return img->li_mem + off;
}

// Release an image's memory and leave it empty.
void Load_Free(Load_Image *img)
{
    free(img->li_mem);
    memset(img, 0, sizeof(*img));
}
