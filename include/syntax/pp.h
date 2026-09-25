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

// Number of a file's first line.
#define PP_LINE_FIRST 1

// Kinds of preprocessing token.
typedef enum Pp_TokenKind Pp_TokenKind;
enum Pp_TokenKind {
    PP_TOKEN_EOF,
    PP_TOKEN_IDENT,
    PP_TOKEN_NUMBER,
    PP_TOKEN_CHAR,
    PP_TOKEN_STRING,
    PP_TOKEN_PUNCT,
    PP_TOKEN_OTHER,      // a character no other kind takes
    PP_TOKEN_SPACE,      // whitespace or a comment
    PP_TOKEN_NEWLINE,
    PP_TOKEN_KIND_COUNT
};

// What stood before a token.
typedef enum Pp_Flags Pp_Flags;
enum Pp_Flags {
    PP_FLAG_NONE  = 0,
    PP_FLAG_BOL   = 1 << 0, // first token on its line
    PP_FLAG_SPACE = 1 << 1  // whitespace before it
};

// Whether written text carries line markers.
typedef enum Pp_Markers Pp_Markers;
enum Pp_Markers {
    PP_MARKERS_OMIT,
    PP_MARKERS_EMIT
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
};

// One source file, read and tokenized.
typedef struct Pp_File Pp_File;
struct Pp_File {
    char     *pf_path;
    uint32_t  pf_index;   // position in the opened-file list
    char     *pf_text;    // text after the pre-pass
    size_t    pf_len;
    Pp_Token *pf_tokens;  // ending in a PP_TOKEN_EOF
    size_t    pf_ntokens;
};

// Origin of a stretch of output lines.
typedef struct Pp_MapEntry Pp_MapEntry;
struct Pp_MapEntry {
    Ast_Line pm_output; // first output line of the stretch
    uint32_t pm_file;
    Ast_Line pm_source; // source line of its first line
};

// Where the printer has got to.
typedef struct Pp_Printer Pp_Printer;
struct Pp_Printer {
    Str_Buf        *pr_out;
    Ast_Line        pr_line;   // output line being printed
    uint32_t        pr_file;   // file the output line comes from
    Ast_Line        pr_source; // source line the output line comes from
    const Pp_Token *pr_prev;   // last token printed on the output line
};

// Files
Pp_File *Pp_OpenFile(const char *path);
void     Pp_ReplaceTrigraphs(Str_Buf *out, const char *text, size_t len);
void     Pp_DeleteSplices(Str_Buf *out, const char *text, size_t len);
void     Pp_Tokenize(Pp_File *file);

// Tokens
bool Pp_TokenEquals(const Pp_Token *tok, const char *text);
bool Pp_IsPunctPrefix(const char *text, size_t len);
bool Pp_NeedsSpace(const Pp_Token *prev, const Pp_Token *next);

// Line map
void        Pp_AddMapEntry(Ast_Line output, uint32_t file, Ast_Line source);
const char *Pp_Locate(Ast_Line line, Ast_Line *source);
const char *Pp_LocateSource(Ast_Line line, Ast_Line *source);

// Printing
void Pp_BreakLine(Pp_Printer *pr);
void Pp_SyncLine(Pp_Printer *pr, const Pp_Token *tok);
void Pp_PrintToken(Pp_Printer *pr, const Pp_Token *tok);
void Pp_Write(FILE *out, const Str_Buf *text, Pp_Markers markers);

// Directives
bool   Pp_IsDirective(const Pp_Token *tok);
size_t Pp_RunDirective(const Pp_File *file, size_t pos);

// Running
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file);
void Pp_Run(const char *path, Str_Buf *out);

#endif // PP_H
