#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"
#include "obj/Elf/load.h"

// Target architecture selected when no -march= is given.
#define DEFAULT_ARCH "x86_64"

// Machine-option prefix recognised inside -m (e.g. -march=x86_64).
#define MARCH_PREFIX "arch="

// Show usage information and exit.
static void Emu_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] PROGRAM\n"
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

// Main function
int main(int argc, char **argv)
{
    const char *arch    = DEFAULT_ARCH;
    const char *program = NULL;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_StartsWith(arg, "-m" MARCH_PREFIX)) {
            arch = arg + 2 + strlen(MARCH_PREFIX);
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

    Emu_ShowImage(&img);
    Elf_Load_Free(&img);
    return 0;
}
