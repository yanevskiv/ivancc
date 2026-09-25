// C source file for the ivanemu emulator.

// Standard headers.
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/elf.h"
#include "util/str.h"
#include "arch/x86_64/emu.h"
#include "arch/x86_64/load.h"

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
static void Emu_ShowImage(const Load_x86_64_Image *img)
{
    fprintf(stdout, "entry  0x%llx\n", (Emu_TypeULLong) img->li_entry);
    fprintf(stdout, "base   0x%llx\n", (Emu_TypeULLong) img->li_base);
    fprintf(stdout, "size   0x%llx\n", (Emu_TypeULLong) img->li_size);
    fprintf(stdout, "stack  0x%llx\n", (Emu_TypeULLong) img->li_stack);
}

// Disassemble forward from the image's base until the bytes stop decoding.
static void Emu_Disassemble(const Load_x86_64_Image *img)
{
    uint64_t rip = img->li_base;
    for (;;) {
        size_t avail = img->li_base + img->li_size - rip;
        const uint8_t *code = Load_x86_64_At(img, rip, 1);
        Emu_x86_64_Insn insn;
        char text[128];

        if (! code || ! Emu_x86_64_Decode(code, avail, &insn)) {
            fprintf(stdout, "%016llx: (bad)\n", (Emu_TypeULLong) rip);
            return;
        }
        Emu_x86_64_Format(&insn, rip, text, sizeof(text));
        fprintf(stdout, "%016llx: %s\n", (Emu_TypeULLong) rip, text);
        rip += insn.ei_len;
    }
}

// Main function
int main(int argc, char **argv)
{
    const char *arch = DEFAULT_ARCH;
    const char *program = NULL;
    bool disasm = false;
    bool info = false;
    Emu_x86_64_Trace trace = EMU_X86_64_QUIET;

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_StartsWith(arg, "-m" MARCH_PREFIX)) {
            arch = arg + 2 + strlen(MARCH_PREFIX);
        } else if (strcmp(arg, "-d") == 0) {
            disasm = true;
        } else if (strcmp(arg, "-i") == 0) {
            info = true;
        } else if (strcmp(arg, "-t") == 0) {
            trace = EMU_X86_64_TRACE;
        } else if (arg[0] == '-' && arg[1]) {
            Emu_Usage(argv[0]);
        } else {
            program = arg;
        }
    }

    if (! program) {
        Emu_Usage(argv[0]);
    }
    Err_Assert(Str_Equals(arch, DEFAULT_ARCH), ERR_EMU_ARCH_UNSUPPORTED, arch, DEFAULT_ARCH);

    Load_x86_64_Image img = {0};
    Err_Assert(Load_x86_64_ReadExec(program, &img), ERR_FILE_ACCESS, program, strerror(errno));
    Err_Assert(img.li_machine == ELF_EM_X86_64, ERR_EMU_NOT_X86_64, program);

    int32_t status = 0;
    if (info) {
        Emu_ShowImage(&img);
    } else if (disasm) {
        Emu_Disassemble(&img);
    } else {
        status = Emu_x86_64_Run(&img, trace);
    }

    Load_x86_64_Free(&img);
    return status;
}
