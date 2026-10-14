/*
 * C header file for x86-64 assembly text in AT&T syntax.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivcc.  If not, see <https://www.gnu.org/licenses/>.
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
    TXT_X86_64_REGEX_RR,            // two general registers
    TXT_X86_64_REGEX_GRP_IMM,       // an immediate and a register
    TXT_X86_64_REGEX_MOV_IMM,       // an immediate moved to a register
    TXT_X86_64_REGEX_MEM_FORM,      // a load, a store or lea
    TXT_X86_64_REGEX_MOVSX,         // a sign-extending move
    TXT_X86_64_REGEX_MOVZX,         // a zero-extending move
    TXT_X86_64_REGEX_LEA_RIP,       // lea of a RIP-relative address
    TXT_X86_64_REGEX_GRP_UNARY,     // one general register
    TXT_X86_64_REGEX_SHIFT,         // a shift by %cl
    TXT_X86_64_REGEX_SETCC,         // a setcc on a byte register
    TXT_X86_64_REGEX_BRANCH,        // a jump or a call to a label
    TXT_X86_64_REGEX_MOV_RR,        // a move between general registers
    TXT_X86_64_REGEX_SSE,           // a scalar SSE operation
    TXT_X86_64_REGEX_X87,           // an x87 operation
    TXT_X86_64_REGEX_CALL_REG,      // a call through a register
    TXT_X86_64_REGEX_BARE,          // an instruction with no operand
    TXT_X86_64_REGEX_REG,           // a general register operand
    TXT_X86_64_REGEX_IMM,           // an immediate operand
    TXT_X86_64_REGEX_MEM,           // a memory operand
    TXT_X86_64_REGEX_RIP,           // a RIP-relative operand
    TXT_X86_64_REGEX_LABEL,         // a branch target operand
    TXT_X86_64_REGEX_XMM,           // an SSE register operand
    TXT_X86_64_REGEX_ST,            // an x87 stack register operand
    TXT_X86_64_REGEX_OCTAL,         // an octal escape
    TXT_X86_64_REGEX_HEX,           // a hex escape
    TXT_X86_64_REGEX_ESCAPE,        // an escape naming a byte
    TXT_X86_64_REGEX_PLAIN,         // a run of unescaped bytes
    TXT_X86_64_REGEX_NUMBER,        // a number in a data list
    TXT_X86_64_REGEX_ADDRESS,       // a symbol and its addend in a data list
    TXT_X86_64_REGEX_SECTION_SHORT, // .text, .data or .rodata
    TXT_X86_64_REGEX_SECTION,       // .section with its flags and type
    TXT_X86_64_REGEX_GLOBL,         // .globl or .global
    TXT_X86_64_REGEX_BYTE,          // .byte
    TXT_X86_64_REGEX_WORD,          // .word, .short or .value
    TXT_X86_64_REGEX_LONG,          // .long or .int
    TXT_X86_64_REGEX_QUAD,          // .quad
    TXT_X86_64_REGEX_ZERO,          // .zero, .skip or .space
    TXT_X86_64_REGEX_ALIGN,         // .align
    TXT_X86_64_REGEX_ASCII,         // .ascii
    TXT_X86_64_REGEX_ASCIZ,         // .asciz or .string
    TXT_X86_64_REGEX_IGNORED,       // .file, .ident, .type or .size
    TXT_X86_64_REGEX_LEADING,       // the blanks opening a statement
    TXT_X86_64_REGEX_TRAILING,      // the blanks closing a statement
    TXT_X86_64_REGEX_BLANK,         // an empty statement
    TXT_X86_64_REGEX_LABEL_DEF,     // a label before its statement
    TXT_X86_64_REGEX_DIRECTIVE,     // any directive
    TXT_X86_64_REGEX_SEPARATED,     // a statement ended by `;`
    TXT_X86_64_REGEX_COMMENTED,     // a statement ended by `#`
    TXT_X86_64_REGEX_STATEMENT,     // a statement ended by its line
    TXT_X86_64_REGEX_COUNT          // number of regexes
};

// The bases a string escape is written in.
typedef enum Txt_x86_64_Base Txt_x86_64_Base;
enum Txt_x86_64_Base {
    TXT_X86_64_BASE_OCTAL = 8, // `\ooo`
    TXT_X86_64_BASE_HEX   = 16 // `\x`
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
