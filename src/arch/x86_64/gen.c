#include <string.h>

#include "util/log.h"
#include "ast/ast.h"
#include "ast/sem.h"
#include "obj/Elf/types.h"
#include "arch/x86_64/asm.h"
#include "arch/x86_64/gen.h"
#include "arch/x86_64/abi.h"

// Number of integer arguments the ABI passes in registers; the rest go on the stack.
#define MAX_REG_ARGS 6

// Size in bytes of a stack slot and a general-purpose register.
#define WORD_SIZE 8

// Required %rsp alignment, in bytes, at the point of a `call`.
#define STACK_ALIGN 16

// Largest global a single scalar initializer may fill.
#define GEN_X86_64_MAX_INIT 8

// The bits of a byte, for splitting an initializer into them.
#define GEN_X86_64_BYTE_MASK 0xFF

// Chunk sizes an aggregate copy moves, largest first.
#define GEN_X86_64_COPY_QUAD (ASM_X86_64_WIDTH_64 / ASM_X86_64_BITS_PER_BYTE)
#define GEN_X86_64_COPY_LONG (ASM_X86_64_WIDTH_32 / ASM_X86_64_BITS_PER_BYTE)

// Bytes a variadic function reserves at the top of its frame to spill the
// argument registers into.
#define VA_SAVE_SIZE (MAX_REG_ARGS * WORD_SIZE)

// Number of values currently pushed with Gen_x86_64_EmitPush().
static int Gen_x86_64_Depth;

// Source of unique label numbers.
static int Gen_x86_64_LabelId;

// Label numbers the innermost loop uses for break and continue, or -1 when
// there is no loop to leave.
static int Gen_x86_64_BreakId = -1;
static int Gen_x86_64_ContinueId = -1;

// The function currently being emitted.
static const Ast_Func *Gen_x86_64_CurrFunc;

// Frame offset holding the caller's buffer pointer for a memory return.
static int Gen_x86_64_RetPtrOffset;

// Registers used to pass the first six integer arguments, in ABI order.
static const Asm_x86_64_Reg Gen_x86_64_ArgReg[6] = {
    ASM_X86_64_REG_RDI,
    ASM_X86_64_REG_RSI,
    ASM_X86_64_REG_RDX,
    ASM_X86_64_REG_RCX,
    ASM_X86_64_REG_R8,
    ASM_X86_64_REG_R9
};

// Return the next unique label number.
int Gen_x86_64_Count(void)
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
int Gen_x86_64_AlignTo(int n, int align)
{
    return (n + align - 1) / align * align;
}

// Return the operand width in bits used to load or store a value of type.
Asm_x86_64_Width Gen_x86_64_TypeWidth(const Ast_Type *type)
{
    return type->at_size * ASM_X86_64_BITS_PER_BYTE;
}

// Compute the address of an lvalue into %rax.
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
        case AST_NODE_KIND_MEMBER: {
            Gen_x86_64_EmitAddr(node->an_lhs);
            if (node->an_member->am_offset) {
                Asm_x86_64_EmitAddImm(node->an_member->am_offset, ASM_X86_64_REG_RAX);
            }
        } break;
        case AST_NODE_KIND_CALL: {
            if (! Sem_IsAggregate(node->an_type)) {
                Log_ShowErrorAt(node->an_line, "codegen: not an lvalue");
            }
            Gen_x86_64_EmitCall(node);
        } break;
        case AST_NODE_KIND_COMPOUND: {
            for (Ast_Node *stmt = node->an_body; stmt; stmt = stmt->an_next) {
                Gen_x86_64_EmitStmt(stmt);
            }
            Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_var->av_offset, ASM_X86_64_REG_RAX);
        } break;
        default: {
            Log_ShowErrorAt(node->an_line, "codegen: not an lvalue");
        }
    }
}

// Load the value at the address in %rax. An array or aggregate is left as that
// address, because no register holds one and every use of it wants the address.
void Gen_x86_64_EmitLoad(const Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        return;
    }
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, 0, ASM_X86_64_REG_RAX, Gen_x86_64_TypeWidth(type));
}

// Narrow the value in %rax to type, sign-extending it back to 64 bits.
void Gen_x86_64_EmitCast(const Ast_Type *type)
{
    switch (type->at_kind) {
        case AST_TYPE_KIND_CHAR: {
            Asm_x86_64_EmitMovsx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_8);
        } break;
        case AST_TYPE_KIND_INT: {
            Asm_x86_64_EmitMovsx(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_32);
        } break;
        case AST_TYPE_KIND_VOID:
        case AST_TYPE_KIND_PTR:
        case AST_TYPE_KIND_ARRAY:
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_UNION: {
            // already as wide as a register, or addressed rather than held in one
        } break;
    }
}

// Write size zero bytes at the address in %rdi.
void Gen_x86_64_EmitZero(int size)
{
    int off = 0;

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

// Copy size bytes from the address in %rax to the address in %rdi, leaving the
// destination in %rax so that an assignment yields the object it assigned to.
void Gen_x86_64_EmitCopy(int size)
{
    int off = 0;

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

// Spill every argument register into the register save area, so that a variadic
// function can reach the arguments it has no parameter for.
void Gen_x86_64_EmitVaSaveArea(void)
{
    for (int i = 0; i < MAX_REG_ARGS; i++) {
        Asm_x86_64_EmitMovStore(Gen_x86_64_ArgReg[i], ASM_X86_64_REG_RBP, -VA_SAVE_SIZE + i * WORD_SIZE, ASM_X86_64_WIDTH_64);
    }
}

// Compute the address of the argument slot indexed by %rdi into %rax, counting
// from base off %rbp.
void Gen_x86_64_EmitVaSlotAddr(int base)
{
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitMovImm(WORD_SIZE, ASM_X86_64_REG_RDI);
    Asm_x86_64_EmitImul(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, base, ASM_X86_64_REG_RDI);
    Asm_x86_64_EmitAdd(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
}

// Turn the value of a return expression into what the ABI returns.
void Gen_x86_64_EmitReturnValue(Ast_Node *node)
{
    if (! Sem_IsAggregate(node->an_type)) {
        return;
    }
    if (Abi_x86_64_ReturnsInMemory(node->an_type)) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, Gen_x86_64_RetPtrOffset, ASM_X86_64_REG_RDI, ASM_X86_64_WIDTH_64);
        Gen_x86_64_EmitCopy(node->an_type->at_size);
        return;
    }

    // Small enough for registers: read the eightbytes out before %rax is reused.
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
    if (Abi_x86_64_Eightbytes(node->an_type) > 1) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RCX, WORD_SIZE, ASM_X86_64_REG_RDX, ASM_X86_64_WIDTH_64);
    }
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RCX, 0, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
}

// Spill one incoming parameter into its frame slot.
void Gen_x86_64_EmitParam(Ast_Var *param, int *reg, int *stack)
{
    int slots = Abi_x86_64_Eightbytes(param->av_type);
    int inReg = ! Abi_x86_64_InMemory(param->av_type) && *reg + slots <= MAX_REG_ARGS;

    if (! Sem_IsAggregate(param->av_type)) {
        Asm_x86_64_Width width = Gen_x86_64_TypeWidth(param->av_type);
        if (inReg) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_ArgReg[(*reg)++], ASM_X86_64_REG_RBP, param->av_offset, width);
        } else {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, 2 * WORD_SIZE + (*stack)++ * WORD_SIZE, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, param->av_offset, width);
        }
        return;
    }

    for (int k = 0; k < slots; k++) {
        if (inReg) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_ArgReg[(*reg)++], ASM_X86_64_REG_RBP, param->av_offset + k * WORD_SIZE, ASM_X86_64_WIDTH_64);
        } else {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, 2 * WORD_SIZE + (*stack)++ * WORD_SIZE, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, param->av_offset + k * WORD_SIZE, ASM_X86_64_WIDTH_64);
        }
    }
}

// Return the first argument register the argument at index takes, or -1 for the stack.
int Gen_x86_64_ArgRegBase(Ast_Node *args, int index, int nHidden)
{
    int used = nHidden;
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int want = Abi_x86_64_Eightbytes(arg->an_type);
        if (Abi_x86_64_InMemory(arg->an_type) || used + want > MAX_REG_ARGS) {
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
int Gen_x86_64_CallStackSlots(Ast_Node *args, int nHidden)
{
    int slots = 0;
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        if (Gen_x86_64_ArgRegBase(args, i, nHidden) < 0) {
            slots += Abi_x86_64_Eightbytes(arg->an_type);
        }
    }
    return slots;
}

// Evaluate one argument and push its eightbytes, lowest ending on top.
void Gen_x86_64_PushArg(Ast_Node *arg)
{
    Gen_x86_64_EmitExpr(arg);

    if (! Sem_IsAggregate(arg->an_type)) {
        Gen_x86_64_EmitPush();
        return;
    }
    for (int k = Abi_x86_64_Eightbytes(arg->an_type) - 1; k >= 0; k--) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, k * WORD_SIZE, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitPush(ASM_X86_64_REG_RCX);
        Gen_x86_64_Depth++;
    }
}

// Push the arguments the ABI places on the stack, last one first.
void Gen_x86_64_CallPushStack(Ast_Node *args, Ast_Node *arg, int index, int nHidden)
{
    if (! arg) {
        return;
    }
    Gen_x86_64_CallPushStack(args, arg->an_next, index + 1, nHidden);
    if (Gen_x86_64_ArgRegBase(args, index, nHidden) < 0) {
        Gen_x86_64_PushArg(arg);
    }
}

// Push the arguments the ABI passes in registers, last one first.
void Gen_x86_64_CallPushReg(Ast_Node *args, Ast_Node *arg, int index, int nHidden)
{
    if (! arg) {
        return;
    }
    Gen_x86_64_CallPushReg(args, arg->an_next, index + 1, nHidden);
    if (Gen_x86_64_ArgRegBase(args, index, nHidden) >= 0) {
        Gen_x86_64_PushArg(arg);
    }
}

// Pop the pushed register arguments into the registers the ABI assigns them.
void Gen_x86_64_CallPopReg(Ast_Node *args, int nHidden)
{
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int base = Gen_x86_64_ArgRegBase(args, i, nHidden);
        if (base < 0) {
            continue;
        }
        for (int k = 0; k < Abi_x86_64_Eightbytes(arg->an_type); k++) {
            Gen_x86_64_EmitPop(Gen_x86_64_ArgReg[base + k]);
        }
    }
}

// Emit a call, leaving its result in %rax, or an aggregate's address there.
void Gen_x86_64_EmitCall(Ast_Node *node)
{
    int nHidden = Abi_x86_64_ReturnsInMemory(node->an_type) ? 1 : 0;
    int nStack  = Gen_x86_64_CallStackSlots(node->an_args, nHidden);

    int nAlignPad = (Gen_x86_64_Depth + nStack) % (STACK_ALIGN / WORD_SIZE);
    if (nAlignPad) {
        Asm_x86_64_EmitSubImm(WORD_SIZE, ASM_X86_64_REG_RSP);
        Gen_x86_64_Depth++;
    }

    Gen_x86_64_CallPushStack(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_CallPushReg(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_CallPopReg(node->an_args, nHidden);

    // A memory return hands the callee the caller's buffer in %rdi.
    if (nHidden) {
        Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RDI);
    }

    Asm_x86_64_EmitMovImm8(0, ASM_X86_64_REG_RAX);
    Asm_x86_64_EmitCall(node->an_funcname);

    if (nStack + nAlignPad > 0) {
        Asm_x86_64_EmitAddImm(WORD_SIZE * (nStack + nAlignPad), ASM_X86_64_REG_RSP);
        Gen_x86_64_Depth -= nStack + nAlignPad;
    }

    // A register return arrives in %rax and %rdx, and has to become an address.
    if (Sem_IsAggregate(node->an_type) && ! nHidden) {
        int slots = Abi_x86_64_Eightbytes(node->an_type);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_WIDTH_64);
        if (slots > 1) {
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RBP, node->an_tmp + WORD_SIZE, ASM_X86_64_WIDTH_64);
        }
        Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RAX);
    }
}

// Apply a compound assignment's operation to the value in %rax and the one in
// %rcx, leaving the result in %rax. A shift finds its count in %cl already.
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, int line)
{
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
            Asm_x86_64_EmitCqo();
            Asm_x86_64_EmitIdiv(ASM_X86_64_REG_RCX);
        } break;
        case AST_NODE_KIND_MOD: {
            Asm_x86_64_EmitCqo();
            Asm_x86_64_EmitIdiv(ASM_X86_64_REG_RCX);
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
            Asm_x86_64_EmitSar(ASM_X86_64_REG_RAX);
        } break;
        default: {
            Log_ShowErrorAt(line, "codegen: unexpected compound assignment %d", op);
        }
    }
}

// Emit code for an expression, leaving its result in %rax.
void Gen_x86_64_EmitExpr(Ast_Node *node)
{
    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            Asm_x86_64_EmitMovImm(node->an_val, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_STR: {
            Asm_x86_64_EmitLeaRip(ASM_X86_64_REG_RAX, ".Lstr%d", node->an_str_idx);
        } break;
        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_DEREF:
        case AST_NODE_KIND_MEMBER:
        case AST_NODE_KIND_COMPOUND: {
            Gen_x86_64_EmitAddr(node);
            Gen_x86_64_EmitLoad(node->an_type);
        } break;
        case AST_NODE_KIND_ADDR: {
            Gen_x86_64_EmitAddr(node->an_lhs);
        } break;
        case AST_NODE_KIND_CAST: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitCast(node->an_type);
        } break;
        case AST_NODE_KIND_ASSIGN: {
            Gen_x86_64_EmitAddr(node->an_lhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);
            if (Sem_IsAggregate(node->an_type)) {
                Gen_x86_64_EmitCopy(node->an_type->at_size);
            } else {
                Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, Gen_x86_64_TypeWidth(node->an_type));
            }
        } break;
        case AST_NODE_KIND_OPASSIGN: {
            Asm_x86_64_Width width = Gen_x86_64_TypeWidth(node->an_type);
            Gen_x86_64_EmitAddr(node->an_lhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, width);
            Gen_x86_64_EmitOpAssign(node->an_op, node->an_line);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, width);
        } break;
        case AST_NODE_KIND_POSTINC: {
            Asm_x86_64_Width width = Gen_x86_64_TypeWidth(node->an_type);
            Gen_x86_64_EmitAddr(node->an_lhs);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RDI, 0, ASM_X86_64_REG_RAX, width);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitAddImm(node->an_val, ASM_X86_64_REG_RCX);
            Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, 0, width);
        } break;
        case AST_NODE_KIND_COND: {
            int count = Gen_x86_64_Count();
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
            Asm_x86_64_EmitNeg(ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_BITNOT: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitNot(ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR: {
            // The shift count has to reach %cl, so it is the operand popped.
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RCX);
            if (node->an_kind == AST_NODE_KIND_SHL) {
                Asm_x86_64_EmitShl(ASM_X86_64_REG_RAX);
            } else {
                Asm_x86_64_EmitSar(ASM_X86_64_REG_RAX);
            }
        } break;
        case AST_NODE_KIND_NOT: {
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSete(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
        } break;
        case AST_NODE_KIND_AND: {
            int count = Gen_x86_64_Count();
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
            int count = Gen_x86_64_Count();
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
        case AST_NODE_KIND_VA_ARG: {
            int count = Gen_x86_64_Count();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Asm_x86_64_EmitAddImm(Gen_x86_64_CurrFunc->af_nparams, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);

            Asm_x86_64_EmitCmpImm(MAX_REG_ARGS, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitSetl(ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitCmpImm(0, ASM_X86_64_REG_RAX);
            Asm_x86_64_EmitJe(".L.va.stack.%d", count);

            Gen_x86_64_EmitVaSlotAddr(-VA_SAVE_SIZE);
            Asm_x86_64_EmitJmp(".L.va.end.%d", count);

            // Past the sixth argument the caller passed it above the return
            // address instead, so the index restarts there.
            Asm_x86_64_EmitLabel(".L.va.stack.%d", count);
            Asm_x86_64_EmitSubImm(MAX_REG_ARGS, ASM_X86_64_REG_RDI);
            Gen_x86_64_EmitVaSlotAddr(2 * WORD_SIZE);

            Asm_x86_64_EmitLabel(".L.va.end.%d", count);
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, 0, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
        } break;
        case AST_NODE_KIND_CALL: {
            Gen_x86_64_EmitCall(node);
        } break;
        default: {
            Gen_x86_64_EmitExpr(node->an_rhs);
            Gen_x86_64_EmitPush();
            Gen_x86_64_EmitExpr(node->an_lhs);
            Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);

            switch (node->an_kind) {
                case AST_NODE_KIND_ADD: {
                    Asm_x86_64_EmitAdd(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_SUB: {
                    Asm_x86_64_EmitSub(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_MUL: {
                    Asm_x86_64_EmitImul(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_DIV: {
                    Asm_x86_64_EmitCqo();
                    Asm_x86_64_EmitIdiv(ASM_X86_64_REG_RDI);
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
                    Asm_x86_64_EmitCqo();
                    Asm_x86_64_EmitIdiv(ASM_X86_64_REG_RDI);
                    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDX, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_EQ: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSete(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_NE: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSetne(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_LT: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSetl(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
                } break;
                case AST_NODE_KIND_LE: {
                    Asm_x86_64_EmitCmp(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitSetle(ASM_X86_64_REG_RAX);
                    Asm_x86_64_EmitMovzb(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RAX);
                } break;
                default: {
                    Log_ShowErrorAt(node->an_line, "codegen: unexpected node kind %d", node->an_kind);
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
                Gen_x86_64_EmitReturnValue(node->an_lhs);
            } else {
                Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
            }
            Asm_x86_64_EmitJmp(".L.return.%s", Gen_x86_64_CurrFunc->af_name);
        } break;
        case AST_NODE_KIND_IF: {
            int count = Gen_x86_64_Count();
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
            int count = Gen_x86_64_Count();
            int brk = Gen_x86_64_BreakId;
            int cnt = Gen_x86_64_ContinueId;
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
            int count = Gen_x86_64_Count();
            int brk = Gen_x86_64_BreakId;
            int cnt = Gen_x86_64_ContinueId;
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
            int count = Gen_x86_64_Count();
            int brk = Gen_x86_64_BreakId;
            Gen_x86_64_BreakId = count;

            // Compare and branch once per case, then fall to default or out.
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
            if (Gen_x86_64_BreakId < 0) {
                Log_ShowErrorAt(node->an_line, "break outside a loop");
            }
            Asm_x86_64_EmitJmp(".L.brk.%d", Gen_x86_64_BreakId);
        } break;
        case AST_NODE_KIND_CONTINUE: {
            if (Gen_x86_64_ContinueId < 0) {
                Log_ShowErrorAt(node->an_line, "continue outside a loop");
            }
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
            Gen_x86_64_EmitZero((int) node->an_val);
        } break;
        case AST_NODE_KIND_NOP: {
            // nothing to emit
        } break;
        default: {
            Log_ShowErrorAt(node->an_line, "codegen: unexpected statement kind %d", node->an_kind);
        }
    }
}

// Give every call that returns an aggregate a frame slot to land the result in.
void Gen_x86_64_AssignCallTemps(Ast_Node *node, int *offset)
{
    if (! node) {
        return;
    }
    if (node->an_kind == AST_NODE_KIND_CALL && Sem_IsAggregate(node->an_type)) {
        *offset = Gen_x86_64_AlignTo(*offset + node->an_type->at_size, node->an_type->at_align);
        node->an_tmp = -*offset;
    }

    Gen_x86_64_AssignCallTemps(node->an_lhs, offset);
    Gen_x86_64_AssignCallTemps(node->an_rhs, offset);
    Gen_x86_64_AssignCallTemps(node->an_cond, offset);
    Gen_x86_64_AssignCallTemps(node->an_then, offset);
    Gen_x86_64_AssignCallTemps(node->an_els, offset);
    Gen_x86_64_AssignCallTemps(node->an_init, offset);
    Gen_x86_64_AssignCallTemps(node->an_inc, offset);
    Gen_x86_64_AssignCallTemps(node->an_body, offset);
    Gen_x86_64_AssignCallTemps(node->an_args, offset);
    Gen_x86_64_AssignCallTemps(node->an_next, offset);
}

// Assign each local a stack slot and record the frame size.
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func)
{
    int offset = func->af_variadic ? VA_SAVE_SIZE : 0;

    // A memory return keeps the caller's buffer pointer in the first slot.
    if (Abi_x86_64_ReturnsInMemory(func->af_ret)) {
        offset += WORD_SIZE;
        Gen_x86_64_RetPtrOffset = -offset;
    } else {
        Gen_x86_64_RetPtrOffset = 0;
    }

    for (Ast_Var *var = func->af_locals; var; var = var->av_next) {
        offset += var->av_type->at_size;
        offset = Gen_x86_64_AlignTo(offset, var->av_type->at_align);
        var->av_offset = -offset;
    }
    Gen_x86_64_AssignCallTemps(func->af_body, &offset);
    func->af_stack_size = Gen_x86_64_AlignTo(offset, STACK_ALIGN);
}

// Write a constant initializer's bytes into a global's image at offset.
void Gen_x86_64_EmitConstant(unsigned char *bytes, int size, int offset, const Ast_Node *value, const Ast_Var *var)
{
    if (value->an_kind != AST_NODE_KIND_NUM) {
        Log_ShowErrorAt(var->av_line, "initializer for '%s' is not a constant", var->av_name);
    }
    if (offset + size > var->av_type->at_size) {
        Log_ShowErrorAt(var->av_line, "initializer for '%s' is larger than it is", var->av_name);
    }
    for (int i = 0; i < size; i++) {
        bytes[offset + i] = (value->an_val >> (i * ASM_X86_64_BITS_PER_BYTE)) & GEN_X86_64_BYTE_MASK;
    }
}

// Emit one global: its bytes in .data when it has an initializer, or the space
// it asks for in .bss when it is zeroed.
void Gen_x86_64_EmitGlobal(Ast_Var *var)
{
    int size = var->av_type->at_size;

    // An extern declaration defines nothing; the symbol comes from elsewhere.
    if (var->av_storage == AST_STORAGE_EXTERN) {
        return;
    }

    unsigned char *bytes = calloc(size ? size : 1, 1);
    for (Ast_Node *item = var->av_init; item; item = item->an_next) {
        Gen_x86_64_EmitConstant(bytes, item->an_type->at_size, (int) item->an_val, item->an_lhs, var);
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
    Asm_x86_64_EmitBytes(bytes, size);
    free(bytes);
}

// Emit every file-scope variable, before the code that refers to them.
void Gen_x86_64_EmitGlobals(void)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        Gen_x86_64_EmitGlobal(var);
    }
}

// Emit the .rodata section holding all string literals.
void Gen_x86_64_EmitDataSection(void)
{
    int count = Ast_StringCount();
    if (count == 0) {
        return;
    }
    Asm_x86_64_EmitSection(".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    for (int i = 0; i < count; i++) {
        Ast_Str *str = Ast_StringAt(i);
        Asm_x86_64_EmitLabel(".Lstr%d", i);
        Asm_x86_64_EmitBytes(str->as_data, str->as_len + 1);
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

        if (func->af_variadic) {
            Gen_x86_64_EmitVaSaveArea();
        }

        // A memory return arrives as a hidden first argument in %rdi.
        int reg = 0;
        int stack = 0;
        if (Abi_x86_64_ReturnsInMemory(func->af_ret)) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_ArgReg[reg++], ASM_X86_64_REG_RBP, Gen_x86_64_RetPtrOffset, ASM_X86_64_WIDTH_64);
        }

        // spill incoming parameters
        for (Ast_Var *param = func->af_params; param; param = param->av_param_next) {
            Gen_x86_64_EmitParam(param, &reg, &stack);
        }

        Gen_x86_64_EmitStmt(func->af_body);

        // epilogue
        Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RAX);
        Asm_x86_64_EmitLabel(".L.return.%s", func->af_name);
        if (Abi_x86_64_ReturnsInMemory(func->af_ret)) {
            Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, Gen_x86_64_RetPtrOffset, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
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
