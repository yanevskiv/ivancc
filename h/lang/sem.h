/*
 * C header file for semantic analysis.
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

#ifndef SEM_H
#define SEM_H

// Standard headers.
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/str.h"
#include "lang/ast.h"

// The least value that no longer fits a signed 64-bit integer.
#define SEM_TWO_TO_63 9223372036854775808.0L

// Lookups over the program being analysed
Ast_Func *Sem_FindFunc(const char *name);
int32_t   Sem_CountNodes(Ast_Node *list);

// Type and expression queries
bool      Sem_IsPointer(const Ast_Type *type);
bool      Sem_IsLvalue(const Ast_Node *node);
bool      Sem_HoldsConst(const Ast_Type *type);
bool      Sem_IsModifiable(const Ast_Type *type);
bool      Sem_IsAggregate(const Ast_Type *type);
const char *Sem_TypeName(const Ast_Type *type);
Ast_Type *Sem_Decay(Ast_Type *type);
bool      Sem_SameType(const Ast_Type *a, const Ast_Type *b);

// Conversions
Ast_Type *Sem_Promote(Ast_Type *type);
Ast_Type *Sem_PromoteArg(Ast_Type *type);
Ast_Type *Sem_CommonType(Ast_Type *lhs, Ast_Type *rhs);
Ast_Node *Sem_Truth(Ast_Node *node);
Ast_Node *Sem_Convert(Ast_Node *node, Ast_Type *type);
void      Sem_UsualArith(Ast_Node *node);
void      Sem_PromoteShift(Ast_Node *node);

// Constant expressions
int64_t   Sem_Truncate(const Ast_Type *type, int64_t value);
Ast_Type *Sem_FoldType(const Ast_Node *node);
Ast_Type *Sem_FoldOperandType(const Ast_Node *node);
bool      Sem_FoldOp(Ast_NodeKind kind, int64_t lhs, int64_t rhs, Ast_TypeSign sign, Ast_Line line, int64_t *value);
bool      Sem_FoldFromFloat(const Ast_Node *node, int64_t *value);
bool      Sem_Fold(const Ast_Node *node, int64_t *value);
long double Sem_RoundFloat(const Ast_Type *type, long double value);
double    Sem_FoldDoubleOp(Ast_NodeKind kind, double lhs, double rhs);
long double Sem_FoldFloatOp(Ast_NodeKind kind, const Ast_Type *type, long double lhs, long double rhs);
bool      Sem_FoldFloat(const Ast_Node *node, long double *value);
bool      Sem_FoldObject(const Ast_Node *node, const char **symbol, int64_t *addend);
bool      Sem_FoldAddr(const Ast_Node *node, const char **symbol, int64_t *addend);

// Checks the parser cannot make
Ast_Type *Sem_FuncAddrType(Ast_Node *node);
Ast_Type *Sem_CallType(Ast_Node *node);
Ast_Type *Sem_CalleeType(Ast_Node *node);
void      Sem_CheckArity(Ast_Node *node, int32_t want, Ast_TypeVariadic variadic, Ast_TypeProto proto, const char *what);
void      Sem_ConvertArgs(Ast_Node *node, Ast_Var *params, int32_t nparams, Ast_TypeVariadic variadic, Ast_TypeProto proto);
void      Sem_CheckCall(Ast_Node *node);

// Floating assignments rewritten into plain ones
Ast_Var  *Sem_NewTemp(Ast_Type *type, Ast_Line line);
Ast_Node *Sem_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Line line);
Ast_Node *Sem_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line);
Ast_Node *Sem_TempRef(Ast_Var *var, Ast_Line line);
Ast_Node *Sem_PinLvalue(Ast_Node *lhs, Ast_Var **ptr);
Ast_Node *Sem_Target(Ast_Node *lhs, Ast_Var *ptr);
bool      Sem_NeedsFloatAssign(const Ast_Node *node);
void      Sem_Replace(Ast_Node *node, const Ast_Node *with);
void      Sem_LowerOpAssign(Ast_Node *node);
void      Sem_LowerPostInc(Ast_Node *node);

// Annotation
Ast_Node *Sem_Stride(const Ast_Type *type, Ast_Line line);
Ast_Node *Sem_ScaleBy(Ast_Node *node, Ast_Node *size);
void      Sem_Arith(Ast_Node *node);
void      Sem_NeedInteger(const Ast_Node *node);
void      Sem_CheckCast(Ast_Node *node);
Ast_Node *Sem_FindLabel(Ast_Node *node, const char *name);
void      Sem_CheckGotos(Ast_Node *node, Ast_Node *body);
void      Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail);
void      Sem_Annotate(Ast_Node *node);
void      Sem_Node(Ast_Node *node);
void      Sem_AnalyzeGlobals(void);
void      Sem_Analyze(Ast_Func *prog);

#endif // SEM_H
