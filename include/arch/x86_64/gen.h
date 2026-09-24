// C header file for x86-64 code generation.

#ifndef GEN_X86_64_H
#define GEN_X86_64_H

#include "syntax/ast.h"
#include "arch/x86_64/asm.h"

// An address a global's image holds.
typedef struct Gen_x86_64_Addr Gen_x86_64_Addr;
struct Gen_x86_64_Addr {
    int         ga_offset;  // bytes into the image the address occupies
    const char *ga_symbol;  // symbol the address is taken from
};

// Code emission helpers
int              Gen_x86_64_Count(void);
void             Gen_x86_64_EmitPush(void);
void             Gen_x86_64_EmitPop(Asm_x86_64_Reg reg);
int              Gen_x86_64_AlignTo(int n, int align);
int              Gen_x86_64_SlotSize(const Ast_Type *type);
Asm_x86_64_Width Gen_x86_64_TypeWidth(const Ast_Type *type);
void             Gen_x86_64_EmitAddr(Ast_Node *node);
Ast_TypeSign     Gen_x86_64_Sign(const Ast_Node *node);
void             Gen_x86_64_EmitLoadFrom(Asm_x86_64_Reg base, int disp, Asm_x86_64_Reg dst, const Ast_Type *type);
void             Gen_x86_64_EmitLoad(const Ast_Type *type);
void             Gen_x86_64_EmitCast(const Ast_Type *type);
void             Gen_x86_64_EmitCopy(int size);
void             Gen_x86_64_EmitZero(int size);

// Bitfields
const Ast_Member *Gen_x86_64_Bitfield(const Ast_Node *node);
void              Gen_x86_64_EmitBitfieldLoad(const Ast_Member *member);
void              Gen_x86_64_EmitBitfieldStore(const Ast_Member *member);

// Variadic arguments
void Gen_x86_64_EmitVaSaveArea(void);
void Gen_x86_64_CountNamedArgs(const Ast_Func *func, int *reg, int *stack);
void Gen_x86_64_EmitVaStart(void);
void Gen_x86_64_EmitVaArg(const Ast_Type *type);

// The SysV call
void Gen_x86_64_EmitReturnValue(Ast_Node *node);
void Gen_x86_64_EmitParam(Ast_Var *param, int *reg, int *stack);
int  Gen_x86_64_ArgRegBase(Ast_Node *args, int index, int nHidden);
int  Gen_x86_64_CallStackSlots(Ast_Node *args, int nHidden);
void Gen_x86_64_PushArg(Ast_Node *arg);
void Gen_x86_64_CallPushStack(Ast_Node *args, Ast_Node *arg, int index, int nHidden);
void Gen_x86_64_CallPushReg(Ast_Node *args, Ast_Node *arg, int index, int nHidden);
void Gen_x86_64_CallPopReg(Ast_Node *args, int nHidden);
void Gen_x86_64_EmitCall(Ast_Node *node);

// Expressions, statements and data
void Gen_x86_64_EmitNarrow(const Ast_Type *type);
void Gen_x86_64_EmitDivide(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitShift(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, const Ast_Type *type, int line);
void Gen_x86_64_EmitExpr(Ast_Node *node);
void Gen_x86_64_EmitStmt(Ast_Node *node);
void Gen_x86_64_AssignCallTemps(Ast_Node *node, int *offset);
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func);
void Gen_x86_64_EmitDataSection(void);
void Gen_x86_64_EmitConstant(unsigned char *bytes, const Ast_Node *item, const Ast_Var *var, Gen_x86_64_Addr *addrs, int *naddrs);
void Gen_x86_64_EmitImage(const unsigned char *bytes, int size, const Gen_x86_64_Addr *addrs, int naddrs);
void Gen_x86_64_EmitGlobal(Ast_Var *var);
void Gen_x86_64_EmitGlobals(void);

// Text section
void Gen_x86_64_EmitFunctions(Ast_Func *prog);
void Gen_x86_64_EmitTextSection(Ast_Func *prog);

// Top-level code generation
void Gen_x86_64_BuildProgram(Ast_Func *prog);

#endif // GEN_X86_64_H
