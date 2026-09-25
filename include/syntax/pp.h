// C header file for the C preprocessor.

#ifndef PP_H
#define PP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "util/str.h"
#include "syntax/ast.h"

// Trigraph keys and the characters they stand for.
#define PP_TRIGRAPH_KEYS   "=(/)'<!>-"
#define PP_TRIGRAPH_VALUES "#[\\]^{|}~"

// Zero bytes the scanner needs after a file's text.
#define PP_SCAN_PADDING 2

// Tokens a file's token array starts with.
#define PP_TOKENS_MIN 64

// Letters that start a pp-number's signed exponent.
#define PP_EXPONENT_CHARS "eEpP"

// Longest punctuator spelling.
#define PP_PUNCT_MAX_LEN 4

// Most blank lines printed to bring the output level with its source.
#define PP_PAD_MAX 8

// File index of no file.
#define PP_FILE_NONE UINT32_MAX

// Search-list index of a file found outside the search list.
#define PP_DIR_NONE UINT32_MAX

// Deepest nesting of includes.
#define PP_INCLUDE_DEPTH_MAX 200

// Number of a file's first line.
#define PP_LINE_FIRST 1

// Buckets in the macro table.
#define PP_MACRO_BUCKETS 1024

// FNV-1a offset basis and prime for hashing macro names.
#define PP_HASH_OFFSET 2166136261u
#define PP_HASH_PRIME  16777619u

// Name of the file that -D and -U turn into.
#define PP_CMDLINE_NAME "<command line>"

// Name a variadic macro's body gives its extra arguments.
#define PP_VA_ARGS "__VA_ARGS__"

// Forward declaration: a token's hide set is a list of these.
typedef struct Pp_HideSet Pp_HideSet;

// Forward declaration: a hide set names these.
typedef struct Pp_Macro Pp_Macro;

// Kinds of preprocessing token.
typedef enum Pp_TokenKind Pp_TokenKind;
enum Pp_TokenKind {
    PP_TOKEN_EOF,
    PP_TOKEN_IDENT,
    PP_TOKEN_NUMBER,
    PP_TOKEN_CHAR,
    PP_TOKEN_STRING,
    PP_TOKEN_HEADER_NAME,
    PP_TOKEN_PUNCT,
    PP_TOKEN_OTHER,        // a character no other kind takes
    PP_TOKEN_SPACE,        // whitespace or a comment
    PP_TOKEN_NEWLINE,
    PP_TOKEN_OPEN_COMMENT, // a comment with no end
    PP_TOKEN_PLACEMARKER,  // an empty argument next to ##
    PP_TOKEN_KIND_COUNT
};

// What stood before a token.
typedef enum Pp_Flags Pp_Flags;
enum Pp_Flags {
    PP_FLAG_NONE  = 0,
    PP_FLAG_BOL   = 1 << 0, // first token on its line
    PP_FLAG_SPACE = 1 << 1  // whitespace before it
};

// Kinds of macro.
typedef enum Pp_MacroKind Pp_MacroKind;
enum Pp_MacroKind {
    PP_MACRO_OBJECT,
    PP_MACRO_FUNCTION,
    PP_MACRO_KIND_COUNT
};

// Which directive an include came from.
typedef enum Pp_Include Pp_Include;
enum Pp_Include {
    PP_INCLUDE_PLAIN,
    PP_INCLUDE_NEXT
};

// Flag a line marker carries.
typedef enum Pp_Move Pp_Move;
enum Pp_Move {
    PP_MOVE_NONE   = 0,
    PP_MOVE_ENTER  = 1, // entering a file
    PP_MOVE_RETURN = 2  // returning to a file
};

// Whether written text carries line markers.
typedef enum Pp_Markers Pp_Markers;
enum Pp_Markers {
    PP_MARKERS_OMIT,
    PP_MARKERS_EMIT
};

// Options controlling a preprocessor run.
typedef struct Pp_Options Pp_Options;
struct Pp_Options {
    const char *const *po_dirs;    // -I directories, in the order given
    size_t             po_ndirs;
    const char        *po_sysdir;  // system include directory
    const char        *po_cmdline; // -D and -U as directives
};

// One preprocessing token.
typedef struct Pp_Token Pp_Token;
struct Pp_Token {
    Pp_TokenKind pt_kind;
    const char  *pt_text; // spelling without a NUL
    size_t       pt_len;
    Pp_Flags     pt_flags;
    uint32_t     pt_file; // index into the opened-file list
    Ast_Line     pt_line;
    Pp_HideSet  *pt_hide; // macros this token came out of
    Pp_Token    *pt_next; // next token of an expansion
};

// One macro in a hide set.
struct Pp_HideSet {
    const Pp_Macro *ph_macro;
    Pp_HideSet     *ph_next;
};

// One macro definition.
struct Pp_Macro {
    char            *ma_name;
    Pp_MacroKind     ma_kind;
    const Pp_Token **ma_params;   // names, then ... if variadic
    size_t           ma_nparams;
    bool             ma_variadic;
    const Pp_Token  *ma_body;     // replacement list, inside its file's tokens
    size_t           ma_nbody;
    Pp_Macro        *ma_next;     // next macro in its bucket
};

// One argument of a macro call.
typedef struct Pp_Arg Pp_Arg;
struct Pp_Arg {
    Pp_Token *pa_raw;      // tokens as written
    Pp_Token *pa_full;     // tokens fully expanded
    bool      pa_expanded; // pa_full is set
};

// One source file, read and tokenized.
typedef struct Pp_File Pp_File;
struct Pp_File {
    char     *pf_path;
    uint32_t  pf_index;   // position in the opened-file list
    uint32_t  pf_dir;     // search-list index it was found under
    bool      pf_system;  // found in the system include directory
    char     *pf_text;    // text after the pre-pass
    size_t    pf_len;
    Pp_Token *pf_tokens;  // ending in a PP_TOKEN_EOF
    size_t    pf_ntokens;
};

// A token stream over a range of a file, with expansions pushed in front.
typedef struct Pp_Reader Pp_Reader;
struct Pp_Reader {
    const Pp_File *rd_file;
    size_t         rd_pos;
    size_t         rd_end;
    Pp_Token      *rd_pending; // expansion tokens read before rd_pos
    Pp_Flags       rd_carry;   // flags of a macro that expanded to nothing
};

// Origin of a stretch of output lines.
typedef struct Pp_MapEntry Pp_MapEntry;
struct Pp_MapEntry {
    Ast_Line pm_output; // first output line of the stretch
    uint32_t pm_file;
    Ast_Line pm_source; // source line of its first line
    Pp_Move  pm_move;
};

// Where the printer has got to.
typedef struct Pp_Printer Pp_Printer;
struct Pp_Printer {
    Str_Buf        *pr_out;
    Ast_Line        pr_line;   // output line being printed
    uint32_t        pr_file;   // file the output line comes from
    Ast_Line        pr_source; // source line the output line comes from
    const Pp_Token *pr_prev;   // last token printed on the output line
    Pp_Move         pr_move;   // flag the next map entry takes
};

// Files
Pp_File *Pp_FindFile(const char *path);
Pp_File *Pp_OpenFile(const char *path, uint32_t dir);
Pp_File *Pp_OpenText(const char *path, const char *raw, size_t len, uint32_t dir);
void     Pp_ReplaceTrigraphs(Str_Buf *out, const char *text, size_t len);
void     Pp_DeleteSplices(Str_Buf *out, const char *text, size_t len);
void     Pp_Tokenize(Pp_File *file);

// Tokens
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

// Macros
bool      Pp_IsMacroName(const Pp_Token *tok);
uint32_t  Pp_HashName(const char *text, size_t len);
Pp_Macro *Pp_FindMacro(const char *text, size_t len);
bool      Pp_FindParam(const Pp_Macro *macro, const Pp_Token *tok, size_t *index);
bool      Pp_SameMacro(const Pp_Macro *macro, const Pp_Macro *def);
void      Pp_DefineMacro(const Pp_Token *name, const Pp_Macro *def);
void      Pp_UndefMacro(const Pp_Token *name);
void      Pp_PutDefine(Str_Buf *cmdline, const char *arg);
void      Pp_PutUndef(Str_Buf *cmdline, const char *name);

// Hide sets
bool        Pp_HideSetHas(const Pp_HideSet *set, const Pp_Macro *macro);
Pp_HideSet *Pp_HideSetAdd(Pp_HideSet *set, const Pp_Macro *macro);
Pp_HideSet *Pp_HideSetIntersect(const Pp_HideSet *a, const Pp_HideSet *b);
Pp_HideSet *Pp_HideSetUnion(Pp_HideSet *a, const Pp_HideSet *b);

// Expansion
const Pp_Token *Pp_PeekToken(const Pp_Reader *rd);
const Pp_Token *Pp_ReadToken(Pp_Reader *rd);
Pp_Arg         *Pp_ReadArgs(Pp_Reader *rd, const Pp_Macro *macro, const Pp_Token *name, const Pp_Token **close);
Pp_Token       *Pp_ExpandArg(Pp_Arg *arg);
Pp_Token       *Pp_Stringize(const Pp_Token *list, const Pp_Token *hash);
void            Pp_PasteTokens(Pp_Token *left, const Pp_Token *right, Ast_Line line);
Pp_Token       *Pp_Substitute(const Pp_Macro *macro, Pp_Arg *args, const Pp_Token *name);
bool            Pp_ExpandMacro(Pp_Reader *rd, const Pp_Token *tok);
Pp_Token       *Pp_ExpandAll(Pp_Reader *rd);
Pp_Token       *Pp_ExpandRange(const Pp_File *file, size_t start, size_t end);

// Include search
void      Pp_SetDirs(const Pp_Options *opts);
char     *Pp_DirName(const char *path);
char     *Pp_JoinPath(const char *dir, const char *name);
Pp_Token *Pp_HeaderFromTokens(const Pp_Token *list, Ast_Line line);
char     *Pp_FindInclude(const Pp_File *from, const Pp_Token *operand, Pp_Include kind, uint32_t *dir);

// Line map
void        Pp_AddMapEntry(Ast_Line output, uint32_t file, Ast_Line source, Pp_Move move);
const char *Pp_Locate(Ast_Line line, Ast_Line *source);
const char *Pp_LocateSource(Ast_Line line, Ast_Line *source);

// Printing
void Pp_BreakLine(Pp_Printer *pr);
void Pp_SyncLine(Pp_Printer *pr, const Pp_Token *tok);
void Pp_PrintToken(Pp_Printer *pr, const Pp_Token *tok);
void Pp_Write(FILE *out, const Str_Buf *text, Pp_Markers markers);

// Directives
bool   Pp_IsDirective(const Pp_Token *tok);
size_t Pp_SkipLine(const Pp_File *file, size_t pos);
size_t Pp_RunDirective(Pp_Printer *pr, const Pp_File *file, size_t pos);
void   Pp_RunInclude(Pp_Printer *pr, const Pp_File *from, size_t pos, Pp_Include kind);
void   Pp_RunDefine(const Pp_File *file, size_t pos);
size_t Pp_ReadParams(const Pp_File *file, size_t pos, size_t end, Pp_Macro *def);
void   Pp_CheckBody(const Pp_Macro *def, Ast_Line line);
void   Pp_RunUndef(const Pp_File *file, size_t pos);

// Running
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file);
void Pp_Run(const char *path, const Pp_Options *opts, Str_Buf *out);

#endif // PP_H
