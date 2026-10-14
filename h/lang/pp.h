/*
 * C header file for the C preprocessor.
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

#ifndef PP_H
#define PP_H

// Standard headers.
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/buf.h"
#include "util/str.h"
#include "lang/ast.h"
#include "lang/par.h"

// Trigraph keys and the characters they stand for.
// [C99:5.2.1.1p1]: the nine trigraph sequences and what replaces each
#define PP_TRIGRAPH_KEYS   "=(/)'<!>-"
#define PP_TRIGRAPH_VALUES "#[\\]^{|}~"

// Zero bytes the scanner needs after a file's text.
#define PP_SCAN_PADDING 2

// Tokens a file's token array starts with.
#define PP_TOKENS_MIN 64

// Letters that start a pp-number's signed exponent.
// [C99:6.4.8p1]: pp-numbers, signed only after e, E, p or P
#define PP_EXPONENT_CHARS "eEpP"

// Longest punctuator spelling.
// [C99:6.4.6p1]: the punctuators
#define PP_PUNCT_MAX_LEN 4

// Most blank lines printed to bring the output level with its source.
#define PP_PAD_MAX 8

// File index of no file.
#define PP_FILE_NONE UINT32_MAX

// Search-list index of a file found outside the search list.
#define PP_DIR_NONE UINT32_MAX

// Deepest nesting of includes.
// [C99:5.2.4.1p1]: at least 15 nesting levels of #include
#define PP_INCLUDE_DEPTH_MAX 200

// Number of a file's first line.
// [C99:6.10.4p2]: a line's number, one past the new-lines before it
#define PP_LINE_FIRST 1

// Buckets in the macro table.
#define PP_MACRO_BUCKETS 1024

// FNV-1a offset basis and prime for hashing macro names.
#define PP_HASH_OFFSET 2166136261u
#define PP_HASH_PRIME  16777619u

// Name of the file that -D and -U turn into.
#define PP_CMDLINE_NAME "<command line>"

// Name of the file that defines the predefined macros.
#define PP_BUILTIN_NAME "<built-in>"

// Column a dependency rule breaks its line after.
#define PP_DEPEND_COLUMNS 72

// strftime formats of the -D arguments that define __DATE__ and __TIME__.
// [C99:6.10.8p1]: __DATE__ as "Mmm dd yyyy", __TIME__ as "hh:mm:ss"
#define PP_DATE_FORMAT "__DATE__=\"%b %e %Y\""
#define PP_TIME_FORMAT "__TIME__=\"%H:%M:%S\""

// -D arguments for __DATE__ and __TIME__ when the time is unknown.
#define PP_DATE_UNKNOWN "__DATE__=\"??? ?? ????\""
#define PP_TIME_UNKNOWN "__TIME__=\"??:??:??\""

// Room for a -D argument strftime writes.
#define PP_STAMP_SIZE 32

// Largest line number #line may give.
// [C99:6.10.4p3]: #line's numbers, 1 to 2147483647
#define PP_LINE_MAX 2147483647

// Name a variadic macro's body gives its extra arguments.
// [C99:6.10.3.1p2]: __VA_ARGS__
#define PP_VA_ARGS "__VA_ARGS__"

// Name of the operator that asks whether a macro is defined.
// [C99:6.10.1p1]: the defined operator
#define PP_DEFINED "defined"

// Name of the operator that runs a string as a pragma.
// [C99:6.10.9p1]: the _Pragma operator
#define PP_PRAGMA_OPERATOR "_Pragma"

// Text a _Pragma operand is run after.
// [C99:6.10.9p1]: _Pragma's operand, run as a #pragma
#define PP_PRAGMA_DIRECTIVE "#pragma "

// Spellings of the answers defined gives.
// [C99:6.10.1p1]: defined's values, 1 and 0
#define PP_TEXT_TRUE  "1"
#define PP_TEXT_FALSE "0"

// Values of the answers a truth operator gives.
#define PP_VALUE_TRUE  1
#define PP_VALUE_FALSE 0

// Bits in the value of an #if expression.
// [C99:6.10.1p4]: #if arithmetic in intmax_t and uintmax_t
#define PP_VALUE_BITS (sizeof(uintmax_t) * CHAR_BIT)

// Forward declaration: a token's hide set is a list of these.
typedef struct Pp_HideSet Pp_HideSet;

// Forward declaration: a hide set names these.
typedef struct Pp_Macro Pp_Macro;

// Kinds of preprocessing token.
// [C99:6.4p1]: the seven kinds of preprocessing token
typedef enum Pp_TokenKind Pp_TokenKind;
enum Pp_TokenKind {
    PP_TOKEN_EOF,          // the end of a file's tokens
    PP_TOKEN_IDENT,        // an identifier
    PP_TOKEN_NUMBER,       // a pp-number
    PP_TOKEN_CHAR,         // a character constant
    PP_TOKEN_STRING,       // a string literal
    PP_TOKEN_HEADER_NAME,  // a header name, after #include
    PP_TOKEN_PUNCT,        // a punctuator
    PP_TOKEN_OTHER,        // a character no other kind takes
    PP_TOKEN_SPACE,        // whitespace or a comment
    PP_TOKEN_NEWLINE,      // the end of a line
    PP_TOKEN_OPEN_COMMENT, // a comment with no end
    PP_TOKEN_PLACEMARKER,  // an empty argument next to ##
    PP_TOKEN_KIND_COUNT    // number of kinds
};

// What stood before a token.
typedef enum Pp_Flags Pp_Flags;
enum Pp_Flags {
    PP_FLAG_NONE  = 0,      // neither below
    PP_FLAG_BOL  = 1 << 0, // first token on its line
    PP_FLAG_SPACE = 1 << 1  // whitespace before it
};

// Kinds of macro.
// [C99:6.10.3p9]:  object-like macros
// [C99:6.10.3p10]: function-like macros
typedef enum Pp_MacroKind Pp_MacroKind;
enum Pp_MacroKind {
    PP_MACRO_OBJECT,     // defined with no parameter list
    PP_MACRO_FUNCTION,   // defined with a parameter list
    PP_MACRO_BUILTIN,    // expanded by the preprocessor itself
    PP_MACRO_KIND_COUNT  // number of kinds
};

// Macros whose expansion the preprocessor computes.
typedef enum Pp_Builtin Pp_Builtin;
enum Pp_Builtin {
    PP_BUILTIN_NONE,    // a macro the preprocessor does not compute
    PP_BUILTIN_FILE,    // __FILE__
    PP_BUILTIN_LINE,    // __LINE__
    PP_BUILTIN_COUNTER, // __COUNTER__, GNU's
    PP_BUILTIN_COUNT    // number of builtins
};

// Which directive an include came from.
typedef enum Pp_Include Pp_Include;
enum Pp_Include {
    PP_INCLUDE_PLAIN, // #include
    PP_INCLUDE_NEXT   // #include_next, GNU's
};

// Flag a line marker carries.
typedef enum Pp_Move Pp_Move;
enum Pp_Move {
    PP_MOVE_NONE   = 0, // staying in the file
    PP_MOVE_ENTER  = 1, // entering a file
    PP_MOVE_RETURN = 2  // returning to a file
};

// Whether written text carries line markers.
typedef enum Pp_Markers Pp_Markers;
enum Pp_Markers {
    PP_MARKERS_OMIT, // no line markers, as -P asks
    PP_MARKERS_EMIT  // a line marker wherever the source moves
};

// Which headers a dependency rule names.
typedef enum Pp_Headers Pp_Headers;
enum Pp_Headers {
    PP_HEADERS_ALL,  // every header, as -M and -MD ask
    PP_HEADERS_USER  // system headers left out, as -MM and -MMD ask
};

// Whether a dependency rule is followed by an empty rule for each header.
typedef enum Pp_Phony Pp_Phony;
enum Pp_Phony {
    PP_PHONY_OMIT, // no empty rules
    PP_PHONY_EMIT  // an empty rule for each header, as -MP asks
};

// Whether an operand of an #if expression is computed.
typedef enum Pp_Eval Pp_Eval;
enum Pp_Eval {
    PP_EVAL_COMPUTE, // its value is computed
    PP_EVAL_SKIP    // parsed for its type alone
};

// Binary operators of an #if expression.
typedef enum Pp_Op Pp_Op;
enum Pp_Op {
    PP_OP_MUL,     // *
    PP_OP_DIV,     // /
    PP_OP_MOD,     // %
    PP_OP_ADD,     // +
    PP_OP_SUB,     // -
    PP_OP_SHL,     // <<
    PP_OP_SHR,     // >>
    PP_OP_LT,      // <
    PP_OP_GT,      // >
    PP_OP_LE,      // <=
    PP_OP_GE,      // >=
    PP_OP_EQ,      // ==
    PP_OP_NE,      // !=
    PP_OP_BIT_AND, // &
    PP_OP_BIT_XOR, // ^
    PP_OP_BIT_OR,  // |
    PP_OP_AND,     // &&
    PP_OP_OR,      // ||
    PP_OP_COUNT    // number of operators
};

// Precedence levels of the binary operators.
typedef enum Pp_Prec Pp_Prec;
enum Pp_Prec {
    PP_PREC_OR,             // ||, the loosest
    PP_PREC_AND,            // &&
    PP_PREC_BIT_OR,         // |
    PP_PREC_BIT_XOR,        // ^
    PP_PREC_BIT_AND,        // &
    PP_PREC_EQUALITY,       // == and !=
    PP_PREC_RELATIONAL,     // <, >, <= and >=
    PP_PREC_SHIFT,          // << and >>
    PP_PREC_ADDITIVE,       // + and -
    PP_PREC_MULTIPLICATIVE, // *, / and %, the tightest
    PP_PREC_COUNT           // number of levels
};

// Options controlling a preprocessor run.
typedef struct Pp_Options Pp_Options;
struct Pp_Options {
    const char *const *po_dirs;    // -I directories, in the order given
    size_t             po_ndirs;   // count of po_dirs
    const char        *po_sysdir;  // system include directory
    const char        *po_cmdline; // -D, -U and --include as directives
};

// One preprocessing token.
typedef struct Pp_Token Pp_Token;
struct Pp_Token {
    Pp_TokenKind pt_kind;  // what sort of token it is
    const char  *pt_text;  // spelling without a NUL
    size_t       pt_len;   // bytes in pt_text
    Pp_Flags     pt_flags; // what stood before it
    uint32_t     pt_file;  // index into the opened-file list
    Ast_Line     pt_line;  // its line in that file
    Pp_HideSet  *pt_hide;  // macros this token came out of
    Pp_Token    *pt_next;  // next token of an expansion
};

// One macro in a hide set.
struct Pp_HideSet {
    const Pp_Macro *ph_macro; // the macro hidden
    Pp_HideSet     *ph_next;  // the set's next macro
};

// One macro definition.
struct Pp_Macro {
    char            *ma_name;     // the name it is defined under
    Pp_MacroKind     ma_kind;     // object-like, function-like or builtin
    const Pp_Token **ma_params;   // names, then ... if variadic
    size_t           ma_nparams;  // count of ma_params
    bool             ma_variadic; // its last parameter is ...
    const Pp_Token  *ma_body;     // replacement list, inside its file's tokens
    size_t           ma_nbody;    // tokens in ma_body
    Pp_Builtin       ma_builtin;  // which builtin a PP_MACRO_BUILTIN is
    Pp_Macro        *ma_next;     // next macro in its bucket
};

// One argument of a macro call.
// [C99:6.10.3.1p1]: an argument, macro-replaced unless by # or ##
typedef struct Pp_Arg Pp_Arg;
struct Pp_Arg {
    Pp_Token *pa_raw;      // tokens as written
    Pp_Token *pa_full;     // tokens fully expanded
    bool      pa_expanded; // pa_full is set
};

// One source file, read and tokenized.
typedef struct Pp_File Pp_File;
struct Pp_File {
    char           *pf_path;    // path it was opened under
    uint32_t        pf_index;   // position in the opened-file list
    uint32_t        pf_dir;     // search-list index it was found under
    bool            pf_system;  // found in the system include directory
    char           *pf_text;    // text after the pre-pass
    size_t          pf_len;     // bytes in pf_text
    Pp_Token       *pf_tokens;  // ending in a PP_TOKEN_EOF
    size_t          pf_ntokens; // count of pf_tokens
    const Pp_Token *pf_guard;   // macro whose definition empties the file
    bool            pf_once;    // ran #pragma once
};

// A token stream over a range of a file, with expansions pushed in front.
typedef struct Pp_Reader Pp_Reader;
struct Pp_Reader {
    const Pp_File *rd_file;    // file whose tokens it reads
    size_t         rd_pos;     // index of the next token in it
    size_t         rd_end;     // index it stops at
    Pp_Token      *rd_pending; // expansion tokens read before rd_pos
    Pp_Flags       rd_carry;   // flags of a macro that expanded to nothing
};

// Origin of a stretch of output lines.
typedef struct Pp_MapEntry Pp_MapEntry;
struct Pp_MapEntry {
    Ast_Line    pm_output; // first output line of the stretch
    const char *pm_name;   // file name, as #line gives it
    Ast_Line    pm_source; // source line of its first line
    Pp_Move     pm_move;   // flag its line marker carries
};

// Where the file being read is, as #line presents it.
typedef struct Pp_Place Pp_Place;
struct Pp_Place {
    const char *pl_name;  // its path, or the name #line gave it
    Ast_Line    pl_shift; // what #line added to its line numbers
};

// Where the printer has got to.
typedef struct Pp_Printer Pp_Printer;
struct Pp_Printer {
    Buf            *pr_out;    // text printed so far
    Ast_Line        pr_line;   // output line being printed
    uint32_t        pr_file;   // file the output line comes from
    Ast_Line        pr_source; // source line the output line comes from
    const Pp_Token *pr_prev;   // last token printed on the output line
    Pp_Move         pr_move;   // flag the next map entry takes
};

// One open conditional.
typedef struct Pp_Cond Pp_Cond;
struct Pp_Cond {
    const Pp_Token *pc_name;  // directive that opened it
    bool            pc_taken; // one of its groups was kept
    bool            pc_else;  // its #else was seen
    Pp_Cond        *pc_next;  // enclosing conditional
};

// The value of an #if expression.
// [C99:6.10.1p4]: #if arithmetic in intmax_t and uintmax_t
typedef struct Pp_Value Pp_Value;
struct Pp_Value {
    uintmax_t pv_bits;     // its bits, read by pv_unsigned's type
    bool      pv_unsigned; // a uintmax_t rather than an intmax_t
};

// The tokens an #if expression is read from.
typedef struct Pp_Expr Pp_Expr;
struct Pp_Expr {
    const Pp_Token *pe_tok;  // next token to read
    Ast_Line        pe_line; // line of the directive
};

// Files
// [C99:5.1.1.2p1]: translation phases 1 to 3
// [C99:6.10.9p1]:  _Pragma's operand, destringized
Pp_File        *Pp_FindFile(const char *path);
Pp_File        *Pp_OpenFile(const char *path, uint32_t dir);
char           *Pp_FileGetContents(const char *path, size_t *len);
Pp_File        *Pp_OpenText(const char *path, const char *raw, size_t len, uint32_t dir);
Pp_File        *Pp_OpenPragma(const Pp_Token *str, Ast_Line line);
const Pp_Token *Pp_FindGuard(const Pp_File *file);
void            Pp_ReplaceTrigraphs(Buf *out, const char *text, size_t len);
void            Pp_DeleteSplices(Buf *out, const char *text, size_t len);
void            Pp_Tokenize(Pp_File *file);
void            Pp_CloseFile(Pp_File *file);

// Tokens
// [C99:6.4p4]: the longest match for each preprocessing token
Pp_Token *Pp_CopyToken(const Pp_Token *tok);
Pp_Token *Pp_CopyList(const Pp_Token *list);
bool      Pp_TokenEquals(const Pp_Token *tok, const char *text);
bool      Pp_SameSpelling(const Pp_Token *a, const Pp_Token *b);
bool      Pp_IsHash(const Pp_Token *tok);
bool      Pp_IsHashHash(const Pp_Token *tok);
bool      Pp_ExpectsHeaderName(const Pp_File *file);
bool      Pp_IsPunctPrefix(const char *text, size_t len);
bool      Pp_NeedsSpace(const Pp_Token *prev, const Pp_Token *next);
bool      Pp_LexOne(const char *text, size_t len, Pp_TokenKind *kind);
char     *Pp_QuoteName(const char *name);

// Macros
// [C99:6.10.3p2]:   redefinition, only identical
// [C99:6.10.3.5p2]: #undef
// [C99:6.10.8]:     the predefined macros
bool            Pp_IsMacroName(const Pp_Token *tok);
const Pp_Token *Pp_MacroNameAt(const Pp_File *file, size_t pos);
uint32_t        Pp_HashName(const char *text, size_t len);
Pp_Macro       *Pp_FindMacro(const char *text, size_t len);
bool            Pp_FindParam(const Pp_Macro *macro, const Pp_Token *tok, size_t *index);
bool            Pp_SameMacro(const Pp_Macro *macro, const Pp_Macro *def);
void            Pp_DefineMacro(const Pp_Token *name, const Pp_Macro *def);
void            Pp_UndefMacro(const Pp_Token *name);
void            Pp_PutDefine(Buf *cmdline, const char *arg);
void            Pp_PutUndef(Buf *cmdline, const char *name);
void            Pp_PutInclude(Buf *cmdline, const char *path);
void            Pp_PutPredefined(Buf *out);
void            Pp_DefineBuiltins(void);

// Hide sets
// [C99:6.10.3.4p2]: a macro's own name, not replaced in its expansion
bool        Pp_HideSetHas(const Pp_HideSet *set, const Pp_Macro *macro);
Pp_HideSet *Pp_HideSetAdd(Pp_HideSet *set, const Pp_Macro *macro);
Pp_HideSet *Pp_HideSetIntersect(const Pp_HideSet *a, const Pp_HideSet *b);
Pp_HideSet *Pp_HideSetUnion(Pp_HideSet *a, const Pp_HideSet *b);

// Expansion
// [C99:6.10.3.1p1]: argument substitution
// [C99:6.10.3.2p2]: the # operator
// [C99:6.10.3.3p3]: the ## operator
// [C99:6.10.3.4p1]: rescanning
// [C99:6.10.8p1]:   __FILE__ and __LINE__
const Pp_Token *Pp_PeekToken(const Pp_Reader *rd);
const Pp_Token *Pp_ReadToken(Pp_Reader *rd);
Pp_Arg         *Pp_ReadArgs(Pp_Reader *rd, const Pp_Macro *macro, const Pp_Token *name, const Pp_Token **close);
Pp_Token       *Pp_ExpandArg(Pp_Arg *arg);
Pp_Token       *Pp_Stringize(const Pp_Token *list, const Pp_Token *hash);
void            Pp_PasteTokens(Pp_Token *left, const Pp_Token *right, Ast_Line line);
Pp_Token       *Pp_Substitute(const Pp_Macro *macro, Pp_Arg *args, const Pp_Token *name);
Pp_Token       *Pp_ExpandBuiltin(const Pp_Macro *macro, const Pp_Token *name);
bool            Pp_ExpandMacro(Pp_Reader *rd, const Pp_Token *tok);
const Pp_Token *Pp_ReadExpanded(Pp_Reader *rd);
Pp_Token       *Pp_ExpandAll(Pp_Reader *rd);
Pp_Token       *Pp_ExpandRange(const Pp_File *file, size_t start, size_t end);

// Include search
// [C99:6.10.2p2]: #include <h>
// [C99:6.10.2p3]: #include "q", then as <q>
void      Pp_SetDirs(const Pp_Options *opts);
char     *Pp_DirName(const char *path);
char     *Pp_JoinPath(const char *dir, const char *name);
Pp_Token *Pp_HeaderFromTokens(const Pp_Token *list, Ast_Line line);
char     *Pp_FindInclude(const Pp_File *from, const Pp_Token *operand, Pp_Include kind, uint32_t *dir);
bool      Pp_FileExists(const char *path);

// Line map
Pp_Place    Pp_FilePlace(const Pp_File *file);
void        Pp_AddMapEntry(Ast_Line output, const char *name, Ast_Line source, Pp_Move move);
const char *Pp_Locate(Ast_Line line, Ast_Line *source);
const char *Pp_LocateSource(Ast_Line line, Ast_Line *source);

// Printing
void Pp_BreakLine(Pp_Printer *pr);
void Pp_SyncLine(Pp_Printer *pr, const Pp_Token *tok);
void Pp_Unsync(Pp_Printer *pr);
void Pp_PrintToken(Pp_Printer *pr, const Pp_Token *tok);
void Pp_Write(FILE *out, const Buf *text, Pp_Markers markers);

// Directives
// [C99:6.10.2]:     #include
// [C99:6.10.3]:     #define
// [C99:6.10.3.5p2]: #undef
// [C99:6.10.4p3]:   #line
// [C99:6.10.4p4]:   #line with a file name
// [C99:6.10.5p1]:   #error
// [C99:6.10.6p1]:   #pragma, an unknown one ignored
// [C99:6.10.9p1]:   _Pragma
bool     Pp_IsDirective(const Pp_Token *tok);
size_t   Pp_SkipLine(const Pp_File *file, size_t pos);
size_t   Pp_RunDirective(Pp_Printer *pr, const Pp_File *file, size_t pos);
void     Pp_RunInclude(Pp_Printer *pr, const Pp_File *from, size_t pos, Pp_Include kind);
void     Pp_RunDefine(const Pp_File *file, size_t pos);
size_t   Pp_ReadParams(const Pp_File *file, size_t pos, size_t end, Pp_Macro *def);
void     Pp_CheckBody(const Pp_Macro *def, Ast_Line line);
void     Pp_RunUndef(const Pp_File *file, size_t pos);
void     Pp_RunLine(Pp_Printer *pr, const Pp_File *file, size_t pos);
Ast_Line Pp_ReadLineNumber(const Pp_Token *tok, Ast_Line line);
void     Pp_RunPragma(const Pp_File *file, const Pp_File *text, size_t pos);
void     Pp_RunPragmaOperator(Pp_Reader *rd, const Pp_Token *op);
void     Pp_RunError(const Pp_File *file, size_t pos);
void     Pp_RunWarning(const Pp_File *file, size_t pos);
char    *Pp_LineText(const Pp_File *file, size_t pos);

// Conditionals
// [C99:6.10.1p6]: #if, #elif and #else, the first true group kept
bool      Pp_OpensCond(const Pp_Token *name);
size_t    Pp_RunIf(const Pp_File *file, size_t pos);
size_t    Pp_RunElif(const Pp_File *file, size_t pos);
size_t    Pp_RunElse(const Pp_File *file, size_t pos);
size_t    Pp_RunEndif(const Pp_File *file, size_t pos);
void      Pp_CheckCond(const Pp_Token *name);
void      Pp_CheckLineEnd(const Pp_Token *next, const Pp_Token *name);
bool      Pp_IsDefined(const Pp_File *file, size_t pos);
size_t    Pp_SkipGroup(const Pp_File *file, size_t pos);
bool      Pp_EvalLine(const Pp_File *file, size_t pos);
Pp_Token *Pp_ExpandCondition(const Pp_File *file, size_t start, size_t end);
Pp_Token *Pp_ReadDefined(Pp_Reader *rd, const Pp_Token *op);

// Expressions
// [C99:6.10.1p4]: #if arithmetic, by 6.6's rules in intmax_t and uintmax_t
Pp_Value Pp_EvalComma(Pp_Expr *ex, Pp_Eval mode);
Pp_Value Pp_EvalCond(Pp_Expr *ex, Pp_Eval mode);
Pp_Value Pp_EvalBinary(Pp_Expr *ex, Pp_Prec min, Pp_Eval mode);
Pp_Value Pp_EvalUnary(Pp_Expr *ex, Pp_Eval mode);
Pp_Value Pp_EvalNumber(const Pp_Token *tok, Ast_Line line);
Pp_Value Pp_EvalChar(const Pp_Token *tok, Ast_Line line);
bool     Pp_IsFloat(const Pp_Token *tok);
bool     Pp_ReadSuffix(const char *suffix, bool *marked);
bool     Pp_FindOp(const Pp_Token *tok, Pp_Op *op);
Pp_Value Pp_ApplyOp(Pp_Op op, Pp_Value a, Pp_Value b, Pp_Eval mode, Ast_Line line);
Pp_Value Pp_Divide(Pp_Op op, Pp_Value a, Pp_Value b, Pp_Eval mode, Ast_Line line);
Pp_Value Pp_Shift(Pp_Op op, Pp_Value val, Pp_Value count);
bool     Pp_IsTrue(Pp_Value val);

// Running
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file);
void Pp_Run(const char *path, const Pp_Options *opts, Buf *out);

// Dependencies
char  *Pp_EscapeMake(const char *name);
bool   Pp_IsDependency(const Pp_File *file, Pp_Headers headers);
size_t Pp_WriteName(FILE *out, const char *name, size_t col);
void   Pp_WriteDepend(FILE *out, const char *const *targets, size_t ntargets, Pp_Headers headers, Pp_Phony phony);

#endif // PP_H
