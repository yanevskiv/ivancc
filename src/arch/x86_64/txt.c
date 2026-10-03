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
#define TXT_X86_64_REGEX_RR "^(add|sub|cmp|and|or|xor|imul)q?" S R64 C R64 "$"

// The operands Enc_x86_64_EmitGrpImm encodes.
#define TXT_X86_64_REGEX_GRP_IMM "^(add|sub|cmp)q?" S IMM C R64 "$"

// The operands Enc_x86_64_EmitMovImm and Enc_x86_64_EmitMovImm8 encode.
#define TXT_X86_64_REGEX_MOV_IMM \
    "^(mov)q?" S IMM C R64 "$" \
    "|^(mov)b?" S IMM C R8 "$"

// The operands Enc_x86_64_EmitMemForm encodes for a load, a store or lea.
#define TXT_X86_64_REGEX_MEM_FORM \
    "^(mov)q?" S MEM C R64 "$" \
    "|^(mov)l?" S MEM C R32 "$" \
    "|^(mov)w?" S MEM C R16 "$" \
    "|^(mov)q?" S R64 C MEM "$" \
    "|^(mov)l?" S R32 C MEM "$" \
    "|^(mov)w?" S R16 C MEM "$" \
    "|^(mov)b?" S R8 C MEM "$" \
    "|^(lea)q?" S MEM C R64 "$"

// The operands Enc_x86_64_EmitMovsx encodes.
#define TXT_X86_64_REGEX_MOVSX \
    "^(movsbq)" S "(" R8 "|" MEM ")" C R64 "$" \
    "|^(movswq)" S "(" R16 "|" MEM ")" C R64 "$" \
    "|^(movslq)" S "(" R32 "|" MEM ")" C R64 "$"

// The operands Enc_x86_64_EmitMovzx encodes.
#define TXT_X86_64_REGEX_MOVZX \
    "^(movzbq)" S "(" R8 "|" MEM ")" C R64 "$" \
    "|^(movzwq)" S "(" R16 "|" MEM ")" C R64 "$"

// The operands Enc_x86_64_EmitLeaRip encodes.
#define TXT_X86_64_REGEX_LEA_RIP "^(lea)q?" S RIP C R64 "$"

// The operands Enc_x86_64_EmitGrpUnary encodes, and push's and pop's.
#define TXT_X86_64_REGEX_GRP_UNARY "^(idiv|div|neg|not|push|pop)q?" S R64 "$"

// The operands Enc_x86_64_EmitShift encodes.
#define TXT_X86_64_REGEX_SHIFT "^(shl|sar|shr)q?" S "(%cl)" C R64 "$"

// The operands Enc_x86_64_EmitSetcc encodes.
#define TXT_X86_64_REGEX_SETCC "^(set(n?[ep]|l|le|b|be|a|ae))" S R8 "$"

// The operands Enc_x86_64_EmitBranch encodes.
#define TXT_X86_64_REGEX_BRANCH \
    "^(jmp|je|jne)" S "(" NAME ")$" \
    "|^(call)q?" S "(" NAME ")$"

// The operands Enc_x86_64_EmitMov encodes between two general registers.
#define TXT_X86_64_REGEX_MOV_RR \
    "^(mov)q?" S R64 C R64 "$" \
    "|^(mov)l?" S R32 C R32 "$"

// The operands Enc_x86_64_EmitSse and Enc_x86_64_EmitSseRR encode.
#define TXT_X86_64_REGEX_SSE \
    "^((add|sub|mul|div|ucomi)s[sd]|cvtss2sd|cvtsd2ss)" S XMM C XMM "$" \
    "|^(cvtsi2s[sd]q)" S R64 C XMM "$" \
    "|^(cvtts[sd]2si)q?" S XMM C R64 "$" \
    "|^(mov)q" S R64 C XMM "$" \
    "|^(mov)q" S XMM C R64 "$"

// The operands Enc_x86_64_EmitX87 encodes.
#define TXT_X86_64_REGEX_X87 \
    "^(fldt|fstpt|fldl|fstpl|flds|fstps|fildq|fisttpq)" S MEM "$" \
    "|^(faddp|fmulp|fsubrp|fdivrp)" S ST0 C ST "$" \
    "|^(fucomip)" S ST C ST0 "$" \
    "|^(fstp)" S ST "$" \
    "|^(fchs)$"

// The operand Enc_x86_64_EmitInstr encodes for an indirect call.
#define TXT_X86_64_REGEX_CALL_REG "^(call)q?" S "\\*" R64 "$"

// The instructions Enc_x86_64_EmitInstr encodes without operands.
#define TXT_X86_64_REGEX_BARE "^(cqo|syscall)$|^(ret)q?$"

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
        fprintf(out, "  %s%cq", Txt_x86_64_OpName[item->ai_op], Txt_x86_64_Att_WidthSuffix(item->ai_src.ao_width));
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

// Count the zero bytes starting at one offset.
size_t Txt_x86_64_Att_ZeroRun(const uint8_t *bytes, size_t at, size_t len)
{
    size_t end = at;

    while (end < len && bytes[end] == 0) {
        end++;
    }
    return end - at;
}

// Write data bytes as `.zero` runs and `.byte` lines.
void Txt_x86_64_Att_WriteBytes(FILE *out, const uint8_t *bytes, size_t len)
{
    size_t i = 0;

    while (i < len) {
        size_t zeros = Txt_x86_64_Att_ZeroRun(bytes, i, len);
        if (zeros >= TXT_X86_64_ZERO_RUN_MIN) {
            fprintf(out, "  .zero %zu\n", zeros);
            i += zeros;
            continue;
        }
        fprintf(out, "  .byte %d", bytes[i]);
        for (size_t count = 1; ++i < len && count < TXT_X86_64_BYTES_PER_LINE; count++) {
            if (Txt_x86_64_Att_ZeroRun(bytes, i, len) >= TXT_X86_64_ZERO_RUN_MIN) {
                break;
            }
            fprintf(out, ", %d", bytes[i]);
        }
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

// Return the AT&T letter naming an operand width.
char Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width)
{
    switch (width) {
        case ASM_X86_64_WIDTH_8: {
            return 'b';
        } break;
        case ASM_X86_64_WIDTH_16: {
            return 'w';
        } break;
        case ASM_X86_64_WIDTH_64: {
            return 'q';
        } break;
        default: {
            return 'l';
        }
    }
}

// Return the operand width an AT&T letter names.
Asm_x86_64_Width Txt_x86_64_Att_SuffixWidth(char ch)
{
    switch (ch) {
        case 'b': {
            return ASM_X86_64_WIDTH_8;
        } break;
        case 'w': {
            return ASM_X86_64_WIDTH_16;
        } break;
        case 'q': {
            return ASM_X86_64_WIDTH_64;
        } break;
        default: {
            return ASM_X86_64_WIDTH_32;
        }
    }
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

// True if ch can open a symbol name.
bool Txt_x86_64_Att_IsNameStart(char ch)
{
    return ch == '.' || ch == '_' || isalpha((uint8_t) ch);
}

// True if ch can continue a symbol name.
bool Txt_x86_64_Att_IsNameChar(char ch)
{
    return ch == '.' || ch == '_' || ch == '$' || isalnum((uint8_t) ch);
}

// True if ch can open a label.
bool Txt_x86_64_Att_IsLabelStart(char ch)
{
    return ch == '$' || Txt_x86_64_Att_IsNameStart(ch);
}

// Scan a symbol and the offset a `.quad` adds to it.
const char *Txt_x86_64_Att_ScanAddress(const char *text, int64_t *addend)
{
    const char *name = text;
    const char *end = NULL;

    *addend = 0;
    if (! Txt_x86_64_Att_IsNameStart(*text)) {
        return NULL;
    }
    while (Txt_x86_64_Att_IsNameChar(*name)) {
        name++;
    }
    if (*name == '\0') {
        return name;
    }
    if (*name != '+' && *name != '-') {
        return NULL;
    }
    end = Txt_x86_64_Att_ScanNumber(*name == '+' ? name + 1 : name, addend);
    return end && *end == '\0' ? name : NULL;
}

// Scan a decimal, octal or hex integer, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanNumber(const char *text, int64_t *value)
{
    const char *ptr = text;

    if (*ptr == '-') {
        ptr++;
    }
    if (Str_StartsWith(ptr, TXT_X86_64_HEX_PREFIX) || Str_StartsWith(ptr, TXT_X86_64_HEX_PREFIX_UPPER)) {
        ptr += strlen(TXT_X86_64_HEX_PREFIX);
        if (! isxdigit((uint8_t) *ptr)) {
            return NULL;
        }
        while (isxdigit((uint8_t) *ptr)) {
            ptr++;
        }
    } else {
        if (! isdigit((uint8_t) *ptr)) {
            return NULL;
        }
        while (isdigit((uint8_t) *ptr)) {
            ptr++;
        }
    }
    *value = strtol(text, NULL, 0);
    return ptr;
}

// Return the value of a digit in base, or -1 when it is not one.
int32_t Txt_x86_64_Att_DigitValue(char ch, Txt_x86_64_Base base)
{
    int32_t value = -1;

    if (isdigit((uint8_t) ch)) {
        value = ch - '0';
    } else if (isxdigit((uint8_t) ch)) {
        value = tolower((uint8_t) ch) - 'a' + TXT_X86_64_BASE_DECIMAL;
    }
    return value < (int32_t) base ? value : -1;
}

// Decode the quoted string at text.
const char *Txt_x86_64_Att_ScanString(const char *text, Buf *bytes)
{
    const char *ptr = text;

    for (ptr++; *ptr != '"'; ptr++) {
        Err_Assert(*ptr != '\0' && (*ptr != '\\' || ptr[1] != '\0'), ERR_TXT_STRING_NOT_TERMINATED, text);
        if (*ptr != '\\') {
            Buf_PutByte(bytes, *ptr);
            continue;
        }
        ptr++;
        switch (*ptr) {
            case 'b': {
                Buf_PutByte(bytes, '\b');
            } break;
            case 'f': {
                Buf_PutByte(bytes, '\f');
            } break;
            case 'n': {
                Buf_PutByte(bytes, '\n');
            } break;
            case 'r': {
                Buf_PutByte(bytes, '\r');
            } break;
            case 't': {
                Buf_PutByte(bytes, '\t');
            } break;
            case '\\':
            case '"': {
                Buf_PutByte(bytes, *ptr);
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                uint32_t value = 0;

                for (size_t n = 0; n < TXT_X86_64_ESCAPE_OCTAL_DIGITS; n++) {
                    int32_t digit = Txt_x86_64_Att_DigitValue(*ptr, TXT_X86_64_BASE_OCTAL);

                    if (digit < 0) {
                        break;
                    }
                    value = value * TXT_X86_64_BASE_OCTAL + (uint32_t) digit;
                    ptr++;
                }
                ptr--;
                Buf_PutByte(bytes, (char) value);
            } break;
            case 'x':
            case 'X': {
                uint32_t value = 0;

                while (Txt_x86_64_Att_DigitValue(ptr[1], TXT_X86_64_BASE_HEX) >= 0) {
                    ptr++;
                    value = value * TXT_X86_64_BASE_HEX + (uint32_t) Txt_x86_64_Att_DigitValue(*ptr, TXT_X86_64_BASE_HEX);
                }
                Buf_PutByte(bytes, (char) value);
            } break;
            default: {
                Err_Raise(ERR_TXT_ESCAPE_NOT_KNOWN, *ptr);
            } break;
        }
    }
    return ptr + 1;
}

// Match line against pattern, compiling regex on its first use.
bool Txt_x86_64_Att_Match(regex_t *regex, bool *compiled, const char *pattern, const char *line, regmatch_t *match)
{
    if (! *compiled) {
        bool fits = regcomp(regex, pattern, REG_EXTENDED) == 0 && regex->re_nsub < TXT_X86_64_REGEX_GROUPS;

        Err_Assert(fits, ERR_TXT_REGEX_NOT_COMPILED, pattern);
        *compiled = true;
    }
    return regexec(regex, line, TXT_X86_64_REGEX_GROUPS, match, 0) == 0;
}

// Return the group that holds field index of a match, or -1 past the last.
int32_t Txt_x86_64_Att_Field(const regmatch_t *match, size_t index)
{
    regoff_t end = -1;

    for (int32_t group = 1; group < TXT_X86_64_REGEX_GROUPS; group++) {
        if (match[group].rm_so < 0 || match[group].rm_so < end) {
            continue;
        }
        end = match[group].rm_eo;
        if (index == 0) {
            return group;
        }
        index--;
    }
    return -1;
}

// Copy field index of a match out of line.
char *Txt_x86_64_Att_FieldText(const char *line, const regmatch_t *match, size_t index)
{
    int32_t group = Txt_x86_64_Att_Field(match, index);

    return Str_Slice(line, (size_t) match[group].rm_so, (size_t) match[group].rm_eo);
}

// Return the opcode field index of a match names.
int32_t Txt_x86_64_Att_FieldOp(const char *line, const regmatch_t *match, size_t index)
{
    char *name = Txt_x86_64_Att_FieldText(line, match, index);
    int32_t opcode = Txt_x86_64_OpByName(name);

    Str_Free(name);
    return opcode;
}

// Return the operand field index of a match spells.
Asm_x86_64_Operand Txt_x86_64_Att_FieldOperand(const char *line, const regmatch_t *match, size_t index)
{
    char *text = Txt_x86_64_Att_FieldText(line, match, index);
    char *paren = strchr(text, '(');
    int64_t value = 0;
    Asm_x86_64_Width width = ASM_X86_64_WIDTH_NONE;
    Asm_x86_64_Operand op;

    if (Str_StartsWith(text, TXT_X86_64_ST_PREFIX)) {
        if (paren) {
            Txt_x86_64_Att_ScanNumber(paren + 1, &value);
        }
        op = Asm_x86_64_St((int32_t) value);
    } else if (text[0] == '%' && Txt_x86_64_XmmByName(text + 1) >= 0) {
        op = Asm_x86_64_XmmReg(Txt_x86_64_XmmByName(text + 1));
    } else if (text[0] == '%') {
        int32_t reg = Txt_x86_64_RegByName(text + 1, &width);

        op = Asm_x86_64_RegWidth(reg, width);
    } else if (text[0] == '$') {
        Txt_x86_64_Att_ScanNumber(text + 1, &value);
        op = Asm_x86_64_Imm(value);
    } else if (paren && Str_Equals(paren, TXT_X86_64_RIP_SUFFIX)) {
        op = Asm_x86_64_Rip(Str_Slice(text, 0, (size_t) (paren - text)));
    } else if (paren) {
        char *base = Str_Slice(paren, strlen(TXT_X86_64_MEM_PREFIX), strlen(paren) - 1);

        if (paren > text) {
            Txt_x86_64_Att_ScanNumber(text, &value);
        }
        op = Asm_x86_64_Mem(Txt_x86_64_RegByName(base, &width), (int32_t) value);
        Str_Free(base);
    } else {
        op = Asm_x86_64_Target(Str_Clone(text));
    }
    Str_Free(text);
    return op;
}

// Build an instruction of opcode from the operand fields of a match.
Asm_x86_64_Item *Txt_x86_64_Att_NewInstr(const char *line, const regmatch_t *match, int32_t opcode)
{
    Asm_x86_64_Item *item = Asm_x86_64_New(ASM_X86_64_ITEM_INSTR);

    item->ai_op = opcode;
    if (Txt_x86_64_Att_Field(match, 2) >= 0) {
        item->ai_src = Txt_x86_64_Att_FieldOperand(line, match, 1);
        item->ai_dst = Txt_x86_64_Att_FieldOperand(line, match, 2);
    } else if (Txt_x86_64_Att_Field(match, 1) >= 0) {
        item->ai_dst = Txt_x86_64_Att_FieldOperand(line, match, 1);
    }
    if (item->ai_src.ao_kind == ASM_X86_64_OPERAND_MEM) {
        item->ai_src.ao_width = item->ai_dst.ao_width;
    }
    return item;
}

// Parse `<op> %r64, %r64`, the operands Enc_x86_64_EmitRR encodes.
bool Txt_x86_64_Att_EmitRR(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_RR, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `<op> $imm, %r64`, the operands Enc_x86_64_EmitGrpImm encodes.
bool Txt_x86_64_Att_EmitGrpImm(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_GRP_IMM, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `mov $imm, %reg`, the operands Enc_x86_64_EmitMovImm encodes.
bool Txt_x86_64_Att_EmitMovImm(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_MOV_IMM, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOV);
    return true;
}

// Parse a load, store or lea, the operands Enc_x86_64_EmitMemForm encodes.
bool Txt_x86_64_Att_EmitMemForm(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_MEM_FORM, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `movs<w>q`, the operands Enc_x86_64_EmitMovsx encodes.
bool Txt_x86_64_Att_EmitMovsx(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_MOVSX, line, match)) {
        return false;
    }
    char *mnemonic = Txt_x86_64_Att_FieldText(line, match, 0);
    Asm_x86_64_Item *item = Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOVSX);

    item->ai_src.ao_width = Txt_x86_64_Att_SuffixWidth(mnemonic[strlen(Txt_x86_64_OpName[ASM_X86_64_OP_MOVSX])]);
    Str_Free(mnemonic);
    return true;
}

// Parse `movz<w>q`, the operands Enc_x86_64_EmitMovzx encodes.
bool Txt_x86_64_Att_EmitMovzx(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_MOVZX, line, match)) {
        return false;
    }
    char *mnemonic = Txt_x86_64_Att_FieldText(line, match, 0);
    Asm_x86_64_Item *item = Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOVZX);

    item->ai_src.ao_width = Txt_x86_64_Att_SuffixWidth(mnemonic[strlen(Txt_x86_64_OpName[ASM_X86_64_OP_MOVZX])]);
    Str_Free(mnemonic);
    return true;
}

// Parse `lea label(%rip), %r64`, the operands Enc_x86_64_EmitLeaRip encodes.
bool Txt_x86_64_Att_EmitLeaRip(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_LEA_RIP, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_LEA);
    return true;
}

// Parse `<op> %r64`, the operand Enc_x86_64_EmitGrpUnary encodes.
bool Txt_x86_64_Att_EmitGrpUnary(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_GRP_UNARY, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `<op> %cl, %r64`, the operands Enc_x86_64_EmitShift encodes.
bool Txt_x86_64_Att_EmitShift(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_SHIFT, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `set<cc> %r8`, the operand Enc_x86_64_EmitSetcc encodes.
bool Txt_x86_64_Att_EmitSetcc(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_SETCC, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `<op> label`, the operand Enc_x86_64_EmitBranch encodes.
bool Txt_x86_64_Att_EmitBranch(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_BRANCH, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `mov %reg, %reg`, the operands Enc_x86_64_EmitMov encodes.
bool Txt_x86_64_Att_EmitMovRR(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_MOV_RR, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_MOV);
    return true;
}

// Parse an SSE operation, the operands Enc_x86_64_EmitSse encodes.
bool Txt_x86_64_Att_EmitSse(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_SSE, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse an x87 operation, the operands Enc_x86_64_EmitX87 encodes.
bool Txt_x86_64_Att_EmitX87(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_X87, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Parse `call *%r64`, the operand Enc_x86_64_EmitInstr encodes.
bool Txt_x86_64_Att_EmitCallReg(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_CALL_REG, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, ASM_X86_64_OP_CALL_REG);
    return true;
}

// Parse an instruction without operands, as Enc_x86_64_EmitInstr encodes it.
bool Txt_x86_64_Att_EmitBare(const char *line)
{
    static regex_t regex;
    static bool compiled = false;
    regmatch_t match[TXT_X86_64_REGEX_GROUPS];

    if (! Txt_x86_64_Att_Match(&regex, &compiled, TXT_X86_64_REGEX_BARE, line, match)) {
        return false;
    }
    Txt_x86_64_Att_NewInstr(line, match, Txt_x86_64_Att_FieldOp(line, match, 0));
    return true;
}

// Emit a .byte/.word/.long/.quad list, little-endian or as an address.
void Txt_x86_64_Att_EmitInts(const char *args, Asm_x86_64_Width width)
{
    char **parts = Str_Tokenize(args, ",");
    for (char **iter = parts; *iter; iter++) {
        char *text = Str_Trim(*iter);
        if (! *text) {
            continue;
        }
        if (Txt_x86_64_Att_EmitAddress(text, width)) {
            continue;
        }
        int64_t value = strtol(text, NULL, 0);
        size_t size = (size_t) width / ASM_X86_64_BITS_PER_BYTE;
        uint8_t bytes[sizeof(value)];
        for (size_t i = 0; i < size; i++) {
            bytes[i] = (value >> (ASM_X86_64_BITS_PER_BYTE * i)) & UINT8_MAX;
        }
        Asm_x86_64_EmitBytes(bytes, size);
    }
    Str_FreeTokens(parts);
}

// Emit a `.quad` item that names a symbol.
bool Txt_x86_64_Att_EmitAddress(const char *text, Asm_x86_64_Width width)
{
    if (! Txt_x86_64_Att_IsNameStart(text[0])) {
        return false;
    }
    int64_t addend = 0;
    const char *end = Txt_x86_64_Att_ScanAddress(text, &addend);

    Err_Assert(width == ASM_X86_64_WIDTH_64 && end, ERR_TXT_QUAD_NOT_ADDRESS, text);
    char *name = Str_Slice(text, 0, (size_t) (end - text));
    Asm_x86_64_EmitAddress(name, addend);
    Str_Free(name);
    return true;
}

// Emit the bytes of a quoted string.
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate)
{
    Buf *bytes = NULL;
    const char *quote = strchr(args, '"');

    if (! quote) {
        return;
    }
    bytes = Buf_New();
    Txt_x86_64_Att_ScanString(quote, bytes);
    Asm_x86_64_EmitBytes(Buf_Data(bytes), Buf_Len(bytes) + (terminate == TXT_X86_64_TERMINATED ? 1 : 0));
    Buf_Free(bytes);
}

// Parse one instruction line by the first form the encoder has for it.
void Txt_x86_64_Att_ParseInstr(const char *line)
{
    if (Txt_x86_64_Att_EmitRR(line))
        return;
    if (Txt_x86_64_Att_EmitGrpImm(line))
        return;
    if (Txt_x86_64_Att_EmitMovImm(line))
        return;
    if (Txt_x86_64_Att_EmitMemForm(line))
        return;
    if (Txt_x86_64_Att_EmitMovsx(line))
        return;
    if (Txt_x86_64_Att_EmitMovzx(line))
        return;
    if (Txt_x86_64_Att_EmitLeaRip(line))
        return;
    if (Txt_x86_64_Att_EmitGrpUnary(line))
        return;
    if (Txt_x86_64_Att_EmitShift(line))
        return;
    if (Txt_x86_64_Att_EmitSetcc(line))
        return;
    if (Txt_x86_64_Att_EmitBranch(line))
        return;
    if (Txt_x86_64_Att_EmitMovRR(line))
        return;
    if (Txt_x86_64_Att_EmitSse(line))
        return;
    if (Txt_x86_64_Att_EmitX87(line))
        return;
    if (Txt_x86_64_Att_EmitCallReg(line))
        return;
    if (Txt_x86_64_Att_EmitBare(line))
        return;
    Err_Raise(ERR_TXT_INSTRUCTION_NOT_KNOWN, line);
}

// Parse one directive line, lowering data directives to raw bytes.
void Txt_x86_64_Att_ParseDirective(const char *line)
{
    size_t namelen = Str_FindFirst(line, TXT_X86_64_BLANKS);
    char name[TXT_X86_64_NAME_MAX];
    if (namelen >= sizeof(name)) {
        namelen = sizeof(name) - 1;
    }
    memcpy(name, line, namelen);
    name[namelen] = '\0';

    const char *args = line + namelen;
    while (*args && strchr(TXT_X86_64_BLANKS, *args)) {
        args++;
    }

    if (Str_Equals(name, ".text")) {
        Asm_x86_64_EmitSection(".text", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_EXECINSTR);
    } else if (Str_Equals(name, ".data")) {
        Asm_x86_64_EmitSection(".data", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    } else if (Str_Equals(name, ".rodata")) {
        Asm_x86_64_EmitSection(".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    } else if (Str_Equals(name, ".section")) {
        char *secname = Str_Slice(args, 0, Str_FindFirst(args, TXT_X86_64_NAME_END));
        uint32_t type = ELF_SHT_PROGBITS;
        uint64_t flags;
        const char *quote = strchr(args, '"');
        if (quote) {
            flags = 0;
            for (const char *flag = quote + 1; *flag && *flag != '"'; flag++) {
                if (*flag == 'a') {
                    flags |= ELF_SHF_ALLOC;
                }
                if (*flag == 'w') {
                    flags |= ELF_SHF_WRITE;
                }
                if (*flag == 'x') {
                    flags |= ELF_SHF_EXECINSTR;
                }
            }
            const char *at = strchr(args, '@');
            if (at && Str_StartsWith(at + 1, "nobits")) {
                type = ELF_SHT_NOBITS;
            }
        } else if (Str_Equals(secname, ".text")) {
            flags = ELF_SHF_ALLOC | ELF_SHF_EXECINSTR;
        } else if (Str_Equals(secname, ".data")) {
            flags = ELF_SHF_ALLOC | ELF_SHF_WRITE;
        } else if (Str_Equals(secname, ".bss")) {
            type  = ELF_SHT_NOBITS;
            flags = ELF_SHF_ALLOC | ELF_SHF_WRITE;
        } else {
            flags = ELF_SHF_ALLOC;
        }
        Asm_x86_64_EmitSection(secname, type, flags);
    } else if (Str_Equals(name, ".globl") || Str_Equals(name, ".global")) {
        char *sym = Str_Slice(args, 0, Str_FindFirst(args, TXT_X86_64_NAME_END));
        Asm_x86_64_EmitGlobl("%s", sym);
        Str_Free(sym);
    } else if (Str_Equals(name, ".byte")) {
        Txt_x86_64_Att_EmitInts(args, ASM_X86_64_WIDTH_8);
    } else if (Str_Equals(name, ".word") || Str_Equals(name, ".short") || Str_Equals(name, ".value")) {
        Txt_x86_64_Att_EmitInts(args, ASM_X86_64_WIDTH_16);
    } else if (Str_Equals(name, ".long") || Str_Equals(name, ".int")) {
        Txt_x86_64_Att_EmitInts(args, ASM_X86_64_WIDTH_32);
    } else if (Str_Equals(name, ".quad")) {
        Txt_x86_64_Att_EmitInts(args, ASM_X86_64_WIDTH_64);
    } else if (Str_Equals(name, ".string") || Str_Equals(name, ".asciz")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_TERMINATED);
    } else if (Str_Equals(name, ".ascii")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_BARE);
    } else if (Str_Equals(name, ".skip") || Str_Equals(name, ".zero") || Str_Equals(name, ".space")) {
        int64_t count = strtol(args, NULL, 0);
        size_t n = count > 0 ? (size_t) count : 0;
        uint8_t *zeros = calloc(n ? n : 1, sizeof(uint8_t));
        Asm_x86_64_EmitBytes(zeros, n);
        free(zeros);
    } else {
        Asm_x86_64_EmitDirective("%s", line);
    }
}

// Parse one line.
void Txt_x86_64_Att_ParseLine(char *line)
{
    char *rest = NULL;
    bool quoted = false;
    for (char *ptr = line; *ptr; ptr++) {
        if (*ptr == '"') {
            quoted = ! quoted;
        } else if (*ptr == '#' && ! quoted) {
            *ptr = '\0';
            break;
        } else if (*ptr == TXT_X86_64_SEPARATOR && ! quoted) {
            *ptr = '\0';
            rest = ptr + 1;
            break;
        }
    }

    char *text = Str_Trim(line);
    char *ptr = text;
    if (Txt_x86_64_Att_IsLabelStart(*ptr)) {
        while (Txt_x86_64_Att_IsNameChar(*ptr)) {
            ptr++;
        }
    }
    if (ptr > text && *ptr == ':') {
        char *name = Str_Slice(text, 0, (size_t) (ptr - text));
        Asm_x86_64_EmitLabel("%s", name);
        Str_Free(name);
        ptr++;
        while (*ptr && strchr(TXT_X86_64_BLANKS, *ptr)) {
            ptr++;
        }
        if (*ptr) {
            Txt_x86_64_Att_ParseLine(ptr);
        }
    } else if (text[0] == '.') {
        Txt_x86_64_Att_ParseDirective(text);
    } else if (*text != '\0') {
        Txt_x86_64_Att_ParseInstr(text);
    }
    if (rest) {
        Txt_x86_64_Att_ParseLine(rest);
    }
}

// Parse AT&T-syntax assembly text onto the end of the instruction list.
void Txt_x86_64_Att_ParseText(const char *text)
{
    char **lines = Str_Tokenize(text, "\n");
    for (char **iter = lines; *iter; iter++) {
        Txt_x86_64_Att_ParseLine(*iter);
    }
    Str_FreeTokens(lines);
}

// Parse AT&T-syntax assembly text into the instruction list.
void Txt_x86_64_Att_Parse(const char *text)
{
    Asm_x86_64_Reset();
    Txt_x86_64_Att_ParseText(text);
}
