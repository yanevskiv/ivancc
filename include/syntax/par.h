#ifndef PAR_H
#define PAR_H

#include "syntax/ast.h"

// One step of a declarator, collected walking outward from the name it declares.
typedef enum Par_DerivKind Par_DerivKind;
enum Par_DerivKind {
    PAR_DERIV_POINTER,
    PAR_DERIV_ARRAY,
    PAR_DERIV_FUNCTION
};

// A parameter list as the grammar collects it, before it becomes a function type.
typedef struct Par_ParamList Par_ParamList;
struct Par_ParamList {
    Ast_Var *pl_head;
    Ast_Var *pl_tail;
    int      pl_count;
    int      pl_variadic; // the list ended in `...`
    int      pl_proto;    // false for `()`, which promises nothing about the parameters
};

// One derivation, holding whichever of the three kinds' operands it needs.
typedef struct Par_Deriv Par_Deriv;
struct Par_Deriv {
    Par_Deriv     *pd_next;
    Par_DerivKind  pd_kind;
    long           pd_len;    // element count of an ARRAY
    int            pd_empty;  // the ARRAY was written `[]`, leaving its length unsaid
    Par_ParamList  pd_params; // parameter list of a FUNCTION
    int            pd_line;
};

// A declarator: the name it declares, and the derivations reading outward from that name.
typedef struct Par_Decl Par_Decl;
struct Par_Decl {
    char      *pc_name;
    Par_Deriv *pc_head;
    Par_Deriv *pc_tail;
    Par_Decl  *pc_next;     // next declarator of a comma-separated member declaration
    Ast_Node  *pc_bits;     // width of a bit-field, or NULL when it is not one
    int        pc_line;
};

// Shared declaration state
void Par_SetDeclSpec(Ast_Storage storage, Ast_Type *type);
void Par_ResetEnum(void);

// Parameter lists
void  Par_ClearParams(Par_ParamList *list);
void  Par_PushParam(Par_ParamList *list, Ast_Var *var);

// Declarators
Par_Decl  *Par_NewDecl(char *name);
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, int line);
Ast_Type  *Par_ApplyDerivs(Ast_Type *base, Par_Deriv *deriv);
Ast_Type  *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl);
Ast_Type  *Par_AdjustParam(Ast_Type *type);
Ast_Var   *Par_MakeParam(Ast_Type *base, Par_Decl *decl, int line);
Ast_Var   *Par_MakeAnonParam(Ast_Type *type, int line);

// Types
Ast_Type *Par_ArrayType(Ast_Type *base, Ast_Node *dims);
Ast_Type *Par_VaListType(void);
Ast_Node *Par_VaArg(Ast_Node *ap, Ast_Type *type, int line);

// Aggregates
Ast_Member *Par_AppendMembers(Ast_Member *head, Ast_Member *tail);
void        Par_AddBitfield(Ast_Member *member, Ast_Node *width, int line);
Ast_Member *Par_MakeMembers(Ast_Type *type, Par_Decl *decls);
Ast_Type   *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, int line);
Ast_Type   *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, int line);
void        Par_AddEnumConst(const char *name, Ast_Node *value, int line);

// Initializers
Ast_Node *Par_InitStore(Ast_Var *var, int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line);
Ast_Node *Par_InitAt(int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line);
void      Par_Designate(Ast_Type *type, Ast_Node *desig, int *index, Ast_Member **member, int line);
void      Par_Step(Ast_Type **type, int *off, Ast_Node *desig, int index, Ast_Member *member);
Ast_Type *Par_ExprType(Ast_Node *node);
void      Par_FlattenSlot(Ast_Type *type, int base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, int line);
void      Par_FlattenList(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int braced, int line);
void      Par_Flatten(Ast_Type *type, int base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, int line);
Ast_Node *Par_FlattenInit(Ast_Type *type, Ast_Node *init, int line);
Ast_Node *Par_InitFlat(Ast_Var *var, Ast_Node *flat, int line);
Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, int line);
Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, int line);

// Declarations
void      Par_CheckComplete(const char *name, Ast_Type *type, int line);
void      Par_AddDeclaredType(const char *name, Ast_Type *type, Ast_Node *init, int line);
Ast_Var  *Par_DeclareLocal(const char *name, Ast_Type *type, int line);
Ast_Node *Par_AddLocal(Par_Decl *decl, Ast_Node *init, int line);

// Functions
Ast_Func *Par_FindFunction(const char *name);
void      Par_AddFunction(Ast_Func *fn);
void      Par_DeclarePrototype(const char *name, Ast_Type *type);
Ast_Func *Par_MakeFunction(Ast_Node *body);
void      Par_BeginExternal(Par_Decl *decl, int line);
void      Par_EndExternal(Ast_Node *init, int line);
void      Par_EndFunction(Ast_Node *body);
void      Par_AddDeclared(Par_Decl *decl, Ast_Node *init, int line);

// Expressions
Ast_Node *Par_Designator(char *name, int line);
Ast_Node *Par_MakeCall(Ast_Node *callee, Ast_Node *args, int line);

#endif // PAR_H
