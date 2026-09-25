// C header file for semantic analysis.

#ifndef SEM_H
#define SEM_H

// Standard headers.
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/str.h"
#include "lang/ast.h"

// Lookups over the program being analysed
Ast_Func *Sem_FindFunc(const char *name);
int32_t   Sem_CountNodes(Ast_Node *list);

// Type and expression queries
bool      Sem_IsPointer(const Ast_Type *type);
bool      Sem_IsLvalue(const Ast_Node *node);
bool      Sem_IsAggregate(const Ast_Type *type);
const char *Sem_TypeName(const Ast_Type *type);
Ast_Type *Sem_Decay(Ast_Type *type);
bool      Sem_SameType(const Ast_Type *a, const Ast_Type *b);

// Conversions
Ast_Type *Sem_Promote(Ast_Type *type);
Ast_Type *Sem_CommonType(Ast_Type *lhs, Ast_Type *rhs);
Ast_Node *Sem_Convert(Ast_Node *node, Ast_Type *type);
void      Sem_UsualArith(Ast_Node *node);
void      Sem_PromoteShift(Ast_Node *node);

// Constant expressions
int64_t   Sem_Truncate(const Ast_Type *type, int64_t value);
Ast_TypeSign Sem_FoldSign(const Ast_Node *node);
bool      Sem_FoldOp(Ast_NodeKind kind, int64_t lhs, int64_t rhs, Ast_TypeSign sign, Ast_Line line, int64_t *value);
bool      Sem_Fold(const Ast_Node *node, int64_t *value);
bool      Sem_FoldAddr(const Ast_Node *node, const char **symbol);

// Checks the parser cannot make
Ast_Type *Sem_FuncAddrType(Ast_Node *node);
Ast_Type *Sem_CallType(Ast_Node *node);
Ast_Type *Sem_CalleeType(Ast_Node *node);
void      Sem_CheckArity(Ast_Node *node, int32_t want, Ast_TypeVariadic variadic, Ast_TypeProto proto, const char *what);
void      Sem_ConvertArgs(Ast_Node *node, Ast_Var *params, int32_t nparams, Ast_TypeVariadic variadic, Ast_TypeProto proto);
void      Sem_CheckCall(Ast_Node *node);

// Annotation
Ast_Node *Sem_ScaleBy(Ast_Node *node, int32_t size);
void      Sem_Arith(Ast_Node *node);
bool      Sem_FindLabel(Ast_Node *node, const char *name);
void      Sem_CheckGotos(Ast_Node *node, Ast_Node *body);
void      Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail);
void      Sem_Node(Ast_Node *node);
void      Sem_Analyze(Ast_Func *prog);

#endif // SEM_H
