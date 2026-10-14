/*
 * C source file for the x86-64 CPU.
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
    cpu->cs_fcw = CPU_X86_64_FCW_INIT;
    cpu->cs_mxcsr = CPU_X86_64_MXCSR_INIT;
}

// Release what a CPU holds.
void Cpu_x86_64_Free(Cpu_x86_64_State *cpu)
{
    free(cpu->cs_fault);
    cpu->cs_fault = NULL;
}

// Record the first fault of a step and stop the step with its exception.
void Cpu_x86_64_Fault(Cpu_x86_64_State *cpu, Cpu_x86_64_Vector vector, Err_Code code, ...)
{
    va_list ap;

    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    free(cpu->cs_fault);
    va_start(ap, code);
    cpu->cs_fault = Err_FormatVa(code, ap);
    va_end(ap);
    cpu->cs_trap = CPU_X86_64_TRAP_EXCEPTION;
    cpu->cs_vector = vector;
}

// Record the address and error code of a step's first #PF.
void Cpu_x86_64_PageFault(Cpu_x86_64_State *cpu, uint64_t addr, uint64_t error)
{
    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    cpu->cs_cr2 = addr;
    cpu->cs_pf_err = error;
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
        Cpu_x86_64_PageFault(cpu, addr, CPU_X86_64_PF_USER);
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
        Cpu_x86_64_PageFault(cpu, addr, CPU_X86_64_PF_USER | CPU_X86_64_PF_WRITE);
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

// Set the flags an unordered comparison leaves behind.
void Cpu_x86_64_FlagsOrder(Cpu_x86_64_State *cpu, Fp_Order order)
{
    bool unordered = order == FP_ORDER_UNORDERED;

    cpu->cs_zf = unordered || order == FP_ORDER_EQUAL;
    cpu->cs_pf = unordered;
    cpu->cs_cf = unordered || order == FP_ORDER_LESS;
    cpu->cs_sf = false;
    cpu->cs_of = false;
}

// Return the mode a rounding control names, MXCSR's RC and the x87's alike.
Fp_RoundMode Cpu_x86_64_RoundMode(uint32_t rc)
{
    static const Fp_RoundMode modes[CPU_X86_64_MXCSR_RC_MASK + 1] = {
        FP_ROUND_NEAREST_EVEN, FP_ROUND_DOWN, FP_ROUND_UP, FP_ROUND_ZERO
    };
    return modes[rc & CPU_X86_64_MXCSR_RC_MASK];
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

// Return how an SSE operation rounds: MXCSR's mode, tiny after rounding.
Fp_Context Cpu_x86_64_SseContext(const Cpu_x86_64_State *cpu)
{
    Fp_Context ctx = {
        .fc_mode      = Cpu_x86_64_RoundMode(cpu->cs_mxcsr >> CPU_X86_64_MXCSR_RC_SHIFT),
        .fc_tininess  = FP_ROUND_TINY_AFTER,
        .fc_flags     = 0
    };
    return ctx;
}

// Unpack an SSE operand: a zero for a denormal under DAZ, else DE noted.
Fp_Value Cpu_x86_64_SseUnpack(const Cpu_x86_64_State *cpu, const Fp_Format *fmt, uint64_t bits, uint32_t *denormal)
{
    Fp_Value value = Fp_FormatUnpack(fmt, Fp_WideMake(0, bits));

    if (value.fv_class != FP_CLASS_SUBNORMAL) {
        return value;
    }
    if (cpu->cs_mxcsr & CPU_X86_64_MXCSR_DAZ) {
        return Fp_FormatMake(value.fv_sign, Fp_WideMake(0, 0), 0);
    }
    *denormal = CPU_X86_64_MXCSR_DE;
    return value;
}

// Return an SSE operation's NaN: its first NaN operand quieted, or indefinite.
Fp_Value Cpu_x86_64_SseQuiet(const Fp_Value *a, const Fp_Value *b)
{
    Fp_Value nan = Fp_FormatMake(FP_SIGN_NEGATIVE, Fp_WideMake(0, 0), 0);

    if (a->fv_class == FP_CLASS_NAN) {
        nan = *a;
    } else if (b->fv_class == FP_CLASS_NAN) {
        nan = *b;
    }
    nan.fv_class = FP_CLASS_NAN;
    nan.fv_mant.fw_hi |= FP_WORD_TOP;
    return nan;
}

// Settle an SSE operation's flags in MXCSR; return whether #XF spares it.
bool Cpu_x86_64_SseSettle(Cpu_x86_64_State *cpu, const Fp_Context *ctx, uint32_t denormal, Fp_Value *result, uint64_t rip)
{
    uint32_t masks = cpu->cs_mxcsr >> CPU_X86_64_MXCSR_MASK_SHIFT & CPU_X86_64_MXCSR_FLAGS;
    uint32_t flags = ctx->fc_flags;
    uint32_t inexact = flags & FP_FLAG_INEXACT ? CPU_X86_64_MXCSR_PE : 0;
    uint32_t pre = denormal;
    uint32_t post = inexact;

    if (flags & FP_FLAG_INVALID) {
        pre = CPU_X86_64_MXCSR_IE;
    } else if (flags & FP_FLAG_DIVIDE) {
        pre = CPU_X86_64_MXCSR_ZE;
    }
    if (pre & ~masks) {
        cpu->cs_mxcsr |= pre;
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_XF, ERR_CPU_SIMD_FLOAT_OPERAND_UNMASKED, (Cpu_x86_64_TypeULLong) cpu->cs_mxcsr, (Cpu_x86_64_TypeULLong) rip);
        return false;
    }
    if (flags & FP_FLAG_OVERFLOW) {
        post = masks & CPU_X86_64_MXCSR_OE ? CPU_X86_64_MXCSR_OE | CPU_X86_64_MXCSR_PE : CPU_X86_64_MXCSR_OE;
    } else if ((flags & FP_FLAG_TINY) && ! (masks & CPU_X86_64_MXCSR_UE)) {
        post = CPU_X86_64_MXCSR_UE | inexact;
    } else if ((flags & FP_FLAG_TINY) && (cpu->cs_mxcsr & CPU_X86_64_MXCSR_FZ)) {
        post = CPU_X86_64_MXCSR_UE | CPU_X86_64_MXCSR_PE;
        *result = Fp_FormatMake(result->fv_sign, Fp_WideMake(0, 0), 0);
    } else if (flags & FP_FLAG_UNDERFLOW) {
        post = CPU_X86_64_MXCSR_UE | inexact;
    }
    cpu->cs_mxcsr |= pre | post;
    if (post & ~masks) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_XF, ERR_CPU_SIMD_FLOAT_RESULT_UNMASKED, (Cpu_x86_64_TypeULLong) cpu->cs_mxcsr, (Cpu_x86_64_TypeULLong) rip);
        return false;
    }
    return true;
}

// Write a value into an SSE register's low lane of its format, the rest kept.
void Cpu_x86_64_SseWrite(uint64_t *dst, const Fp_Format *fmt, const Fp_Value *value)
{
    uint64_t bits = Fp_FormatPack(fmt, value).fw_lo;

    if (fmt->ff_bits == CPU_X86_64_WIDTH_64) {
        dst[0] = bits;
    } else {
        dst[0] = (dst[0] & ~(uint64_t) CPU_X86_64_MASK_32) | bits;
    }
}

// Execute one scalar SSE operation or conversion.
void Cpu_x86_64_StepSse(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    uint64_t *dst = cpu->cs_xmm[insn->ci_reg & CPU_X86_64_REG_INDEX_MASK];
    Cpu_x86_64_OperandWidth width = insn->ci_rexw ? CPU_X86_64_WIDTH_64 : CPU_X86_64_WIDTH_32;
    bool dbl = insn->ci_rep == ENC_X86_64_OPCODE_SSE_DOUBLE || (insn->ci_rep == 0 && insn->ci_opsize16);
    Cpu_x86_64_OperandWidth lane = dbl ? CPU_X86_64_WIDTH_64 : CPU_X86_64_WIDTH_32;
    const Fp_Format *fmt = dbl ? &Fp_FormatBinary64 : &Fp_FormatBinary32;
    const Fp_Format *other = dbl ? &Fp_FormatBinary32 : &Fp_FormatBinary64;
    Fp_Context ctx = Cpu_x86_64_SseContext(cpu);
    uint32_t denormal = 0;
    Fp_Value a;
    Fp_Value b;
    Fp_Value result;

    switch (insn->ci_op2) {
        case ENC_X86_64_OPCODE2_MOVQ_XMM_RM: {
            dst[0] = Cpu_x86_64_ReadRm(cpu, insn, next, width);
            dst[1] = 0;
        } break;
        case ENC_X86_64_OPCODE2_MOVQ_RM_XMM: {
            Cpu_x86_64_WriteRm(cpu, insn, next, dst[0], width);
        } break;
        case ENC_X86_64_OPCODE2_UCOMIS: {
            a = Cpu_x86_64_SseUnpack(cpu, fmt, dst[0] & (dbl ? ~(uint64_t) 0 : CPU_X86_64_MASK_32), &denormal);
            b = Cpu_x86_64_SseUnpack(cpu, fmt, Cpu_x86_64_ReadSse(cpu, insn, next, lane), &denormal);
            Fp_Order order = Fp_ArithCmp(&a, &b, FP_COMPARE_QUIET, &ctx);
            if (Cpu_x86_64_SseSettle(cpu, &ctx, denormal, &a, rip)) {
                Cpu_x86_64_FlagsOrder(cpu, order);
            }
        } break;
        case ENC_X86_64_OPCODE2_CVTSI2S: {
            uint64_t bits = Cpu_x86_64_ReadRm(cpu, insn, next, width);
            result = Fp_ToValue(width == CPU_X86_64_WIDTH_64 ? (int64_t) bits : (int32_t) bits, fmt, &ctx);
            if (Cpu_x86_64_SseSettle(cpu, &ctx, denormal, &result, rip)) {
                Cpu_x86_64_SseWrite(dst, fmt, &result);
            }
        } break;
        case ENC_X86_64_OPCODE2_CVTTS2SI: {
            b = Cpu_x86_64_SseUnpack(cpu, fmt, Cpu_x86_64_ReadSse(cpu, insn, next, lane), &denormal);
            ctx.fc_mode = FP_ROUND_ZERO;
            uint64_t integer = Fp_ToInteger(&b, width, &ctx);
            if (ctx.fc_flags & FP_FLAG_INVALID) {
                integer = width == CPU_X86_64_WIDTH_64 ? CPU_X86_64_INDEFINITE_64 : CPU_X86_64_INDEFINITE_32;
            }
            if (Cpu_x86_64_SseSettle(cpu, &ctx, 0, &b, rip)) {
                Cpu_x86_64_WriteReg(cpu, insn->ci_reg, integer, width);
            }
        } break;
        case ENC_X86_64_OPCODE2_CVTS2S: {
            b = Cpu_x86_64_SseUnpack(cpu, fmt, Cpu_x86_64_ReadSse(cpu, insn, next, lane), &denormal);
            result = Fp_RoundValue(&b, other, &ctx);
            if (b.fv_class == FP_CLASS_NAN) {
                result = Cpu_x86_64_SseQuiet(&b, &b);
            }
            if (Cpu_x86_64_SseSettle(cpu, &ctx, denormal, &result, rip)) {
                Cpu_x86_64_SseWrite(dst, other, &result);
            }
        } break;
        default: {
            a = Cpu_x86_64_SseUnpack(cpu, fmt, dst[0] & (dbl ? ~(uint64_t) 0 : CPU_X86_64_MASK_32), &denormal);
            b = Cpu_x86_64_SseUnpack(cpu, fmt, Cpu_x86_64_ReadSse(cpu, insn, next, lane), &denormal);
            switch (insn->ci_op2) {
                case ENC_X86_64_OPCODE2_ADDS: {
                    result = Fp_ArithAdd(&a, &b, fmt, &ctx);
                } break;
                case ENC_X86_64_OPCODE2_SUBS: {
                    result = Fp_ArithSub(&a, &b, fmt, &ctx);
                } break;
                case ENC_X86_64_OPCODE2_MULS: {
                    result = Fp_ArithMul(&a, &b, fmt, &ctx);
                } break;
                case ENC_X86_64_OPCODE2_DIVS: {
                    result = Fp_ArithDiv(&a, &b, fmt, &ctx);
                } break;
                default: {
                    if (dbl) {
                        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_SSE_DOUBLE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                    } else {
                        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                    }
                    return;
                }
            }
            if (result.fv_class == FP_CLASS_NAN) {
                result = Cpu_x86_64_SseQuiet(&a, &b);
            }
            if (Cpu_x86_64_SseSettle(cpu, &ctx, denormal, &result, rip)) {
                Cpu_x86_64_SseWrite(dst, fmt, &result);
            }
        } break;
    }
}

// Execute a group 15 operation: a load or store of MXCSR.
void Cpu_x86_64_StepGrp15(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;

    if (insn->ci_rmkind == CPU_X86_64_RM_REG || insn->ci_rep || insn->ci_opsize16
        || (digit != ENC_X86_64_GRP_LDMXCSR && digit != ENC_X86_64_GRP_STMXCSR)) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_GROUP15_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        return;
    }

    uint64_t addr = Cpu_x86_64_RmAddr(cpu, insn, next);
    if (digit == ENC_X86_64_GRP_STMXCSR) {
        Cpu_x86_64_WriteMem(cpu, addr, cpu->cs_mxcsr, CPU_X86_64_WIDTH_32);
        return;
    }

    uint64_t mxcsr = Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_32);
    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    if (mxcsr & ~(uint64_t) CPU_X86_64_MXCSR_MASK) {
        Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_GP, ERR_CPU_MXCSR_RESERVED, (Cpu_x86_64_TypeULLong) mxcsr, (Cpu_x86_64_TypeULLong) rip);
        return;
    }
    cpu->cs_mxcsr = (uint32_t) mxcsr;
}

// Return the x87 register %st(i) names.
Fp_Wide *Cpu_x86_64_St(Cpu_x86_64_State *cpu, int32_t i)
{
    return &cpu->cs_st[(cpu->cs_top + i) & CPU_X86_64_ST_MASK];
}

// Return whether %st(i) holds a value, %st(-1) being the one a push fills.
bool Cpu_x86_64_IsStFull(const Cpu_x86_64_State *cpu, int32_t i)
{
    return (cpu->cs_full >> ((cpu->cs_top + i) & CPU_X86_64_ST_MASK) & 1) != 0;
}

// Write %st(i), which then holds a value.
void Cpu_x86_64_StWrite(Cpu_x86_64_State *cpu, int32_t i, Fp_Wide bits)
{
    *Cpu_x86_64_St(cpu, i) = bits;
    cpu->cs_full |= (uint8_t) (1u << ((cpu->cs_top + i) & CPU_X86_64_ST_MASK));
}

// Push a value onto the x87 stack.
void Cpu_x86_64_StPush(Cpu_x86_64_State *cpu, Fp_Wide bits)
{
    cpu->cs_top = (cpu->cs_top - 1) & CPU_X86_64_ST_MASK;
    Cpu_x86_64_StWrite(cpu, 0, bits);
}

// Pop the x87 stack.
void Cpu_x86_64_StPop(Cpu_x86_64_State *cpu)
{
    cpu->cs_full &= (uint8_t) ~(1u << cpu->cs_top);
    cpu->cs_top = (cpu->cs_top + 1) & CPU_X86_64_ST_MASK;
}

// Raise an x87 operation's flags in the status word, its C1 among them.
void Cpu_x86_64_X87Status(Cpu_x86_64_State *cpu, uint32_t flags)
{
    cpu->cs_fsw = (uint16_t) ((cpu->cs_fsw & ~CPU_X86_64_FSW_C1) | flags);
}

// Fault a push onto a full register: IE, SF and C1; masked, the indefinite.
void Cpu_x86_64_X87Overflow(Cpu_x86_64_State *cpu)
{
    Cpu_x86_64_X87Status(cpu, CPU_X86_64_FSW_IE | CPU_X86_64_FSW_SF | CPU_X86_64_FSW_C1);
    if (cpu->cs_fcw & CPU_X86_64_FSW_IE) {
        Cpu_x86_64_StPush(cpu, Cpu_x86_64_X87Indefinite());
    }
}

// Fault a read of an empty register: IE and SF; return whether IE is masked.
bool Cpu_x86_64_X87Underflow(Cpu_x86_64_State *cpu)
{
    Cpu_x86_64_X87Status(cpu, CPU_X86_64_FSW_IE | CPU_X86_64_FSW_SF);
    return (cpu->cs_fcw & CPU_X86_64_FSW_IE) != 0;
}

// Return how an x87 operation rounds: RC's mode, tiny after rounding.
Fp_Context Cpu_x86_64_X87Context(const Cpu_x86_64_State *cpu)
{
    Fp_Context ctx = {
        .fc_mode      = Cpu_x86_64_RoundMode(cpu->cs_fcw >> CPU_X86_64_FCW_RC_SHIFT),
        .fc_tininess  = FP_ROUND_TINY_AFTER,
        .fc_flags     = 0
    };
    return ctx;
}

// Return the format PC rounds arithmetic to: its precision, binary80's range.
Fp_Format Cpu_x86_64_X87Precision(const Cpu_x86_64_State *cpu)
{
    Fp_Format fmt = Fp_FormatBinary80;

    switch (cpu->cs_fcw >> CPU_X86_64_FCW_PC_SHIFT & CPU_X86_64_FCW_FIELD) {
        case CPU_X86_64_FCW_PC_24: {
            fmt.ff_mant = Fp_FormatBinary32.ff_mant;
        } break;
        case CPU_X86_64_FCW_PC_53: {
            fmt.ff_mant = Fp_FormatBinary64.ff_mant;
        } break;
        default: {
        }
    }
    return fmt;
}

// Unpack an x87 register's bits, DE noted for a denormal or pseudo-denormal.
Fp_Value Cpu_x86_64_X87Unpack(Fp_Wide bits, uint32_t *denormal)
{
    Fp_Value value = Fp_FormatUnpack(&Fp_FormatBinary80, bits);

    if (value.fv_class == FP_CLASS_SUBNORMAL) {
        *denormal = CPU_X86_64_FSW_DE;
    }
    return value;
}

// Return an x87 operation's NaN: its larger NaN quieted, else the indefinite.
Fp_Value Cpu_x86_64_X87Quiet(const Fp_Value *a, const Fp_Value *b)
{
    Fp_Value nan = Fp_FormatMake(FP_SIGN_NEGATIVE, Fp_WideMake(0, 0), 0);

    if (a->fv_class == FP_CLASS_UNSUPPORTED || b->fv_class == FP_CLASS_UNSUPPORTED) {
        nan.fv_class = FP_CLASS_NAN;
    } else if (a->fv_class == FP_CLASS_NAN && b->fv_class == FP_CLASS_NAN) {
        bool a_quiet = ! Fp_FormatIsSignalling(a);
        bool b_quiet = ! Fp_FormatIsSignalling(b);
        int32_t order = a_quiet != b_quiet ? (a_quiet ? 1 : -1) : Fp_WideCmp(a->fv_mant, b->fv_mant);

        nan = order >= 0 ? *a : *b;
        if (order == 0 && a->fv_sign != b->fv_sign) {
            nan.fv_sign = FP_SIGN_POSITIVE;
        }
    } else if (a->fv_class == FP_CLASS_NAN) {
        nan = *a;
    } else if (b->fv_class == FP_CLASS_NAN) {
        nan = *b;
    }
    nan.fv_class = FP_CLASS_NAN;
    nan.fv_mant.fw_hi |= FP_WORD_TOP;
    return nan;
}

// Return the bits of the x87's floating-point indefinite.
Fp_Wide Cpu_x86_64_X87Indefinite(void)
{
    return Fp_WideMake(CPU_X86_64_X87_INDEFINITE_HI, CPU_X86_64_X87_INDEFINITE_LO);
}

// Settle an x87 operation's flags and C1; return whether it writes its result.
bool Cpu_x86_64_X87Settle(Cpu_x86_64_State *cpu, const Fp_Context *ctx, uint32_t denormal, Cpu_x86_64_X87Dest dest)
{
    uint32_t masks = cpu->cs_fcw & CPU_X86_64_X87_EXCEPTIONS;
    uint32_t flags = ctx->fc_flags;
    uint32_t pre = denormal;
    uint32_t post = flags & FP_FLAG_INEXACT ? CPU_X86_64_FSW_PE : 0;
    uint32_t c1 = flags & FP_FLAG_ROUNDED_UP ? CPU_X86_64_FSW_C1 : 0;

    if (flags & FP_FLAG_INVALID) {
        pre = CPU_X86_64_FSW_IE;
    } else if (flags & FP_FLAG_DIVIDE) {
        pre = CPU_X86_64_FSW_ZE;
    }
    if (pre & ~masks) {
        Cpu_x86_64_X87Status(cpu, pre);
        return false;
    }
    if (flags & FP_FLAG_OVERFLOW) {
        post |= CPU_X86_64_FSW_OE;
    } else if ((flags & FP_FLAG_UNDERFLOW) || ((flags & FP_FLAG_TINY) && ! (masks & CPU_X86_64_FSW_UE))) {
        post |= CPU_X86_64_FSW_UE;
    }
    if (dest == CPU_X86_64_X87_DEST_MEM && (post & (CPU_X86_64_FSW_OE | CPU_X86_64_FSW_UE) & ~masks)) {
        Cpu_x86_64_X87Status(cpu, post & (CPU_X86_64_FSW_OE | CPU_X86_64_FSW_UE));
        return false;
    }
    Cpu_x86_64_X87Status(cpu, pre | post | c1);
    return true;
}

// Compute an x87 arithmetic form's %st(i) op %st, rounded to a format.
Fp_Value Cpu_x86_64_X87Compute(int32_t form, const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx)
{
    switch (form) {
        case ENC_X86_64_X87_FADDP: {
            return Fp_ArithAdd(a, b, fmt, ctx);
        } break;
        case ENC_X86_64_X87_FMULP: {
            return Fp_ArithMul(a, b, fmt, ctx);
        } break;
        case ENC_X86_64_X87_FSUBRP: {
            return Fp_ArithSub(a, b, fmt, ctx);
        } break;
        default: {
            return Fp_ArithDiv(a, b, fmt, ctx);
        }
    }
}

// Run an arithmetic form on %st(i) and %st into %st(i), then pop the stack.
void Cpu_x86_64_X87Arith(Cpu_x86_64_State *cpu, int32_t i, int32_t form)
{
    uint32_t masks = cpu->cs_fcw & CPU_X86_64_X87_EXCEPTIONS;
    Fp_Format fmt = Cpu_x86_64_X87Precision(cpu);
    Fp_Context ctx = Cpu_x86_64_X87Context(cpu);
    uint32_t denormal = 0;

    if (! Cpu_x86_64_IsStFull(cpu, 0) || ! Cpu_x86_64_IsStFull(cpu, i)) {
        if (Cpu_x86_64_X87Underflow(cpu)) {
            Cpu_x86_64_StWrite(cpu, i, Cpu_x86_64_X87Indefinite());
            Cpu_x86_64_StPop(cpu);
        }
        return;
    }

    Fp_Value a = Cpu_x86_64_X87Unpack(*Cpu_x86_64_St(cpu, i), &denormal);
    Fp_Value b = Cpu_x86_64_X87Unpack(*Cpu_x86_64_St(cpu, 0), &denormal);
    if (Fp_FormatIsNan(&a) || Fp_FormatIsNan(&b)) {
        denormal = 0;
    }
    Fp_Value result = Cpu_x86_64_X87Compute(form, &a, &b, &fmt, &ctx);
    bool over = (ctx.fc_flags & FP_FLAG_OVERFLOW) && ! (masks & CPU_X86_64_FSW_OE);
    bool under = (ctx.fc_flags & FP_FLAG_TINY) && ! (masks & CPU_X86_64_FSW_UE);

    if (result.fv_class == FP_CLASS_NAN) {
        result = Cpu_x86_64_X87Quiet(&a, &b);
    } else if (over || under) {
        Fp_Context scaled = Cpu_x86_64_X87Context(cpu);
        fmt.ff_min -= CPU_X86_64_X87_SCALE;
        fmt.ff_max += CPU_X86_64_X87_SCALE;
        result = Cpu_x86_64_X87Compute(form, &a, &b, &fmt, &scaled);
        result.fv_exp += over ? -CPU_X86_64_X87_SCALE : CPU_X86_64_X87_SCALE;
        ctx.fc_flags = (ctx.fc_flags & (FP_FLAG_OVERFLOW | FP_FLAG_TINY)) | scaled.fc_flags;
    }
    if (Cpu_x86_64_X87Settle(cpu, &ctx, denormal, CPU_X86_64_X87_DEST_REG)) {
        Cpu_x86_64_StWrite(cpu, i, Fp_FormatPack(&Fp_FormatBinary80, &result));
        Cpu_x86_64_StPop(cpu);
    }
}

// Compare %st with %st(i) into ZF, PF and CF, then pop unless IE or DE traps.
void Cpu_x86_64_X87Compare(Cpu_x86_64_State *cpu, int32_t i)
{
    Fp_Context ctx = Cpu_x86_64_X87Context(cpu);
    uint32_t denormal = 0;

    if (! Cpu_x86_64_IsStFull(cpu, 0) || ! Cpu_x86_64_IsStFull(cpu, i)) {
        Cpu_x86_64_FlagsOrder(cpu, FP_ORDER_UNORDERED);
        if (Cpu_x86_64_X87Underflow(cpu)) {
            Cpu_x86_64_StPop(cpu);
        }
        return;
    }

    Fp_Value a = Cpu_x86_64_X87Unpack(*Cpu_x86_64_St(cpu, 0), &denormal);
    Fp_Value b = Cpu_x86_64_X87Unpack(*Cpu_x86_64_St(cpu, i), &denormal);
    if (Fp_FormatIsNan(&a) || Fp_FormatIsNan(&b)) {
        denormal = 0;
    }
    Cpu_x86_64_FlagsOrder(cpu, Fp_ArithCmp(&a, &b, FP_COMPARE_QUIET, &ctx));
    if (Cpu_x86_64_X87Settle(cpu, &ctx, denormal, CPU_X86_64_X87_DEST_REG)) {
        Cpu_x86_64_StPop(cpu);
    }
}

// Push a binary32 or binary64 value; DE, even unmasked, still loads it.
void Cpu_x86_64_X87Load(Cpu_x86_64_State *cpu, const Fp_Format *fmt, uint64_t bits)
{
    Fp_Value value = Fp_FormatUnpack(fmt, Fp_WideMake(0, bits));
    uint32_t flags = value.fv_class == FP_CLASS_SUBNORMAL ? CPU_X86_64_FSW_DE : 0;

    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    if (Cpu_x86_64_IsStFull(cpu, -1)) {
        Cpu_x86_64_X87Overflow(cpu);
        return;
    }
    if (Fp_FormatIsSignalling(&value)) {
        flags = CPU_X86_64_FSW_IE;
        if (! (cpu->cs_fcw & CPU_X86_64_FSW_IE)) {
            Cpu_x86_64_X87Status(cpu, flags);
            return;
        }
    }
    if (value.fv_class == FP_CLASS_NAN) {
        value = Cpu_x86_64_X87Quiet(&value, &value);
    }
    Cpu_x86_64_X87Status(cpu, flags);
    Cpu_x86_64_StPush(cpu, Fp_FormatPack(&Fp_FormatBinary80, &value));
}

// Push a signed 64-bit integer, which the extended format holds exactly.
void Cpu_x86_64_X87LoadInteger(Cpu_x86_64_State *cpu, uint64_t bits)
{
    Fp_Context ctx = Cpu_x86_64_X87Context(cpu);

    if (cpu->cs_trap != CPU_X86_64_TRAP_NONE) {
        return;
    }
    if (Cpu_x86_64_IsStFull(cpu, -1)) {
        Cpu_x86_64_X87Overflow(cpu);
        return;
    }

    Fp_Value value = Fp_ToValue((int64_t) bits, &Fp_FormatBinary80, &ctx);
    Cpu_x86_64_X87Status(cpu, 0);
    Cpu_x86_64_StPush(cpu, Fp_FormatPack(&Fp_FormatBinary80, &value));
}

// Push the extended value at addr as its bits are, whatever they encode.
void Cpu_x86_64_X87LoadExtended(Cpu_x86_64_State *cpu, uint64_t addr)
{
    if (! Cpu_x86_64_ReadAt(cpu, addr, CPU_X86_64_X87_REG_SIZE)) {
        return;
    }
    if (Cpu_x86_64_IsStFull(cpu, -1)) {
        Cpu_x86_64_X87Overflow(cpu);
        return;
    }

    uint64_t lo = Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_64);
    uint64_t hi = Cpu_x86_64_ReadMem(cpu, addr + CPU_X86_64_X87_TOP_OFF, CPU_X86_64_WIDTH_16);
    Cpu_x86_64_X87Status(cpu, 0);
    Cpu_x86_64_StPush(cpu, Fp_WideMake(hi, lo));
}

// Store %st as binary32 or binary64 at addr in RC's mode, then pop.
void Cpu_x86_64_X87Store(Cpu_x86_64_State *cpu, uint64_t addr, const Fp_Format *fmt)
{
    Cpu_x86_64_OperandWidth width = (Cpu_x86_64_OperandWidth) fmt->ff_bits;
    Fp_Context ctx = Cpu_x86_64_X87Context(cpu);
    Fp_Value zero = Fp_FormatMake(FP_SIGN_POSITIVE, Fp_WideMake(0, 0), 0);
    Fp_Value result = Cpu_x86_64_X87Quiet(&zero, &zero);

    if (! Cpu_x86_64_WriteAt(cpu, addr, (size_t) width / CPU_X86_64_BITS_PER_BYTE)) {
        return;
    }
    if (! Cpu_x86_64_IsStFull(cpu, 0)) {
        if (Cpu_x86_64_X87Underflow(cpu)) {
            Cpu_x86_64_WriteMem(cpu, addr, Fp_FormatPack(fmt, &result).fw_lo, width);
            Cpu_x86_64_StPop(cpu);
        }
        return;
    }

    Fp_Value value = Fp_FormatUnpack(&Fp_FormatBinary80, *Cpu_x86_64_St(cpu, 0));
    result = Fp_RoundValue(&value, fmt, &ctx);
    if (Fp_FormatIsNan(&value)) {
        result = Cpu_x86_64_X87Quiet(&value, &value);
    }
    if (Cpu_x86_64_X87Settle(cpu, &ctx, 0, CPU_X86_64_X87_DEST_MEM)) {
        Cpu_x86_64_WriteMem(cpu, addr, Fp_FormatPack(fmt, &result).fw_lo, width);
        Cpu_x86_64_StPop(cpu);
    }
}

// Store %st at addr truncated to a signed 64-bit integer, then pop.
void Cpu_x86_64_X87StoreInteger(Cpu_x86_64_State *cpu, uint64_t addr)
{
    Fp_Context ctx = Cpu_x86_64_X87Context(cpu);
    uint64_t integer = CPU_X86_64_INDEFINITE_64;

    if (! Cpu_x86_64_WriteAt(cpu, addr, sizeof(uint64_t))) {
        return;
    }
    if (! Cpu_x86_64_IsStFull(cpu, 0)) {
        if (Cpu_x86_64_X87Underflow(cpu)) {
            Cpu_x86_64_WriteMem(cpu, addr, integer, CPU_X86_64_WIDTH_64);
            Cpu_x86_64_StPop(cpu);
        }
        return;
    }

    Fp_Value value = Fp_FormatUnpack(&Fp_FormatBinary80, *Cpu_x86_64_St(cpu, 0));
    ctx.fc_mode = FP_ROUND_ZERO;
    integer = Fp_ToInteger(&value, CPU_X86_64_WIDTH_64, &ctx);
    if (ctx.fc_flags & FP_FLAG_INVALID) {
        integer = CPU_X86_64_INDEFINITE_64;
    }
    if (Cpu_x86_64_X87Settle(cpu, &ctx, 0, CPU_X86_64_X87_DEST_MEM)) {
        Cpu_x86_64_WriteMem(cpu, addr, integer, CPU_X86_64_WIDTH_64);
        Cpu_x86_64_StPop(cpu);
    }
}

// Store %st's bits at addr as they are, then pop.
void Cpu_x86_64_X87StoreExtended(Cpu_x86_64_State *cpu, uint64_t addr)
{
    Fp_Wide bits = Cpu_x86_64_X87Indefinite();

    if (! Cpu_x86_64_WriteAt(cpu, addr, CPU_X86_64_X87_REG_SIZE)) {
        return;
    }
    if (! Cpu_x86_64_IsStFull(cpu, 0)) {
        if (! Cpu_x86_64_X87Underflow(cpu)) {
            return;
        }
    } else {
        bits = *Cpu_x86_64_St(cpu, 0);
        Cpu_x86_64_X87Status(cpu, 0);
    }
    Cpu_x86_64_WriteMem(cpu, addr, bits.fw_lo, CPU_X86_64_WIDTH_64);
    Cpu_x86_64_WriteMem(cpu, addr + CPU_X86_64_X87_TOP_OFF, bits.fw_hi, CPU_X86_64_WIDTH_16);
    Cpu_x86_64_StPop(cpu);
}

// Copy %st into %st(i), then pop.
void Cpu_x86_64_X87Copy(Cpu_x86_64_State *cpu, int32_t i)
{
    Fp_Wide bits = Cpu_x86_64_X87Indefinite();

    if (! Cpu_x86_64_IsStFull(cpu, 0)) {
        if (! Cpu_x86_64_X87Underflow(cpu)) {
            return;
        }
    } else {
        bits = *Cpu_x86_64_St(cpu, 0);
        Cpu_x86_64_X87Status(cpu, 0);
    }
    Cpu_x86_64_StWrite(cpu, i, bits);
    Cpu_x86_64_StPop(cpu);
}

// Flip %st's sign bit, whatever it encodes.
void Cpu_x86_64_X87Negate(Cpu_x86_64_State *cpu)
{
    if (! Cpu_x86_64_IsStFull(cpu, 0)) {
        if (Cpu_x86_64_X87Underflow(cpu)) {
            Cpu_x86_64_StWrite(cpu, 0, Cpu_x86_64_X87Indefinite());
        }
        return;
    }
    Cpu_x86_64_St(cpu, 0)->fw_hi ^= CPU_X86_64_X87_SIGN_BIT;
    Cpu_x86_64_X87Status(cpu, 0);
}

// Raise #MF for an unmasked x87 exception pending; return whether none is.
bool Cpu_x86_64_X87Wait(Cpu_x86_64_State *cpu, uint64_t rip)
{
    if (! (cpu->cs_fsw & ~cpu->cs_fcw & CPU_X86_64_X87_EXCEPTIONS)) {
        return true;
    }
    Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_MF, ERR_CPU_X87_FLOAT_UNMASKED, (Cpu_x86_64_TypeULLong) Cpu_x86_64_ReadFsw(cpu), (Cpu_x86_64_TypeULLong) rip);
    return false;
}

// Set the x87 control word, its reserved bits read as they always do.
void Cpu_x86_64_WriteFcw(Cpu_x86_64_State *cpu, uint16_t fcw)
{
    cpu->cs_fcw = (fcw & CPU_X86_64_FCW_WRITABLE) | CPU_X86_64_FCW_FIXED;
}

// Return the x87 status word, its TOP, ES and B filled in.
uint16_t Cpu_x86_64_ReadFsw(const Cpu_x86_64_State *cpu)
{
    uint16_t pending = cpu->cs_fsw & ~cpu->cs_fcw & CPU_X86_64_X87_EXCEPTIONS;
    uint16_t summary = pending ? CPU_X86_64_FSW_ES | CPU_X86_64_FSW_B : 0;

    return (uint16_t) (cpu->cs_fsw | summary | cpu->cs_top << CPU_X86_64_FSW_TOP_SHIFT);
}

// Set the x87 status word, its TOP moving the stack, its ES and B ignored.
void Cpu_x86_64_WriteFsw(Cpu_x86_64_State *cpu, uint16_t fsw)
{
    cpu->cs_top = (fsw & CPU_X86_64_FSW_TOP_MASK) >> CPU_X86_64_FSW_TOP_SHIFT;
    cpu->cs_fsw = fsw & (uint16_t) ~(CPU_X86_64_FSW_TOP_MASK | CPU_X86_64_FSW_ES | CPU_X86_64_FSW_B);
}

// Classify a value held in an x87 register, as its tag says it.
Cpu_x86_64_Tag Cpu_x86_64_ClassifyTag(Fp_Wide bits)
{
    uint64_t exp = bits.fw_hi & CPU_X86_64_X87_EXP_MASK;

    if (exp == 0 && bits.fw_lo == 0) {
        return CPU_X86_64_TAG_ZERO;
    }
    if (exp == 0 || exp == CPU_X86_64_X87_EXP_MASK || ! (bits.fw_lo & FP_WORD_TOP)) {
        return CPU_X86_64_TAG_SPECIAL;
    }
    return CPU_X86_64_TAG_VALID;
}

// Return the x87 tag word, each full register's tag found from its value.
uint16_t Cpu_x86_64_ReadFtw(const Cpu_x86_64_State *cpu)
{
    uint16_t ftw = 0;

    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        Cpu_x86_64_Tag tag = (cpu->cs_full >> i & 1) ? Cpu_x86_64_ClassifyTag(cpu->cs_st[i]) : CPU_X86_64_TAG_EMPTY;
        ftw |= (uint16_t) (tag << (i * CPU_X86_64_TAG_BITS));
    }
    return ftw;
}

// Set which x87 registers are full from a tag word.
void Cpu_x86_64_WriteFtw(Cpu_x86_64_State *cpu, uint16_t ftw)
{
    cpu->cs_full = 0;
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        if ((ftw >> (i * CPU_X86_64_TAG_BITS) & CPU_X86_64_TAG_MASK) != CPU_X86_64_TAG_EMPTY) {
            cpu->cs_full |= (uint8_t) (1u << i);
        }
    }
}

// Store the x87 environment at addr, then mask every x87 exception.
void Cpu_x86_64_StoreEnv(Cpu_x86_64_State *cpu, uint64_t addr)
{
    if (! Cpu_x86_64_WriteAt(cpu, addr, CPU_X86_64_ENV_SIZE)) {
        return;
    }
    for (uint64_t off = 0; off < CPU_X86_64_ENV_SIZE; off += CPU_X86_64_ENV_SLOT) {
        Cpu_x86_64_WriteMem(cpu, addr + off, 0, CPU_X86_64_WIDTH_32);
    }
    Cpu_x86_64_WriteMem(cpu, addr + CPU_X86_64_ENV_FCW_OFF, cpu->cs_fcw, CPU_X86_64_WIDTH_16);
    Cpu_x86_64_WriteMem(cpu, addr + CPU_X86_64_ENV_FSW_OFF, Cpu_x86_64_ReadFsw(cpu), CPU_X86_64_WIDTH_16);
    Cpu_x86_64_WriteMem(cpu, addr + CPU_X86_64_ENV_FTW_OFF, Cpu_x86_64_ReadFtw(cpu), CPU_X86_64_WIDTH_16);
    cpu->cs_fcw |= CPU_X86_64_X87_EXCEPTIONS;
}

// Load the x87 environment from addr.
void Cpu_x86_64_LoadEnv(Cpu_x86_64_State *cpu, uint64_t addr)
{
    if (! Cpu_x86_64_ReadAt(cpu, addr, CPU_X86_64_ENV_SIZE)) {
        return;
    }
    Cpu_x86_64_WriteFcw(cpu, (uint16_t) Cpu_x86_64_ReadMem(cpu, addr + CPU_X86_64_ENV_FCW_OFF, CPU_X86_64_WIDTH_16));
    Cpu_x86_64_WriteFsw(cpu, (uint16_t) Cpu_x86_64_ReadMem(cpu, addr + CPU_X86_64_ENV_FSW_OFF, CPU_X86_64_WIDTH_16));
    Cpu_x86_64_WriteFtw(cpu, (uint16_t) Cpu_x86_64_ReadMem(cpu, addr + CPU_X86_64_ENV_FTW_OFF, CPU_X86_64_WIDTH_16));
}

// Execute an x87 load or store of memory.
void Cpu_x86_64_StepX87Mem(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    uint64_t addr = Cpu_x86_64_RmAddr(cpu, insn, next);
    int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;

    switch (insn->ci_op << ENC_X86_64_REG_SHIFT | digit) {
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M32: {
            Cpu_x86_64_X87Load(cpu, &Fp_FormatBinary32, Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_32));
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M32: {
            Cpu_x86_64_X87Store(cpu, addr, &Fp_FormatBinary32);
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M64: {
            Cpu_x86_64_X87Load(cpu, &Fp_FormatBinary64, Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_64));
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M64: {
            Cpu_x86_64_X87Store(cpu, addr, &Fp_FormatBinary64);
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FISTTP_M64: {
            Cpu_x86_64_X87StoreInteger(cpu, addr);
        } break;
        case ENC_X86_64_OPCODE_X87_DF << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FILD_M64: {
            Cpu_x86_64_X87LoadInteger(cpu, Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_64));
        } break;
        case ENC_X86_64_OPCODE_X87_DB << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLD_M80: {
            Cpu_x86_64_X87LoadExtended(cpu, addr);
        } break;
        case ENC_X86_64_OPCODE_X87_DB << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FSTP_M80: {
            Cpu_x86_64_X87StoreExtended(cpu, addr);
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLDCW: {
            uint64_t fcw = Cpu_x86_64_ReadMem(cpu, addr, CPU_X86_64_WIDTH_16);
            if (cpu->cs_trap == CPU_X86_64_TRAP_NONE) {
                Cpu_x86_64_WriteFcw(cpu, (uint16_t) fcw);
            }
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FNSTCW: {
            Cpu_x86_64_WriteMem(cpu, addr, cpu->cs_fcw, CPU_X86_64_WIDTH_16);
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FLDENV: {
            Cpu_x86_64_LoadEnv(cpu, addr);
        } break;
        case ENC_X86_64_OPCODE_X87_D9 << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FNSTENV: {
            Cpu_x86_64_StoreEnv(cpu, addr);
        } break;
        case ENC_X86_64_OPCODE_X87_DD << ENC_X86_64_REG_SHIFT | ENC_X86_64_X87_FNSTSW_M16: {
            Cpu_x86_64_WriteMem(cpu, addr, Cpu_x86_64_ReadFsw(cpu), CPU_X86_64_WIDTH_16);
        } break;
        default: {
            Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_MEM_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        }
    }
}

// Execute an x87 operation, a waiting one after any exception pending.
void Cpu_x86_64_StepX87(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip)
{
    if (! Cpu_x86_64_IsX87NoWait(insn) && ! Cpu_x86_64_X87Wait(cpu, rip)) {
        return;
    }
    if (insn->ci_rmkind != CPU_X86_64_RM_REG) {
        Cpu_x86_64_StepX87Mem(cpu, insn, next, rip);
        return;
    }

    int32_t i = insn->ci_rm & ENC_X86_64_REG_MASK;
    int32_t form = CPU_X86_64_X87_REG_FORM | (insn->ci_reg & ENC_X86_64_REG_MASK) << ENC_X86_64_REG_SHIFT;

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_X87_D9: {
            if (form != ENC_X86_64_X87_FCHS || i != 0) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_D9_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            Cpu_x86_64_X87Negate(cpu);
        } break;
        case ENC_X86_64_OPCODE_X87_DB: {
            if (form + i != ENC_X86_64_X87_FNCLEX) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DB_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            cpu->cs_fsw &= (uint16_t) ~(CPU_X86_64_X87_EXCEPTIONS | CPU_X86_64_FSW_SF);
        } break;
        case ENC_X86_64_OPCODE_X87_DE: {
            if (form != ENC_X86_64_X87_FADDP && form != ENC_X86_64_X87_FMULP
                && form != ENC_X86_64_X87_FSUBRP && form != ENC_X86_64_X87_FDIVRP) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DE_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            Cpu_x86_64_X87Arith(cpu, i, form);
        } break;
        case ENC_X86_64_OPCODE_X87_DF: {
            if (form + i == ENC_X86_64_X87_FNSTSW_AX) {
                Cpu_x86_64_WriteReg(cpu, CPU_X86_64_REG_RAX, Cpu_x86_64_ReadFsw(cpu), CPU_X86_64_WIDTH_16);
                return;
            }
            if (form != ENC_X86_64_X87_FUCOMIP) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DF_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            Cpu_x86_64_X87Compare(cpu, i);
        } break;
        case ENC_X86_64_OPCODE_X87_DD: {
            if (form != ENC_X86_64_X87_FSTP) {
                Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_DD_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
                return;
            }
            Cpu_x86_64_X87Copy(cpu, i);
        } break;
        default: {
            Cpu_x86_64_Fault(cpu, CPU_X86_64_VECTOR_UD, ERR_CPU_X87_NOT_IMPLEMENTED, (Cpu_x86_64_TypeULLong) rip);
        }
    }
}

// Divide hi:lo by d at width, hi below d so the quotient fits, bit by bit.
uint64_t Cpu_x86_64_DivideLong(uint64_t hi, uint64_t lo, uint64_t d, Cpu_x86_64_OperandWidth width, uint64_t *rem)
{
    uint64_t quot = 0;
    uint64_t part = hi;
    uint64_t sign = (uint64_t) 1 << (width - 1);
    uint64_t mask = sign | (sign - 1);

    for (uint64_t bit = sign; bit != 0; bit >>= 1) {
        bool carry = (part & sign) != 0;
        part = ((part << 1) | ((lo & bit) != 0)) & mask;
        if (carry || part >= d) {
            part = (part - d) & mask;
            quot |= bit;
        }
    }
    *rem = part;
    return quot;
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
    bool num_neg = is_signed && (hi & sign);
    bool d_neg = is_signed && (d & sign);
    uint64_t hi_mag = num_neg ? (~hi + (lo == 0)) & mask : hi;
    uint64_t lo_mag = num_neg ? -lo & mask : lo;
    uint64_t d_mag = d_neg ? -d & mask : d;
    uint64_t r = 0;
    uint64_t q = hi_mag < d_mag ? Cpu_x86_64_DivideLong(hi_mag, lo_mag, d_mag, width, &r) : 0;

    // Phase: fault on a quotient the width cannot hold
    bool q_neg = num_neg != d_neg;
    uint64_t limit = ! is_signed ? mask : (q_neg ? sign : sign - 1);
    if (hi_mag >= d_mag || q > limit) {
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
        Cpu_x86_64_PageFault(cpu, rip, CPU_X86_64_PF_USER | CPU_X86_64_PF_FETCH);
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
            case ENC_X86_64_OPCODE2_GRP15: {
                Cpu_x86_64_StepGrp15(cpu, &insn, next, rip);
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
        case ENC_X86_64_OPCODE_FWAIT: {
            Cpu_x86_64_X87Wait(cpu, rip);
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
        case ENC_X86_64_OPCODE2_SETNP:
        case ENC_X86_64_OPCODE2_GRP15: {
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

    if (op == ENC_X86_64_OPCODE_RET || op == ENC_X86_64_OPCODE_CQO || op == ENC_X86_64_OPCODE_FWAIT) {
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
            case ENC_X86_64_OPCODE2_GRP15: {
                int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;
                return digit == ENC_X86_64_GRP_LDMXCSR ? "ldmxcsr" : digit == ENC_X86_64_GRP_STMXCSR ? "stmxcsr" : "(bad)";
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
        case ENC_X86_64_OPCODE_FWAIT: {
            return "fwait";
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
    int32_t form = CPU_X86_64_X87_REG_FORM | digit << ENC_X86_64_REG_SHIFT;
    int32_t i = insn->ci_rm & ENC_X86_64_REG_MASK;
    bool mem = insn->ci_rmkind != CPU_X86_64_RM_REG;

    switch (insn->ci_op) {
        case ENC_X86_64_OPCODE_X87_D9: {
            if (! mem) {
                return form == ENC_X86_64_X87_FCHS ? "fchs" : "(bad)";
            }
            switch (digit) {
                case ENC_X86_64_X87_FLD_M32: {
                    return "flds";
                } break;
                case ENC_X86_64_X87_FSTP_M32: {
                    return "fstps";
                } break;
                case ENC_X86_64_X87_FLDENV: {
                    return "fldenv";
                } break;
                case ENC_X86_64_X87_FLDCW: {
                    return "fldcw";
                } break;
                case ENC_X86_64_X87_FNSTENV: {
                    return "fnstenv";
                } break;
                case ENC_X86_64_X87_FNSTCW: {
                    return "fnstcw";
                } break;
                default: {
                    return "(bad)";
                }
            }
        } break;
        case ENC_X86_64_OPCODE_X87_DB: {
            if (! mem) {
                return form + i == ENC_X86_64_X87_FNCLEX ? "fnclex" : "(bad)";
            }
            return digit == ENC_X86_64_X87_FLD_M80 ? "fldt" : digit == ENC_X86_64_X87_FSTP_M80 ? "fstpt" : "(bad)";
        } break;
        case ENC_X86_64_OPCODE_X87_DD: {
            if (! mem) {
                return "fstp";
            }
            return digit == ENC_X86_64_X87_FLD_M64 ? "fldl" : digit == ENC_X86_64_X87_FSTP_M64 ? "fstpl"
                 : digit == ENC_X86_64_X87_FISTTP_M64 ? "fisttpq" : digit == ENC_X86_64_X87_FNSTSW_M16 ? "fnstsw" : "(bad)";
        } break;
        case ENC_X86_64_OPCODE_X87_DE: {
            switch (form) {
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
            if (! mem) {
                return form + i == ENC_X86_64_X87_FNSTSW_AX ? "fnstsw" : "fucomip";
            }
            return "fildq";
        } break;
        default: {
            return "(bad)";
        }
    }
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

// Return whether a one-byte opcode is an x87 escape the interpreter answers.
bool Cpu_x86_64_IsX87(int32_t op)
{
    return op == ENC_X86_64_OPCODE_X87_D9 || op == ENC_X86_64_OPCODE_X87_DB || op == ENC_X86_64_OPCODE_X87_DD
        || op == ENC_X86_64_OPCODE_X87_DE || op == ENC_X86_64_OPCODE_X87_DF;
}

// Return whether an x87 instruction runs past an exception pending, no-wait.
bool Cpu_x86_64_IsX87NoWait(const Cpu_x86_64_Insn *insn)
{
    int32_t digit = insn->ci_reg & ENC_X86_64_REG_MASK;
    int32_t form = CPU_X86_64_X87_REG_FORM | digit << ENC_X86_64_REG_SHIFT | (insn->ci_rm & ENC_X86_64_REG_MASK);

    if (insn->ci_rmkind != CPU_X86_64_RM_REG) {
        return (insn->ci_op == ENC_X86_64_OPCODE_X87_D9 && (digit == ENC_X86_64_X87_FNSTCW || digit == ENC_X86_64_X87_FNSTENV))
               || (insn->ci_op == ENC_X86_64_OPCODE_X87_DD && digit == ENC_X86_64_X87_FNSTSW_M16);
    }
    return (insn->ci_op == ENC_X86_64_OPCODE_X87_DB && form == ENC_X86_64_X87_FNCLEX)
           || (insn->ci_op == ENC_X86_64_OPCODE_X87_DF && form == ENC_X86_64_X87_FNSTSW_AX);
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
        bool reg = insn->ci_rmkind == CPU_X86_64_RM_REG;
        int32_t second = CPU_X86_64_X87_REG_FORM | (insn->ci_reg & ENC_X86_64_REG_MASK) << ENC_X86_64_REG_SHIFT | (insn->ci_rm & ENC_X86_64_REG_MASK);
        if (reg && (insn->ci_op == ENC_X86_64_OPCODE_X87_D9 || insn->ci_op == ENC_X86_64_OPCODE_X87_DB)) {
            snprintf(out, n, "%s", name);
        } else if (reg && insn->ci_op == ENC_X86_64_OPCODE_X87_DF && second == ENC_X86_64_X87_FNSTSW_AX) {
            snprintf(out, n, "%s %%ax", name);
        } else if (reg) {
            snprintf(out, n, "%s %%st(%d)", name, insn->ci_rm & ENC_X86_64_REG_MASK);
        } else {
            Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_64, next, rm, sizeof(rm));
            snprintf(out, n, "%s %s", name, rm);
        }
        return;
    }
    if (insn->ci_op == ENC_X86_64_OPCODE_ESCAPE && insn->ci_op2 == ENC_X86_64_OPCODE2_GRP15) {
        Cpu_x86_64_FormatRm(insn, CPU_X86_64_WIDTH_32, next, rm, sizeof(rm));
        snprintf(out, n, "%s %s", name, rm);
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
