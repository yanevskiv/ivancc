#ifndef ELF_READ_H
#define ELF_READ_H

#include <stddef.h>
#include <stdint.h>

#include "obj/Elf/types.h"

// Reading ELF files
const Elf64_Ehdr *Elf_Read_Ehdr(const uint8_t *data, size_t n);
Elf *Elf_Read_Mem(const void *buf, size_t n);
Elf *Elf_Read_Path(const char *path);

#endif // ELF_READ_H
