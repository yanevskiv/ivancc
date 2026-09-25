// C source file for the C preprocessor.

// Take every include from the module's header.
#include "lang/pp.h"

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

// Spelling of each binary operator of #if.
static const char *const Pp_OpText[PP_OP_COUNT] = {
    [PP_OP_MUL]     = "*",
    [PP_OP_DIV]     = "/",
    [PP_OP_MOD]     = "%",
    [PP_OP_ADD]     = "+",
    [PP_OP_SUB]     = "-",
    [PP_OP_SHL]     = "<<",
    [PP_OP_SHR]     = ">>",
    [PP_OP_LT]      = "<",
    [PP_OP_GT]      = ">",
    [PP_OP_LE]      = "<=",
    [PP_OP_GE]      = ">=",
    [PP_OP_EQ]      = "==",
    [PP_OP_NE]      = "!=",
    [PP_OP_BIT_AND] = "&",
    [PP_OP_BIT_XOR] = "^",
    [PP_OP_BIT_OR]  = "|",
    [PP_OP_AND]     = "&&",
    [PP_OP_OR]      = "||"
};

// Precedence of each binary operator of #if.
static const Pp_Prec Pp_OpPrec[PP_OP_COUNT] = {
    [PP_OP_MUL]     = PP_PREC_MULTIPLICATIVE,
    [PP_OP_DIV]     = PP_PREC_MULTIPLICATIVE,
    [PP_OP_MOD]     = PP_PREC_MULTIPLICATIVE,
    [PP_OP_ADD]     = PP_PREC_ADDITIVE,
    [PP_OP_SUB]     = PP_PREC_ADDITIVE,
    [PP_OP_SHL]     = PP_PREC_SHIFT,
    [PP_OP_SHR]     = PP_PREC_SHIFT,
    [PP_OP_LT]      = PP_PREC_RELATIONAL,
    [PP_OP_GT]      = PP_PREC_RELATIONAL,
    [PP_OP_LE]      = PP_PREC_RELATIONAL,
    [PP_OP_GE]      = PP_PREC_RELATIONAL,
    [PP_OP_EQ]      = PP_PREC_EQUALITY,
    [PP_OP_NE]      = PP_PREC_EQUALITY,
    [PP_OP_BIT_AND] = PP_PREC_BIT_AND,
    [PP_OP_BIT_XOR] = PP_PREC_BIT_XOR,
    [PP_OP_BIT_OR]  = PP_PREC_BIT_OR,
    [PP_OP_AND]     = PP_PREC_AND,
    [PP_OP_OR]      = PP_PREC_OR
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

// The conditionals open in the file being run, innermost first.
static Pp_Cond *Pp_Conds;

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

// Return a heap copy of a token list.
Pp_Token *Pp_CopyList(const Pp_Token *list)
{
    Pp_Token *head = NULL;
    Pp_Token **tail = &head;

    for (; list; list = list->pt_next) {
        *tail = Pp_CopyToken(list);
        tail = &(*tail)->pt_next;
    }
    return head;
}

// True if a token is spelled text.
bool Pp_TokenEquals(const Pp_Token *tok, const char *text)
{
    size_t len = strlen(text);

    return tok->pt_len == len && memcmp(tok->pt_text, text, len) == 0;
}

// True if two tokens are spelled alike.
bool Pp_SameSpelling(const Pp_Token *a, const Pp_Token *b)
{
    return a->pt_len == b->pt_len && memcmp(a->pt_text, b->pt_text, a->pt_len) == 0;
}

// True if a token is the # punctuator.
bool Pp_IsHash(const Pp_Token *tok)
{
    return tok->pt_kind == PP_TOKEN_PUNCT && (Pp_TokenEquals(tok, "#") || Pp_TokenEquals(tok, "%:"));
}

// True if a token is the ## punctuator.
bool Pp_IsHashHash(const Pp_Token *tok)
{
    return tok->pt_kind == PP_TOKEN_PUNCT && (Pp_TokenEquals(tok, "##") || Pp_TokenEquals(tok, "%:%:"));
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

// Find the parameter a token names.
bool Pp_FindParam(const Pp_Macro *macro, const Pp_Token *tok, size_t *index)
{
    if (tok->pt_kind != PP_TOKEN_IDENT) {
        return false;
    }
    for (size_t i = 0; i < macro->ma_nparams; i++) {
        bool rest = macro->ma_variadic && i + 1 == macro->ma_nparams;

        if (rest ? Pp_TokenEquals(tok, PP_VA_ARGS) : Pp_SameSpelling(tok, macro->ma_params[i])) {
            *index = i;
            return true;
        }
    }
    return false;
}

// True if a definition matches a macro's.
bool Pp_SameMacro(const Pp_Macro *macro, const Pp_Macro *def)
{
    bool shape = macro->ma_kind == def->ma_kind && macro->ma_variadic == def->ma_variadic;

    if (! shape || macro->ma_nparams != def->ma_nparams || macro->ma_nbody != def->ma_nbody) {
        return false;
    }
    for (size_t i = 0; i < def->ma_nparams; i++) {
        if (! Pp_SameSpelling(macro->ma_params[i], def->ma_params[i])) {
            return false;
        }
    }
    for (size_t i = 0; i < def->ma_nbody; i++) {
        const Pp_Token *old = &macro->ma_body[i];
        const Pp_Token *fresh = &def->ma_body[i];
        bool spaced = (old->pt_flags & PP_FLAG_SPACE) == (fresh->pt_flags & PP_FLAG_SPACE);

        if (! Pp_SameSpelling(old, fresh) || (i > 0 && ! spaced)) {
            return false;
        }
    }
    return true;
}

// Define the macro a token names.
void Pp_DefineMacro(const Pp_Token *name, const Pp_Macro *def)
{
    uint32_t bucket = Pp_HashName(name->pt_text, name->pt_len) % PP_MACRO_BUCKETS;
    Pp_Macro *macro = Pp_FindMacro(name->pt_text, name->pt_len);

    if (macro && ! Pp_SameMacro(macro, def)) {
        Err_WarnAt(name->pt_line, ERR_PP_MACRO_REDEFINED, (int) name->pt_len, name->pt_text);
    }
    if (! macro) {
        macro = calloc(1, sizeof(*macro));
        macro->ma_name = Str_Format("%.*s", (int) name->pt_len, name->pt_text);
        macro->ma_next = Pp_Macros[bucket];
        Pp_Macros[bucket] = macro;
    }
    macro->ma_kind = def->ma_kind;
    macro->ma_params = def->ma_params;
    macro->ma_nparams = def->ma_nparams;
    macro->ma_variadic = def->ma_variadic;
    macro->ma_body = def->ma_body;
    macro->ma_nbody = def->ma_nbody;
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

// Return the macros two hide sets share.
Pp_HideSet *Pp_HideSetIntersect(const Pp_HideSet *a, const Pp_HideSet *b)
{
    Pp_HideSet *set = NULL;

    for (; a; a = a->ph_next) {
        if (Pp_HideSetHas(b, a->ph_macro)) {
            set = Pp_HideSetAdd(set, a->ph_macro);
        }
    }
    return set;
}

// Return the macros either hide set holds.
Pp_HideSet *Pp_HideSetUnion(Pp_HideSet *a, const Pp_HideSet *b)
{
    Pp_HideSet *set = a;

    for (; b; b = b->ph_next) {
        if (! Pp_HideSetHas(a, b->ph_macro)) {
            set = Pp_HideSetAdd(set, b->ph_macro);
        }
    }
    return set;
}

// Return the next token without reading it.
const Pp_Token *Pp_PeekToken(const Pp_Reader *rd)
{
    if (rd->rd_pending) {
        return rd->rd_pending;
    }
    if (rd->rd_pos < rd->rd_end) {
        return &rd->rd_file->pf_tokens[rd->rd_pos];
    }
    return NULL;
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

// Read a call's arguments up to its closing parenthesis.
Pp_Arg *Pp_ReadArgs(Pp_Reader *rd, const Pp_Macro *macro, const Pp_Token *name, const Pp_Token **close)
{
    size_t n = 0;
    size_t given = 0;
    size_t depth = 0;
    size_t required = macro->ma_variadic ? macro->ma_nparams - 1 : macro->ma_nparams;
    Pp_Arg *args = calloc(macro->ma_nparams + 1, sizeof(*args));
    Pp_Token **tail = &args[0].pa_raw;

    for (;;) {
        const Pp_Token *tok = Pp_ReadToken(rd);

        Err_AssertAt(name->pt_line, tok, ERR_PP_MACRO_UNTERMINATED, macro->ma_name);
        if (depth == 0 && Pp_TokenEquals(tok, ")")) {
            *close = tok;
            break;
        }
        if (depth == 0 && Pp_TokenEquals(tok, ",") && ! (macro->ma_variadic && n + 1 == macro->ma_nparams)) {
            n++;
            tail = n <= macro->ma_nparams ? &args[n].pa_raw : NULL;
            continue;
        }
        if (Pp_TokenEquals(tok, "(")) {
            depth++;
        } else if (Pp_TokenEquals(tok, ")")) {
            depth--;
        }
        if (tail) {
            *tail = Pp_CopyToken(tok);
            tail = &(*tail)->pt_next;
        }
    }

    given = n + 1;
    if (macro->ma_nparams == 0 && n == 0 && ! args[0].pa_raw) {
        given = 0;
    }
    Err_AssertAt(name->pt_line, macro->ma_variadic ? given >= required : given == required, ERR_PP_MACRO_ARGS_COUNT, macro->ma_name, required, given);
    return args;
}

// Return an argument fully expanded.
Pp_Token *Pp_ExpandArg(Pp_Arg *arg)
{
    Pp_Reader rd = {
        .rd_file    = NULL,
        .rd_pos     = 0,
        .rd_end     = 0,
        .rd_pending = arg->pa_raw,
        .rd_carry   = PP_FLAG_NONE
    };

    if (! arg->pa_expanded) {
        arg->pa_full = Pp_ExpandAll(&rd);
        arg->pa_expanded = true;
    }
    return arg->pa_full;
}

// Spell an argument as a string literal.
Pp_Token *Pp_Stringize(const Pp_Token *list, const Pp_Token *hash)
{
    Str_Buf *text = Str_BufNew();
    Pp_Token *str = Pp_CopyToken(hash);

    Str_BufPutByte(text, '"');
    for (const Pp_Token *tok = list; tok; tok = tok->pt_next) {
        bool quoted = tok->pt_kind == PP_TOKEN_STRING || tok->pt_kind == PP_TOKEN_CHAR;

        if (tok != list && (tok->pt_flags & (PP_FLAG_BOL | PP_FLAG_SPACE))) {
            Str_BufPutByte(text, ' ');
        }
        for (size_t i = 0; i < tok->pt_len; i++) {
            if (quoted && (tok->pt_text[i] == '"' || tok->pt_text[i] == '\\')) {
                Str_BufPutByte(text, '\\');
            }
            Str_BufPutByte(text, tok->pt_text[i]);
        }
    }
    Str_BufPutByte(text, '"');

    str->pt_kind = PP_TOKEN_STRING;
    str->pt_len = Str_BufLen(text);
    str->pt_text = Str_BufTake(text);
    return str;
}

// Paste a token onto the end of another.
void Pp_PasteTokens(Pp_Token *left, const Pp_Token *right, Ast_Line line)
{
    if (right->pt_kind == PP_TOKEN_PLACEMARKER) {
        return;
    }
    if (left->pt_kind == PP_TOKEN_PLACEMARKER) {
        left->pt_kind = right->pt_kind;
        left->pt_text = right->pt_text;
        left->pt_len = right->pt_len;
        left->pt_hide = right->pt_hide;
        return;
    }

    Pp_TokenKind kind = PP_TOKEN_EOF;
    size_t len = left->pt_len + right->pt_len;
    char *text = Str_Format("%.*s%.*s", (int) left->pt_len, left->pt_text, (int) right->pt_len, right->pt_text);

    Err_AssertAt(line, Pp_LexOne(text, len, &kind), ERR_PP_PASTE_INVALID, (int) left->pt_len, left->pt_text, (int) right->pt_len, right->pt_text);
    left->pt_kind = kind;
    left->pt_text = text;
    left->pt_len = len;
    left->pt_hide = Pp_HideSetIntersect(left->pt_hide, right->pt_hide);
}

// Build a macro's replacement list with its arguments in place.
Pp_Token *Pp_Substitute(const Pp_Macro *macro, Pp_Arg *args, const Pp_Token *name)
{
    bool paste = false;
    Pp_Token *head = NULL;
    Pp_Token *last = NULL;
    Pp_Token **link = &head;

    // Phase: substitute
    for (size_t i = 0; i < macro->ma_nbody; i++) {
        size_t param = 0;
        Pp_Token *list = NULL;
        const Pp_Token *tok = &macro->ma_body[i];
        const Pp_Token *next = i + 1 < macro->ma_nbody ? &macro->ma_body[i + 1] : NULL;

        if (Pp_IsHashHash(tok)) {
            paste = true;
            continue;
        }
        if (macro->ma_kind == PP_MACRO_FUNCTION && Pp_IsHash(tok) && Pp_FindParam(macro, next, &param)) {
            list = Pp_Stringize(args[param].pa_raw, tok);
            i++;
        } else if (Pp_FindParam(macro, tok, &param)) {
            bool raw = paste || (next && Pp_IsHashHash(next));

            list = Pp_CopyList(raw ? args[param].pa_raw : Pp_ExpandArg(&args[param]));
            if (raw && ! list) {
                list = Pp_CopyToken(tok);
                list->pt_kind = PP_TOKEN_PLACEMARKER;
                list->pt_len = 0;
            }
            if (list) {
                list->pt_flags = tok->pt_flags;
            }
        } else {
            list = Pp_CopyToken(tok);
        }

        if (paste) {
            Pp_PasteTokens(last, list, name->pt_line);
            list = list->pt_next;
        }
        paste = false;
        if (! list) {
            continue;
        }
        if (last) {
            last->pt_next = list;
        } else {
            head = list;
        }
        last = list;
        while (last->pt_next) {
            last = last->pt_next;
        }
    }

    // Phase: drop placemarkers
    while (*link) {
        if ((*link)->pt_kind == PP_TOKEN_PLACEMARKER) {
            *link = (*link)->pt_next;
        } else {
            link = &(*link)->pt_next;
        }
    }
    return head;
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

    Pp_Arg *args = NULL;
    Pp_HideSet *hide = tok->pt_hide;

    if (macro->ma_kind == PP_MACRO_FUNCTION) {
        const Pp_Token *open = Pp_PeekToken(rd);
        const Pp_Token *close = NULL;

        if (! open || ! Pp_TokenEquals(open, "(")) {
            return false;
        }
        Pp_ReadToken(rd);
        args = Pp_ReadArgs(rd, macro, tok, &close);
        hide = Pp_HideSetIntersect(hide, close->pt_hide);
    }
    hide = Pp_HideSetAdd(hide, macro);

    Pp_Token *last = NULL;
    Pp_Token *head = Pp_Substitute(macro, args, tok);

    for (Pp_Token *copy = head; copy; copy = copy->pt_next) {
        copy->pt_flags = (copy->pt_flags & (PP_FLAG_BOL | PP_FLAG_SPACE)) ? PP_FLAG_SPACE : PP_FLAG_NONE;
        copy->pt_file = tok->pt_file;
        copy->pt_line = tok->pt_line;
        copy->pt_hide = Pp_HideSetUnion(hide, copy->pt_hide);
        last = copy;
    }
    if (! head) {
        rd->rd_carry |= tok->pt_flags;
        return true;
    }
    head->pt_flags = tok->pt_flags;
    last->pt_next = rd->rd_pending;
    rd->rd_pending = head;
    return true;
}

// Expand every token a reader holds into a list.
Pp_Token *Pp_ExpandAll(Pp_Reader *rd)
{
    Pp_Token *head = NULL;
    Pp_Token **tail = &head;
    const Pp_Token *tok = NULL;

    while ((tok = Pp_ReadToken(rd)) != NULL) {
        if (! Pp_ExpandMacro(rd, tok)) {
            *tail = Pp_CopyToken(tok);
            tail = &(*tail)->pt_next;
        }
    }
    return head;
}

// Expand a range of a file's tokens into a list.
Pp_Token *Pp_ExpandRange(const Pp_File *file, size_t start, size_t end)
{
    Pp_Reader rd = {
        .rd_file    = file,
        .rd_pos     = start,
        .rd_end     = end,
        .rd_pending = NULL,
        .rd_carry   = PP_FLAG_NONE
    };

    return Pp_ExpandAll(&rd);
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
    bool later = tok->pt_file == pr->pr_file && tok->pt_line > pr->pr_source;

    if ((tok->pt_flags & PP_FLAG_BOL) || later) {
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
    return (tok->pt_flags & PP_FLAG_BOL) && Pp_IsHash(tok);
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
    if (Pp_OpensCond(name)) {
        return Pp_RunIf(file, pos);
    }
    if (Pp_TokenEquals(name, "elif")) {
        return Pp_RunElif(file, pos);
    }
    if (Pp_TokenEquals(name, "else")) {
        return Pp_RunElse(file, pos);
    }
    if (Pp_TokenEquals(name, "endif")) {
        return Pp_RunEndif(file, pos);
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

    size_t start = pos + 3;
    size_t end = Pp_SkipLine(file, pos);
    const Pp_Token *paren = &file->pf_tokens[start];
    Pp_Macro def = {
        .ma_name     = NULL,
        .ma_kind     = PP_MACRO_OBJECT,
        .ma_params   = NULL,
        .ma_nparams  = 0,
        .ma_variadic = false,
        .ma_body     = NULL,
        .ma_nbody    = 0,
        .ma_next     = NULL
    };

    if (Pp_TokenEquals(paren, "(") && ! (paren->pt_flags & (PP_FLAG_BOL | PP_FLAG_SPACE))) {
        def.ma_kind = PP_MACRO_FUNCTION;
        start = Pp_ReadParams(file, start + 1, end, &def);
    }
    def.ma_body = &file->pf_tokens[start];
    def.ma_nbody = end - start;
    Pp_CheckBody(&def, hash->pt_line);
    Pp_DefineMacro(name, &def);
}

// Read a macro's parameter list.
size_t Pp_ReadParams(const Pp_File *file, size_t pos, size_t end, Pp_Macro *def)
{
    const Pp_Token *tokens = file->pf_tokens;
    Ast_Line line = tokens[pos - 1].pt_line;

    if (pos < end && Pp_TokenEquals(&tokens[pos], ")")) {
        return pos + 1;
    }
    for (;;) {
        size_t dup = 0;
        const Pp_Token *param = &tokens[pos];

        Err_AssertAt(line, pos + 1 < end, ERR_PP_MACRO_PARAMS_MALFORMED);
        if (Pp_TokenEquals(param, "...")) {
            def->ma_variadic = true;
        } else {
            Err_AssertAt(line, param->pt_kind == PP_TOKEN_IDENT, ERR_PP_MACRO_PARAMS_MALFORMED);
            Err_AssertAt(line, ! Pp_TokenEquals(param, PP_VA_ARGS), ERR_PP_VA_ARGS_MISPLACED);
            Err_AssertAt(line, ! Pp_FindParam(def, param, &dup), ERR_PP_MACRO_PARAM_DUPLICATE, (int) param->pt_len, param->pt_text);
        }
        def->ma_params = realloc(def->ma_params, (def->ma_nparams + 1) * sizeof(*def->ma_params));
        def->ma_params[def->ma_nparams++] = param;
        pos += 2;
        if (Pp_TokenEquals(&tokens[pos - 1], ")")) {
            return pos;
        }
        Err_AssertAt(line, ! def->ma_variadic && Pp_TokenEquals(&tokens[pos - 1], ","), ERR_PP_MACRO_PARAMS_MALFORMED);
    }
}

// Check a replacement list against the constraints of 6.10.3.
void Pp_CheckBody(const Pp_Macro *def, Ast_Line line)
{
    size_t param = 0;
    size_t n = def->ma_nbody;

    if (n > 0) {
        Err_AssertAt(line, ! Pp_IsHashHash(&def->ma_body[0]) && ! Pp_IsHashHash(&def->ma_body[n - 1]), ERR_PP_PASTE_AT_EDGE);
    }
    for (size_t i = 0; i < n; i++) {
        const Pp_Token *tok = &def->ma_body[i];

        Err_AssertAt(line, def->ma_variadic || ! Pp_TokenEquals(tok, PP_VA_ARGS), ERR_PP_VA_ARGS_MISPLACED);
        if (def->ma_kind == PP_MACRO_FUNCTION && Pp_IsHash(tok)) {
            Err_AssertAt(line, i + 1 < n && Pp_FindParam(def, &def->ma_body[i + 1], &param), ERR_PP_STRINGIZE_NOT_PARAM);
        }
    }
}

// Run the #undef directive at pos.
void Pp_RunUndef(const Pp_File *file, size_t pos)
{
    const Pp_Token *hash = &file->pf_tokens[pos];
    const Pp_Token *name = &file->pf_tokens[pos + 2];

    Err_AssertAt(hash->pt_line, Pp_IsMacroName(name), ERR_PP_MACRO_NAME_MISSING);
    Pp_UndefMacro(name);
}

// True if a directive name opens a conditional.
bool Pp_OpensCond(const Pp_Token *name)
{
    return Pp_TokenEquals(name, "if") || Pp_TokenEquals(name, "ifdef") || Pp_TokenEquals(name, "ifndef");
}

// Run the #if, #ifdef or #ifndef directive at pos.
size_t Pp_RunIf(const Pp_File *file, size_t pos)
{
    bool keep = false;
    Pp_Cond *cond = calloc(1, sizeof(*cond));
    const Pp_Token *name = &file->pf_tokens[pos + 1];

    cond->pc_name = name;
    cond->pc_next = Pp_Conds;
    Pp_Conds = cond;
    if (Pp_TokenEquals(name, "if")) {
        keep = Pp_EvalLine(file, pos);
    } else {
        keep = Pp_IsDefined(file, pos) == Pp_TokenEquals(name, "ifdef");
    }
    if (! keep) {
        return Pp_SkipGroup(file, pos);
    }
    cond->pc_taken = true;
    return Pp_SkipLine(file, pos);
}

// Run the #elif directive at pos.
size_t Pp_RunElif(const Pp_File *file, size_t pos)
{
    Pp_CheckCond(&file->pf_tokens[pos + 1]);
    if (Pp_Conds->pc_taken || ! Pp_EvalLine(file, pos)) {
        return Pp_SkipGroup(file, pos);
    }
    Pp_Conds->pc_taken = true;
    return Pp_SkipLine(file, pos);
}

// Run the #else directive at pos.
size_t Pp_RunElse(const Pp_File *file, size_t pos)
{
    const Pp_Token *name = &file->pf_tokens[pos + 1];

    Pp_CheckCond(name);
    Pp_CheckLineEnd(file, pos + 2, name);
    Pp_Conds->pc_else = true;
    if (Pp_Conds->pc_taken) {
        return Pp_SkipGroup(file, pos);
    }
    Pp_Conds->pc_taken = true;
    return Pp_SkipLine(file, pos);
}

// Run the #endif directive at pos.
size_t Pp_RunEndif(const Pp_File *file, size_t pos)
{
    Pp_Cond *cond = Pp_Conds;
    const Pp_Token *name = &file->pf_tokens[pos + 1];

    Err_AssertAt(name->pt_line, cond, ERR_PP_COND_WITHOUT_IF, (int) name->pt_len, name->pt_text);
    Pp_CheckLineEnd(file, pos + 2, name);
    Pp_Conds = cond->pc_next;
    free(cond);
    return Pp_SkipLine(file, pos);
}

// Check that a directive continues an open conditional.
void Pp_CheckCond(const Pp_Token *name)
{
    Err_AssertAt(name->pt_line, Pp_Conds, ERR_PP_COND_WITHOUT_IF, (int) name->pt_len, name->pt_text);
    Err_AssertAt(name->pt_line, ! Pp_Conds->pc_else, ERR_PP_COND_AFTER_ELSE, (int) name->pt_len, name->pt_text);
}

// Warn about tokens left on a directive's line.
void Pp_CheckLineEnd(const Pp_File *file, size_t pos, const Pp_Token *name)
{
    if (! (file->pf_tokens[pos].pt_flags & PP_FLAG_BOL)) {
        Err_WarnAt(name->pt_line, ERR_PP_COND_EXTRA_TOKENS, (int) name->pt_len, name->pt_text);
    }
}

// True if the macro an #ifdef or #ifndef names is defined.
bool Pp_IsDefined(const Pp_File *file, size_t pos)
{
    const Pp_Token *name = &file->pf_tokens[pos + 1];
    const Pp_Token *macro = &file->pf_tokens[pos + 2];

    Err_AssertAt(name->pt_line, Pp_IsMacroName(macro), ERR_PP_MACRO_NAME_MISSING);
    Pp_CheckLineEnd(file, pos + 3, name);
    return Pp_FindMacro(macro->pt_text, macro->pt_len) != NULL;
}

// Return the position of the directive that ends a skipped group.
size_t Pp_SkipGroup(const Pp_File *file, size_t pos)
{
    size_t depth = 0;

    for (pos = Pp_SkipLine(file, pos); file->pf_tokens[pos].pt_kind != PP_TOKEN_EOF; pos = Pp_SkipLine(file, pos)) {
        const Pp_Token *name = &file->pf_tokens[pos + 1];

        if (! Pp_IsDirective(&file->pf_tokens[pos]) || (name->pt_flags & PP_FLAG_BOL)) {
            continue;
        }
        if (Pp_OpensCond(name)) {
            depth++;
        } else if (depth == 0 && (Pp_TokenEquals(name, "elif") || Pp_TokenEquals(name, "else") || Pp_TokenEquals(name, "endif"))) {
            return pos;
        } else if (Pp_TokenEquals(name, "endif")) {
            depth--;
        }
    }
    return pos;
}

// Evaluate the expression of the #if or #elif directive at pos.
bool Pp_EvalLine(const Pp_File *file, size_t pos)
{
    const Pp_Token *name = &file->pf_tokens[pos + 1];
    Pp_Expr ex = {
        .pe_tok  = Pp_ExpandCondition(file, pos + 2, Pp_SkipLine(file, pos)),
        .pe_line = name->pt_line
    };

    Err_AssertAt(ex.pe_line, ex.pe_tok, ERR_PP_EXPR_EMPTY, (int) name->pt_len, name->pt_text);

    Pp_Value val = Pp_EvalComma(&ex, PP_EVAL_COMPUTE);

    Err_AssertAt(ex.pe_line, ! ex.pe_tok, ERR_PP_EXPR_OPERATOR_MISSING, (int) ex.pe_tok->pt_len, ex.pe_tok->pt_text);
    return Pp_IsTrue(val);
}

// Expand a range of a file's tokens with each defined operator answered.
Pp_Token *Pp_ExpandCondition(const Pp_File *file, size_t start, size_t end)
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
        if (tok->pt_kind == PP_TOKEN_IDENT && Pp_TokenEquals(tok, PP_DEFINED)) {
            *tail = Pp_ReadDefined(&rd, tok);
        } else if (! Pp_ExpandMacro(&rd, tok)) {
            *tail = Pp_CopyToken(tok);
        } else {
            continue;
        }
        tail = &(*tail)->pt_next;
    }
    return head;
}

// Read the operand of a defined operator into its answer.
Pp_Token *Pp_ReadDefined(Pp_Reader *rd, const Pp_Token *op)
{
    const Pp_Token *name = Pp_ReadToken(rd);
    bool paren = name && Pp_TokenEquals(name, "(");

    if (paren) {
        name = Pp_ReadToken(rd);
    }
    Err_AssertAt(op->pt_line, name && name->pt_kind == PP_TOKEN_IDENT, ERR_PP_DEFINED_NAME_MISSING);
    if (paren) {
        const Pp_Token *close = Pp_ReadToken(rd);

        Err_AssertAt(op->pt_line, close && Pp_TokenEquals(close, ")"), ERR_PP_DEFINED_PAREN_MISSING);
    }

    Pp_Token *answer = Pp_CopyToken(op);

    answer->pt_kind = PP_TOKEN_NUMBER;
    answer->pt_text = Pp_FindMacro(name->pt_text, name->pt_len) ? PP_TEXT_TRUE : PP_TEXT_FALSE;
    answer->pt_len = strlen(answer->pt_text);
    return answer;
}

// Evaluate a comma expression.
Pp_Value Pp_EvalComma(Pp_Expr *ex, Pp_Eval mode)
{
    Pp_Value val = Pp_EvalCond(ex, mode);

    while (ex->pe_tok && Pp_TokenEquals(ex->pe_tok, ",")) {
        ex->pe_tok = ex->pe_tok->pt_next;
        val = Pp_EvalCond(ex, mode);
    }
    return val;
}

// Evaluate a conditional expression.
Pp_Value Pp_EvalCond(Pp_Expr *ex, Pp_Eval mode)
{
    Pp_Value cond = Pp_EvalBinary(ex, PP_PREC_OR, mode);

    if (! ex->pe_tok || ! Pp_TokenEquals(ex->pe_tok, "?")) {
        return cond;
    }
    ex->pe_tok = ex->pe_tok->pt_next;

    bool pick = Pp_IsTrue(cond);
    Pp_Value yes = Pp_EvalComma(ex, pick ? mode : PP_EVAL_SKIP);

    Err_AssertAt(ex->pe_line, ex->pe_tok && Pp_TokenEquals(ex->pe_tok, ":"), ERR_PP_EXPR_COLON_MISSING);
    ex->pe_tok = ex->pe_tok->pt_next;

    Pp_Value no = Pp_EvalCond(ex, pick ? PP_EVAL_SKIP : mode);
    Pp_Value val = pick ? yes : no;

    val.pv_unsigned = yes.pv_unsigned || no.pv_unsigned;
    return val;
}

// Evaluate binary operators no looser than min.
Pp_Value Pp_EvalBinary(Pp_Expr *ex, Pp_Prec min, Pp_Eval mode)
{
    Pp_Value lhs = Pp_EvalUnary(ex, mode);

    for (;;) {
        Pp_Op op = PP_OP_COUNT;
        Pp_Eval right = mode;

        if (! ex->pe_tok || ! Pp_FindOp(ex->pe_tok, &op) || Pp_OpPrec[op] < min) {
            return lhs;
        }
        ex->pe_tok = ex->pe_tok->pt_next;
        if ((op == PP_OP_AND && ! Pp_IsTrue(lhs)) || (op == PP_OP_OR && Pp_IsTrue(lhs))) {
            right = PP_EVAL_SKIP;
        }

        Pp_Value rhs = Pp_EvalBinary(ex, (Pp_Prec) (Pp_OpPrec[op] + 1), right);

        lhs = Pp_ApplyOp(op, lhs, rhs, mode, ex->pe_line);
    }
}

// Evaluate a unary expression.
Pp_Value Pp_EvalUnary(Pp_Expr *ex, Pp_Eval mode)
{
    const Pp_Token *tok = ex->pe_tok;
    Pp_Value val = {
        .pv_bits     = 0,
        .pv_unsigned = false
    };

    Err_AssertAt(ex->pe_line, tok, ERR_PP_EXPR_VALUE_MISSING);
    ex->pe_tok = tok->pt_next;
    if (tok->pt_kind == PP_TOKEN_IDENT) {
        return val;
    }
    if (tok->pt_kind == PP_TOKEN_NUMBER) {
        return Pp_EvalNumber(tok, ex->pe_line);
    }
    if (tok->pt_kind == PP_TOKEN_CHAR) {
        return Pp_EvalChar(tok, ex->pe_line);
    }
    if (Pp_TokenEquals(tok, "(")) {
        val = Pp_EvalComma(ex, mode);
        Err_AssertAt(ex->pe_line, ex->pe_tok && Pp_TokenEquals(ex->pe_tok, ")"), ERR_PP_EXPR_PAREN_MISSING);
        ex->pe_tok = ex->pe_tok->pt_next;
        return val;
    }
    if (Pp_TokenEquals(tok, "+")) {
        return Pp_EvalUnary(ex, mode);
    }
    if (Pp_TokenEquals(tok, "-")) {
        val = Pp_EvalUnary(ex, mode);
        val.pv_bits = -val.pv_bits;
        return val;
    }
    if (Pp_TokenEquals(tok, "~")) {
        val = Pp_EvalUnary(ex, mode);
        val.pv_bits = ~val.pv_bits;
        return val;
    }
    if (Pp_TokenEquals(tok, "!")) {
        val.pv_bits = Pp_IsTrue(Pp_EvalUnary(ex, mode)) ? PP_VALUE_FALSE : PP_VALUE_TRUE;
        return val;
    }
    Err_RaiseAt(ex->pe_line, ERR_PP_EXPR_TOKEN_INVALID, (int) tok->pt_len, tok->pt_text);
    return val;
}

// Read the value of an integer constant.
Pp_Value Pp_EvalNumber(const Pp_Token *tok, Ast_Line line)
{
    char *end = NULL;
    bool valid = false;
    char *text = Str_Format("%.*s", (int) tok->pt_len, tok->pt_text);
    Pp_Value val = {
        .pv_bits     = 0,
        .pv_unsigned = false
    };

    Err_AssertAt(line, ! Pp_IsFloat(tok), ERR_PP_EXPR_FLOAT);
    errno = 0;
    val.pv_bits = strtoumax(text, &end, 0);
    Err_AssertAt(line, errno != ERANGE, ERR_PP_EXPR_TOO_LARGE);
    valid = Pp_ReadSuffix(end, &val.pv_unsigned);
    Err_AssertAt(line, valid, ERR_PP_EXPR_SUFFIX_INVALID, end);
    val.pv_unsigned = val.pv_unsigned || val.pv_bits > INTMAX_MAX;
    Str_Free(text);
    return val;
}

// Read the value of a character constant.
Pp_Value Pp_EvalChar(const Pp_Token *tok, Ast_Line line)
{
    bool wide = tok->pt_text[0] == 'L';
    Par_Num num = Par_CharLiteral(tok->pt_text + wide + 1, tok->pt_len - wide - 2, wide ? AST_TYPE_SIZE_INT : AST_TYPE_SIZE_CHAR, line);
    Pp_Value val = {
        .pv_bits     = (uintmax_t) num.pn_val,
        .pv_unsigned = false
    };

    return val;
}

// True if a pp-number is a floating constant.
bool Pp_IsFloat(const Pp_Token *tok)
{
    bool hex = tok->pt_len > 1 && tok->pt_text[0] == '0' && (tok->pt_text[1] == 'x' || tok->pt_text[1] == 'X');

    for (size_t i = 0; i < tok->pt_len; i++) {
        char c = tok->pt_text[i];

        if (c == '.' || (hex ? (c == 'p' || c == 'P') : (c == 'e' || c == 'E'))) {
            return true;
        }
    }
    return false;
}

// Read the suffix of an integer constant.
bool Pp_ReadSuffix(const char *suffix, bool *marked)
{
    const char *p = suffix;

    *marked = false;
    if (*p == 'u' || *p == 'U') {
        *marked = true;
        p++;
    }
    if ((p[0] == 'l' && p[1] == 'l') || (p[0] == 'L' && p[1] == 'L')) {
        p += 2;
    } else if (*p == 'l' || *p == 'L') {
        p++;
    }
    if (! *marked && (*p == 'u' || *p == 'U')) {
        *marked = true;
        p++;
    }
    return *p == '\0';
}

// Find the binary operator a token spells.
bool Pp_FindOp(const Pp_Token *tok, Pp_Op *op)
{
    if (tok->pt_kind != PP_TOKEN_PUNCT) {
        return false;
    }
    for (Pp_Op i = 0; i < PP_OP_COUNT; i++) {
        if (Pp_TokenEquals(tok, Pp_OpText[i])) {
            *op = i;
            return true;
        }
    }
    return false;
}

// Apply a binary operator to two values.
Pp_Value Pp_ApplyOp(Pp_Op op, Pp_Value a, Pp_Value b, Pp_Eval mode, Ast_Line line)
{
    bool truth = false;
    intmax_t x = (intmax_t) a.pv_bits;
    intmax_t y = (intmax_t) b.pv_bits;
    Pp_Value val = {
        .pv_bits     = 0,
        .pv_unsigned = a.pv_unsigned || b.pv_unsigned
    };

    switch (op) {
        case PP_OP_MUL: {
            val.pv_bits = a.pv_bits * b.pv_bits;
            return val;
        } break;
        case PP_OP_DIV:
        case PP_OP_MOD: {
            return Pp_Divide(op, a, b, mode, line);
        } break;
        case PP_OP_ADD: {
            val.pv_bits = a.pv_bits + b.pv_bits;
            return val;
        } break;
        case PP_OP_SUB: {
            val.pv_bits = a.pv_bits - b.pv_bits;
            return val;
        } break;
        case PP_OP_SHL:
        case PP_OP_SHR: {
            return Pp_Shift(op, a, b);
        } break;
        case PP_OP_LT: {
            truth = val.pv_unsigned ? a.pv_bits < b.pv_bits : x < y;
        } break;
        case PP_OP_GT: {
            truth = val.pv_unsigned ? a.pv_bits > b.pv_bits : x > y;
        } break;
        case PP_OP_LE: {
            truth = val.pv_unsigned ? a.pv_bits <= b.pv_bits : x <= y;
        } break;
        case PP_OP_GE: {
            truth = val.pv_unsigned ? a.pv_bits >= b.pv_bits : x >= y;
        } break;
        case PP_OP_EQ: {
            truth = a.pv_bits == b.pv_bits;
        } break;
        case PP_OP_NE: {
            truth = a.pv_bits != b.pv_bits;
        } break;
        case PP_OP_BIT_AND: {
            val.pv_bits = a.pv_bits & b.pv_bits;
            return val;
        } break;
        case PP_OP_BIT_XOR: {
            val.pv_bits = a.pv_bits ^ b.pv_bits;
            return val;
        } break;
        case PP_OP_BIT_OR: {
            val.pv_bits = a.pv_bits | b.pv_bits;
            return val;
        } break;
        case PP_OP_AND: {
            truth = Pp_IsTrue(a) && Pp_IsTrue(b);
        } break;
        case PP_OP_OR: {
            truth = Pp_IsTrue(a) || Pp_IsTrue(b);
        } break;
        case PP_OP_COUNT: {
            // empty
        } break;
    }
    val.pv_bits = truth ? PP_VALUE_TRUE : PP_VALUE_FALSE;
    val.pv_unsigned = false;
    return val;
}

// Divide one value by another.
Pp_Value Pp_Divide(Pp_Op op, Pp_Value a, Pp_Value b, Pp_Eval mode, Ast_Line line)
{
    intmax_t x = (intmax_t) a.pv_bits;
    intmax_t y = (intmax_t) b.pv_bits;
    Pp_Value val = {
        .pv_bits     = 0,
        .pv_unsigned = a.pv_unsigned || b.pv_unsigned
    };

    if (b.pv_bits == 0) {
        Err_AssertAt(line, mode == PP_EVAL_SKIP, ERR_PP_EXPR_DIVISION_BY_ZERO);
        return val;
    }
    if (val.pv_unsigned) {
        val.pv_bits = op == PP_OP_DIV ? a.pv_bits / b.pv_bits : a.pv_bits % b.pv_bits;
    } else if (x == INTMAX_MIN && y == -1) {
        val.pv_bits = op == PP_OP_DIV ? a.pv_bits : 0;
    } else {
        val.pv_bits = (uintmax_t) (op == PP_OP_DIV ? x / y : x % y);
    }
    return val;
}

// Shift a value by a count of bits.
Pp_Value Pp_Shift(Pp_Op op, Pp_Value val, Pp_Value count)
{
    bool back = ! count.pv_unsigned && (intmax_t) count.pv_bits < 0;
    bool fill = ! val.pv_unsigned && (intmax_t) val.pv_bits < 0;
    bool left = (op == PP_OP_SHL) != back;
    uintmax_t n = back ? -count.pv_bits : count.pv_bits;

    if (n >= PP_VALUE_BITS) {
        val.pv_bits = (! left && fill) ? UINTMAX_MAX : 0;
    } else if (left) {
        val.pv_bits <<= n;
    } else if (fill) {
        val.pv_bits = ~(~val.pv_bits >> n);
    } else {
        val.pv_bits >>= n;
    }
    return val;
}

// True if a value is not zero.
bool Pp_IsTrue(Pp_Value val)
{
    return val.pv_bits != 0;
}

// Preprocess one file into the printer.
void Pp_RunFile(Pp_Printer *pr, const Pp_File *file)
{
    Pp_Cond *outer = Pp_Conds;
    uint32_t includer = Pp_CurFile;
    Pp_Reader rd = {
        .rd_file    = file,
        .rd_pos     = 0,
        .rd_end     = file->pf_ntokens - 1,
        .rd_pending = NULL,
        .rd_carry   = PP_FLAG_NONE
    };

    Pp_CurFile = file->pf_index;
    Pp_Conds = NULL;
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
    if (Pp_Conds) {
        Err_RaiseAt(Pp_Conds->pc_name->pt_line, ERR_PP_COND_UNTERMINATED, (int) Pp_Conds->pc_name->pt_len, Pp_Conds->pc_name->pt_text);
    }
    Pp_Conds = outer;
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
