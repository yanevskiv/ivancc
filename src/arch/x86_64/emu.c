// C source file for the x86-64 instruction-set emulator.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arch/x86_64/enc.h"
#include "arch/x86_64/emu.h"

// 64-bit register names, numbered as ModRM and REX number them.
static const char *Emu_x86_64_Name64[EMU_X86_64_REG_COUNT] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"
};

// 32-bit register names, in the same order.
static const char *Emu_x86_64_Name32[EMU_X86_64_REG_COUNT] = {
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};

// 16-bit register names.
static const char *Emu_x86_64_Name16[EMU_X86_64_REG_COUNT] = {
    "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",
    "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
};

// 8-bit register names, in the same order.
static const char *Emu_x86_64_Name8[EMU_X86_64_REG_COUNT] = {
    "al",  "cl",  "dl",  "bl",  "spl", "bpl", "sil", "dil",
    "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
};

// Return the name of a register at the given width.
const char *Emu_x86_64_RegName(int reg, int width)
{
    switch (width) {
        case EMU_X86_64_WIDTH_8: {
            return Emu_x86_64_Name8[reg & EMU_X86_64_REG_INDEX_MASK];
        } break;
        case EMU_X86_64_WIDTH_16: {
            return Emu_x86_64_Name16[reg & EMU_X86_64_REG_INDEX_MASK];
        } break;
        case EMU_X86_64_WIDTH_32: {
            return Emu_x86_64_Name32[reg & EMU_X86_64_REG_INDEX_MASK];
        } break;
        default: {
            return Emu_x86_64_Name64[reg & EMU_X86_64_REG_INDEX_MASK];
        }
    }
}

// Read a little-endian signed value of n bytes.
int64_t Emu_x86_64_ReadImm(const uint8_t *p, int n)
{
    uint64_t val = 0;
    for (int i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (EMU_X86_64_BITS_PER_BYTE * i);
    }
    int bits = EMU_X86_64_BITS_PER_BYTE * n;
    if (n < EMU_X86_64_IMM64 && (val >> (bits - 1)) & 1) {
        val |= EMU_X86_64_MASK_64 << bits;
    }
    return (int64_t) val;
}

// Return whether a one-byte opcode is followed by a ModRM byte.
int Emu_x86_64_HasModRM(int op)
{
    switch (op) {
        case ENC_X86_64_OPCODE_ADD_RM_R:
        case ENC_X86_64_OPCODE_OR_RM_R:
        case ENC_X86_64_OPCODE_AND_RM_R:
        case ENC_X86_64_OPCODE_SUB_RM_R:
        case ENC_X86_64_OPCODE_XOR_RM_R:
        case ENC_X86_64_OPCODE_CMP_RM_R:
        case ENC_X86_64_OPCODE_GRP2_RM_CL:
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32:
        case ENC_X86_64_OPCODE_MOV_RM8_R8:
        case ENC_X86_64_OPCODE_MOV_RM_R:
        case ENC_X86_64_OPCODE_MOV_R_RM:
        case ENC_X86_64_OPCODE_LEA_R_M:
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP3_RM:
        case ENC_X86_64_OPCODE_GRP5_RM: {
            return 1;
        } break;
        default: {
            return 0;
        }
    }
}

// Return whether a two-byte opcode is followed by a ModRM byte.
int Emu_x86_64_HasModRM2(int op2)
{
    switch (op2) {
        case ENC_X86_64_OPCODE2_SETE:
        case ENC_X86_64_OPCODE2_SETNE:
        case ENC_X86_64_OPCODE2_SETL:
        case ENC_X86_64_OPCODE2_SETLE:
        case ENC_X86_64_OPCODE2_SETB:
        case ENC_X86_64_OPCODE2_SETBE:
        case ENC_X86_64_OPCODE2_MOVZX_R_RM16:
        case ENC_X86_64_OPCODE2_MOVSX_R_RM16:
        case ENC_X86_64_OPCODE2_IMUL_R_RM:
        case ENC_X86_64_OPCODE2_MOVZX_R_RM8:
        case ENC_X86_64_OPCODE2_MOVSX_R_RM8: {
            return 1;
        } break;
        default: {
            return 0;
        }
    }
}

// Decode the ModRM byte at p and return the bytes consumed.
int Emu_x86_64_DecodeModRM(const uint8_t *p, int avail, int rex, Emu_x86_64_Insn *insn)
{
    if (avail < 1) {
        return 0;
    }
    int modrm = p[0];
    int mod   = modrm >> ENC_X86_64_MOD_SHIFT;
    int rm    = modrm & ENC_X86_64_REG_MASK;
    int n     = 1;

    insn->ei_reg = ((modrm >> ENC_X86_64_REG_SHIFT) & ENC_X86_64_REG_MASK)
                 | ((rex & ENC_X86_64_REX_R) ? EMU_X86_64_REG_HIGH_BIT : 0);

    if (mod == ENC_X86_64_MOD_DIRECT) {
        insn->ei_rmkind = EMU_X86_64_RM_REG;
        insn->ei_rm     = rm | ((rex & ENC_X86_64_REX_B) ? EMU_X86_64_REG_HIGH_BIT : 0);
        return n;
    }

    int base = rm;
    if (rm == (ENC_X86_64_SIB_BASE_RSP & ENC_X86_64_REG_MASK)) {
        if (avail < n + 1) {
            return 0;
        }
        base = p[n] & ENC_X86_64_REG_MASK;
        n++;
    }

    if (mod == ENC_X86_64_MOD_INDIRECT && rm == ENC_X86_64_RM_RIP) {
        if (avail < n + EMU_X86_64_IMM32) {
            return 0;
        }
        insn->ei_rmkind = EMU_X86_64_RM_RIP;
        insn->ei_disp   = (int32_t) Emu_x86_64_ReadImm(p + n, EMU_X86_64_IMM32);
        return n + EMU_X86_64_IMM32;
    }

    insn->ei_rmkind = EMU_X86_64_RM_MEM;
    insn->ei_rm     = base | ((rex & ENC_X86_64_REX_B) ? EMU_X86_64_REG_HIGH_BIT : 0);

    if (mod == ENC_X86_64_MOD_DISP8) {
        if (avail < n + 1) {
            return 0;
        }
        insn->ei_disp = (int32_t) Emu_x86_64_ReadImm(p + n, EMU_X86_64_IMM8);
        n += EMU_X86_64_IMM8;
    } else if (mod == ENC_X86_64_MOD_DISP32) {
        if (avail < n + EMU_X86_64_IMM32) {
            return 0;
        }
        insn->ei_disp = (int32_t) Emu_x86_64_ReadImm(p + n, EMU_X86_64_IMM32);
        n += EMU_X86_64_IMM32;
    }
    return n;
}

// Return the operand width an instruction selects.
int Emu_x86_64_Width(const Emu_x86_64_Insn *insn)
{
    if (insn->ei_rexw) {
        return EMU_X86_64_WIDTH_64;
    }
    return insn->ei_opsize16 ? EMU_X86_64_WIDTH_16 : EMU_X86_64_WIDTH_32;
}

// Decode one instruction, returning its length or 0; avail bounds the read.
int Emu_x86_64_Decode(const uint8_t *code, int avail, Emu_x86_64_Insn *insn)
{
    memset(insn, 0, sizeof(*insn));
    insn->ei_op2    = EMU_X86_64_NO_OPCODE2;
    insn->ei_rmkind = EMU_X86_64_RM_NONE;

    int n   = 0;
    int rex = 0;
    if (avail > 0 && code[0] == ENC_X86_64_OPCODE_OPSIZE) {
        insn->ei_opsize16 = 1;
        n++;
    }
    if (avail > n && (code[n] & EMU_X86_64_REX_PREFIX_MASK) == ENC_X86_64_REX_BASE) {
        rex = code[n++];
        insn->ei_rexw = (rex & ENC_X86_64_REX_W) != 0;
    }
    if (avail < n + 1) {
        return 0;
    }

    int op = code[n++];
    insn->ei_op = op;

    if (op == ENC_X86_64_OPCODE_ESCAPE) {
        if (avail < n + 1) {
            return 0;
        }
        int op2 = code[n++];
        insn->ei_op2 = op2;
        if (Emu_x86_64_HasModRM2(op2)) {
            int used = Emu_x86_64_DecodeModRM(code + n, avail - n, rex, insn);
            if (! used) {
                return 0;
            }
            n += used;
        } else if (op2 == ENC_X86_64_OPCODE2_JE_REL32 || op2 == ENC_X86_64_OPCODE2_JNE_REL32) {
            if (avail < n + EMU_X86_64_IMM32) {
                return 0;
            }
            insn->ei_imm = Emu_x86_64_ReadImm(code + n, EMU_X86_64_IMM32);
            n += EMU_X86_64_IMM32;
        } else if (op2 != ENC_X86_64_OPCODE2_SYSCALL) {
            return 0;
        }
        insn->ei_len = n;
        return n;
    }

    if ((op & EMU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_PUSH_R || (op & EMU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_POP_R) {
        insn->ei_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? EMU_X86_64_REG_HIGH_BIT : 0);
        insn->ei_op     = op & EMU_X86_64_OPCODE_REG_MASK;
        insn->ei_rmkind = EMU_X86_64_RM_REG;
        insn->ei_len    = n;
        return n;
    }
    if ((op & EMU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R8_IMM8 || (op & EMU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R_IMM64) {
        int wide = (op & EMU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R_IMM64;
        int size = wide ? (insn->ei_rexw ? EMU_X86_64_IMM64 : EMU_X86_64_IMM32) : EMU_X86_64_IMM8;
        if (avail < n + size) {
            return 0;
        }
        insn->ei_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? EMU_X86_64_REG_HIGH_BIT : 0);
        insn->ei_op     = op & EMU_X86_64_OPCODE_REG_MASK;
        insn->ei_rmkind = EMU_X86_64_RM_REG;
        insn->ei_imm    = Emu_x86_64_ReadImm(code + n, size);
        insn->ei_len    = n + size;
        return insn->ei_len;
    }

    if (Emu_x86_64_HasModRM(op)) {
        int used = Emu_x86_64_DecodeModRM(code + n, avail - n, rex, insn);
        if (! used) {
            return 0;
        }
        n += used;
        if (op == ENC_X86_64_OPCODE_MOV_RM_IMM32 || op == ENC_X86_64_OPCODE_GRP1_RM_IMM32) {
            if (avail < n + EMU_X86_64_IMM32) {
                return 0;
            }
            insn->ei_imm = Emu_x86_64_ReadImm(code + n, EMU_X86_64_IMM32);
            n += EMU_X86_64_IMM32;
        }
        insn->ei_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_CALL_REL32 || op == ENC_X86_64_OPCODE_JMP_REL32) {
        if (avail < n + EMU_X86_64_IMM32) {
            return 0;
        }
        insn->ei_imm = Emu_x86_64_ReadImm(code + n, EMU_X86_64_IMM32);
        n += EMU_X86_64_IMM32;
        insn->ei_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_RET || op == ENC_X86_64_OPCODE_CQO) {
        insn->ei_len = n;
        return n;
    }
    return 0;
}

// Name the operation a decoded instruction performs.
const char *Emu_x86_64_Mnemonic(const Emu_x86_64_Insn *insn)
{
    if (insn->ei_op == ENC_X86_64_OPCODE_ESCAPE) {
        switch (insn->ei_op2) {
            case ENC_X86_64_OPCODE2_SYSCALL:     { return "syscall"; } break;
            case ENC_X86_64_OPCODE2_JE_REL32:    { return "je";      } break;
            case ENC_X86_64_OPCODE2_JNE_REL32:   { return "jne";     } break;
            case ENC_X86_64_OPCODE2_SETE:        { return "sete";    } break;
            case ENC_X86_64_OPCODE2_SETNE:       { return "setne";   } break;
            case ENC_X86_64_OPCODE2_SETL:        { return "setl";    } break;
            case ENC_X86_64_OPCODE2_SETLE:       { return "setle";   } break;
            case ENC_X86_64_OPCODE2_SETB:        { return "setb";    } break;
            case ENC_X86_64_OPCODE2_SETBE:       { return "setbe";   } break;
            case ENC_X86_64_OPCODE2_IMUL_R_RM:   { return "imul";    } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8:  { return "movzbq"; } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM16: { return "movzwq"; } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8:  { return "movsbq"; } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM16: { return "movswq"; } break;
            default:                             { return "(bad)";   }
        }
    }

    switch (insn->ei_op) {
        case ENC_X86_64_OPCODE_ADD_RM_R:      { return "add";   } break;
        case ENC_X86_64_OPCODE_OR_RM_R:       { return "or";    } break;
        case ENC_X86_64_OPCODE_AND_RM_R:      { return "and";   } break;
        case ENC_X86_64_OPCODE_XOR_RM_R:      { return "xor";   } break;
        case ENC_X86_64_OPCODE_SUB_RM_R:      { return "sub";   } break;
        case ENC_X86_64_OPCODE_CMP_RM_R:      { return "cmp";   } break;
        case ENC_X86_64_OPCODE_PUSH_R:        { return "push";  } break;
        case ENC_X86_64_OPCODE_POP_R:         { return "pop";   } break;
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32: { return "movslq"; } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8:
        case ENC_X86_64_OPCODE_MOV_RM_R:
        case ENC_X86_64_OPCODE_MOV_R_RM:
        case ENC_X86_64_OPCODE_MOV_R8_IMM8:
        case ENC_X86_64_OPCODE_MOV_R_IMM64:
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:  { return "mov";   } break;
        case ENC_X86_64_OPCODE_LEA_R_M:       { return "lea";   } break;
        case ENC_X86_64_OPCODE_CQO:           { return "cqto";  } break;
        case ENC_X86_64_OPCODE_RET:           { return "ret";   } break;
        case ENC_X86_64_OPCODE_CALL_REL32:    { return "call";  } break;
        case ENC_X86_64_OPCODE_JMP_REL32:     { return "jmp";   } break;
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            switch (insn->ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_ADD: { return "add"; } break;
                case ENC_X86_64_GRP_SUB: { return "sub"; } break;
                case ENC_X86_64_GRP_CMP: { return "cmp"; } break;
                default:                 { return "(bad)"; }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            switch (insn->ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_SHL: { return "shl"; } break;
                case ENC_X86_64_GRP_SAR: { return "sar"; } break;
                case ENC_X86_64_GRP_SHR: { return "shr"; } break;
                default:                 { return "(bad)"; }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            switch (insn->ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_CALL: { return "call"; } break;
                default:                  { return "(bad)"; }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn->ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NOT:  { return "not";  } break;
                case ENC_X86_64_GRP_NEG:  { return "neg";  } break;
                case ENC_X86_64_GRP_IDIV: { return "idiv"; } break;
                case ENC_X86_64_GRP_DIV:  { return "div";  } break;
                default:                  { return "(bad)"; }
            }
        } break;
        default: { return "(bad)"; }
    }
}

// Write the r/m operand into out, as a register, disp(%base) or disp(%rip).
void Emu_x86_64_FormatRm(const Emu_x86_64_Insn *insn, int width, uint64_t next, char *out, int n)
{
    switch (insn->ei_rmkind) {
        case EMU_X86_64_RM_REG: {
            snprintf(out, n, "%%%s", Emu_x86_64_RegName(insn->ei_rm, width));
        } break;
        case EMU_X86_64_RM_MEM: {
            const char *base = Emu_x86_64_RegName(insn->ei_rm, EMU_X86_64_WIDTH_64);
            if (insn->ei_disp) {
                snprintf(out, n, "%d(%%%s)", insn->ei_disp, base);
            } else {
                snprintf(out, n, "(%%%s)", base);
            }
        } break;
        case EMU_X86_64_RM_RIP: {
            snprintf(out, n, "0x%llx(%%rip)", (unsigned long long) (next + (int64_t) insn->ei_disp));
        } break;
        default: {
            snprintf(out, n, "?");
        }
    }
}

// Write one decoded instruction in AT&T syntax, with rip the address it sits at.
void Emu_x86_64_Format(const Emu_x86_64_Insn *insn, uint64_t rip, char *out, int n)
{
    const char *name = Emu_x86_64_Mnemonic(insn);
    uint64_t next = rip + insn->ei_len;
    int width = Emu_x86_64_Width(insn);
    char rm[64];

    if (insn->ei_op == ENC_X86_64_OPCODE_CALL_REL32 || insn->ei_op == ENC_X86_64_OPCODE_JMP_REL32
        || insn->ei_op2 == ENC_X86_64_OPCODE2_JE_REL32 || insn->ei_op2 == ENC_X86_64_OPCODE2_JNE_REL32) {
        snprintf(out, n, "%s 0x%llx", name, (unsigned long long) (next + insn->ei_imm));
        return;
    }
    if (insn->ei_rmkind == EMU_X86_64_RM_NONE) {
        snprintf(out, n, "%s", name);
        return;
    }

    switch (insn->ei_op) {
        case ENC_X86_64_OPCODE_PUSH_R:
        case ENC_X86_64_OPCODE_POP_R: {
            snprintf(out, n, "%s %%%s", name, Emu_x86_64_RegName(insn->ei_rm, EMU_X86_64_WIDTH_64));
        } break;
        case ENC_X86_64_OPCODE_MOV_R8_IMM8: {
            snprintf(out, n, "%s $0x%llx, %%%s", name, (unsigned long long) insn->ei_imm, Emu_x86_64_RegName(insn->ei_rm, EMU_X86_64_WIDTH_8));
        } break;
        case ENC_X86_64_OPCODE_MOV_R_IMM64: {
            snprintf(out, n, "%s $0x%llx, %%%s", name, (unsigned long long) insn->ei_imm, Emu_x86_64_RegName(insn->ei_rm, width));
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            Emu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s $0x%llx, %s", name, (unsigned long long) insn->ei_imm, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            Emu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            Emu_x86_64_FormatRm(insn, EMU_X86_64_WIDTH_64, next, rm, sizeof(rm));
            snprintf(out, n, "%s *%s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            Emu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%cl, %s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8: {
            Emu_x86_64_FormatRm(insn, EMU_X86_64_WIDTH_8, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Emu_x86_64_RegName(insn->ei_reg, EMU_X86_64_WIDTH_8), rm);
        } break;
        case ENC_X86_64_OPCODE_ADD_RM_R:
        case ENC_X86_64_OPCODE_OR_RM_R:
        case ENC_X86_64_OPCODE_AND_RM_R:
        case ENC_X86_64_OPCODE_SUB_RM_R:
        case ENC_X86_64_OPCODE_XOR_RM_R:
        case ENC_X86_64_OPCODE_CMP_RM_R:
        case ENC_X86_64_OPCODE_MOV_RM_R: {
            Emu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Emu_x86_64_RegName(insn->ei_reg, width), rm);
        } break;
        default: {
            int srcw = width;
            if (insn->ei_op2 == ENC_X86_64_OPCODE2_MOVZX_R_RM8 || insn->ei_op2 == ENC_X86_64_OPCODE2_MOVSX_R_RM8) {
                srcw = EMU_X86_64_WIDTH_8;
            } else if (insn->ei_op2 == ENC_X86_64_OPCODE2_MOVZX_R_RM16 || insn->ei_op2 == ENC_X86_64_OPCODE2_MOVSX_R_RM16) {
                srcw = EMU_X86_64_WIDTH_16;
            } else if (insn->ei_op == ENC_X86_64_OPCODE_MOVSXD_R_RM32) {
                srcw = EMU_X86_64_WIDTH_32;
            }
            if (insn->ei_op2 >= ENC_X86_64_OPCODE2_SETB && insn->ei_op2 <= ENC_X86_64_OPCODE2_SETLE) {
                Emu_x86_64_FormatRm(insn, EMU_X86_64_WIDTH_8, next, rm, sizeof(rm));
                snprintf(out, n, "%s %s", name, rm);
                return;
            }
            Emu_x86_64_FormatRm(insn, srcw, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s, %%%s", name, rm, Emu_x86_64_RegName(insn->ei_reg, width));
        }
    }
}

// Start a program: entry in %rip, the image's stack in %rsp, everything else zero.
void Emu_x86_64_Init(Emu_x86_64_Cpu *cpu, const Elf_LoadImage *img)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu->ec_img = img;
    cpu->ec_rip = img->li_entry;
    cpu->ec_reg[EMU_X86_64_REG_RSP] = img->li_stack;
}

// Report a fault against the instruction that caused it and stop the program.
void Emu_x86_64_Fault(Emu_x86_64_Cpu *cpu, const char *what, uint64_t addr)
{
    fprintf(stderr, "ivanemu: %s at 0x%llx from %%rip = 0x%llx\n", what,
            (unsigned long long) addr, (unsigned long long) cpu->ec_rip);
    cpu->ec_halted = 1;
    cpu->ec_status = EMU_X86_64_STATUS_FAULT;
}

// Read a register at the given width.
uint64_t Emu_x86_64_ReadReg(const Emu_x86_64_Cpu *cpu, int reg, int width)
{
    uint64_t val = cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK];
    switch (width) {
        case EMU_X86_64_WIDTH_8:  { return val & EMU_X86_64_MASK_8; } break;
        case EMU_X86_64_WIDTH_16: { return val & EMU_X86_64_MASK_16; } break;
        case EMU_X86_64_WIDTH_32: { return val & EMU_X86_64_MASK_32; } break;
        default:                  { return val; }
    }
}

// Write a register the way the hardware does, zero-extending a 32-bit result.
void Emu_x86_64_WriteReg(Emu_x86_64_Cpu *cpu, int reg, uint64_t value, int width)
{
    switch (width) {
        case EMU_X86_64_WIDTH_8: {
            cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] = (cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] & ~(uint64_t) EMU_X86_64_MASK_8) | (value & EMU_X86_64_MASK_8);
        } break;
        case EMU_X86_64_WIDTH_16: {
            cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] = (cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] & ~(uint64_t) EMU_X86_64_MASK_16) | (value & EMU_X86_64_MASK_16);
        } break;
        case EMU_X86_64_WIDTH_32: {
            cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] = value & EMU_X86_64_MASK_32;
        } break;
        default: {
            cpu->ec_reg[reg & EMU_X86_64_REG_INDEX_MASK] = value;
        }
    }
}

// Return whether an address names a device register rather than memory.
int Emu_x86_64_IsDevice(uint64_t addr)
{
    return addr >= EMU_X86_64_DEV_BASE && addr < EMU_X86_64_DEV_BASE + EMU_X86_64_DEV_SIZE;
}

// Read a device register, the UART being write-only and always ready.
uint64_t Emu_x86_64_ReadDev(Emu_x86_64_Cpu *cpu, uint64_t addr)
{
    (void) cpu;
    return addr == EMU_X86_64_DEV_STATUS ? 1 : 0;
}

// Write a device register: a byte to the terminal, or the status to stop with.
void Emu_x86_64_WriteDev(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value)
{
    switch (addr) {
        case EMU_X86_64_DEV_DATA: {
            uint8_t byte = value & EMU_X86_64_MASK_8;
            write(EMU_X86_64_UART_FD, &byte, sizeof(byte));
        } break;
        case EMU_X86_64_DEV_HALT: {
            cpu->ec_halted = 1;
            cpu->ec_status = value & EMU_X86_64_MASK_8;
        } break;
        default: {
            // the status register is read-only, and the rest is unassigned
        } break;
    }
}

// Read width bits from the image, faulting if that address is not mapped.
uint64_t Emu_x86_64_ReadMem(Emu_x86_64_Cpu *cpu, uint64_t addr, int width)
{
    if (Emu_x86_64_IsDevice(addr)) {
        return Emu_x86_64_ReadDev(cpu, addr);
    }

    int n = width / EMU_X86_64_BITS_PER_BYTE;
    const uint8_t *p = Elf_Load_At(cpu->ec_img, addr, n);
    if (! p) {
        Emu_x86_64_Fault(cpu, "read of unmapped memory", addr);
        return 0;
    }
    uint64_t val = 0;
    for (int i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (EMU_X86_64_BITS_PER_BYTE * i);
    }
    return val;
}

// Write width bits into the image, faulting if that address is not mapped.
void Emu_x86_64_WriteMem(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value, int width)
{
    if (Emu_x86_64_IsDevice(addr)) {
        Emu_x86_64_WriteDev(cpu, addr, value);
        return;
    }

    int n = width / EMU_X86_64_BITS_PER_BYTE;
    uint8_t *p = Elf_Load_At(cpu->ec_img, addr, n);
    if (! p) {
        Emu_x86_64_Fault(cpu, "write to unmapped memory", addr);
        return;
    }
    for (int i = 0; i < n; i++) {
        p[i] = (value >> (EMU_X86_64_BITS_PER_BYTE * i)) & EMU_X86_64_MASK_8;
    }
}

// Compute the address an instruction's memory operand names.
uint64_t Emu_x86_64_RmAddr(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next)
{
    if (insn->ei_rmkind == EMU_X86_64_RM_RIP) {
        return next + (int64_t) insn->ei_disp;
    }
    return cpu->ec_reg[insn->ei_rm & EMU_X86_64_REG_INDEX_MASK] + (int64_t) insn->ei_disp;
}

// Read an instruction's r/m operand, wherever it lives.
uint64_t Emu_x86_64_ReadRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, int width)
{
    if (insn->ei_rmkind == EMU_X86_64_RM_REG) {
        return Emu_x86_64_ReadReg(cpu, insn->ei_rm, width);
    }
    return Emu_x86_64_ReadMem(cpu, Emu_x86_64_RmAddr(cpu, insn, next), width);
}

// Write an instruction's r/m operand, wherever it lives.
void Emu_x86_64_WriteRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, uint64_t value, int width)
{
    if (insn->ei_rmkind == EMU_X86_64_RM_REG) {
        Emu_x86_64_WriteReg(cpu, insn->ei_rm, value, width);
        return;
    }
    Emu_x86_64_WriteMem(cpu, Emu_x86_64_RmAddr(cpu, insn, next), value, width);
}

// Set the flags a - b leaves behind.
void Emu_x86_64_FlagsSub(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, int width)
{
    uint64_t mask = EMU_X86_64_MASK_64 >> (EMU_X86_64_WIDTH_64 - width);
    int sign = width - 1;
    a &= mask;
    b &= mask;
    uint64_t res = (a - b) & mask;

    cpu->ec_zf = res == 0;
    cpu->ec_sf = (res >> sign) & 1;
    cpu->ec_cf = a < b;
    cpu->ec_of = (((a ^ b) & (a ^ res)) >> sign) & 1;
}

// Set the flags a + b leaves behind.
void Emu_x86_64_FlagsAdd(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, int width)
{
    uint64_t mask = EMU_X86_64_MASK_64 >> (EMU_X86_64_WIDTH_64 - width);
    int sign = width - 1;
    a &= mask;
    b &= mask;
    uint64_t res = (a + b) & mask;

    cpu->ec_zf = res == 0;
    cpu->ec_sf = (res >> sign) & 1;
    cpu->ec_cf = res < a;
    cpu->ec_of = ((~(a ^ b) & (a ^ res)) >> sign) & 1;
}

// Answer a syscall: the two the runtime makes, and nothing else.
void Emu_x86_64_Syscall(Emu_x86_64_Cpu *cpu)
{
    uint64_t nr = cpu->ec_reg[EMU_X86_64_REG_RAX];
    switch (nr) {
        case EMU_X86_64_SYS_WRITE: {
            uint64_t fd  = cpu->ec_reg[EMU_X86_64_REG_RDI];
            uint64_t buf = cpu->ec_reg[EMU_X86_64_REG_RSI];
            uint64_t len = cpu->ec_reg[EMU_X86_64_REG_RDX];
            const uint8_t *p = Elf_Load_At(cpu->ec_img, buf, len);
            if (! p) {
                Emu_x86_64_Fault(cpu, "write from unmapped memory", buf);
                return;
            }
            ssize_t n = write((int) fd, p, (size_t) len);
            cpu->ec_reg[EMU_X86_64_REG_RAX] = n < 0 ? EMU_X86_64_MASK_64 : (uint64_t) n;
        } break;
        case EMU_X86_64_SYS_EXIT: {
            cpu->ec_halted = 1;
            cpu->ec_status = cpu->ec_reg[EMU_X86_64_REG_RDI] & EMU_X86_64_MASK_8;
        } break;
        default: {
            fprintf(stderr, "ivanemu: unimplemented syscall %llu from %%rip = 0x%llx\n",
                    (unsigned long long) nr, (unsigned long long) cpu->ec_rip);
            cpu->ec_halted = 1;
            cpu->ec_status = EMU_X86_64_STATUS_FAULT;
        }
    }
}

// Execute the instruction at %rip and leave %rip on the next one.
void Emu_x86_64_Step(Emu_x86_64_Cpu *cpu, int trace)
{
    uint64_t rip = cpu->ec_rip;
    int avail = (int) (cpu->ec_img->li_base + cpu->ec_img->li_size - rip);
    const uint8_t *code = Elf_Load_At(cpu->ec_img, rip, sizeof(*code));
    Emu_x86_64_Insn insn;

    if (! code || ! Emu_x86_64_Decode(code, avail, &insn)) {
        Emu_x86_64_Fault(cpu, "undecodable instruction", rip);
        return;
    }
    if (trace) {
        char text[128];
        Emu_x86_64_Format(&insn, rip, text, sizeof(text));
        fprintf(stderr, "%016llx: %s\n", (unsigned long long) rip, text);
    }

    uint64_t next = rip + insn.ei_len;
    int width = Emu_x86_64_Width(&insn);
    uint64_t *rsp = &cpu->ec_reg[EMU_X86_64_REG_RSP];
    cpu->ec_rip = next;

    if (insn.ei_op == ENC_X86_64_OPCODE_ESCAPE) {
        switch (insn.ei_op2) {
            case ENC_X86_64_OPCODE2_SYSCALL: {
                Emu_x86_64_Syscall(cpu);
            } break;
            case ENC_X86_64_OPCODE2_JE_REL32: {
                if (cpu->ec_zf) {
                    cpu->ec_rip = next + insn.ei_imm;
                }
            } break;
            case ENC_X86_64_OPCODE2_JNE_REL32: {
                if (! cpu->ec_zf) {
                    cpu->ec_rip = next + insn.ei_imm;
                }
            } break;
            case ENC_X86_64_OPCODE2_SETE: {
                Emu_x86_64_WriteRm(cpu, &insn, next, cpu->ec_zf, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETNE: {
                Emu_x86_64_WriteRm(cpu, &insn, next, ! cpu->ec_zf, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETL: {
                Emu_x86_64_WriteRm(cpu, &insn, next, cpu->ec_sf != cpu->ec_of, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETLE: {
                Emu_x86_64_WriteRm(cpu, &insn, next, cpu->ec_zf || cpu->ec_sf != cpu->ec_of, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETB: {
                Emu_x86_64_WriteRm(cpu, &insn, next, cpu->ec_cf, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETBE: {
                Emu_x86_64_WriteRm(cpu, &insn, next, cpu->ec_cf || cpu->ec_zf, EMU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_IMUL_R_RM: {
                uint64_t a = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, a * b, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_8);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, b & EMU_X86_64_MASK_8, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_8);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, (uint64_t) (int64_t) (int8_t) b, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM16: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_16);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, b & EMU_X86_64_MASK_16, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM16: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_16);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, (uint64_t) (int64_t) (int16_t) b, width);
            } break;
            default: {
                Emu_x86_64_Fault(cpu, "unimplemented two-byte opcode", rip);
            }
        }
        return;
    }

    switch (insn.ei_op) {
        case ENC_X86_64_OPCODE_ADD_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_FlagsAdd(cpu, a, b, width);
            Emu_x86_64_WriteRm(cpu, &insn, next, a + b, width);
        } break;
        case ENC_X86_64_OPCODE_SUB_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_FlagsSub(cpu, a, b, width);
            Emu_x86_64_WriteRm(cpu, &insn, next, a - b, width);
        } break;
        case ENC_X86_64_OPCODE_CMP_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_FlagsSub(cpu, a, b, width);
        } break;
        case ENC_X86_64_OPCODE_AND_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_WriteRm(cpu, &insn, next, a & b, width);
        } break;
        case ENC_X86_64_OPCODE_OR_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_WriteRm(cpu, &insn, next, a | b, width);
        } break;
        case ENC_X86_64_OPCODE_XOR_RM_R: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
            Emu_x86_64_WriteRm(cpu, &insn, next, a ^ b, width);
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            int count = cpu->ec_reg[EMU_X86_64_REG_RCX] & (width == EMU_X86_64_WIDTH_64 ? EMU_X86_64_SHIFT_MASK_64 : EMU_X86_64_SHIFT_MASK_32);
            switch (insn.ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_SHL: {
                    Emu_x86_64_WriteRm(cpu, &insn, next, a << count, width);
                } break;
                case ENC_X86_64_GRP_SAR: {
                    int64_t signed_a = width == EMU_X86_64_WIDTH_64 ? (int64_t) a : (int32_t) a;
                    Emu_x86_64_WriteRm(cpu, &insn, next, (uint64_t) (signed_a >> count), width);
                } break;
                case ENC_X86_64_GRP_SHR: {
                    Emu_x86_64_WriteRm(cpu, &insn, next, a >> count, width);
                } break;
                default: {
                    Emu_x86_64_Fault(cpu, "unimplemented group 2 opcode", rip);
                }
            }
        } break;
        case ENC_X86_64_OPCODE_PUSH_R: {
            *rsp -= EMU_X86_64_STACK_SLOT;
            Emu_x86_64_WriteMem(cpu, *rsp, cpu->ec_reg[insn.ei_rm & EMU_X86_64_REG_INDEX_MASK], EMU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_POP_R: {
            uint64_t val = Emu_x86_64_ReadMem(cpu, *rsp, EMU_X86_64_WIDTH_64);
            *rsp += EMU_X86_64_STACK_SLOT;
            cpu->ec_reg[insn.ei_rm & EMU_X86_64_REG_INDEX_MASK] = val;
        } break;
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32: {
            uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_32);
            Emu_x86_64_WriteReg(cpu, insn.ei_reg, (uint64_t) (int64_t) (int32_t) b, EMU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8: {
            Emu_x86_64_WriteRm(cpu, &insn, next, Emu_x86_64_ReadReg(cpu, insn.ei_reg, EMU_X86_64_WIDTH_8), EMU_X86_64_WIDTH_8);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_R: {
            Emu_x86_64_WriteRm(cpu, &insn, next, Emu_x86_64_ReadReg(cpu, insn.ei_reg, width), width);
        } break;
        case ENC_X86_64_OPCODE_MOV_R_RM: {
            Emu_x86_64_WriteReg(cpu, insn.ei_reg, Emu_x86_64_ReadRm(cpu, &insn, next, width), width);
        } break;
        case ENC_X86_64_OPCODE_LEA_R_M: {
            Emu_x86_64_WriteReg(cpu, insn.ei_reg, Emu_x86_64_RmAddr(cpu, &insn, next), width);
        } break;
        case ENC_X86_64_OPCODE_CQO: {
            int64_t rax = (int64_t) cpu->ec_reg[EMU_X86_64_REG_RAX];
            cpu->ec_reg[EMU_X86_64_REG_RDX] = rax < 0 ? EMU_X86_64_MASK_64 : 0;
        } break;
        case ENC_X86_64_OPCODE_MOV_R8_IMM8: {
            Emu_x86_64_WriteReg(cpu, insn.ei_rm, insn.ei_imm, EMU_X86_64_WIDTH_8);
        } break;
        case ENC_X86_64_OPCODE_MOV_R_IMM64: {
            Emu_x86_64_WriteReg(cpu, insn.ei_rm, insn.ei_imm, width);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_IMM32: {
            Emu_x86_64_WriteRm(cpu, &insn, next, insn.ei_imm, width);
        } break;
        case ENC_X86_64_OPCODE_RET: {
            cpu->ec_rip = Emu_x86_64_ReadMem(cpu, *rsp, EMU_X86_64_WIDTH_64);
            *rsp += EMU_X86_64_STACK_SLOT;
        } break;
        case ENC_X86_64_OPCODE_CALL_REL32: {
            *rsp -= EMU_X86_64_STACK_SLOT;
            Emu_x86_64_WriteMem(cpu, *rsp, next, EMU_X86_64_WIDTH_64);
            cpu->ec_rip = next + insn.ei_imm;
        } break;
        case ENC_X86_64_OPCODE_JMP_REL32: {
            cpu->ec_rip = next + insn.ei_imm;
        } break;
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = (uint64_t) insn.ei_imm;
            switch (insn.ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_ADD: {
                    Emu_x86_64_FlagsAdd(cpu, a, b, width);
                    Emu_x86_64_WriteRm(cpu, &insn, next, a + b, width);
                } break;
                case ENC_X86_64_GRP_SUB: {
                    Emu_x86_64_FlagsSub(cpu, a, b, width);
                    Emu_x86_64_WriteRm(cpu, &insn, next, a - b, width);
                } break;
                case ENC_X86_64_GRP_CMP: {
                    Emu_x86_64_FlagsSub(cpu, a, b, width);
                } break;
                default: {
                    Emu_x86_64_Fault(cpu, "unimplemented group 1 opcode", rip);
                }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            if ((insn.ei_reg & ENC_X86_64_REG_MASK) != ENC_X86_64_GRP_CALL) {
                Emu_x86_64_Fault(cpu, "unimplemented group 5 opcode", rip);
                return;
            }
            uint64_t target = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_64);
            *rsp -= EMU_X86_64_STACK_SLOT;
            Emu_x86_64_WriteMem(cpu, *rsp, next, EMU_X86_64_WIDTH_64);
            cpu->ec_rip = target;
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn.ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NEG: {
                    uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    Emu_x86_64_FlagsSub(cpu, 0, a, width);
                    Emu_x86_64_WriteRm(cpu, &insn, next, 0 - a, width);
                } break;
                case ENC_X86_64_GRP_NOT: {
                    uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    Emu_x86_64_WriteRm(cpu, &insn, next, ~a, width);
                } break;
                case ENC_X86_64_GRP_IDIV: {
                    int64_t d = (int64_t) Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    if (d == 0) {
                        Emu_x86_64_Fault(cpu, "divide by zero", rip);
                        return;
                    }
                    __int128 num = ((__int128) (int64_t) cpu->ec_reg[EMU_X86_64_REG_RDX] << EMU_X86_64_WIDTH_64)
                                 | cpu->ec_reg[EMU_X86_64_REG_RAX];
                    cpu->ec_reg[EMU_X86_64_REG_RAX] = (uint64_t) (int64_t) (num / d);
                    cpu->ec_reg[EMU_X86_64_REG_RDX] = (uint64_t) (int64_t) (num % d);
                } break;
                case ENC_X86_64_GRP_DIV: {
                    uint64_t d = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    if (d == 0) {
                        Emu_x86_64_Fault(cpu, "divide by zero", rip);
                        return;
                    }
                    unsigned __int128 num = ((unsigned __int128) cpu->ec_reg[EMU_X86_64_REG_RDX] << EMU_X86_64_WIDTH_64)
                                          | cpu->ec_reg[EMU_X86_64_REG_RAX];
                    cpu->ec_reg[EMU_X86_64_REG_RAX] = (uint64_t) (num / d);
                    cpu->ec_reg[EMU_X86_64_REG_RDX] = (uint64_t) (num % d);
                } break;
                default: {
                    Emu_x86_64_Fault(cpu, "unimplemented group 3 opcode", rip);
                }
            }
        } break;
        default: {
            Emu_x86_64_Fault(cpu, "unimplemented opcode", rip);
        }
    }
}

// Run a loaded program to completion and return the status it stopped with.
int Emu_x86_64_Run(const Elf_LoadImage *img, int trace)
{
    Emu_x86_64_Cpu cpu;
    Emu_x86_64_Init(&cpu, img);
    while (! cpu.ec_halted) {
        Emu_x86_64_Step(&cpu, trace);
    }
    return cpu.ec_status;
}
