/*
 * C source file for the ivanemu emulator.
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
#define EMU_DEFAULT_ARCH "x86_64"

// Machine-option prefix recognised inside -m (e.g. -march=x86_64).
#define EMU_MARCH_PREFIX "arch="

// Show usage information and exit.
static void Emu_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] PROGRAM\n"
        "  -d          disassemble instead of running\n"
        "  -i          print what the loader made of the file and stop\n"
        "  -t          trace each instruction to stderr as it runs\n"
        "  -march=ARCH target architecture (default: " EMU_DEFAULT_ARCH ")\n",
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
    const char *arch = EMU_DEFAULT_ARCH;
    const char *program = NULL;
    bool disasm = false;
    bool info = false;
    Emu_x86_64_Trace trace = EMU_X86_64_QUIET;

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_StartsWith(arg, "-m" EMU_MARCH_PREFIX)) {
            arch = arg + 2 + strlen(EMU_MARCH_PREFIX);
        } else if (Str_Equals(arg, "-d")) {
            disasm = true;
        } else if (Str_Equals(arg, "-i")) {
            info = true;
        } else if (Str_Equals(arg, "-t")) {
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
    Err_Assert(Str_Equals(arch, EMU_DEFAULT_ARCH), ERR_EMU_ARCH_NOT_SUPPORTED, arch, EMU_DEFAULT_ARCH);

    Load_x86_64_Image img = {0};
    Err_Assert(Load_x86_64_ReadExec(program, &img), ERR_EMU_PROGRAM_NOT_READABLE, program, strerror(errno));
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
