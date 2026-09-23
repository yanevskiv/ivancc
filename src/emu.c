#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"
#include "object/elf.h"
#include "arch/x86_64/emu.h"

// Target architecture selected when no -march= is given.
#define DEFAULT_ARCH "x86_64"

// Machine-option prefix recognised inside -m (e.g. -march=x86_64).
#define MARCH_PREFIX "arch="

// Show usage information and exit.
static void Emu_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] PROGRAM\n"
        "  -d          disassemble instead of running\n"
        "  -i          print what the loader made of the file and stop\n"
        "  -t          trace each instruction to stderr as it runs\n"
        "  -march=ARCH target architecture (default: " DEFAULT_ARCH ")\n",
        prog);
    exit(1);
}

// Print what the loader made of an executable.
static void Emu_ShowImage(const Elf_LoadImage *img)
{
    printf("entry  0x%llx\n", (unsigned long long) img->li_entry);
    printf("base   0x%llx\n", (unsigned long long) img->li_base);
    printf("size   0x%llx\n", (unsigned long long) img->li_size);
    printf("stack  0x%llx\n", (unsigned long long) img->li_stack);
}

// Disassemble forward from the image's base until the bytes stop decoding.
static void Emu_Disassemble(const Elf_LoadImage *img)
{
    uint64_t rip = img->li_base;
    for (;;) {
        int avail = (int) (img->li_base + img->li_size - rip);
        const uint8_t *code = Elf_Load_At(img, rip, 1);
        Emu_x86_64_Insn insn;
        char text[128];

        if (! code || ! Emu_x86_64_Decode(code, avail, &insn)) {
            printf("%016llx: (bad)\n", (unsigned long long) rip);
            return;
        }
        Emu_x86_64_Format(&insn, rip, text, sizeof(text));
        printf("%016llx: %s\n", (unsigned long long) rip, text);
        rip += insn.ei_len;
    }
}

// Main function
int main(int argc, char **argv)
{
    const char *arch    = DEFAULT_ARCH;
    const char *program = NULL;
    int disasm = 0;
    int info   = 0;
    int trace  = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_StartsWith(arg, "-m" MARCH_PREFIX)) {
            arch = arg + 2 + strlen(MARCH_PREFIX);
        } else if (strcmp(arg, "-d") == 0) {
            disasm = 1;
        } else if (strcmp(arg, "-i") == 0) {
            info = 1;
        } else if (strcmp(arg, "-t") == 0) {
            trace = 1;
        } else if (arg[0] == '-' && arg[1]) {
            Emu_Usage(argv[0]);
        } else {
            program = arg;
        }
    }

    if (! program) {
        Emu_Usage(argv[0]);
    }
    if (! Str_Equals(arch, DEFAULT_ARCH)) {
        Log_ShowError("unsupported architecture '%s' (only " DEFAULT_ARCH " is supported)", arch);
    }

    Elf_LoadImage img = {0};
    if (Elf_Load_ReadExec(program, &img) != 0) {
        perror(program);
        return 1;
    }
    if (img.li_machine != ELF_EM_X86_64) {
        Log_ShowError("'%s' is not an x86_64 executable", program);
    }

    int status = 0;
    if (info) {
        Emu_ShowImage(&img);
    } else if (disasm) {
        Emu_Disassemble(&img);
    } else {
        status = Emu_x86_64_Run(&img, trace);
    }

    Elf_Load_Free(&img);
    return status;
}
