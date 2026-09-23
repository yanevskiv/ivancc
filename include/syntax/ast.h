// C header file for the abstract syntax tree.

#ifndef AST_H
#define AST_H

// Maximum number of distinct string literals in one translation unit.
#define AST_MAX_STRINGS 1024

// Bits in a byte, for placing a bitfield inside the unit that holds it.
#define AST_BITS_PER_BYTE 8

// The kind of a type.
typedef enum Ast_TypeKind Ast_TypeKind;
enum Ast_TypeKind {
    AST_TYPE_KIND_VOID,
    AST_TYPE_KIND_CHAR,
    AST_TYPE_KIND_INT,
    AST_TYPE_KIND_PTR,
    AST_TYPE_KIND_ARRAY,
    AST_TYPE_KIND_FUNC,
    AST_TYPE_KIND_STRUCT,
    AST_TYPE_KIND_UNION
};

// The target ABI's sizes in bytes; C does not define void's.
typedef enum Ast_TypeSize Ast_TypeSize;
enum Ast_TypeSize {
    AST_TYPE_SIZE_VOID = 1,
    AST_TYPE_SIZE_CHAR = 1,
    AST_TYPE_SIZE_INT  = 4,
    AST_TYPE_SIZE_PTR  = 8,
    AST_TYPE_SIZE_FUNC = 1   // C gives a function no size; gcc answers 1
};

// The target ABI's alignments in bytes.
typedef enum Ast_TypeAlign Ast_TypeAlign;
enum Ast_TypeAlign {
    AST_TYPE_ALIGN_VOID = 1,
    AST_TYPE_ALIGN_CHAR = 1,
    AST_TYPE_ALIGN_INT  = 4,
    AST_TYPE_ALIGN_PTR  = 8,
    AST_TYPE_ALIGN_FUNC = 1
};

// Forward declaration: a struct type lists its members.
typedef struct Ast_Member Ast_Member;

// Forward declaration: a function type lists its parameters.
typedef struct Ast_Var Ast_Var;

// A C type: a primitive, or a pointer, array or aggregate built over others.
typedef struct Ast_Type Ast_Type;
struct Ast_Type {
    Ast_TypeKind at_kind;
    int          at_size;    // bytes an object of this type occupies
    int          at_align;   // address multiple an object must sit on
    Ast_Type    *at_base;    // pointee for PTR, element type for ARRAY
    int          at_len;     // element count for ARRAY
    char        *at_tag;     // tag a STRUCT or UNION was declared with, or NULL
    Ast_Member  *at_members; // members of a STRUCT or UNION, in declaration order
    int          at_complete; // false until the member list has been seen
    Ast_Type    *at_ret;     // return type of a FUNC
    Ast_Var     *at_params;  // parameters of a FUNC, in declaration order
    int          at_nparams; // number of parameters a FUNC declares
    int          at_variadic; // true when a FUNC's parameter list ended in `...`
    int          at_proto;   // false for `int f()`, whose parameter list is unspecified
};

// One member of a struct or union, at the offset layout gave it.
struct Ast_Member {
    Ast_Member *am_next;
    char       *am_name;     // NULL for a bitfield declared only to pad
    Ast_Type   *am_type;
    Ast_Type   *am_owner;    // aggregate the member was declared in
    int         am_offset;   // bytes from the start of the enclosing aggregate
    int         am_line;     // source line the member was declared on
    int         am_flexible; // true for a trailing `d[]`, which takes no space
    int         am_bits;     // width of a bitfield, or 0 when it is not one
    int         am_bitoff;   // bits into am_offset where a bitfield starts
};

// An interned string literal, kept with its length because it may embed a NUL.
typedef struct Ast_Str Ast_Str;
struct Ast_Str {
    char *as_data; // decoded bytes, also NUL-terminated so it can be printed
    int   as_len;  // number of bytes before that terminator
};

// The kind of an AST node.
typedef enum Ast_NodeKind Ast_NodeKind;
enum Ast_NodeKind {
    AST_NODE_KIND_NUM,       // integer literal
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
    AST_NODE_KIND_MEMBER,    // lhs.an_member, with `a->b` parsed as `(*a).b`
    AST_NODE_KIND_CAST,      // (type) lhs
    AST_NODE_KIND_SIZEOF,    // sizeof lhs, folded to a constant by the Sem_ pass
    AST_NODE_KIND_EQ,        // lhs == rhs
    AST_NODE_KIND_NE,        // lhs != rhs
    AST_NODE_KIND_LT,        // lhs <  rhs   (> is LT reversed)
    AST_NODE_KIND_LE,        // lhs <= rhs   (>= is LE reversed)
    AST_NODE_KIND_AND,       // lhs && rhs
    AST_NODE_KIND_OR,        // lhs || rhs
    AST_NODE_KIND_ASSIGN,    // lhs = rhs
    AST_NODE_KIND_OPASSIGN,  // lhs an_op= rhs, with the address evaluated once
    AST_NODE_KIND_POSTINC,   // lhs++ or lhs--, stepping by an_val
    AST_NODE_KIND_COND,      // cond ? then : els
    AST_NODE_KIND_COMMA,     // lhs, rhs
    AST_NODE_KIND_INIT,      // one flattened initializer: an_val is a byte offset into the object
    AST_NODE_KIND_INITLIST,  // a braced initializer list, its items chained on an_body
    AST_NODE_KIND_DESIGNATOR,// `[an_val]` or `.an_memname` naming where an item lands
    AST_NODE_KIND_ZERO,      // zero an_val bytes of the object an_lhs addresses
    AST_NODE_KIND_COMPOUND,  // (type){...}: the unnamed an_var object the an_body statements fill
    AST_NODE_KIND_CALL,      // function call, direct by name or indirect through an_lhs
    AST_NODE_KIND_FUNCADDR,  // a function named as a value, which is its address
    AST_NODE_KIND_VA_START,  // __builtin_va_start(lhs, last), which fills the lhs va_list
    AST_NODE_KIND_VA_ARG,    // __builtin_va_arg(lhs, T), the next argument the lhs va_list reaches
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
    AST_NODE_KIND_EXPR_STMT, // expression used as a statement
    AST_NODE_KIND_NOP        // empty statement / bare declaration
};

// What a declaration's storage class asks for; register, auto and inline map to NONE.
typedef enum Ast_Storage Ast_Storage;
enum Ast_Storage {
    AST_STORAGE_NONE,
    AST_STORAGE_STATIC,  // visible only to this translation unit
    AST_STORAGE_EXTERN,  // declared here, defined elsewhere
    AST_STORAGE_TYPEDEF  // binds a name to a type rather than declaring an object
};

// Forward declaration: a global's initializer is one of these.
typedef struct Ast_Node Ast_Node;

// A local variable or function parameter.
struct Ast_Var {
    Ast_Var *av_next;       // chains every local in a function
    Ast_Var *av_param_next; // chains parameters in declaration order
    Ast_Var *av_scope_next; // chains the variables of one lexical scope
    char     *av_name;      // identifier as written in the source
    Ast_Type *av_type;      // declared type
    int      av_line;       // source line the declaration appeared on
    int      av_offset;     // offset from %rbp, filled in by the back end
    char     *av_symbol;    // name the symbol takes, which a static local mangles
    int       av_global;    // true when the variable lives in .data or .bss
    Ast_Storage av_storage; // storage class the declaration asked for
    Ast_Node *av_init;      // initializer of a global, or NULL for zeroed
};

// A struct, union or enum tag, which lives in a namespace of its own.
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
    long           ae_value;
};

// A name a typedef declaration bound to a type.
typedef struct Ast_Typedef Ast_Typedef;
struct Ast_Typedef {
    Ast_Typedef *ad_next;
    char        *ad_name;
    Ast_Type    *ad_type;
};

// One lexical scope: what was declared directly inside a pair of braces.
typedef struct Ast_Scope Ast_Scope;
struct Ast_Scope {
    Ast_Scope   *as_parent;   // the scope this one is nested in
    Ast_Var     *as_vars;     // declared here, innermost names first
    Ast_Tag     *as_tags;     // struct, union and enum tags declared here
    Ast_Typedef *as_typedefs; // typedef names declared here
    Ast_EnumConst *as_enums;  // enumeration constants declared here
};

// A node in the abstract syntax tree.
struct Ast_Node {
    Ast_NodeKind an_kind;     // which kind of node this is
    Ast_NodeKind an_op;       // operation of AST_NODE_KIND_OPASSIGN
    Ast_Type    *an_type;     // type of the value, filled in by the Sem_ pass
    int          an_line;     // source line the construct started on
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
    int          an_label;    // label number a case is emitted with
    long         an_val;      // integer value for AST_NODE_KIND_NUM
    int          an_str_idx;  // string table slot for AST_NODE_KIND_STR
    Ast_Var     *an_var;      // variable a VAR names, or the object a COMPOUND fills
    Ast_Node    *an_items;    // flattened initializer a COMPOUND fills that object with
    Ast_Member  *an_member;   // resolved member of AST_NODE_KIND_MEMBER
    char        *an_memname;  // member name a MEMBER node was written with
    int          an_tmp;      // frame slot a CALL returning an aggregate lands in
    int          an_calltmp;  // frame slot an indirect CALL parks its callee address in
};

// A function definition.
typedef struct Ast_Func Ast_Func;
struct Ast_Func {
    Ast_Func *af_next;       // next function in the program
    char     *af_name;       // function name
    Ast_Node *af_body;       // function body (AST_NODE_KIND_BLOCK)
    Ast_Type *af_ret;        // type the function returns
    Ast_Var  *af_params;     // parameters, in declaration order
    int       af_nparams;    // number of parameters
    int       af_variadic;   // true if the parameter list ended in `...`
    int       af_proto;      // false for `int f()` and an old-style definition, which promise nothing
    int       af_static;     // true when the function is local to this file
    Ast_Var  *af_locals;     // every local, including parameters
    int       af_stack_size; // frame size, filled in by the code generator
};

// The incomplete type, which only a pointer or a return type may name.
extern Ast_Type Ast_TypeVoid;

// The byte, which is what a string literal is an array of.
extern Ast_Type Ast_TypeChar;

// The default arithmetic type, which every integer literal and every promotion lands on.
extern Ast_Type Ast_TypeInt;

// The finished program, produced by the parser.
extern Ast_Func *Ast_Program;

// Every variable declared at file scope, in declaration order.
extern Ast_Var *Ast_Globals;

// Type construction
int       Ast_AlignTo(int n, int align);
int       Ast_AlignDown(int n, int align);
Ast_Type *Ast_NewPointer(Ast_Type *base);
Ast_Type *Ast_NewArray(Ast_Type *base, int len);
Ast_Type *Ast_NewFunction(Ast_Type *ret, Ast_Var *params, int nparams, int variadic, int proto);
Ast_Type *Ast_NewAggregate(Ast_TypeKind kind, const char *tag);
Ast_Member *Ast_NewMember(const char *name, Ast_Type *type, int line);
int       Ast_PlaceBitfield(Ast_Member *member, int bits);
Ast_Member *Ast_NamedMembers(Ast_Member *members);
void      Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, int line);
Ast_Member *Ast_FindMember(const Ast_Type *type, const char *name);

// Node construction
Ast_Node *Ast_NewNode(Ast_NodeKind kind, int line);
Ast_Node *Ast_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, int line);
Ast_Node *Ast_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, int line);
Ast_Node *Ast_NewNum(long val, int line);
Ast_Node *Ast_NewVarNode(Ast_Var *var, int line);
Ast_Node *Ast_NewOpAssign(Ast_NodeKind op, Ast_Node *lhs, Ast_Node *rhs, int line);
Ast_Node *Ast_NewPostInc(Ast_Node *lhs, long step, int line);
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, int line);

// Variable scopes
void     Ast_BeginScope(void);
void     Ast_EndScope(void);
void     Ast_PushScope(void);
void     Ast_PopScope(void);
Ast_Var *Ast_FindVar(const char *name);
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, int line);
void     Ast_DeclareParam(Ast_Var *var);
Ast_Var *Ast_DeclareGlobal(const char *name, Ast_Type *type, int line);
Ast_Var *Ast_DeclareStaticLocal(const char *name, const char *symbol, Ast_Type *type, int line);
Ast_Var *Ast_CurrentLocals(void);

// Tags and typedef names
Ast_Type *Ast_FindTag(const char *name);
Ast_Type *Ast_FindTagHere(const char *name);
void      Ast_DeclareTag(const char *name, Ast_Type *type);
Ast_Type *Ast_FindTypedef(const char *name);
void      Ast_DeclareTypedef(const char *name, Ast_Type *type);
int       Ast_FindEnumConst(const char *name, long *value);
void      Ast_DeclareEnumConst(const char *name, long value);

// String literal interning
int      Ast_AddString(char *s, int len);
int      Ast_StringCount(void);
Ast_Str *Ast_StringAt(int idx);

#endif // AST_H
