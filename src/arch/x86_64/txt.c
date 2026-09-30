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

// Return the opcode an extending mnemonic names.
int32_t Txt_x86_64_Att_ExtendOp(const char *mnemonic, Asm_x86_64_Width *width)
{
    int32_t opcode = -1;

    if (Str_StartsWith(mnemonic, Txt_x86_64_OpName[ASM_X86_64_OP_MOVSX])) {
        opcode = ASM_X86_64_OP_MOVSX;
    } else if (Str_StartsWith(mnemonic, Txt_x86_64_OpName[ASM_X86_64_OP_MOVZX])) {
        opcode = ASM_X86_64_OP_MOVZX;
    }
    if (opcode < 0) {
        return -1;
    }

    const char *suffix = mnemonic + strlen(Txt_x86_64_OpName[opcode]);
    if (*suffix == '\0' || ! Str_Equals(suffix + 1, "q")) {
        return -1;
    }
    switch (suffix[0]) {
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
    return opcode;
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

// Parse an x87 stack register, `%st` or `%st(i)`.
bool Txt_x86_64_Att_ParseSt(const char *text, Asm_x86_64_Operand *op)
{
    int64_t index = 0;
    const char *end;

    if (! Str_StartsWith(text, TXT_X86_64_ST_PREFIX)) {
        return false;
    }
    text += strlen(TXT_X86_64_ST_PREFIX);
    if (*text == '\0') {
        *op = Asm_x86_64_St(0);
        return true;
    }
    if (*text != '(') {
        return false;
    }
    end = Txt_x86_64_Att_ScanNumber(text + 1, &index);
    if (! Str_Equals(end, ")") || index < 0 || index >= TXT_X86_64_ST_COUNT) {
        return false;
    }
    *op = Asm_x86_64_St((int32_t) index);
    return true;
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

// True if text names a branch target.
bool Txt_x86_64_Att_IsTarget(const char *text)
{
    const char *ptr = text;

    while (Txt_x86_64_Att_IsNameChar(*ptr)) {
        ptr++;
    }
    return ptr > text && *ptr == '\0';
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

// Scan a register name, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanReg(const char *text)
{
    const char *ptr = text;

    if (! isalpha((uint8_t) *ptr)) {
        return NULL;
    }
    while (isalnum((uint8_t) *ptr)) {
        ptr++;
    }
    return ptr;
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
        Err_Assert(*ptr != '\0', ERR_TXT_STRING_UNTERMINATED, text);
        if (*ptr != '\\') {
            Buf_PutByte(bytes, *ptr);
            continue;
        }
        ptr++;
        Err_Assert(*ptr != '\0', ERR_TXT_STRING_UNTERMINATED, text);
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
                Err_Raise(ERR_TXT_ESCAPE_UNKNOWN, *ptr);
            } break;
        }
    }
    return ptr + 1;
}

// Parse one AT&T operand into op.
bool Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op)
{
    int64_t imm = 0;
    int64_t disp = 0;
    const char *end;
    const char *ptr = text;
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
        end = Txt_x86_64_Att_ScanNumber(text + 1, &imm);
        if (end && *end == '\0') {
            *op = Asm_x86_64_Imm(imm);
            return true;
        }
    }

    while (Txt_x86_64_Att_IsNameChar(*ptr)) {
        ptr++;
    }
    if (ptr > text && Str_Equals(ptr, TXT_X86_64_RIP_SUFFIX)) {
        *op = Asm_x86_64_Rip(Str_Slice(text, 0, (size_t) (ptr - text)));
        return true;
    }

    ptr = Txt_x86_64_Att_ScanNumber(text, &disp);
    if (! ptr) {
        ptr = text;
    }
    if (Str_StartsWith(ptr, TXT_X86_64_MEM_PREFIX)) {
        const char *basename = ptr + strlen(TXT_X86_64_MEM_PREFIX);
        end = Txt_x86_64_Att_ScanReg(basename);
        if (Str_Equals(end, ")")) {
            char *name = Str_Slice(basename, 0, (size_t) (end - basename));
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

// Parse one instruction line ("mnemonic [op[, op]]") into an instruction item.
void Txt_x86_64_Att_ParseInstr(const char *line)
{
    size_t mnemlen = Str_FindFirst(line, TXT_X86_64_BLANKS);
    Err_Assert(mnemlen > 0 && mnemlen < TXT_X86_64_NAME_MAX, ERR_TXT_MNEMONIC_MALFORMED, line);
    char mnemonic[TXT_X86_64_NAME_MAX];
    memcpy(mnemonic, line, mnemlen);
    mnemonic[mnemlen] = '\0';

    Asm_x86_64_Width ext_width = ASM_X86_64_WIDTH_NONE;
    int32_t ext_opcode = Txt_x86_64_Att_ExtendOp(mnemonic, &ext_width);

    int32_t opcode = ext_width ? ext_opcode : Txt_x86_64_OpByName(mnemonic);
    if (opcode < 0 && mnemlen > 1 && strchr(TXT_X86_64_WIDTH_SUFFIXES, mnemonic[mnemlen - 1])) {
        mnemonic[mnemlen - 1] = '\0';
        opcode = Txt_x86_64_OpByName(mnemonic);
    }
    Err_Assert(opcode >= 0, ERR_TXT_MNEMONIC_UNKNOWN, (int) mnemlen, line);

    Asm_x86_64_Operand ops[TXT_X86_64_OPERANDS_MAX] = {0};
    int32_t nops = 0;
    const char *rest = line + mnemlen;
    while (*rest && strchr(TXT_X86_64_BLANKS, *rest)) {
        rest++;
    }
    if (*rest) {
        char **parts = Str_Tokenize(rest, ",");
        for (char **iter = parts; *iter; iter++) {
            char *text = Str_Trim(*iter);
            if (! *text) {
                continue;
            }
            Err_Assert(nops < TXT_X86_64_OPERANDS_MAX, ERR_TXT_OPERANDS_TOO_MANY, line);
            const char *operand = text;
            if (*operand == '*') {
                opcode = Txt_x86_64_Att_IndirectOp(opcode);
                Err_Assert(opcode >= 0, ERR_TXT_OPERAND_NOT_INDIRECT, line);
                operand++;
            }
            Err_Assert(Txt_x86_64_Att_ParseOperand(operand, &ops[nops]), ERR_TXT_OPERAND_MALFORMED, text);
            nops++;
        }
        Str_FreeTokens(parts);
    }

    if (Txt_x86_64_Att_IsDirectBranch(opcode)) {
        Err_Assert(nops == 1, ERR_TXT_BRANCH_OPERAND_COUNT, line);
        Err_Assert(ops[0].ao_kind == ASM_X86_64_OPERAND_LABEL, ERR_TXT_BRANCH_NOT_LABEL, line);
    }

    Asm_x86_64_Item *item = Asm_x86_64_New(ASM_X86_64_ITEM_INSTR);
    item->ai_op = opcode;
    if (nops == TXT_X86_64_OPERANDS_MAX) {
        item->ai_src = ops[0];
        item->ai_dst = ops[1];
        if (ext_width) {
            item->ai_src.ao_width = ext_width;
        } else if (item->ai_src.ao_kind == ASM_X86_64_OPERAND_MEM && item->ai_dst.ao_kind == ASM_X86_64_OPERAND_REG) {
            item->ai_src.ao_width = item->ai_dst.ao_width;
        }
    } else if (nops == 1) {
        item->ai_dst = ops[0];
    }
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
    bool quoted = false;
    for (char *ptr = line; *ptr; ptr++) {
        if (*ptr == '"') {
            quoted = ! quoted;
        } else if (*ptr == '#' && ! quoted) {
            *ptr = '\0';
            break;
        }
    }

    char *text = Str_Trim(line);
    if (*text == '\0') {
        return;
    }

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
    } else {
        Txt_x86_64_Att_ParseInstr(text);
    }
}

// Parse AT&T-syntax assembly text into the instruction list.
void Txt_x86_64_Att_Parse(const char *text)
{
    Asm_x86_64_Reset();
    char **lines = Str_Tokenize(text, "\n");
    for (char **iter = lines; *iter; iter++) {
        Txt_x86_64_Att_ParseLine(*iter);
    }
    Str_FreeTokens(lines);
}
