/* Preprocessing-token rules for the cc preprocessor. */
%option noyywrap nounput noinput
%option yylineno
%option prefix="pp"

%x PP_HEADER

%{
#include <stdlib.h>
#include "util/err.h"
#include "syntax/pp.h"
%}

DIGIT   [0-9]
ALPHA   [A-Za-z_]
ALNUM   [A-Za-z_0-9]

%%

<INITIAL,PP_HEADER>[ \t\f\v\r]+                 return PP_TOKEN_SPACE;
<INITIAL,PP_HEADER>"//"[^\n]*                   return PP_TOKEN_SPACE;
<INITIAL,PP_HEADER>"/*"([^*]|\*+[^*/])*\*+"/"   return PP_TOKEN_SPACE;
<INITIAL,PP_HEADER>"/*"                         return PP_TOKEN_OPEN_COMMENT;

<PP_HEADER>\"[^"\n]*\"|"<"[^>\n]*">"           { BEGIN(INITIAL); return PP_TOKEN_HEADER_NAME; }
<PP_HEADER>.|\n                                 { yyless(0); BEGIN(INITIAL); }

\n                                      return PP_TOKEN_NEWLINE;

{ALPHA}{ALNUM}*                         return PP_TOKEN_IDENT;
\.?{DIGIT}({ALNUM}|\.|[eEpP][+-])*      return PP_TOKEN_NUMBER;
L?'([^'\\\n]|\\.)+'                     return PP_TOKEN_CHAR;
L?\"([^"\\\n]|\\.)*\"                   return PP_TOKEN_STRING;

"%:%:"|"..."|"<<="|">>="                return PP_TOKEN_PUNCT;
"->"|"++"|"--"|"<<"|">>"|"<="|">="      return PP_TOKEN_PUNCT;
"=="|"!="|"&&"|"||"|"##"                return PP_TOKEN_PUNCT;
"*="|"/="|"%="|"+="|"-="|"&="|"^="|"|=" return PP_TOKEN_PUNCT;
"<:"|":>"|"<%"|"%>"|"%:"                return PP_TOKEN_PUNCT;
[][(){}.&*+\-~!/%<>^|?:;=,#]            return PP_TOKEN_PUNCT;

.                                       return PP_TOKEN_OTHER;

%%

// Tokenize a file's text.
void Pp_Tokenize(Pp_File *file)
{
    size_t cap = 0;
    Pp_Flags flags = PP_FLAG_BOL;
    YY_BUFFER_STATE buf = pp_scan_buffer(file->pf_text, file->pf_len + PP_SCAN_PADDING);

    pplineno = PP_LINE_FIRST;
    BEGIN(INITIAL);
    for (;;) {
        Pp_TokenKind kind = (Pp_TokenKind) pplex();

        Err_AssertAt((Ast_Line) pplineno, kind != PP_TOKEN_OPEN_COMMENT, ERR_PP_COMMENT_UNTERMINATED);
        if (kind == PP_TOKEN_SPACE) {
            flags |= PP_FLAG_SPACE;
            continue;
        }
        if (kind == PP_TOKEN_NEWLINE) {
            flags = PP_FLAG_BOL;
            continue;
        }
        if (file->pf_ntokens == cap) {
            cap = cap ? cap * 2 : PP_TOKENS_MIN;
            file->pf_tokens = realloc(file->pf_tokens, cap * sizeof(*file->pf_tokens));
        }
        Pp_Token tok = {
            .pt_kind  = kind,
            .pt_text  = pptext,
            .pt_len   = (size_t) ppleng,
            .pt_flags = flags,
            .pt_file  = file->pf_index,
            .pt_line  = (Ast_Line) pplineno,
            .pt_hide  = NULL,
            .pt_next  = NULL
        };

        if (kind == PP_TOKEN_EOF) {
            tok.pt_len = 0;
            tok.pt_flags = PP_FLAG_BOL;
        }
        file->pf_tokens[file->pf_ntokens++] = tok;
        if (kind == PP_TOKEN_EOF) {
            break;
        }
        if (Pp_ExpectsHeaderName(file)) {
            BEGIN(PP_HEADER);
        }
        flags = PP_FLAG_NONE;
    }
    pp_delete_buffer(buf);
}

// Lex text as exactly one token.
bool Pp_LexOne(const char *text, size_t len, Pp_TokenKind *kind)
{
    bool whole = false;
    YY_BUFFER_STATE buf = pp_scan_bytes(text, (int) len);

    BEGIN(INITIAL);
    *kind = (Pp_TokenKind) pplex();
    whole = (size_t) ppleng == len;
    pp_delete_buffer(buf);
    return whole && *kind != PP_TOKEN_SPACE && *kind != PP_TOKEN_OPEN_COMMENT;
}
