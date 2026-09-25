// C source file for the ivanld linker.

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "util/err.h"
#include "util/log.h"
#include "util/str.h"
#include "object/elf.h"

// Permission bits for the executable ld writes (rwxr-xr-x).
#define LD_MODE 0755

// Default output name when no -o is given.
#define LD_DEFAULT_OUTPUT "a.out"

// Show usage information and exit.
static void Ld_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] INPUT.o...\n"
        "  -o OUTPUT          write the output to OUTPUT (default: " LD_DEFAULT_OUTPUT ")\n"
        "  -e ENTRY           set the entry symbol (default: _start)\n"
        "  -r                 merge the inputs into one relocatable object\n"
        "  -place=SEC@ADDR    place output section SEC at ADDR (text/data name a\n"
        "                     default section; any other name is used verbatim)\n",
        prog);
    exit(1);
}

// Map a -place name to its ELF section: text -> .text, data/rodata -> .rodata.
static char *Ld_PlaceName(const char *spec, size_t len)
{
    char *name = Str_Slice(spec, 0, len);
    if (strcmp(name, "text") == 0) {
        Str_Free(name);
        return Str_Clone(".text");
    }
    if (strcmp(name, "data") == 0 || strcmp(name, "rodata") == 0) {
        Str_Free(name);
        return Str_Clone(".rodata");
    }
    return name;
}

// Parse a -place=SEC@ADDR argument into opts.
static void Ld_ParsePlace(const char *spec, Elf_LinkOptions *opts)
{
    const char *at = strchr(spec, '@');
    Err_Assert(at, ERR_LD_PLACE_MALFORMED, spec);
    Elf_Link_AddPlace(opts, Ld_PlaceName(spec, (size_t) (at - spec)), strtoull(at + 1, NULL, 0));
}

// Main function
int main(int argc, char **argv)
{
    const char  *output = LD_DEFAULT_OUTPUT;
    Elf_LinkOptions opts = {0};

    size_t nobjs = 0;
    const char **objs = calloc(argc, sizeof(*objs));

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        } else if (strcmp(arg, "-e") == 0 && i + 1 < argc) {
            opts.lo_entry = argv[++i];
        } else if (strcmp(arg, "-r") == 0) {
            opts.lo_relocatable = true;
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

    Elf *e = Elf_Link_Run((const char *const *) objs, nobjs, &opts);
    Err_Assert(Elf_Write_Path(e, output), ERR_FILE_ACCESS, output, strerror(errno));
    Elf_Free(e);
    if (! opts.lo_relocatable) {
        chmod(output, LD_MODE);
    }

    free(opts.lo_places);
    free(objs);
    return 0;
}
