/*
 * C header file for the parser's declarator and parameter helpers.
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

#ifndef PAR_H
#define PAR_H

// Standard headers.
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/str.h"
#include "lang/ast.h"
#include "lang/sem.h"

// Bits in the value a literal is read into.
#define PAR_VALUE_BITS 64

// Name of the array that holds a function's own name.
#define PAR_FUNC_NAME "__func__"

// The asm qualifiers as a diagnostic names them.
#define PAR_ASM_VOLATILE_NAME "volatile"
#define PAR_ASM_INLINE_NAME   "inline"

// All bits of one byte set.
#define PAR_BYTE_MASK 0xFF

// Digits an escape sequence may carry.
#define PAR_MAX_OCTAL_DIGITS 3
#define PAR_MAX_HEX_DIGITS   8
#define PAR_UCN_SHORT_DIGITS 4
#define PAR_UCN_LONG_DIGITS  8

// The first code point UTF-8 spends two, three and four bytes on.
#define PAR_UTF8_MAX_ONE   0x80
#define PAR_UTF8_MAX_TWO   0x800
#define PAR_UTF8_MAX_THREE 0x10000

// The byte counts those ranges take.
#define PAR_UTF8_LEN_TWO   2
#define PAR_UTF8_LEN_THREE 3
#define PAR_UTF8_LEN_FOUR  4

// The tag, payload mask and shift of one UTF-8 continuation byte.
#define PAR_UTF8_CONT  0x80
#define PAR_UTF8_MASK  0x3F
#define PAR_UTF8_SHIFT 6

// What an array declarator's brackets carry besides a length.
typedef enum Par_ArrayDecor Par_ArrayDecor;
enum Par_ArrayDecor {
    PAR_ARRAY_NONE   = 0,      // plain brackets
    PAR_ARRAY_STATIC = 1 << 0, // `static` before the length
    PAR_ARRAY_QUAL   = 1 << 1  // one or more qualifiers
};

// The qualifiers an asm statement was written with.
typedef enum Par_AsmQual Par_AsmQual;
enum Par_AsmQual {
    PAR_ASM_QUAL_NONE     = 0,
    PAR_ASM_QUAL_VOLATILE = 1 << 0,
    PAR_ASM_QUAL_INLINE   = 1 << 1
};

// One type specifier keyword.
typedef enum Par_Spec Par_Spec;
enum Par_Spec {
    PAR_SPEC_NONE     = 0,       // no specifier keyword was written
    PAR_SPEC_VOID     = 1 << 0,
    PAR_SPEC_BOOL     = 1 << 1,
    PAR_SPEC_CHAR     = 1 << 2,
    PAR_SPEC_SHORT    = 1 << 3,
    PAR_SPEC_INT      = 1 << 4,
    PAR_SPEC_LONG     = 1 << 5,
    PAR_SPEC_LLONG    = 1 << 6,  // set when a second `long` arrives
    PAR_SPEC_SIGNED   = 1 << 7,
    PAR_SPEC_UNSIGNED = 1 << 8,
    PAR_SPEC_FLOAT    = 1 << 9,
    PAR_SPEC_DOUBLE   = 1 << 10
};

// The storage-class and function specifiers a place in the grammar allows.
typedef enum Par_StorageUse Par_StorageUse;
enum Par_StorageUse {
    PAR_STORAGE_FORBIDDEN, // a member or a type name
    PAR_STORAGE_REGISTER,  // a parameter, which may be `register`
    PAR_STORAGE_ANY,       // a declaration
    PAR_STORAGE_COUNT      // number of uses
};

// One step of a declarator, collected walking outward from the name.
typedef enum Par_DerivKind Par_DerivKind;
enum Par_DerivKind {
    PAR_DERIV_POINTER,
    PAR_DERIV_ARRAY,
    PAR_DERIV_FUNCTION,
    PAR_DERIV_COUNT // number of kinds
};

// Whether an initializer list was written with its own braces.
typedef enum Par_List Par_List;
enum Par_List {
    PAR_LIST_UNBRACED,
    PAR_LIST_BRACED
};

// The bases an escape sequence is written in.
typedef enum Par_Base Par_Base;
enum Par_Base {
    PAR_BASE_OCTAL   = 8,
    PAR_BASE_DECIMAL = 10,
    PAR_BASE_HEX     = 16
};

// An integer literal.
typedef struct Par_Num Par_Num;
struct Par_Num {
    int64_t   pn_val;
    Ast_Type *pn_type;
};

// A floating literal.
typedef struct Par_FNum Par_FNum;
struct Par_FNum {
    long double pf_val;
    Ast_Type   *pf_type;
};

// The specifiers and qualifiers one declaration wrote.
typedef struct Par_Specs Par_Specs;
struct Par_Specs {
    Par_Spec    ps_specs;    // the type specifier keywords seen
    Ast_Qual    ps_qual;     // the qualifier keywords seen
    Ast_Type   *ps_type;     // the type a struct, union, enum or typedef name named
    Ast_Storage ps_storage;  // the storage class seen, NONE for `auto` and `register`
    int32_t     ps_nstorage; // the storage-class keywords seen
    bool        ps_register; // the storage class seen was `register`
    bool        ps_inline;   // `inline` was seen
};

// A parameter list as the grammar collects it.
typedef struct Par_ParamList Par_ParamList;
struct Par_ParamList {
    Ast_Var      *pl_head;
    Ast_Var      *pl_tail;
    int32_t       pl_count;
    Ast_TypeVa    pl_va;    // the list ended in `...`
    Ast_TypeProto pl_proto; // written `()`
};

// One derivation and the operands its kind needs.
typedef struct Par_Deriv Par_Deriv;
struct Par_Deriv {
    Par_Deriv     *pd_next;
    Par_DerivKind  pd_kind;
    Ast_Qual       pd_qual;   // qualifiers written after a POINTER's star
    int64_t        pd_len;    // element count of an ARRAY
    Ast_Node      *pd_vlen;   // run-time element count of an ARRAY, or NULL
    bool           pd_empty;  // the ARRAY was written `[]`
    bool           pd_star;   // the ARRAY was written `[*]`
    Par_ArrayDecor pd_decor;  // what the ARRAY's brackets carried besides a length
    Par_ParamList  pd_params; // parameter list of a FUNCTION
    Ast_Line       pd_line;
};

// A declarator.
typedef struct Par_Decl Par_Decl;
struct Par_Decl {
    char      *pc_name;
    Par_Deriv *pc_head;
    Par_Deriv *pc_tail;
    Par_Decl  *pc_next;     // next declarator of a comma-separated member declaration
    Ast_Node  *pc_bits;     // width of a bit-field, or NULL
    Ast_Line   pc_line;
};

// Shared declaration state
void Par_SetDeclSpec(const Par_Specs *specs, Par_StorageUse use, Ast_Line line);
void Par_ResetEnum(void);

// Parameter lists
void  Par_ClearParams(Par_ParamList *list);
void  Par_PushParam(Par_ParamList *list, Ast_Var *var);

// Declarators
Par_Decl  *Par_NewDecl(char *name);
void       Par_NeedName(Par_Decl *decl, Ast_Line line);
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, Ast_Line line);
void       Par_AddPointers(Par_Decl *decl, const Par_Deriv *star);
void       Par_SetArrayLen(Par_Deriv *deriv, Ast_Node *len);
Ast_Type  *Par_ApplyDerivs(Ast_Type *base, Par_Deriv *deriv);
Ast_Type  *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl);
Ast_Type  *Par_AdjustParam(Ast_Type *type);
void       Par_TakeArrayDecor(Par_Decl *decl, Ast_Line line);
Ast_Var   *Par_MakeParam(Ast_Type *base, Par_Decl *decl, Ast_Line line);
Ast_Var   *Par_MakeKnrParam(char *name, Ast_Line line);
void       Par_SetKnrParam(Par_Decl *decl, Ast_Line line);
void       Par_CheckKnrParams(void);
Ast_Var   *Par_MakeAnonParam(Ast_Type *type, Ast_Line line);
void       Par_KeepVmType(Ast_Var *param, Ast_Type *type);
Ast_Node  *Par_SizeParams(void);
void       Par_BeginBody(void);

// Literals
Par_Num  Par_NumLiteral(const char *text);
Par_FNum Par_FloatLiteral(const char *text);
Par_Num  Par_CharLiteral(const char *body, size_t len, size_t width, Ast_Line line);
Ast_Str  Par_StringLiteral(const char *body, size_t len, size_t width, Ast_Line line);
Ast_Str  Par_WidenString(Ast_Str str, size_t width);
Ast_Str  Par_ConcatStrings(Ast_Str left, Ast_Str right);

// Escape sequences
int32_t  Par_DigitValue(char ch);
uint64_t Par_ScanDigits(const char *text, size_t len, size_t *pos, Par_Base base, size_t count);
uint64_t Par_GetElement(const char *data, size_t width);
void     Par_PutElement(char *buf, size_t *len, size_t width, uint64_t value);
void     Par_PutEscape(char *buf, size_t *len, size_t width, uint64_t value, Ast_Line line);
void     Par_PutUtf8(char *buf, size_t *len, uint64_t value);
char    *Par_UnescapeLiteral(const char *body, size_t len, size_t width, size_t *out_len, Ast_Line line);

// Types
void        Par_ClearSpecs(Par_Specs *specs);
Par_Specs   Par_StorageSpec(Ast_Storage storage);
Par_Spec    Par_AddSpec(Par_Spec specs, Par_Spec spec, Ast_Line line);
void        Par_TakeSpec(Par_Specs *into, const Par_Specs *one, Ast_Line line);
Ast_Storage Par_SpecsStorage(const Par_Specs *specs, Par_StorageUse use, Ast_Line line);
Ast_Type   *Par_SpecType(Par_Spec specs, Ast_Line line);
Ast_Type   *Par_SpecsType(const Par_Specs *specs, Ast_Line line);
Ast_Type   *Par_VaListType(void);
Ast_Node   *Par_VaArg(Ast_Node *ap, Ast_Type *type, Ast_Line line);
Ast_Node   *Par_VaCopy(Ast_Node *dst, Ast_Node *src, Ast_Line line);

// Aggregates
Ast_Member *Par_AppendMembers(Ast_Member *head, Ast_Member *tail);
void        Par_AddBitfield(Ast_Member *member, Ast_Node *width, Ast_Line line);
Ast_Member *Par_MakeMembers(Ast_Type *type, Par_Decl *decls);
void        Par_CheckTagKind(const Ast_Type *type, Ast_TypeKind kind, const char *tag, Ast_Line line);
Ast_Type   *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line);
Ast_Type   *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line);
void        Par_AddEnumConst(const char *name, Ast_Node *value, Ast_Line line);

// Initializers
Ast_Node *Par_InitStore(Ast_Var *var, int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line);
Ast_Node *Par_InitAt(int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line);
void      Par_Designate(Ast_Type *type, Ast_Node *desig, int32_t *index, Ast_Member **member, Ast_Line line);
void      Par_Step(Ast_Type **type, int32_t *off, Ast_Node *desig, int32_t index, Ast_Member *member);
void      Par_Reach(const Ast_Type *type, int32_t len);
Ast_Type *Par_ExprType(Ast_Node *node);
bool      Par_IsStringInit(const Ast_Type *type, const Ast_Node *init);
void      Par_FlattenString(Ast_Type *type, int32_t base, Ast_Node *init, Ast_Node **tail, Ast_Line line);
void      Par_FlattenSlot(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, Ast_Line line);
void      Par_FlattenList(Ast_Type *type, int32_t base, Ast_Node **item, Ast_Node **tail, Par_List braced, Ast_Line line);
void      Par_Flatten(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, Ast_Line line);
Ast_Node *Par_FlattenInit(Ast_Type **type, Ast_Node *init, Ast_Line line);
Ast_Node *Par_InitFlat(Ast_Var *var, Ast_Node *flat, Ast_Line line);
Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, Ast_Line line);
Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, Ast_Line line);

// Declarations
void      Par_CheckComplete(const char *name, Ast_Type *type, Ast_Line line);
void      Par_NeedFixedSize(const Ast_Type *type, Ast_Line line);
Ast_Node *Par_SizeExpr(Ast_Type *type, Ast_Line line);
Ast_Node *Par_WithSizes(Ast_Type *type, Ast_Node *expr, Ast_Line line);
void      Par_Redeclare(Ast_Var *var, Ast_Line line);
void      Par_AddDeclaredType(const char *name, Ast_Type *type, Ast_Node *init, Ast_Line line);
void      Par_CheckRedeclaration(const char *name, const Ast_Type *type, Ast_Line line);
Ast_Var  *Par_DeclareLocal(const char *name, Ast_Type *type, Ast_Line line);
Ast_Node *Par_DefineLocal(Par_Decl *decl, Ast_Var *var, Ast_Node *init, Ast_Line line);
Ast_Node *Par_AddLocal(Par_Decl *decl, Ast_Node *init, Ast_Line line);
void      Par_CompleteTentatives(void);

// Functions
void      Par_AddFunction(Ast_Func *fn, Ast_Line line);
void      Par_DeclarePrototype(const char *name, Ast_Type *type, Ast_Line line);
Ast_Func *Par_MakeFunction(Ast_Node *body);
void      Par_BeginExternal(Par_Decl *decl, Ast_Line line);
void      Par_EndExternal(Ast_Node *init, Ast_Line line);
void      Par_EndFunction(Ast_Node *body);
Ast_Var  *Par_FindFuncName(const char *name, Ast_Line line);
void      Par_AddDeclared(Par_Decl *decl, Ast_Node *init, Ast_Line line);

// Expressions
Ast_Node *Par_Designator(char *name, Ast_Line line);
Ast_Node *Par_MakeCall(Ast_Node *callee, Ast_Node *args, Ast_Line line);
Ast_Node *Par_SizeOfType(Ast_Type *type, Ast_Line line);

// Statements
Ast_Node   *Par_NewJump(Ast_NodeKind kind, Ast_Line line);
Par_AsmQual Par_AddAsmQual(Par_AsmQual quals, Par_AsmQual qual, Ast_Line line);
Ast_Node   *Par_NewAsm(Ast_Str text, Ast_Line line);

// Parsing
void Par_ParseText(const char *text, size_t len);

#endif // PAR_H
