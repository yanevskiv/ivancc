#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arch/x86_64/enc.h"
#include "arch/x86_64/emu.h"

// 64-bit register names, numbered as ModRM and REX number them.
static const char *Emu_x86_64_Name64[16] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"
};

// 32-bit register names, in the same order.
static const char *Emu_x86_64_Name32[16] = {
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};

// 8-bit register names, in the same order.
static const char *Emu_x86_64_Name8[16] = {
    "al",  "cl",  "dl",  "bl",  "spl", "bpl", "sil", "dil",
    "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
};

// Return the name of a register at the given width.
const char *Emu_x86_64_RegName(int reg, int width)
{
    switch (width) {
        case EMU_X86_64_WIDTH_8: {
            return Emu_x86_64_Name8[reg & 15];
        } break;
        case EMU_X86_64_WIDTH_32: {
            return Emu_x86_64_Name32[reg & 15];
        } break;
        default: {
            return Emu_x86_64_Name64[reg & 15];
        }
    }
}

// Read a little-endian signed value of n bytes.
int64_t Emu_x86_64_ReadImm(const uint8_t *p, int n)
{
    uint64_t val = 0;
    for (int i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (8 * i);
    }
    if (n < 8 && (val >> (8 * n - 1)) & 1) {
        val |= ~(uint64_t) 0 << (8 * n);
    }
    return (int64_t) val;
}

// Return whether a one-byte opcode is followed by a ModRM byte.
int Emu_x86_64_HasModRM(int op)
{
    switch (op) {
        case ENC_X86_64_OPCODE_ADD_RM_R:
        case ENC_X86_64_OPCODE_SUB_RM_R:
        case ENC_X86_64_OPCODE_CMP_RM_R:
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32:
        case ENC_X86_64_OPCODE_MOV_RM8_R8:
        case ENC_X86_64_OPCODE_MOV_RM_R:
        case ENC_X86_64_OPCODE_MOV_R_RM:
        case ENC_X86_64_OPCODE_LEA_R_M:
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP3_RM: {
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

// Decode the ModRM byte at p, and the SIB and displacement it may pull in,
// returning the bytes consumed.
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
                 | ((rex & ENC_X86_64_REX_R) ? 8 : 0);

    if (mod == ENC_X86_64_MOD_DIRECT) {
        insn->ei_rmkind = EMU_X86_64_RM_REG;
        insn->ei_rm     = rm | ((rex & ENC_X86_64_REX_B) ? 8 : 0);
        return n;
    }

    // r/m == 4 means a SIB byte follows; we only ever emit "base, no index".
    int base = rm;
    if (rm == (ENC_X86_64_SIB_BASE_RSP & ENC_X86_64_REG_MASK)) {
        if (avail < n + 1) {
            return 0;
        }
        base = p[n] & ENC_X86_64_REG_MASK;
        n++;
    }

    if (mod == ENC_X86_64_MOD_INDIRECT && rm == ENC_X86_64_RM_RIP) {
        if (avail < n + 4) {
            return 0;
        }
        insn->ei_rmkind = EMU_X86_64_RM_RIP;
        insn->ei_disp   = (int32_t) Emu_x86_64_ReadImm(p + n, 4);
        return n + 4;
    }

    insn->ei_rmkind = EMU_X86_64_RM_MEM;
    insn->ei_rm     = base | ((rex & ENC_X86_64_REX_B) ? 8 : 0);

    if (mod == ENC_X86_64_MOD_DISP8) {
        if (avail < n + 1) {
            return 0;
        }
        insn->ei_disp = (int32_t) Emu_x86_64_ReadImm(p + n, 1);
        n += 1;
    } else if (mod == ENC_X86_64_MOD_DISP32) {
        if (avail < n + 4) {
            return 0;
        }
        insn->ei_disp = (int32_t) Emu_x86_64_ReadImm(p + n, 4);
        n += 4;
    }
    return n;
}

// Decode one instruction, returning its length or 0 if the bytes are not one we
// emit. avail bounds the read so a truncated tail cannot run off the image.
int Emu_x86_64_Decode(const uint8_t *code, int avail, Emu_x86_64_Insn *insn)
{
    memset(insn, 0, sizeof(*insn));
    insn->ei_op2    = -1;
    insn->ei_rmkind = EMU_X86_64_RM_NONE;

    int n   = 0;
    int rex = 0;
    if (avail > 0 && (code[0] & 0xF0) == ENC_X86_64_REX_BASE) {
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
            if (avail < n + 4) {
                return 0;
            }
            insn->ei_imm = Emu_x86_64_ReadImm(code + n, 4);
            n += 4;
        } else if (op2 != ENC_X86_64_OPCODE2_SYSCALL) {
            return 0;
        }
        insn->ei_len = n;
        return n;
    }

    // push/pop and the immediate moves carry their register in the opcode byte.
    if ((op & 0xF8) == ENC_X86_64_OPCODE_PUSH_R || (op & 0xF8) == ENC_X86_64_OPCODE_POP_R) {
        insn->ei_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? 8 : 0);
        insn->ei_op     = op & 0xF8;
        insn->ei_rmkind = EMU_X86_64_RM_REG;
        insn->ei_len    = n;
        return n;
    }
    if ((op & 0xF8) == ENC_X86_64_OPCODE_MOV_R8_IMM8 || (op & 0xF8) == ENC_X86_64_OPCODE_MOV_R_IMM64) {
        int wide = (op & 0xF8) == ENC_X86_64_OPCODE_MOV_R_IMM64;
        int size = wide ? (insn->ei_rexw ? 8 : 4) : 1;
        if (avail < n + size) {
            return 0;
        }
        insn->ei_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? 8 : 0);
        insn->ei_op     = op & 0xF8;
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
            if (avail < n + 4) {
                return 0;
            }
            insn->ei_imm = Emu_x86_64_ReadImm(code + n, 4);
            n += 4;
        }
        insn->ei_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_CALL_REL32 || op == ENC_X86_64_OPCODE_JMP_REL32) {
        if (avail < n + 4) {
            return 0;
        }
        insn->ei_imm = Emu_x86_64_ReadImm(code + n, 4);
        n += 4;
        insn->ei_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_RET || op == ENC_X86_64_OPCODE_CQO) {
        insn->ei_len = n;
        return n;
    }
    return 0;
}

// Name the operation a decoded instruction performs, spelled as our own
// assembler spells it.
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
            case ENC_X86_64_OPCODE2_IMUL_R_RM:   { return "imul";    } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8: { return "movzb";   } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8: { return "movs";    } break;
            default:                             { return "(bad)";   }
        }
    }

    switch (insn->ei_op) {
        case ENC_X86_64_OPCODE_ADD_RM_R:      { return "add";   } break;
        case ENC_X86_64_OPCODE_SUB_RM_R:      { return "sub";   } break;
        case ENC_X86_64_OPCODE_CMP_RM_R:      { return "cmp";   } break;
        case ENC_X86_64_OPCODE_PUSH_R:        { return "push";  } break;
        case ENC_X86_64_OPCODE_POP_R:         { return "pop";   } break;
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32: { return "movs";  } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8:
        case ENC_X86_64_OPCODE_MOV_RM_R:
        case ENC_X86_64_OPCODE_MOV_R_RM:
        case ENC_X86_64_OPCODE_MOV_R8_IMM8:
        case ENC_X86_64_OPCODE_MOV_R_IMM64:
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:  { return "mov";   } break;
        case ENC_X86_64_OPCODE_LEA_R_M:       { return "lea";   } break;
        case ENC_X86_64_OPCODE_CQO:           { return "cqo";   } break;
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
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn->ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NEG:  { return "neg";  } break;
                case ENC_X86_64_GRP_IDIV: { return "idiv"; } break;
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
    int width = insn->ei_rexw ? EMU_X86_64_WIDTH_64 : EMU_X86_64_WIDTH_32;
    char rm[64];

    // Branches and calls name an absolute target; the encoding is relative.
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
        case ENC_X86_64_OPCODE_MOV_RM8_R8: {
            Emu_x86_64_FormatRm(insn, EMU_X86_64_WIDTH_8, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Emu_x86_64_RegName(insn->ei_reg, EMU_X86_64_WIDTH_8), rm);
        } break;
        case ENC_X86_64_OPCODE_ADD_RM_R:
        case ENC_X86_64_OPCODE_SUB_RM_R:
        case ENC_X86_64_OPCODE_CMP_RM_R:
        case ENC_X86_64_OPCODE_MOV_RM_R: {
            Emu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Emu_x86_64_RegName(insn->ei_reg, width), rm);
        } break;
        default: {
            // Everything left reads r/m and writes the reg field.
            int srcw = width;
            if (insn->ei_op2 == ENC_X86_64_OPCODE2_MOVZX_R_RM8 || insn->ei_op2 == ENC_X86_64_OPCODE2_MOVSX_R_RM8) {
                srcw = EMU_X86_64_WIDTH_8;
            } else if (insn->ei_op == ENC_X86_64_OPCODE_MOVSXD_R_RM32) {
                srcw = EMU_X86_64_WIDTH_32;
            }
            Emu_x86_64_FormatRm(insn, srcw, next, rm, sizeof(rm));
            if (insn->ei_op2 >= 0 && insn->ei_op2 >= ENC_X86_64_OPCODE2_SETE && insn->ei_op2 <= ENC_X86_64_OPCODE2_SETLE) {
                snprintf(out, n, "%s %s", name, rm);
            } else {
                snprintf(out, n, "%s %s, %%%s", name, rm, Emu_x86_64_RegName(insn->ei_reg, width));
            }
        }
    }
}

// Start a program: entry in %rip, the image's stack in %rsp, everything else
// zero, which is what a freshly loaded image is entitled to assume.
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
    uint64_t val = cpu->ec_reg[reg & 15];
    switch (width) {
        case EMU_X86_64_WIDTH_8:  { return val & 0xFF; } break;
        case EMU_X86_64_WIDTH_32: { return val & 0xFFFFFFFF; } break;
        default:                  { return val; }
    }
}

// Write a register, zero-extending a 32-bit result and preserving the bits
// above a byte, as the hardware does.
void Emu_x86_64_WriteReg(Emu_x86_64_Cpu *cpu, int reg, uint64_t value, int width)
{
    switch (width) {
        case EMU_X86_64_WIDTH_8: {
            cpu->ec_reg[reg & 15] = (cpu->ec_reg[reg & 15] & ~(uint64_t) 0xFF) | (value & 0xFF);
        } break;
        case EMU_X86_64_WIDTH_32: {
            cpu->ec_reg[reg & 15] = value & 0xFFFFFFFF;
        } break;
        default: {
            cpu->ec_reg[reg & 15] = value;
        }
    }
}

// Read width bits from the image, faulting if that address is not mapped.
uint64_t Emu_x86_64_ReadMem(Emu_x86_64_Cpu *cpu, uint64_t addr, int width)
{
    int n = width / 8;
    const uint8_t *p = Elf_Load_At(cpu->ec_img, addr, n);
    if (! p) {
        Emu_x86_64_Fault(cpu, "read of unmapped memory", addr);
        return 0;
    }
    uint64_t val = 0;
    for (int i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (8 * i);
    }
    return val;
}

// Write width bits into the image, faulting if that address is not mapped.
void Emu_x86_64_WriteMem(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value, int width)
{
    int n = width / 8;
    uint8_t *p = Elf_Load_At(cpu->ec_img, addr, n);
    if (! p) {
        Emu_x86_64_Fault(cpu, "write to unmapped memory", addr);
        return;
    }
    for (int i = 0; i < n; i++) {
        p[i] = (value >> (8 * i)) & 0xFF;
    }
}

// Compute the address an instruction's memory operand names; next is the
// address of the instruction after it, which is what %rip holds by then.
uint64_t Emu_x86_64_RmAddr(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next)
{
    if (insn->ei_rmkind == EMU_X86_64_RM_RIP) {
        return next + (int64_t) insn->ei_disp;
    }
    return cpu->ec_reg[insn->ei_rm & 15] + (int64_t) insn->ei_disp;
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

// Set the flags a - b leaves behind, which is what every compare here needs.
void Emu_x86_64_FlagsSub(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, int width)
{
    uint64_t mask = width == EMU_X86_64_WIDTH_64 ? ~(uint64_t) 0 : 0xFFFFFFFF;
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
    uint64_t mask = width == EMU_X86_64_WIDTH_64 ? ~(uint64_t) 0 : 0xFFFFFFFF;
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
        case EMU_X86_64_SYS_EXIT: {
            cpu->ec_halted = 1;
            cpu->ec_status = cpu->ec_reg[EMU_X86_64_REG_RDI] & 0xFF;
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
    const uint8_t *code = Elf_Load_At(cpu->ec_img, rip, 1);
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
    int width = insn.ei_rexw ? EMU_X86_64_WIDTH_64 : EMU_X86_64_WIDTH_32;
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
            case ENC_X86_64_OPCODE2_IMUL_R_RM: {
                uint64_t a = Emu_x86_64_ReadReg(cpu, insn.ei_reg, width);
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, a * b, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_8);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, b & 0xFF, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8: {
                uint64_t b = Emu_x86_64_ReadRm(cpu, &insn, next, EMU_X86_64_WIDTH_8);
                Emu_x86_64_WriteReg(cpu, insn.ei_reg, (uint64_t) (int64_t) (int8_t) b, width);
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
        case ENC_X86_64_OPCODE_PUSH_R: {
            *rsp -= 8;
            Emu_x86_64_WriteMem(cpu, *rsp, cpu->ec_reg[insn.ei_rm & 15], EMU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_POP_R: {
            uint64_t val = Emu_x86_64_ReadMem(cpu, *rsp, EMU_X86_64_WIDTH_64);
            *rsp += 8;
            cpu->ec_reg[insn.ei_rm & 15] = val;
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
            cpu->ec_reg[EMU_X86_64_REG_RDX] = rax < 0 ? ~(uint64_t) 0 : 0;
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
            *rsp += 8;
        } break;
        case ENC_X86_64_OPCODE_CALL_REL32: {
            *rsp -= 8;
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
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn.ei_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NEG: {
                    uint64_t a = Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    Emu_x86_64_FlagsSub(cpu, 0, a, width);
                    Emu_x86_64_WriteRm(cpu, &insn, next, 0 - a, width);
                } break;
                case ENC_X86_64_GRP_IDIV: {
                    int64_t d = (int64_t) Emu_x86_64_ReadRm(cpu, &insn, next, width);
                    if (d == 0) {
                        Emu_x86_64_Fault(cpu, "divide by zero", rip);
                        return;
                    }
                    __int128 num = ((__int128) (int64_t) cpu->ec_reg[EMU_X86_64_REG_RDX] << 64)
                                 | cpu->ec_reg[EMU_X86_64_REG_RAX];
                    cpu->ec_reg[EMU_X86_64_REG_RAX] = (uint64_t) (int64_t) (num / d);
                    cpu->ec_reg[EMU_X86_64_REG_RDX] = (uint64_t) (int64_t) (num % d);
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
