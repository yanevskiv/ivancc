/* Preprocessing-token rules for the cc preprocessor. */
%option noyywrap nounput noinput
%option yylineno
%option prefix="pp"

%{
#include <stdlib.h>
#include "util/err.h"
#include "syntax/pp.h"
%}

DIGIT   [0-9]
ALPHA   [A-Za-z_]
ALNUM   [A-Za-z_0-9]

%%

[ \t\f\v\r]+                            return PP_TOKEN_SPACE;
\n                                      return PP_TOKEN_NEWLINE;
"//"[^\n]*                              return PP_TOKEN_SPACE;
"/*"([^*]|\*+[^*/])*\*+"/"              return PP_TOKEN_SPACE;
"/*"                                    { Err_RaiseAt((Ast_Line) pplineno, ERR_PP_COMMENT_UNTERMINATED); }

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
    for (;;) {
        Pp_TokenKind kind = (Pp_TokenKind) pplex();

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
            .pt_line  = (Ast_Line) pplineno
        };

        if (kind == PP_TOKEN_EOF) {
            tok.pt_len = 0;
            tok.pt_flags = PP_FLAG_BOL;
        }
        file->pf_tokens[file->pf_ntokens++] = tok;
        if (kind == PP_TOKEN_EOF) {
            break;
        }
        flags = PP_FLAG_NONE;
    }
    pp_delete_buffer(buf);
}
