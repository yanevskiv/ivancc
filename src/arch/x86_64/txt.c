/*
 * C source file for x86-64 assembly text in AT&T syntax.
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

// Module header.
#include "arch/x86_64/txt.h"

// A 64-bit general register.
#define R64 "(%r([abcd]x|[sd]i|[sb]p|[89]|1[0-5]))"

// A 32-bit general register.
#define R32 "(%(e[abcd]x|e[sd]i|e[sb]p|r([89]|1[0-5])d))"

// A 16-bit general register.
#define R16 "(%([abcd]x|[sd]i|[sb]p|r([89]|1[0-5])w))"

// An 8-bit general register.
#define R8 "(%([abcd]l|[sd]il|[sb]pl|r([89]|1[0-5])b))"

// An SSE register.
#define XMM "(%xmm([0-9]|1[0-5]))"

// An x87 stack register.
#define ST "(%st(\\([0-7]\\))?)"

// The top of the x87 stack.
#define ST0 "(%st(\\(0\\))?)"

// A decimal, octal or hex integer.
#define NUM "-?(0[xX][0-9a-fA-F]+|[0-9]+)"

// An immediate.
#define IMM "(\\$" NUM ")"

// A symbol name.
#define NAME "[._a-zA-Z][._$a-zA-Z0-9]*"

// Memory at a displacement from a 64-bit base.
#define MEM "((" NUM ")?\\(" R64 "\\))"

// A symbol addressed relative to %rip.
#define RIP "(" NAME "\\(%rip\\))"

// The blanks after a mnemonic.
#define S "[ \t]+"

// The comma between two operands.
#define C "[ \t]*,[ \t]*"

// The operands Enc_x86_64_EmitRR encodes, and imul's.
#define TXT_X86_64_PATTERN_RR "^(add|sub|cmp|and|or|xor|imul)q?" S R64 C R64 "$"

// The operands Enc_x86_64_EmitGrpImm encodes.
#define TXT_X86_64_PATTERN_GRP_IMM "^(add|sub|cmp)q?" S IMM C R64 "$"

// The operands Enc_x86_64_EmitMovImm and Enc_x86_64_EmitMovImm8 encode.
#define TXT_X86_64_PATTERN_MOV_IMM \
    "^(mov)q?" S IMM C R64 "$" \
    "|^(mov)b?" S IMM C R8 "$"

// The operands Enc_x86_64_EmitMemForm encodes for a load, a store or lea.
#define TXT_X86_64_PATTERN_MEM_FORM \
    "^(mov)q?" S MEM C R64 "$" \
    "|^(mov)l?" S MEM C R32 "$" \
    "|^(mov)w?" S MEM C R16 "$" \
    "|^(mov)q?" S R64 C MEM "$" \
    "|^(mov)l?" S R32 C MEM "$" \
    "|^(mov)w?" S R16 C MEM "$" \
    "|^(mov)b?" S R8 C MEM "$" \
    "|^(lea)q?" S MEM C R64 "$"

// The operands Enc_x86_64_EmitMovsx encodes.
#define TXT_X86_64_PATTERN_MOVSX \
    "^(movsbq)" S "(" R8 "|" MEM ")" C R64 "$" \
    "|^(movswq)" S "(" R16 "|" MEM ")" C R64 "$" \
    "|^(movslq)" S "(" R32 "|" MEM ")" C R64 "$"

// The operands Enc_x86_64_EmitMovzx encodes.
#define TXT_X86_64_PATTERN_MOVZX \
    "^(movzbq)" S "(" R8 "|" MEM ")" C R64 "$" \
    "|^(movzwq)" S "(" R16 "|" MEM ")" C R64 "$"

// The operands Enc_x86_64_EmitLeaRip encodes.
#define TXT_X86_64_PATTERN_LEA_RIP "^(lea)q?" S RIP C R64 "$"

// The operands Enc_x86_64_EmitGrpUnary encodes, and push's and pop's.
#define TXT_X86_64_PATTERN_GRP_UNARY "^(idiv|div|neg|not|push|pop)q?" S R64 "$"

// The operands Enc_x86_64_EmitShift encodes.
#define TXT_X86_64_PATTERN_SHIFT "^(shl|sar|shr)q?" S "(%cl)" C R64 "$"

// The operands Enc_x86_64_EmitSetcc encodes.
#define TXT_X86_64_PATTERN_SETCC "^(set(n?[ep]|l|le|b|be|a|ae))" S R8 "$"

// The operands Enc_x86_64_EmitBranch encodes.
#define TXT_X86_64_PATTERN_BRANCH \
    "^(jmp|je|jne)" S "(" NAME ")$" \
    "|^(call)q?" S "(" NAME ")$"

// The operands Enc_x86_64_EmitMov encodes between two general registers.
#define TXT_X86_64_PATTERN_MOV_RR \
    "^(mov)q?" S R64 C R64 "$" \
    "|^(mov)l?" S R32 C R32 "$"

// The operands Enc_x86_64_EmitSse and Enc_x86_64_EmitSseRR encode.
#define TXT_X86_64_PATTERN_SSE \
    "^((add|sub|mul|div|ucomi)s[sd]|cvtss2sd|cvtsd2ss)" S XMM C XMM "$" \
    "|^(cvtsi2s[sd]q)" S R64 C XMM "$" \
    "|^(cvtts[sd]2si)q?" S XMM C R64 "$" \
    "|^(mov)q" S R64 C XMM "$" \
    "|^(mov)q" S XMM C R64 "$"

// The operands Enc_x86_64_EmitX87 encodes.
#define TXT_X86_64_PATTERN_X87 \
    "^(fldt|fstpt|fldl|fstpl|flds|fstps|fildq|fisttpq)" S MEM "$" \
    "|^(faddp|fmulp|fsubrp|fdivrp)" S ST0 C ST "$" \
    "|^(fucomip)" S ST C ST0 "$" \
    "|^(fstp)" S ST "$" \
    "|^(fchs)$"

// The operand Enc_x86_64_EmitInstr encodes for an indirect call.
#define TXT_X86_64_PATTERN_CALL_REG "^(call)q?" S "\\*" R64 "$"

// The instructions Enc_x86_64_EmitInstr encodes without operands.
#define TXT_X86_64_PATTERN_BARE "^(cqo|syscall)$|^(ret)q?$"

// A label's name, which may open with `$`.
#define LABEL "[._$a-zA-Z][._$a-zA-Z0-9]*"

// A section's name.
#define SECNAME "[._a-zA-Z][._a-zA-Z0-9-]*"

// A quoted string, its escapes passed over.
#define QUOTED "\"([^\"\\\\]|\\\\.)*\""

// A quoted string whose escapes the assembler reads.
#define STRING "\"([^\"\\\\]|\\\\([0-7]{1,3}|[xX][0-9a-fA-F]+|[bfnrt\"\\\\]))*\""

// A general register operand.
#define TXT_X86_64_PATTERN_REG "^(" R64 "|" R32 "|" R16 "|" R8 ")$"

// An immediate operand.
#define TXT_X86_64_PATTERN_IMM "^\\$(" NUM ")$"

// A memory operand, its displacement and its base.
#define TXT_X86_64_PATTERN_MEM "^((" NUM ")?)\\(" R64 "\\)$"

// A RIP-relative operand.
#define TXT_X86_64_PATTERN_RIP "^(" NAME ")\\(%rip\\)$"

// A branch target operand.
#define TXT_X86_64_PATTERN_LABEL "^(" NAME ")$"

// An SSE register operand.
#define TXT_X86_64_PATTERN_XMM "^" XMM "$"

// An x87 stack register operand, and its index.
#define TXT_X86_64_PATTERN_ST "^%st((\\([0-7]\\))?)$"

// An escape that gives a byte in octal.
#define TXT_X86_64_PATTERN_OCTAL "^\\\\[0-7]{1,3}"

// An escape that gives a byte in hex.
#define TXT_X86_64_PATTERN_HEX "^\\\\[xX][0-9a-fA-F]+"

// An escape that names a byte.
#define TXT_X86_64_PATTERN_ESCAPE "^\\\\[bfnrt\"\\\\]"

// A run of bytes that are their own.
#define TXT_X86_64_PATTERN_PLAIN "^[^\"\\\\]+"

// A number in a data list.
#define TXT_X86_64_PATTERN_NUMBER "^[ \t]*(" NUM ")[ \t]*(,|$)"

// A symbol and its addend in a data list.
#define TXT_X86_64_PATTERN_ADDRESS "^[ \t]*(" NAME ")(([+-](0[xX][0-9a-fA-F]+|[0-9]+))?)[ \t]*(,|$)"

// A section named by a directive of its own.
#define TXT_X86_64_PATTERN_SECTION_SHORT "^(\\.text|\\.data|\\.rodata)$"

// A section with its flags and type.
#define TXT_X86_64_PATTERN_SECTION \
    "^\\.section" S "(" SECNAME ")((" C "\"[awx]*\")?)((" C "@(progbits|nobits))?)$"

// A symbol made global.
#define TXT_X86_64_PATTERN_GLOBL "^\\.globa?l" S "(" NAME ")$"

// A list of bytes.
#define TXT_X86_64_PATTERN_BYTE "^\\.byte" S "(.*)$"

// A list of 16-bit values.
#define TXT_X86_64_PATTERN_WORD "^\\.(word|short|value)" S "(.*)$"

// A list of 32-bit values.
#define TXT_X86_64_PATTERN_LONG "^\\.(long|int)" S "(.*)$"

// A list of 64-bit values and addresses.
#define TXT_X86_64_PATTERN_QUAD "^\\.quad" S "(.*)$"

// A run of zero bytes.
#define TXT_X86_64_PATTERN_ZERO "^\\.(zero|skip|space)" S "(" NUM ")$"

// A string without a terminating NUL.
#define TXT_X86_64_PATTERN_ASCII "^\\.ascii" S "(" STRING ")$"

// A string with a terminating NUL.
#define TXT_X86_64_PATTERN_ASCIZ "^\\.(asciz|string)" S "(" STRING ")$"

// The directives the assembler passes over.
#define TXT_X86_64_PATTERN_IGNORED "^\\.(file|ident|type|size)(" S ".*)?$"

// The blanks that open a statement.
#define TXT_X86_64_PATTERN_LEADING "^[ \t]*"

// The blanks that close a statement.
#define TXT_X86_64_PATTERN_TRAILING "[ \t]*$"

// A statement that holds nothing.
#define TXT_X86_64_PATTERN_BLANK "^$"

// A label, before the statement after it.
#define TXT_X86_64_PATTERN_LABEL_DEF "^(" LABEL ")[ \t]*:"

// A directive.
#define TXT_X86_64_PATTERN_DIRECTIVE "^\\."

// A statement and the `;` that ends it.
#define TXT_X86_64_PATTERN_SEPARATED "^([^\"#;]|" QUOTED ")*;"

// A statement and the `#` that opens the comment after it.
#define TXT_X86_64_PATTERN_COMMENTED "^([^\"#;]|" QUOTED ")*#"

// A statement that runs to the end of its line.
#define TXT_X86_64_PATTERN_STATEMENT "^([^\"#;]|" QUOTED ")*$"

// 64-bit register names.
static const char *Txt_x86_64_Reg64Name[ASM_X86_64_REG_COUNT] = {
    [ASM_X86_64_REG_RAX] = "rax",
    [ASM_X86_64_REG_RCX] = "rcx",
    [ASM_X86_64_REG_RDX] = "rdx",
    [ASM_X86_64_REG_RBX] = "rbx",
    [ASM_X86_64_REG_RSP] = "rsp",
    [ASM_X86_64_REG_RBP] = "rbp",
    [ASM_X86_64_REG_RSI] = "rsi",
    [ASM_X86_64_REG_RDI] = "rdi",
    [ASM_X86_64_REG_R8]  = "r8",
    [ASM_X86_64_REG_R9]  = "r9",
    [ASM_X86_64_REG_R10] = "r10",
    [ASM_X86_64_REG_R11] = "r11",
    [ASM_X86_64_REG_R12] = "r12",
    [ASM_X86_64_REG_R13] = "r13",
    [ASM_X86_64_REG_R14] = "r14",
    [ASM_X86_64_REG_R15] = "r15"
};

// 8-bit (low-byte) register names.
static const char *Txt_x86_64_Reg8Name[ASM_X86_64_REG_COUNT] = {
    [ASM_X86_64_REG_RAX] = "al",
    [ASM_X86_64_REG_RCX] = "cl",
    [ASM_X86_64_REG_RDX] = "dl",
    [ASM_X86_64_REG_RBX] = "bl",
    [ASM_X86_64_REG_RSP] = "spl",
    [ASM_X86_64_REG_RBP] = "bpl",
    [ASM_X86_64_REG_RSI] = "sil",
    [ASM_X86_64_REG_RDI] = "dil",
    [ASM_X86_64_REG_R8]  = "r8b",
    [ASM_X86_64_REG_R9]  = "r9b",
    [ASM_X86_64_REG_R10] = "r10b",
    [ASM_X86_64_REG_R11] = "r11b",
    [ASM_X86_64_REG_R12] = "r12b",
    [ASM_X86_64_REG_R13] = "r13b",
    [ASM_X86_64_REG_R14] = "r14b",
    [ASM_X86_64_REG_R15] = "r15b"
};

// 32-bit register names.
static const char *Txt_x86_64_Reg32Name[ASM_X86_64_REG_COUNT] = {
    [ASM_X86_64_REG_RAX] = "eax",
    [ASM_X86_64_REG_RCX] = "ecx",
    [ASM_X86_64_REG_RDX] = "edx",
    [ASM_X86_64_REG_RBX] = "ebx",
    [ASM_X86_64_REG_RSP] = "esp",
    [ASM_X86_64_REG_RBP] = "ebp",
    [ASM_X86_64_REG_RSI] = "esi",
    [ASM_X86_64_REG_RDI] = "edi",
    [ASM_X86_64_REG_R8]  = "r8d",
    [ASM_X86_64_REG_R9]  = "r9d",
    [ASM_X86_64_REG_R10] = "r10d",
    [ASM_X86_64_REG_R11] = "r11d",
    [ASM_X86_64_REG_R12] = "r12d",
    [ASM_X86_64_REG_R13] = "r13d",
    [ASM_X86_64_REG_R14] = "r14d",
    [ASM_X86_64_REG_R15] = "r15d"
};

// 16-bit register names.
static const char *Txt_x86_64_Reg16Name[ASM_X86_64_REG_COUNT] = {
    [ASM_X86_64_REG_RAX] = "ax",
    [ASM_X86_64_REG_RCX] = "cx",
    [ASM_X86_64_REG_RDX] = "dx",
    [ASM_X86_64_REG_RBX] = "bx",
    [ASM_X86_64_REG_RSP] = "sp",
    [ASM_X86_64_REG_RBP] = "bp",
    [ASM_X86_64_REG_RSI] = "si",
    [ASM_X86_64_REG_RDI] = "di",
    [ASM_X86_64_REG_R8]  = "r8w",
    [ASM_X86_64_REG_R9]  = "r9w",
    [ASM_X86_64_REG_R10] = "r10w",
    [ASM_X86_64_REG_R11] = "r11w",
    [ASM_X86_64_REG_R12] = "r12w",
    [ASM_X86_64_REG_R13] = "r13w",
    [ASM_X86_64_REG_R14] = "r14w",
    [ASM_X86_64_REG_R15] = "r15w"
};

// SSE register names.
static const char *Txt_x86_64_XmmName[ASM_X86_64_XMM_COUNT] = {
    [ASM_X86_64_XMM0]  = "xmm0",
    [ASM_X86_64_XMM1]  = "xmm1",
    [ASM_X86_64_XMM2]  = "xmm2",
    [ASM_X86_64_XMM3]  = "xmm3",
    [ASM_X86_64_XMM4]  = "xmm4",
    [ASM_X86_64_XMM5]  = "xmm5",
    [ASM_X86_64_XMM6]  = "xmm6",
    [ASM_X86_64_XMM7]  = "xmm7",
    [ASM_X86_64_XMM8]  = "xmm8",
    [ASM_X86_64_XMM9]  = "xmm9",
    [ASM_X86_64_XMM10] = "xmm10",
    [ASM_X86_64_XMM11] = "xmm11",
    [ASM_X86_64_XMM12] = "xmm12",
    [ASM_X86_64_XMM13] = "xmm13",
    [ASM_X86_64_XMM14] = "xmm14",
    [ASM_X86_64_XMM15] = "xmm15"
};

// Mnemonics.
static const char *Txt_x86_64_OpName[ASM_X86_64_OP_COUNT] = {
    [ASM_X86_64_OP_MOV]     = "mov",
    [ASM_X86_64_OP_MOVSX]   = "movs",
    [ASM_X86_64_OP_LEA]     = "lea",
    [ASM_X86_64_OP_PUSH]    = "push",
    [ASM_X86_64_OP_POP]     = "pop",
    [ASM_X86_64_OP_ADD]     = "add",
    [ASM_X86_64_OP_SUB]     = "sub",
    [ASM_X86_64_OP_IMUL]    = "imul",
    [ASM_X86_64_OP_IDIV]    = "idiv",
    [ASM_X86_64_OP_DIV]     = "div",
    [ASM_X86_64_OP_AND]     = "and",
    [ASM_X86_64_OP_OR]      = "or",
    [ASM_X86_64_OP_XOR]     = "xor",
    [ASM_X86_64_OP_NOT]     = "not",
    [ASM_X86_64_OP_CALL_REG] = "call",
    [ASM_X86_64_OP_SHL]     = "shl",
    [ASM_X86_64_OP_SAR]     = "sar",
    [ASM_X86_64_OP_SHR]     = "shr",
    [ASM_X86_64_OP_CQO]     = "cqo",
    [ASM_X86_64_OP_NEG]     = "neg",
    [ASM_X86_64_OP_CMP]     = "cmp",
    [ASM_X86_64_OP_SETE]    = "sete",
    [ASM_X86_64_OP_SETNE]   = "setne",
    [ASM_X86_64_OP_SETL]    = "setl",
    [ASM_X86_64_OP_SETLE]   = "setle",
    [ASM_X86_64_OP_SETB]    = "setb",
    [ASM_X86_64_OP_SETBE]   = "setbe",
    [ASM_X86_64_OP_MOVZX]   = "movz",
    [ASM_X86_64_OP_JMP]     = "jmp",
    [ASM_X86_64_OP_JE]      = "je",
    [ASM_X86_64_OP_JNE]     = "jne",
    [ASM_X86_64_OP_CALL]    = "call",
    [ASM_X86_64_OP_RET]     = "ret",
    [ASM_X86_64_OP_SYSCALL] = "syscall",
    [ASM_X86_64_OP_SETA]    = "seta",
    [ASM_X86_64_OP_SETAE]   = "setae",
    [ASM_X86_64_OP_SETP]    = "setp",
    [ASM_X86_64_OP_SETNP]   = "setnp",
    [ASM_X86_64_OP_ADDSD]   = "addsd",
    [ASM_X86_64_OP_ADDSS]   = "addss",
    [ASM_X86_64_OP_SUBSD]   = "subsd",
    [ASM_X86_64_OP_SUBSS]   = "subss",
    [ASM_X86_64_OP_MULSD]   = "mulsd",
    [ASM_X86_64_OP_MULSS]   = "mulss",
    [ASM_X86_64_OP_DIVSD]   = "divsd",
    [ASM_X86_64_OP_DIVSS]   = "divss",
    [ASM_X86_64_OP_UCOMISD] = "ucomisd",
    [ASM_X86_64_OP_UCOMISS] = "ucomiss",
    [ASM_X86_64_OP_CVTSS2SD] = "cvtss2sd",
    [ASM_X86_64_OP_CVTSD2SS] = "cvtsd2ss",
    [ASM_X86_64_OP_CVTSI2SD] = "cvtsi2sdq",
    [ASM_X86_64_OP_CVTSI2SS] = "cvtsi2ssq",
    [ASM_X86_64_OP_CVTTSD2SI] = "cvttsd2si",
    [ASM_X86_64_OP_CVTTSS2SI] = "cvttss2si",
    [ASM_X86_64_OP_FLDT]    = "fldt",
    [ASM_X86_64_OP_FSTPT]   = "fstpt",
    [ASM_X86_64_OP_FLDL]    = "fldl",
    [ASM_X86_64_OP_FSTPL]   = "fstpl",
    [ASM_X86_64_OP_FLDS]    = "flds",
    [ASM_X86_64_OP_FSTPS]   = "fstps",
    [ASM_X86_64_OP_FILDQ]   = "fildq",
    [ASM_X86_64_OP_FISTTPQ] = "fisttpq",
    [ASM_X86_64_OP_FADDP]   = "faddp",
    [ASM_X86_64_OP_FMULP]   = "fmulp",
    [ASM_X86_64_OP_FSUBRP]  = "fsubrp",
    [ASM_X86_64_OP_FDIVRP]  = "fdivrp",
    [ASM_X86_64_OP_FCHS]    = "fchs",
    [ASM_X86_64_OP_FUCOMIP] = "fucomip",
    [ASM_X86_64_OP_FSTP]    = "fstp"
};

// The patterns, by Txt_x86_64_RegexIndex.
static const char *Txt_x86_64_Pattern[TXT_X86_64_REGEX_COUNT] = {
    [TXT_X86_64_REGEX_RR]            = TXT_X86_64_PATTERN_RR,
    [TXT_X86_64_REGEX_GRP_IMM]       = TXT_X86_64_PATTERN_GRP_IMM,
    [TXT_X86_64_REGEX_MOV_IMM]       = TXT_X86_64_PATTERN_MOV_IMM,
    [TXT_X86_64_REGEX_MEM_FORM]      = TXT_X86_64_PATTERN_MEM_FORM,
    [TXT_X86_64_REGEX_MOVSX]         = TXT_X86_64_PATTERN_MOVSX,
    [TXT_X86_64_REGEX_MOVZX]         = TXT_X86_64_PATTERN_MOVZX,
    [TXT_X86_64_REGEX_LEA_RIP]       = TXT_X86_64_PATTERN_LEA_RIP,
    [TXT_X86_64_REGEX_GRP_UNARY]     = TXT_X86_64_PATTERN_GRP_UNARY,
    [TXT_X86_64_REGEX_SHIFT]         = TXT_X86_64_PATTERN_SHIFT,
    [TXT_X86_64_REGEX_SETCC]         = TXT_X86_64_PATTERN_SETCC,
    [TXT_X86_64_REGEX_BRANCH]        = TXT_X86_64_PATTERN_BRANCH,
    [TXT_X86_64_REGEX_MOV_RR]        = TXT_X86_64_PATTERN_MOV_RR,
    [TXT_X86_64_REGEX_SSE]           = TXT_X86_64_PATTERN_SSE,
    [TXT_X86_64_REGEX_X87]           = TXT_X86_64_PATTERN_X87,
    [TXT_X86_64_REGEX_CALL_REG]      = TXT_X86_64_PATTERN_CALL_REG,
    [TXT_X86_64_REGEX_BARE]          = TXT_X86_64_PATTERN_BARE,
    [TXT_X86_64_REGEX_REG]           = TXT_X86_64_PATTERN_REG,
    [TXT_X86_64_REGEX_IMM]           = TXT_X86_64_PATTERN_IMM,
    [TXT_X86_64_REGEX_MEM]           = TXT_X86_64_PATTERN_MEM,
    [TXT_X86_64_REGEX_RIP]           = TXT_X86_64_PATTERN_RIP,
    [TXT_X86_64_REGEX_LABEL]         = TXT_X86_64_PATTERN_LABEL,
    [TXT_X86_64_REGEX_XMM]           = TXT_X86_64_PATTERN_XMM,
    [TXT_X86_64_REGEX_ST]            = TXT_X86_64_PATTERN_ST,
    [TXT_X86_64_REGEX_OCTAL]         = TXT_X86_64_PATTERN_OCTAL,
    [TXT_X86_64_REGEX_HEX]           = TXT_X86_64_PATTERN_HEX,
    [TXT_X86_64_REGEX_ESCAPE]        = TXT_X86_64_PATTERN_ESCAPE,
    [TXT_X86_64_REGEX_PLAIN]         = TXT_X86_64_PATTERN_PLAIN,
    [TXT_X86_64_REGEX_NUMBER]        = TXT_X86_64_PATTERN_NUMBER,
    [TXT_X86_64_REGEX_ADDRESS]       = TXT_X86_64_PATTERN_ADDRESS,
    [TXT_X86_64_REGEX_SECTION_SHORT] = TXT_X86_64_PATTERN_SECTION_SHORT,
    [TXT_X86_64_REGEX_SECTION]       = TXT_X86_64_PATTERN_SECTION,
    [TXT_X86_64_REGEX_GLOBL]         = TXT_X86_64_PATTERN_GLOBL,
    [TXT_X86_64_REGEX_BYTE]          = TXT_X86_64_PATTERN_BYTE,
    [TXT_X86_64_REGEX_WORD]          = TXT_X86_64_PATTERN_WORD,
    [TXT_X86_64_REGEX_LONG]          = TXT_X86_64_PATTERN_LONG,
    [TXT_X86_64_REGEX_QUAD]          = TXT_X86_64_PATTERN_QUAD,
    [TXT_X86_64_REGEX_ZERO]          = TXT_X86_64_PATTERN_ZERO,
    [TXT_X86_64_REGEX_ASCII]         = TXT_X86_64_PATTERN_ASCII,
    [TXT_X86_64_REGEX_ASCIZ]         = TXT_X86_64_PATTERN_ASCIZ,
    [TXT_X86_64_REGEX_IGNORED]       = TXT_X86_64_PATTERN_IGNORED,
    [TXT_X86_64_REGEX_LEADING]       = TXT_X86_64_PATTERN_LEADING,
    [TXT_X86_64_REGEX_TRAILING]      = TXT_X86_64_PATTERN_TRAILING,
    [TXT_X86_64_REGEX_BLANK]         = TXT_X86_64_PATTERN_BLANK,
    [TXT_X86_64_REGEX_LABEL_DEF]     = TXT_X86_64_PATTERN_LABEL_DEF,
    [TXT_X86_64_REGEX_DIRECTIVE]     = TXT_X86_64_PATTERN_DIRECTIVE,
    [TXT_X86_64_REGEX_SEPARATED]     = TXT_X86_64_PATTERN_SEPARATED,
    [TXT_X86_64_REGEX_COMMENTED]     = TXT_X86_64_PATTERN_COMMENTED,
    [TXT_X86_64_REGEX_STATEMENT]     = TXT_X86_64_PATTERN_STATEMENT
};

// The compiled regexes, by Txt_x86_64_RegexIndex.
static regex_t Txt_x86_64_Regex[TXT_X86_64_REGEX_COUNT];

// Whether Txt_x86_64_Att_RegexPrecompile has compiled the regexes.
static bool Txt_x86_64_RegexCompiled = false;

// Compile every regex from its pattern, once.
void Txt_x86_64_Att_RegexPrecompile(void)
{
    if (Txt_x86_64_RegexCompiled) {
        return;
    }
    for (int32_t index = 0; index < TXT_X86_64_REGEX_COUNT; index++) {
        bool compiled = Str_RegexCompile(&Txt_x86_64_Regex[index], Txt_x86_64_Pattern[index]);

        Err_Assert(compiled, ERR_TXT_REGEX_NOT_COMPILED, Txt_x86_64_Pattern[index]);
    }
    Txt_x86_64_RegexCompiled = true;
}

// Return the opcode field index of a match names.
int32_t Txt_x86_64_Att_FieldOp(const char *line, const regmatch_t *match, size_t index)
{
    char *name = Str_RegexFieldText(line, match, index);
    int32_t opcode = Txt_x86_64_OpByName(name);

    Str_Free(name);
    return opcode;
}

// Return the register index for an AT&T name like "rax"/"al".
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width)
{
    for (int32_t i = 0; i < ASM_X86_64_REG_COUNT; i++) {
        if (Str_Equals(name, Txt_x86_64_Reg64Name[i])) {
            *width = ASM_X86_64_WIDTH_64;
            return i;
        }
        if (Str_Equals(name, Txt_x86_64_Reg32Name[i])) {
            *width = ASM_X86_64_WIDTH_32;
            return i;
        }
        if (Str_Equals(name, Txt_x86_64_Reg16Name[i])) {
            *width = ASM_X86_64_WIDTH_16;
            return i;
        }
        if (Str_Equals(name, Txt_x86_64_Reg8Name[i])) {
            *width = ASM_X86_64_WIDTH_8;
            return i;
        }
    }
    return -1;
}

// Return the SSE register for an AT&T name like "xmm0".
int32_t Txt_x86_64_XmmByName(const char *name)
{
    for (int32_t i = 0; i < ASM_X86_64_XMM_COUNT; i++) {
        if (Str_Equals(name, Txt_x86_64_XmmName[i])) {
            return i;
        }
    }
    return -1;
}

// Return the opcode for a mnemonic.
int32_t Txt_x86_64_OpByName(const char *name)
{
    for (int32_t i = 0; i < ASM_X86_64_OP_COUNT; i++) {
        if (Str_Equals(name, Txt_x86_64_OpName[i])) {
            return i;
        }
    }
    return -1;
}

// Write one operand in AT&T syntax.
void Txt_x86_64_Att_WriteOperand(FILE *out, const Asm_x86_64_Operand *op)
{
    switch (op->ao_kind) {
        case ASM_X86_64_OPERAND_REG: {
            const char *name = Txt_x86_64_Reg64Name[op->ao_reg];
            if (op->ao_width == ASM_X86_64_WIDTH_8) {
                name = Txt_x86_64_Reg8Name[op->ao_reg];
            } else if (op->ao_width == ASM_X86_64_WIDTH_16) {
                name = Txt_x86_64_Reg16Name[op->ao_reg];
            } else if (op->ao_width == ASM_X86_64_WIDTH_32) {
                name = Txt_x86_64_Reg32Name[op->ao_reg];
            }
            fprintf(out, "%%%s", name);
        } break;
        case ASM_X86_64_OPERAND_IMM: {
            fprintf(out, "$%ld", op->ao_imm);
        } break;
        case ASM_X86_64_OPERAND_MEM: {
            if (op->ao_disp) {
                fprintf(out, "%d(%%%s)", op->ao_disp, Txt_x86_64_Reg64Name[op->ao_reg]);
            } else {
                fprintf(out, "(%%%s)", Txt_x86_64_Reg64Name[op->ao_reg]);
            }
        } break;
        case ASM_X86_64_OPERAND_RIP: {
            fprintf(out, "%s(%%rip)", op->ao_label);
        } break;
        case ASM_X86_64_OPERAND_LABEL: {
            fprintf(out, "%s", op->ao_label);
        } break;
        case ASM_X86_64_OPERAND_XMM: {
            fprintf(out, "%%%s", Txt_x86_64_XmmName[op->ao_xmm]);
        } break;
        case ASM_X86_64_OPERAND_ST: {
            if (op->ao_st) {
                fprintf(out, "%%st(%d)", op->ao_st);
            } else {
                fprintf(out, "%%st");
            }
        } break;
        case ASM_X86_64_OPERAND_NONE:
        case ASM_X86_64_OPERAND_COUNT: {
            // empty
        } break;
    }
}

// Write one instruction: mnemonic plus operands in AT&T order.
void Txt_x86_64_Att_WriteInstr(FILE *out, const Asm_x86_64_Item *item)
{
    if (item->ai_op == ASM_X86_64_OP_MOVSX || item->ai_op == ASM_X86_64_OP_MOVZX) {
        Asm_x86_64_Width width = item->ai_src.ao_width;

        fprintf(out, "  %s%cq", Txt_x86_64_OpName[item->ai_op], width == ASM_X86_64_WIDTH_8 ? 'b' : width == ASM_X86_64_WIDTH_16 ? 'w' : 'l');
    } else if (item->ai_op == ASM_X86_64_OP_MOV && (item->ai_src.ao_kind == ASM_X86_64_OPERAND_XMM || item->ai_dst.ao_kind == ASM_X86_64_OPERAND_XMM)) {
        fprintf(out, "  movq");
    } else {
        fprintf(out, "  %s", Txt_x86_64_OpName[item->ai_op]);
    }

    bool have_src = item->ai_src.ao_kind != ASM_X86_64_OPERAND_NONE;
    bool have_dst = item->ai_dst.ao_kind != ASM_X86_64_OPERAND_NONE;

    if (have_src) {
        fputc(' ', out);
        Txt_x86_64_Att_WriteOperand(out, &item->ai_src);
    }
    if (have_dst) {
        fputs(have_src ? ", " : " ", out);
        if (item->ai_op == ASM_X86_64_OP_CALL_REG) {
            fputc('*', out);
        }
        Txt_x86_64_Att_WriteOperand(out, &item->ai_dst);
    }
    fputc('\n', out);
}

// Write data bytes as `.zero` runs and `.byte` lines.
void Txt_x86_64_Att_WriteBytes(FILE *out, const uint8_t *bytes, size_t len)
{
    size_t i = 0;
    size_t count = 0;

    while (i < len) {
        size_t end = i;

        while (end < len && bytes[end] == 0) {
            end++;
        }
        if (end - i >= TXT_X86_64_ZERO_RUN_MIN) {
            if (count > 0) {
                fputc('\n', out);
                count = 0;
            }
            fprintf(out, "  .zero %zu\n", end - i);
            i = end;
            continue;
        }
        fprintf(out, count > 0 ? ", %d" : "  .byte %d", bytes[i++]);
        if (++count == TXT_X86_64_BYTES_PER_LINE) {
            fputc('\n', out);
            count = 0;
        }
    }
    if (count > 0) {
        fputc('\n', out);
    }
}

// Walk the instruction list and write AT&T-syntax assembly to out.
void Txt_x86_64_Att_Write(FILE *out)
{
    for (Asm_x86_64_Item *item = Asm_x86_64_Items(); item; item = item->ai_next) {
        switch (item->ai_kind) {
            case ASM_X86_64_ITEM_INSTR: {
                Txt_x86_64_Att_WriteInstr(out, item);
            } break;
            case ASM_X86_64_ITEM_LABEL: {
                fprintf(out, "%s:\n", item->ai_label);
            } break;
            case ASM_X86_64_ITEM_GLOBL: {
                fprintf(out, "  .globl %s\n", item->ai_label);
            } break;
            case ASM_X86_64_ITEM_SECTION: {
                if (Str_Equals(item->ai_secname, ".text")) {
                    fprintf(out, "  .text\n");
                } else {
                    fprintf(out, "  .section %s\n", item->ai_secname);
                }
            } break;
            case ASM_X86_64_ITEM_BYTES: {
                Txt_x86_64_Att_WriteBytes(out, item->ai_bytes, item->ai_nbytes);
            } break;
            case ASM_X86_64_ITEM_ADDR: {
                if (item->ai_addend) {
                    fprintf(out, "  .quad %s%+lld\n", item->ai_label, (long long) item->ai_addend);
                } else {
                    fprintf(out, "  .quad %s\n", item->ai_label);
                }
            } break;
            case ASM_X86_64_ITEM_DIRECTIVE: {
                fprintf(out, "  %s\n", item->ai_text);
            } break;
            case ASM_X86_64_ITEM_COUNT: {
                // empty
            } break;
        }
    }
}

// Read one operand in AT&T syntax.
Asm_x86_64_Operand Txt_x86_64_Att_ReadOperand(const char *text)
{
    regmatch_t match[STR_REGEX_GROUPS];
    Asm_x86_64_Width width = ASM_X86_64_WIDTH_NONE;
    Asm_x86_64_Operand op = {0};

    Txt_x86_64_Att_RegexPrecompile();
    if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_REG], text, match)) {
        int32_t index = Txt_x86_64_RegByName(text + 1, &width);

        op = Asm_x86_64_RegWidth(index, width);
    } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_IMM], text, match)) {
        op = Asm_x86_64_Imm(strtol(text + 1, NULL, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MEM], text, match)) {
        char *base = Str_RegexFieldText(text, match, 1);

        op = Asm_x86_64_Mem(Txt_x86_64_RegByName(base + 1, &width), (int32_t) strtol(text, NULL, 0));
        Str_Free(base);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_RIP], text, match)) {
        op = Asm_x86_64_Rip(Str_RegexFieldText(text, match, 0));
    } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_LABEL], text, match)) {
        op = Asm_x86_64_Target(Str_Clone(text));
    } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_XMM], text, match)) {
        op = Asm_x86_64_XmmReg(Txt_x86_64_XmmByName(text + 1));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ST], text, match)) {
        char *index = Str_RegexFieldText(text, match, 0);

        op = Asm_x86_64_St(*index ? (int32_t) strtol(index + 1, NULL, 0) : 0);
        Str_Free(index);
    } else {
        Err_Raise(ERR_TXT_OPERAND_NOT_KNOWN, text);
    }
    return op;
}

// Build an instruction of opcode from the operand fields of a match.
Asm_x86_64_Item *Txt_x86_64_Att_NewInstr(const char *line, const regmatch_t *match, int32_t opcode)
{
    Asm_x86_64_Item *item = Asm_x86_64_New(ASM_X86_64_ITEM_INSTR);

    item->ai_op = opcode;
    for (size_t index = 1; Str_RegexField(match, index) >= 0; index++) {
        char *text = Str_RegexFieldText(line, match, index);

        item->ai_src = item->ai_dst;
        item->ai_dst = Txt_x86_64_Att_ReadOperand(text);
        Str_Free(text);
    }
    if (opcode == ASM_X86_64_OP_MOVSX || opcode == ASM_X86_64_OP_MOVZX) {
        char *mnemonic = Str_RegexFieldText(line, match, 0);
        char ch = mnemonic[strlen(Txt_x86_64_OpName[opcode])];

        item->ai_src.ao_width = ch == 'b' ? ASM_X86_64_WIDTH_8 : ch == 'w' ? ASM_X86_64_WIDTH_16 : ASM_X86_64_WIDTH_32;
        Str_Free(mnemonic);
    } else if (item->ai_src.ao_kind == ASM_X86_64_OPERAND_MEM) {
        item->ai_src.ao_width = item->ai_dst.ao_width;
    }
    return item;
}

// Read one instruction by the first form the encoder has for it.
void Txt_x86_64_Att_ReadInstr(const char *line)
{
    regmatch_t match[STR_REGEX_GROUPS];

    Txt_x86_64_Att_RegexPrecompile();
    if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_RR], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_GRP_IMM], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MOV_IMM], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MEM_FORM], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MOVSX], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOVSX);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MOVZX], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOVZX);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_LEA_RIP], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_GRP_UNARY], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SHIFT], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SETCC], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_BRANCH], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_MOV_RR], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SSE], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_X87], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_CALL_REG], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_CALL_REG);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_BARE], line, match)) {
        Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    } else {
        Err_Raise(ERR_TXT_INSTRUCTION_NOT_KNOWN, line);
    }
}

// Read the quoted string at text into a bytes item.
void Txt_x86_64_Att_ReadString(const char *text, Txt_x86_64_Terminate terminate)
{
    regmatch_t match[STR_REGEX_GROUPS];
    Buf *bytes = Buf_New();

    Txt_x86_64_Att_RegexPrecompile();
    for (const char *ptr = text + 1; *ptr != '"'; ptr += match[0].rm_eo) {
        if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_OCTAL], ptr, match)) {
            char *digits = Str_Slice(ptr, 1, (size_t) match[0].rm_eo);

            Buf_PutByte(bytes, (char) strtol(digits, NULL, TXT_X86_64_BASE_OCTAL));
            Str_Free(digits);
        } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_HEX], ptr, match)) {
            Buf_PutByte(bytes, (char) strtol(ptr + 2, NULL, TXT_X86_64_BASE_HEX));
        } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ESCAPE], ptr, match)) {
            Buf_PutByte(bytes, TXT_X86_64_ESCAPED[strchr(TXT_X86_64_ESCAPES, ptr[1]) - TXT_X86_64_ESCAPES]);
        } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_PLAIN], ptr, match)) {
            Buf_PutBytes(bytes, ptr, (size_t) match[0].rm_eo);
        }
    }
    Asm_x86_64_EmitBytes(Buf_Data(bytes), Buf_Len(bytes) + (terminate == TXT_X86_64_TERMINATED ? 1 : 0));
    Buf_Free(bytes);
}

// Read a .byte/.word/.long/.quad list, little-endian or as addresses.
void Txt_x86_64_Att_ReadInts(const char *args, Asm_x86_64_Width width)
{
    regmatch_t match[STR_REGEX_GROUPS];

    Txt_x86_64_Att_RegexPrecompile();
    for (const char *ptr = args; *ptr; ptr += match[0].rm_eo) {
        if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_NUMBER], ptr, match)) {
            int64_t value = strtol(ptr, NULL, 0);
            size_t size = (size_t) width / ASM_X86_64_BITS_PER_BYTE;
            uint8_t bytes[sizeof(value)];

            for (size_t i = 0; i < size; i++) {
                bytes[i] = (value >> (ASM_X86_64_BITS_PER_BYTE * i)) & UINT8_MAX;
            }
            Asm_x86_64_EmitBytes(bytes, size);
        } else if (width == ASM_X86_64_WIDTH_64 && Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ADDRESS], ptr, match)) {
            char *name = Str_RegexFieldText(ptr, match, 0);
            char *addend = Str_RegexFieldText(ptr, match, 1);

            Asm_x86_64_EmitAddress(name, strtol(addend, NULL, 0));
            Str_Free(name);
            Str_Free(addend);
        } else {
            Err_Raise(ERR_TXT_DATA_NOT_KNOWN, ptr);
        }
    }
}

// Read a section, keeping name, with its flags and type if given.
void Txt_x86_64_Att_ReadSection(char *name, const char *flags, const char *type)
{
    uint32_t sectype = ELF_SHT_PROGBITS;
    uint64_t secflags = ELF_SHF_ALLOC;

    if (*flags) {
        secflags = (strchr(flags, 'a') ? ELF_SHF_ALLOC : 0)
                 | (strchr(flags, 'w') ? ELF_SHF_WRITE : 0)
                 | (strchr(flags, 'x') ? ELF_SHF_EXECINSTR : 0);
    } else if (Str_Equals(name, ".text")) {
        secflags |= ELF_SHF_EXECINSTR;
    } else if (Str_Equals(name, ".data") || Str_Equals(name, ".bss")) {
        secflags |= ELF_SHF_WRITE;
    }
    if (strstr(type, "nobits") || (! *flags && Str_Equals(name, ".bss"))) {
        sectype = ELF_SHT_NOBITS;
    }
    Asm_x86_64_EmitSection(name, sectype, secflags);
}

// Read one directive into the items it stands for.
void Txt_x86_64_Att_ReadDirective(const char *line)
{
    regmatch_t match[STR_REGEX_GROUPS];

    Txt_x86_64_Att_RegexPrecompile();
    if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SECTION_SHORT], line, match)) {
        Txt_x86_64_Att_ReadSection(Str_Clone(line), "", "");
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SECTION], line, match)) {
        char *flags = Str_RegexFieldText(line, match, 1);
        char *type = Str_RegexFieldText(line, match, 2);

        Txt_x86_64_Att_ReadSection(Str_RegexFieldText(line, match, 0), flags, type);
        Str_Free(flags);
        Str_Free(type);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_GLOBL], line, match)) {
        char *name = Str_RegexFieldText(line, match, 0);

        Asm_x86_64_EmitGlobl("%s", name);
        Str_Free(name);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_BYTE], line, match)) {
        Txt_x86_64_Att_ReadInts(Str_RegexFieldTail(line, match, 0), ASM_X86_64_WIDTH_8);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_WORD], line, match)) {
        Txt_x86_64_Att_ReadInts(Str_RegexFieldTail(line, match, 1), ASM_X86_64_WIDTH_16);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_LONG], line, match)) {
        Txt_x86_64_Att_ReadInts(Str_RegexFieldTail(line, match, 1), ASM_X86_64_WIDTH_32);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_QUAD], line, match)) {
        Txt_x86_64_Att_ReadInts(Str_RegexFieldTail(line, match, 0), ASM_X86_64_WIDTH_64);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ZERO], line, match)) {
        int64_t count = strtol(Str_RegexFieldTail(line, match, 1), NULL, 0);
        size_t n = count > 0 ? (size_t) count : 0;
        uint8_t *zeros = calloc(n ? n : 1, sizeof(uint8_t));

        Asm_x86_64_EmitBytes(zeros, n);
        free(zeros);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ASCII], line, match)) {
        Txt_x86_64_Att_ReadString(Str_RegexFieldTail(line, match, 0), TXT_X86_64_BARE);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_ASCIZ], line, match)) {
        Txt_x86_64_Att_ReadString(Str_RegexFieldTail(line, match, 1), TXT_X86_64_TERMINATED);
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_IGNORED], line, match)) {
        Asm_x86_64_EmitDirective("%s", line);
    } else {
        Err_Raise(ERR_TXT_DIRECTIVE_NOT_KNOWN, line);
    }
}

// Read one statement: blank, a label, a directive or an instruction.
void Txt_x86_64_Att_ReadStatement(const char *text)
{
    regmatch_t match[STR_REGEX_GROUPS];

    Txt_x86_64_Att_RegexPrecompile();
    Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_LEADING], text, match);
    text += match[0].rm_eo;
    Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_TRAILING], text, match);

    char *body = Str_Slice(text, 0, (size_t) match[0].rm_so);

    if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_BLANK], body, match)) {
        // empty
    } else if (Str_RegexMatch(&Txt_x86_64_Regex[TXT_X86_64_REGEX_LABEL_DEF], body, match)) {
        char *name = Str_RegexFieldText(body, match, 0);

        Asm_x86_64_EmitLabel("%s", name);
        Str_Free(name);
        Txt_x86_64_Att_ReadStatement(body + match[0].rm_eo);
    } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_DIRECTIVE], body, match)) {
        Txt_x86_64_Att_ReadDirective(body);
    } else {
        Txt_x86_64_Att_ReadInstr(body);
    }
    Str_Free(body);
}

// Read AT&T-syntax text onto the end of the item list, line by line.
void Txt_x86_64_Att_Read(const char *text)
{
    char **lines = Str_Tokenize(text, "\n");
    regmatch_t match[STR_REGEX_GROUPS];

    Txt_x86_64_Att_RegexPrecompile();
    for (char **iter = lines; *iter; iter++) {
        const char *ptr = *iter;

        while (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_SEPARATED], ptr, match)) {
            char *statement = Str_Slice(ptr, 0, (size_t) match[0].rm_eo - 1);

            Txt_x86_64_Att_ReadStatement(statement);
            Str_Free(statement);
            ptr += match[0].rm_eo;
        }
        if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_COMMENTED], ptr, match)) {
            char *statement = Str_Slice(ptr, 0, (size_t) match[0].rm_eo - 1);

            Txt_x86_64_Att_ReadStatement(statement);
            Str_Free(statement);
        } else if (Str_RegexSpan(&Txt_x86_64_Regex[TXT_X86_64_REGEX_STATEMENT], ptr, match)) {
            Txt_x86_64_Att_ReadStatement(ptr);
        } else {
            Err_Raise(ERR_TXT_STRING_NOT_TERMINATED, ptr);
        }
    }
    Str_FreeTokens(lines);
}
