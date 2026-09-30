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

// Length of the `%st` that opens an x87 stack register.
#define TXT_X86_64_ST_PREFIX_LEN 3

// Number of x87 stack registers.
#define TXT_X86_64_ST_COUNT 8

// Shortest run of zero bytes written as one `.zero`.
#define TXT_X86_64_ZERO_RUN_MIN 8

// Most values one `.byte` line holds.
#define TXT_X86_64_BYTES_PER_LINE 16

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
void Txt_x86_64_Att_WriteOperand(FILE *out, const Asm_x86_64_Operand *op);
void Txt_x86_64_Att_WriteInstr(FILE *out, const Asm_x86_64_Item *item);
size_t Txt_x86_64_Att_ZeroRun(const uint8_t *bytes, size_t at, size_t len);
void Txt_x86_64_Att_WriteBytes(FILE *out, const uint8_t *bytes, size_t len);
void Txt_x86_64_Att_Write(FILE *out);

// Name-to-value lookups
char    Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width);
int32_t Txt_x86_64_Att_ExtendOp(const char *mnem, Asm_x86_64_Width *width);
int32_t Txt_x86_64_Att_IndirectOp(int32_t opcode);
bool    Txt_x86_64_Att_IsDirectBranch(int32_t opcode);
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width);
int32_t Txt_x86_64_XmmByName(const char *name);
int32_t Txt_x86_64_OpByName(const char *name);

// Text scanning
bool Txt_x86_64_Att_IsNameStart(char c);
bool Txt_x86_64_Att_IsNameChar(char c);
bool Txt_x86_64_Att_IsLabelStart(char c);
bool Txt_x86_64_Att_IsTarget(const char *text);
bool Txt_x86_64_Att_IsAddress(const char *text);
const char *Txt_x86_64_Att_ScanAddress(const char *text, int64_t *addend);
const char *Txt_x86_64_Att_ScanReg(const char *p);
const char *Txt_x86_64_Att_ScanNumber(const char *p, int64_t *out);
int32_t Txt_x86_64_Att_DigitValue(char c, Txt_x86_64_Base base);
const char *Txt_x86_64_Att_ScanString(const char *p, Buf *out);

// AT&T syntax parser
bool Txt_x86_64_Att_ParseSt(const char *text, Asm_x86_64_Operand *op);
bool Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op);
void Txt_x86_64_Att_EmitInts(const char *args, size_t width);
bool Txt_x86_64_Att_EmitAddress(const char *text, size_t width);
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate);
void Txt_x86_64_Att_ParseInstr(const char *line);
void Txt_x86_64_Att_ParseDirective(const char *line);
void Txt_x86_64_Att_ParseLine(char *line);
void Txt_x86_64_Att_Parse(const char *text);

#endif // TXT_X86_64_H
