/*
 * C header file for the ivcc compiler driver.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivcc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CC_H
#define CC_H

// Standard headers.
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/link.h"
#include "util/buf.h"
#include "util/str.h"
#include "lang/ast.h"
#include "lang/par.h"
#include "lang/pp.h"
#include "lang/sem.h"
#include "arch/x86_64/enc.h"
#include "arch/x86_64/gen.h"
#include "arch/x86_64/txt.h"

// Permission bits for the executables cc writes (rwxr-xr-x).
#define CC_ELF_MODE 0755

// Default output name for a freestanding executable.
#define CC_DEFAULT_OUTPUT "a.out"

// Output name that means standard output rather than a file.
#define CC_STDOUT_NAME "-"

// The one language standard --std accepts.
#define CC_DEFAULT_STD "c99"

// Target architecture selected when no -march= is given.
#define CC_DEFAULT_ARCH "x86_64"

// Runtime target selected when no -mtarget= is given.
#define CC_DEFAULT_TARGET "linux"

// Machine-option prefixes recognised inside -m (e.g. -march=x86_64).
#define CC_MARCH_PREFIX "arch="
#define CC_MTARGET_PREFIX "target="

// Where the runtime objects sit relative to the directory holding this binary.
#define CC_RUNTIME_DIR "/../lib/"

// Where the system headers sit relative to the directory holding this binary.
#define CC_INCLUDE_DIR "/../include"

// Values getopt_long returns for the options with no short form.
typedef enum Cc_Option Cc_Option;
enum Cc_Option {
    CC_OPTION_STD = UCHAR_MAX + 1, // -std=
    CC_OPTION_INCLUDE,             // -include
    CC_OPTION_M,                   // -M
    CC_OPTION_MM,                  // -MM
    CC_OPTION_MD,                  // -MD
    CC_OPTION_MMD,                 // -MMD
    CC_OPTION_MP,                  // -MP
    CC_OPTION_MF,                  // -MF
    CC_OPTION_MT,                  // -MT
    CC_OPTION_MQ                   // -MQ
};

// Where a run stops.
typedef enum Cc_Mode Cc_Mode;
enum Cc_Mode {
    CC_MODE_PP,     // -E
    CC_MODE_TEXT,   // -S
    CC_MODE_OBJECT, // -c
    CC_MODE_EXEC    // a linked executable
};

// What a run does with its dependency rule.
typedef enum Cc_DependMode Cc_DependMode;
enum Cc_DependMode {
    CC_DEPEND_NONE,    // no rule
    CC_DEPEND_INSTEAD, // write the rule instead of compiling
    CC_DEPEND_BESIDE   // write the rule and compile
};

// How a run writes its dependency rule.
typedef struct Cc_Depend Cc_Depend;
struct Cc_Depend {
    Cc_DependMode cd_mode;     // whether a rule is written, and with a compile
    const char   *cd_file;     // --MF or --MD file
    char        **cd_targets;  // escaped for make
    size_t        cd_ntargets; // count of cd_targets
    Pp_Headers    cd_headers;  // whether system headers are named
    Pp_Phony      cd_phony;    // whether each header gets an empty rule
};

// A runtime target: the macros it predefines and the files it links.
typedef struct Cc_Target Cc_Target;
struct Cc_Target {
    const char        *ct_name;     // the name -mtarget takes
    const char *const *ct_macros;   // NULL-terminated
    const char *const *ct_runtime;  // NULL-terminated, linked before -l's
    const char *const *ct_libs;     // NULL-terminated, linked after -l's
};

// Usage
void Cc_ShowUsage(const char *prog);

// Directories
char *Cc_GetExeDir(void);
char *Cc_GetRuntimeDir(const char *prefix, const char *arch, const Cc_Target *target);
char *Cc_GetIncludeDir(void);

// Targets
const Cc_Target *Cc_FindTarget(const char *name);
void             Cc_PutTargetMacros(Buf *out, const Cc_Target *target);

// Output
FILE *Cc_OpenOutput(const char *output, const char *mode);
void  Cc_CloseOutput(FILE *out);
void  Cc_RemoveOutput(void);

// Dependencies
void  Cc_AddTarget(Cc_Depend *dep, char *target);
char *Cc_DefaultTarget(const char *input, const char *output, Cc_DependMode mode);
char *Cc_DefaultDependFile(const char *input, const char *output, Cc_DependMode mode);
void  Cc_WriteDepend(Cc_Depend *dep, const char *input, const char *output);
void  Cc_FreeDepend(Cc_Depend *dep);

// Writing
void Cc_x86_64_WriteText(FILE *out, Ast_Func *prog);
void Cc_x86_64_WriteObject(FILE *out, Ast_Func *prog);
void Cc_x86_64_WriteExec(FILE *out, Ast_Func *prog, const char *prefix, const char *arch, const Cc_Target *target,
                         const char **libs, size_t nlibs);

#endif // CC_H
