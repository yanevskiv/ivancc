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
#include <ctype.h>
#include <regex.h>
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

// Most digits an octal escape takes.
#define TXT_X86_64_ESCAPE_OCTAL_DIGITS 3

// The `%st` that opens an x87 stack register.
#define TXT_X86_64_ST_PREFIX "%st"

// Shortest run of zero bytes written as one `.zero`.
#define TXT_X86_64_ZERO_RUN_MIN 8

// Most values one `.byte` line holds.
#define TXT_X86_64_BYTES_PER_LINE 16

// Longest mnemonic or directive name.
#define TXT_X86_64_NAME_MAX 32

// Most groups one instruction pattern holds, plus one.
#define TXT_X86_64_REGEX_GROUPS 128

// The character that ends a statement before the end of its line.
#define TXT_X86_64_SEPARATOR ';'

// Characters that separate a line's fields.
#define TXT_X86_64_BLANKS " \t"

// Characters that end a section or symbol name.
#define TXT_X86_64_NAME_END " ,\t"

// The prefixes that open a hex number.
#define TXT_X86_64_HEX_PREFIX       "0x"
#define TXT_X86_64_HEX_PREFIX_UPPER "0X"

// The `(%` that opens a base-register memory operand.
#define TXT_X86_64_MEM_PREFIX "(%"

// The tail of a RIP-relative operand.
#define TXT_X86_64_RIP_SUFFIX "(%rip)"

// Whether a string directive appends a terminating NUL.
typedef enum Txt_x86_64_Terminate Txt_x86_64_Terminate;
enum Txt_x86_64_Terminate {
    TXT_X86_64_BARE,      // .ascii
    TXT_X86_64_TERMINATED // .string and .asciz
};

// The bases a string escape is written in.
typedef enum Txt_x86_64_Base Txt_x86_64_Base;
enum Txt_x86_64_Base {
    TXT_X86_64_BASE_OCTAL   = 8,
    TXT_X86_64_BASE_DECIMAL = 10,
    TXT_X86_64_BASE_HEX     = 16
};

// AT&T syntax writer
void   Txt_x86_64_Att_WriteOperand(FILE *out, const Asm_x86_64_Operand *op);
void   Txt_x86_64_Att_WriteInstr(FILE *out, const Asm_x86_64_Item *item);
size_t Txt_x86_64_Att_ZeroRun(const uint8_t *bytes, size_t at, size_t len);
void   Txt_x86_64_Att_WriteBytes(FILE *out, const uint8_t *bytes, size_t len);
void   Txt_x86_64_Att_Write(FILE *out);

// Name-to-value lookups
char             Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width);
Asm_x86_64_Width Txt_x86_64_Att_SuffixWidth(char ch);
int32_t          Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width);
int32_t          Txt_x86_64_XmmByName(const char *name);
int32_t          Txt_x86_64_OpByName(const char *name);

// Text scanning
bool        Txt_x86_64_Att_IsNameStart(char ch);
bool        Txt_x86_64_Att_IsNameChar(char ch);
bool        Txt_x86_64_Att_IsLabelStart(char ch);
const char *Txt_x86_64_Att_ScanAddress(const char *text, int64_t *addend);
const char *Txt_x86_64_Att_ScanNumber(const char *text, int64_t *value);
int32_t     Txt_x86_64_Att_DigitValue(char ch, Txt_x86_64_Base base);
const char *Txt_x86_64_Att_ScanString(const char *text, Buf *bytes);

// Regex matches
void               Txt_x86_64_Att_Compile(regex_t *regex, const char *pattern);
bool               Txt_x86_64_Att_Match(const regex_t *regex, const char *line, regmatch_t *match);
int32_t            Txt_x86_64_Att_Field(const regmatch_t *match, size_t index);
char              *Txt_x86_64_Att_FieldText(const char *line, const regmatch_t *match, size_t index);
int32_t            Txt_x86_64_Att_FieldOp(const char *line, const regmatch_t *match, size_t index);
Asm_x86_64_Operand Txt_x86_64_Att_FieldOperand(const char *line, const regmatch_t *match, size_t index);
Asm_x86_64_Item   *Txt_x86_64_Att_NewInstr(const char *line, const regmatch_t *match, int32_t opcode);
Asm_x86_64_Item   *Txt_x86_64_Att_NewExtend(const char *line, const regmatch_t *match, int32_t opcode);

// AT&T syntax parser
void Txt_x86_64_Att_EmitInts(const char *args, Asm_x86_64_Width width);
bool Txt_x86_64_Att_EmitAddress(const char *text, Asm_x86_64_Width width);
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate);
void Txt_x86_64_Att_ParseInstr(const char *line);
void Txt_x86_64_Att_ParseDirective(const char *line);
void Txt_x86_64_Att_ParseLine(char *line);
void Txt_x86_64_Att_ParseText(const char *text);
void Txt_x86_64_Att_Parse(const char *text);

#endif // TXT_X86_64_H
