/* lexer.flex - token rules for the cc compiler.
 *
 * Generates build/lex.yy.c (via `flex -o build/lex.yy.c`).  Tokens and the
 * semantic-value union are defined by bison in build/parser.tab.h.
 */
%option noyywrap nounput noinput
%option yylineno

%{
#include <stdlib.h>
#include <string.h>
#include "util/log.h"
#include "util/str.h"
#include "ast/ast.h"
#include "parser.tab.h"

/* Stamp every token with the line it starts on. */
#define YY_USER_ACTION  yylloc = yylineno;

/* Decode a C literal body into raw bytes, reporting the decoded length. */
static char *Lex_Unescape(const char *p, int len, int *out_len)
{
    int n = 0;
    char *buf = malloc(len + 1);

    for (int i = 0; i < len; i++) {
        if (p[i] != '\\' || i + 1 == len) {
            buf[n++] = p[i];
            continue;
        }
        switch (p[++i]) {
            case 'n': {
                buf[n++] = '\n';
            } break;
            case 't': {
                buf[n++] = '\t';
            } break;
            case 'r': {
                buf[n++] = '\r';
            } break;
            case '0': {
                buf[n++] = '\0';
            } break;
            case '\\': {
                buf[n++] = '\\';
            } break;
            case '\'': {
                buf[n++] = '\'';
            } break;
            case '"': {
                buf[n++] = '"';
            } break;
            default: {
                buf[n++] = p[i];
            } break;
        }
    }

    buf[n] = '\0';
    *out_len = n;
    return buf;
}

%}

DIGIT   [0-9]
ALPHA   [A-Za-z_]
ALNUM   [A-Za-z_0-9]

%%

[ \t\r\n]+              ;                       /* whitespace          */
"//"[^\n]*              ;                       /* line comment        */
"/*"([^*]|\*+[^*/])*\*+"/"  ;                   /* block comment       */

"int"                   return INT;
"char"                  return CHAR;
"void"                  return VOID;
"const"                 return CONST;
"return"                return RETURN;
"if"                    return IF;
"else"                  return ELSE;
"for"                   return FOR;
"while"                 return WHILE;
"break"                 return BREAK;
"do"                    return DO;
"switch"                return SWITCH;
"goto"                  return GOTO;
"static"                return STATIC;
"extern"                return EXTERN;
"register"              return REGISTER;
"auto"                  return AUTO;
"inline"                return INLINE;
"case"                  return CASE;
"default"               return DEFAULT;
"continue"              return CONTINUE;
"sizeof"                return SIZEOF;
"struct"                return STRUCT;
"union"                 return UNION;
"enum"                  return ENUM;
"typedef"               return TYPEDEF;
"__builtin_va_arg"      return BUILTIN_VA_ARG;

{ALPHA}{ALNUM}*         { yylval.str = strdup(yytext);
                          return Ast_FindTypedef(yytext) ? TYPEDEF_NAME : IDENT; }

0[xX][0-9A-Fa-f]+       { yylval.num = strtol(yytext, NULL, 16); return NUM; }
{DIGIT}+                { yylval.num = strtol(yytext, NULL, 10); return NUM; }

\"([^"\\\n]|\\.)*\"     { Ast_Str *lit = &yylval.str_lit;
                          lit->as_data = Lex_Unescape(yytext + 1, yyleng - 2, &lit->as_len);
                          return STR; }
'([^'\\\n]|\\.)'        { int n; char *s = Lex_Unescape(yytext + 1, yyleng - 2, &n);
                          yylval.num = (unsigned char) s[0]; Str_Free(s); return NUM; }

"=="                    return EQ;
"!="                    return NE;
"<="                    return LE;
">="                    return GE;
"++"                    return INC;
"--"                    return DEC;
"+="                    return ADD_ASSIGN;
"-="                    return SUB_ASSIGN;
"*="                    return MUL_ASSIGN;
"/="                    return DIV_ASSIGN;
"%="                    return MOD_ASSIGN;
"&="                    return AND_ASSIGN;
"|="                    return OR_ASSIGN;
"^="                    return XOR_ASSIGN;
"<<="                   return SHL_ASSIGN;
">>="                   return SHR_ASSIGN;
"<<"                    return SHL;
">>"                    return SHR;
"&&"                    return AND;
"||"                    return OR;
"..."                   return ELLIPSIS;
"->"                    return ARROW;

"+"                     return ADD;
"-"                     return SUB;
"*"                     return MUL;
"/"                     return DIV;
"%"                     return MOD;
"="                     return ASSIGN;
"<"                     return LT;
">"                     return GT;
"!"                     return NOT;
"&"                     return AMP;
"|"                     return PIPE;
"^"                     return CARET;
"~"                     return TILDE;

"("                     return LPAREN;
")"                     return RPAREN;
"["                     return LSQUARE;
"]"                     return RSQUARE;
"{"                     return LBRACE;
"}"                     return RBRACE;
"?"                     return QUESTION;
":"                     return COLON;
";"                     return SEMI;
","                     return COMMA;
"."                     return DOT;

.                       { Log_ShowErrorAt(yylineno, "lexer: unexpected character '%s'", yytext); }

%%
