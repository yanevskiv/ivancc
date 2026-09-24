// C header file for the parser's declarator and parameter helpers.

#ifndef PAR_H
#define PAR_H

#include "syntax/ast.h"

// What an array declarator's brackets carry besides a length.
#define PAR_ARRAY_STATIC 1
#define PAR_ARRAY_QUAL   2

// Bits in the unsigned long a literal's value is read into.
#define PAR_LONG_BITS 64

// One type specifier keyword.
typedef enum Par_Spec Par_Spec;
enum Par_Spec {
    PAR_SPEC_NONE     = 0,
    PAR_SPEC_VOID     = 1 << 0,
    PAR_SPEC_BOOL     = 1 << 1,
    PAR_SPEC_CHAR     = 1 << 2,
    PAR_SPEC_SHORT    = 1 << 3,
    PAR_SPEC_INT      = 1 << 4,
    PAR_SPEC_LONG     = 1 << 5,
    PAR_SPEC_LLONG    = 1 << 6,  // set when a second `long` arrives
    PAR_SPEC_SIGNED   = 1 << 7,
    PAR_SPEC_UNSIGNED = 1 << 8
};

// One step of a declarator, collected walking outward from the name.
typedef enum Par_DerivKind Par_DerivKind;
enum Par_DerivKind {
    PAR_DERIV_POINTER,
    PAR_DERIV_ARRAY,
    PAR_DERIV_FUNCTION
};

// An integer literal.
typedef struct Par_Num Par_Num;
struct Par_Num {
    long      pn_val;
    Ast_Type *pn_type;
};

// The type specifiers and qualifiers one declaration wrote.
typedef struct Par_Specs Par_Specs;
struct Par_Specs {
    int       ps_specs; // the PAR_SPEC_ keywords seen
    int       ps_qual;  // the AST_QUAL_ keywords seen
    Ast_Type *ps_type;  // the type a struct, union, enum or typedef name named
};

// A parameter list as the grammar collects it.
typedef struct Par_ParamList Par_ParamList;
struct Par_ParamList {
    Ast_Var *pl_head;
    Ast_Var *pl_tail;
    int      pl_count;
    int      pl_variadic; // the list ended in `...`
    int      pl_proto;    // false for `()`
};

// One derivation and the operands its kind needs.
typedef struct Par_Deriv Par_Deriv;
struct Par_Deriv {
    Par_Deriv     *pd_next;
    Par_DerivKind  pd_kind;
    long           pd_len;    // element count of an ARRAY
    int            pd_empty;  // the ARRAY was written `[]`
    int            pd_decor;  // PAR_ARRAY_* the ARRAY's brackets carried
    Par_ParamList  pd_params; // parameter list of a FUNCTION
    int            pd_line;
};

// A declarator.
typedef struct Par_Decl Par_Decl;
struct Par_Decl {
    char      *pc_name;
    Par_Deriv *pc_head;
    Par_Deriv *pc_tail;
    Par_Decl  *pc_next;     // next declarator of a comma-separated member declaration
    Ast_Node  *pc_bits;     // width of a bit-field, or NULL
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
void       Par_NeedName(Par_Decl *decl, int line);
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, int line);
Ast_Type  *Par_ApplyDerivs(Ast_Type *base, Par_Deriv *deriv);
Ast_Type  *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl);
Ast_Type  *Par_AdjustParam(Ast_Type *type);
void       Par_TakeArrayDecor(Par_Decl *decl, int line);
Ast_Var   *Par_MakeParam(Ast_Type *base, Par_Decl *decl, int line);
Ast_Var   *Par_MakeKnrParam(char *name, int line);
void       Par_SetKnrParam(Par_Decl *decl, int line);
void       Par_CheckKnrParams(void);
Ast_Var   *Par_MakeAnonParam(Ast_Type *type, int line);

// Types
Par_Num   Par_NumLiteral(const char *text);
void      Par_ClearSpecs(Par_Specs *specs);
int       Par_AddSpec(int specs, Par_Spec spec, int line);
void      Par_TakeSpec(Par_Specs *into, const Par_Specs *one, int line);
Ast_Type *Par_SpecType(int specs, int line);
Ast_Type *Par_SpecsType(const Par_Specs *specs, int line);
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
