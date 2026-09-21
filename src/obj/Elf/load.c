#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util/file.h"
#include "util/log.h"
#include "util/str.h"
#include "obj/Elf/read.h"
#include "obj/Elf/load.h"

// Bytes of stack reserved above the image; nothing here grows one on demand.
#define LOAD_STACK_SIZE 0x100000

// Alignment the SysV ABI requires of %rsp at a call boundary.
#define LOAD_STACK_ALIGN 16

// Round addr down to a multiple of align.
uint64_t Elf_Load_AlignDown(uint64_t addr, uint64_t align)
{
    return addr - addr % align;
}

// Round addr up to a multiple of align.
uint64_t Elf_Load_AlignUp(uint64_t addr, uint64_t align)
{
    return Elf_Load_AlignDown(addr + align - 1, align);
}

// Read an ET_EXEC file into a flat image: every PT_LOAD at its virtual address,
// a stack above them, and zeros everywhere the file has no bytes.
int Elf_Load_ReadExec(const char *path, Elf_LoadImage *img)
{
    long len = 0;
    char *file = File_GetContent(path, &len);
    if (! file) {
        return -1;
    }

    const uint8_t *data = (const uint8_t *) file;
    const Elf64_Ehdr *eh = Elf_Read_Ehdr(data, (size_t) len);
    if (! eh) {
        Str_Free(file);
        Log_ShowError("not an ELF file: '%s'", path);
    }
    if (eh->e_ident[4] != ELF_CLASS64 || eh->e_ident[5] != ELF_DATA2LSB) {
        Str_Free(file);
        Log_ShowError("not a 64-bit little-endian ELF file: '%s'", path);
    }
    if (eh->e_type != ELF_ET_EXEC) {
        Str_Free(file);
        Log_ShowError("not an executable: '%s'", path);
    }

    // Phase: the extent of every PT_LOAD, which the image has to cover.
    uint64_t lo = UINT64_MAX;
    uint64_t hi = 0;
    for (int i = 0; i < eh->e_phnum; i++) {
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
    if (lo > hi) {
        Str_Free(file);
        Log_ShowError("no loadable segments in '%s'", path);
    }

    // The stack shares the allocation so that one range covers every access.
    img->li_base    = Elf_Load_AlignDown(lo, ELF_PAGE);
    img->li_size    = Elf_Load_AlignUp(hi, ELF_PAGE) - img->li_base + LOAD_STACK_SIZE;
    img->li_entry   = eh->e_entry;
    img->li_machine = eh->e_machine;
    img->li_stack   = Elf_Load_AlignDown(img->li_base + img->li_size, LOAD_STACK_ALIGN);
    img->li_mem     = calloc(img->li_size, 1);

    // Phase: the bytes themselves, leaving p_memsz beyond p_filesz zeroed.
    for (int i = 0; i < eh->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *) (data + eh->e_phoff + (uint64_t) i * eh->e_phentsize);
        // A segment with no file bytes, such as .bss, has nothing to copy and
        // no file offset worth checking.
        if (ph->p_type != ELF_PT_LOAD || ph->p_filesz == 0) {
            continue;
        }
        if (ph->p_offset + ph->p_filesz > (uint64_t) len) {
            Str_Free(file);
            Log_ShowError("segment runs past the end of '%s'", path);
        }
        memcpy(img->li_mem + (ph->p_vaddr - img->li_base), data + ph->p_offset, ph->p_filesz);
    }

    Str_Free(file);
    return 0;
}

// Return a pointer to size bytes of the image at vaddr, or NULL if that range
// is not mapped; the caller reports the fault, since only it knows the %rip.
void *Elf_Load_At(const Elf_LoadImage *img, uint64_t vaddr, uint64_t size)
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
void Elf_Load_Free(Elf_LoadImage *img)
{
    free(img->li_mem);
    memset(img, 0, sizeof(*img));
}
