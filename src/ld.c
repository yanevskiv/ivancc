/*
 * C source file for the ivanld linker.
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
#include "ld.h"

// Show usage information and exit.
void Ld_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] INPUT...\n"
        "  INPUT              an object, or an archive whose members are linked\n"
        "                     where they define a symbol the inputs before it need\n"
        "  -o OUTPUT          write the output to OUTPUT (default: " LD_DEFAULT_OUTPUT ")\n"
        "  -e ENTRY           set the entry symbol (default: _start)\n"
        "  -r                 merge the inputs into one relocatable object\n"
        "  -t, --trace        print each input as it is read; twice, each archive\n"
        "                     member linked too, as (ARCHIVE)MEMBER\n"
        "  -place=SEC@ADDR    place output section SEC at ADDR (text/data name a\n"
        "                     default section; any other name is used verbatim)\n",
        prog);
    exit(1);
}

// Map a -place name to its ELF section: text -> .text, data/rodata -> .rodata.
char *Ld_PlaceName(const char *spec, size_t len)
{
    char *name = Str_Slice(spec, 0, len);
    if (Str_Equals(name, "text")) {
        Str_Free(name);
        return Str_Clone(".text");
    }
    if (Str_Equals(name, "data") || Str_Equals(name, "rodata")) {
        Str_Free(name);
        return Str_Clone(".rodata");
    }
    return name;
}

// Parse a -place=SEC@ADDR argument into opts.
void Ld_ParsePlace(const char *spec, Link_Options *opts)
{
    const char *at = strchr(spec, '@');
    Err_Assert(at, ERR_LD_PLACE_MALFORMED, spec);
    Link_PlaceAdd(opts, Ld_PlaceName(spec, (size_t) (at - spec)), strtoull(at + 1, NULL, 0));
}

// Main function
int main(int argc, char **argv)
{
    const char  *output = LD_DEFAULT_OUTPUT;
    Link_Options opts = {0};

    size_t nobjs = 0;
    const char **objs = calloc(argc, sizeof(*objs));

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (Str_Equals(arg, "-o") && i + 1 < argc) {
            output = argv[++i];
        } else if (Str_Equals(arg, "-e") && i + 1 < argc) {
            opts.lo_entry = argv[++i];
        } else if (Str_Equals(arg, "-r")) {
            opts.lo_relocatable = true;
        } else if (Str_Equals(arg, "-t") || Str_Equals(arg, "--trace")) {
            opts.lo_trace = opts.lo_trace == LINK_TRACE_NONE ? LINK_TRACE_FILES : LINK_TRACE_MEMBERS;
        } else if (strncmp(arg, "-place=", 7) == 0) {
            Ld_ParsePlace(arg + 7, &opts);
        } else if (arg[0] == '-') {
            Ld_Usage(argv[0]);
        } else {
            objs[nobjs++] = arg;
        }
    }

    if (nobjs == 0) {
        Ld_Usage(argv[0]);
    }

    Elf *e = Link_Build((const char *const *) objs, nobjs, &opts);
    Err_Assert(Elf_WritePath(e, output), ERR_LD_OUTPUT_NOT_WRITEABLE, output, strerror(errno));
    Elf_Free(e);
    if (! opts.lo_relocatable) {
        chmod(output, LD_MODE);
    }

    free(opts.lo_places);
    free(objs);
    return 0;
}
