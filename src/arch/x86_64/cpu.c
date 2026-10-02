/*
 * C source file for the x86-64 CPU.
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
#include "arch/x86_64/cpu.h"

// 64-bit register names, numbered as ModRM and REX number them.
static const char *Cpu_x86_64_Name64[CPU_X86_64_REG_COUNT] = {
    [CPU_X86_64_REG_RAX] = "rax",
    [CPU_X86_64_REG_RCX] = "rcx",
    [CPU_X86_64_REG_RDX] = "rdx",
    [CPU_X86_64_REG_RBX] = "rbx",
    [CPU_X86_64_REG_RSP] = "rsp",
    [CPU_X86_64_REG_RBP] = "rbp",
    [CPU_X86_64_REG_RSI] = "rsi",
    [CPU_X86_64_REG_RDI] = "rdi",
    [CPU_X86_64_REG_R8]  = "r8",
    [CPU_X86_64_REG_R9]  = "r9",
    [CPU_X86_64_REG_R10] = "r10",
    [CPU_X86_64_REG_R11] = "r11",
    [CPU_X86_64_REG_R12] = "r12",
    [CPU_X86_64_REG_R13] = "r13",
    [CPU_X86_64_REG_R14] = "r14",
    [CPU_X86_64_REG_R15] = "r15"
};

// 32-bit register names.
static const char *Cpu_x86_64_Name32[CPU_X86_64_REG_COUNT] = {
    [CPU_X86_64_REG_RAX] = "eax",
    [CPU_X86_64_REG_RCX] = "ecx",
    [CPU_X86_64_REG_RDX] = "edx",
    [CPU_X86_64_REG_RBX] = "ebx",
    [CPU_X86_64_REG_RSP] = "esp",
    [CPU_X86_64_REG_RBP] = "ebp",
    [CPU_X86_64_REG_RSI] = "esi",
    [CPU_X86_64_REG_RDI] = "edi",
    [CPU_X86_64_REG_R8]  = "r8d",
    [CPU_X86_64_REG_R9]  = "r9d",
    [CPU_X86_64_REG_R10] = "r10d",
    [CPU_X86_64_REG_R11] = "r11d",
    [CPU_X86_64_REG_R12] = "r12d",
    [CPU_X86_64_REG_R13] = "r13d",
    [CPU_X86_64_REG_R14] = "r14d",
    [CPU_X86_64_REG_R15] = "r15d"
};

// 16-bit register names.
static const char *Cpu_x86_64_Name16[CPU_X86_64_REG_COUNT] = {
    [CPU_X86_64_REG_RAX] = "ax",
    [CPU_X86_64_REG_RCX] = "cx",
    [CPU_X86_64_REG_RDX] = "dx",
    [CPU_X86_64_REG_RBX] = "bx",
    [CPU_X86_64_REG_RSP] = "sp",
    [CPU_X86_64_REG_RBP] = "bp",
    [CPU_X86_64_REG_RSI] = "si",
    [CPU_X86_64_REG_RDI] = "di",
    [CPU_X86_64_REG_R8]  = "r8w",
    [CPU_X86_64_REG_R9]  = "r9w",
    [CPU_X86_64_REG_R10] = "r10w",
    [CPU_X86_64_REG_R11] = "r11w",
    [CPU_X86_64_REG_R12] = "r12w",
    [CPU_X86_64_REG_R13] = "r13w",
    [CPU_X86_64_REG_R14] = "r14w",
    [CPU_X86_64_REG_R15] = "r15w"
};

// 8-bit register names.
static const char *Cpu_x86_64_Name8[CPU_X86_64_REG_COUNT] = {
    [CPU_X86_64_REG_RAX] = "al",
    [CPU_X86_64_REG_RCX] = "cl",
    [CPU_X86_64_REG_RDX] = "dl",
    [CPU_X86_64_REG_RBX] = "bl",
    [CPU_X86_64_REG_RSP] = "spl",
    [CPU_X86_64_REG_RBP] = "bpl",
    [CPU_X86_64_REG_RSI] = "sil",
    [CPU_X86_64_REG_RDI] = "dil",
    [CPU_X86_64_REG_R8]  = "r8b",
    [CPU_X86_64_REG_R9]  = "r9b",
    [CPU_X86_64_REG_R10] = "r10b",
    [CPU_X86_64_REG_R11] = "r11b",
    [CPU_X86_64_REG_R12] = "r12b",
    [CPU_X86_64_REG_R13] = "r13b",
    [CPU_X86_64_REG_R14] = "r14b",
    [CPU_X86_64_REG_R15] = "r15b"
};

// Reset a CPU onto a bus, to start at rip with the stack at rsp.
void Cpu_x86_64_Init(Cpu_x86_64_State *cpu, const Cpu_x86_64_Bus *bus, uint64_t rip, uint64_t rsp)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu->cs_bus = bus;
    cpu->cs_rip = rip;
    cpu->cs_reg[CPU_X86_64_REG_RSP] = rsp;
}

// Report the first fault of a step and stop the step with its exception.
void Cpu_x86_64_Fault(Cpu_x86_64_State *cpu, Cpu_x86_64_Vector vector, Err_Code code, ...)
{
    va_list ap;

    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    va_start(ap, code);
    Err_ShowVa(LOG_SEVERITY_ERROR, LOG_LINE_NONE, code, ap);
    va_end(ap);
    cpu->cs_trap = CPU_X86_64_TRAP_EXCEPTION;
    cpu->cs_vector = vector;
}

// Read a register at the given width.
uint64_t Cpu_x86_64_ReadReg(const Cpu_x86_64_State *cpu, Cpu_x86_64_Reg reg, Cpu_x86_64_OperandWidth width)
{
    uint64_t val = cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK];
    switch (width) {
        case CPU_X86_64_WIDTH_8: {
            return val & CPU_X86_64_MASK_8;
        } break;
        case CPU_X86_64_WIDTH_16: {
            return val & CPU_X86_64_MASK_16;
        } break;
        case CPU_X86_64_WIDTH_32: {
            return val & CPU_X86_64_MASK_32;
        } break;
        default: {
            return val;
        }
    }
}

// Write a register the way the hardware does, zero-extending a 32-bit result.
void Cpu_x86_64_WriteReg(Cpu_x86_64_State *cpu, Cpu_x86_64_Reg reg, uint64_t value, Cpu_x86_64_OperandWidth width)
{
    switch (width) {
        case CPU_X86_64_WIDTH_8: {
            cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] = (cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] & ~(uint64_t) CPU_X86_64_MASK_8) | (value & CPU_X86_64_MASK_8);
        } break;
        case CPU_X86_64_WIDTH_16: {
            cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] = (cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] & ~(uint64_t) CPU_X86_64_MASK_16) | (value & CPU_X86_64_MASK_16);
        } break;
        case CPU_X86_64_WIDTH_32: {
            cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] = value & CPU_X86_64_MASK_32;
        } break;
        default: {
            cpu->cs_reg[reg & CPU_X86_64_REG_INDEX_MASK] = value;
        }
    }
}

// Return whether an address falls in the bus's device window.
bool Cpu_x86_64_IsDevice(const Cpu_x86_64_State *cpu, uint64_t addr)
{
    return addr - cpu->cs_bus->cb_io_base < cpu->cs_bus->cb_io_size;
}

// Return the bytes a read of memory takes.
const uint8_t *Cpu_x86_64_ReadAt(Cpu_x86_64_State *cpu, uint64_t addr, size_t size)
{
    size_t avail = 0;
    const uint8_t *ptr = cpu->cs_bus->cb_map(cpu->cs_bus->cb_ctx, addr, &avail);
    if (! ptr || avail < size) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_PF, ERR_CPU_READ_NOT_MAPPED, (Cpu_x86_64_TypeULLong) addr, (Cpu_x86_64_TypeULLong) cpu->cs_rip);
        return NULL;
    }
    return ptr;
}

// Return the bytes a write to memory takes.
uint8_t *Cpu_x86_64_WriteAt(Cpu_x86_64_State *cpu, uint64_t addr, size_t size)
{
    size_t avail = 0;
    uint8_t *ptr = cpu->cs_bus->cb_map(cpu->cs_bus->cb_ctx, addr, &avail);
    if (! ptr || avail < size) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_PF, ERR_CPU_WRITE_NOT_MAPPED, (Cpu_x86_64_TypeULLong) addr, (Cpu_x86_64_TypeULLong) cpu->cs_rip);
        return NULL;
    }
    return ptr;
}

// Read width bits from a device or memory, faulting where none is mapped.
uint64_t Cpu_x86_64_ReadMem(Cpu_x86_64_State *cpu, uint64_t addr, Cpu_x86_64_OperandWidth width)
{
    size_t n = width / CPU_X86_64_BITS_PER_BYTE;
    if (Cpu_x86_64_IsDevice(cpu, addr)) {
        return cpu->cs_bus->cb_load(cpu->cs_bus->cb_ctx, addr, n);
    }

    const uint8_t *p = Cpu_x86_64_ReadAt(cpu, addr, n);
    if (! p) {
        return 0;
    }
    uint64_t val = 0;
    for (size_t i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (CPU_X86_64_BITS_PER_BYTE * i);
    }
    return val;
}

// Write width bits to a device or memory, faulting where none is mapped.
void Cpu_x86_64_WriteMem(Cpu_x86_64_State *cpu, uint64_t addr, uint64_t value, Cpu_x86_64_OperandWidth width)
{
    size_t n = width / CPU_X86_64_BITS_PER_BYTE;
    if (Cpu_x86_64_IsDevice(cpu, addr)) {
        cpu->cs_bus->cb_store(cpu->cs_bus->cb_ctx, addr, n, value);
        return;
    }

    uint8_t *p = Cpu_x86_64_WriteAt(cpu, addr, n);
    if (! p) {
        return;
    }
    for (size_t i = 0; i < n; i++) {
        p[i] = (value >> (CPU_X86_64_BITS_PER_BYTE * i)) & CPU_X86_64_MASK_8;
    }
}

// Compute the address an instruction's memory operand names.
uint64_t Cpu_x86_64_RmAddr(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next)
{
    if (insn->ci_rmkind == CPU_X86_64_RM_RIP) {
        return next + (int64_t) insn->ci_disp;
    }
    return cpu->cs_reg[insn->ci_rm & CPU_X86_64_REG_INDEX_MASK] + (int64_t) insn->ci_disp;
}

// Read an instruction's r/m operand.
uint64_t Cpu_x86_64_ReadRm(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, Cpu_x86_64_OperandWidth width)
{
    if (insn->ci_rmkind == CPU_X86_64_RM_REG) {
        return Cpu_x86_64_ReadReg(cpu, insn->ci_rm, width);
    }
    return Cpu_x86_64_ReadMem(cpu, Cpu_x86_64_RmAddr(cpu, insn, next), width);
}

// Write an instruction's r/m operand.
void Cpu_x86_64_WriteRm(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t value, Cpu_x86_64_OperandWidth width)
{
    if (insn->ci_rmkind == CPU_X86_64_RM_REG) {
        Cpu_x86_64_WriteReg(cpu, insn->ci_rm, value, width);
        return;
    }
    Cpu_x86_64_WriteMem(cpu, Cpu_x86_64_RmAddr(cpu, insn, next), value, width);
}

// Set the flags a - b leaves behind.
void Cpu_x86_64_FlagsSub(Cpu_x86_64_State *cpu, uint64_t a, uint64_t b, Cpu_x86_64_OperandWidth width)
{
    uint64_t mask = CPU_X86_64_MASK_64 >> (CPU_X86_64_WIDTH_64 - width);
    int32_t sign = width - 1;
    a &= mask;
    b &= mask;
    uint64_t res = (a - b) & mask;

    cpu->cs_zf = res == 0;
    cpu->cs_sf = (res >> sign) & 1;
    cpu->cs_cf = a < b;
    cpu->cs_of = (((a ^ b) & (a ^ res)) >> sign) & 1;
    cpu->cs_pf = Cpu_x86_64_Parity(res);
}

// Set the flags a + b leaves behind.
void Cpu_x86_64_FlagsAdd(Cpu_x86_64_State *cpu, uint64_t a, uint64_t b, Cpu_x86_64_OperandWidth width)
{
    uint64_t mask = CPU_X86_64_MASK_64 >> (CPU_X86_64_WIDTH_64 - width);
    int32_t sign = width - 1;
    a &= mask;
    b &= mask;
    uint64_t res = (a + b) & mask;

    cpu->cs_zf = res == 0;
    cpu->cs_sf = (res >> sign) & 1;
    cpu->cs_cf = res < a;
    cpu->cs_of = ((~(a ^ b) & (a ^ res)) >> sign) & 1;
    cpu->cs_pf = Cpu_x86_64_Parity(res);
}

// Return whether the low byte of a result has an even number of set bits.
bool Cpu_x86_64_Parity(uint64_t res)
{
    uint8_t byte = res & CPU_X86_64_MASK_8;
    bool even = true;

    for (; byte; byte &= byte - 1) {
        even = ! even;
    }
    return even;
}

// Set the flags an unordered comparison of a with b leaves behind.
void Cpu_x86_64_FlagsCompare(Cpu_x86_64_State *cpu, long double a, long double b)
{
    bool unordered = isnan(a) || isnan(b);

    cpu->cs_zf = unordered || a == b;
    cpu->cs_pf = unordered;
    cpu->cs_cf = unordered || a < b;
    cpu->cs_sf = false;
    cpu->cs_of = false;
}

// Truncate a value toward zero into a signed integer of width bits.
uint64_t Cpu_x86_64_Truncate(long double value, Cpu_x86_64_OperandWidth width)
{
    long double limit = ldexpl(1, width - 1);

    if (isnan(value) || value >= limit || value <= -limit - 1) {
        return width == CPU_X86_64_WIDTH_64 ? CPU_X86_64_INDEFINITE_64 : CPU_X86_64_INDEFINITE_32;
    }
    return (uint64_t) (int64_t) value;
}

// Read an SSE instruction's source, an SSE register's low lane or memory.
uint64_t Cpu_x86_64_ReadSse(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, Cpu_x86_64_OperandWidth width)
{
    if (insn->ci_rmkind != CPU_X86_64_RM_REG) {
        return Cpu_x86_64_ReadMem(cpu, Cpu_x86_64_RmAddr(cpu, insn, next), width);
    }
    uint64_t lane = cpu->cs_xmm[insn->ci_rm & CPU_X86_64_REG_INDEX_MASK][0];
    return width == CPU_X86_64_WIDTH_64 ? lane : lane & CPU_X86_64_MASK_32;
}

// Execute one scalar SSE operation or conversion.
void Cpu_x86_64_StepSse(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    uint64_t *dst = cpu->cs_xmm[insn->ci_reg & CPU_X86_64_REG_INDEX_MASK];
    Cpu_x86_64_OperandWidth width = insn->ci_rexw ? CPU_X86_64_WIDTH_64 : CPU_X86_64_WIDTH_32;
    bool dbl = insn->ci_rep == ENC_X86_64_OPCODE_SSE_DOUBLE || (insn->ci_rep == 0 && insn->ci_opsize16);
    Cpu_x86_64_OperandWidth lane = dbl ? CPU_X86_64_WIDTH_64 : CPU_X86_64_WIDTH_32;

    switch (insn->ci_op2) {
        case ENC_X86_64_OPCODE2_MOVQ_XMM_RM: {
            dst[0] = Cpu_x86_64_ReadRm(cpu, insn, next, width);
            dst[1] = 0;
        } break;
        case ENC_X86_64_OPCODE2_MOVQ_RM_XMM: {
            Cpu_x86_64_WriteRm(cpu, insn, next, dst[0], width);
        } break;
        case ENC_X86_64_OPCODE2_UCOMIS: {
            uint64_t b = Cpu_x86_64_ReadSse(cpu, insn, next, lane);
            if (dbl) {
                Cpu_x86_64_FlagsCompare(cpu, Fp_DoubleFromBits(dst[0]), Fp_DoubleFromBits(b));
            } else {
                Cpu_x86_64_FlagsCompare(cpu, Fp_FloatFromBits((uint32_t) dst[0]), Fp_FloatFromBits((uint32_t) b));
            }
        } break;
        case ENC_X86_64_OPCODE2_CVTSI2S: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, insn, next, width);
            int64_t v = width == CPU_X86_64_WIDTH_64 ? (int64_t) a : (int32_t) a;
            if (dbl) {
                dst[0] = Fp_DoubleBits((double) v);
            } else {
                dst[0] = (dst[0] & ~(uint64_t) CPU_X86_64_MASK_32) | Fp_FloatBits((float) v);
            }
        } break;
        case ENC_X86_64_OPCODE2_CVTTS2SI: {
            uint64_t b = Cpu_x86_64_ReadSse(cpu, insn, next, lane);
            long double v = dbl ? Fp_DoubleFromBits(b) : Fp_FloatFromBits((uint32_t) b);
            Cpu_x86_64_WriteReg(cpu, insn->ci_reg, Cpu_x86_64_Truncate(v, width), width);
        } break;
        case ENC_X86_64_OPCODE2_CVTS2S: {
            if (dbl) {
                double v = Fp_DoubleFromBits(Cpu_x86_64_ReadSse(cpu, insn, next, CPU_X86_64_WIDTH_64));
                dst[0] = (dst[0] & ~(uint64_t) CPU_X86_64_MASK_32) | Fp_FloatBits((float) v);
            } else {
                float v = Fp_FloatFromBits((uint32_t) Cpu_x86_64_ReadSse(cpu, insn, next, CPU_X86_64_WIDTH_32));
                dst[0] = Fp_DoubleBits((double) v);
            }
        } break;
        default: {
            uint64_t b = Cpu_x86_64_ReadSse(cpu, insn, next, lane);
            if (dbl) {
                double x = Fp_DoubleFromBits(dst[0]);
                double y = Fp_DoubleFromBits(b);
                double r = 0;
                switch (insn->ci_op2) {
                    case ENC_X86_64_OPCODE2_ADDS: {
                        r = x + y;
                    } break;
                    case ENC_X86_64_OPCODE2_SUBS: {
                        r = x - y;
                    } break;
                    case ENC_X86_64_OPCODE2_MULS: {
                        r = x * y;
                    } break;
                    case ENC_X86_64_OPCODE2_DIVS: {
                        r = x / y;
                    } break;
                    default: {
                        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_SSE_DOUBLE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                    }
                }
                dst[0] = Fp_DoubleBits(r);
            } else {
                float x = Fp_FloatFromBits((uint32_t) dst[0]);
                float y = Fp_FloatFromBits((uint32_t) b);
                float r = 0;
                switch (insn->ci_op2) {
                    case ENC_X86_64_OPCODE2_ADDS: {
                        r = x + y;
                    } break;
                    case ENC_X86_64_OPCODE2_SUBS: {
                        r = x - y;
                    } break;
                    case ENC_X86_64_OPCODE2_MULS: {
                        r = x * y;
                    } break;
                    case ENC_X86_64_OPCODE2_DIVS: {
                        r = x / y;
                    } break;
                    default: {
                        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                    }
                }
                dst[0] = (dst[0] & ~(uint64_t) CPU_X86_64_MASK_32) | Fp_FloatBits(r);
            }
        } break;
    }
}

// Return the x87 register %st(i) names.
long double *Cpu_x86_64_St(Cpu_x86_64_State *cpu, int32_t i)
{
    return &cpu->cs_st[(cpu->cs_top + i) & CPU_X86_64_ST_MASK];
}

// Push a value onto the x87 stack.
void Cpu_x86_64_StPush(Cpu_x86_64_State *cpu, long double value)
{
    cpu->cs_top = (cpu->cs_top - 1) & CPU_X86_64_ST_MASK;
    cpu->cs_st[cpu->cs_top] = value;
}

// Pop the x87 stack.
long double Cpu_x86_64_StPop(Cpu_x86_64_State *cpu)
{
    long double value = cpu->cs_st[cpu->cs_top];
    cpu->cs_top = (cpu->cs_top + 1) & CPU_X86_64_ST_MASK;
    return value;
}

// Execute an x87 load or store of memory.
void Cpu_x86_64_StepX87Mem(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    uint64_t addr = Cpu_x86_64_RmAddr(cpu, insn, next);
    int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;

    switch (insn->ci_op << ENC_X86_64_REG_SHIFT | digit) {
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M32: {
            Cpu_x86_64_StPush(cpu, Fp_FloatFromBits((uint32_t) Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_32)));
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M32: {
            Cpu_x86_64_WriteMem(cpu, addr, Fp_FloatBits((float) Cpu_x86_64_StPop(cpu)), CPU_X86_64_WIDTH_32);
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M64: {
            Cpu_x86_64_StPush(cpu, Fp_DoubleFromBits(Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_64)));
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M64: {
            Cpu_x86_64_WriteMem(cpu, addr, Fp_DoubleBits((double) Cpu_x86_64_StPop(cpu)), CPU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FISTTP_M64: {
            Cpu_x86_64_WriteMem(cpu, addr, Cpu_x86_64_Truncate(Cpu_x86_64_StPop(cpu), CPU_X86_64_WIDTH_64), CPU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_X87_DF << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FILD_M64: {
            Cpu_x86_64_StPush(cpu, (long double) (int64_t) Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_64));
        } break;
        case ENC_X86_64_OPCODE_X87_DB << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M80: {
            const uint8_t *p = Cpu_x86_64_ReadAt(cpu, addr, FP_EXTENDED_SIZE);
            if (! p) {
                return;
            }
            Cpu_x86_64_StPush(cpu, Fp_DecodeExtended(p));
        } break;
        case ENC_X86_64_OPCODE_X87_DB << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M80: {
            uint8_t *p = Cpu_x86_64_WriteAt(cpu, addr, FP_EXTENDED_SIZE);
            if (! p) {
                return;
            }
            Fp_EncodeExtended(Cpu_x86_64_StPop(cpu), p);
        } break;
        default: {
            Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_MEM_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        }
    }
}

// Execute an x87 operation.
void Cpu_x86_64_StepX87(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    if (insn->ci_rmkind != CPU_X86_64_RM_REG) {
        Cpu_x86_64_StepX87Mem(cpu, insn, next, rip);
        return;
    }

    int32_t i = insn->ci_rm & ENC_X86_64_REG_MASK;
    int32_t form = CPU_X86_64_X87_REG_FORM | (insn->ci_reg & ENC_X86_64_REG_MASK) << ENC_X86_64_REG_SHIFT;
    long double *top = Cpu_x86_64_St(cpu, 0);
    long double *sti = Cpu_x86_64_St(cpu, i);

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_X87_D9: {
            if (form != ENC_X86_64_X87_FCHS || i != 0) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_D9_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            *top = -*top;
        } break;
        case ENC_X86_64_OPCODE_X87_DE: {
            switch (form) {
                case ENC_X86_64_X87_FADDP: {
                    *sti = *sti + *top;
                } break;
                case ENC_X86_64_X87_FMULP: {
                    *sti = *sti * *top;
                } break;
                case ENC_X86_64_X87_FSUBRP: {
                    *sti = *sti - *top;
                } break;
                case ENC_X86_64_X87_FDIVRP: {
                    *sti = *sti / *top;
                } break;
                default: {
                    Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                    return;
                }
            }
            Cpu_x86_64_StPop(cpu);
        } break;
        case ENC_X86_64_OPCODE_X87_DF: {
            if (form != ENC_X86_64_X87_FUCOMIP) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DF_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            Cpu_x86_64_FlagsCompare(cpu, *top, *sti);
            Cpu_x86_64_StPop(cpu);
        } break;
        case ENC_X86_64_OPCODE_X87_DD: {
            if (form != ENC_X86_64_X87_FSTP) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DD_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            *sti = *top;
            Cpu_x86_64_StPop(cpu);
        } break;
        default: {
            Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        }
    }
}

// Divide %rdx:%rax by an r/m operand at its width as the hardware does.
void Cpu_x86_64_Divide(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip, Cpu_x86_64_OperandWidth width)
{
    bool is_signed = (insn->ci_reg & ENC_X86_64_REG_MASK) == ENC_X86_64_GRP_IDIV;
    uint64_t sign = (uint64_t) 1 << (width - 1);
    uint64_t mask = sign | (sign - 1);
    uint64_t d = Cpu_x86_64_ReadRm(cpu, insn, next, width);
    if (d == 0) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_DE, ERR_CPU_DIVIDE_BY_ZERO, (Cpu_x86_64_TypeULLong) rip);
        return;
    }

    // Phase: divide the magnitudes
    uint64_t hi = Cpu_x86_64_ReadReg(cpu, CPU_X86_64_REG_RDX, width);
    uint64_t lo = Cpu_x86_64_ReadReg(cpu, CPU_X86_64_REG_RAX, width);
    Cpu_x86_64_TypeUInt128 num = ((Cpu_x86_64_TypeUInt128) hi << width) | lo;
    bool num_neg = is_signed && (hi & sign);
    bool d_neg = is_signed && (d & sign);
    if (num_neg && width < CPU_X86_64_WIDTH_64) {
        num |= ~(((Cpu_x86_64_TypeUInt128) 1 << (2 * width)) - 1);
    }
    Cpu_x86_64_TypeUInt128 num_mag = num_neg ? -num : num;
    uint64_t d_mag = d_neg ? -d & mask : d;
    Cpu_x86_64_TypeUInt128 q = num_mag / d_mag;
    uint64_t r = (uint64_t) (num_mag % d_mag);

    // Phase: fault on a quotient the width cannot hold
    bool q_neg = num_neg != d_neg;
    uint64_t limit = ! is_signed ? mask : (q_neg ? sign : sign - 1);
    if (q > limit) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_DE, ERR_CPU_QUOTIENT_TOO_LARGE, (Cpu_x86_64_TypeULLong) rip);
        return;
    }
    Cpu_x86_64_WriteReg(cpu, CPU_X86_64_REG_RAX, q_neg ? -(uint64_t) q : (uint64_t) q, width);
    Cpu_x86_64_WriteReg(cpu, CPU_X86_64_REG_RDX, num_neg ? -r : r, width);
}

// Execute the instruction at %rip and leave %rip on the next one.
void Cpu_x86_64_Step(Cpu_x86_64_State *cpu)
{
    uint64_t rip = cpu->cs_rip;
    size_t avail = 0;
    const uint8_t *code = cpu->cs_bus->cb_map(cpu->cs_bus->cb_ctx, rip, &avail);
    Cpu_x86_64_Insn insn;

    cpu->cs_trap = CPU_X86_64_TRAP_NONE;
    if (! code) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_PF, ERR_CPU_FETCH_NOT_MAPPED, (Cpu_x86_64_TypeULLong) rip);
        return;
    }
    if (! Cpu_x86_64_Decode(code, avail, &insn)) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_OPCODE_NOT_DECODABLE, (Cpu_x86_64_TypeULLong) rip);
        return;
    }

    uint64_t next = rip + insn.ci_len;
    Cpu_x86_64_OperandWidth width = Cpu_x86_64_Width(&insn);
    uint64_t *rsp = &cpu->cs_reg[CPU_X86_64_REG_RSP];
    cpu->cs_rip = next;

    if (insn.ci_op == ENC_X86_64_OPCODE_ESCAPE) {
        switch (insn.ci_op2) {
            case ENC_X86_64_OPCODE2_SYSCALL: {
                cpu->cs_trap = CPU_X86_64_TRAP_SYSCALL;
            } break;
            case ENC_X86_64_OPCODE2_JE_REL32: {
                if (cpu->cs_zf) {
                    cpu->cs_rip = next + insn.ci_imm;
                }
            } break;
            case ENC_X86_64_OPCODE2_JNE_REL32: {
                if (! cpu->cs_zf) {
                    cpu->cs_rip = next + insn.ci_imm;
                }
            } break;
            case ENC_X86_64_OPCODE2_SETE: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_zf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETNE: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, ! cpu->cs_zf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETL: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_sf != cpu->cs_of, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETLE: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_zf || cpu->cs_sf != cpu->cs_of, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETB: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_cf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETBE: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_cf || cpu->cs_zf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETA: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, ! cpu->cs_cf && ! cpu->cs_zf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETAE: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, ! cpu->cs_cf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETP: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, cpu->cs_pf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_SETNP: {
                Cpu_x86_64_WriteRm(cpu, &insn, next, ! cpu->cs_pf, CPU_X86_64_WIDTH_8);
            } break;
            case ENC_X86_64_OPCODE2_CVTSI2S:
            case ENC_X86_64_OPCODE2_CVTTS2SI:
            case ENC_X86_64_OPCODE2_UCOMIS:
            case ENC_X86_64_OPCODE2_ADDS:
            case ENC_X86_64_OPCODE2_MULS:
            case ENC_X86_64_OPCODE2_CVTS2S:
            case ENC_X86_64_OPCODE2_SUBS:
            case ENC_X86_64_OPCODE2_DIVS:
            case ENC_X86_64_OPCODE2_MOVQ_XMM_RM:
            case ENC_X86_64_OPCODE2_MOVQ_RM_XMM: {
                Cpu_x86_64_StepSse(cpu, &insn, next, rip);
            } break;
            case ENC_X86_64_OPCODE2_IMUL_R_RM: {
                uint64_t a = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
                uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
                Cpu_x86_64_WriteReg(cpu, insn.ci_reg, a * b, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8: {
                uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_8);
                Cpu_x86_64_WriteReg(cpu, insn.ci_reg, b & CPU_X86_64_MASK_8, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8: {
                uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_8);
                Cpu_x86_64_WriteReg(cpu, insn.ci_reg, (uint64_t) (int64_t) (int8_t) b, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM16: {
                uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_16);
                Cpu_x86_64_WriteReg(cpu, insn.ci_reg, b & CPU_X86_64_MASK_16, width);
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM16: {
                uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_16);
                Cpu_x86_64_WriteReg(cpu, insn.ci_reg, (uint64_t) (int64_t) (int16_t) b, width);
            } break;
            default: {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_TWO_BYTE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
            }
        }
        return;
    }

    switch (insn.ci_op) {
        case ENC_X86_64_OPCODE_ADD_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_FlagsAdd(cpu, a, b, width);
            Cpu_x86_64_WriteRm(cpu, &insn, next, a + b, width);
        } break;
        case ENC_X86_64_OPCODE_SUB_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_FlagsSub(cpu, a, b, width);
            Cpu_x86_64_WriteRm(cpu, &insn, next, a - b, width);
        } break;
        case ENC_X86_64_OPCODE_CMP_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_FlagsSub(cpu, a, b, width);
        } break;
        case ENC_X86_64_OPCODE_AND_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_WriteRm(cpu, &insn, next, a & b, width);
        } break;
        case ENC_X86_64_OPCODE_OR_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_WriteRm(cpu, &insn, next, a | b, width);
        } break;
        case ENC_X86_64_OPCODE_XOR_RM_R: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width);
            Cpu_x86_64_WriteRm(cpu, &insn, next, a ^ b, width);
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            int32_t count = cpu->cs_reg[CPU_X86_64_REG_RCX] & (width == CPU_X86_64_WIDTH_64 ? CPU_X86_64_SHIFT_MASK_64 : CPU_X86_64_SHIFT_MASK_32);
            switch (insn.ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_SHL: {
                    Cpu_x86_64_WriteRm(cpu, &insn, next, a << count, width);
                } break;
                case ENC_X86_64_GRP_SAR: {
                    int64_t signed_a = width == CPU_X86_64_WIDTH_64 ? (int64_t) a : (int32_t) a;
                    Cpu_x86_64_WriteRm(cpu, &insn, next, (uint64_t) (signed_a >> count), width);
                } break;
                case ENC_X86_64_GRP_SHR: {
                    Cpu_x86_64_WriteRm(cpu, &insn, next, a >> count, width);
                } break;
                default: {
                    Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_GROUP2_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                }
            }
        } break;
        case ENC_X86_64_OPCODE_PUSH_R: {
            *rsp -= CPU_X86_64_STACK_SLOT;
            Cpu_x86_64_WriteMem(cpu, *rsp, cpu->cs_reg[insn.ci_rm & CPU_X86_64_REG_INDEX_MASK], CPU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_POP_R: {
            uint64_t val = Cpu_x86_64_ReadMem(cpu, *rsp, CPU_X86_64_WIDTH_64);
            *rsp += CPU_X86_64_STACK_SLOT;
            cpu->cs_reg[insn.ci_rm & CPU_X86_64_REG_INDEX_MASK] = val;
        } break;
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32: {
            uint64_t b = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_32);
            Cpu_x86_64_WriteReg(cpu, insn.ci_reg, (uint64_t) (int64_t) (int32_t) b, CPU_X86_64_WIDTH_64);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8: {
            Cpu_x86_64_WriteRm(cpu, &insn, next, Cpu_x86_64_ReadReg(cpu, insn.ci_reg, CPU_X86_64_WIDTH_8), CPU_X86_64_WIDTH_8);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_R: {
            Cpu_x86_64_WriteRm(cpu, &insn, next, Cpu_x86_64_ReadReg(cpu, insn.ci_reg, width), width);
        } break;
        case ENC_X86_64_OPCODE_MOV_R_RM: {
            Cpu_x86_64_WriteReg(cpu, insn.ci_reg, Cpu_x86_64_ReadRm(cpu, &insn, next, width), width);
        } break;
        case ENC_X86_64_OPCODE_LEA_R_M: {
            Cpu_x86_64_WriteReg(cpu, insn.ci_reg, Cpu_x86_64_RmAddr(cpu, &insn, next), width);
        } break;
        case ENC_X86_64_OPCODE_CQO: {
            uint64_t a = Cpu_x86_64_ReadReg(cpu, CPU_X86_64_REG_RAX, width);
            bool neg = (a >> (width - 1)) & 1;
            Cpu_x86_64_WriteReg(cpu, CPU_X86_64_REG_RDX, neg ? CPU_X86_64_MASK_64 : 0, width);
        } break;
        case ENC_X86_64_OPCODE_MOV_R8_IMM8: {
            Cpu_x86_64_WriteReg(cpu, insn.ci_rm, insn.ci_imm, CPU_X86_64_WIDTH_8);
        } break;
        case ENC_X86_64_OPCODE_MOV_R_IMM64: {
            Cpu_x86_64_WriteReg(cpu, insn.ci_rm, insn.ci_imm, width);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_IMM32: {
            Cpu_x86_64_WriteRm(cpu, &insn, next, insn.ci_imm, width);
        } break;
        case ENC_X86_64_OPCODE_RET: {
            cpu->cs_rip = Cpu_x86_64_ReadMem(cpu, *rsp, CPU_X86_64_WIDTH_64);
            *rsp += CPU_X86_64_STACK_SLOT;
        } break;
        case ENC_X86_64_OPCODE_CALL_REL32: {
            *rsp -= CPU_X86_64_STACK_SLOT;
            Cpu_x86_64_WriteMem(cpu, *rsp, next, CPU_X86_64_WIDTH_64);
            cpu->cs_rip = next + insn.ci_imm;
        } break;
        case ENC_X86_64_OPCODE_JMP_REL32: {
            cpu->cs_rip = next + insn.ci_imm;
        } break;
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
            uint64_t b = (uint64_t) insn.ci_imm;
            switch (insn.ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_ADD: {
                    Cpu_x86_64_FlagsAdd(cpu, a, b, width);
                    Cpu_x86_64_WriteRm(cpu, &insn, next, a + b, width);
                } break;
                case ENC_X86_64_GRP_SUB: {
                    Cpu_x86_64_FlagsSub(cpu, a, b, width);
                    Cpu_x86_64_WriteRm(cpu, &insn, next, a - b, width);
                } break;
                case ENC_X86_64_GRP_CMP: {
                    Cpu_x86_64_FlagsSub(cpu, a, b, width);
                } break;
                default: {
                    Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_GROUP1_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            if ((insn.ci_reg & ENC_X86_64_REG_MASK) != ENC_X86_64_GRP_CALL) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_GROUP5_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            uint64_t target = Cpu_x86_64_ReadRm(cpu, &insn, next, CPU_X86_64_WIDTH_64);
            *rsp -= CPU_X86_64_STACK_SLOT;
            Cpu_x86_64_WriteMem(cpu, *rsp, next, CPU_X86_64_WIDTH_64);
            cpu->cs_rip = target;
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn.ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NEG: {
                    uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
                    Cpu_x86_64_FlagsSub(cpu, 0, a, width);
                    Cpu_x86_64_WriteRm(cpu, &insn, next, 0 - a, width);
                } break;
                case ENC_X86_64_GRP_NOT: {
                    uint64_t a = Cpu_x86_64_ReadRm(cpu, &insn, next, width);
                    Cpu_x86_64_WriteRm(cpu, &insn, next, ~a, width);
                } break;
                case ENC_X86_64_GRP_IDIV:
                case ENC_X86_64_GRP_DIV: {
                    Cpu_x86_64_Divide(cpu, &insn, next, rip, width);
                } break;
                default: {
                    Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_GROUP3_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                }
            }
        } break;
        case ENC_X86_64_OPCODE_X87_D9:
        case ENC_X86_64_OPCODE_X87_DB:
        case ENC_X86_64_OPCODE_X87_DD:
        case ENC_X86_64_OPCODE_X87_DE:
        case ENC_X86_64_OPCODE_X87_DF: {
            Cpu_x86_64_StepX87(cpu, &insn, next, rip);
        } break;
        default: {
            Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_OPCODE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        }
    }
}

// Read a little-endian signed value of n bytes.
int64_t Cpu_x86_64_ReadImm(const uint8_t *p, size_t n)
{
    uint64_t val = 0;
    for (size_t i = 0; i < n; i++) {
        val |= (uint64_t) p[i] << (CPU_X86_64_BITS_PER_BYTE * i);
    }
    size_t bits = CPU_X86_64_BITS_PER_BYTE * n;
    if (n < CPU_X86_64_IMM64 && (val >> (bits - 1)) & 1) {
        val |= CPU_X86_64_MASK_64 << bits;
    }
    return (int64_t) val;
}

// Return whether a one-byte opcode is followed by a ModRM byte.
bool Cpu_x86_64_HasModRM(int32_t op)
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
            return true;
        } break;
        default: {
            return Cpu_x86_64_IsX87(op);
        }
    }
}

// Return whether a one-byte opcode is an x87 escape the interpreter answers.
bool Cpu_x86_64_IsX87(int32_t op)
{
    return op == ENC_X86_64_OPCODE_X87_D9 || op == ENC_X86_64_OPCODE_X87_DB || op == ENC_X86_64_OPCODE_X87_DD
        || op == ENC_X86_64_OPCODE_X87_DE || op == ENC_X86_64_OPCODE_X87_DF;
}

// Return whether a two-byte opcode is a scalar SSE operation or conversion.
bool Cpu_x86_64_IsSse(int32_t op2)
{
    switch (op2) {
        case ENC_X86_64_OPCODE2_CVTSI2S:
        case ENC_X86_64_OPCODE2_CVTTS2SI:
        case ENC_X86_64_OPCODE2_UCOMIS:
        case ENC_X86_64_OPCODE2_ADDS:
        case ENC_X86_64_OPCODE2_MULS:
        case ENC_X86_64_OPCODE2_CVTS2S:
        case ENC_X86_64_OPCODE2_SUBS:
        case ENC_X86_64_OPCODE2_DIVS:
        case ENC_X86_64_OPCODE2_MOVQ_XMM_RM:
        case ENC_X86_64_OPCODE2_MOVQ_RM_XMM: {
            return true;
        } break;
        default: {
            return false;
        }
    }
}

// Return whether a two-byte opcode is followed by a ModRM byte.
bool Cpu_x86_64_HasModRM2(int32_t op2)
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
        case ENC_X86_64_OPCODE2_MOVSX_R_RM8:
        case ENC_X86_64_OPCODE2_SETA:
        case ENC_X86_64_OPCODE2_SETAE:
        case ENC_X86_64_OPCODE2_SETP:
        case ENC_X86_64_OPCODE2_SETNP: {
            return true;
        } break;
        default: {
            return Cpu_x86_64_IsSse(op2);
        }
    }
}

// Decode the ModRM byte at p and return the bytes consumed.
size_t Cpu_x86_64_DecodeModRM(const uint8_t *p, size_t avail, uint8_t rex, Cpu_x86_64_Insn *insn)
{
    if (avail < 1) {
        return 0;
    }
    uint8_t modrm = p[0];
    Enc_x86_64_Mod mod = modrm >> ENC_X86_64_MOD_SHIFT;
    uint8_t rm = modrm & ENC_X86_64_REG_MASK;
    size_t n = 1;

    insn->ci_reg = ((modrm >> ENC_X86_64_REG_SHIFT) & ENC_X86_64_REG_MASK)
                 | ((rex & ENC_X86_64_REX_R) ? CPU_X86_64_REG_HIGH_BIT : 0);

    if (mod == ENC_X86_64_MOD_DIRECT) {
        insn->ci_rmkind = CPU_X86_64_RM_REG;
        insn->ci_rm     = rm | ((rex & ENC_X86_64_REX_B) ? CPU_X86_64_REG_HIGH_BIT : 0);
        return n;
    }

    uint8_t base = rm;
    if (rm == (ENC_X86_64_SIB_BASE_RSP & ENC_X86_64_REG_MASK)) {
        if (avail < n + 1) {
            return 0;
        }
        base = p[n] & ENC_X86_64_REG_MASK;
        n++;
    }

    if (mod == ENC_X86_64_MOD_INDIRECT && rm == ENC_X86_64_RM_RIP) {
        if (avail < n + CPU_X86_64_IMM32) {
            return 0;
        }
        insn->ci_rmkind = CPU_X86_64_RM_RIP;
        insn->ci_disp   = (int32_t) Cpu_x86_64_ReadImm(p + n, CPU_X86_64_IMM32);
        return n + CPU_X86_64_IMM32;
    }

    insn->ci_rmkind = CPU_X86_64_RM_MEM;
    insn->ci_rm     = base | ((rex & ENC_X86_64_REX_B) ? CPU_X86_64_REG_HIGH_BIT : 0);

    if (mod == ENC_X86_64_MOD_DISP8) {
        if (avail < n + 1) {
            return 0;
        }
        insn->ci_disp = (int32_t) Cpu_x86_64_ReadImm(p + n, CPU_X86_64_IMM8);
        n += CPU_X86_64_IMM8;
    } else if (mod == ENC_X86_64_MOD_DISP32) {
        if (avail < n + CPU_X86_64_IMM32) {
            return 0;
        }
        insn->ci_disp = (int32_t) Cpu_x86_64_ReadImm(p + n, CPU_X86_64_IMM32);
        n += CPU_X86_64_IMM32;
    }
    return n;
}

// Return the operand width an instruction selects.
Cpu_x86_64_OperandWidth Cpu_x86_64_Width(const Cpu_x86_64_Insn *insn)
{
    if (insn->ci_rexw) {
        return CPU_X86_64_WIDTH_64;
    }
    return insn->ci_opsize16 ? CPU_X86_64_WIDTH_16 : CPU_X86_64_WIDTH_32;
}

// Decode one instruction and return its length.
size_t Cpu_x86_64_Decode(const uint8_t *code, size_t avail, Cpu_x86_64_Insn *insn)
{
    memset(insn, 0, sizeof(*insn));
    insn->ci_op2    = CPU_X86_64_NO_OPCODE2;
    insn->ci_rmkind = CPU_X86_64_RM_NONE;

    size_t n = 0;
    uint8_t rex = 0;
    for (; avail > n; n++) {
        if (code[n] == ENC_X86_64_OPCODE_OPSIZE) {
            insn->ci_opsize16 = true;
        } else if (code[n] == ENC_X86_64_OPCODE_SSE_DOUBLE || code[n] == ENC_X86_64_OPCODE_SSE_SINGLE) {
            insn->ci_rep = code[n];
        } else {
            break;
        }
    }
    if (avail > n && (code[n] & CPU_X86_64_REX_PREFIX_MASK) == ENC_X86_64_REX_BASE) {
        rex = code[n++];
        insn->ci_rexw = (rex & ENC_X86_64_REX_W) != 0;
    }
    if (avail < n + 1) {
        return 0;
    }

    int32_t op = code[n++];
    insn->ci_op = op;

    if (op == ENC_X86_64_OPCODE_ESCAPE) {
        if (avail < n + 1) {
            return 0;
        }
        int32_t op2 = code[n++];
        insn->ci_op2 = op2;
        if (Cpu_x86_64_HasModRM2(op2)) {
            size_t used = Cpu_x86_64_DecodeModRM(code + n, avail - n, rex, insn);
            if (! used) {
                return 0;
            }
            n += used;
        } else if (op2 == ENC_X86_64_OPCODE2_JE_REL32 || op2 == ENC_X86_64_OPCODE2_JNE_REL32) {
            if (avail < n + CPU_X86_64_IMM32) {
                return 0;
            }
            insn->ci_imm = Cpu_x86_64_ReadImm(code + n, CPU_X86_64_IMM32);
            n += CPU_X86_64_IMM32;
        } else if (op2 != ENC_X86_64_OPCODE2_SYSCALL) {
            return 0;
        }
        insn->ci_len = n;
        return n;
    }

    if ((op & CPU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_PUSH_R || (op & CPU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_POP_R) {
        insn->ci_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? CPU_X86_64_REG_HIGH_BIT : 0);
        insn->ci_op     = op & CPU_X86_64_OPCODE_REG_MASK;
        insn->ci_rmkind = CPU_X86_64_RM_REG;
        insn->ci_len    = n;
        return n;
    }
    if ((op & CPU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R8_IMM8 || (op & CPU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R_IMM64) {
        bool wide = (op & CPU_X86_64_OPCODE_REG_MASK) == ENC_X86_64_OPCODE_MOV_R_IMM64;
        size_t size = wide ? (insn->ci_rexw ? CPU_X86_64_IMM64 : CPU_X86_64_IMM32) : CPU_X86_64_IMM8;
        if (avail < n + size) {
            return 0;
        }
        insn->ci_rm     = (op & ENC_X86_64_REG_MASK) | ((rex & ENC_X86_64_REX_B) ? CPU_X86_64_REG_HIGH_BIT : 0);
        insn->ci_op     = op & CPU_X86_64_OPCODE_REG_MASK;
        insn->ci_rmkind = CPU_X86_64_RM_REG;
        insn->ci_imm    = Cpu_x86_64_ReadImm(code + n, size);
        insn->ci_len    = n + size;
        return insn->ci_len;
    }

    if (Cpu_x86_64_HasModRM(op)) {
        size_t used = Cpu_x86_64_DecodeModRM(code + n, avail - n, rex, insn);
        if (! used) {
            return 0;
        }
        n += used;
        if (op == ENC_X86_64_OPCODE_MOV_RM_IMM32 || op == ENC_X86_64_OPCODE_GRP1_RM_IMM32) {
            if (avail < n + CPU_X86_64_IMM32) {
                return 0;
            }
            insn->ci_imm = Cpu_x86_64_ReadImm(code + n, CPU_X86_64_IMM32);
            n += CPU_X86_64_IMM32;
        }
        insn->ci_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_CALL_REL32 || op == ENC_X86_64_OPCODE_JMP_REL32) {
        if (avail < n + CPU_X86_64_IMM32) {
            return 0;
        }
        insn->ci_imm = Cpu_x86_64_ReadImm(code + n, CPU_X86_64_IMM32);
        n += CPU_X86_64_IMM32;
        insn->ci_len = n;
        return n;
    }

    if (op == ENC_X86_64_OPCODE_RET || op == ENC_X86_64_OPCODE_CQO) {
        insn->ci_len = n;
        return n;
    }
    return 0;
}

// Return the name of a register at the given width.
const char *Cpu_x86_64_RegName(Cpu_x86_64_Reg reg, Cpu_x86_64_OperandWidth width)
{
    switch (width) {
        case CPU_X86_64_WIDTH_8: {
            return Cpu_x86_64_Name8[reg & CPU_X86_64_REG_INDEX_MASK];
        } break;
        case CPU_X86_64_WIDTH_16: {
            return Cpu_x86_64_Name16[reg & CPU_X86_64_REG_INDEX_MASK];
        } break;
        case CPU_X86_64_WIDTH_32: {
            return Cpu_x86_64_Name32[reg & CPU_X86_64_REG_INDEX_MASK];
        } break;
        default: {
            return Cpu_x86_64_Name64[reg & CPU_X86_64_REG_INDEX_MASK];
        }
    }
}

// Name the operation a decoded instruction performs.
const char *Cpu_x86_64_Mnemonic(const Cpu_x86_64_Insn *insn)
{
    if (insn->ci_op == ENC_X86_64_OPCODE_ESCAPE) {
        switch (insn->ci_op2) {
            case ENC_X86_64_OPCODE2_SYSCALL: {
                return "syscall";
            } break;
            case ENC_X86_64_OPCODE2_JE_REL32: {
                return "je";
            } break;
            case ENC_X86_64_OPCODE2_JNE_REL32: {
                return "jne";
            } break;
            case ENC_X86_64_OPCODE2_SETE: {
                return "sete";
            } break;
            case ENC_X86_64_OPCODE2_SETNE: {
                return "setne";
            } break;
            case ENC_X86_64_OPCODE2_SETL: {
                return "setl";
            } break;
            case ENC_X86_64_OPCODE2_SETLE: {
                return "setle";
            } break;
            case ENC_X86_64_OPCODE2_SETB: {
                return "setb";
            } break;
            case ENC_X86_64_OPCODE2_SETBE: {
                return "setbe";
            } break;
            case ENC_X86_64_OPCODE2_IMUL_R_RM: {
                return "imul";
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM8: {
                return "movzbq";
            } break;
            case ENC_X86_64_OPCODE2_MOVZX_R_RM16: {
                return "movzwq";
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM8: {
                return "movsbq";
            } break;
            case ENC_X86_64_OPCODE2_MOVSX_R_RM16: {
                return "movswq";
            } break;
            case ENC_X86_64_OPCODE2_SETA: {
                return "seta";
            } break;
            case ENC_X86_64_OPCODE2_SETAE: {
                return "setae";
            } break;
            case ENC_X86_64_OPCODE2_SETP: {
                return "setp";
            } break;
            case ENC_X86_64_OPCODE2_SETNP: {
                return "setnp";
            } break;
            default: {
                return Cpu_x86_64_SseMnemonic(insn);
            }
        }
    }
    if (Cpu_x86_64_IsX87(insn->ci_op)) {
        return Cpu_x86_64_X87Mnemonic(insn);
    }

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_ADD_RM_R: {
            return "add";
        } break;
        case ENC_X86_64_OPCODE_OR_RM_R: {
            return "or";
        } break;
        case ENC_X86_64_OPCODE_AND_RM_R: {
            return "and";
        } break;
        case ENC_X86_64_OPCODE_XOR_RM_R: {
            return "xor";
        } break;
        case ENC_X86_64_OPCODE_SUB_RM_R: {
            return "sub";
        } break;
        case ENC_X86_64_OPCODE_CMP_RM_R: {
            return "cmp";
        } break;
        case ENC_X86_64_OPCODE_PUSH_R: {
            return "push";
        } break;
        case ENC_X86_64_OPCODE_POP_R: {
            return "pop";
        } break;
        case ENC_X86_64_OPCODE_MOVSXD_R_RM32: {
            return "movslq";
        } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8:
        case ENC_X86_64_OPCODE_MOV_RM_R:
        case ENC_X86_64_OPCODE_MOV_R_RM:
        case ENC_X86_64_OPCODE_MOV_R8_IMM8:
        case ENC_X86_64_OPCODE_MOV_R_IMM64:
        case ENC_X86_64_OPCODE_MOV_RM_IMM32: {
            return "mov";
        } break;
        case ENC_X86_64_OPCODE_LEA_R_M: {
            return "lea";
        } break;
        case ENC_X86_64_OPCODE_CQO: {
            return "cqto";
        } break;
        case ENC_X86_64_OPCODE_RET: {
            return "ret";
        } break;
        case ENC_X86_64_OPCODE_CALL_REL32: {
            return "call";
        } break;
        case ENC_X86_64_OPCODE_JMP_REL32: {
            return "jmp";
        } break;
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            switch (insn->ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_ADD: {
                    return "add";
                } break;
                case ENC_X86_64_GRP_SUB: {
                    return "sub";
                } break;
                case ENC_X86_64_GRP_CMP: {
                    return "cmp";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            switch (insn->ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_SHL: {
                    return "shl";
                } break;
                case ENC_X86_64_GRP_SAR: {
                    return "sar";
                } break;
                case ENC_X86_64_GRP_SHR: {
                    return "shr";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            switch (insn->ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_CALL: {
                    return "call";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            switch (insn->ci_reg & ENC_X86_64_REG_MASK) {
                case ENC_X86_64_GRP_NOT: {
                    return "not";
                } break;
                case ENC_X86_64_GRP_NEG: {
                    return "neg";
                } break;
                case ENC_X86_64_GRP_IDIV: {
                    return "idiv";
                } break;
                case ENC_X86_64_GRP_DIV: {
                    return "div";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        default: {
            return "(bad)";
        }
    }
}

// Name the scalar SSE operation a decoded instruction performs.
const char *Cpu_x86_64_SseMnemonic(const Cpu_x86_64_Insn *insn)
{
    bool dbl = insn->ci_rep == ENC_X86_64_OPCODE_SSE_DOUBLE || (insn->ci_rep == 0 && insn->ci_opsize16);

    switch (insn->ci_op2) {
        case ENC_X86_64_OPCODE2_ADDS: {
            return dbl ? "addsd" : "addss";
        } break;
        case ENC_X86_64_OPCODE2_SUBS: {
            return dbl ? "subsd" : "subss";
        } break;
        case ENC_X86_64_OPCODE2_MULS: {
            return dbl ? "mulsd" : "mulss";
        } break;
        case ENC_X86_64_OPCODE2_DIVS: {
            return dbl ? "divsd" : "divss";
        } break;
        case ENC_X86_64_OPCODE2_UCOMIS: {
            return dbl ? "ucomisd" : "ucomiss";
        } break;
        case ENC_X86_64_OPCODE2_CVTS2S: {
            return dbl ? "cvtsd2ss" : "cvtss2sd";
        } break;
        case ENC_X86_64_OPCODE2_CVTSI2S: {
            return dbl ? "cvtsi2sd" : "cvtsi2ss";
        } break;
        case ENC_X86_64_OPCODE2_CVTTS2SI: {
            return dbl ? "cvttsd2si" : "cvttss2si";
        } break;
        case ENC_X86_64_OPCODE2_MOVQ_XMM_RM:
        case ENC_X86_64_OPCODE2_MOVQ_RM_XMM: {
            return insn->ci_rexw ? "movq" : "movd";
        } break;
        default: {
            return "(bad)";
        }
    }
}

// Name the x87 operation a decoded instruction performs.
const char *Cpu_x86_64_X87Mnemonic(const Cpu_x86_64_Insn *insn)
{
    int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;
    bool mem = insn->ci_rmkind != CPU_X86_64_RM_REG;

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_X87_D9: {
            if (! mem) {
                return (CPU_X86_64_X87_REG_FORM | digit << ENC_X86_64_REG_SHIFT) == ENC_X86_64_X87_FCHS ? "fchs" : "(bad)";
            }
            return digit == ENC_X86_64_X87_FLD_M32 ? "flds" : digit == ENC_X86_64_X87_FSTP_M32 ? "fstps" : "(bad)";
        } break;
        case ENC_X86_64_OPCODE_X87_DB: {
            return digit == ENC_X86_64_X87_FLD_M80 ? "fldt" : digit == ENC_X86_64_X87_FSTP_M80 ? "fstpt" : "(bad)";
        } break;
        case ENC_X86_64_OPCODE_X87_DD: {
            if (! mem) {
                return "fstp";
            }
            return digit == ENC_X86_64_X87_FLD_M64 ? "fldl" : digit == ENC_X86_64_X87_FSTP_M64 ? "fstpl"
                 : digit == ENC_X86_64_X87_FISTTP_M64 ? "fisttpq" : "(bad)";
        } break;
        case ENC_X86_64_OPCODE_X87_DE: {
            switch ((CPU_X86_64_X87_REG_FORM | digit << ENC_X86_64_REG_SHIFT)) {
                case ENC_X86_64_X87_FADDP: {
                    return "faddp";
                } break;
                case ENC_X86_64_X87_FMULP: {
                    return "fmulp";
                } break;
                case ENC_X86_64_X87_FSUBRP: {
                    return "fsubrp";
                } break;
                case ENC_X86_64_X87_FDIVRP: {
                    return "fdivrp";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        case ENC_X86_64_OPCODE_X87_DF: {
            return mem ? "fildq" : "fucomip";
        } break;
        default: {
            return "(bad)";
        }
    }
}

// Write the r/m operand into out, as a register, disp(%base) or disp(%rip).
void Cpu_x86_64_FormatRm(const Cpu_x86_64_Insn *insn, Cpu_x86_64_OperandWidth width, uint64_t next, char *out, size_t n)
{
    switch (insn->ci_rmkind) {
        case CPU_X86_64_RM_REG: {
            snprintf(out, n, "%%%s", Cpu_x86_64_RegName(insn->ci_rm, width));
        } break;
        case CPU_X86_64_RM_MEM: {
            const char *base = Cpu_x86_64_RegName(insn->ci_rm, CPU_X86_64_WIDTH_64);
            if (insn->ci_disp) {
                snprintf(out, n, "%d(%%%s)", insn->ci_disp, base);
            } else {
                snprintf(out, n, "(%%%s)", base);
            }
        } break;
        case CPU_X86_64_RM_RIP: {
            snprintf(out, n, "0x%llx(%%rip)", (Cpu_x86_64_TypeULLong) (next + (int64_t) insn->ci_disp));
        } break;
        default: {
            snprintf(out, n, "?");
        }
    }
}

// Write one decoded instruction in AT&T syntax.
void Cpu_x86_64_Format(const Cpu_x86_64_Insn *insn, uint64_t rip, char *out, size_t n)
{
    const char *name = Cpu_x86_64_Mnemonic(insn);
    uint64_t next = rip + insn->ci_len;
    Cpu_x86_64_OperandWidth width = Cpu_x86_64_Width(insn);
    char rm[64];

    if (insn->ci_op == ENC_X86_64_OPCODE_CALL_REL32 || insn->ci_op == ENC_X86_64_OPCODE_JMP_REL32
        || insn->ci_op2 == ENC_X86_64_OPCODE2_JE_REL32 || insn->ci_op2 == ENC_X86_64_OPCODE2_JNE_REL32) {
        snprintf(out, n, "%s 0x%llx", name, (Cpu_x86_64_TypeULLong) (next + insn->ci_imm));
        return;
    }
    if (insn->ci_rmkind == CPU_X86_64_RM_NONE) {
        snprintf(out, n, "%s", name);
        return;
    }
    if (Cpu_x86_64_IsX87(insn->ci_op)) {
        if (insn->ci_op == ENC_X86_64_OPCODE_X87_D9 && insn->ci_rmkind == CPU_X86_64_RM_REG) {
            snprintf(out, n, "%s", name);
        } else if (insn->ci_rmkind == CPU_X86_64_RM_REG) {
            snprintf(out, n, "%s %%st(%d)", name, insn->ci_rm & ENC_X86_64_REG_MASK);
        } else {
            Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_64, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s", name, rm);
        }
        return;
    }
    if (insn->ci_op == ENC_X86_64_OPCODE_ESCAPE && Cpu_x86_64_IsSse(insn->ci_op2)) {
        bool gprm = insn->ci_op2 == ENC_X86_64_OPCODE2_CVTSI2S || insn->ci_op2 == ENC_X86_64_OPCODE2_MOVQ_XMM_RM
                 || insn->ci_op2 == ENC_X86_64_OPCODE2_MOVQ_RM_XMM;
        bool gpreg = insn->ci_op2 == ENC_X86_64_OPCODE2_CVTTS2SI;
        char reg[16];
        if (insn->ci_rmkind == CPU_X86_64_RM_REG && ! gprm) {
            snprintf(rm, sizeof(rm), "%%xmm%d", insn->ci_rm & CPU_X86_64_REG_INDEX_MASK);
        } else {
            Cpu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
        }
        if (gpreg) {
            snprintf(reg, sizeof(reg), "%%%s", Cpu_x86_64_RegName(insn->ci_reg, width));
        } else {
            snprintf(reg, sizeof(reg), "%%xmm%d", insn->ci_reg & CPU_X86_64_REG_INDEX_MASK);
        }
        if (insn->ci_op2 == ENC_X86_64_OPCODE2_MOVQ_RM_XMM) {
            snprintf(out, n, "%s %s, %s", name, reg, rm);
        } else {
            snprintf(out, n, "%s %s, %s", name, rm, reg);
        }
        return;
    }

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_PUSH_R:
        case ENC_X86_64_OPCODE_POP_R: {
            snprintf(out, n, "%s %%%s", name, Cpu_x86_64_RegName(insn->ci_rm, CPU_X86_64_WIDTH_64));
        } break;
        case ENC_X86_64_OPCODE_MOV_R8_IMM8: {
            snprintf(out, n, "%s $0x%llx, %%%s", name, (Cpu_x86_64_TypeULLong) insn->ci_imm, Cpu_x86_64_RegName(insn->ci_rm, CPU_X86_64_WIDTH_8));
        } break;
        case ENC_X86_64_OPCODE_MOV_R_IMM64: {
            snprintf(out, n, "%s $0x%llx, %%%s", name, (Cpu_x86_64_TypeULLong) insn->ci_imm, Cpu_x86_64_RegName(insn->ci_rm, width));
        } break;
        case ENC_X86_64_OPCODE_MOV_RM_IMM32:
        case ENC_X86_64_OPCODE_GRP1_RM_IMM32: {
            Cpu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s $0x%llx, %s", name, (Cpu_x86_64_TypeULLong) insn->ci_imm, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP3_RM: {
            Cpu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP5_RM: {
            Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_64, next, rm, sizeof(rm));
            snprintf(out, n, "%s *%s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_GRP2_RM_CL: {
            Cpu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%cl, %s", name, rm);
        } break;
        case ENC_X86_64_OPCODE_MOV_RM8_R8: {
            Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_8, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Cpu_x86_64_RegName(insn->ci_reg, CPU_X86_64_WIDTH_8), rm);
        } break;
        case ENC_X86_64_OPCODE_ADD_RM_R:
        case ENC_X86_64_OPCODE_OR_RM_R:
        case ENC_X86_64_OPCODE_AND_RM_R:
        case ENC_X86_64_OPCODE_SUB_RM_R:
        case ENC_X86_64_OPCODE_XOR_RM_R:
        case ENC_X86_64_OPCODE_CMP_RM_R:
        case ENC_X86_64_OPCODE_MOV_RM_R: {
            Cpu_x86_64_FormatRm(insn, width, next, rm, sizeof(rm));
            snprintf(out, n, "%s %%%s, %s", name, Cpu_x86_64_RegName(insn->ci_reg, width), rm);
        } break;
        default: {
            Cpu_x86_64_OperandWidth srcw = width;
            if (insn->ci_op2 == ENC_X86_64_OPCODE2_MOVZX_R_RM8 || insn->ci_op2 == ENC_X86_64_OPCODE2_MOVSX_R_RM8) {
                srcw = CPU_X86_64_WIDTH_8;
            } else if (insn->ci_op2 == ENC_X86_64_OPCODE2_MOVZX_R_RM16 || insn->ci_op2 == ENC_X86_64_OPCODE2_MOVSX_R_RM16) {
                srcw = CPU_X86_64_WIDTH_16;
            } else if (insn->ci_op == ENC_X86_64_OPCODE_MOVSXD_R_RM32) {
                srcw = CPU_X86_64_WIDTH_32;
            }
            if (insn->ci_op2 >= ENC_X86_64_OPCODE2_SETB && insn->ci_op2 <= ENC_X86_64_OPCODE2_SETLE) {
                Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_8, next, rm, sizeof(rm));
                snprintf(out, n, "%s %s", name, rm);
                return;
            }
            Cpu_x86_64_FormatRm(insn, srcw, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s, %%%s", name, rm, Cpu_x86_64_RegName(insn->ci_reg, width));
        }
    }
}
