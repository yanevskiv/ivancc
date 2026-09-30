/*
 * Token rules for the cc compiler.
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

%option noyywrap nounput noinput
%option yylineno

%{
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "util/console/err.h"
#include "util/str.h"
#include "lang/ast.h"
#include "lang/par.h"
#include "c.tab.h"

/* Stamp every token with the line it starts on. */
#define YY_USER_ACTION  yylloc = (Ast_Line) yylineno;

%}

DIGIT   [0-9]
ALPHA   [A-Za-z_]
ALNUM   [A-Za-z_0-9]
HEX     [0-9A-Fa-f]
ISUFFIX ([uU](l|L|ll|LL)?|(l|L|ll|LL)[uU]?)
FSUFFIX [fFlL]
DEXP    [eE][+-]?{DIGIT}+
BEXP    [pP][+-]?{DIGIT}+

%%

[ \t\r\n]+              ;                       /* whitespace          */
"//"[^\n]*              ;                       /* line comment        */
"/*"([^*]|\*+[^*/])*\*+"/"  ;                   /* block comment       */

"int"                   return INT;
"char"                  return CHAR;
"short"                 return SHORT;
"long"                  return LONG;
"signed"                return SIGNED;
"unsigned"              return UNSIGNED;
"_Bool"                 return BOOL;
"float"                 return FLOAT;
"double"                return DOUBLE;
"void"                  return VOID;
"const"                 return CONST;
"volatile"              return VOLATILE;
"restrict"              return RESTRICT;
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
"__asm__"               return ASM;
"__builtin_va_list"     return BUILTIN_VA_LIST;
"__builtin_va_start"    return BUILTIN_VA_START;
"__builtin_va_arg"      return BUILTIN_VA_ARG;
"__builtin_va_end"      return BUILTIN_VA_END;
"__builtin_va_copy"     return BUILTIN_VA_COPY;

{ALPHA}{ALNUM}*         { yylval.name = Str_Clone(yytext);
                          return Ast_FindTypedef(yytext) ? TYPEDEF_NAME : IDENT; }

0[xX]{HEX}+{ISUFFIX}?        { yylval.num = Par_NumLiteral(yytext); return NUM; }
0[0-7]*{ISUFFIX}?            { yylval.num = Par_NumLiteral(yytext); return NUM; }
[1-9]{DIGIT}*{ISUFFIX}?      { yylval.num = Par_NumLiteral(yytext); return NUM; }

{DIGIT}+{DEXP}{FSUFFIX}?                    { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }
{DIGIT}*"."{DIGIT}+{DEXP}?{FSUFFIX}?        { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }
{DIGIT}+"."{DEXP}?{FSUFFIX}?                { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }
0[xX]{HEX}+{BEXP}{FSUFFIX}?                 { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }
0[xX]{HEX}*"."{HEX}+{BEXP}{FSUFFIX}?        { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }
0[xX]{HEX}+"."{BEXP}{FSUFFIX}?              { yylval.fnum = Par_FloatLiteral(yytext); return FNUM; }

L?\"([^"\\\n]|\\.)*\"   { bool wide = yytext[0] == 'L';
                          yylval.str = Par_StringLiteral(yytext + wide + 1, yyleng - wide - 2, wide ? AST_TYPE_SIZE_INT : AST_TYPE_SIZE_CHAR, (Ast_Line) yylineno);
                          return STR; }
L?'([^'\\\n]|\\.)+'     { bool wide = yytext[0] == 'L';
                          yylval.num = Par_CharLiteral(yytext + wide + 1, yyleng - wide - 2, wide ? AST_TYPE_SIZE_INT : AST_TYPE_SIZE_CHAR, (Ast_Line) yylineno);
                          return NUM; }

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

"<:"                    return LSQUARE;
":>"                    return RSQUARE;
"<%"                    return LBRACE;
"%>"                    return RBRACE;

.                       { Err_RaiseAt((Ast_Line) yylineno, ERR_LEX_UNEXPECTED_CHAR, yytext); }

%%

// Parse preprocessed text into the program.
void Par_ParseText(const char *text, size_t len)
{
    Err_Assert(len <= INT_MAX, ERR_PAR_TEXT_TOO_LONG, len);

    YY_BUFFER_STATE buf = yy_scan_bytes(text, (int) len);

    yylineno = 1;
    yyparse();
    yy_delete_buffer(buf);
    Par_CompleteTentatives();
}
