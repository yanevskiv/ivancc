/*
 * C header file for the abstract syntax tree.
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

#ifndef AST_H
#define AST_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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
// [C99:6.2.5]:     types
// [C99:6.3.1.1p1]: integer conversion ranks
typedef enum Ast_TypeKind Ast_TypeKind;
enum Ast_TypeKind {
    AST_TYPE_KIND_VOID,                               // void
    AST_TYPE_KIND_BOOL,                               // _Bool
    AST_TYPE_KIND_CHAR,                               // char
    AST_TYPE_KIND_SHORT,                              // short
    AST_TYPE_KIND_INT,                                // int
    AST_TYPE_KIND_LONG,                               // long
    AST_TYPE_KIND_LLONG,                              // long long
    AST_TYPE_KIND_FLOAT,                              // float
    AST_TYPE_KIND_DOUBLE,                             // double
    AST_TYPE_KIND_LDOUBLE,                            // long double
    AST_TYPE_KIND_PTR,                                // a pointer
    AST_TYPE_KIND_ARRAY,                              // an array
    AST_TYPE_KIND_FUNC,                               // a function
    AST_TYPE_KIND_STRUCT,                             // a struct
    AST_TYPE_KIND_UNION,                              // a union
    AST_TYPE_KIND_COUNT,                              // number of kinds
    AST_TYPE_KIND_FIRST_INT   = AST_TYPE_KIND_BOOL,   // narrowest integer kind
    AST_TYPE_KIND_LAST_INT    = AST_TYPE_KIND_LLONG,  // widest integer kind
    AST_TYPE_KIND_FIRST_FLOAT = AST_TYPE_KIND_FLOAT,  // narrowest floating kind
    AST_TYPE_KIND_LAST_FLOAT  = AST_TYPE_KIND_LDOUBLE // widest floating kind
};

// The target ABI's sizes in bytes.
typedef enum Ast_TypeSize Ast_TypeSize;
enum Ast_TypeSize {
    AST_TYPE_SIZE_VOID    = 1,  // gcc's
    AST_TYPE_SIZE_BOOL    = 1,  // _Bool
    AST_TYPE_SIZE_CHAR    = 1,  // char
    AST_TYPE_SIZE_SHORT   = 2,  // short
    AST_TYPE_SIZE_INT     = 4,  // int
    AST_TYPE_SIZE_LONG    = 8,  // long
    AST_TYPE_SIZE_LLONG   = 8,  // long long
    AST_TYPE_SIZE_FLOAT   = 4,  // float
    AST_TYPE_SIZE_DOUBLE  = 8,  // double
    AST_TYPE_SIZE_LDOUBLE = 16, // ten bytes of x87 extended, padded
    AST_TYPE_SIZE_PTR     = 8,  // any pointer
    AST_TYPE_SIZE_FUNC    = 1   // C gives a function no size; gcc answers 1
};

// The target ABI's alignments in bytes.
typedef enum Ast_TypeAlign Ast_TypeAlign;
enum Ast_TypeAlign {
    AST_TYPE_ALIGN_VOID    = 1,  // gcc's
    AST_TYPE_ALIGN_BOOL    = 1,  // _Bool
    AST_TYPE_ALIGN_CHAR    = 1,  // char
    AST_TYPE_ALIGN_SHORT   = 2,  // short
    AST_TYPE_ALIGN_INT     = 4,  // int
    AST_TYPE_ALIGN_LONG    = 8,  // long
    AST_TYPE_ALIGN_LLONG   = 8,  // long long
    AST_TYPE_ALIGN_FLOAT   = 4,  // float
    AST_TYPE_ALIGN_DOUBLE  = 8,  // double
    AST_TYPE_ALIGN_LDOUBLE = 16, // long double
    AST_TYPE_ALIGN_PTR     = 8,  // any pointer
    AST_TYPE_ALIGN_FUNC    = 1   // gcc's
};

// Whether a function's parameter list ended in `...`.
// [C99:6.7.5.3p9]: `, ...`, nothing said of the parameters after it
typedef enum Ast_TypeVa Ast_TypeVa;
enum Ast_TypeVa {
    AST_TYPE_FIXED, // no `...`
    AST_TYPE_VA     // ending in `...`
};

// Whether a function was declared with a prototype.
// [C99:6.7.5.3p14]: (), nothing said of the parameters
typedef enum Ast_TypeProto Ast_TypeProto;
enum Ast_TypeProto {
    AST_TYPE_NOPROTO, // written `int f()`
    AST_TYPE_PROTO    // written with its parameter types
};

// Whether an integer type holds negative values.
// [C99:6.2.5p6]: an unsigned type for each signed one
typedef enum Ast_TypeSign Ast_TypeSign;
enum Ast_TypeSign {
    AST_TYPE_SIGNED,  // holding negative values
    AST_TYPE_UNSIGNED // holding none
};

// The qualifiers a declaration may carry.
// [C99:6.7.3]: type qualifiers
typedef enum Ast_Qual Ast_Qual;
enum Ast_Qual {
    AST_QUAL_CONST    = 1, // const
    AST_QUAL_VOLATILE = 2, // volatile
    AST_QUAL_RESTRICT = 4  // restrict
};

// Whether a type's members have been seen.
// [C99:6.2.5p1]: object, function and incomplete types
typedef enum Ast_TypeComplete Ast_TypeComplete;
enum Ast_TypeComplete {
    AST_TYPE_INCOMPLETE, // members not yet seen
    AST_TYPE_COMPLETE    // members seen
};

// The kind of an AST node.
typedef enum Ast_NodeKind Ast_NodeKind;
enum Ast_NodeKind {
    AST_NODE_KIND_NUM,        // integer literal
    AST_NODE_KIND_FNUM,       // floating literal
    AST_NODE_KIND_STR,        // string literal
    AST_NODE_KIND_VAR,        // a reference to a local variable
    AST_NODE_KIND_ADD,        // lhs + rhs
    AST_NODE_KIND_SUB,        // lhs - rhs
    AST_NODE_KIND_MUL,        // lhs * rhs
    AST_NODE_KIND_DIV,        // lhs / rhs
    AST_NODE_KIND_MOD,        // lhs % rhs
    AST_NODE_KIND_NEG,        // -lhs
    AST_NODE_KIND_NOT,        // !lhs
    AST_NODE_KIND_BITNOT,     // ~lhs
    AST_NODE_KIND_BITAND,     // lhs & rhs
    AST_NODE_KIND_BITOR,      // lhs | rhs
    AST_NODE_KIND_BITXOR,     // lhs ^ rhs
    AST_NODE_KIND_SHL,        // lhs << rhs
    AST_NODE_KIND_SHR,        // lhs >> rhs, arithmetic on a signed operand
    AST_NODE_KIND_ADDR,       // &lhs
    AST_NODE_KIND_DEREF,      // *lhs
    AST_NODE_KIND_MEMBER,     // lhs.an_member
    AST_NODE_KIND_CAST,       // (type) lhs
    AST_NODE_KIND_SIZEOF,     // sizeof lhs
    AST_NODE_KIND_EQ,         // lhs == rhs
    AST_NODE_KIND_NE,         // lhs != rhs
    AST_NODE_KIND_LT,         // lhs <  rhs   (> is LT reversed)
    AST_NODE_KIND_LE,         // lhs <= rhs   (>= is LE reversed)
    AST_NODE_KIND_AND,        // lhs && rhs
    AST_NODE_KIND_OR,         // lhs || rhs
    AST_NODE_KIND_ASSIGN,     // lhs = rhs
    AST_NODE_KIND_OPASSIGN,   // lhs an_op= rhs
    AST_NODE_KIND_POSTINC,    // lhs++ or lhs--, stepping by an_step
    AST_NODE_KIND_COND,       // cond ? then : els
    AST_NODE_KIND_COMMA,      // lhs, rhs
    AST_NODE_KIND_INIT,       // an initializer item, or a flattened one
    AST_NODE_KIND_INITLIST,   // a braced initializer, its items in an_items
    AST_NODE_KIND_DESIGNATOR, // `[an_index]` or `.an_memname`, offsetof's too
    AST_NODE_KIND_ZERO,       // zero an_size bytes at the address an_lhs
    AST_NODE_KIND_COMPOUND,   // (type){...}
    AST_NODE_KIND_CALL,       // lhs(args), a FUNCADDR lhs when direct
    AST_NODE_KIND_FUNCADDR,   // a function named as a value
    AST_NODE_KIND_VA_START,   // __builtin_va_start(lhs, last)
    AST_NODE_KIND_VA_ARG,     // __builtin_va_arg(lhs, T)
    AST_NODE_KIND_FLT_ROUNDS, // __builtin_flt_rounds()
    AST_NODE_KIND_RETURN,     // return lhs;
    AST_NODE_KIND_IF,         // if (cond) then; else els;
    AST_NODE_KIND_FOR,        // for (init; cond; inc) body;
    AST_NODE_KIND_DO,         // do body; while (cond);
    AST_NODE_KIND_SWITCH,     // switch (cond) body;
    AST_NODE_KIND_CASE,       // case cond: body;
    AST_NODE_KIND_DEFAULT,    // default: body;
    AST_NODE_KIND_GOTO,       // goto an_labelname;
    AST_NODE_KIND_LABEL,      // an_labelname: body;
    AST_NODE_KIND_BREAK,      // break;
    AST_NODE_KIND_CONTINUE,   // continue;
    AST_NODE_KIND_BLOCK,      // { ... }
    AST_NODE_KIND_DECL,       // the statements one declaration becomes
    AST_NODE_KIND_VLA,        // allocate an_var, an array of an_lhs bytes
    AST_NODE_KIND_VSIZE,      // an_var = an_lhs elements of an_rhs bytes
    AST_NODE_KIND_EXPR_STMT,  // expression used as a statement
    AST_NODE_KIND_ASM,        // __asm__ (an_asm);
    AST_NODE_KIND_NOP,        // empty statement / bare declaration
    AST_NODE_KIND_COUNT       // number of kinds
};

// What a declaration's storage class asks for.
// [C99:6.2.4p1]: static and automatic storage durations
// [C99:6.7.1]:   storage-class specifiers
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
// [C99:6.7.2.1p10]: bit-fields packed into adjacent bits of a unit
// [C99:6.7.2.1p13]: a struct's members at increasing addresses, the first at its start
// [C99:6.7.2.1p14]: a union's members, each at its start
// [C99:6.7.2.1p15]: padding at the end of a struct or union
struct Ast_Member {
    Ast_Member *am_next;     // the member declared after it
    char       *am_name;     // NULL for a bitfield declared only to pad
    Ast_Type   *am_type;     // the member's type
    Ast_Type   *am_owner;    // aggregate the member was declared in
    int32_t     am_offset;   // bytes from the start of the enclosing aggregate
    Ast_Line    am_line;     // source line the member was declared on
    bool        am_flexible; // true for a trailing `d[]`
    int32_t     am_bits;     // width of a bitfield, or 0
    int32_t     am_bitoff;   // bits into am_offset where a bitfield starts
    int32_t     am_align;    // alignment `_Alignas` asked for, or 0
};

// An interned string literal.
// [C99:6.4.5p6]: identical string literals, possibly one array
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
    int32_t      av_align;      // alignment `_Alignas` asked for, or 0
};

// A struct, union or enum tag.
// [C99:6.7.2.3]: tags
typedef struct Ast_Tag Ast_Tag;
struct Ast_Tag {
    Ast_Tag  *ag_next; // the tag declared before it in its scope
    char     *ag_name; // the tag's name
    Ast_Type *ag_type; // the type it names
};

// An enumeration constant: an int that answers to a name.
// [C99:6.4.4.3p2]: an enumeration constant, of type int
typedef struct Ast_EnumConst Ast_EnumConst;
struct Ast_EnumConst {
    Ast_EnumConst *ae_next;  // the constant declared before it in its scope
    char          *ae_name;  // the constant's name
    int64_t        ae_value; // the int it stands for
};

// A name a typedef declaration bound to a type.
// [C99:6.7.7p3]: a typedef name, the type its declarator gives
typedef struct Ast_Typedef Ast_Typedef;
struct Ast_Typedef {
    Ast_Typedef *ad_next; // the typedef declared before it in its scope
    char        *ad_name; // the name bound
    Ast_Type    *ad_type; // the type it stands for
};

// The scope of one variably modified name, inside those declared before it.
typedef struct Ast_VmScope Ast_VmScope;
struct Ast_VmScope {
    Ast_VmScope *vs_outer; // the variably modified name declared before it
};

// One lexical scope: what was declared directly inside a pair of braces.
// [C99:6.2.1p4]: file, block and function prototype scopes
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
    Ast_NodeKind an_kind;       // which kind of node this is
    Ast_NodeKind an_op;         // operation of an OPASSIGN
    Ast_Type    *an_type;       // type of the value
    Ast_Line     an_line;       // source line the construct started on
    Ast_Node    *an_next;       // next node in a statement / argument list
    Ast_Node    *an_lhs;        // generic left operand
    Ast_Node    *an_rhs;        // generic right operand
    Ast_Node    *an_cond;       // test of an IF, loop, COND, SWITCH or CASE
    Ast_Node    *an_then;       // then branch of an IF or COND
    Ast_Node    *an_els;        // else branch of an IF or COND
    Ast_Node    *an_init;       // initializer of a FOR
    Ast_Node    *an_inc;        // increment of a FOR
    Ast_Node    *an_body;       // body of a BLOCK, DECL, loop, SWITCH or label
    char        *an_funcname;   // function a FUNCADDR names
    char        *an_labelname;  // label a GOTO jumps to or a LABEL defines
    Ast_Node    *an_target;     // LABEL a GOTO jumps to
    char        *an_asm;        // template of an ASM
    Ast_Node    *an_args;       // argument list of a CALL
    Ast_Node    *an_cases;      // cases of a SWITCH, in source order
    Ast_Node    *an_casenext;   // next case of the switch this one belongs to
    int32_t      an_label;      // label number a case is emitted with
    int64_t      an_val;        // value of a NUM or a CASE
    int64_t      an_step;       // amount a POSTINC steps by
    int64_t      an_index;      // array index a DESIGNATOR names
    int32_t      an_offset;     // byte offset an INIT stores at
    int32_t      an_size;       // bytes a ZERO clears
    int32_t      an_vladepth;   // variable-length arrays live at a LABEL
    long double  an_fval;       // value of an FNUM
    size_t       an_stridx;     // string table slot of a STR
    Ast_Var     *an_var;        // variable a VAR names or a COMPOUND fills
    Ast_Node    *an_items;      // items of an INITLIST, or a COMPOUND flattened
    Ast_Node    *an_desig;      // designators aiming an INIT item
    Ast_Member  *an_member;     // resolved member of a MEMBER
    Ast_Member  *an_bitfield;   // bit-field a flattened INIT stores into
    char        *an_memname;    // member name a MEMBER node was written with
    int32_t      an_tmp;        // frame slot an aggregate return lands in
    int32_t      an_calltmp;    // slot an indirect CALL parks its callee in
    Ast_VmScope *an_vm;         // variably modified scopes of a jump or label
    bool         an_initstore;  // an ASSIGN that initializes, not assigns
};

// A function definition.
typedef struct Ast_Func Ast_Func;
struct Ast_Func {
    Ast_Func *af_next;       // next function in the program
    char     *af_name;       // function name
    Ast_Node *af_body;       // function body (AST_NODE_KIND_BLOCK)
    Ast_Type *af_type;       // the FUNC type it was declared or defined with
    bool      af_static;     // true when the function is local to this file
    bool      af_external;   // true when a declaration makes it external
    Ast_Var  *af_locals;     // every local, including parameters
    int32_t   af_stack_size; // frame size
};

// The finished program, produced by the parser.
extern Ast_Func *Ast_Program;

// Every variable declared at file scope.
extern Ast_Var *Ast_Globals;

// Every asm written at file scope.
extern Ast_Node *Ast_FileAsms;

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
// [C99:6.2.5p20]:  derived types
// [C99:6.2.7p1]:   compatible types
// [C99:6.7.2.1p2]: no incomplete or function member, a flexible array last
// [C99:6.7.2.1p7]: a struct or union without a named member, undefined
// [C11:6.7.5p6]:   a member's alignment, the strictest specified
int32_t     Ast_AlignTo(int32_t n, int32_t align);
int32_t     Ast_AlignDown(int32_t n, int32_t align);
Ast_Type   *Ast_IntegerType(Ast_TypeKind kind, Ast_TypeSign sign);
Ast_Type   *Ast_Qualify(Ast_Type *type, Ast_Qual qual);
bool        Ast_IsInteger(const Ast_Type *type);
bool        Ast_IsFloating(const Ast_Type *type);
bool        Ast_IsArithmetic(const Ast_Type *type);
Ast_Type   *Ast_NewPointer(Ast_Type *base);
Ast_Type   *Ast_NewArray(Ast_Type *base, int32_t len);
Ast_Type   *Ast_NewUnsizedArray(Ast_Type *base);
bool        Ast_IsUnsized(const Ast_Type *type);
Ast_Type   *Ast_SizeArray(const Ast_Type *type, int32_t len);
Ast_Type   *Ast_NewVla(Ast_Type *base, Ast_Node *len);
bool        Ast_IsVla(const Ast_Type *type);
bool        Ast_IsVm(const Ast_Type *type);
Ast_Type   *Ast_NewFunction(Ast_Type *ret, Ast_Var *params, int32_t nparams, Ast_TypeVa va, Ast_TypeProto proto);
Ast_Type   *Ast_NewAggregate(Ast_TypeKind kind, const char *tag);
Ast_Member *Ast_NewMember(const char *name, Ast_Type *type, Ast_Line line);
int32_t     Ast_PlaceBitfield(Ast_Member *member, int32_t bits);
Ast_Member *Ast_NamedMembers(Ast_Member *members);
int32_t     Ast_MemberAlign(const Ast_Member *member);
void        Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, Ast_Line line);
Ast_Member *Ast_FindMember(const Ast_Type *type, const char *name);
bool        Ast_IsCompatible(const Ast_Type *a, const Ast_Type *b);
bool        Ast_IsCompatibleUnqualified(const Ast_Type *a, const Ast_Type *b);
bool        Ast_IsCompatibleParams(const Ast_Type *a, const Ast_Type *b);

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
void      Ast_AddFileAsm(Ast_Node *node);

// Variable scopes
// [C99:6.2.1p4]: file, block and function prototype scopes
void         Ast_BeginScope(void);
void         Ast_EndScope(void);
void         Ast_PushScope(void);
void         Ast_PopScope(void);
Ast_Var     *Ast_FindVar(const char *name);
Ast_Scope   *Ast_SharedScope(void);
Ast_Var     *Ast_FindVarHere(const char *name);
Ast_Var     *Ast_FindGlobal(const char *symbol);
Ast_Func    *Ast_FindFunction(const char *name);
Ast_Var     *Ast_DeclareVar(const char *name, Ast_Type *type, Ast_Line line);
void         Ast_DeclareParam(Ast_Var *var);
void         Ast_DeclarePrototypeParam(Ast_Var *var);
Ast_Var     *Ast_DeclareGlobal(const char *name, Ast_Type *type, Ast_Line line);
Ast_Var     *Ast_DeclareStaticLocal(const char *name, const char *symbol, Ast_Type *type, Ast_Line line);
Ast_Var     *Ast_DeclareExternLocal(const char *name, Ast_Type *type, Ast_Line line);
Ast_Var     *Ast_CurrentLocals(void);
void         Ast_OpenVmScope(void);
Ast_VmScope *Ast_CurrentVmScope(void);
bool         Ast_ContainsVmScope(const Ast_VmScope *scope, const Ast_VmScope *outer);

// Tags and typedef names
// [C99:6.2.3p1]: name spaces for labels, tags, members and other names
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
// [C99:6.4.5p6]: identical string literals, possibly one array
size_t   Ast_AddString(char *s, size_t len, size_t width);
size_t   Ast_StringCount(void);
Ast_Str *Ast_StringAt(size_t idx);

#endif // AST_H
