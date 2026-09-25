// C source file for the ivancc compiler driver.

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util/err.h"
#include "util/log.h"
#include "util/str.h"
#include "syntax/ast.h"
#include "syntax/par.h"
#include "syntax/pp.h"
#include "syntax/sem.h"
#include "object/elf.h"
#include "arch/x86_64/gen.h"
#include "arch/x86_64/enc.h"
#include "arch/x86_64/link.h"
#include "arch/x86_64/txt.h"

// Permission bits for the executables cc writes (rwxr-xr-x).
#define ELF_MODE 0755

// Default output name for a freestanding executable.
#define DEFAULT_OUTPUT "a.out"

// Output name that means standard output rather than a file.
#define STDOUT_NAME "-"

// The one language standard --std accepts.
#define DEFAULT_STD "c99"

// Target architecture selected when no -march= is given.
#define DEFAULT_ARCH "x86_64"

// Runtime target selected when no -mtarget= is given.
#define DEFAULT_TARGET "linux"

// Machine-option prefixes recognised inside -m (e.g. -march=x86_64).
#define MARCH_PREFIX "arch="
#define MTARGET_PREFIX "target="

// Where the runtime objects sit relative to the directory holding this binary.
#define RUNTIME_DIR "/../lib/"

// Where the system headers sit relative to the directory holding this binary.
#define INCLUDE_DIR "/../include"


// Values getopt_long returns for the options with no short form.
typedef enum Cc_Option Cc_Option;
enum Cc_Option {
    CC_OPTION_STD = UCHAR_MAX + 1,
    CC_OPTION_INCLUDE
};

// Runtime objects the default (linked) output is always merged with.
static const char *const Cc_RuntimeNames[] = { "crt0.o", "libc.o" };

// The output file this run created.
static const char *Cc_OutputPath;

// Show usage information and exit.
static void Cc_ShowUsage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] INPUT.c\n"
        "  -o OUTPUT   write output to OUTPUT (default: " DEFAULT_OUTPUT ", " STDOUT_NAME " is stdout)\n"
        "  -S          write assembly text instead of an executable\n"
        "  -c          write a relocatable object (.o) instead of an executable\n"
        "  -E          write the preprocessed text instead of an executable\n"
        "  -P          leave line markers out of -E's output\n"
        "  -I DIR      search DIR for included files\n"
        "  -D NAME[=V] define NAME as V (default 1)\n"
        "  -U NAME     undefine NAME\n"
        "  --std=STD   language standard (only " DEFAULT_STD ")\n"
        "  -march=ARCH target architecture (default: " DEFAULT_ARCH ")\n"
        "  -mtarget=T  runtime to link against (default: " DEFAULT_TARGET ")\n"
        "  -B DIR      read the runtime objects from DIR\n",
        prog);
    exit(1);
}

// Remove the output file after an error.
static void Cc_RemoveOutput(void)
{
    if (Err_Status() != ERR_SUCCESS && Cc_OutputPath) {
        remove(Cc_OutputPath);
    }
}

// Return the directory holding this executable.
static char *Cc_GetExeDir(void)
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) {
        return NULL;
    }
    buf[len] = '\0';

    char *slash = strrchr(buf, '/');
    if (! slash) {
        return NULL;
    }
    *slash = '\0';
    return Str_Clone(buf);
}

// Return the directory to read the target's runtime objects from, honouring -B.
static char *Cc_GetRuntimeDir(const char *prefix, const char *target)
{
    if (prefix) {
        return Str_Clone(prefix);
    }

    char *exedir = Cc_GetExeDir();
    Err_Assert(exedir, ERR_CC_RUNTIME_NOT_FOUND);
    char *dir = Str_Format("%s" RUNTIME_DIR "%s", exedir, target);
    Str_Free(exedir);
    return dir;
}

// Return the system include directory.
static char *Cc_GetIncludeDir(void)
{
    char *exedir = Cc_GetExeDir();

    if (! exedir) {
        return NULL;
    }

    char *dir = Str_Format("%s" INCLUDE_DIR, exedir);

    Str_Free(exedir);
    return dir;
}

// Open the output stream.
static FILE *Cc_OpenOutput(const char *output, const char *mode)
{
    if (Str_Equals(output, STDOUT_NAME)) {
        return stdout;
    }

    FILE *out = fopen(output, mode);

    Err_Assert(out, ERR_FILE_ACCESS, output, strerror(errno));
    Cc_OutputPath = output;
    return out;
}

// Close the output stream.
static void Cc_CloseOutput(FILE *out)
{
    if (out == stdout) {
        fflush(out);
    } else {
        fclose(out);
    }
    Cc_OutputPath = NULL;
}

// Write the program as AT&T assembly text.
static void Cc_x86_64_WriteText(FILE *out, Ast_Func *prog)
{
    Gen_x86_64_BuildProgram(prog);
    Txt_x86_64_Att_Write(out);
}

// Write the program as a relocatable object, references left undefined.
static void Cc_x86_64_WriteObject(FILE *out, Ast_Func *prog)
{
    Gen_x86_64_BuildProgram(prog);
    Enc_x86_64_BuildObject();
    Enc_x86_64_Write(out);
}

// Write the program linked against the runtime as a static executable.
static void Cc_x86_64_WriteExec(FILE *out, Ast_Func *prog, const char *prefix, const char *target)
{
    Gen_x86_64_BuildProgram(prog);
    Enc_x86_64_BuildObject();

    size_t nruntime = sizeof(Cc_RuntimeNames) / sizeof(Cc_RuntimeNames[0]);
    char *libdir = Cc_GetRuntimeDir(prefix, target);
    char *runtime[sizeof(Cc_RuntimeNames) / sizeof(Cc_RuntimeNames[0])];
    for (size_t i = 0; i < nruntime; i++) {
        runtime[i] = Str_Format("%s/%s", libdir, Cc_RuntimeNames[i]);
    }

    Elf *obj = Enc_x86_64_GetObject();
    Link_x86_64_Options opts = {
        .lo_entry = "_start"
    };
    Link_x86_64_MergeFiles(obj, (const char *const *) runtime, nruntime);
    Link_x86_64_Exec(obj, &opts);

    Enc_x86_64_Write(out);

    for (size_t i = 0; i < nruntime; i++) {
        Str_Free(runtime[i]);
    }
    Str_Free(libdir);
}

// Main function
int main(int argc, char **argv)
{
    const char *output = NULL;
    const char *arch = DEFAULT_ARCH;
    const char *target = DEFAULT_TARGET;
    const char *prefix = NULL;
    const char **incdirs = NULL;
    size_t nincdirs = 0;
    Str_Buf *cmdline = Str_BufNew();
    bool emit_text = false;
    bool emit_obj = false;
    bool emit_pp = false;
    Pp_Markers markers = PP_MARKERS_EMIT;

    static struct option longopts[] = {
        { "std",     required_argument, NULL, CC_OPTION_STD },
        { "include", required_argument, NULL, CC_OPTION_INCLUDE },
        { 0, 0, 0, 0 }
    };

    Log_SetProgramName(argv[0]);
    atexit(Cc_RemoveOutput);

    int32_t opt;
    while ((opt = getopt_long(argc, argv, "o:cEPSgB:I:D:U:M::l:L:W:f:m:O::", longopts, NULL)) != -1) {
        switch (opt) {
            case 'o': {
                output = optarg;
            } break;
            case 'S': {
                emit_text = true;
            } break;
            case 'c': {
                emit_obj = true;
            } break;
            case 'E': {
                emit_pp = true;
            } break;
            case 'P': {
                markers = PP_MARKERS_OMIT;
            } break;
            case 'B': {
                prefix = optarg;
            } break;
            case 'm': {
                if (Str_StartsWith(optarg, MARCH_PREFIX)) {
                    arch = optarg + strlen(MARCH_PREFIX);
                } else if (Str_StartsWith(optarg, MTARGET_PREFIX)) {
                    target = optarg + strlen(MTARGET_PREFIX);
                }
            } break;
            case 'I': {
                incdirs = realloc(incdirs, (nincdirs + 1) * sizeof(*incdirs));
                incdirs[nincdirs++] = optarg;
            } break;
            case 'D': {
                Pp_PutDefine(cmdline, optarg);
            } break;
            case 'U': {
                Pp_PutUndef(cmdline, optarg);
            } break;
            case 'M': {
                char flag[] = { '-', (char) opt, '\0' };
                Err_Raise(ERR_CC_OPTION_UNSUPPORTED, flag);
            } break;
            case CC_OPTION_STD: {
                Err_Assert(Str_Equals(optarg, DEFAULT_STD), ERR_CC_STD_UNSUPPORTED, optarg, DEFAULT_STD);
            } break;
            case CC_OPTION_INCLUDE: {
                Err_Raise(ERR_CC_OPTION_UNSUPPORTED, "--include");
            } break;
            case 'g':
            case 'l':
            case 'L':
            case 'W':
            case 'f':
            case 'O': {
                // empty
            } break;
            default: {
                Cc_ShowUsage(argv[0]);
            } break;
        }
    }

    Err_Assert(Str_Equals(arch, DEFAULT_ARCH), ERR_CC_ARCH_UNSUPPORTED, arch, DEFAULT_ARCH);

    if (optind >= argc) {
        Cc_ShowUsage(argv[0]);
    }
    const char *input = argv[optind];

    char *outbuf = NULL;
    if (! output) {
        if (emit_pp) {
            output = STDOUT_NAME;
        } else if (emit_text) {
            output = outbuf = Str_ChangeOrAppendExt(input, ".s");
        } else if (emit_obj) {
            output = outbuf = Str_ChangeOrAppendExt(input, ".o");
        } else {
            output = outbuf = Str_Clone(DEFAULT_OUTPUT);
        }
    }

    // Front end: build the AST
    Str_Buf *text = Str_BufNew();
    char *sysdir = Cc_GetIncludeDir();
    Pp_Options pp_opts = {
        .po_dirs    = incdirs,
        .po_ndirs   = nincdirs,
        .po_sysdir  = sysdir,
        .po_cmdline = Str_BufData(cmdline)
    };

    Pp_Run(input, &pp_opts, text);
    Str_BufFree(cmdline);
    Str_Free(sysdir);
    free(incdirs);
    Log_SetLineLocator(Pp_Locate);
    if (emit_pp) {
        FILE *out = Cc_OpenOutput(output, "w");
        Pp_Write(out, text, markers);
        Cc_CloseOutput(out);
        Str_BufFree(text);
        Str_Free(outbuf);
        return 0;
    }
    Par_ParseText(Str_BufData(text), Str_BufLen(text));
    Str_BufFree(text);
    Sem_Analyze(Ast_Program);

    // Back end: emit assembly text or a freestanding executable
    FILE *out = Cc_OpenOutput(output, emit_text ? "w" : "wb");
    if (emit_text) {
        Cc_x86_64_WriteText(out, Ast_Program);
    } else if (emit_obj) {
        Cc_x86_64_WriteObject(out, Ast_Program);
    } else {
        Cc_x86_64_WriteExec(out, Ast_Program, prefix, target);
    }
    Cc_CloseOutput(out);

    if (! emit_text && ! emit_obj && ! Str_Equals(output, STDOUT_NAME)) {
        chmod(output, ELF_MODE);
    }

    Str_Free(outbuf);
    return 0;
}
