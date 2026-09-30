/*
 * C header file for the abstract syntax tree.
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

#ifndef AST_H
#define AST_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/str.h"

// Slots the string literal table starts with before it grows.
#define AST_STRINGS_FIRST_CAP 64

// Bits in a byte, for placing a bitfield inside the unit that holds it.
#define AST_BITS_PER_BYTE 8

// A source line number.
typedef uint32_t Ast_Line;

// Forward declaration: a struct type lists its members.
typedef struct Ast_Member Ast_Member;

// Forward declaration: a function type lists its parameters.
typedef struct Ast_Var Ast_Var;

// Forward declaration: a global's initializer is one of these.
typedef struct Ast_Node Ast_Node;

// The kind of a type, the integer and floating kinds in rank order.
typedef enum Ast_TypeKind Ast_TypeKind;
enum Ast_TypeKind {
    AST_TYPE_KIND_VOID,
    AST_TYPE_KIND_BOOL,
    AST_TYPE_KIND_CHAR,
    AST_TYPE_KIND_SHORT,
    AST_TYPE_KIND_INT,
    AST_TYPE_KIND_LONG,
    AST_TYPE_KIND_LLONG,
    AST_TYPE_KIND_FLOAT,
    AST_TYPE_KIND_DOUBLE,
    AST_TYPE_KIND_LDOUBLE,
    AST_TYPE_KIND_PTR,
    AST_TYPE_KIND_ARRAY,
    AST_TYPE_KIND_FUNC,
    AST_TYPE_KIND_STRUCT,
    AST_TYPE_KIND_UNION,
    AST_TYPE_KIND_COUNT,                              // number of kinds
    AST_TYPE_KIND_FIRST_INT   = AST_TYPE_KIND_BOOL,   // narrowest integer kind
    AST_TYPE_KIND_LAST_INT    = AST_TYPE_KIND_LLONG,  // widest integer kind
    AST_TYPE_KIND_FIRST_FLOAT = AST_TYPE_KIND_FLOAT,  // narrowest floating kind
    AST_TYPE_KIND_LAST_FLOAT  = AST_TYPE_KIND_LDOUBLE // widest floating kind
};

// The target ABI's sizes in bytes.
typedef enum Ast_TypeSize Ast_TypeSize;
enum Ast_TypeSize {
    AST_TYPE_SIZE_VOID    = 1,
    AST_TYPE_SIZE_BOOL    = 1,
    AST_TYPE_SIZE_CHAR    = 1,
    AST_TYPE_SIZE_SHORT   = 2,
    AST_TYPE_SIZE_INT     = 4,
    AST_TYPE_SIZE_LONG    = 8,
    AST_TYPE_SIZE_LLONG   = 8,
    AST_TYPE_SIZE_FLOAT   = 4,
    AST_TYPE_SIZE_DOUBLE  = 8,
    AST_TYPE_SIZE_LDOUBLE = 16, // ten bytes of x87 extended, padded
    AST_TYPE_SIZE_PTR     = 8,
    AST_TYPE_SIZE_FUNC    = 1   // C gives a function no size; gcc answers 1
};

// The target ABI's alignments in bytes.
typedef enum Ast_TypeAlign Ast_TypeAlign;
enum Ast_TypeAlign {
    AST_TYPE_ALIGN_VOID    = 1,
    AST_TYPE_ALIGN_BOOL    = 1,
    AST_TYPE_ALIGN_CHAR    = 1,
    AST_TYPE_ALIGN_SHORT   = 2,
    AST_TYPE_ALIGN_INT     = 4,
    AST_TYPE_ALIGN_LONG    = 8,
    AST_TYPE_ALIGN_LLONG   = 8,
    AST_TYPE_ALIGN_FLOAT   = 4,
    AST_TYPE_ALIGN_DOUBLE  = 8,
    AST_TYPE_ALIGN_LDOUBLE = 16,
    AST_TYPE_ALIGN_PTR     = 8,
    AST_TYPE_ALIGN_FUNC    = 1
};

// Whether a function's parameter list ended in `...`.
typedef enum Ast_TypeVa Ast_TypeVa;
enum Ast_TypeVa {
    AST_TYPE_FIXED,
    AST_TYPE_VA
};

// Whether a function was declared with a prototype.
typedef enum Ast_TypeProto Ast_TypeProto;
enum Ast_TypeProto {
    AST_TYPE_NOPROTO, // written `int f()`
    AST_TYPE_PROTO
};

// Whether an integer type holds negative values.
typedef enum Ast_TypeSign Ast_TypeSign;
enum Ast_TypeSign {
    AST_TYPE_SIGNED,
    AST_TYPE_UNSIGNED
};

// The qualifiers a declaration may carry.
typedef enum Ast_Qual Ast_Qual;
enum Ast_Qual {
    AST_QUAL_CONST    = 1,
    AST_QUAL_VOLATILE = 2,
    AST_QUAL_RESTRICT = 4
};

// Whether a type's members have been seen.
typedef enum Ast_TypeComplete Ast_TypeComplete;
enum Ast_TypeComplete {
    AST_TYPE_INCOMPLETE,
    AST_TYPE_COMPLETE
};

// The kind of an AST node.
typedef enum Ast_NodeKind Ast_NodeKind;
enum Ast_NodeKind {
    AST_NODE_KIND_NUM,       // integer literal
    AST_NODE_KIND_FNUM,      // floating literal
    AST_NODE_KIND_STR,       // string literal
    AST_NODE_KIND_VAR,       // a reference to a local variable
    AST_NODE_KIND_ADD,       // lhs + rhs
    AST_NODE_KIND_SUB,       // lhs - rhs
    AST_NODE_KIND_MUL,       // lhs * rhs
    AST_NODE_KIND_DIV,       // lhs / rhs
    AST_NODE_KIND_MOD,       // lhs % rhs
    AST_NODE_KIND_NEG,       // -lhs
    AST_NODE_KIND_NOT,       // !lhs
    AST_NODE_KIND_BITNOT,    // ~lhs
    AST_NODE_KIND_BITAND,    // lhs & rhs
    AST_NODE_KIND_BITOR,     // lhs | rhs
    AST_NODE_KIND_BITXOR,    // lhs ^ rhs
    AST_NODE_KIND_SHL,       // lhs << rhs
    AST_NODE_KIND_SHR,       // lhs >> rhs, arithmetic on a signed operand
    AST_NODE_KIND_ADDR,      // &lhs
    AST_NODE_KIND_DEREF,     // *lhs
    AST_NODE_KIND_MEMBER,    // lhs.an_member
    AST_NODE_KIND_CAST,      // (type) lhs
    AST_NODE_KIND_SIZEOF,    // sizeof lhs
    AST_NODE_KIND_EQ,        // lhs == rhs
    AST_NODE_KIND_NE,        // lhs != rhs
    AST_NODE_KIND_LT,        // lhs <  rhs   (> is LT reversed)
    AST_NODE_KIND_LE,        // lhs <= rhs   (>= is LE reversed)
    AST_NODE_KIND_AND,       // lhs && rhs
    AST_NODE_KIND_OR,        // lhs || rhs
    AST_NODE_KIND_ASSIGN,    // lhs = rhs
    AST_NODE_KIND_OPASSIGN,  // lhs an_op= rhs
    AST_NODE_KIND_POSTINC,   // lhs++ or lhs--, stepping by an_val
    AST_NODE_KIND_COND,      // cond ? then : els
    AST_NODE_KIND_COMMA,     // lhs, rhs
    AST_NODE_KIND_INIT,      // one flattened initializer
    AST_NODE_KIND_INITLIST,  // a braced initializer list
    AST_NODE_KIND_DESIGNATOR,// `[an_val]` or `.an_memname` naming where an item lands
    AST_NODE_KIND_ZERO,      // zero an_val bytes of the object an_lhs addresses
    AST_NODE_KIND_COMPOUND,  // (type){...}
    AST_NODE_KIND_CALL,      // function call
    AST_NODE_KIND_FUNCADDR,  // a function named as a value
    AST_NODE_KIND_VA_START,  // __builtin_va_start(lhs, last)
    AST_NODE_KIND_VA_ARG,    // __builtin_va_arg(lhs, T)
    AST_NODE_KIND_RETURN,    // return lhs;
    AST_NODE_KIND_IF,        // if (cond) then; else els;
    AST_NODE_KIND_FOR,       // for (init; cond; inc) body;
    AST_NODE_KIND_DO,        // do body; while (cond);
    AST_NODE_KIND_SWITCH,    // switch (cond) body;
    AST_NODE_KIND_CASE,      // case cond: lhs;
    AST_NODE_KIND_DEFAULT,   // default: lhs;
    AST_NODE_KIND_GOTO,      // goto an_funcname;
    AST_NODE_KIND_LABEL,     // an_funcname: lhs;
    AST_NODE_KIND_BREAK,     // break;
    AST_NODE_KIND_CONTINUE,  // continue;
    AST_NODE_KIND_BLOCK,     // { ... }
    AST_NODE_KIND_DECL,      // the statements one declaration becomes
    AST_NODE_KIND_VLA,       // allocate an_var, an array of an_lhs bytes
    AST_NODE_KIND_VSIZE,     // an_var = an_lhs elements of an_rhs bytes
    AST_NODE_KIND_EXPR_STMT, // expression used as a statement
    AST_NODE_KIND_NOP,       // empty statement / bare declaration
    AST_NODE_KIND_COUNT      // number of kinds
};

// What a declaration's storage class asks for.
typedef enum Ast_Storage Ast_Storage;
enum Ast_Storage {
    AST_STORAGE_NONE,    // no storage class was written
    AST_STORAGE_STATIC,  // visible only to this translation unit
    AST_STORAGE_EXTERN,  // declared here, defined elsewhere
    AST_STORAGE_TYPEDEF, // binds a name to a type rather than declaring an object
    AST_STORAGE_COUNT    // number of storage classes
};

// A C type.
typedef struct Ast_Type Ast_Type;
struct Ast_Type {
    Ast_TypeKind     at_kind;     // which kind of type this is
    int32_t          at_size;     // bytes an object of this type occupies
    int32_t          at_align;    // address multiple an object must sit on
    Ast_TypeSign     at_sign;     // signedness of an integer kind
    Ast_Qual         at_qual;     // the qualifiers written on the declaration
    Ast_Type        *at_base;     // pointee for PTR, element type for ARRAY
    int32_t          at_len;      // element count for ARRAY
    Ast_Node        *at_vlen;     // run-time length of an ARRAY; NOP for `[*]`
    Ast_Var         *at_vsize;    // local that sizes a variable-length ARRAY
    char            *at_tag;      // tag a STRUCT or UNION was declared with, or NULL
    Ast_Member      *at_members;  // members of a STRUCT or UNION
    Ast_TypeComplete at_complete; // incomplete until the member list or the length is known
    Ast_Type        *at_ret;      // return type of a FUNC
    Ast_Var         *at_params;   // parameters of a FUNC
    int32_t          at_nparams;  // number of parameters a FUNC declares
    Ast_TypeVa       at_va;       // whether a FUNC's parameter list ended in `...`
    Ast_TypeProto    at_proto;    // whether a FUNC was declared with a prototype
};

// One member of a struct or union, at the offset layout gave it.
struct Ast_Member {
    Ast_Member *am_next;
    char       *am_name;     // NULL for a bitfield declared only to pad
    Ast_Type   *am_type;
    Ast_Type   *am_owner;    // aggregate the member was declared in
    int32_t     am_offset;   // bytes from the start of the enclosing aggregate
    Ast_Line    am_line;     // source line the member was declared on
    bool        am_flexible; // true for a trailing `d[]`
    int32_t     am_bits;     // width of a bitfield, or 0
    int32_t     am_bitoff;   // bits into am_offset where a bitfield starts
};

// An interned string literal.
typedef struct Ast_Str Ast_Str;
struct Ast_Str {
    char   *as_data;  // decoded bytes
    size_t  as_len;   // number of bytes before that terminator
    size_t  as_width; // bytes one element takes
};

// A local variable or function parameter.
struct Ast_Var {
    Ast_Var     *av_next;       // chains every local in a function
    Ast_Var     *av_param_next; // chains parameters in declaration order
    Ast_Var     *av_scope_next; // chains the variables of one lexical scope
    char        *av_name;       // identifier as written in the source
    Ast_Type    *av_type;       // declared type
    Ast_Line     av_line;       // source line the declaration appeared on
    int32_t      av_offset;     // offset from %rbp
    char        *av_symbol;     // name the symbol takes
    bool         av_global;     // true when the variable lives in .data or .bss
    Ast_Storage  av_storage;    // storage class the declaration asked for
    Ast_Node    *av_init;       // initializer of a global, or NULL
    Ast_Type    *av_vmtype;     // a variably modified parameter's declared type
};

// A struct, union or enum tag.
typedef struct Ast_Tag Ast_Tag;
struct Ast_Tag {
    Ast_Tag  *ag_next;
    char     *ag_name;
    Ast_Type *ag_type;
};

// An enumeration constant: an int that answers to a name.
typedef struct Ast_EnumConst Ast_EnumConst;
struct Ast_EnumConst {
    Ast_EnumConst *ae_next;
    char          *ae_name;
    int64_t        ae_value;
};

// A name a typedef declaration bound to a type.
typedef struct Ast_Typedef Ast_Typedef;
struct Ast_Typedef {
    Ast_Typedef *ad_next;
    char        *ad_name;
    Ast_Type    *ad_type;
};

// The scope of one variably modified name, inside those declared before it.
typedef struct Ast_VmScope Ast_VmScope;
struct Ast_VmScope {
    Ast_VmScope *vs_outer; // the variably modified name declared before it
};

// One lexical scope: what was declared directly inside a pair of braces.
typedef struct Ast_Scope Ast_Scope;
struct Ast_Scope {
    Ast_Scope       *as_parent;   // the scope this one is nested in
    Ast_VmScope     *as_vm;       // innermost variably modified name in scope
    Ast_Var         *as_vars;     // declared here, innermost names first
    Ast_Tag         *as_tags;     // struct, union and enum tags declared here
    Ast_Typedef     *as_typedefs; // typedef names declared here
    Ast_EnumConst   *as_enums;    // enumeration constants declared here
    bool             as_params;   // holds a function definition's parameters
};

// A node in the abstract syntax tree.
struct Ast_Node {
    Ast_NodeKind an_kind;     // which kind of node this is
    Ast_NodeKind an_op;       // operation of AST_NODE_KIND_OPASSIGN
    Ast_Type    *an_type;     // type of the value
    Ast_Line     an_line;     // source line the construct started on
    Ast_Node    *an_next;     // next node in a statement / argument list
    Ast_Node    *an_lhs;      // generic left operand
    Ast_Node    *an_rhs;      // generic right operand
    Ast_Node    *an_cond;     // condition of AST_NODE_KIND_IF / AST_NODE_KIND_FOR
    Ast_Node    *an_then;     // then branch of AST_NODE_KIND_IF
    Ast_Node    *an_els;      // else branch of AST_NODE_KIND_IF
    Ast_Node    *an_init;     // initialiser of AST_NODE_KIND_FOR
    Ast_Node    *an_inc;      // increment of AST_NODE_KIND_FOR
    Ast_Node    *an_body;     // statement list for AST_NODE_KIND_BLOCK / FOR body
    char        *an_funcname; // callee of a CALL, or the label a GOTO names
    Ast_Node    *an_args;     // argument list for AST_NODE_KIND_CALL
    Ast_Node    *an_cases;    // cases of AST_NODE_KIND_SWITCH, in source order
    Ast_Node    *an_case_next; // next case of the switch this one belongs to
    int32_t      an_label;    // label number a case is emitted with
    int64_t      an_val;      // integer value for AST_NODE_KIND_NUM
    long double  an_fval;     // value for AST_NODE_KIND_FNUM
    size_t       an_str_idx;  // string table slot for AST_NODE_KIND_STR
    Ast_Var     *an_var;      // variable a VAR names, or the object a COMPOUND fills
    Ast_Node    *an_items;    // flattened initializer a COMPOUND fills
    Ast_Member  *an_member;   // resolved member of AST_NODE_KIND_MEMBER
    char        *an_memname;  // member name a MEMBER node was written with
    int32_t      an_tmp;      // frame slot an aggregate return lands in
    int32_t      an_calltmp;  // frame slot an indirect CALL parks its callee in
    Ast_VmScope *an_vm;       // variably modified scopes a jump or label is in
    bool         an_initstore; // an ASSIGN that initializes rather than assigns
};

// A function definition.
typedef struct Ast_Func Ast_Func;
struct Ast_Func {
    Ast_Func *af_next;       // next function in the program
    char     *af_name;       // function name
    Ast_Node *af_body;       // function body (AST_NODE_KIND_BLOCK)
    Ast_Type *af_type;       // the FUNC type it was declared or defined with
    bool      af_static;     // true when the function is local to this file
    Ast_Var  *af_locals;     // every local, including parameters
    int32_t   af_stack_size; // frame size
};

// The finished program, produced by the parser.
extern Ast_Func *Ast_Program;

// Every variable declared at file scope.
extern Ast_Var *Ast_Globals;

// Primitive types
extern Ast_Type Ast_TypeVoid;
extern Ast_Type Ast_TypeBool;
extern Ast_Type Ast_TypeChar;
extern Ast_Type Ast_TypeUChar;
extern Ast_Type Ast_TypeShort;
extern Ast_Type Ast_TypeUShort;
extern Ast_Type Ast_TypeInt;
extern Ast_Type Ast_TypeUInt;
extern Ast_Type Ast_TypeLong;
extern Ast_Type Ast_TypeULong;
extern Ast_Type Ast_TypeLLong;
extern Ast_Type Ast_TypeULLong;
extern Ast_Type Ast_TypeFloat;
extern Ast_Type Ast_TypeDouble;
extern Ast_Type Ast_TypeLDouble;

// Type construction
int32_t   Ast_AlignTo(int32_t n, int32_t align);
int32_t   Ast_AlignDown(int32_t n, int32_t align);
Ast_Type *Ast_IntegerType(Ast_TypeKind kind, Ast_TypeSign sign);
Ast_Type *Ast_Qualify(Ast_Type *type, Ast_Qual qual);
bool      Ast_IsInteger(const Ast_Type *type);
bool      Ast_IsFloating(const Ast_Type *type);
bool      Ast_IsArithmetic(const Ast_Type *type);
Ast_Type *Ast_NewPointer(Ast_Type *base);
Ast_Type *Ast_NewArray(Ast_Type *base, int32_t len);
Ast_Type *Ast_NewUnsizedArray(Ast_Type *base);
bool      Ast_IsUnsized(const Ast_Type *type);
Ast_Type *Ast_SizeArray(const Ast_Type *type, int32_t len);
Ast_Type *Ast_NewVla(Ast_Type *base, Ast_Node *len);
bool      Ast_IsVla(const Ast_Type *type);
bool      Ast_IsVm(const Ast_Type *type);
Ast_Type *Ast_NewFunction(Ast_Type *ret, Ast_Var *params, int32_t nparams, Ast_TypeVa va, Ast_TypeProto proto);
Ast_Type *Ast_NewAggregate(Ast_TypeKind kind, const char *tag);
Ast_Member *Ast_NewMember(const char *name, Ast_Type *type, Ast_Line line);
int32_t   Ast_PlaceBitfield(Ast_Member *member, int32_t bits);
Ast_Member *Ast_NamedMembers(Ast_Member *members);
void      Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, Ast_Line line);
Ast_Member *Ast_FindMember(const Ast_Type *type, const char *name);
bool      Ast_IsCompatible(const Ast_Type *a, const Ast_Type *b);
bool      Ast_IsCompatibleUnqualified(const Ast_Type *a, const Ast_Type *b);
bool      Ast_IsCompatibleParams(const Ast_Type *a, const Ast_Type *b);

// Node construction
Ast_Node *Ast_NewNode(Ast_NodeKind kind, Ast_Line line);
Ast_Node *Ast_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line);
Ast_Node *Ast_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Line line);
Ast_Node *Ast_NewNum(int64_t val, Ast_Line line);
Ast_Node *Ast_NewFNum(long double val, Ast_Type *type, Ast_Line line);
Ast_Node *Ast_NewVarNode(Ast_Var *var, Ast_Line line);
Ast_Node *Ast_NewOpAssign(Ast_NodeKind op, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line);
Ast_Node *Ast_NewPostInc(Ast_Node *lhs, int64_t step, Ast_Line line);
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, Ast_Line line);

// Variable scopes
void     Ast_BeginScope(void);
void     Ast_EndScope(void);
void     Ast_PushScope(void);
void     Ast_PopScope(void);
Ast_Var *Ast_FindVar(const char *name);
Ast_Scope *Ast_SharedScope(void);
Ast_Var *Ast_FindVarHere(const char *name);
Ast_Var *Ast_FindGlobal(const char *symbol);
Ast_Func *Ast_FindFunction(const char *name);
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, Ast_Line line);
void     Ast_DeclareParam(Ast_Var *var);
void     Ast_DeclarePrototypeParam(Ast_Var *var);
Ast_Var *Ast_DeclareGlobal(const char *name, Ast_Type *type, Ast_Line line);
Ast_Var *Ast_DeclareStaticLocal(const char *name, const char *symbol, Ast_Type *type, Ast_Line line);
Ast_Var *Ast_DeclareExternLocal(const char *name, Ast_Type *type, Ast_Line line);
Ast_Var *Ast_CurrentLocals(void);
void     Ast_OpenVmScope(void);
Ast_VmScope *Ast_CurrentVmScope(void);
bool     Ast_ContainsVmScope(const Ast_VmScope *scope, const Ast_VmScope *outer);

// Tags and typedef names
Ast_Type *Ast_FindTag(const char *name);
Ast_Type *Ast_FindTagHere(const char *name);
void      Ast_DeclareTag(const char *name, Ast_Type *type);
Ast_Type *Ast_FindTypedef(const char *name);
Ast_Type *Ast_FindTypedefHere(const char *name);
void      Ast_DeclareTypedef(const char *name, Ast_Type *type);
bool      Ast_FindEnumConst(const char *name, int64_t *value);
bool      Ast_IsEnumConstHere(const char *name);
void      Ast_DeclareEnumConst(const char *name, int64_t value);

// String literal interning
size_t   Ast_AddString(char *s, size_t len, size_t width);
size_t   Ast_StringCount(void);
Ast_Str *Ast_StringAt(size_t idx);

#endif // AST_H
