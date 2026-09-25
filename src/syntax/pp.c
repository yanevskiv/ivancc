// C source file for the C preprocessor.

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util/err.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/str.h"
#include "syntax/pp.h"

// Every punctuator spelling of 6.4.6.
static const char *const Pp_Punctuators[] = {
    "[", "]", "(", ")", "{", "}", ".", "->",
    "++", "--", "&", "*", "+", "-", "~", "!",
    "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=", "^", "|", "&&", "||",
    "?", ":", ";", "...",
    "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=",
    ",", "#", "##",
    "<:", ":>", "<%", "%>", "%:", "%:%:"
};

// The files opened so far, in the order they were opened.
static Pp_File **Pp_Files;
static uint32_t  Pp_NumFiles;

// The line map, in output order.
static Pp_MapEntry *Pp_Map;
static size_t       Pp_MapLen;

// The file whose tokens are being read.
static uint32_t Pp_CurFile = PP_FILE_NONE;

// Read, pre-pass and tokenize the file at path.
Pp_File *Pp_OpenFile(const char *path)
{
    size_t len = 0;
    char *raw = Fs_FileGetContents(path, &len);

    Err_Assert(raw, ERR_FILE_ACCESS, path, strerror(errno));

    Str_Buf *plain = Str_BufNew();
    Str_Buf *text = Str_BufNew();

    Pp_ReplaceTrigraphs(plain, raw, len);
    Pp_DeleteSplices(text, Str_BufData(plain), Str_BufLen(plain));
    Str_BufPutByte(text, '\0');

    Pp_File *file = calloc(1, sizeof(*file));

    file->pf_path = Str_Clone(path);
    file->pf_index = Pp_NumFiles;
    file->pf_len = Str_BufLen(text) - 1;
    file->pf_text = Str_BufTake(text);

    Pp_Files = realloc(Pp_Files, (Pp_NumFiles + 1) * sizeof(*Pp_Files));
    Pp_Files[Pp_NumFiles++] = file;

    Pp_CurFile = file->pf_index;
    Pp_Tokenize(file);

    Str_BufFree(plain);
    free(raw);
    return file;
}

// Replace every trigraph with the character it stands for.
void Pp_ReplaceTrigraphs(Str_Buf *out, const char *text, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        const char *key = NULL;

        if (text[i] == '?' && i + 2 < len && text[i + 1] == '?' && text[i + 2] != '\0') {
            key = strchr(PP_TRIGRAPH_KEYS, text[i + 2]);
        }
        if (key) {
            Str_BufPutByte(out, PP_TRIGRAPH_VALUES[key - PP_TRIGRAPH_KEYS]);
            i += 2;
        } else {
            Str_BufPutByte(out, text[i]);
        }
    }
}

// Delete every backslash-newline.
void Pp_DeleteSplices(Str_Buf *out, const char *text, size_t len)
{
    size_t owed = 0;

    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\\' && i + 1 < len && text[i + 1] == '\n') {
            owed++;
            i++;
            continue;
        }
        Str_BufPutByte(out, text[i]);
        if (text[i] == '\n') {
            for (; owed > 0; owed--) {
                Str_BufPutByte(out, '\n');
            }
        }
    }
    for (; owed > 0; owed--) {
        Str_BufPutByte(out, '\n');
    }
}

// True if a token is spelled text.
bool Pp_TokenEquals(const Pp_Token *tok, const char *text)
{
    size_t len = strlen(text);

    return tok->pt_len == len && memcmp(tok->pt_text, text, len) == 0;
}

// True if some punctuator starts with the len bytes of text.
bool Pp_IsPunctPrefix(const char *text, size_t len)
{
    size_t count = sizeof(Pp_Punctuators) / sizeof(Pp_Punctuators[0]);

    for (size_t i = 0; i < count; i++) {
        if (strlen(Pp_Punctuators[i]) >= len && memcmp(Pp_Punctuators[i], text, len) == 0) {
            return true;
        }
    }
    return false;
}

// True if two tokens printed together would lex as something else.
bool Pp_NeedsSpace(const Pp_Token *prev, const Pp_Token *next)
{
    char last = prev->pt_text[prev->pt_len - 1];
    char first = next->pt_text[0];

    if (prev->pt_kind == PP_TOKEN_OTHER || next->pt_kind == PP_TOKEN_OTHER) {
        return true;
    }

    switch (prev->pt_kind) {
        case PP_TOKEN_IDENT: {
            return next->pt_kind != PP_TOKEN_PUNCT;
        } break;
        case PP_TOKEN_NUMBER: {
            if (next->pt_kind == PP_TOKEN_IDENT || next->pt_kind == PP_TOKEN_NUMBER || first == '.') {
                return true;
            }
            return (first == '+' || first == '-') && strchr(PP_EXPONENT_CHARS, last) != NULL;
        } break;
        case PP_TOKEN_PUNCT: {
            char joined[PP_PUNCT_MAX_LEN + 1];

            if (next->pt_kind == PP_TOKEN_NUMBER) {
                return Pp_TokenEquals(prev, ".");
            }
            if (next->pt_kind != PP_TOKEN_PUNCT) {
                return false;
            }
            if (last == '/' && (first == '/' || first == '*')) {
                return true;
            }
            memcpy(joined, prev->pt_text, prev->pt_len);
            joined[prev->pt_len] = first;
            return Pp_IsPunctPrefix(joined, prev->pt_len + 1);
        } break;
        default: {
            return false;
        } break;
    }
}

// Record where a stretch of output lines comes from.
void Pp_AddMapEntry(Ast_Line output, uint32_t file, Ast_Line source)
{
    if (Pp_MapLen == 0 || Pp_Map[Pp_MapLen - 1].pm_output != output) {
        Pp_Map = realloc(Pp_Map, (Pp_MapLen + 1) * sizeof(*Pp_Map));
        Pp_MapLen++;
    }

    Pp_MapEntry *entry = &Pp_Map[Pp_MapLen - 1];

    entry->pm_output = output;
    entry->pm_file = file;
    entry->pm_source = source;
}

// Map a line of the output to its file and source line.
const char *Pp_Locate(Ast_Line line, Ast_Line *source)
{
    size_t lo = 0;
    size_t hi = Pp_MapLen;

    if (Pp_MapLen == 0 || line < Pp_Map[0].pm_output) {
        *source = line;
        return Pp_Files[0]->pf_path;
    }
    while (hi - lo > 1) {
        size_t mid = lo + (hi - lo) / 2;

        if (Pp_Map[mid].pm_output <= line) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    const Pp_MapEntry *entry = &Pp_Map[lo];

    *source = entry->pm_source + (line - entry->pm_output);
    return Pp_Files[entry->pm_file]->pf_path;
}

// Map a line of the file being read to its path.
const char *Pp_LocateSource(Ast_Line line, Ast_Line *source)
{
    *source = line;
    return Pp_Files[Pp_CurFile]->pf_path;
}

// End the output line.
void Pp_BreakLine(Pp_Printer *pr)
{
    Str_BufPutByte(pr->pr_out, '\n');
    pr->pr_line++;
    pr->pr_prev = NULL;
}

// Move the printer to the line a token starts on.
void Pp_SyncLine(Pp_Printer *pr, const Pp_Token *tok)
{
    bool ahead = tok->pt_file == pr->pr_file && tok->pt_line >= pr->pr_source;

    if (ahead && tok->pt_line - pr->pr_source <= PP_PAD_MAX) {
        for (; pr->pr_source < tok->pt_line; pr->pr_source++) {
            Pp_BreakLine(pr);
        }
        return;
    }

    if (pr->pr_prev) {
        Pp_BreakLine(pr);
    }
    pr->pr_file = tok->pt_file;
    pr->pr_source = tok->pt_line;
    Pp_AddMapEntry(pr->pr_line, pr->pr_file, pr->pr_source);
}

// Print one token.
void Pp_PrintToken(Pp_Printer *pr, const Pp_Token *tok)
{
    if (tok->pt_flags & PP_FLAG_BOL) {
        Pp_SyncLine(pr, tok);
    }
    if (pr->pr_prev && ((tok->pt_flags & PP_FLAG_SPACE) || Pp_NeedsSpace(pr->pr_prev, tok))) {
        Str_BufPutByte(pr->pr_out, ' ');
    }
    Str_BufPutBytes(pr->pr_out, tok->pt_text, tok->pt_len);
    pr->pr_prev = tok;
}

// Write preprocessed text.
void Pp_Write(FILE *out, const Str_Buf *text, Pp_Markers markers)
{
    size_t next = 0;
    size_t len = Str_BufLen(text);
    Ast_Line line = PP_LINE_FIRST;
    const char *data = Str_BufData(text);

    for (size_t pos = 0; pos < len; line++) {
        const char *end = memchr(data + pos, '\n', len - pos);
        size_t n = end ? (size_t) (end - (data + pos)) + 1 : len - pos;

        if (markers == PP_MARKERS_EMIT && next < Pp_MapLen && Pp_Map[next].pm_output == line) {
            fprintf(out, "# %u \"%s\"\n", Pp_Map[next].pm_source, Pp_Files[Pp_Map[next].pm_file]->pf_path);
            next++;
        }
        fwrite(data + pos, 1, n, out);
        pos += n;
    }
}

// True if a token opens a directive.
bool Pp_IsDirective(const Pp_Token *tok)
{
    return (tok->pt_flags & PP_FLAG_BOL) && (Pp_TokenEquals(tok, "#") || Pp_TokenEquals(tok, "%:"));
}

// Run the directive at pos.
size_t Pp_RunDirective(const Pp_File *file, size_t pos)
{
    const Pp_Token *hash = &file->pf_tokens[pos];
    const Pp_Token *name = &file->pf_tokens[pos + 1];

    Err_AssertAt(hash->pt_line, name->pt_flags & PP_FLAG_BOL, ERR_PP_DIRECTIVE_UNKNOWN, (int) name->pt_len, name->pt_text);
    return pos + 1;
}

// Preprocess one file into the printer.
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file)
{
    size_t pos = 0;

    Pp_CurFile = file->pf_index;
    while (file->pf_tokens[pos].pt_kind != PP_TOKEN_EOF) {
        if (Pp_IsDirective(&file->pf_tokens[pos])) {
            pos = Pp_RunDirective(file, pos);
        } else {
            Pp_PrintToken(pr, &file->pf_tokens[pos]);
            pos++;
        }
    }
}

// Preprocess the file at path into out.
void Pp_Run(const char *path, Str_Buf *out)
{
    Pp_Printer pr = {
        .pr_out    = out,
        .pr_line   = PP_LINE_FIRST,
        .pr_file   = PP_FILE_NONE,
        .pr_source = PP_LINE_FIRST,
        .pr_prev   = NULL
    };

    Log_SetLineLocator(Pp_LocateSource);
    Pp_RunFile(&pr, Pp_OpenFile(path));
    if (pr.pr_prev) {
        Pp_BreakLine(&pr);
    }
}
