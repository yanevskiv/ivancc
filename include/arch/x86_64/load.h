// C header file for loading x86-64 executables.

#ifndef LOAD_X86_64_H
#define LOAD_X86_64_H

#include <stdbool.h>
#include <stdint.h>

// A loaded program: one flat buffer holding every PT_LOAD and a stack.
typedef struct Load_x86_64_Image Load_x86_64_Image;
struct Load_x86_64_Image {
    uint8_t  *li_mem;      // li_size bytes, zeroed and then filled
    uint64_t  li_base;     // virtual address li_mem[0] stands for
    uint64_t  li_size;     // bytes li_mem holds
    uint64_t  li_entry;    // e_entry
    uint64_t  li_stack;    // initial %rsp, 16-byte aligned
    uint16_t  li_machine;  // e_machine, for the caller to accept or reject
};

// Loading
uint64_t Load_x86_64_AlignDown(uint64_t addr, uint64_t align);
uint64_t Load_x86_64_AlignUp(uint64_t addr, uint64_t align);
bool     Load_x86_64_ReadExec(const char *path, Load_x86_64_Image *img);
void    *Load_x86_64_At(const Load_x86_64_Image *img, uint64_t vaddr, uint64_t size);
void     Load_x86_64_Free(Load_x86_64_Image *img);

#endif // LOAD_X86_64_H
