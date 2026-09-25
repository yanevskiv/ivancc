// C source file for loading x86-64 executables.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util/err.h"
#include "object/elf.h"
#include "arch/x86_64/load.h"

// Bytes of stack reserved above the image.
#define LOAD_X86_64_STACK_SIZE 0x100000

// Alignment the SysV ABI requires of %rsp at a call boundary.
#define LOAD_X86_64_STACK_ALIGN 16

// Round addr down to a multiple of align.
uint64_t Load_x86_64_AlignDown(uint64_t addr, uint64_t align)
{
    return addr - addr % align;
}

// Round addr up to a multiple of align.
uint64_t Load_x86_64_AlignUp(uint64_t addr, uint64_t align)
{
    return Load_x86_64_AlignDown(addr + align - 1, align);
}

// Read an ET_EXEC file into a flat image, with a stack above it.
bool Load_x86_64_ReadExec(const char *path, Load_x86_64_Image *img)
{
    size_t len = 0;
    uint8_t *file = Elf_Read_Bytes(path, &len);
    if (! file) {
        return false;
    }

    const uint8_t *data = file;
    const Elf64_Ehdr *eh = Elf_Read_Ehdr(data, len);
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

    img->li_base    = Load_x86_64_AlignDown(lo, ELF_PAGE);
    img->li_size    = Load_x86_64_AlignUp(hi, ELF_PAGE) - img->li_base + LOAD_X86_64_STACK_SIZE;
    img->li_entry   = eh->e_entry;
    img->li_machine = eh->e_machine;
    img->li_stack   = Load_x86_64_AlignDown(img->li_base + img->li_size, LOAD_X86_64_STACK_ALIGN);
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

// Return a pointer to size bytes of the image at vaddr.
void *Load_x86_64_At(const Load_x86_64_Image *img, uint64_t vaddr, uint64_t size)
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
void Load_x86_64_Free(Load_x86_64_Image *img)
{
    free(img->li_mem);
    memset(img, 0, sizeof(*img));
}
