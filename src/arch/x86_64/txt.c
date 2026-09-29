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
        const char *stem = item->ai_op == ASM_X86_64_OP_MOVSX ? "movs" : "movz";
        fprintf(out, "  %s%cq", stem, Txt_x86_64_Att_WidthSuffix(item->ai_src.ao_width));
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
                if (strcmp(item->ai_secname, ".text") == 0) {
                    fprintf(out, "  .text\n");
                } else {
                    fprintf(out, "  .section %s\n", item->ai_secname);
                }
            } break;
            case ASM_X86_64_ITEM_BYTES: {
                for (size_t i = 0; i < item->ai_nbytes; i++) {
                    fprintf(out, "  .byte %d\n", item->ai_bytes[i]);
                }
            } break;
            case ASM_X86_64_ITEM_ADDR: {
                fprintf(out, "  .quad %s\n", item->ai_label);
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

// Return the opcode an extending mnemonic names.
int32_t Txt_x86_64_Att_ExtendOp(const char *mnem, Asm_x86_64_Width *width)
{
    const char *rest = NULL;

    if (strncmp(mnem, "movs", 4) == 0) {
        rest = mnem + 4;
    } else if (strncmp(mnem, "movz", 4) == 0) {
        rest = mnem + 4;
    }
    if (! rest || strlen(rest) != 2 || rest[1] != 'q') {
        return -1;
    }
    switch (rest[0]) {
        case 'b': {
            *width = ASM_X86_64_WIDTH_8;
        } break;
        case 'w': {
            *width = ASM_X86_64_WIDTH_16;
        } break;
        case 'l': {
            *width = ASM_X86_64_WIDTH_32;
        } break;
        default: {
            return -1;
        }
    }
    return mnem[3] == 's' ? ASM_X86_64_OP_MOVSX : ASM_X86_64_OP_MOVZX;
}

// Return the indirect form of an opcode.
int32_t Txt_x86_64_Att_IndirectOp(int32_t opcode)
{
    if (opcode == ASM_X86_64_OP_CALL) {
        return ASM_X86_64_OP_CALL_REG;
    }
    return -1;
}

// True if an opcode branches to a label.
bool Txt_x86_64_Att_IsDirectBranch(int32_t opcode)
{
    return opcode == ASM_X86_64_OP_JMP || opcode == ASM_X86_64_OP_JE || opcode == ASM_X86_64_OP_JNE || opcode == ASM_X86_64_OP_CALL;
}

// Return the register index for an AT&T name like "rax"/"al".
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width)
{
    for (int32_t i = 0; i < ASM_X86_64_REG_COUNT; i++) {
        if (strcmp(name, Txt_x86_64_Reg64Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_64;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg32Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_32;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg16Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_16;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg8Name[i]) == 0) {
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
        if (strcmp(name, Txt_x86_64_XmmName[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// Parse an x87 stack register, `%st` or `%st(i)`.
bool Txt_x86_64_Att_ParseSt(const char *text, Asm_x86_64_Operand *op)
{
    int64_t st = 0;
    const char *end;

    if (strncmp(text, "%st", TXT_X86_64_ST_PREFIX_LEN) != 0) {
        return false;
    }
    text += TXT_X86_64_ST_PREFIX_LEN;
    if (*text == '\0') {
        *op = Asm_x86_64_St(0);
        return true;
    }
    if (*text != '(') {
        return false;
    }
    end = Txt_x86_64_Att_ScanNumber(text + 1, &st);
    if (! end || strcmp(end, ")") != 0 || st < 0 || st >= TXT_X86_64_ST_COUNT) {
        return false;
    }
    *op = Asm_x86_64_St((int32_t) st);
    return true;
}

// Return the opcode for a mnemonic.
int32_t Txt_x86_64_OpByName(const char *name)
{
    for (int32_t i = 0; i < ASM_X86_64_OP_COUNT; i++) {
        if (Txt_x86_64_OpName[i] && strcmp(name, Txt_x86_64_OpName[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// True if c can open a symbol name.
bool Txt_x86_64_Att_IsNameStart(char c)
{
    return c == '.' || c == '_' || isalpha((uint8_t) c);
}

// True if c can continue a symbol name.
bool Txt_x86_64_Att_IsNameChar(char c)
{
    return c == '.' || c == '_' || c == '$' || isalnum((uint8_t) c);
}

// True if c can open a label.
bool Txt_x86_64_Att_IsLabelStart(char c)
{
    return c == '$' || Txt_x86_64_Att_IsNameStart(c);
}

// True if text names a branch target.
bool Txt_x86_64_Att_IsTarget(const char *text)
{
    const char *p = text;

    while (Txt_x86_64_Att_IsNameChar(*p)) {
        p++;
    }
    return p > text && *p == '\0';
}

// True if text names a symbol a .quad can hold.
bool Txt_x86_64_Att_IsAddress(const char *text)
{
    return Txt_x86_64_Att_IsNameStart(text[0]) && Txt_x86_64_Att_IsTarget(text);
}

// Scan a register name, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanReg(const char *p)
{
    if (! isalpha((uint8_t) *p)) {
        return NULL;
    }
    while (isalnum((uint8_t) *p)) {
        p++;
    }
    return p;
}

// Scan a decimal, octal or hex integer, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanNumber(const char *p, int64_t *out)
{
    const char *start = p;

    if (*p == '-') {
        p++;
    }
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
        if (! isxdigit((uint8_t) *p)) {
            return NULL;
        }
        while (isxdigit((uint8_t) *p)) {
            p++;
        }
    } else {
        if (! isdigit((uint8_t) *p)) {
            return NULL;
        }
        while (isdigit((uint8_t) *p)) {
            p++;
        }
    }
    *out = strtol(start, NULL, 0);
    return p;
}

// Return the value of a digit in base, or -1 when it is not one.
int32_t Txt_x86_64_Att_DigitValue(char c, Txt_x86_64_Base base)
{
    int32_t value = -1;

    if (isdigit((uint8_t) c)) {
        value = c - '0';
    } else if (isxdigit((uint8_t) c)) {
        value = tolower((uint8_t) c) - 'a' + TXT_X86_64_BASE_DECIMAL;
    }
    return value < (int32_t) base ? value : -1;
}

// Decode the quoted string at p.
const char *Txt_x86_64_Att_ScanString(const char *p, Buf *out)
{
    const char *start = p;

    for (p++; *p != '"'; p++) {
        Err_Assert(*p != '\0', ERR_TXT_STRING_UNTERMINATED, start);
        if (*p != '\\') {
            Buf_PutByte(out, *p);
            continue;
        }
        p++;
        Err_Assert(*p != '\0', ERR_TXT_STRING_UNTERMINATED, start);
        switch (*p) {
            case 'b': {
                Buf_PutByte(out, '\b');
            } break;
            case 'f': {
                Buf_PutByte(out, '\f');
            } break;
            case 'n': {
                Buf_PutByte(out, '\n');
            } break;
            case 'r': {
                Buf_PutByte(out, '\r');
            } break;
            case 't': {
                Buf_PutByte(out, '\t');
            } break;
            case '\\':
            case '"': {
                Buf_PutByte(out, *p);
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
                    int32_t digit = Txt_x86_64_Att_DigitValue(*p, TXT_X86_64_BASE_OCTAL);

                    if (digit < 0) {
                        break;
                    }
                    value = value * TXT_X86_64_BASE_OCTAL + (uint32_t) digit;
                    p++;
                }
                p--;
                Buf_PutByte(out, (char) value);
            } break;
            case 'x':
            case 'X': {
                uint32_t value = 0;

                while (Txt_x86_64_Att_DigitValue(p[1], TXT_X86_64_BASE_HEX) >= 0) {
                    p++;
                    value = value * TXT_X86_64_BASE_HEX + (uint32_t) Txt_x86_64_Att_DigitValue(*p, TXT_X86_64_BASE_HEX);
                }
                Buf_PutByte(out, (char) value);
            } break;
            default: {
                Err_Raise(ERR_TXT_ESCAPE_UNKNOWN, *p);
            } break;
        }
    }
    return p + 1;
}

// Parse one AT&T operand into op.
bool Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op)
{
    int64_t disp = 0;
    const char *end;
    const char *p = text;
    Asm_x86_64_Width width;

    if (Txt_x86_64_Att_ParseSt(text, op)) {
        return true;
    }
    if (text[0] == '%') {
        end = Txt_x86_64_Att_ScanReg(text + 1);
        if (end && *end == '\0') {
            int32_t xmm = Txt_x86_64_XmmByName(text + 1);
            if (xmm >= 0) {
                *op = Asm_x86_64_XmmReg(xmm);
                return true;
            }
            int32_t reg = Txt_x86_64_RegByName(text + 1, &width);
            if (reg < 0) {
                return false;
            }
            *op = Asm_x86_64_RegWidth(reg, width);
            return true;
        }
    }
    if (text[0] == '$') {
        end = Txt_x86_64_Att_ScanNumber(text + 1, &disp);
        if (end && *end == '\0') {
            *op = Asm_x86_64_Imm(disp);
            return true;
        }
    }

    while (Txt_x86_64_Att_IsNameChar(*p)) {
        p++;
    }
    if (p > text && strcmp(p, "(%rip)") == 0) {
        *op = Asm_x86_64_Rip(Str_Slice(text, 0, (size_t) (p - text)));
        return true;
    }

    disp = 0;
    p = Txt_x86_64_Att_ScanNumber(text, &disp);
    if (! p) {
        p = text;
    }
    if (p[0] == '(' && p[1] == '%') {
        end = Txt_x86_64_Att_ScanReg(p + 2);
        if (end && end[0] == ')' && end[1] == '\0') {
            char *name = Str_Slice(p, 2, (size_t) (end - p));
            int32_t base = Txt_x86_64_RegByName(name, &width);
            Str_Free(name);
            if (base < 0) {
                return false;
            }
            *op = Asm_x86_64_Mem(base, (int32_t) disp);
            return true;
        }
    }

    if (Txt_x86_64_Att_IsTarget(text)) {
        *op = Asm_x86_64_Target(Str_Clone(text));
        return true;
    }
    return false;
}

// Emit a .byte/.word/.long/.quad list, little-endian or as an address.
void Txt_x86_64_Att_EmitInts(const char *args, size_t width)
{
    Str_List parts = Str_Split(args, ",");
    for (size_t i = 0; i < parts.sl_count; i++) {
        char *text = Str_Trim(parts.sl_items[i]);
        if (! *text) {
            continue;
        }
        if (Txt_x86_64_Att_EmitAddress(text, width)) {
            continue;
        }
        int64_t val = strtol(text, NULL, 0);
        uint8_t bytes[8];
        for (size_t b = 0; b < width; b++) {
            bytes[b] = (val >> (8 * b)) & 0xFF;
        }
        Asm_x86_64_EmitBytes(bytes, width);
    }
    Str_ListFree(&parts);
}

// Emit a `.quad` item that names a symbol.
bool Txt_x86_64_Att_EmitAddress(const char *text, size_t width)
{
    if (! Txt_x86_64_Att_IsNameStart(text[0])) {
        return false;
    }
    Err_Assert(width == 8 && Txt_x86_64_Att_IsAddress(text), ERR_TXT_QUAD_NOT_ADDRESS, text);
    Asm_x86_64_EmitAddress(text);
    return true;
}

// Emit the bytes of a quoted string.
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate)
{
    Buf *bytes = NULL;
    const char *p = strchr(args, '"');

    if (! p) {
        return;
    }
    bytes = Buf_New();
    Txt_x86_64_Att_ScanString(p, bytes);
    Asm_x86_64_EmitBytes(Buf_Data(bytes), Buf_Len(bytes) + (terminate == TXT_X86_64_TERMINATED ? 1 : 0));
    Buf_Free(bytes);
}

// Parse one instruction line ("mnemonic [op[, op]]") into an instruction item.
void Txt_x86_64_Att_ParseInstr(const char *line)
{
    size_t mlen = strcspn(line, " \t");
    Err_Assert(mlen > 0 && mlen < 32, ERR_TXT_MNEMONIC_MALFORMED, line);
    char mnem[32];
    memcpy(mnem, line, mlen);
    mnem[mlen] = '\0';

    Asm_x86_64_Width ext_width = ASM_X86_64_WIDTH_NONE;
    int32_t ext_opcode = Txt_x86_64_Att_ExtendOp(mnem, &ext_width);

    int32_t opcode = ext_width ? ext_opcode : Txt_x86_64_OpByName(mnem);
    if (opcode < 0 && mlen >= 2 && strchr("bwlq", mnem[mlen - 1])) {
        mnem[mlen - 1] = '\0';
        opcode = Txt_x86_64_OpByName(mnem);
    }
    Err_Assert(opcode >= 0, ERR_TXT_MNEMONIC_UNKNOWN, (int) mlen, line);

    Asm_x86_64_Operand ops[2] = {0};
    int32_t n_ops = 0;
    const char *rest = line + mlen;
    while (*rest == ' ' || *rest == '\t') {
        rest++;
    }
    if (*rest) {
        Str_List parts = Str_Split(rest, ",");
        for (size_t i = 0; i < parts.sl_count; i++) {
            char *text = Str_Trim(parts.sl_items[i]);
            if (! *text) {
                continue;
            }
            Err_Assert(n_ops < 2, ERR_TXT_OPERANDS_TOO_MANY, line);
            const char *op_name = text;
            if (*op_name == '*') {
                opcode = Txt_x86_64_Att_IndirectOp(opcode);
                Err_Assert(opcode >= 0, ERR_TXT_OPERAND_NOT_INDIRECT, line);
                op_name++;
            }
            Err_Assert(Txt_x86_64_Att_ParseOperand(op_name, &ops[n_ops]), ERR_TXT_OPERAND_MALFORMED, text);
            n_ops++;
        }
        Str_ListFree(&parts);
    }

    if (Txt_x86_64_Att_IsDirectBranch(opcode)) {
        Err_Assert(n_ops == 1, ERR_TXT_BRANCH_OPERAND_COUNT, line);
        Err_Assert(ops[0].ao_kind == ASM_X86_64_OPERAND_LABEL, ERR_TXT_BRANCH_NOT_LABEL, line);
    }

    Asm_x86_64_Item *item = Asm_x86_64_New(ASM_X86_64_ITEM_INSTR);
    item->ai_op = opcode;
    if (n_ops == 2) {
        item->ai_src = ops[0];
        item->ai_dst = ops[1];
        if (ext_width) {
            item->ai_src.ao_width = ext_width;
        } else if (item->ai_src.ao_kind == ASM_X86_64_OPERAND_MEM && item->ai_dst.ao_kind == ASM_X86_64_OPERAND_REG) {
            item->ai_src.ao_width = item->ai_dst.ao_width;
        }
    } else if (n_ops == 1) {
        item->ai_dst = ops[0];
    }
}

// Parse one directive line, lowering data directives to raw bytes.
void Txt_x86_64_Att_ParseDirective(const char *line)
{
    size_t nlen = strcspn(line, " \t");
    char name[32];
    if (nlen >= sizeof(name)) {
        nlen = sizeof(name) - 1;
    }
    memcpy(name, line, nlen);
    name[nlen] = '\0';

    const char *args = line + nlen;
    while (*args == ' ' || *args == '\t') {
        args++;
    }

    if (Str_Equals(name, ".text")) {
        Asm_x86_64_EmitSection(".text", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_EXECINSTR);
    } else if (Str_Equals(name, ".data")) {
        Asm_x86_64_EmitSection(".data", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    } else if (Str_Equals(name, ".rodata")) {
        Asm_x86_64_EmitSection(".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    } else if (Str_Equals(name, ".section")) {
        char *secname = Str_Slice(args, 0, strcspn(args, " ,\t"));
        uint32_t type = ELF_SHT_PROGBITS;
        uint64_t flags;
        const char *quote = strchr(args, '"');
        if (quote) {
            flags = 0;
            for (const char *c = quote + 1; *c && *c != '"'; c++) {
                if (*c == 'a') flags |= ELF_SHF_ALLOC;
                if (*c == 'w') flags |= ELF_SHF_WRITE;
                if (*c == 'x') flags |= ELF_SHF_EXECINSTR;
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
        char *sym = Str_Slice(args, 0, strcspn(args, " ,\t"));
        Asm_x86_64_EmitGlobl("%s", sym);
        Str_Free(sym);
    } else if (Str_Equals(name, ".byte")) {
        Txt_x86_64_Att_EmitInts(args, 1);
    } else if (Str_Equals(name, ".word") || Str_Equals(name, ".short") || Str_Equals(name, ".value")) {
        Txt_x86_64_Att_EmitInts(args, 2);
    } else if (Str_Equals(name, ".long") || Str_Equals(name, ".int")) {
        Txt_x86_64_Att_EmitInts(args, 4);
    } else if (Str_Equals(name, ".quad")) {
        Txt_x86_64_Att_EmitInts(args, 8);
    } else if (Str_Equals(name, ".string") || Str_Equals(name, ".asciz")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_TERMINATED);
    } else if (Str_Equals(name, ".ascii")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_BARE);
    } else if (Str_Equals(name, ".skip") || Str_Equals(name, ".zero") || Str_Equals(name, ".space")) {
        int64_t count = strtol(args, NULL, 0);
        size_t n = count > 0 ? (size_t) count : 0;
        uint8_t *zeros = calloc(n ? n : 1, 1);
        Asm_x86_64_EmitBytes(zeros, n);
        free(zeros);
    } else {
        Asm_x86_64_EmitDirective("%s", line);
    }
}

// Parse one line.
void Txt_x86_64_Att_ParseLine(char *line)
{
    bool inq = false;
    for (char *c = line; *c; c++) {
        if (*c == '"') {
            inq = ! inq;
        } else if (*c == '#' && ! inq) {
            *c = '\0';
            break;
        }
    }

    char *text = Str_Trim(line);
    if (*text == '\0') {
        return;
    }

    char *p = text;
    if (Txt_x86_64_Att_IsLabelStart(*p)) {
        while (Txt_x86_64_Att_IsNameChar(*p)) {
            p++;
        }
    }
    if (p > text && *p == ':') {
        char *name = Str_Slice(text, 0, (size_t) (p - text));
        Asm_x86_64_EmitLabel("%s", name);
        Str_Free(name);
        p++;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p) {
            Txt_x86_64_Att_ParseLine(p);
        }
    } else if (text[0] == '.') {
        Txt_x86_64_Att_ParseDirective(text);
    } else {
        Txt_x86_64_Att_ParseInstr(text);
    }
}

// Parse AT&T-syntax assembly text into the instruction list.
void Txt_x86_64_Att_Parse(const char *text)
{
    Asm_x86_64_Reset();
    Str_List lines = Str_Split(text, "\n");
    for (size_t i = 0; i < lines.sl_count; i++) {
        Txt_x86_64_Att_ParseLine(lines.sl_items[i]);
    }
    Str_ListFree(&lines);
}
