#ifndef GEN_X86_64_H
#define GEN_X86_64_H

#include "ast/ast.h"
#include "arch/x86_64/asm.h"

// Code emission helpers
int              Gen_x86_64_Count(void);
void             Gen_x86_64_EmitPush(void);
void             Gen_x86_64_EmitPop(Asm_x86_64_Reg reg);
int              Gen_x86_64_AlignTo(int n, int align);
Asm_x86_64_Width Gen_x86_64_TypeWidth(const Ast_Type *type);
void             Gen_x86_64_EmitAddr(Ast_Node *node);
void             Gen_x86_64_EmitLoad(const Ast_Type *type);
void             Gen_x86_64_EmitCast(const Ast_Type *type);
void             Gen_x86_64_EmitCopy(int size);
void             Gen_x86_64_EmitZero(int size);

// Variadic arguments
void Gen_x86_64_EmitVaSaveArea(void);
void Gen_x86_64_EmitVaSlotAddr(int base);

// Function call arguments
int  Gen_x86_64_CallCountArgs(Ast_Node *args);
void Gen_x86_64_CallPushArgs(Ast_Node *arg);
void Gen_x86_64_CallPopArgs(int nReg);

// Expressions, statements and data
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, int line);
void Gen_x86_64_EmitExpr(Ast_Node *node);
void Gen_x86_64_EmitStmt(Ast_Node *node);
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func);
void Gen_x86_64_EmitDataSection(void);
void Gen_x86_64_EmitConstant(unsigned char *bytes, int size, int offset, const Ast_Node *value, const Ast_Var *var);
void Gen_x86_64_EmitGlobal(Ast_Var *var);
void Gen_x86_64_EmitGlobals(void);

// Text section
void Gen_x86_64_EmitFunctions(Ast_Func *prog);
void Gen_x86_64_EmitTextSection(Ast_Func *prog);

// Top-level code generation
void Gen_x86_64_BuildProgram(Ast_Func *prog);

#endif // GEN_X86_64_H
