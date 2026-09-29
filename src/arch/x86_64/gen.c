/*
 * C source file for x86-64 code generation.
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
#include "arch/x86_64/gen.h"

// Number of integer arguments the ABI passes in registers.
#define GEN_X86_64_SYSV_MAX_REG_ARGS 6

// Required %rsp alignment.
#define GEN_X86_64_SYSV_STACK_ALIGN 16

// Bytes a variadic function reserves to spill the argument registers into.
#define GEN_X86_64_SYSV_VA_SAVE_SIZE (GEN_X86_64_SYSV_MAX_REG_ARGS * GEN_X86_64_SYSV_EIGHTBYTE)

// Size in bytes of a stack slot and a general-purpose register.
#define GEN_X86_64_WORD_SIZE 8

// Largest global a single scalar initializer may fill.
#define GEN_X86_64_MAX_INIT 8

// Most addresses one global's image may hold.
#define GEN_X86_64_MAX_ADDRS 256

// The bits of a byte, for splitting an initializer into them.
#define GEN_X86_64_BYTE_MASK 0xFF

// Chunk sizes an aggregate copy moves, largest first.
#define GEN_X86_64_COPY_QUAD (ASM_X86_64_WIDTH_64 / ASM_X86_64_BITS_PER_BYTE)
#define GEN_X86_64_COPY_LONG (ASM_X86_64_WIDTH_32 / ASM_X86_64_BITS_PER_BYTE)

// Number of values currently pushed with Gen_x86_64_EmitPush().
static int32_t Gen_x86_64_Depth;

// Source of unique label numbers.
static int32_t Gen_x86_64_LabelId;

// Label numbers the innermost loop uses for break and continue.
static int32_t Gen_x86_64_BreakId = -1;
static int32_t Gen_x86_64_ContinueId = -1;

// The function currently being emitted.
static const Ast_Func *Gen_x86_64_CurrFunc;

// Frame offset holding the caller's buffer pointer for a memory return.
static int32_t Gen_x86_64_SysV_RetPtrOffset;

// Registers used to pass the first six integer arguments.
static const Asm_x86_64_Reg Gen_x86_64_SysV_ArgReg[GEN_X86_64_SYSV_MAX_REG_ARGS] = {
    ASM_X86_64_REG_RDI,
    ASM_X86_64_REG_RSI,
    ASM_X86_64_REG_RDX,
    ASM_X86_64_REG_RCX,
    ASM_X86_64_REG_R8,
    ASM_X86_64_REG_R9
};

// Give a type the class the SysV ABI passes it by.
Gen_x86_64_SysV_Class Gen_x86_64_SysV_Classify(const Ast_Type *type)
{
    if (! Sem_IsAggregate(type)) {
        return GEN_X86_64_SYSV_CLASS_INTEGER;
    }
    if (type->at_size > GEN_X86_64_SYSV_MAX_REG_SIZE) {
        return GEN_X86_64_SYSV_CLASS_MEMORY;
    }
    return GEN_X86_64_SYSV_CLASS_INTEGER;
}

// Return the number of registers or stack slots a type occupies when passed.
int32_t Gen_x86_64_SysV_Eightbytes(const Ast_Type *type)
{
    int32_t size = Sem_IsAggregate(type) ? type->at_size : GEN_X86_64_SYSV_EIGHTBYTE;
    return (size + GEN_X86_64_SYSV_EIGHTBYTE - 1) / GEN_X86_64_SYSV_EIGHTBYTE;
}

// True when an argument of this type is passed on the stack.
bool Gen_x86_64_SysV_InMemory(const Ast_Type *type)
{
    return Gen_x86_64_SysV_Classify(type) == GEN_X86_64_SYSV_CLASS_MEMORY;
}

// True when this type is returned through a hidden pointer.
bool Gen_x86_64_SysV_ReturnsInMemory(const Ast_Type *type)
{
    return type && Sem_IsAggregate(type) && type->at_size > GEN_X86_64_SYSV_MAX_REG_SIZE;
}

// Turn the value of a return expression into what the ABI returns.
void Gen_x86_64_SysV_EmitReturnValue(Ast_Node *node)
{
    if (! Sem_IsAggregate(node->an_type)) {
        return;
    }
    if (Gen_x86_64_SysV_ReturnsInMemory(node->an_type)) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, Gen_x86_64_SysV_RetPtrOffset, ASM_X86_64_REG_RDI, ASM_X86_64_WIDTH_64);
        Gen_x86_64_EmitCopy(node->an_type->at_size);
        return;
    }

    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
    if (Gen_x86_64_SysV_Eightbytes(node->an_type) > 1) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RCX, GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RDX, ASM_X86_64_WIDTH_64);
    }
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RCX, 0, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
}

// Spill one incoming parameter into its frame slot.
void Gen_x86_64_SysV_EmitParam(Ast_Var *param, int32_t *reg, int32_t *stack)
{
    int32_t slots = Gen_x86_64_SysV_Eightbytes(param->av_type);
    bool inReg = ! Gen_x86_64_SysV_InMemory(param->av_type) && *reg + slots <= GEN_X86_64_SYSV_MAX_REG_ARGS;

    if (! Sem_IsAggregate(param->av_type)) {
        Asm_x86_64_Width width = Gen_x86_64_TypeWidth(param->av_type);
        if (inReg) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_SysV_ArgReg[(*reg)++], ASM_X86_64_REG_RBP, param->av_offset, width);
        } else {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, 2 * GEN_X86_64_SYSV_EIGHTBYTE + (*stack)++ * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, param->av_offset, width);
        }
        return;
    }

    for (int32_t k = 0; k < slots; k++) {
        if (inReg) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_SysV_ArgReg[(*reg)++], ASM_X86_64_REG_RBP, param->av_offset + k * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_WIDTH_64);
        } else {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, 2 * GEN_X86_64_SYSV_EIGHTBYTE + (*stack)++ * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, param->av_offset + k * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_WIDTH_64);
        }
    }
}

// Return the first argument register the argument at index takes.
int32_t Gen_x86_64_SysV_ArgRegBase(Ast_Node *args, int32_t index, int32_t nHidden)
{
    int32_t used = nHidden;
    int32_t i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int32_t want = Gen_x86_64_SysV_Eightbytes(arg->an_type);
        if (Gen_x86_64_SysV_InMemory(arg->an_type) || used + want > GEN_X86_64_SYSV_MAX_REG_ARGS) {
            if (i == index) {
                return -1;
            }
            continue;
        }
        if (i == index) {
            return used;
        }
        used += want;
    }
    return -1;
}

// Count the eightbytes a call leaves on the stack for its memory arguments.
int32_t Gen_x86_64_SysV_CallStackSlots(Ast_Node *args, int32_t nHidden)
{
    int32_t slots = 0;
    int32_t i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        if (Gen_x86_64_SysV_ArgRegBase(args, i, nHidden) < 0) {
            slots += Gen_x86_64_SysV_Eightbytes(arg->an_type);
        }
    }
    return slots;
}

// Evaluate one argument and push its eightbytes, lowest ending on top.
void Gen_x86_64_SysV_PushArg(Ast_Node *arg)
{
    Gen_x86_64_EmitExpr(arg);

    if (! Sem_IsAggregate(arg->an_type)) {
        Gen_x86_64_EmitPush();
        return;
    }
    for (int32_t k = Gen_x86_64_SysV_Eightbytes(arg->an_type) - 1; k >= 0; k--) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, k * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitPush(ASM_X86_64_REG_RCX);
        Gen_x86_64_Depth++;
    }
}

// Push the arguments the ABI places on the stack, last one first.
void Gen_x86_64_SysV_CallPushStack(Ast_Node *args, Ast_Node *arg, int32_t index, int32_t nHidden)
{
    if (! arg) {
        return;
    }
    Gen_x86_64_SysV_CallPushStack(args, arg->an_next, index + 1, nHidden);
    if (Gen_x86_64_SysV_ArgRegBase(args, index, nHidden) < 0) {
        Gen_x86_64_SysV_PushArg(arg);
    }
}

// Push the arguments the ABI passes in registers, last one first.
void Gen_x86_64_SysV_CallPushReg(Ast_Node *args, Ast_Node *arg, int32_t index, int32_t nHidden)
{
    if (! arg) {
        return;
    }
    Gen_x86_64_SysV_CallPushReg(args, arg->an_next, index + 1, nHidden);
    if (Gen_x86_64_SysV_ArgRegBase(args, index, nHidden) >= 0) {
        Gen_x86_64_SysV_PushArg(arg);
    }
}

// Pop the pushed register arguments into the registers the ABI assigns them.
void Gen_x86_64_SysV_CallPopReg(Ast_Node *args, int32_t nHidden)
{
    int32_t i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int32_t base = Gen_x86_64_SysV_ArgRegBase(args, i, nHidden);
        if (base < 0) {
            continue;
        }
        for (int32_t k = 0; k < Gen_x86_64_SysV_Eightbytes(arg->an_type); k++) {
            Gen_x86_64_EmitPop(Gen_x86_64_SysV_ArgReg[base + k]);
        }
    }
}

// Emit a call.
void Gen_x86_64_SysV_EmitCall(Ast_Node *node)
{
    int32_t nHidden = Gen_x86_64_SysV_ReturnsInMemory(node->an_type) ? 1 : 0;
    int32_t nStack = Gen_x86_64_SysV_CallStackSlots(node->an_args, nHidden);

    int32_t nAlignPad = (Gen_x86_64_Depth + nStack) % (GEN_X86_64_SYSV_STACK_ALIGN / GEN_X86_64_SYSV_EIGHTBYTE);
    if (nAlignPad) {
        Asm_x86_64_EmitSubImm(GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RSP);
        Gen_x86_64_Depth++;
    }

    if (node->an_lhs) {
        Gen_x86_64_EmitExpr(node->an_lhs);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, node->an_calltmp, ASM_X86_64_WIDTH_64);
    }

    Gen_x86_64_SysV_CallPushStack(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_SysV_CallPushReg(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_SysV_CallPopReg(node->an_args, nHidden);

    if (nHidden) {
        Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RDI);
    }

    Asm_x86_64_EmitMovImm8(0, ASM_X86_64_REG_RAX);
    if (node->an_lhs) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, node->an_calltmp, ASM_X86_64_REG_R11, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitCallReg(ASM_X86_64_REG_R11);
    } else {
        Asm_x86_64_EmitCall(node->an_funcname);
    }

    if (nStack + nAlignPad > 0) {
        Asm_x86_64_EmitAddImm(GEN_X86_64_SYSV_EIGHTBYTE * (nStack + nAlignPad), ASM_X86_64_REG_RSP);
        Gen_x86_64_Depth -= nStack + nAlignPad;
    }

    if (Sem_IsAggregate(node->an_type) && ! nHidden) {
        int32_t slots = Gen_x86_64_SysV_Eightbytes(node->an_type);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_WIDTH_64);
        if (slots > 1) {
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RBP, node->an_tmp + GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_WIDTH_64);
        }
        Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RAX);
    }
}

// Spill every argument register into the save area.
void Gen_x86_64_SysV_EmitVaSaveArea(void)
{
    for (int32_t i = 0; i < GEN_X86_64_SYSV_MAX_REG_ARGS; i++) {
        Asm_x86_64_EmitMovStore(Gen_x86_64_SysV_ArgReg[i], ASM_X86_64_REG_RBP, -GEN_X86_64_SYSV_VA_SAVE_SIZE + i * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_WIDTH_64);
    }
}

// Count the argument registers and stack slots the named parameters used.
void Gen_x86_64_SysV_CountNamedArgs(const Ast_Func *func, int32_t *reg, int32_t *stack)
{
    *reg   = Gen_x86_64_SysV_ReturnsInMemory(func->af_ret) ? 1 : 0;
    *stack = 0;

    for (Ast_Var *param = func->af_params; param; param = param->av_param_next) {
        int32_t slots = Gen_x86_64_SysV_Eightbytes(param->av_type);
        if (! Gen_x86_64_SysV_InMemory(param->av_type) && *reg + slots <= GEN_X86_64_SYSV_MAX_REG_ARGS) {
            *reg += slots;
        } else {
            *stack += slots;
        }
    }
}

// Fill the va_list at the address in %rax.
void Gen_x86_64_SysV_EmitVaStart(void)
{
    int32_t reg = 0;
    int32_t stack = 0;

    Gen_x86_64_SysV_CountNamedArgs(Gen_x86_64_CurrFunc, &reg, &stack);
    Asm_x86_64_EmitMovImm(reg * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX, GEN_X86_64_SYSV_VA_GP_OFFSET, ASM_X86_64_WIDTH_32);
    Asm_x86_64_EmitMovImm(GEN_X86_64_SYSV_VA_SAVE_SIZE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX, GEN_X86_64_SYSV_VA_FP_OFFSET, ASM_X86_64_WIDTH_32);
    Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, 2 * GEN_X86_64_SYSV_EIGHTBYTE + stack * GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX, GEN_X86_64_SYSV_VA_OVERFLOW, ASM_X86_64_WIDTH_64);
    Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, -GEN_X86_64_SYSV_VA_SAVE_SIZE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX, GEN_X86_64_SYSV_VA_REG_SAVE, ASM_X86_64_WIDTH_64);
}

// Read into %rax the next argument the va_list at %rax reaches.
void Gen_x86_64_SysV_EmitVaArg(const Ast_Type *type)
{
    int32_t count = Gen_x86_64_Count();

    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, GEN_X86_64_SYSV_VA_GP_OFFSET, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_32);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitCmpImm(GEN_X86_64_SYSV_VA_SAVE_SIZE, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitSetl(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
    Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitJe(".L.va.stack.%d", count);

    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, GEN_X86_64_SYSV_VA_REG_SAVE, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
    Asm_x86_64_EmitAdd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitAddImm(GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, GEN_X86_64_SYSV_VA_GP_OFFSET, ASM_X86_64_WIDTH_32);
    Asm_x86_64_EmitJmp(".L.va.end.%d", count);

    Asm_x86_64_EmitLabel(".L.va.stack.%d", count);
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, GEN_X86_64_SYSV_VA_OVERFLOW, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitAddImm(GEN_X86_64_SYSV_EIGHTBYTE, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, GEN_X86_64_SYSV_VA_OVERFLOW, ASM_X86_64_WIDTH_64);

    Asm_x86_64_EmitLabel(".L.va.end.%d", count);
    Gen_x86_64_EmitLoad(type);
}

// Return the next unique label number.
int32_t Gen_x86_64_Count(void)
{
    return Gen_x86_64_LabelId++;
}

// Push %rax onto the stack and track the depth.
void Gen_x86_64_EmitPush(void)
{
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    Gen_x86_64_Depth++;
}

// Pop the top of the stack into reg and track the depth.
void Gen_x86_64_EmitPop(Asm_x86_64_Reg reg)
{
    Asm_x86_64_EmitPop(reg);
    Gen_x86_64_Depth--;
}

// Round n up to the nearest multiple of align.
int32_t Gen_x86_64_AlignTo(int32_t n, int32_t align)
{
    return (n + align - 1) / align * align;
}

// Return the frame bytes a local reserves.
int32_t Gen_x86_64_SlotSize(const Ast_Type *type)
{
    if (! Sem_IsAggregate(type)) {
        return type->at_size;
    }
    return Gen_x86_64_AlignTo(type->at_size, GEN_X86_64_WORD_SIZE);
}

// Return the operand width in bits used to load or store a value of type.
Asm_x86_64_Width Gen_x86_64_TypeWidth(const Ast_Type *type)
{
    return type->at_size * ASM_X86_64_BITS_PER_BYTE;
}

// Compute into %rax the address an expression designates, lvalue or not.
void Gen_x86_64_EmitAddr(Ast_Node *node)
{
    switch (node->an_kind) {
        case AST_NODE_KIND_VAR: {
            if (node->an_var->av_global) {
                Asm_x86_64_EmitLeaRip(ASM_X86_64_REG_RAX, "%s", node->an_var->av_symbol);
            } else {
                Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_var->av_offset, ASM_X86_64_REG_RAX);
            }
        } break;
        case AST_NODE_KIND_DEREF: {
            Gen_x86_64_EmitExpr(node->an_lhs);
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            Asm_x86_64_EmitLeaRip(ASM_X86_64_REG_RAX, "%s", node->an_funcname);
        } break;
        case AST_NODE_KIND_MEMBER: {
            Gen_x86_64_EmitAddr(node->an_lhs);
            if (node->an_member->am_offset) {
                Asm_x86_64_EmitAddImm(node->an_member->am_offset, ASM_X86_64_REG_RAX);
            }
        } break;
        case AST_NODE_KIND_CALL:
        case AST_NODE_KIND_ASSIGN:
        case AST_NODE_KIND_COMMA:
        case AST_NODE_KIND_COND: {
            Err_AssertAt(node->an_line, Gen_x86_64_ByAddress(node->an_type), ERR_GEN_NOT_LVALUE);
            Gen_x86_64_EmitExpr(node);
        } break;
        case AST_NODE_KIND_COMPOUND: {
            for (Ast_Node *stmt = node->an_body; stmt; stmt = stmt->an_next) {
                Gen_x86_64_EmitStmt(stmt);
            }
            Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_var->av_offset, ASM_X86_64_REG_RAX);
        } break;
        default: {
            Err_RaiseAt(node->an_line, ERR_GEN_NOT_LVALUE);
        }
    }
}

// Return the signedness an operator's operands give it.
Ast_TypeSign Gen_x86_64_Sign(const Ast_Node *node)
{
    const Ast_Type *type = node->an_lhs ? node->an_lhs->an_type : node->an_type;

    return type ? type->at_sign : AST_TYPE_SIGNED;
}

// Load a value of type from disp(%base) into %dst.
void Gen_x86_64_EmitLoadFrom(Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Reg dst, const Ast_Type *type)
{
    if (type->at_sign == AST_TYPE_UNSIGNED || Ast_IsFloating(type)) {
        Asm_x86_64_EmitMovLoadZero(base, disp, dst, Gen_x86_64_TypeWidth(type));
        return;
    }
    Asm_x86_64_EmitMovLoad(base, disp, dst, Gen_x86_64_TypeWidth(type));
}

// Load the value at the address in %rax.
void Gen_x86_64_EmitLoad(const Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY || type->at_kind == AST_TYPE_KIND_FUNC || Gen_x86_64_ByAddress(type)) {
        return;
    }
    Gen_x86_64_EmitLoadFrom(ASM_X86_64_REG_RAX, 0, ASM_X86_64_REG_RAX, type);
}

// Narrow the value in %rax to type.
void Gen_x86_64_EmitCast(const Ast_Type *type)
{
    Asm_x86_64_Width width = Gen_x86_64_TypeWidth(type);

    switch (type->at_kind) {
        case AST_TYPE_KIND_BOOL: {
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSetne(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
        } break;
        case AST_TYPE_KIND_CHAR:
        case AST_TYPE_KIND_SHORT:
        case AST_TYPE_KIND_INT: {
            if (type->at_sign != AST_TYPE_UNSIGNED) {
                Asm_x86_64_EmitMovsx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, width);
            } else if (width == ASM_X86_64_WIDTH_32) {
                Asm_x86_64_EmitMovRRWidth(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, width);
            } else {
                Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, width);
            }
        } break;
        case AST_TYPE_KIND_LONG:
        case AST_TYPE_KIND_LLONG:
        case AST_TYPE_KIND_FLOAT:
        case AST_TYPE_KIND_DOUBLE:
        case AST_TYPE_KIND_LDOUBLE:
        case AST_TYPE_KIND_VOID:
        case AST_TYPE_KIND_PTR:
        case AST_TYPE_KIND_ARRAY:
        case AST_TYPE_KIND_FUNC:
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_UNION:
        case AST_TYPE_KIND_COUNT: {
            // already as wide as a register
        } break;
    }
}

// Copy size bytes from %rax to %rdi.
void Gen_x86_64_EmitCopy(int32_t size)
{
    int32_t off = 0;

    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RSI);
    while (size - off >= GEN_X86_64_COPY_QUAD) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_64);
        off += GEN_X86_64_COPY_QUAD;
    }
    while (size - off >= GEN_X86_64_COPY_LONG) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_32);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_32);
        off += GEN_X86_64_COPY_LONG;
    }
    while (off < size) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_8);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_8);
        off++;
    }
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
}

// Write size zero bytes at the address in %rdi.
void Gen_x86_64_EmitZero(int32_t size)
{
    int32_t off = 0;

    Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RCX);
    while (size - off >= GEN_X86_64_COPY_QUAD) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_64);
        off += GEN_X86_64_COPY_QUAD;
    }
    while (size - off >= GEN_X86_64_COPY_LONG) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_32);
        off += GEN_X86_64_COPY_LONG;
    }
    while (off < size) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_8);
        off++;
    }
}

// Return the bitfield a node reads or writes.
const Ast_Member *Gen_x86_64_Bitfield(const Ast_Node *node)
{
    if (node->an_kind == AST_NODE_KIND_MEMBER && node->an_member->am_bits) {
        return node->an_member;
    }
    return NULL;
}

// Load into %rax the bitfield at the address in %rdi.
void Gen_x86_64_EmitBitfieldLoad(const Ast_Member *member)
{
    Gen_x86_64_EmitLoadFrom(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, member->am_type);
    Asm_x86_64_EmitMovImm(ASM_X86_64_WIDTH_64 - member->am_bitoff - member->am_bits, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitShl(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovImm(ASM_X86_64_WIDTH_64 - member->am_bits, ASM_X86_64_REG_RCX);
    if (member->am_type->at_sign == AST_TYPE_UNSIGNED) {
        Asm_x86_64_EmitShr(ASM_X86_64_REG_RAX);
    } else {
        Asm_x86_64_EmitSar(ASM_X86_64_REG_RAX);
    }
}

// Store the low bits of %rax into the bitfield at the address in %rdi.
void Gen_x86_64_EmitBitfieldStore(const Ast_Member *member)
{
    int64_t mask = (((int64_t) 1 << member->am_bits) - 1) << member->am_bitoff;
    Asm_x86_64_Width width = Gen_x86_64_TypeWidth(member->am_type);

    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDX);
    Asm_x86_64_EmitMovImm(member->am_bitoff, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitShl(ASM_X86_64_REG_RDX);
    Asm_x86_64_EmitMovImm(mask, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitAnd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDX);
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, width);
    Asm_x86_64_EmitMovImm(~mask, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitAnd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitOr(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, width);
    Gen_x86_64_EmitBitfieldLoad(member);
}

// Return whether a value of this type travels in %rax as its address.
bool Gen_x86_64_ByAddress(const Ast_Type *type)
{
    return Sem_IsAggregate(type) || type->at_kind == AST_TYPE_KIND_LDOUBLE;
}

// Return whether a value of this type is a float or a double.
bool Gen_x86_64_IsSse(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_FLOAT || type->at_kind == AST_TYPE_KIND_DOUBLE;
}

// Return whether this is an unsigned integer too wide for a signed conversion.
bool Gen_x86_64_IsWideUnsigned(const Ast_Type *type)
{
    return Ast_IsInteger(type) && type->at_sign == AST_TYPE_UNSIGNED && type->at_size == GEN_X86_64_WORD_SIZE;
}

// Return the SSE instruction an arithmetic operator takes at a type.
Asm_x86_64_Op Gen_x86_64_SseOp(Ast_NodeKind kind, const Ast_Type *type)
{
    bool dbl = type->at_kind == AST_TYPE_KIND_DOUBLE;

    switch (kind) {
        case AST_NODE_KIND_ADD: {
            return dbl ? ASM_X86_64_OP_ADDSD : ASM_X86_64_OP_ADDSS;
        } break;
        case AST_NODE_KIND_SUB: {
            return dbl ? ASM_X86_64_OP_SUBSD : ASM_X86_64_OP_SUBSS;
        } break;
        case AST_NODE_KIND_MUL: {
            return dbl ? ASM_X86_64_OP_MULSD : ASM_X86_64_OP_MULSS;
        } break;
        case AST_NODE_KIND_DIV: {
            return dbl ? ASM_X86_64_OP_DIVSD : ASM_X86_64_OP_DIVSS;
        } break;
        default: {
            return dbl ? ASM_X86_64_OP_UCOMISD : ASM_X86_64_OP_UCOMISS;
        }
    }
}

// Return the x87 instruction an arithmetic operator takes.
Asm_x86_64_Op Gen_x86_64_X87Op(Ast_NodeKind kind)
{
    switch (kind) {
        case AST_NODE_KIND_ADD: {
            return ASM_X86_64_OP_FADDP;
        } break;
        case AST_NODE_KIND_SUB: {
            return ASM_X86_64_OP_FSUBRP;
        } break;
        case AST_NODE_KIND_MUL: {
            return ASM_X86_64_OP_FMULP;
        } break;
        default: {
            return ASM_X86_64_OP_FDIVRP;
        }
    }
}

// Load a floating literal into %rax.
void Gen_x86_64_EmitFNum(const Ast_Node *node)
{
    uint8_t bytes[FP_EXTENDED_SIZE];
    uint64_t low = 0;
    uint64_t high = 0;

    switch (node->an_type->at_kind) {
        case AST_TYPE_KIND_FLOAT: {
            Asm_x86_64_EmitMovImm(Fp_FloatBits((float) node->an_fval), ASM_X86_64_REG_RAX);
        } break;
        case AST_TYPE_KIND_DOUBLE: {
            Asm_x86_64_EmitMovImm((int64_t) Fp_DoubleBits((double) node->an_fval), ASM_X86_64_REG_RAX);
        } break;
        default: {
            Fp_EncodeExtended(node->an_fval, bytes);
            for (int32_t i = 0; i < FP_EXTENDED_SIZE; i++) {
                if (i < FP_EXTENDED_TOP_OFF) {
                    low |= (uint64_t) bytes[i] << (i * FP_BITS_PER_BYTE);
                } else {
                    high |= (uint64_t) bytes[i] << ((i - FP_EXTENDED_TOP_OFF) * FP_BITS_PER_BYTE);
                }
            }
            Asm_x86_64_EmitMovImm((int64_t) low, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_WIDTH_64);
            Asm_x86_64_EmitMovImm((int64_t) high, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RBP, node->an_tmp + FP_EXTENDED_TOP_OFF, ASM_X86_64_WIDTH_16);
            Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RAX);
        }
    }
}

// Pop %st into a frame slot and leave its address in %rax.
void Gen_x86_64_EmitX87Result(int32_t tmp)
{
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FSTPT, ASM_X86_64_REG_RBP, tmp);
    Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, tmp, ASM_X86_64_REG_RAX);
}

// Push the integer in %rax onto the x87 stack.
void Gen_x86_64_EmitIntToX87(const Ast_Type *from)
{
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FILDQ, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
    if (! Gen_x86_64_IsWideUnsigned(from)) {
        return;
    }

    // fildq read the top bit as a sign, so add 2^64 back when it was set
    Asm_x86_64_EmitMovImm(GEN_X86_64_SIGN_SHIFT, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitShr(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovImm(GEN_X86_64_FLOAT_TWO_TO_64, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitImul(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDS, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitX87(ASM_X86_64_OP_FADDP);
}

// Push the arithmetic value in %rax onto the x87 stack.
void Gen_x86_64_EmitToX87(const Ast_Type *from)
{
    if (from->at_kind == AST_TYPE_KIND_LDOUBLE) {
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDT, ASM_X86_64_REG_RAX, 0);
        return;
    }
    if (! Gen_x86_64_IsSse(from)) {
        Gen_x86_64_EmitIntToX87(from);
        return;
    }
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitX87Mem(from->at_kind == AST_TYPE_KIND_FLOAT ? ASM_X86_64_OP_FLDS : ASM_X86_64_OP_FLDL, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
}

// Pop %st into %rax as a float or a double.
void Gen_x86_64_EmitX87ToSse(const Ast_Type *to)
{
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    if (to->at_kind == AST_TYPE_KIND_FLOAT) {
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FSTPS, ASM_X86_64_REG_RSP, 0);
        Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
        Asm_x86_64_EmitMovRRWidth(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_32);
        return;
    }
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FSTPL, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
}

// Pop %st into %rax as an integer, truncating toward zero.
void Gen_x86_64_EmitX87ToInt(const Ast_Type *to)
{
    Asm_x86_64_EmitPush(ASM_X86_64_REG_RAX);
    if (! Gen_x86_64_IsWideUnsigned(to)) {
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FISTTPQ, ASM_X86_64_REG_RSP, 0);
        Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
        Gen_x86_64_EmitCast(to);
        return;
    }

    // take 2^63 off a value that reaches it, and put the top bit back after
    Asm_x86_64_EmitMovImm(GEN_X86_64_FLOAT_TWO_TO_63, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RSP, 0, ASM_X86_64_WIDTH_64);
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDS, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitX87(ASM_X86_64_OP_FUCOMIP);
    Asm_x86_64_EmitSetbe(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDX);
    Asm_x86_64_EmitMovImm(GEN_X86_64_FLOAT_TWO_TO_63, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitImul(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RSP, 0, ASM_X86_64_WIDTH_64);
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDS, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitX87(ASM_X86_64_OP_FSUBRP);
    Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FISTTPQ, ASM_X86_64_REG_RSP, 0);
    Asm_x86_64_EmitPop(ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovImm(GEN_X86_64_SIGN_SHIFT, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitShl(ASM_X86_64_REG_RDX);
    Asm_x86_64_EmitXor(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RAX);
}

// Convert the value in %rax from one arithmetic type to another.
void Gen_x86_64_EmitConvert(const Ast_Type *from, const Ast_Type *to, int32_t tmp)
{
    bool wide = Gen_x86_64_IsWideUnsigned(from) || Gen_x86_64_IsWideUnsigned(to);

    if (to->at_kind == AST_TYPE_KIND_VOID) {
        return;
    }
    if (! Ast_IsFloating(from) && ! Ast_IsFloating(to)) {
        Gen_x86_64_EmitCast(to);
        return;
    }
    if (Gen_x86_64_IsSse(from) && Gen_x86_64_IsSse(to)) {
        if (from->at_kind != to->at_kind) {
            Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RAX, ASM_X86_64_XMM0);
            Asm_x86_64_EmitSse(to->at_kind == AST_TYPE_KIND_DOUBLE ? ASM_X86_64_OP_CVTSS2SD : ASM_X86_64_OP_CVTSD2SS, ASM_X86_64_XMM0, ASM_X86_64_XMM0);
            Asm_x86_64_EmitMovFromXmm(ASM_X86_64_XMM0, ASM_X86_64_REG_RAX);
        }
        return;
    }
    if (! wide && Ast_IsInteger(from) && Gen_x86_64_IsSse(to)) {
        Asm_x86_64_EmitCvtToSse(to->at_kind == AST_TYPE_KIND_DOUBLE ? ASM_X86_64_OP_CVTSI2SD : ASM_X86_64_OP_CVTSI2SS, ASM_X86_64_REG_RAX, ASM_X86_64_XMM0);
        Asm_x86_64_EmitMovFromXmm(ASM_X86_64_XMM0, ASM_X86_64_REG_RAX);
        return;
    }
    if (! wide && Gen_x86_64_IsSse(from) && Ast_IsInteger(to)) {
        Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RAX, ASM_X86_64_XMM0);
        Asm_x86_64_EmitCvtFromSse(from->at_kind == AST_TYPE_KIND_DOUBLE ? ASM_X86_64_OP_CVTTSD2SI : ASM_X86_64_OP_CVTTSS2SI, ASM_X86_64_XMM0, ASM_X86_64_REG_RAX);
        Gen_x86_64_EmitCast(to);
        return;
    }

    Gen_x86_64_EmitToX87(from);
    if (to->at_kind == AST_TYPE_KIND_LDOUBLE) {
        Gen_x86_64_EmitX87Result(tmp);
    } else if (Gen_x86_64_IsSse(to)) {
        Gen_x86_64_EmitX87ToSse(to);
    } else {
        Gen_x86_64_EmitX87ToInt(to);
    }
}

// Turn the flags an unordered compare set into 0 or 1 in %rax.
void Gen_x86_64_EmitFloatCompare(Ast_NodeKind kind)
{
    switch (kind) {
        case AST_NODE_KIND_EQ: {
            Asm_x86_64_EmitSete(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSetnp(ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitAnd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_NE: {
            Asm_x86_64_EmitSetne(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSetp(ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitOr(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_LT: {
            Asm_x86_64_EmitSeta(ASM_X86_64_REG_RAX);
        } break;
        default: {
            Asm_x86_64_EmitSetae(ASM_X86_64_REG_RAX);
        }
    }
    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
}

// Apply an arithmetic or comparison operator to floating %rax and %rdi.
void Gen_x86_64_EmitFloatBinary(Ast_Node *node)
{
    const Ast_Type *type = node->an_lhs->an_type;
    bool compare = node->an_kind == AST_NODE_KIND_EQ || node->an_kind == AST_NODE_KIND_NE || node->an_kind == AST_NODE_KIND_LT || node->an_kind == AST_NODE_KIND_LE;

    if (type->at_kind == AST_TYPE_KIND_LDOUBLE) {
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDT, ASM_X86_64_REG_RAX, 0);
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDT, ASM_X86_64_REG_RDI, 0);
        if (! compare) {
            Asm_x86_64_EmitX87(Gen_x86_64_X87Op(node->an_kind));
            Gen_x86_64_EmitX87Result(node->an_tmp);
            return;
        }
        Asm_x86_64_EmitX87(ASM_X86_64_OP_FUCOMIP);
        Asm_x86_64_EmitX87(ASM_X86_64_OP_FSTP);
        Gen_x86_64_EmitFloatCompare(node->an_kind);
        return;
    }

    if (! compare) {
        Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RAX, ASM_X86_64_XMM0);
        Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RDI, ASM_X86_64_XMM1);
        Asm_x86_64_EmitSse(Gen_x86_64_SseOp(node->an_kind, type), ASM_X86_64_XMM1, ASM_X86_64_XMM0);
        Asm_x86_64_EmitMovFromXmm(ASM_X86_64_XMM0, ASM_X86_64_REG_RAX);
        return;
    }
    Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RDI, ASM_X86_64_XMM0);
    Asm_x86_64_EmitMovToXmm(ASM_X86_64_REG_RAX, ASM_X86_64_XMM1);
    Asm_x86_64_EmitSse(Gen_x86_64_SseOp(node->an_kind, type), ASM_X86_64_XMM1, ASM_X86_64_XMM0);
    Gen_x86_64_EmitFloatCompare(node->an_kind);
}

// Negate the floating value in %rax.
void Gen_x86_64_EmitFloatNeg(Ast_Node *node)
{
    if (node->an_type->at_kind == AST_TYPE_KIND_LDOUBLE) {
        Asm_x86_64_EmitX87Mem(ASM_X86_64_OP_FLDT, ASM_X86_64_REG_RAX, 0);
        Asm_x86_64_EmitX87(ASM_X86_64_OP_FCHS);
        Gen_x86_64_EmitX87Result(node->an_tmp);
        return;
    }
    Asm_x86_64_EmitMovImm(node->an_type->at_kind == AST_TYPE_KIND_FLOAT ? GEN_X86_64_FLOAT_SIGN : GEN_X86_64_DOUBLE_SIGN, ASM_X86_64_REG_RCX);
    Asm_x86_64_EmitXor(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
}

// Bring a result in %rax back into its type.
void Gen_x86_64_EmitNarrow(const Ast_Type *type)
{
    if (Ast_IsInteger(type) && type->at_sign == AST_TYPE_UNSIGNED && type->at_size < GEN_X86_64_WORD_SIZE) {
        Gen_x86_64_EmitCast(type);
    }
}

// Divide %rax by %reg.
void Gen_x86_64_EmitDivide(Ast_TypeSign sign, Asm_x86_64_Reg reg)
{
    if (sign == AST_TYPE_UNSIGNED) {
        Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RDX);
        Asm_x86_64_EmitDiv(reg);
        return;
    }
    Asm_x86_64_EmitCqo();
    Asm_x86_64_EmitIdiv(reg);
}

// Shift %reg right by %cl.
void Gen_x86_64_EmitShift(Ast_TypeSign sign, Asm_x86_64_Reg reg)
{
    if (sign == AST_TYPE_UNSIGNED) {
        Asm_x86_64_EmitShr(reg);
        return;
    }
    Asm_x86_64_EmitSar(reg);
}

// Apply a compound assignment's operation to %rax and %rcx into %rax.
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, const Ast_Type *type, Ast_Line line)
{
    Ast_TypeSign sign = type->at_sign;

    switch (op) {
        case AST_NODE_KIND_ADD: {
            Asm_x86_64_EmitAdd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_SUB: {
            Asm_x86_64_EmitSub(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_MUL: {
            Asm_x86_64_EmitImul(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_DIV: {
            Gen_x86_64_EmitDivide(sign, ASM_X86_64_REG_RCX);
        } break;
        case AST_NODE_KIND_MOD: {
            Gen_x86_64_EmitDivide(sign, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_BITAND: {
            Asm_x86_64_EmitAnd(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_BITOR: {
            Asm_x86_64_EmitOr(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_BITXOR: {
            Asm_x86_64_EmitXor(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_SHL: {
            Asm_x86_64_EmitShl(ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_SHR: {
            Gen_x86_64_EmitShift(sign, ASM_X86_64_REG_RAX);
        } break;
        default: {
            Err_RaiseAt(line, ERR_GEN_UNEXPECTED_OPASSIGN, op);
        }
    }
}

// Emit code for an expression.
void Gen_x86_64_EmitExpr(Ast_Node *node)
{
    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            Asm_x86_64_EmitMovImm(node->an_val, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_FNUM: {
            Gen_x86_64_EmitFNum(node);
        } break;
        case AST_NODE_KIND_STR: {
            Asm_x86_64_EmitLeaRip(ASM_X86_64_REG_RAX, ".Lstr%d", node->an_str_idx);
        } break;
        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_DEREF:
        case AST_NODE_KIND_COMPOUND: {
            Gen_x86_64_EmitAddr(node);
            Gen_x86_64_EmitLoad(node->an_type);
        } break;
        case AST_NODE_KIND_MEMBER: {
            const Ast_Member *bits = Gen_x86_64_Bitfield(node);
            Gen_x86_64_EmitAddr(node);
            if (bits) {
                Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
                Gen_x86_64_EmitBitfieldLoad(bits);
            } else {
                Gen_x86_64_EmitLoad(node->an_type);
            }
        } break;
        case AST_NODE_KIND_ADDR: {
            Gen_x86_64_EmitAddr(node->an_lhs);
        } break;
        case AST_NODE_KIND_CAST: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitConvert(node->an_lhs->an_type, node->an_type, node->an_tmp);
        } break;
        case AST_NODE_KIND_ASSIGN: {
            const Ast_Member *bits = Gen_x86_64_Bitfield(node->an_lhs);
            Gen_x86_64_EmitAddr(node->an_lhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);
            if (bits) {
                Gen_x86_64_EmitBitfieldStore(bits);
            } else if (Gen_x86_64_ByAddress(node->an_type)) {
                Gen_x86_64_EmitCopy(node->an_type->at_size);
            } else {
                Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, Gen_x86_64_TypeWidth(node->an_type));
            }
        } break;
        case AST_NODE_KIND_OPASSIGN: {
            const Ast_Member *bits = Gen_x86_64_Bitfield(node->an_lhs);
            Asm_x86_64_Width width = Gen_x86_64_TypeWidth(node->an_type);
            Gen_x86_64_EmitAddr(node->an_lhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);
            if (bits) {
                Gen_x86_64_EmitPush();
                Gen_x86_64_EmitBitfieldLoad(bits);
                Gen_x86_64_EmitPop(ASM_X86_64_REG_RCX);
                Gen_x86_64_EmitOpAssign(node->an_op, node->an_type, node->an_line);
                Gen_x86_64_EmitBitfieldStore(bits);
            } else {
                Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
                Gen_x86_64_EmitLoadFrom(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, node->an_type);
                Gen_x86_64_EmitOpAssign(node->an_op, node->an_type, node->an_line);
                Gen_x86_64_EmitNarrow(node->an_type);
                Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, width);
            }
        } break;
        case AST_NODE_KIND_POSTINC: {
            const Ast_Member *bits = Gen_x86_64_Bitfield(node->an_lhs);
            Asm_x86_64_Width width = Gen_x86_64_TypeWidth(node->an_type);
            Gen_x86_64_EmitAddr(node->an_lhs);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
            if (bits) {
                Gen_x86_64_EmitBitfieldLoad(bits);
                Gen_x86_64_EmitPush();
                Asm_x86_64_EmitAddImm(node->an_val, ASM_X86_64_REG_RAX);
                Gen_x86_64_EmitBitfieldStore(bits);
                Gen_x86_64_EmitPop(ASM_X86_64_REG_RAX);
            } else {
                Gen_x86_64_EmitLoadFrom(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, node->an_type);
                Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
                Asm_x86_64_EmitAddImm(node->an_val, ASM_X86_64_REG_RCX);
                Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, 0, width);
            }
        } break;
        case AST_NODE_KIND_COND: {
            int32_t count = Gen_x86_64_Count();
            Gen_x86_64_EmitExpr(node->an_cond);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJe(".L.else.%d", count);
            Gen_x86_64_EmitExpr(node->an_then);
            Asm_x86_64_EmitJmp(".L.endif.%d", count);
            Asm_x86_64_EmitLabel(".L.else.%d", count);
            Gen_x86_64_EmitExpr(node->an_els);
            Asm_x86_64_EmitLabel(".L.endif.%d", count);
        } break;
        case AST_NODE_KIND_COMMA: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitExpr(node->an_rhs);
        } break;
        case AST_NODE_KIND_NEG: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            if (Ast_IsFloating(node->an_type)) {
                Gen_x86_64_EmitFloatNeg(node);
                break;
            }
            Asm_x86_64_EmitNeg(ASM_X86_64_REG_RAX);
            Gen_x86_64_EmitNarrow(node->an_type);
        } break;
        case AST_NODE_KIND_BITNOT: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitNot(ASM_X86_64_REG_RAX);
            Gen_x86_64_EmitNarrow(node->an_type);
        } break;
        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR: {
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RCX);
            if (node->an_kind == AST_NODE_KIND_SHL) {
                Asm_x86_64_EmitShl(ASM_X86_64_REG_RAX);
                Gen_x86_64_EmitNarrow(node->an_type);
            } else {
                Gen_x86_64_EmitShift(node->an_type->at_sign, ASM_X86_64_REG_RAX);
            }
        } break;
        case AST_NODE_KIND_NOT: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSete(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
        } break;
        case AST_NODE_KIND_AND: {
            int32_t count = Gen_x86_64_Count();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJe(".L.false.%d", count);
            Gen_x86_64_EmitExpr(node->an_rhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJe(".L.false.%d", count);
            Asm_x86_64_EmitMovImm(1, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJmp(".L.end.%d", count);
            Asm_x86_64_EmitLabel(".L.false.%d", count);
            Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitLabel(".L.end.%d", count);
        } break;
        case AST_NODE_KIND_OR: {
            int32_t count = Gen_x86_64_Count();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJne(".L.true.%d", count);
            Gen_x86_64_EmitExpr(node->an_rhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJne(".L.true.%d", count);
            Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJmp(".L.end.%d", count);
            Asm_x86_64_EmitLabel(".L.true.%d", count);
            Asm_x86_64_EmitMovImm(1, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitLabel(".L.end.%d", count);
        } break;
        case AST_NODE_KIND_VA_START: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_SysV_EmitVaStart();
        } break;
        case AST_NODE_KIND_VA_ARG: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_SysV_EmitVaArg(node->an_type);
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            Asm_x86_64_EmitLeaRip(ASM_X86_64_REG_RAX, "%s", node->an_funcname);
        } break;
        case AST_NODE_KIND_CALL: {
            Gen_x86_64_SysV_EmitCall(node);
        } break;
        default: {
            Ast_TypeSign sign = Gen_x86_64_Sign(node);

            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);
            if (Ast_IsFloating(node->an_lhs->an_type)) {
                Gen_x86_64_EmitFloatBinary(node);
                break;
            }

            switch (node->an_kind) {
                case AST_NODE_KIND_ADD: {
                    Asm_x86_64_EmitAdd(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Gen_x86_64_EmitNarrow(node->an_type);
                } break;
                case AST_NODE_KIND_SUB: {
                    Asm_x86_64_EmitSub(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Gen_x86_64_EmitNarrow(node->an_type);
                } break;
                case AST_NODE_KIND_MUL: {
                    Asm_x86_64_EmitImul(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Gen_x86_64_EmitNarrow(node->an_type);
                } break;
                case AST_NODE_KIND_DIV: {
                    Gen_x86_64_EmitDivide(sign, ASM_X86_64_REG_RDI);
                } break;
                case AST_NODE_KIND_BITAND: {
                    Asm_x86_64_EmitAnd(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_BITOR: {
                    Asm_x86_64_EmitOr(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_BITXOR: {
                    Asm_x86_64_EmitXor(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_MOD: {
                    Gen_x86_64_EmitDivide(sign, ASM_X86_64_REG_RDI);
                    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_EQ: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSete(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
                } break;
                case AST_NODE_KIND_NE: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSetne(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
                } break;
                case AST_NODE_KIND_LT: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    if (sign == AST_TYPE_UNSIGNED) {
                        Asm_x86_64_EmitSetb(ASM_X86_64_REG_RAX);
                    } else {
                        Asm_x86_64_EmitSetl(ASM_X86_64_REG_RAX);
                    }
                    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
                } break;
                case AST_NODE_KIND_LE: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    if (sign == AST_TYPE_UNSIGNED) {
                        Asm_x86_64_EmitSetbe(ASM_X86_64_REG_RAX);
                    } else {
                        Asm_x86_64_EmitSetle(ASM_X86_64_REG_RAX);
                    }
                    Asm_x86_64_EmitMovzx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
                } break;
                default: {
                    Err_RaiseAt(node->an_line, ERR_GEN_UNEXPECTED_EXPR, node->an_kind);
                }
            }
        }
    }
}

// Emit code for a statement.
void Gen_x86_64_EmitStmt(Ast_Node *node)
{
    switch (node->an_kind) {
        case AST_NODE_KIND_RETURN: {
            if (node->an_lhs) {
                Gen_x86_64_EmitExpr(node->an_lhs);
                Gen_x86_64_SysV_EmitReturnValue(node->an_lhs);
            } else {
                Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
            }
            Asm_x86_64_EmitJmp(".L.return.%s", Gen_x86_64_CurrFunc->af_name);
        } break;
        case AST_NODE_KIND_IF: {
            int32_t count = Gen_x86_64_Count();
            Gen_x86_64_EmitExpr(node->an_cond);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJe(".L.else.%d", count);
            Gen_x86_64_EmitStmt(node->an_then);
            Asm_x86_64_EmitJmp(".L.endif.%d", count);
            Asm_x86_64_EmitLabel(".L.else.%d", count);
            if (node->an_els) {
                Gen_x86_64_EmitStmt(node->an_els);
            }
            Asm_x86_64_EmitLabel(".L.endif.%d", count);
        } break;
        case AST_NODE_KIND_FOR: {
            int32_t count = Gen_x86_64_Count();
            int32_t brk = Gen_x86_64_BreakId;
            int32_t cnt = Gen_x86_64_ContinueId;
            Gen_x86_64_BreakId = Gen_x86_64_ContinueId = count;

            if (node->an_init) {
                Gen_x86_64_EmitStmt(node->an_init);
            }
            Asm_x86_64_EmitLabel(".L.begin.%d", count);
            if (node->an_cond) {
                Gen_x86_64_EmitExpr(node->an_cond);
                Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
                Asm_x86_64_EmitJe(".L.brk.%d", count);
            }
            Gen_x86_64_EmitStmt(node->an_body);
            Asm_x86_64_EmitLabel(".L.cnt.%d", count);
            if (node->an_inc) {
                Gen_x86_64_EmitExpr(node->an_inc);
            }
            Asm_x86_64_EmitJmp(".L.begin.%d", count);
            Asm_x86_64_EmitLabel(".L.brk.%d", count);

            Gen_x86_64_BreakId = brk;
            Gen_x86_64_ContinueId = cnt;
        } break;
        case AST_NODE_KIND_DO: {
            int32_t count = Gen_x86_64_Count();
            int32_t brk = Gen_x86_64_BreakId;
            int32_t cnt = Gen_x86_64_ContinueId;
            Gen_x86_64_BreakId = Gen_x86_64_ContinueId = count;

            Asm_x86_64_EmitLabel(".L.begin.%d", count);
            Gen_x86_64_EmitStmt(node->an_body);
            Asm_x86_64_EmitLabel(".L.cnt.%d", count);
            Gen_x86_64_EmitExpr(node->an_cond);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJne(".L.begin.%d", count);
            Asm_x86_64_EmitLabel(".L.brk.%d", count);

            Gen_x86_64_BreakId = brk;
            Gen_x86_64_ContinueId = cnt;
        } break;
        case AST_NODE_KIND_SWITCH: {
            int32_t count = Gen_x86_64_Count();
            int32_t brk = Gen_x86_64_BreakId;
            Gen_x86_64_BreakId = count;

            Gen_x86_64_EmitExpr(node->an_cond);
            Ast_Node *deflt = NULL;
            for (Ast_Node *c = node->an_cases; c; c = c->an_case_next) {
                c->an_label = Gen_x86_64_Count();
                if (c->an_kind == AST_NODE_KIND_DEFAULT) {
                    deflt = c;
                    continue;
                }
                Asm_x86_64_EmitCmpImm(c->an_val, ASM_X86_64_REG_RAX);
                Asm_x86_64_EmitJe(".L.case.%d", c->an_label);
            }
            if (deflt) {
                Asm_x86_64_EmitJmp(".L.case.%d", deflt->an_label);
            } else {
                Asm_x86_64_EmitJmp(".L.brk.%d", count);
            }

            Gen_x86_64_EmitStmt(node->an_body);
            Asm_x86_64_EmitLabel(".L.brk.%d", count);
            Gen_x86_64_BreakId = brk;
        } break;
        case AST_NODE_KIND_CASE:
        case AST_NODE_KIND_DEFAULT: {
            Asm_x86_64_EmitLabel(".L.case.%d", node->an_label);
            Gen_x86_64_EmitStmt(node->an_lhs);
        } break;
        case AST_NODE_KIND_LABEL: {
            Asm_x86_64_EmitLabel(".L.user.%s.%s", Gen_x86_64_CurrFunc->af_name, node->an_funcname);
            Gen_x86_64_EmitStmt(node->an_lhs);
        } break;
        case AST_NODE_KIND_GOTO: {
            Asm_x86_64_EmitJmp(".L.user.%s.%s", Gen_x86_64_CurrFunc->af_name, node->an_funcname);
        } break;
        case AST_NODE_KIND_BREAK: {
            Err_AssertAt(node->an_line, Gen_x86_64_BreakId >= 0, ERR_GEN_BREAK_OUTSIDE_LOOP);
            Asm_x86_64_EmitJmp(".L.brk.%d", Gen_x86_64_BreakId);
        } break;
        case AST_NODE_KIND_CONTINUE: {
            Err_AssertAt(node->an_line, Gen_x86_64_ContinueId >= 0, ERR_GEN_CONTINUE_OUTSIDE_LOOP);
            Asm_x86_64_EmitJmp(".L.cnt.%d", Gen_x86_64_ContinueId);
        } break;
        case AST_NODE_KIND_BLOCK: {
            for (Ast_Node *stmt = node->an_body; stmt; stmt = stmt->an_next) {
                Gen_x86_64_EmitStmt(stmt);
            }
        } break;
        case AST_NODE_KIND_EXPR_STMT: {
            Gen_x86_64_EmitExpr(node->an_lhs);
        } break;
        case AST_NODE_KIND_ZERO: {
            Gen_x86_64_EmitAddr(node->an_lhs);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
            Gen_x86_64_EmitZero((int32_t) node->an_val);
        } break;
        case AST_NODE_KIND_NOP: {
            // empty
        } break;
        default: {
            Err_RaiseAt(node->an_line, ERR_GEN_UNEXPECTED_STMT, node->an_kind);
        }
    }
}

// Return whether a node makes a new value it must hand over by address.
bool Gen_x86_64_NeedsTemp(const Ast_Node *node)
{
    if (! node->an_type || ! Gen_x86_64_ByAddress(node->an_type)) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_CALL: {
            return true;
        } break;
        case AST_NODE_KIND_FNUM:
        case AST_NODE_KIND_CAST:
        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB:
        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_NEG: {
            return node->an_type->at_kind == AST_TYPE_KIND_LDOUBLE;
        } break;
        default: {
            return false;
        }
    }
}

// Give every node that makes a value held by address a frame slot for it.
void Gen_x86_64_AssignTemps(Ast_Node *node, int32_t *offset)
{
    if (! node) {
        return;
    }
    if (Gen_x86_64_NeedsTemp(node)) {
        *offset = Gen_x86_64_AlignTo(*offset + Gen_x86_64_SlotSize(node->an_type), node->an_type->at_align);
        node->an_tmp = -*offset;
    }
    if (node->an_kind == AST_NODE_KIND_CALL && node->an_lhs) {
        *offset = Gen_x86_64_AlignTo(*offset + GEN_X86_64_WORD_SIZE, GEN_X86_64_WORD_SIZE);
        node->an_calltmp = -*offset;
    }

    Gen_x86_64_AssignTemps(node->an_lhs, offset);
    Gen_x86_64_AssignTemps(node->an_rhs, offset);
    Gen_x86_64_AssignTemps(node->an_cond, offset);
    Gen_x86_64_AssignTemps(node->an_then, offset);
    Gen_x86_64_AssignTemps(node->an_els, offset);
    Gen_x86_64_AssignTemps(node->an_init, offset);
    Gen_x86_64_AssignTemps(node->an_inc, offset);
    Gen_x86_64_AssignTemps(node->an_body, offset);
    Gen_x86_64_AssignTemps(node->an_args, offset);
    Gen_x86_64_AssignTemps(node->an_next, offset);
}

// Assign each local a stack slot and record the frame size.
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func)
{
    int32_t offset = func->af_variadic == AST_TYPE_VARIADIC ? GEN_X86_64_SYSV_VA_SAVE_SIZE : 0;

    if (Gen_x86_64_SysV_ReturnsInMemory(func->af_ret)) {
        offset += GEN_X86_64_WORD_SIZE;
        Gen_x86_64_SysV_RetPtrOffset = -offset;
    } else {
        Gen_x86_64_SysV_RetPtrOffset = 0;
    }

    for (Ast_Var *var = func->af_locals; var; var = var->av_next) {
        offset += Gen_x86_64_SlotSize(var->av_type);
        offset = Gen_x86_64_AlignTo(offset, var->av_type->at_align);
        var->av_offset = -offset;
    }
    Gen_x86_64_AssignTemps(func->af_body, &offset);
    func->af_stack_size = Gen_x86_64_AlignTo(offset, GEN_X86_64_SYSV_STACK_ALIGN);
}

// Emit the .rodata section holding all string literals.
void Gen_x86_64_EmitDataSection(void)
{
    size_t count = Ast_StringCount();
    if (count == 0) {
        return;
    }
    Asm_x86_64_EmitSection(".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    for (size_t i = 0; i < count; i++) {
        Ast_Str *str = Ast_StringAt(i);
        Asm_x86_64_EmitLabel(".Lstr%zu", i);
        Asm_x86_64_EmitBytes(str->as_data, str->as_len + str->as_width);
    }
}

// Write one floating initializer into the bytes it fills.
void Gen_x86_64_EmitFloatConstant(uint8_t *bytes, const Ast_Node *item, const Ast_Var *var)
{
    uint32_t single = 0;
    uint64_t dbl = 0;
    long double value = 0;

    Err_AssertAt(var->av_line, Sem_FoldFloat(item->an_lhs, &value), ERR_GEN_INIT_NOT_CONSTANT, var->av_name);
    switch (item->an_type->at_kind) {
        case AST_TYPE_KIND_FLOAT: {
            single = Fp_FloatBits((float) value);
            for (int32_t i = 0; i < AST_TYPE_SIZE_FLOAT; i++) {
                bytes[i] = (uint8_t) (single >> (i * FP_BITS_PER_BYTE));
            }
        } break;
        case AST_TYPE_KIND_DOUBLE: {
            dbl = Fp_DoubleBits((double) value);
            for (int32_t i = 0; i < AST_TYPE_SIZE_DOUBLE; i++) {
                bytes[i] = (uint8_t) (dbl >> (i * FP_BITS_PER_BYTE));
            }
        } break;
        default: {
            Fp_EncodeExtended(value, bytes);
        }
    }
}

// Write one flattened initializer into a global's image.
void Gen_x86_64_EmitConstant(uint8_t *bytes, const Ast_Node *item, const Ast_Var *var, Gen_x86_64_Addr *addrs, int32_t *naddrs)
{
    int32_t size = item->an_type->at_size;
    int32_t offset = (int32_t) item->an_val;
    int64_t val = 0;
    const char *symbol = NULL;

    Err_AssertAt(var->av_line, offset + size <= var->av_type->at_size, ERR_GEN_INIT_TOO_LARGE, var->av_name);
    if (Ast_IsFloating(item->an_type)) {
        Gen_x86_64_EmitFloatConstant(bytes + offset, item, var);
        return;
    }
    if (Sem_FoldAddr(item->an_lhs, &symbol)) {
        Err_AssertAt(var->av_line, size == GEN_X86_64_WORD_SIZE, ERR_GEN_INIT_ADDRESS_WIDTH, var->av_name);
        Err_AssertAt(var->av_line, *naddrs < GEN_X86_64_MAX_ADDRS, ERR_GEN_INIT_TOO_MANY_ADDRESSES, var->av_name, GEN_X86_64_MAX_ADDRS);
        addrs[(*naddrs)++] = (Gen_x86_64_Addr) { offset, symbol };
        return;
    }
    Err_AssertAt(var->av_line, Sem_Fold(item->an_lhs, &val), ERR_GEN_INIT_NOT_CONSTANT, var->av_name);

    if (item->an_member) {
        int64_t mask = (((int64_t) 1 << item->an_member->am_bits) - 1) << item->an_member->am_bitoff;
        val = (val << item->an_member->am_bitoff) & mask;
    }
    for (int32_t i = 0; i < size; i++) {
        uint8_t byte = (val >> (i * ASM_X86_64_BITS_PER_BYTE)) & GEN_X86_64_BYTE_MASK;
        if (item->an_member) {
            bytes[offset + i] |= byte;
        } else {
            bytes[offset + i] = byte;
        }
    }
}

// Emit an image as runs of bytes broken by the addresses the linker fills in.
void Gen_x86_64_EmitImage(const uint8_t *bytes, int32_t size, const Gen_x86_64_Addr *addrs, int32_t naddrs)
{
    int32_t at = 0;

    for (int32_t i = 0; i < naddrs; i++) {
        if (addrs[i].ga_offset > at) {
            Asm_x86_64_EmitBytes(bytes + at, addrs[i].ga_offset - at);
        }
        Asm_x86_64_EmitAddress(addrs[i].ga_symbol);
        at = addrs[i].ga_offset + GEN_X86_64_WORD_SIZE;
    }
    if (size > at) {
        Asm_x86_64_EmitBytes(bytes + at, size - at);
    }
}

// Emit one global into .data.
void Gen_x86_64_EmitGlobal(Ast_Var *var)
{
    int32_t size = var->av_type->at_size;
    int32_t naddrs = 0;
    Gen_x86_64_Addr addrs[GEN_X86_64_MAX_ADDRS];

    if (var->av_storage == AST_STORAGE_EXTERN) {
        return;
    }

    uint8_t *bytes = calloc(size ? size : 1, 1);
    for (Ast_Node *item = var->av_init; item; item = item->an_next) {
        Gen_x86_64_EmitConstant(bytes, item, var, addrs, &naddrs);
    }

    if (var->av_init) {
        Asm_x86_64_EmitSection(".data", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    } else {
        Asm_x86_64_EmitSection(".bss", ELF_SHT_NOBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    }
    if (var->av_storage != AST_STORAGE_STATIC) {
        Asm_x86_64_EmitGlobl("%s", var->av_symbol);
    }
    Asm_x86_64_EmitLabel("%s", var->av_symbol);
    Gen_x86_64_EmitImage(bytes, size, addrs, naddrs);
    free(bytes);
}

// Emit every file-scope variable.
void Gen_x86_64_EmitGlobals(void)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        Gen_x86_64_EmitGlobal(var);
    }
}

// Emit the prologue, body and epilogue for every function.
void Gen_x86_64_EmitFunctions(Ast_Func *prog)
{
    for (Ast_Func *func = prog; func; func = func->af_next) {
        if (! func->af_body) {
            continue;  // a prototype emits nothing
        }
        Gen_x86_64_AssignLvarOffsets(func);
        Gen_x86_64_CurrFunc = func;
        Gen_x86_64_BreakId = Gen_x86_64_ContinueId = -1;

        if (! func->af_static) {
            Asm_x86_64_EmitGlobl(func->af_name);
        }
        Asm_x86_64_EmitLabel(func->af_name);

        // prologue
        Asm_x86_64_EmitPush(ASM_X86_64_REG_RBP);
        Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RSP, ASM_X86_64_REG_RBP);
        if (func->af_stack_size) {
            Asm_x86_64_EmitSubImm(func->af_stack_size, ASM_X86_64_REG_RSP);
        }

        if (func->af_variadic == AST_TYPE_VARIADIC) {
            Gen_x86_64_SysV_EmitVaSaveArea();
        }

        int32_t reg = 0;
        int32_t stack = 0;
        if (Gen_x86_64_SysV_ReturnsInMemory(func->af_ret)) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_SysV_ArgReg[reg++], ASM_X86_64_REG_RBP, Gen_x86_64_SysV_RetPtrOffset, ASM_X86_64_WIDTH_64);
        }

        // spill incoming parameters
        for (Ast_Var *param = func->af_params; param; param = param->av_param_next) {
            Gen_x86_64_SysV_EmitParam(param, &reg, &stack);
        }

        Gen_x86_64_EmitStmt(func->af_body);

        // epilogue
        Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
        Asm_x86_64_EmitLabel(".L.return.%s", func->af_name);
        if (Gen_x86_64_SysV_ReturnsInMemory(func->af_ret)) {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, Gen_x86_64_SysV_RetPtrOffset, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
        }
        Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RBP, ASM_X86_64_REG_RSP);
        Asm_x86_64_EmitPop(ASM_X86_64_REG_RBP);
        Asm_x86_64_EmitRet();
    }
}

// Emit the .text section.
void Gen_x86_64_EmitTextSection(Ast_Func *prog)
{
    Asm_x86_64_EmitSection(".text", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_EXECINSTR);
    Gen_x86_64_EmitFunctions(prog);
}

// Build the instruction list for the whole program.
void Gen_x86_64_BuildProgram(Ast_Func *prog)
{
    Asm_x86_64_Reset();
    Gen_x86_64_Depth = 0;
    Gen_x86_64_LabelId = 0;

    Asm_x86_64_EmitDirective(".file \"cc\"");
    Gen_x86_64_EmitDataSection();
    Gen_x86_64_EmitGlobals();
    Gen_x86_64_EmitTextSection(prog);
    Asm_x86_64_EmitDirective(".section .note.GNU-stack,\"\",@progbits");
}
