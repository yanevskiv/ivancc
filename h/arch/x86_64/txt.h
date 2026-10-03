/*
 * C header file for x86-64 assembly text in AT&T syntax.
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

#ifndef TXT_X86_64_H
#define TXT_X86_64_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/object/elf.h"
#include "util/buf.h"
#include "util/str.h"
#include "arch/x86_64/asm.h"

// Shortest run of zero bytes written as one `.zero`.
#define TXT_X86_64_ZERO_RUN_MIN 8

// Most values one `.byte` line holds.
#define TXT_X86_64_BYTES_PER_LINE 16

// The letters a named escape is written with, and the bytes they name.
#define TXT_X86_64_ESCAPES "bfnrt\"\\"
#define TXT_X86_64_ESCAPED "\b\f\n\r\t\"\\"

// Whether a string directive appends a terminating NUL.
typedef enum Txt_x86_64_Terminate Txt_x86_64_Terminate;
enum Txt_x86_64_Terminate {
    TXT_X86_64_BARE,      // .ascii
    TXT_X86_64_TERMINATED // .string and .asciz
};

// The regexes the reader matches, in the order txt.c defines their patterns.
typedef enum Txt_x86_64_RegexIndex Txt_x86_64_RegexIndex;
enum Txt_x86_64_RegexIndex {
    TXT_X86_64_REGEX_RR,
    TXT_X86_64_REGEX_GRP_IMM,
    TXT_X86_64_REGEX_MOV_IMM,
    TXT_X86_64_REGEX_MEM_FORM,
    TXT_X86_64_REGEX_MOVSX,
    TXT_X86_64_REGEX_MOVZX,
    TXT_X86_64_REGEX_LEA_RIP,
    TXT_X86_64_REGEX_GRP_UNARY,
    TXT_X86_64_REGEX_SHIFT,
    TXT_X86_64_REGEX_SETCC,
    TXT_X86_64_REGEX_BRANCH,
    TXT_X86_64_REGEX_MOV_RR,
    TXT_X86_64_REGEX_SSE,
    TXT_X86_64_REGEX_X87,
    TXT_X86_64_REGEX_CALL_REG,
    TXT_X86_64_REGEX_BARE,
    TXT_X86_64_REGEX_REG,
    TXT_X86_64_REGEX_IMM,
    TXT_X86_64_REGEX_MEM,
    TXT_X86_64_REGEX_RIP,
    TXT_X86_64_REGEX_LABEL,
    TXT_X86_64_REGEX_XMM,
    TXT_X86_64_REGEX_ST,
    TXT_X86_64_REGEX_OCTAL,
    TXT_X86_64_REGEX_HEX,
    TXT_X86_64_REGEX_ESCAPE,
    TXT_X86_64_REGEX_PLAIN,
    TXT_X86_64_REGEX_NUMBER,
    TXT_X86_64_REGEX_ADDRESS,
    TXT_X86_64_REGEX_SECTION_SHORT,
    TXT_X86_64_REGEX_SECTION,
    TXT_X86_64_REGEX_GLOBL,
    TXT_X86_64_REGEX_BYTE,
    TXT_X86_64_REGEX_WORD,
    TXT_X86_64_REGEX_LONG,
    TXT_X86_64_REGEX_QUAD,
    TXT_X86_64_REGEX_ZERO,
    TXT_X86_64_REGEX_ASCII,
    TXT_X86_64_REGEX_ASCIZ,
    TXT_X86_64_REGEX_IGNORED,
    TXT_X86_64_REGEX_LEADING,
    TXT_X86_64_REGEX_TRAILING,
    TXT_X86_64_REGEX_BLANK,
    TXT_X86_64_REGEX_LABEL_DEF,
    TXT_X86_64_REGEX_DIRECTIVE,
    TXT_X86_64_REGEX_SEPARATED,
    TXT_X86_64_REGEX_COMMENTED,
    TXT_X86_64_REGEX_STATEMENT,
    TXT_X86_64_REGEX_COUNT
};

// The bases a string escape is written in.
typedef enum Txt_x86_64_Base Txt_x86_64_Base;
enum Txt_x86_64_Base {
    TXT_X86_64_BASE_OCTAL = 8,
    TXT_X86_64_BASE_HEX   = 16
};

// Regex matches
void    Txt_x86_64_Att_RegexPrecompile(void);
int32_t Txt_x86_64_Att_FieldOp(const char *line, const regmatch_t *match, size_t index);

// Name-to-value lookups
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width);
int32_t Txt_x86_64_XmmByName(const char *name);
int32_t Txt_x86_64_OpByName(const char *name);

// AT&T syntax writer
void Txt_x86_64_Att_WriteOperand(FILE *out, const Asm_x86_64_Operand *op);
void Txt_x86_64_Att_WriteInstr(FILE *out, const Asm_x86_64_Item *item);
void Txt_x86_64_Att_WriteBytes(FILE *out, const uint8_t *bytes, size_t len);
void Txt_x86_64_Att_Write(FILE *out);

// AT&T syntax reader
Asm_x86_64_Operand Txt_x86_64_Att_ReadOperand(const char *text);
Asm_x86_64_Item   *Txt_x86_64_Att_NewInstr(const char *line, const regmatch_t *match, int32_t opcode);
void               Txt_x86_64_Att_ReadInstr(const char *line);
void               Txt_x86_64_Att_ReadString(const char *text, Txt_x86_64_Terminate terminate);
void               Txt_x86_64_Att_ReadInts(const char *args, Asm_x86_64_Width width);
void               Txt_x86_64_Att_ReadSection(char *name, const char *flags, const char *type);
void               Txt_x86_64_Att_ReadDirective(const char *line);
void               Txt_x86_64_Att_ReadStatement(const char *text);
void               Txt_x86_64_Att_Read(const char *text);

#endif // TXT_X86_64_H
