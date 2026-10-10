/*
 * C source file for the ivancc compiler driver.
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
#include "cc.h"

// The macros -mtarget=linux predefines, as gcc's under -std=c99.
static const char *const Cc_LinuxMacros[] = { "__linux__", "__linux", "__gnu_linux__", "__unix__", "__unix", NULL };

// The runtime files -mtarget=linux links.
static const char *const Cc_LinuxRuntime[] = { "crt0.o", "libc.a", NULL };

// The macros -mtarget=ivanemu predefines.
static const char *const Cc_EmuMacros[] = { "__ivanemu__", NULL };

// The runtime files -mtarget=ivanemu links.
static const char *const Cc_EmuRuntime[] = { "crt0.o", NULL };

// Every runtime target -mtarget accepts.
static const Cc_Target Cc_Targets[] = {
    { "linux",   Cc_LinuxMacros, Cc_LinuxRuntime },
    { "ivanemu", Cc_EmuMacros,   Cc_EmuRuntime }
};

// The output file this run created.
static const char *Cc_OutputPath;

// Show usage information and exit.
void Cc_ShowUsage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] INPUT.c\n"
        "  -o OUTPUT   write output to OUTPUT (default: " CC_DEFAULT_OUTPUT ", " CC_STDOUT_NAME " is stdout)\n"
        "  -S          write assembly text instead of an executable\n"
        "  -c          write a relocatable object (.o) instead of an executable\n"
        "  -E          write the preprocessed text instead of an executable\n"
        "  -P          leave line markers out of -E's output\n"
        "  -I DIR      search DIR for included files\n"
        "  -D NAME[=V] define NAME as V (default 1)\n"
        "  -U NAME     undefine NAME\n"
        "  --include=F read F before INPUT.c\n"
        "  --M         write a make rule for INPUT.c instead of compiling\n"
        "  --MM        --M without the system headers\n"
        "  --MD[=F]    write the rule to F (default: OUTPUT with .d) and compile\n"
        "  --MMD[=F]   --MD without the system headers\n"
        "  --MP        add an empty rule for each header\n"
        "  --MF=F      write the rule to F\n"
        "  --MT=T      name the rule's target T\n"
        "  --MQ=T      name the rule's target T, escaped for make\n"
        "  --std=STD   language standard (only " CC_DEFAULT_STD ")\n"
        "  -march=ARCH target architecture (default: " CC_DEFAULT_ARCH ")\n"
        "  -mtarget=T  runtime to link against (default: " CC_DEFAULT_TARGET ")\n"
        "  -B DIR      read the runtime objects from DIR\n",
        prog);
    exit(1);
}

// Return the directory holding this executable.
char *Cc_GetExeDir(void)
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
char *Cc_GetRuntimeDir(const char *prefix, const char *arch, const Cc_Target *target)
{
    if (prefix) {
        return Str_Clone(prefix);
    }

    char *exedir = Cc_GetExeDir();
    Err_Assert(exedir, ERR_CC_RUNTIME_NOT_FOUND);
    char *dir = Str_Format("%s" CC_RUNTIME_DIR "%s/%s", exedir, arch, target->ct_name);
    Str_Free(exedir);
    return dir;
}

// Return the system include directory.
char *Cc_GetIncludeDir(void)
{
    char *exedir = Cc_GetExeDir();

    if (! exedir) {
        return NULL;
    }

    char *dir = Str_Format("%s" CC_INCLUDE_DIR, exedir);

    Str_Free(exedir);
    return dir;
}

// Return the runtime target -mtarget names.
const Cc_Target *Cc_FindTarget(const char *name)
{
    for (size_t i = 0; i < sizeof(Cc_Targets) / sizeof(Cc_Targets[0]); i++) {
        if (Str_Equals(Cc_Targets[i].ct_name, name)) {
            return &Cc_Targets[i];
        }
    }
    Err_Raise(ERR_CC_TARGET_NOT_SUPPORTED, name);
    return NULL;
}

// Append the directives that predefine the target's macros.
void Cc_PutTargetMacros(Buf *out, const Cc_Target *target)
{
    for (const char *const *iter = target->ct_macros; *iter; iter++) {
        Pp_PutDefine(out, *iter);
    }
}

// Open the output stream.
FILE *Cc_OpenOutput(const char *output, const char *mode)
{
    if (Str_Equals(output, CC_STDOUT_NAME)) {
        return stdout;
    }

    FILE *out = fopen(output, mode);

    Err_Assert(out, ERR_CC_OUTPUT_NOT_WRITEABLE, output, strerror(errno));
    Cc_OutputPath = output;
    return out;
}

// Close the output stream.
void Cc_CloseOutput(FILE *out)
{
    if (out == stdout) {
        fflush(out);
    } else {
        fclose(out);
    }
    Cc_OutputPath = NULL;
}

// Remove the output file after an error, if it is a regular file.
void Cc_RemoveOutput(void)
{
    struct stat st;

    if (Err_Status() != ERR_SUCCESS && Cc_OutputPath && stat(Cc_OutputPath, &st) == 0 && S_ISREG(st.st_mode)) {
        remove(Cc_OutputPath);
    }
}

// Add a target to the dependency rule.
void Cc_AddTarget(Cc_Depend *dep, char *target)
{
    dep->cd_targets = realloc(dep->cd_targets, (dep->cd_ntargets + 1) * sizeof(*dep->cd_targets));
    dep->cd_targets[dep->cd_ntargets++] = target;
}

// Return the target a rule gets when no --MT or --MQ names one.
char *Cc_DefaultTarget(const char *input, const char *output, Cc_DependMode mode)
{
    if (mode == CC_DEPEND_BESIDE && ! Str_Equals(output, CC_STDOUT_NAME)) {
        return Pp_EscapeMake(output);
    }

    const char *slash = strrchr(input, '/');
    char *object = Str_ModifyExtension(slash ? slash + 1 : input, ".o");
    char *target = Pp_EscapeMake(object);

    Str_Free(object);
    return target;
}

// Return where the rule goes when no --MF names a file.
char *Cc_DefaultDependFile(const char *input, const char *output, Cc_DependMode mode)
{
    if (mode == CC_DEPEND_INSTEAD) {
        return Str_Clone(output);
    }
    return Str_ModifyExtension(Str_Equals(output, CC_STDOUT_NAME) ? input : output, ".d");
}

// Write the dependency rule.
void Cc_WriteDepend(Cc_Depend *dep, const char *input, const char *output)
{
    if (dep->cd_ntargets == 0) {
        Cc_AddTarget(dep, Cc_DefaultTarget(input, output, dep->cd_mode));
    }

    char *path = dep->cd_file ? Str_Clone(dep->cd_file) : Cc_DefaultDependFile(input, output, dep->cd_mode);
    FILE *out = Cc_OpenOutput(path, "w");

    Pp_WriteDepend(out, (const char *const *) dep->cd_targets, dep->cd_ntargets, dep->cd_headers, dep->cd_phony);
    Cc_CloseOutput(out);
    Str_Free(path);
}

// Free the dependency rule's targets.
void Cc_FreeDepend(Cc_Depend *dep)
{
    for (size_t i = 0; i < dep->cd_ntargets; i++) {
        Str_Free(dep->cd_targets[i]);
    }
    free(dep->cd_targets);
}

// Write the program as AT&T assembly text.
void Cc_x86_64_WriteText(FILE *out, Ast_Func *prog)
{
    Gen_x86_64_BuildProgram(prog);
    Txt_x86_64_Att_Write(out);
}

// Write the program as a relocatable object, references left undefined.
void Cc_x86_64_WriteObject(FILE *out, Ast_Func *prog)
{
    Gen_x86_64_BuildProgram(prog);
    Enc_x86_64_BuildObject();
    Enc_x86_64_Write(out);
}

// Write the program linked against the runtime as a static executable.
void Cc_x86_64_WriteExec(FILE *out, Ast_Func *prog, const char *prefix, const char *arch, const Cc_Target *target)
{
    Gen_x86_64_BuildProgram(prog);
    Enc_x86_64_BuildObject();

    size_t nruntime = 0;
    while (target->ct_runtime[nruntime]) {
        nruntime++;
    }
    char *libdir = Cc_GetRuntimeDir(prefix, arch, target);
    char **runtime = calloc(nruntime, sizeof(*runtime));
    for (size_t i = 0; i < nruntime; i++) {
        runtime[i] = Str_Format("%s/%s", libdir, target->ct_runtime[i]);
    }

    Elf *obj = Enc_x86_64_GetObject();
    Link_Options opts = {
        .lo_entry = "_start"
    };
    Link_MergeFiles(obj, (const char *const *) runtime, nruntime, &opts);
    Link_ExecFinalize(obj, &opts);

    Enc_x86_64_Write(out);

    for (size_t i = 0; i < nruntime; i++) {
        Str_Free(runtime[i]);
    }
    free(runtime);
    Str_Free(libdir);
}

// Main function
int main(int argc, char **argv)
{
    const char *output = NULL;
    const char *arch = CC_DEFAULT_ARCH;
    const char *target = CC_DEFAULT_TARGET;
    const char *prefix = NULL;
    const char **incdirs = NULL;
    size_t nincdirs = 0;
    Buf *forced = Buf_New();
    Buf *cmdline = Buf_New();
    Cc_Mode mode = CC_MODE_EXEC;
    Pp_Markers markers = PP_MARKERS_EMIT;
    Cc_Depend dep = {
        .cd_mode     = CC_DEPEND_NONE,
        .cd_file     = NULL,
        .cd_targets  = NULL,
        .cd_ntargets = 0,
        .cd_headers  = PP_HEADERS_ALL,
        .cd_phony    = PP_PHONY_OMIT
    };

    static struct option longopts[] = {
        { "std",     required_argument, NULL, CC_OPTION_STD },
        { "include", required_argument, NULL, CC_OPTION_INCLUDE },
        { "M",       no_argument,       NULL, CC_OPTION_M },
        { "MM",      no_argument,       NULL, CC_OPTION_MM },
        { "MD",      optional_argument, NULL, CC_OPTION_MD },
        { "MMD",     optional_argument, NULL, CC_OPTION_MMD },
        { "MP",      no_argument,       NULL, CC_OPTION_MP },
        { "MF",      required_argument, NULL, CC_OPTION_MF },
        { "MT",      required_argument, NULL, CC_OPTION_MT },
        { "MQ",      required_argument, NULL, CC_OPTION_MQ },
        { 0, 0, 0, 0 }
    };

    Log_SetProgramName(argv[0]);
    atexit(Cc_RemoveOutput);

    int32_t opt;
    while ((opt = getopt_long(argc, argv, "o:cEPSgB:I:D:U:l:L:W:f:m:O::", longopts, NULL)) != -1) {
        switch (opt) {
            case 'o': {
                output = optarg;
            } break;
            case 'S': {
                if (mode > CC_MODE_TEXT) {
                    mode = CC_MODE_TEXT;
                }
            } break;
            case 'c': {
                if (mode > CC_MODE_OBJECT) {
                    mode = CC_MODE_OBJECT;
                }
            } break;
            case 'E': {
                mode = CC_MODE_PP;
            } break;
            case 'P': {
                markers = PP_MARKERS_OMIT;
            } break;
            case 'B': {
                prefix = optarg;
            } break;
            case 'm': {
                if (Str_StartsWith(optarg, CC_MARCH_PREFIX)) {
                    arch = optarg + strlen(CC_MARCH_PREFIX);
                } else if (Str_StartsWith(optarg, CC_MTARGET_PREFIX)) {
                    target = optarg + strlen(CC_MTARGET_PREFIX);
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
            case CC_OPTION_STD: {
                Err_Assert(Str_Equals(optarg, CC_DEFAULT_STD), ERR_CC_STD_NOT_SUPPORTED, optarg, CC_DEFAULT_STD);
            } break;
            case CC_OPTION_INCLUDE: {
                Pp_PutInclude(forced, optarg);
            } break;
            case CC_OPTION_M: {
                dep.cd_mode = CC_DEPEND_INSTEAD;
                dep.cd_headers = PP_HEADERS_ALL;
            } break;
            case CC_OPTION_MM: {
                dep.cd_mode = CC_DEPEND_INSTEAD;
                dep.cd_headers = PP_HEADERS_USER;
            } break;
            case CC_OPTION_MD: {
                if (dep.cd_mode == CC_DEPEND_NONE) {
                    dep.cd_mode = CC_DEPEND_BESIDE;
                }
                dep.cd_headers = PP_HEADERS_ALL;
                if (optarg) {
                    dep.cd_file = optarg;
                }
            } break;
            case CC_OPTION_MMD: {
                if (dep.cd_mode == CC_DEPEND_NONE) {
                    dep.cd_mode = CC_DEPEND_BESIDE;
                }
                dep.cd_headers = PP_HEADERS_USER;
                if (optarg) {
                    dep.cd_file = optarg;
                }
            } break;
            case CC_OPTION_MP: {
                dep.cd_phony = PP_PHONY_EMIT;
            } break;
            case CC_OPTION_MF: {
                dep.cd_file = optarg;
            } break;
            case CC_OPTION_MT: {
                Cc_AddTarget(&dep, Str_Clone(optarg));
            } break;
            case CC_OPTION_MQ: {
                Cc_AddTarget(&dep, Pp_EscapeMake(optarg));
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

    Err_Assert(Str_Equals(arch, CC_DEFAULT_ARCH), ERR_CC_ARCH_NOT_SUPPORTED, arch, CC_DEFAULT_ARCH);
    const Cc_Target *runtime = Cc_FindTarget(target);
    Buf *directives = Buf_New();
    Cc_PutTargetMacros(directives, runtime);
    Buf_PutBytes(directives, Buf_Data(cmdline), Buf_Len(cmdline));
    Buf_PutBytes(directives, Buf_Data(forced), Buf_Len(forced));
    Buf_Free(cmdline);
    Buf_Free(forced);

    if (optind >= argc) {
        Cc_ShowUsage(argv[0]);
    }
    const char *input = argv[optind];
    Err_Assert(optind + 1 >= argc, ERR_CC_TOO_MANY_INPUTS, argv[optind + 1], input);

    char *outbuf = NULL;
    if (! output) {
        if (mode == CC_MODE_PP || dep.cd_mode == CC_DEPEND_INSTEAD) {
            output = CC_STDOUT_NAME;
        } else if (mode == CC_MODE_TEXT) {
            output = outbuf = Str_ModifyExtension(input, ".s");
        } else if (mode == CC_MODE_OBJECT) {
            output = outbuf = Str_ModifyExtension(input, ".o");
        } else {
            output = outbuf = Str_Clone(CC_DEFAULT_OUTPUT);
        }
    }

    // Phase: front end
    Buf *text = Buf_New();
    char *sysdir = Cc_GetIncludeDir();
    Pp_Options pp_opts = {
        .po_dirs    = incdirs,
        .po_ndirs   = nincdirs,
        .po_sysdir  = sysdir,
        .po_cmdline = Buf_Data(directives)
    };

    Pp_Run(input, &pp_opts, text);
    Buf_Free(directives);
    Str_Free(sysdir);
    free(incdirs);
    Log_SetLineLocator(Pp_Locate);
    if (dep.cd_mode != CC_DEPEND_NONE) {
        Cc_WriteDepend(&dep, input, output);
    }
    Cc_FreeDepend(&dep);
    if (dep.cd_mode == CC_DEPEND_INSTEAD) {
        Buf_Free(text);
        Str_Free(outbuf);
        return 0;
    }
    if (mode == CC_MODE_PP) {
        FILE *out = Cc_OpenOutput(output, "w");
        Pp_Write(out, text, markers);
        Cc_CloseOutput(out);
        Buf_Free(text);
        Str_Free(outbuf);
        return 0;
    }
    Par_ParseText(Buf_Data(text), Buf_Len(text));
    Buf_Free(text);
    Sem_Analyze(Ast_Program);

    // Phase: back end
    FILE *out = Cc_OpenOutput(output, mode == CC_MODE_TEXT ? "w" : "wb");
    if (mode == CC_MODE_TEXT) {
        Cc_x86_64_WriteText(out, Ast_Program);
    } else if (mode == CC_MODE_OBJECT) {
        Cc_x86_64_WriteObject(out, Ast_Program);
    } else {
        Cc_x86_64_WriteExec(out, Ast_Program, prefix, arch, runtime);
    }
    Cc_CloseOutput(out);

    if (mode == CC_MODE_EXEC && ! Str_Equals(output, CC_STDOUT_NAME)) {
        chmod(output, CC_ELF_MODE);
    }

    Str_Free(outbuf);
    return 0;
}
