/*
 * C header file for semantic analysis.
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

#ifndef SEM_H
#define SEM_H

// Project headers.
#include "util/console/err.h"
#include "util/str.h"
#include "lang/ast.h"

// The least value that no longer fits a signed 64-bit integer.
// [C99:6.3.1.4p1]: a floating value converted to an integer, truncated toward zero
#define SEM_TWO_TO_63 9223372036854775808.0L

// Node lists
int32_t Sem_CountNodes(Ast_Node *list);

// Type and expression queries
// [C99:6.2.5p21]:  scalar and aggregate types
// [C99:6.2.7p1]:   compatible types
// [C99:6.3.2.1p1]: lvalues, and the modifiable ones
// [C99:6.3.2.1p3]: an array converted to a pointer to its first element
// [C99:6.3.2.1p4]: a function converted to a pointer to it
bool        Sem_IsPointer(const Ast_Type *type);
bool        Sem_IsLvalue(const Ast_Node *node);
bool        Sem_HoldsConst(const Ast_Type *type);
bool        Sem_IsModifiable(const Ast_Type *type);
bool        Sem_IsAggregate(const Ast_Type *type);
const char *Sem_TypeName(const Ast_Type *type);
Ast_Type   *Sem_Decay(Ast_Type *type);
bool        Sem_SameType(const Ast_Type *a, const Ast_Type *b);

// Conversions
// [C99:6.3.1.1p2]:  the integer promotions
// [C99:6.3.1.2p1]:  a scalar converted to _Bool, 0 or 1
// [C99:6.3.1.8p1]:  the usual arithmetic conversions
// [C99:6.3.2.3p3]:  null pointer constants
// [C99:6.5.2.2p6]:  the default argument promotions
// [C99:6.5.7p3]:    a shift's type, its promoted left operand's
// [C99:6.5.15p3]:   the operand pairs a conditional takes
// [C99:6.5.15p5]:   a conditional's arithmetic, struct, union or void result
// [C99:6.5.15p6]:   a conditional's pointer result, qualified as both
// [C99:6.5.16.1p1]: the operand pairs simple assignment takes
Ast_Type *Sem_Promote(Ast_Type *type);
Ast_Type *Sem_PromoteArg(Ast_Type *type);
Ast_Type *Sem_CommonType(Ast_Type *lhs, Ast_Type *rhs);
Ast_Node *Sem_Truth(Ast_Node *node);
Ast_Type *Sem_ValueType(Ast_Type *type);
bool      Sem_IsNullPointer(const Ast_Node *node);
void      Sem_CheckAssign(const Ast_Type *to, const Ast_Node *from, const char *what, Ast_Line line);
Ast_Type *Sem_CondType(const Ast_Node *node);
Ast_Node *Sem_Convert(Ast_Node *node, Ast_Type *type);
void      Sem_UsualArith(Ast_Node *node);
void      Sem_PromoteShift(Ast_Node *node);

// Constant expressions
// [C99:6.5.5p5]: / and %, by zero undefined
// [C99:6.6p4]:   a constant within its type's range
// [C99:6.6p5]:   translation-time floating at least as precise as run time
// [C99:6.6p6]:   integer constant expressions
// [C99:6.6p7]:   an initializer's constants
// [C99:6.6p9]:   address constants
int64_t     Sem_Truncate(const Ast_Type *type, int64_t value);
Ast_Type   *Sem_StringType(const Ast_Node *node);
Ast_Type   *Sem_FoldValueType(const Ast_Node *node);
Ast_Type   *Sem_FoldType(const Ast_Node *node);
bool        Sem_IsFoldTyped(const Ast_Node *node);
Ast_Type   *Sem_FoldOperandType(const Ast_Node *node);
bool        Sem_FoldOp(Ast_NodeKind kind, int64_t lhs, int64_t rhs, Ast_TypeSign sign, Ast_Line line, int64_t *value);
bool        Sem_FoldFromFloat(const Ast_Node *node, int64_t *value);
bool        Sem_Fold(const Ast_Node *node, int64_t *value);
long double Sem_RoundFloat(const Ast_Type *type, long double value);
double      Sem_FoldDoubleOp(Ast_NodeKind kind, double lhs, double rhs);
long double Sem_FoldFloatOp(Ast_NodeKind kind, const Ast_Type *type, long double lhs, long double rhs);
bool        Sem_FoldFloat(const Ast_Node *node, long double *value);
bool        Sem_FoldObject(const Ast_Node *node, const char **symbol, int64_t *addend);
bool        Sem_FoldAddr(const Ast_Node *node, const char **symbol, int64_t *addend);

// Checks the parser cannot make
// [C99:6.5.2.2p1]: the callee, a pointer to a function
// [C99:6.5.2.2p2]: a prototype's arguments, as many as its parameters
// [C99:6.5.2.2p7]: arguments converted as if assigned, extra ones promoted
Ast_Type *Sem_CallType(Ast_Node *node);
Ast_Type *Sem_CalleeType(Ast_Node *node);
void      Sem_CheckArity(Ast_Node *node, int32_t want, Ast_TypeVa va, Ast_TypeProto proto, const char *what);
void      Sem_ConvertArgs(Ast_Node *node, Ast_Var *params, int32_t nparams, Ast_TypeVa va, Ast_TypeProto proto);
void      Sem_CheckCall(Ast_Node *node);

// Floating assignments rewritten into plain ones
// [C99:6.5.2.4p2]:  postfix ++, the value before
// [C99:6.5.16.2p3]: E1 op= E2 as E1 = E1 op (E2), E1 evaluated once
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
// [C99:6.5.2.3p1]:  the . operator's operands
// [C99:6.5.2.3p2]:  the -> operator's operands
// [C99:6.5.2.4p1]:  postfix ++ and --, on a modifiable real or pointer lvalue
// [C99:6.5.3.1p1]:  prefix ++ and --, on a modifiable real or pointer lvalue
// [C99:6.5.3.2p1]:  unary &, of a function, an object or a [] or * result
// [C99:6.5.3.2p2]:  unary *, of a pointer
// [C99:6.5.3.3p1]:  + and - of arithmetic, ~ of integer, ! of scalar
// [C99:6.5.4p2]:    a cast, scalar to scalar or anything to void
// [C99:6.5.5p2]:    * and / of arithmetic, % of integer
// [C99:6.5.6p2]:    + of arithmetic, or of a pointer and an integer
// [C99:6.5.6p3]:    - of arithmetic, of two pointers, or of a pointer and an integer
// [C99:6.5.6p8]:    a pointer plus an integer, counted in elements
// [C99:6.5.6p9]:    a pointer minus a pointer, counted in elements
// [C99:6.5.16p2]:   assignment, to a modifiable lvalue
// [C99:6.5.16.2p1]: += and -=, of a pointer and an integer or of arithmetic
// [C99:6.8.4.2p2]:  a switch, not into a variably modified type's scope
// [C99:6.8.4.2p3]:  case values, integer constants, all distinct
// [C99:6.8.6.1p1]:  goto, to a label in the function, not into such a scope
// [C99:6.8.6.4p1]:  return, with a value only from a function that has one
// [C99:7.15.1.4p4]: va_start's parmN, the parameter before the `, ...`
Ast_Node *Sem_Stride(const Ast_Type *type, Ast_Line line);
Ast_Node *Sem_ScaleBy(Ast_Node *node, Ast_Node *size);
void      Sem_Arith(Ast_Node *node);
void      Sem_NeedInteger(const Ast_Node *node);
void      Sem_NeedArithmetic(const Ast_Node *node);
bool      Sem_IsScalar(const Ast_Type *type);
void      Sem_NeedScalar(const Ast_Node *operand, Ast_Line line);
void      Sem_CheckOpAssign(const Ast_Node *node);
void      Sem_CheckCast(Ast_Node *node);
Ast_Node *Sem_FindLabel(Ast_Node *node, const char *name);
void      Sem_CheckGotos(Ast_Node *node, Ast_Node *body);
void      Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail);
void      Sem_Annotate(Ast_Node *node);
void      Sem_Node(Ast_Node *node);
void      Sem_AnalyzeGlobals(void);
void      Sem_Analyze(Ast_Func *prog);

#endif // SEM_H
