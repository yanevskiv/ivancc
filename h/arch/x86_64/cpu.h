/*
 * C header file for the x86-64 CPU.
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

#ifndef CPU_X86_64_H
#define CPU_X86_64_H

// Standard headers.
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/fp.h"
#include "arch/x86_64/enc.h"

// Bits in a byte, for turning an operand width into a count of bytes.
#define CPU_X86_64_BITS_PER_BYTE 8

// The value a result of each width is truncated to.
#define CPU_X86_64_MASK_8  0xFF
#define CPU_X86_64_MASK_16 0xFFFF
#define CPU_X86_64_MASK_32 0xFFFFFFFF
#define CPU_X86_64_MASK_64 (~(uint64_t) 0)

// A shift count wraps at the operand's width, as the hardware masks it.
#define CPU_X86_64_SHIFT_MASK_32 31
#define CPU_X86_64_SHIFT_MASK_64 63

// Bytes an immediate operand occupies.
#define CPU_X86_64_IMM8  1
#define CPU_X86_64_IMM32 4
#define CPU_X86_64_IMM64 8

// The mask that indexes the register file.
#define CPU_X86_64_REG_INDEX_MASK 15

// The nibble marking a REX prefix, and the bit extending a register.
#define CPU_X86_64_REX_PREFIX_MASK 0xF0
#define CPU_X86_64_REG_HIGH_BIT    8

// The bits of an opcode outside its baked-in register.
#define CPU_X86_64_OPCODE_REG_MASK 0xF8

// Bytes push, pop, call and ret move %rsp by.
#define CPU_X86_64_STACK_SLOT 8

// ci_op2 when an instruction has no second opcode byte.
#define CPU_X86_64_NO_OPCODE2 (-1)

// SSE registers, and the 64-bit lanes each holds.
#define CPU_X86_64_XMM_COUNT 16
#define CPU_X86_64_XMM_LANES 2

// x87 stack registers, and the mask that wraps an index into them.
#define CPU_X86_64_ST_COUNT 8
#define CPU_X86_64_ST_MASK  7

// The ModRM bits an x87 register form carries around its digit and register.
#define CPU_X86_64_X87_REG_FORM 0xC0

// The MXCSR a reset leaves: every exception masked, rounding to nearest.
// [Amd64-Vol1:4.2.2]: MXCSR's fields and the values a reset gives them
#define CPU_X86_64_MXCSR_INIT 0x1F80

// The MXCSR bits the CPU keeps, so the ones LDMXCSR may set.
// [Amd64-Vol4:LDMXCSR]: a nonzero reserved bit raises #GP
#define CPU_X86_64_MXCSR_MASK 0xFFFF

// The x87 control word FNINIT leaves: every exception masked, rounding to
// nearest, at double-extended precision.
// [Amd64-Vol1:6.2.3]: the x87 control word's fields, and 037Fh
#define CPU_X86_64_FCW_INIT 0x037F

// The x87 control word's bits a load sets, and its reserved bit that reads
// as one, as in 037Fh; the other reserved bits read as zero.
// [Amd64-Vol1:6.2.3]: all bits but the reserved can be read and written
#define CPU_X86_64_FCW_WRITABLE 0x1F3F
#define CPU_X86_64_FCW_FIXED    0x0040

// The exception flags of the x87 status word, and their masks in the
// control word.
// [Amd64-Vol1:6.2.2]: the x87 status word's fields
#define CPU_X86_64_X87_EXCEPTIONS 0x3F

// The flags of the x87 status word FNCLEX clears beside the exceptions: the
// stack fault, the exception summary and busy.
// [Amd64-Vol5:FCLEX]: the flags FNCLEX clears
#define CPU_X86_64_FSW_SF 0x0040
#define CPU_X86_64_FSW_ES 0x0080
#define CPU_X86_64_FSW_B  0x8000

// The place of TOP in the x87 status word.
#define CPU_X86_64_FSW_TOP_SHIFT 11
#define CPU_X86_64_FSW_TOP_MASK  0x3800

// Bits of one register's tag in the x87 tag word.
// [Amd64-Vol1:6.2.4]: the x87 tag word, two bits a physical register
#define CPU_X86_64_TAG_BITS 2
#define CPU_X86_64_TAG_MASK 3

// The layout of the x87 environment FNSTENV and FLDENV move, its pointers
// and opcode stored as zero, since the CPU does not keep them.
// [Amd64-Vol5:FSTENV]:   a 28-byte area at 32- and 64-bit operand sizes
// [Amd64-Vol2:11.4.4.2]: the environment, laid out as the FNSAVE image opens
// [Amd64-Vol2:11.4.4.1]: Figure 11-4, the FNSAVE image in protected mode
#define CPU_X86_64_ENV_FCW_OFF 0
#define CPU_X86_64_ENV_FSW_OFF 4
#define CPU_X86_64_ENV_FTW_OFF 8
#define CPU_X86_64_ENV_SIZE    28

// A dword of the x87 environment, each field in the low half of one.
#define CPU_X86_64_ENV_SLOT 4

// The bits of a #PF's error code: a write, from user mode, and a fetch.
#define CPU_X86_64_PF_WRITE 0x2
#define CPU_X86_64_PF_USER  0x4
#define CPU_X86_64_PF_FETCH 0x10

// The integer a conversion leaves when its value is out of range.
#define CPU_X86_64_INDEFINITE_32 ((uint64_t) 1 << 31)
#define CPU_X86_64_INDEFINITE_64 ((uint64_t) 1 << 63)

// The type a `%llx` conversion takes.
typedef unsigned long long Cpu_x86_64_TypeULLong;

// One SSE register, as its 64-bit lanes.
typedef uint64_t Cpu_x86_64_XmmLanes[CPU_X86_64_XMM_LANES];

// Map an address to its memory and the bytes mapped from it.
typedef uint8_t *(*Cpu_x86_64_MemoryMapper)(void *ctx, uint64_t addr, size_t *avail);

// Read a device in the window.
typedef uint64_t (*Cpu_x86_64_DeviceLoader)(void *ctx, uint64_t addr, size_t size);

// Write a device in the window.
typedef void (*Cpu_x86_64_DeviceStorer)(void *ctx, uint64_t addr, size_t size, uint64_t value);

// Registers, numbered as ModRM and REX number them.
typedef enum Cpu_x86_64_Reg Cpu_x86_64_Reg;
enum Cpu_x86_64_Reg {
    CPU_X86_64_REG_RAX,  // %rax
    CPU_X86_64_REG_RCX,  // %rcx
    CPU_X86_64_REG_RDX,  // %rdx
    CPU_X86_64_REG_RBX,  // %rbx
    CPU_X86_64_REG_RSP,  // %rsp
    CPU_X86_64_REG_RBP,  // %rbp
    CPU_X86_64_REG_RSI,  // %rsi
    CPU_X86_64_REG_RDI,  // %rdi
    CPU_X86_64_REG_R8,   // %r8
    CPU_X86_64_REG_R9,   // %r9
    CPU_X86_64_REG_R10,  // %r10
    CPU_X86_64_REG_R11,  // %r11
    CPU_X86_64_REG_R12,  // %r12
    CPU_X86_64_REG_R13,  // %r13
    CPU_X86_64_REG_R14,  // %r14
    CPU_X86_64_REG_R15,  // %r15
    CPU_X86_64_REG_COUNT // number of registers
};

// Register widths a decoded operand can name.
typedef enum Cpu_x86_64_OperandWidth Cpu_x86_64_OperandWidth;
enum Cpu_x86_64_OperandWidth {
    CPU_X86_64_WIDTH_8  = 8,  // %al
    CPU_X86_64_WIDTH_16 = 16, // %ax
    CPU_X86_64_WIDTH_32 = 32, // %eax
    CPU_X86_64_WIDTH_64 = 64  // %rax
};

// What stopped a step for the CPU's consumer to deal with.
typedef enum Cpu_x86_64_Trap Cpu_x86_64_Trap;
enum Cpu_x86_64_Trap {
    CPU_X86_64_TRAP_NONE,      // the instruction ran
    CPU_X86_64_TRAP_SYSCALL,   // a syscall, with %rip past it
    CPU_X86_64_TRAP_EXCEPTION, // an exception, which cs_vector names
    CPU_X86_64_TRAP_COUNT      // number of traps
};

// Exception vectors a step can raise, numbered as the hardware numbers them.
typedef enum Cpu_x86_64_Vector Cpu_x86_64_Vector;
enum Cpu_x86_64_Vector {
    CPU_X86_64_VECTOR_DE = 0,  // #DE, a divide error
    CPU_X86_64_VECTOR_UD = 6,  // #UD, an opcode the CPU cannot run
    CPU_X86_64_VECTOR_GP = 13, // #GP, an operand the instruction refuses
    CPU_X86_64_VECTOR_PF = 14  // #PF, an address the bus refused
};

// What the x87 tag word says of a physical register, by its contents.
// [Amd64-Vol2:11.5.10.1]: Table 11-8, each tag from the register's contents
typedef enum Cpu_x86_64_Tag Cpu_x86_64_Tag;
enum Cpu_x86_64_Tag {
    CPU_X86_64_TAG_VALID   = 0, // a normal number
    CPU_X86_64_TAG_ZERO    = 1, // a zero
    CPU_X86_64_TAG_SPECIAL = 2, // a NaN, an infinity, a denormal or unsupported
    CPU_X86_64_TAG_EMPTY   = 3  // no value
};

// How an instruction reaches its r/m operand.
typedef enum Cpu_x86_64_RmKind Cpu_x86_64_RmKind;
enum Cpu_x86_64_RmKind {
    CPU_X86_64_RM_NONE, // the instruction has no ModRM byte
    CPU_X86_64_RM_REG,  // %reg
    CPU_X86_64_RM_MEM,  // disp(%base)
    CPU_X86_64_RM_RIP,  // disp(%rip)
    CPU_X86_64_RM_COUNT // number of kinds
};

// One decoded instruction, as the fields an interpreter needs.
typedef struct Cpu_x86_64_Insn Cpu_x86_64_Insn;
struct Cpu_x86_64_Insn {
    size_t            ci_len;      // bytes the instruction occupies
    int32_t           ci_op;       // primary opcode byte
    int32_t           ci_op2;      // byte following 0x0F, or -1
    bool              ci_rexw;     // true when REX.W selects a 64-bit operand
    bool              ci_opsize16; // true when a 0x66 prefix selects a 16-bit operand or an SSE form
    int32_t           ci_rep;      // an 0xF2 or 0xF3 prefix selecting an SSE form, or 0
    Cpu_x86_64_Reg    ci_reg;      // ModRM reg field, extended by REX.R
    Cpu_x86_64_Reg    ci_rm;       // r/m register, or the base register of a memory operand
    Cpu_x86_64_RmKind ci_rmkind;   // what ModRM's r/m names, if any
    int32_t           ci_disp;     // displacement of a MEM or RIP operand
    int64_t           ci_imm;      // immediate or branch displacement, sign-extended
};

// The memory a CPU reaches, supplied by the program that runs it.
typedef struct Cpu_x86_64_Bus Cpu_x86_64_Bus;
struct Cpu_x86_64_Bus {
    void                    *cb_ctx;     // what each function below is handed
    Cpu_x86_64_MemoryMapper  cb_map;     // the memory at an address, or NULL
    uint64_t                 cb_io_base; // the device window's first address
    uint64_t                 cb_io_size; // bytes the device window spans, or 0
    Cpu_x86_64_DeviceLoader  cb_load;    // read a device in the window
    Cpu_x86_64_DeviceStorer  cb_store;   // write a device in the window
};

// A running CPU: the register file, the flags and the bus they reach.
typedef struct Cpu_x86_64_State Cpu_x86_64_State;
struct Cpu_x86_64_State {
    uint64_t              cs_reg[CPU_X86_64_REG_COUNT]; // the general registers
    uint64_t              cs_rip;    // the next instruction's address
    bool                  cs_zf;     // the result was zero
    bool                  cs_sf;     // the result was negative
    bool                  cs_of;     // the result overflowed a signed operand
    bool                  cs_cf;     // the result carried out of an unsigned operand
    bool                  cs_pf;     // the result's low byte had even parity, or a comparison was unordered
    Cpu_x86_64_XmmLanes   cs_xmm[CPU_X86_64_XMM_COUNT]; // the SSE registers
    long double           cs_st[CPU_X86_64_ST_COUNT]; // the x87 registers
    int32_t               cs_top;    // the x87 register %st names
    uint8_t               cs_full;   // the full x87 registers, a bit each
    uint16_t              cs_fcw;    // the x87 control word
    uint16_t              cs_fsw;    // the x87 status word, but TOP, ES and B
    uint32_t              cs_mxcsr;  // the SSE control and status register
    Cpu_x86_64_Trap       cs_trap;   // what stopped the last step, or CPU_X86_64_TRAP_NONE
    Cpu_x86_64_Vector     cs_vector; // the exception a CPU_X86_64_TRAP_EXCEPTION raised
    char                 *cs_fault;  // the diagnostic of the last exception
    uint64_t              cs_cr2;    // the address the last #PF could not reach
    uint64_t              cs_pf_err; // the error code of the last #PF
    const Cpu_x86_64_Bus *cs_bus;    // the memory it reaches
};

// Running
void           Cpu_x86_64_Init(Cpu_x86_64_State *cpu, const Cpu_x86_64_Bus *bus, uint64_t rip, uint64_t rsp);
void           Cpu_x86_64_Fault(Cpu_x86_64_State *cpu, Cpu_x86_64_Vector vector, Err_Code code, ...);
void           Cpu_x86_64_PageFault(Cpu_x86_64_State *cpu, uint64_t addr, uint64_t error);
uint64_t       Cpu_x86_64_ReadReg(const Cpu_x86_64_State *cpu, Cpu_x86_64_Reg reg, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_WriteReg(Cpu_x86_64_State *cpu, Cpu_x86_64_Reg reg, uint64_t value, Cpu_x86_64_OperandWidth width);
bool           Cpu_x86_64_IsDevice(const Cpu_x86_64_State *cpu, uint64_t addr);
const uint8_t *Cpu_x86_64_ReadAt(Cpu_x86_64_State *cpu, uint64_t addr, size_t size);
uint8_t       *Cpu_x86_64_WriteAt(Cpu_x86_64_State *cpu, uint64_t addr, size_t size);
uint64_t       Cpu_x86_64_ReadMem(Cpu_x86_64_State *cpu, uint64_t addr, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_WriteMem(Cpu_x86_64_State *cpu, uint64_t addr, uint64_t value, Cpu_x86_64_OperandWidth width);
uint64_t       Cpu_x86_64_RmAddr(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next);
uint64_t       Cpu_x86_64_ReadRm(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_WriteRm(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t value, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_FlagsSub(Cpu_x86_64_State *cpu, uint64_t a, uint64_t b, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_FlagsAdd(Cpu_x86_64_State *cpu, uint64_t a, uint64_t b, Cpu_x86_64_OperandWidth width);
bool           Cpu_x86_64_Parity(uint64_t res);
void           Cpu_x86_64_FlagsCompare(Cpu_x86_64_State *cpu, long double a, long double b);
uint64_t       Cpu_x86_64_Truncate(long double value, Cpu_x86_64_OperandWidth width);
uint64_t       Cpu_x86_64_ReadSse(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_StepSse(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip);
long double   *Cpu_x86_64_St(Cpu_x86_64_State *cpu, int32_t i);
void           Cpu_x86_64_StPush(Cpu_x86_64_State *cpu, long double value);
long double    Cpu_x86_64_StPop(Cpu_x86_64_State *cpu);
void           Cpu_x86_64_WriteFcw(Cpu_x86_64_State *cpu, uint16_t fcw);
uint16_t       Cpu_x86_64_ReadFsw(const Cpu_x86_64_State *cpu);
void           Cpu_x86_64_WriteFsw(Cpu_x86_64_State *cpu, uint16_t fsw);
Cpu_x86_64_Tag Cpu_x86_64_ClassifyTag(long double value);
uint16_t       Cpu_x86_64_ReadFtw(const Cpu_x86_64_State *cpu);
void           Cpu_x86_64_WriteFtw(Cpu_x86_64_State *cpu, uint16_t ftw);
void           Cpu_x86_64_StoreEnv(Cpu_x86_64_State *cpu, uint64_t addr);
void           Cpu_x86_64_LoadEnv(Cpu_x86_64_State *cpu, uint64_t addr);
void           Cpu_x86_64_StepGrp15(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip);
void           Cpu_x86_64_StepX87Mem(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip);
void           Cpu_x86_64_StepX87(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip);
uint64_t       Cpu_x86_64_DivideLong(uint64_t hi, uint64_t lo, uint64_t d, Cpu_x86_64_OperandWidth width, uint64_t *rem);
void           Cpu_x86_64_Divide(Cpu_x86_64_State *cpu, const Cpu_x86_64_Insn *insn, uint64_t next, uint64_t rip, Cpu_x86_64_OperandWidth width);
void           Cpu_x86_64_Step(Cpu_x86_64_State *cpu);
void           Cpu_x86_64_Free(Cpu_x86_64_State *cpu);

// Decoding
int64_t                 Cpu_x86_64_ReadImm(const uint8_t *p, size_t n);
bool                    Cpu_x86_64_HasModRM(int32_t op);
bool                    Cpu_x86_64_HasModRM2(int32_t op2);
size_t                  Cpu_x86_64_DecodeModRM(const uint8_t *p, size_t avail, uint8_t rex, Cpu_x86_64_Insn *insn);
Cpu_x86_64_OperandWidth Cpu_x86_64_Width(const Cpu_x86_64_Insn *insn);
size_t                  Cpu_x86_64_Decode(const uint8_t *code, size_t avail, Cpu_x86_64_Insn *insn);

// Naming what was decoded
const char *Cpu_x86_64_RegName(Cpu_x86_64_Reg reg, Cpu_x86_64_OperandWidth width);
const char *Cpu_x86_64_Mnemonic(const Cpu_x86_64_Insn *insn);
const char *Cpu_x86_64_SseMnemonic(const Cpu_x86_64_Insn *insn);
const char *Cpu_x86_64_X87Mnemonic(const Cpu_x86_64_Insn *insn);
bool        Cpu_x86_64_IsSse(int32_t op2);
bool        Cpu_x86_64_IsX87(int32_t op);
void        Cpu_x86_64_FormatRm(const Cpu_x86_64_Insn *insn, Cpu_x86_64_OperandWidth width, uint64_t next, char *out, size_t n);
void        Cpu_x86_64_Format(const Cpu_x86_64_Insn *insn, uint64_t rip, char *out, size_t n);

#endif // CPU_X86_64_H
