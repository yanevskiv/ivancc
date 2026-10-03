/*
 * C source file for the ivanas assembler.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/str.h"
#include "arch/x86_64/enc.h"
#include "arch/x86_64/txt.h"

// Show usage information and exit.
static void As_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] INPUT.s\n"
        "  -o OUTPUT   write the object to OUTPUT (default: INPUT.o)\n",
        prog);
    exit(1);
}

// Read the whole source file at path as text.
static char *As_ReadSource(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (! file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 0) {
        fclose(file);
        return NULL;
    }

    char *text = malloc((size_t) size + 1);

    if (fread(text, 1, (size_t) size, file) != (size_t) size) {
        free(text);
        fclose(file);
        return NULL;
    }
    fclose(file);

    text[size] = '\0';
    return text;
}

// Read AT&T assembly from input and write a relocatable object to output.
static void As_Assemble(const char *input, const char *output)
{
    char *text = As_ReadSource(input);

    Err_Assert(text, ERR_AS_INPUT_NOT_READABLE, input, strerror(errno));
    Asm_x86_64_Reset();
    Txt_x86_64_Att_Read(text);
    free(text);

    Enc_x86_64_BuildObject();

    FILE *out = fopen(output, "wb");

    Err_Assert(out, ERR_AS_OUTPUT_NOT_WRITEABLE, output, strerror(errno));
    bool written = Enc_x86_64_Write(out);
    bool closed = fclose(out) == 0;
    Err_Assert(written && closed, ERR_AS_OUTPUT_WRITE_FAILED, output, strerror(errno));
}

// Main function
int main(int argc, char **argv)
{
    const char *output = NULL;
    const char *input = NULL;

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_Equals(arg, "-o") && i + 1 < argc) {
            output = argv[++i];
        } else if (arg[0] == '-' && arg[1]) {
            As_Usage(argv[0]);
        } else {
            input = arg;
        }
    }

    if (! input) {
        As_Usage(argv[0]);
    }

    char *outbuf = NULL;
    if (! output) {
        output = outbuf = Str_ModifyExtension(input, ".o");
    }

    As_Assemble(input, output);
    Str_Free(outbuf);
    return 0;
}
