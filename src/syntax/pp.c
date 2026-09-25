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

// The include search list.
static const char **Pp_Dirs;
static uint32_t     Pp_NumDirs;

// Search-list index of the system directory.
static uint32_t Pp_SysDir = PP_DIR_NONE;

// Includes open around the file being run.
static uint32_t Pp_Depth;

// File index of the source file named on the command line.
static uint32_t Pp_MainFile = PP_FILE_NONE;

// Every defined macro, hashed by name.
static Pp_Macro *Pp_Macros[PP_MACRO_BUCKETS];

// Return the opened file with this path.
Pp_File *Pp_FindFile(const char *path)
{
    for (uint32_t i = 0; i < Pp_NumFiles; i++) {
        if (Str_Equals(Pp_Files[i]->pf_path, path)) {
            return Pp_Files[i];
        }
    }
    return NULL;
}

// Read, pre-pass and tokenize the file at path.
Pp_File *Pp_OpenFile(const char *path, uint32_t dir)
{
    Pp_File *cached = Pp_FindFile(path);

    if (cached) {
        return cached;
    }

    size_t len = 0;
    char *raw = Fs_FileGetContents(path, &len);

    Err_Assert(raw, ERR_FILE_ACCESS, path, strerror(errno));

    Pp_File *file = Pp_OpenText(path, raw, len, dir);

    free(raw);
    return file;
}

// Pre-pass and tokenize text as the file at path.
Pp_File *Pp_OpenText(const char *path, const char *raw, size_t len, uint32_t dir)
{
    Str_Buf *plain = Str_BufNew();
    Str_Buf *text = Str_BufNew();

    Pp_ReplaceTrigraphs(plain, raw, len);
    Pp_DeleteSplices(text, Str_BufData(plain), Str_BufLen(plain));
    Str_BufPutByte(text, '\0');

    Pp_File *file = calloc(1, sizeof(*file));

    file->pf_path = Str_Clone(path);
    file->pf_index = Pp_NumFiles;
    file->pf_dir = dir;
    file->pf_system = dir != PP_DIR_NONE && dir == Pp_SysDir;
    file->pf_len = Str_BufLen(text) - 1;
    file->pf_text = Str_BufTake(text);

    Pp_Files = realloc(Pp_Files, (Pp_NumFiles + 1) * sizeof(*Pp_Files));
    Pp_Files[Pp_NumFiles++] = file;

    uint32_t reader = Pp_CurFile;

    Pp_CurFile = file->pf_index;
    Pp_Tokenize(file);
    Pp_CurFile = reader;

    Str_BufFree(plain);
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

// Return a heap copy of a token.
Pp_Token *Pp_CopyToken(const Pp_Token *tok)
{
    Pp_Token *copy = malloc(sizeof(*copy));

    *copy = *tok;
    copy->pt_next = NULL;
    return copy;
}

// True if a token is spelled text.
bool Pp_TokenEquals(const Pp_Token *tok, const char *text)
{
    size_t len = strlen(text);

    return tok->pt_len == len && memcmp(tok->pt_text, text, len) == 0;
}

// True if a file's next token is an include's operand.
bool Pp_ExpectsHeaderName(const Pp_File *file)
{
    size_t n = file->pf_ntokens;

    if (n < 2 || ! Pp_IsDirective(&file->pf_tokens[n - 2])) {
        return false;
    }

    const Pp_Token *name = &file->pf_tokens[n - 1];

    return ! (name->pt_flags & PP_FLAG_BOL) && (Pp_TokenEquals(name, "include") || Pp_TokenEquals(name, "include_next"));
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

// True if a directive's token can name a macro.
bool Pp_IsMacroName(const Pp_Token *tok)
{
    return tok->pt_kind == PP_TOKEN_IDENT && ! (tok->pt_flags & PP_FLAG_BOL);
}

// Hash a macro name.
uint32_t Pp_HashName(const char *text, size_t len)
{
    uint32_t hash = PP_HASH_OFFSET;

    for (size_t i = 0; i < len; i++) {
        hash = (hash ^ (uint8_t) text[i]) * PP_HASH_PRIME;
    }
    return hash;
}

// Return the macro text names.
Pp_Macro *Pp_FindMacro(const char *text, size_t len)
{
    Pp_Macro *macro = Pp_Macros[Pp_HashName(text, len) % PP_MACRO_BUCKETS];

    for (; macro; macro = macro->ma_next) {
        if (strncmp(macro->ma_name, text, len) == 0 && macro->ma_name[len] == '\0') {
            return macro;
        }
    }
    return NULL;
}

// True if a replacement list matches a macro's.
bool Pp_SameBody(const Pp_Macro *macro, const Pp_Token *body, size_t nbody)
{
    if (macro->ma_nbody != nbody) {
        return false;
    }
    for (size_t i = 0; i < nbody; i++) {
        const Pp_Token *old = &macro->ma_body[i];
        bool spaced = (old->pt_flags & PP_FLAG_SPACE) == (body[i].pt_flags & PP_FLAG_SPACE);

        if (old->pt_len != body[i].pt_len || memcmp(old->pt_text, body[i].pt_text, old->pt_len) != 0) {
            return false;
        }
        if (i > 0 && ! spaced) {
            return false;
        }
    }
    return true;
}

// Define the macro a token names.
void Pp_DefineMacro(const Pp_Token *name, const Pp_Token *body, size_t nbody)
{
    Pp_Macro *macro = Pp_FindMacro(name->pt_text, name->pt_len);

    if (macro) {
        if (! Pp_SameBody(macro, body, nbody)) {
            Err_WarnAt(name->pt_line, ERR_PP_MACRO_REDEFINED, (int) name->pt_len, name->pt_text);
        }
        macro->ma_body = body;
        macro->ma_nbody = nbody;
        return;
    }

    uint32_t bucket = Pp_HashName(name->pt_text, name->pt_len) % PP_MACRO_BUCKETS;

    macro = calloc(1, sizeof(*macro));
    macro->ma_name = Str_Format("%.*s", (int) name->pt_len, name->pt_text);
    macro->ma_body = body;
    macro->ma_nbody = nbody;
    macro->ma_next = Pp_Macros[bucket];
    Pp_Macros[bucket] = macro;
}

// Forget the macro a token names.
void Pp_UndefMacro(const Pp_Token *name)
{
    Pp_Macro *macro = Pp_FindMacro(name->pt_text, name->pt_len);
    Pp_Macro **link = &Pp_Macros[Pp_HashName(name->pt_text, name->pt_len) % PP_MACRO_BUCKETS];

    if (! macro) {
        return;
    }
    while (*link != macro) {
        link = &(*link)->ma_next;
    }
    *link = macro->ma_next;
}

// Append the directive -D arg stands for.
void Pp_PutDefine(Str_Buf *cmdline, const char *arg)
{
    const char *eq = strchr(arg, '=');

    if (eq) {
        Str_BufPrint(cmdline, "#define %.*s %s\n", (int) (eq - arg), arg, eq + 1);
    } else {
        Str_BufPrint(cmdline, "#define %s 1\n", arg);
    }
}

// Append the directive -U name stands for.
void Pp_PutUndef(Str_Buf *cmdline, const char *name)
{
    Str_BufPrint(cmdline, "#undef %s\n", name);
}

// True if a hide set holds a macro.
bool Pp_HideSetHas(const Pp_HideSet *set, const Pp_Macro *macro)
{
    for (; set; set = set->ph_next) {
        if (set->ph_macro == macro) {
            return true;
        }
    }
    return false;
}

// Return a hide set with one more macro.
Pp_HideSet *Pp_HideSetAdd(Pp_HideSet *set, const Pp_Macro *macro)
{
    Pp_HideSet *more = malloc(sizeof(*more));

    more->ph_macro = macro;
    more->ph_next = set;
    return more;
}

// Read the next token.
const Pp_Token *Pp_ReadToken(Pp_Reader *rd)
{
    const Pp_Token *tok = rd->rd_pending;

    if (tok) {
        rd->rd_pending = tok->pt_next;
    } else if (rd->rd_pos < rd->rd_end) {
        tok = &rd->rd_file->pf_tokens[rd->rd_pos++];
    } else {
        return NULL;
    }
    if (rd->rd_carry == PP_FLAG_NONE) {
        return tok;
    }

    Pp_Token *carried = Pp_CopyToken(tok);

    carried->pt_flags |= rd->rd_carry;
    rd->rd_carry = PP_FLAG_NONE;
    return carried;
}

// Push the expansion of a macro name back onto the reader.
bool Pp_ExpandMacro(Pp_Reader *rd, const Pp_Token *tok)
{
    if (tok->pt_kind != PP_TOKEN_IDENT) {
        return false;
    }

    Pp_Macro *macro = Pp_FindMacro(tok->pt_text, tok->pt_len);

    if (! macro || Pp_HideSetHas(tok->pt_hide, macro)) {
        return false;
    }

    Pp_Token *head = rd->rd_pending;
    Pp_HideSet *hide = Pp_HideSetAdd(tok->pt_hide, macro);

    for (size_t i = macro->ma_nbody; i > 0; i--) {
        Pp_Token *copy = Pp_CopyToken(&macro->ma_body[i - 1]);

        copy->pt_flags &= PP_FLAG_SPACE;
        copy->pt_file = tok->pt_file;
        copy->pt_line = tok->pt_line;
        copy->pt_hide = hide;
        copy->pt_next = head;
        head = copy;
    }
    if (macro->ma_nbody == 0) {
        rd->rd_carry |= tok->pt_flags;
    } else {
        head->pt_flags = tok->pt_flags;
    }
    rd->rd_pending = head;
    return true;
}

// Expand a range of a file's tokens into a list.
Pp_Token *Pp_ExpandRange(const Pp_File *file, size_t start, size_t end)
{
    Pp_Token *head = NULL;
    Pp_Token **tail = &head;
    const Pp_Token *tok = NULL;
    Pp_Reader rd = {
        .rd_file    = file,
        .rd_pos     = start,
        .rd_end     = end,
        .rd_pending = NULL,
        .rd_carry   = PP_FLAG_NONE
    };

    while ((tok = Pp_ReadToken(&rd)) != NULL) {
        if (! Pp_ExpandMacro(&rd, tok)) {
            *tail = Pp_CopyToken(tok);
            tail = &(*tail)->pt_next;
        }
    }
    return head;
}

// Build the include search list.
void Pp_SetDirs(const Pp_Options *opts)
{
    Pp_Dirs = malloc((opts->po_ndirs + 1) * sizeof(*Pp_Dirs));
    for (size_t i = 0; i < opts->po_ndirs; i++) {
        Pp_Dirs[Pp_NumDirs++] = Str_Clone(opts->po_dirs[i]);
    }
    if (opts->po_sysdir) {
        Pp_SysDir = Pp_NumDirs;
        Pp_Dirs[Pp_NumDirs++] = Str_Clone(opts->po_sysdir);
    }
}

// Return the directory part of a path.
char *Pp_DirName(const char *path)
{
    const char *slash = strrchr(path, '/');

    if (! slash) {
        return Str_Clone("");
    }
    return Str_Slice(path, 0, slash == path ? 1 : (size_t) (slash - path));
}

// Join a directory and a file name.
char *Pp_JoinPath(const char *dir, const char *name)
{
    size_t len = strlen(dir);

    if (name[0] == '/' || len == 0) {
        return Str_Clone(name);
    }
    if (dir[len - 1] == '/') {
        return Str_Format("%s%s", dir, name);
    }
    return Str_Format("%s/%s", dir, name);
}

// Build an include's operand from expanded tokens.
Pp_Token *Pp_HeaderFromTokens(const Pp_Token *list, Ast_Line line)
{
    bool quoted = list && list->pt_kind == PP_TOKEN_STRING && list->pt_text[0] == '"';
    bool angled = list && Pp_TokenEquals(list, "<");

    Err_AssertAt(line, quoted || angled, ERR_PP_INCLUDE_MALFORMED);

    Pp_Token *operand = Pp_CopyToken(list);

    operand->pt_kind = PP_TOKEN_HEADER_NAME;
    if (quoted) {
        return operand;
    }

    Str_Buf *name = Str_BufNew();
    const Pp_Token *tok = list->pt_next;

    Str_BufPutByte(name, '<');
    for (; tok && ! Pp_TokenEquals(tok, ">"); tok = tok->pt_next) {
        if (tok != list->pt_next && (tok->pt_flags & PP_FLAG_SPACE)) {
            Str_BufPutByte(name, ' ');
        }
        Str_BufPutBytes(name, tok->pt_text, tok->pt_len);
    }
    Err_AssertAt(line, tok, ERR_PP_INCLUDE_MALFORMED);
    Str_BufPutByte(name, '>');
    operand->pt_len = Str_BufLen(name);
    operand->pt_text = Str_BufTake(name);
    return operand;
}

// Find the file an include names.
char *Pp_FindInclude(const Pp_File *from, const Pp_Token *operand, Pp_Include kind, uint32_t *dir)
{
    char *name = Str_Format("%.*s", (int) operand->pt_len - 2, operand->pt_text + 1);
    uint32_t first = 0;

    if (name[0] == '/' && Fs_FileExists(name)) {
        *dir = PP_DIR_NONE;
        return name;
    }
    if (kind == PP_INCLUDE_NEXT && from->pf_dir != PP_DIR_NONE) {
        first = from->pf_dir + 1;
    }
    if (kind == PP_INCLUDE_PLAIN && operand->pt_text[0] == '"') {
        char *base = Pp_DirName(from->pf_path);
        char *path = Pp_JoinPath(base, name);

        Str_Free(base);
        if (Fs_FileExists(path)) {
            Str_Free(name);
            *dir = from->pf_dir;
            return path;
        }
        Str_Free(path);
    }
    for (uint32_t i = first; i < Pp_NumDirs; i++) {
        char *path = Pp_JoinPath(Pp_Dirs[i], name);

        if (Fs_FileExists(path)) {
            Str_Free(name);
            *dir = i;
            return path;
        }
        Str_Free(path);
    }

    Err_RaiseAt(operand->pt_line, ERR_PP_INCLUDE_NOT_FOUND, name);
    return NULL;
}

// Record where a stretch of output lines comes from.
void Pp_AddMapEntry(Ast_Line output, uint32_t file, Ast_Line source, Pp_Move move)
{
    if (Pp_MapLen == 0 || Pp_Map[Pp_MapLen - 1].pm_output != output) {
        Pp_Map = realloc(Pp_Map, (Pp_MapLen + 1) * sizeof(*Pp_Map));
        Pp_MapLen++;
    }

    Pp_MapEntry *entry = &Pp_Map[Pp_MapLen - 1];

    entry->pm_output = output;
    entry->pm_file = file;
    entry->pm_source = source;
    entry->pm_move = move;
}

// Map a line of the output to its file and source line.
const char *Pp_Locate(Ast_Line line, Ast_Line *source)
{
    size_t lo = 0;
    size_t hi = Pp_MapLen;

    if (Pp_MapLen == 0 || line < Pp_Map[0].pm_output) {
        *source = line;
        return Pp_Files[Pp_MainFile]->pf_path;
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
    Pp_AddMapEntry(pr->pr_line, pr->pr_file, pr->pr_source, pr->pr_move);
    pr->pr_move = PP_MOVE_NONE;
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
            const Pp_MapEntry *entry = &Pp_Map[next++];

            fprintf(out, "# %u \"%s\"", entry->pm_source, Pp_Files[entry->pm_file]->pf_path);
            if (entry->pm_move != PP_MOVE_NONE) {
                fprintf(out, " %d", (int) entry->pm_move);
            }
            fputc('\n', out);
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

// Return the position of the first token on the next line.
size_t Pp_SkipLine(const Pp_File *file, size_t pos)
{
    do {
        pos++;
    } while (! (file->pf_tokens[pos].pt_flags & PP_FLAG_BOL));
    return pos;
}

// Run the directive at pos.
size_t Pp_RunDirective(Pp_Printer *pr, const Pp_File *file, size_t pos)
{
    const Pp_Token *hash = &file->pf_tokens[pos];
    const Pp_Token *name = &file->pf_tokens[pos + 1];

    if (name->pt_flags & PP_FLAG_BOL) {
        return pos + 1;
    }

    if (Pp_TokenEquals(name, "include")) {
        Pp_RunInclude(pr, file, pos, PP_INCLUDE_PLAIN);
    } else if (Pp_TokenEquals(name, "include_next")) {
        Pp_RunInclude(pr, file, pos, PP_INCLUDE_NEXT);
    } else if (Pp_TokenEquals(name, "define")) {
        Pp_RunDefine(file, pos);
    } else if (Pp_TokenEquals(name, "undef")) {
        Pp_RunUndef(file, pos);
    } else {
        Err_RaiseAt(hash->pt_line, ERR_PP_DIRECTIVE_UNKNOWN, (int) name->pt_len, name->pt_text);
    }
    return Pp_SkipLine(file, pos);
}

// Run the include directive at pos.
void Pp_RunInclude(Pp_Printer *pr, const Pp_File *from, size_t pos, Pp_Include kind)
{
    uint32_t dir = PP_DIR_NONE;
    const Pp_Token *hash = &from->pf_tokens[pos];
    const Pp_Token *operand = &from->pf_tokens[pos + 2];

    if (operand->pt_kind != PP_TOKEN_HEADER_NAME) {
        operand = Pp_HeaderFromTokens(Pp_ExpandRange(from, pos + 2, Pp_SkipLine(from, pos)), hash->pt_line);
    }
    Err_AssertAt(hash->pt_line, Pp_Depth < PP_INCLUDE_DEPTH_MAX, ERR_PP_INCLUDE_TOO_DEEP, PP_INCLUDE_DEPTH_MAX);

    char *path = Pp_FindInclude(from, operand, kind, &dir);
    Pp_File *file = Pp_OpenFile(path, dir);

    Str_Free(path);
    Pp_Depth++;
    pr->pr_move = PP_MOVE_ENTER;
    Pp_RunFile(pr, file);
    pr->pr_move = PP_MOVE_RETURN;
    Pp_Depth--;
}

// Run the #define directive at pos.
void Pp_RunDefine(const Pp_File *file, size_t pos)
{
    const Pp_Token *hash = &file->pf_tokens[pos];
    const Pp_Token *name = &file->pf_tokens[pos + 2];

    Err_AssertAt(hash->pt_line, Pp_IsMacroName(name), ERR_PP_MACRO_NAME_MISSING);

    const Pp_Token *body = &file->pf_tokens[pos + 3];
    bool function = Pp_TokenEquals(body, "(") && ! (body->pt_flags & (PP_FLAG_BOL | PP_FLAG_SPACE));

    Err_AssertAt(hash->pt_line, ! function, ERR_PP_MACRO_FUNCTION_UNSUPPORTED);
    Pp_DefineMacro(name, body, Pp_SkipLine(file, pos) - (pos + 3));
}

// Run the #undef directive at pos.
void Pp_RunUndef(const Pp_File *file, size_t pos)
{
    const Pp_Token *hash = &file->pf_tokens[pos];
    const Pp_Token *name = &file->pf_tokens[pos + 2];

    Err_AssertAt(hash->pt_line, Pp_IsMacroName(name), ERR_PP_MACRO_NAME_MISSING);
    Pp_UndefMacro(name);
}

// Preprocess one file into the printer.
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file)
{
    uint32_t includer = Pp_CurFile;
    Pp_Reader rd = {
        .rd_file    = file,
        .rd_pos     = 0,
        .rd_end     = file->pf_ntokens - 1,
        .rd_pending = NULL,
        .rd_carry   = PP_FLAG_NONE
    };

    Pp_CurFile = file->pf_index;
    for (;;) {
        if (! rd.rd_pending && Pp_IsDirective(&file->pf_tokens[rd.rd_pos])) {
            rd.rd_pos = Pp_RunDirective(pr, file, rd.rd_pos);
            continue;
        }

        const Pp_Token *tok = Pp_ReadToken(&rd);

        if (! tok) {
            break;
        }
        if (! Pp_ExpandMacro(&rd, tok)) {
            Pp_PrintToken(pr, tok);
        }
    }
    Pp_CurFile = includer;
}

// Preprocess the file at path into out.
void Pp_Run(const char *path, const Pp_Options *opts, Str_Buf *out)
{
    Pp_Printer pr = {
        .pr_out    = out,
        .pr_line   = PP_LINE_FIRST,
        .pr_file   = PP_FILE_NONE,
        .pr_source = PP_LINE_FIRST,
        .pr_prev   = NULL,
        .pr_move   = PP_MOVE_NONE
    };

    Log_SetLineLocator(Pp_LocateSource);
    Pp_SetDirs(opts);
    if (opts->po_cmdline) {
        Pp_RunFile(&pr, Pp_OpenText(PP_CMDLINE_NAME, opts->po_cmdline, strlen(opts->po_cmdline), PP_DIR_NONE));
    }

    Pp_File *source = Pp_OpenFile(path, PP_DIR_NONE);

    Pp_MainFile = source->pf_index;
    Pp_RunFile(&pr, source);
    if (pr.pr_prev) {
        Pp_BreakLine(&pr);
    }
}
