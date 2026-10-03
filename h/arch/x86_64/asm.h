/*
 * C header file for the x86-64 instruction list.
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

#ifndef ASM_X86_64_H
#define ASM_X86_64_H

// Standard headers.
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/str.h"

// Bits in a byte, for turning a type's size into an operand width.
#define ASM_X86_64_BITS_PER_BYTE 8

// Registers
typedef enum Asm_x86_64_Reg Asm_x86_64_Reg;
enum Asm_x86_64_Reg {
    ASM_X86_64_REG_RAX = 0,
    ASM_X86_64_REG_RCX,
    ASM_X86_64_REG_RDX,
    ASM_X86_64_REG_RBX,
    ASM_X86_64_REG_RSP,
    ASM_X86_64_REG_RBP,
    ASM_X86_64_REG_RSI,
    ASM_X86_64_REG_RDI,
    ASM_X86_64_REG_R8,
    ASM_X86_64_REG_R9,
    ASM_X86_64_REG_R10,
    ASM_X86_64_REG_R11,
    ASM_X86_64_REG_R12,
    ASM_X86_64_REG_R13,
    ASM_X86_64_REG_R14,
    ASM_X86_64_REG_R15,
    ASM_X86_64_REG_COUNT // number of registers
};

// The width in bits of a register operand, and so of the access it makes.
typedef enum Asm_x86_64_Width Asm_x86_64_Width;
enum Asm_x86_64_Width {
    ASM_X86_64_WIDTH_NONE = 0, // the operand is not a register
    ASM_X86_64_WIDTH_8    = 8,
    ASM_X86_64_WIDTH_16   = 16,
    ASM_X86_64_WIDTH_32   = 32,
    ASM_X86_64_WIDTH_64   = 64
};

// SSE registers
typedef enum Asm_x86_64_Xmm Asm_x86_64_Xmm;
enum Asm_x86_64_Xmm {
    ASM_X86_64_XMM0 = 0,
    ASM_X86_64_XMM1,
    ASM_X86_64_XMM2,
    ASM_X86_64_XMM3,
    ASM_X86_64_XMM4,
    ASM_X86_64_XMM5,
    ASM_X86_64_XMM6,
    ASM_X86_64_XMM7,
    ASM_X86_64_XMM8,
    ASM_X86_64_XMM9,
    ASM_X86_64_XMM10,
    ASM_X86_64_XMM11,
    ASM_X86_64_XMM12,
    ASM_X86_64_XMM13,
    ASM_X86_64_XMM14,
    ASM_X86_64_XMM15,
    ASM_X86_64_XMM_COUNT // number of registers
};

// Opcodes
typedef enum Asm_x86_64_Op Asm_x86_64_Op;
enum Asm_x86_64_Op {
    ASM_X86_64_OP_MOV = 0,
    ASM_X86_64_OP_MOVSX,
    ASM_X86_64_OP_LEA,
    ASM_X86_64_OP_PUSH,
    ASM_X86_64_OP_POP,
    ASM_X86_64_OP_ADD,
    ASM_X86_64_OP_SUB,
    ASM_X86_64_OP_IMUL,
    ASM_X86_64_OP_IDIV,
    ASM_X86_64_OP_DIV,
    ASM_X86_64_OP_AND,
    ASM_X86_64_OP_OR,
    ASM_X86_64_OP_XOR,
    ASM_X86_64_OP_NOT,
    ASM_X86_64_OP_SHL,
    ASM_X86_64_OP_SAR,
    ASM_X86_64_OP_SHR,
    ASM_X86_64_OP_CQO,
    ASM_X86_64_OP_NEG,
    ASM_X86_64_OP_CMP,
    ASM_X86_64_OP_SETE,
    ASM_X86_64_OP_SETNE,
    ASM_X86_64_OP_SETL,
    ASM_X86_64_OP_SETLE,
    ASM_X86_64_OP_SETB,
    ASM_X86_64_OP_SETBE,
    ASM_X86_64_OP_MOVZX,
    ASM_X86_64_OP_JMP,
    ASM_X86_64_OP_JE,
    ASM_X86_64_OP_JNE,
    ASM_X86_64_OP_CALL,
    ASM_X86_64_OP_CALL_REG,
    ASM_X86_64_OP_RET,
    ASM_X86_64_OP_SYSCALL,
    ASM_X86_64_OP_SETA,
    ASM_X86_64_OP_SETAE,
    ASM_X86_64_OP_SETP,
    ASM_X86_64_OP_SETNP,
    ASM_X86_64_OP_ADDSD,
    ASM_X86_64_OP_ADDSS,
    ASM_X86_64_OP_SUBSD,
    ASM_X86_64_OP_SUBSS,
    ASM_X86_64_OP_MULSD,
    ASM_X86_64_OP_MULSS,
    ASM_X86_64_OP_DIVSD,
    ASM_X86_64_OP_DIVSS,
    ASM_X86_64_OP_UCOMISD,
    ASM_X86_64_OP_UCOMISS,
    ASM_X86_64_OP_CVTSS2SD,
    ASM_X86_64_OP_CVTSD2SS,
    ASM_X86_64_OP_CVTSI2SD,  // from a 64-bit register
    ASM_X86_64_OP_CVTSI2SS,  // from a 64-bit register
    ASM_X86_64_OP_CVTTSD2SI, // into a 64-bit register
    ASM_X86_64_OP_CVTTSS2SI, // into a 64-bit register
    ASM_X86_64_OP_FLDT,
    ASM_X86_64_OP_FSTPT,
    ASM_X86_64_OP_FLDL,
    ASM_X86_64_OP_FSTPL,
    ASM_X86_64_OP_FLDS,
    ASM_X86_64_OP_FSTPS,
    ASM_X86_64_OP_FILDQ,
    ASM_X86_64_OP_FISTTPQ,
    ASM_X86_64_OP_FADDP,     // %st(1) += %st, then pop
    ASM_X86_64_OP_FMULP,     // %st(1) *= %st, then pop
    ASM_X86_64_OP_FSUBRP,    // %st(1) -= %st, then pop, as gas spells it
    ASM_X86_64_OP_FDIVRP,    // %st(1) /= %st, then pop, as gas spells it
    ASM_X86_64_OP_FCHS,
    ASM_X86_64_OP_FUCOMIP,   // compare %st with %st(1), then pop
    ASM_X86_64_OP_FSTP,      // pop %st
    ASM_X86_64_OP_COUNT // number of operations
};

// How an operand is addressed.
typedef enum Asm_x86_64_OperandKind Asm_x86_64_OperandKind;
enum Asm_x86_64_OperandKind {
    ASM_X86_64_OPERAND_NONE,
    ASM_X86_64_OPERAND_REG,   // ao_reg, ao_width      %rax / %al
    ASM_X86_64_OPERAND_IMM,   // ao_imm                $42
    ASM_X86_64_OPERAND_MEM,   // ao_reg (base), ao_disp   -8(%rbp)
    ASM_X86_64_OPERAND_RIP,   // ao_label              .Lstr0(%rip)
    ASM_X86_64_OPERAND_LABEL, // ao_label              jump / call target
    ASM_X86_64_OPERAND_XMM,   // ao_xmm                %xmm0
    ASM_X86_64_OPERAND_ST,    // ao_st                 %st(1)
    ASM_X86_64_OPERAND_COUNT  // number of kinds
};

// The kind of one item in the instruction list.
typedef enum Asm_x86_64_ItemKind Asm_x86_64_ItemKind;
enum Asm_x86_64_ItemKind {
    ASM_X86_64_ITEM_INSTR,     // ai_op, ai_dst, ai_src
    ASM_X86_64_ITEM_LABEL,     // ai_label defined here
    ASM_X86_64_ITEM_GLOBL,     // ai_label marked global
    ASM_X86_64_ITEM_SECTION,   // switch to ai_secname
    ASM_X86_64_ITEM_BYTES,     // ai_bytes / ai_nbytes raw data
    ASM_X86_64_ITEM_ADDR,      // eight bytes holding the address of ai_label
    ASM_X86_64_ITEM_DIRECTIVE, // ai_text raw assembler line
    ASM_X86_64_ITEM_COUNT      // number of kinds
};

// A single instruction operand.
typedef struct Asm_x86_64_Operand Asm_x86_64_Operand;
struct Asm_x86_64_Operand {
    Asm_x86_64_OperandKind ao_kind;
    Asm_x86_64_Reg         ao_reg;    // REG, or base of MEM
    int64_t                ao_imm;    // IMM
    int32_t                ao_disp;   // MEM displacement
    const char            *ao_label;  // RIP / LABEL
    Asm_x86_64_Width       ao_width;  // REG width, as ASM_X86_64_WIDTH_*
    Asm_x86_64_Xmm         ao_xmm;    // XMM
    int32_t                ao_st;     // ST, counted from the top of the x87 stack
};

// One node in the ordered instruction list.
typedef struct Asm_x86_64_Item Asm_x86_64_Item;
struct Asm_x86_64_Item {
    Asm_x86_64_Item     *ai_next;
    Asm_x86_64_ItemKind  ai_kind;
    Asm_x86_64_Op        ai_op;       // INSTR
    Asm_x86_64_Operand   ai_dst;      // INSTR
    Asm_x86_64_Operand   ai_src;      // INSTR
    const char          *ai_label;    // LABEL / GLOBL / ADDR
    int64_t              ai_addend;   // ADDR
    const char          *ai_text;     // DIRECTIVE
    const char          *ai_secname;  // SECTION
    uint32_t             ai_sectype;  // SECTION
    uint64_t             ai_secflags; // SECTION
    uint8_t             *ai_bytes;    // BYTES
    size_t               ai_nbytes;   // BYTES
};

// Operand constructors
Asm_x86_64_Operand Asm_x86_64_Reg64(Asm_x86_64_Reg reg);
Asm_x86_64_Operand Asm_x86_64_Reg8(Asm_x86_64_Reg reg);
Asm_x86_64_Operand Asm_x86_64_RegWidth(Asm_x86_64_Reg reg, Asm_x86_64_Width width);
Asm_x86_64_Operand Asm_x86_64_Imm(int64_t val);
Asm_x86_64_Operand Asm_x86_64_Mem(Asm_x86_64_Reg base, int32_t disp);
Asm_x86_64_Operand Asm_x86_64_Rip(const char *label);
Asm_x86_64_Operand Asm_x86_64_Target(const char *label);
Asm_x86_64_Operand Asm_x86_64_XmmReg(Asm_x86_64_Xmm xmm);
Asm_x86_64_Operand Asm_x86_64_St(int32_t st);

// Instruction list
Asm_x86_64_Item *Asm_x86_64_New(Asm_x86_64_ItemKind kind);
void Asm_x86_64_Clear(void);
Asm_x86_64_Item *Asm_x86_64_Items(void);

// Non-instruction items
void Asm_x86_64_EmitLabel(const char *name, ...);
void Asm_x86_64_EmitSection(const char *name, uint32_t type, uint64_t flags);
void Asm_x86_64_EmitGlobl(const char *name, ...);
void Asm_x86_64_EmitBytes(const void *data, size_t len);
void Asm_x86_64_EmitAddress(const char *label, int64_t addend);
void Asm_x86_64_EmitDirective(const char *text, ...);

// Register-to-register
void Asm_x86_64_EmitAdd(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitSub(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitImul(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitAnd(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitOr(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitXor(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitCmp(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitMovRR(Asm_x86_64_Reg src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitMovRRWidth(Asm_x86_64_Reg src, Asm_x86_64_Reg dst, Asm_x86_64_Width width);
void Asm_x86_64_EmitMovsx(Asm_x86_64_Reg src, Asm_x86_64_Reg dst, Asm_x86_64_Width width);
void Asm_x86_64_EmitMovzx(Asm_x86_64_Reg src, Asm_x86_64_Reg dst, Asm_x86_64_Width width);

// Single register
void Asm_x86_64_EmitIdiv(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitDiv(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitNeg(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitNot(Asm_x86_64_Reg reg);

// Shifts of a register by %cl
void Asm_x86_64_EmitShl(Asm_x86_64_Reg dst);
void Asm_x86_64_EmitSar(Asm_x86_64_Reg dst);
void Asm_x86_64_EmitShr(Asm_x86_64_Reg dst);
void Asm_x86_64_EmitCqo(void);

// Condition flags to a register
void Asm_x86_64_EmitSete(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetne(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetl(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetle(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetb(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetbe(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSeta(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetae(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetp(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitSetnp(Asm_x86_64_Reg reg);

// Immediate operands
void Asm_x86_64_EmitCmpImm(int64_t imm, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitMovImm(int64_t imm, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitMovImm8(int64_t imm, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitAddImm(int64_t imm, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitSubImm(int64_t imm, Asm_x86_64_Reg dst);

// Memory loads, stores and addresses
void Asm_x86_64_EmitMovLoad(Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Reg dst, Asm_x86_64_Width width);
void Asm_x86_64_EmitMovLoadZero(Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Reg dst, Asm_x86_64_Width width);
void Asm_x86_64_EmitMovStore(Asm_x86_64_Reg src, Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Width width);
void Asm_x86_64_EmitLea(Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitLeaRip(Asm_x86_64_Reg dst, const char *label, ...);

// Stack
void Asm_x86_64_EmitPush(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitPop(Asm_x86_64_Reg reg);

// Jumps, calls and returns
void Asm_x86_64_EmitJmp(const char *label, ...);
void Asm_x86_64_EmitJe(const char *label, ...);
void Asm_x86_64_EmitJne(const char *label, ...);
void Asm_x86_64_EmitCall(const char *label, ...);
void Asm_x86_64_EmitCallReg(Asm_x86_64_Reg reg);
void Asm_x86_64_EmitRet(void);
void Asm_x86_64_EmitSyscall(void);

// SSE scalars
void Asm_x86_64_EmitMovToXmm(Asm_x86_64_Reg src, Asm_x86_64_Xmm dst);
void Asm_x86_64_EmitMovFromXmm(Asm_x86_64_Xmm src, Asm_x86_64_Reg dst);
void Asm_x86_64_EmitSse(Asm_x86_64_Op op, Asm_x86_64_Xmm src, Asm_x86_64_Xmm dst);
void Asm_x86_64_EmitCvtToSse(Asm_x86_64_Op op, Asm_x86_64_Reg src, Asm_x86_64_Xmm dst);
void Asm_x86_64_EmitCvtFromSse(Asm_x86_64_Op op, Asm_x86_64_Xmm src, Asm_x86_64_Reg dst);

// The x87 stack
void Asm_x86_64_EmitX87Mem(Asm_x86_64_Op op, Asm_x86_64_Reg base, int32_t disp);
void Asm_x86_64_EmitX87(Asm_x86_64_Op op);

#endif // ASM_X86_64_H
