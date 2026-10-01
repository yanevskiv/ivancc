/*
 * C source file for the abstract syntax tree.
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

// Module header.
#include "lang/ast.h"

// The finished program, filled in by the parser.
Ast_Func *Ast_Program;

// Every variable declared at file scope.
Ast_Var *Ast_Globals;

// Every asm written at file scope.
Ast_Node *Ast_FileAsms;

// The incomplete type.
Ast_Type Ast_TypeVoid = {
    .at_kind     = AST_TYPE_KIND_VOID,
    .at_size     = AST_TYPE_SIZE_VOID,
    .at_align    = AST_TYPE_ALIGN_VOID,
    .at_complete = AST_TYPE_COMPLETE
};

// The type every conversion narrows to 0 or 1.
Ast_Type Ast_TypeBool = {
    .at_kind     = AST_TYPE_KIND_BOOL,
    .at_size     = AST_TYPE_SIZE_BOOL,
    .at_align    = AST_TYPE_ALIGN_BOOL,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The byte.
Ast_Type Ast_TypeChar = {
    .at_kind     = AST_TYPE_KIND_CHAR,
    .at_size     = AST_TYPE_SIZE_CHAR,
    .at_align    = AST_TYPE_ALIGN_CHAR,
    .at_sign     = AST_TYPE_SIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The unsigned byte.
Ast_Type Ast_TypeUChar = {
    .at_kind     = AST_TYPE_KIND_CHAR,
    .at_size     = AST_TYPE_SIZE_CHAR,
    .at_align    = AST_TYPE_ALIGN_CHAR,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The two-byte integer.
Ast_Type Ast_TypeShort = {
    .at_kind     = AST_TYPE_KIND_SHORT,
    .at_size     = AST_TYPE_SIZE_SHORT,
    .at_align    = AST_TYPE_ALIGN_SHORT,
    .at_sign     = AST_TYPE_SIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The unsigned two-byte integer.
Ast_Type Ast_TypeUShort = {
    .at_kind     = AST_TYPE_KIND_SHORT,
    .at_size     = AST_TYPE_SIZE_SHORT,
    .at_align    = AST_TYPE_ALIGN_SHORT,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The default arithmetic type.
Ast_Type Ast_TypeInt = {
    .at_kind     = AST_TYPE_KIND_INT,
    .at_size     = AST_TYPE_SIZE_INT,
    .at_align    = AST_TYPE_ALIGN_INT,
    .at_sign     = AST_TYPE_SIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The unsigned default arithmetic type.
Ast_Type Ast_TypeUInt = {
    .at_kind     = AST_TYPE_KIND_INT,
    .at_size     = AST_TYPE_SIZE_INT,
    .at_align    = AST_TYPE_ALIGN_INT,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The integer as wide as a pointer.
Ast_Type Ast_TypeLong = {
    .at_kind     = AST_TYPE_KIND_LONG,
    .at_size     = AST_TYPE_SIZE_LONG,
    .at_align    = AST_TYPE_ALIGN_LONG,
    .at_sign     = AST_TYPE_SIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The unsigned integer as wide as a pointer.
Ast_Type Ast_TypeULong = {
    .at_kind     = AST_TYPE_KIND_LONG,
    .at_size     = AST_TYPE_SIZE_LONG,
    .at_align    = AST_TYPE_ALIGN_LONG,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The widest signed integer.
Ast_Type Ast_TypeLLong = {
    .at_kind     = AST_TYPE_KIND_LLONG,
    .at_size     = AST_TYPE_SIZE_LLONG,
    .at_align    = AST_TYPE_ALIGN_LLONG,
    .at_sign     = AST_TYPE_SIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The widest integer.
Ast_Type Ast_TypeULLong = {
    .at_kind     = AST_TYPE_KIND_LLONG,
    .at_size     = AST_TYPE_SIZE_LLONG,
    .at_align    = AST_TYPE_ALIGN_LLONG,
    .at_sign     = AST_TYPE_UNSIGNED,
    .at_complete = AST_TYPE_COMPLETE
};

// The IEEE binary32 type.
Ast_Type Ast_TypeFloat = {
    .at_kind     = AST_TYPE_KIND_FLOAT,
    .at_size     = AST_TYPE_SIZE_FLOAT,
    .at_align    = AST_TYPE_ALIGN_FLOAT,
    .at_complete = AST_TYPE_COMPLETE
};

// The IEEE binary64 type.
Ast_Type Ast_TypeDouble = {
    .at_kind     = AST_TYPE_KIND_DOUBLE,
    .at_size     = AST_TYPE_SIZE_DOUBLE,
    .at_align    = AST_TYPE_ALIGN_DOUBLE,
    .at_complete = AST_TYPE_COMPLETE
};

// The x87 80-bit extended type.
Ast_Type Ast_TypeLDouble = {
    .at_kind     = AST_TYPE_KIND_LDOUBLE,
    .at_size     = AST_TYPE_SIZE_LDOUBLE,
    .at_align    = AST_TYPE_ALIGN_LDOUBLE,
    .at_complete = AST_TYPE_COMPLETE
};

// Every integer type.
static Ast_Type *Ast_IntTypes[AST_TYPE_KIND_COUNT][2] = {
    [AST_TYPE_KIND_BOOL]  = {&Ast_TypeBool,  &Ast_TypeBool},
    [AST_TYPE_KIND_CHAR]  = {&Ast_TypeChar,  &Ast_TypeUChar},
    [AST_TYPE_KIND_SHORT] = {&Ast_TypeShort, &Ast_TypeUShort},
    [AST_TYPE_KIND_INT]   = {&Ast_TypeInt,   &Ast_TypeUInt},
    [AST_TYPE_KIND_LONG]  = {&Ast_TypeLong,  &Ast_TypeULong},
    [AST_TYPE_KIND_LLONG] = {&Ast_TypeLLong, &Ast_TypeULLong}
};

// Table of interned string literals.
static Ast_Str *Ast_Strings;

// Number of entries currently used in Ast_Strings.
static size_t Ast_NumStrings;

// Number of entries Ast_Strings has room for.
static size_t Ast_CapStrings;

// The last global declared.
static Ast_Var *Ast_GlobalsTail;

// The last asm written at file scope.
static Ast_Node *Ast_FileAsmsTail;

// The outermost scope.
static Ast_Scope Ast_FileScope;

// The innermost scope currently open.
static Ast_Scope *Ast_CurScope = &Ast_FileScope;

// Locals of the function currently being parsed.
static Ast_Var *Ast_Locals;

// Round n up to the nearest multiple of align.
int32_t Ast_AlignTo(int32_t n, int32_t align)
{
    return (n + align - 1) / align * align;
}

// Round n down to the multiple of align at or below it.
int32_t Ast_AlignDown(int32_t n, int32_t align)
{
    return n / align * align;
}

// Return the shared type of that kind and signedness.
Ast_Type *Ast_IntegerType(Ast_TypeKind kind, Ast_TypeSign sign)
{
    return Ast_IntTypes[kind][sign];
}

// Return the type carrying those qualifiers.
Ast_Type *Ast_Qualify(Ast_Type *type, Ast_Qual qual)
{
    if (! qual || type->at_qual == qual) {
        return type;
    }

    Ast_Type *copy = calloc(1, sizeof(Ast_Type));
    *copy = *type;
    copy->at_qual = qual;
    return copy;
}

// Return whether this type is an integer type.
bool Ast_IsInteger(const Ast_Type *type)
{
    return type->at_kind >= AST_TYPE_KIND_FIRST_INT && type->at_kind <= AST_TYPE_KIND_LAST_INT;
}

// Return whether this type is a floating type.
bool Ast_IsFloating(const Ast_Type *type)
{
    return type->at_kind >= AST_TYPE_KIND_FIRST_FLOAT && type->at_kind <= AST_TYPE_KIND_LAST_FLOAT;
}

// Return whether this type is an integer or a floating type.
bool Ast_IsArithmetic(const Ast_Type *type)
{
    return Ast_IsInteger(type) || Ast_IsFloating(type);
}

// Build the pointer type that points at base.
Ast_Type *Ast_NewPointer(Ast_Type *base)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind     = AST_TYPE_KIND_PTR;
    type->at_size     = AST_TYPE_SIZE_PTR;
    type->at_align    = AST_TYPE_ALIGN_PTR;
    type->at_base     = base;
    type->at_complete = AST_TYPE_COMPLETE;
    return type;
}

// Build the type of an array of len elements of base.
Ast_Type *Ast_NewArray(Ast_Type *base, int32_t len)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind     = AST_TYPE_KIND_ARRAY;
    type->at_size     = base->at_size * len;
    type->at_align    = base->at_align;
    type->at_base     = base;
    type->at_len      = len;
    type->at_complete = base->at_complete;
    return type;
}

// Build the type of an array of base whose length is not yet known.
Ast_Type *Ast_NewUnsizedArray(Ast_Type *base)
{
    Ast_Type *type = Ast_NewArray(base, 0);
    type->at_complete = AST_TYPE_INCOMPLETE;
    return type;
}

// Return whether this type is an array still waiting for its length.
bool Ast_IsUnsized(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_ARRAY && ! type->at_complete && type->at_base->at_complete;
}

// Return an unsized array's type with len elements.
Ast_Type *Ast_SizeArray(const Ast_Type *type, int32_t len)
{
    return Ast_Qualify(Ast_NewArray(type->at_base, len), type->at_qual);
}

// Build the type of an array of base whose length len computes at run time.
Ast_Type *Ast_NewVla(Ast_Type *base, Ast_Node *len)
{
    Ast_Type *type = Ast_NewArray(base, 0);
    type->at_vlen = len;
    return type;
}

// Return whether this type is a variable-length array.
bool Ast_IsVla(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_ARRAY && type->at_vlen != NULL;
}

// Return whether this type has a variable-length array anywhere in it.
bool Ast_IsVm(const Ast_Type *type)
{
    if (Ast_IsVla(type)) {
        return true;
    }
    if (type->at_kind != AST_TYPE_KIND_PTR && type->at_kind != AST_TYPE_KIND_ARRAY) {
        return false;
    }
    return Ast_IsVm(type->at_base);
}

// Build a function type.
Ast_Type *Ast_NewFunction(Ast_Type *ret, Ast_Var *params, int32_t nparams, Ast_TypeVa va, Ast_TypeProto proto)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind     = AST_TYPE_KIND_FUNC;
    type->at_size     = AST_TYPE_SIZE_FUNC;
    type->at_align    = AST_TYPE_ALIGN_FUNC;
    type->at_ret      = ret;
    type->at_params   = params;
    type->at_nparams  = nparams;
    type->at_va       = va;
    type->at_proto    = proto;
    type->at_complete = AST_TYPE_COMPLETE;
    return type;
}

// Build an empty struct or union type.
Ast_Type *Ast_NewAggregate(Ast_TypeKind kind, const char *tag)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind  = kind;
    type->at_align = 1;
    type->at_tag   = Str_Clone(tag);
    return type;
}

// Build one member of a struct or union.
Ast_Member *Ast_NewMember(const char *name, Ast_Type *type, Ast_Line line)
{
    Ast_Member *member = calloc(1, sizeof(Ast_Member));
    member->am_name = Str_Clone(name);
    member->am_type = type;
    member->am_line = line;
    return member;
}

// Place one bitfield at the bit cursor and return where the next one starts.
int32_t Ast_PlaceBitfield(Ast_Member *member, int32_t bits)
{
    int32_t unit = member->am_type->at_size * AST_BITS_PER_BYTE;

    if (member->am_bits == 0) {
        return Ast_AlignTo(bits, unit);
    }
    if (bits / unit != (bits + member->am_bits - 1) / unit) {
        bits = Ast_AlignTo(bits, unit);
    }
    member->am_offset = Ast_AlignDown(bits / AST_BITS_PER_BYTE, member->am_type->at_size);
    member->am_bitoff = bits % unit;
    return bits + member->am_bits;
}

// Drop the members no name can reach.
Ast_Member *Ast_NamedMembers(Ast_Member *members)
{
    Ast_Member head = {0};
    Ast_Member *last = &head;

    for (Ast_Member *member = members; member; member = member->am_next) {
        if (member->am_name) {
            last->am_next = member;
            last = member;
        }
    }
    last->am_next = NULL;
    return head.am_next;
}

// Lay out an aggregate's members and size it.
void Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, Ast_Line line)
{
    int32_t bits = 0;
    int32_t align = 1;

    for (Ast_Member *member = members; member; member = member->am_next) {
        member->am_owner = type;
        if (member->am_flexible) {
            Err_AssertAt(member->am_line, type->at_kind != AST_TYPE_KIND_UNION, ERR_AST_FLEXIBLE_IN_UNION);
            Err_AssertAt(member->am_line, ! member->am_next, ERR_AST_FLEXIBLE_NOT_LAST, member->am_name);
            Err_AssertAt(member->am_line, member != members, ERR_AST_FLEXIBLE_ALONE);
            member->am_offset = Ast_AlignTo(bits, member->am_type->at_align * AST_BITS_PER_BYTE) / AST_BITS_PER_BYTE;
            if (member->am_type->at_align > align) {
                align = member->am_type->at_align;
            }
            continue;
        }
        Err_AssertAt(member->am_line, member->am_type->at_complete, ERR_AST_MEMBER_NOT_COMPLETE, member->am_name);
        for (Ast_Member *seen = members; seen != member; seen = seen->am_next) {
            Err_AssertAt(member->am_line, ! member->am_name || ! Str_Equals(seen->am_name, member->am_name), ERR_AST_MEMBER_DUPLICATE, member->am_name);
        }
        if (member->am_type->at_align > align) {
            align = member->am_type->at_align;
        }
        if (type->at_kind == AST_TYPE_KIND_UNION) {
            member->am_offset = 0;
            if (member->am_type->at_size * AST_BITS_PER_BYTE > bits) {
                bits = member->am_type->at_size * AST_BITS_PER_BYTE;
            }
            continue;
        }
        if (member->am_bits || ! member->am_name) {
            bits = Ast_PlaceBitfield(member, bits);
            continue;
        }
        bits = Ast_AlignTo(bits, member->am_type->at_align * AST_BITS_PER_BYTE);
        member->am_offset = bits / AST_BITS_PER_BYTE;
        bits += member->am_type->at_size * AST_BITS_PER_BYTE;
    }

    Ast_Member *named = Ast_NamedMembers(members);
    Err_AssertAt(line, named, ERR_AST_AGGREGATE_NOT_NAMED);

    type->at_members  = named;
    type->at_complete = AST_TYPE_COMPLETE;
    type->at_align    = align;
    type->at_size     = Ast_AlignTo(bits, align * AST_BITS_PER_BYTE) / AST_BITS_PER_BYTE;
}

// Return the named member of an aggregate.
Ast_Member *Ast_FindMember(const Ast_Type *type, const char *name)
{
    for (Ast_Member *member = type->at_members; member; member = member->am_next) {
        if (Str_Equals(member->am_name, name)) {
            return member;
        }
    }
    return NULL;
}

// Return whether two types are compatible.
bool Ast_IsCompatible(const Ast_Type *a, const Ast_Type *b)
{
    return a->at_qual == b->at_qual && Ast_IsCompatibleUnqualified(a, b);
}

// Return whether two types are compatible but for their outermost qualifiers.
bool Ast_IsCompatibleUnqualified(const Ast_Type *a, const Ast_Type *b)
{
    if (a == b) {
        return true;
    }
    if (a->at_kind != b->at_kind) {
        return false;
    }

    switch (a->at_kind) {
        case AST_TYPE_KIND_PTR: {
            return Ast_IsCompatible(a->at_base, b->at_base);
        } break;
        case AST_TYPE_KIND_ARRAY: {
            bool fixed = ! Ast_IsUnsized(a) && ! Ast_IsUnsized(b) && ! Ast_IsVla(a) && ! Ast_IsVla(b);
            return Ast_IsCompatible(a->at_base, b->at_base) && (! fixed || a->at_len == b->at_len);
        } break;
        case AST_TYPE_KIND_FUNC: {
            return Ast_IsCompatible(a->at_ret, b->at_ret) && Ast_IsCompatibleParams(a, b);
        } break;
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_UNION: {
            bool tags = Str_Equals(a->at_tag, b->at_tag);
            bool open = ! a->at_complete || ! b->at_complete;
            return tags && (open || a->at_members == b->at_members);
        } break;
        default: {
            return a->at_sign == b->at_sign;
        }
    }
}

// Return whether two function types' parameter lists are compatible.
bool Ast_IsCompatibleParams(const Ast_Type *a, const Ast_Type *b)
{
    const Ast_Var *pb = b->at_params;

    if (a->at_proto != AST_TYPE_PROTO || b->at_proto != AST_TYPE_PROTO) {
        return true;
    }
    if (a->at_nparams != b->at_nparams || a->at_va != b->at_va) {
        return false;
    }
    for (const Ast_Var *pa = a->at_params; pa && pb; pa = pa->av_param_next, pb = pb->av_param_next) {
        if (! Ast_IsCompatibleUnqualified(pa->av_type, pb->av_type)) {
            return false;
        }
    }
    return true;
}

// Allocate a zeroed node of the given kind.
Ast_Node *Ast_NewNode(Ast_NodeKind kind, Ast_Line line)
{
    Ast_Node *node = calloc(1, sizeof(Ast_Node));
    node->an_kind = kind;
    node->an_line = line;
    return node;
}

// Build a binary-operator node with the given operands.
Ast_Node *Ast_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(kind, line);
    node->an_lhs = lhs;
    node->an_rhs = rhs;
    return node;
}

// Build a unary-operator node with the given operand.
Ast_Node *Ast_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(kind, line);
    node->an_lhs = lhs;
    return node;
}

// Build an integer-literal node.
Ast_Node *Ast_NewNum(int64_t val, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_NUM, line);
    node->an_val = val;
    return node;
}

// Build a floating-literal node of the given type.
Ast_Node *Ast_NewFNum(long double val, Ast_Type *type, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_FNUM, line);
    node->an_fval = val;
    node->an_type = type;
    return node;
}

// Build a node that references a local variable.
Ast_Node *Ast_NewVarNode(Ast_Var *var, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_VAR, line);
    node->an_var = var;
    return node;
}

// Build a compound assignment.
Ast_Node *Ast_NewOpAssign(Ast_NodeKind op, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line)
{
    Ast_Node *node = Ast_NewBinary(AST_NODE_KIND_OPASSIGN, lhs, rhs, line);
    node->an_op = op;
    return node;
}

// Build a postfix ++ or --.
Ast_Node *Ast_NewPostInc(Ast_Node *lhs, int64_t step, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_POSTINC, lhs, line);
    node->an_step = step;
    return node;
}

// Build a member access.
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_MEMBER, lhs, line);
    node->an_memname = Str_Clone(name);
    return node;
}

// Append an asm to those written at file scope.
void Ast_AddFileAsm(Ast_Node *node)
{
    if (Ast_FileAsmsTail) {
        Ast_FileAsmsTail->an_next = node;
    } else {
        Ast_FileAsms = node;
    }
    Ast_FileAsmsTail = node;
}

// Start a fresh function.
void Ast_BeginScope(void)
{
    Ast_Locals   = NULL;
    Ast_CurScope = &Ast_FileScope;
    Ast_PushScope();
    Ast_CurScope->as_params = true;
}

// Leave a function.
void Ast_EndScope(void)
{
    Ast_CurScope = &Ast_FileScope;
}

// Enter a nested scope.
void Ast_PushScope(void)
{
    Ast_Scope *scope = calloc(1, sizeof(Ast_Scope));
    scope->as_parent = Ast_CurScope;
    scope->as_vm     = Ast_CurScope->as_vm;
    Ast_CurScope = scope;
}

// Leave a scope, keeping the frame slots its variables were given.
void Ast_PopScope(void)
{
    Ast_CurScope = Ast_CurScope->as_parent;
}

// Look up a variable by name.
Ast_Var *Ast_FindVar(const char *name)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_Var *var = scope->as_vars; var; var = var->av_scope_next) {
            if (Str_Equals(var->av_name, name)) {
                return var;
            }
        }
    }
    return Ast_FindGlobal(name);
}

// Return the parameters' scope a function body's outermost block shares.
Ast_Scope *Ast_SharedScope(void)
{
    Ast_Scope *outer = Ast_CurScope->as_parent;
    return outer && outer->as_params ? outer : NULL;
}

// Look up a variable declared in the innermost scope.
Ast_Var *Ast_FindVarHere(const char *name)
{
    Ast_Scope *scopes[] = { Ast_CurScope, Ast_SharedScope() };

    for (size_t i = 0; i < sizeof(scopes) / sizeof(scopes[0]) && scopes[i]; i++) {
        for (Ast_Var *var = scopes[i]->as_vars; var; var = var->av_scope_next) {
            if (Str_Equals(var->av_name, name)) {
                return var;
            }
        }
    }
    return NULL;
}

// Find a file-scope variable by the symbol it takes.
Ast_Var *Ast_FindGlobal(const char *symbol)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        if (Str_Equals(var->av_symbol, symbol)) {
            return var;
        }
    }
    return NULL;
}

// Find a function already declared or defined under name.
Ast_Func *Ast_FindFunction(const char *name)
{
    for (Ast_Func *fn = Ast_Program; fn; fn = fn->af_next) {
        if (Str_Equals(fn->af_name, name)) {
            return fn;
        }
    }
    return NULL;
}

// Declare a variable in the innermost scope, shadowing a name from above.
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, Ast_Line line)
{
    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_name   = Str_Clone(name);
    var->av_symbol = var->av_name;
    var->av_type   = type;
    var->av_line   = line;
    var->av_next   = Ast_Locals;
    Ast_Locals = var;

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
}

// Bring a parameter a declarator already built into the body's scope.
void Ast_DeclareParam(Ast_Var *var)
{
    var->av_symbol = var->av_name;
    var->av_next   = Ast_Locals;
    Ast_Locals     = var;

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
}

// Make a parameter visible to the ones after it in its list.
void Ast_DeclarePrototypeParam(Ast_Var *var)
{
    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
}

// Declare a variable at file scope, reusing the slot if it is already there.
Ast_Var *Ast_DeclareGlobal(const char *name, Ast_Type *type, Ast_Line line)
{
    Ast_Var *var = Ast_FindGlobal(name);
    if (var) {
        return var;
    }

    var = calloc(1, sizeof(Ast_Var));
    var->av_name   = Str_Clone(name);
    var->av_symbol = var->av_name;
    var->av_type   = type;
    var->av_line   = line;
    var->av_global = true;

    if (Ast_GlobalsTail) {
        Ast_GlobalsTail->av_next = var;
    } else {
        Ast_Globals = var;
    }
    Ast_GlobalsTail = var;
    return var;
}

// Declare a static local.
Ast_Var *Ast_DeclareStaticLocal(const char *name, const char *symbol, Ast_Type *type, Ast_Line line)
{
    Ast_Var *var = Ast_DeclareGlobal(symbol, type, line);
    var->av_name = Str_Clone(name);

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
}

// Declare a file-scope name inside a block, as `extern` there does.
Ast_Var *Ast_DeclareExternLocal(const char *name, Ast_Type *type, Ast_Line line)
{
    Ast_Var *global = Ast_FindGlobal(name);
    if (global && Ast_IsUnsized(type)) {
        type = global->av_type;
    }

    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_name    = Str_Clone(name);
    var->av_symbol  = var->av_name;
    var->av_type    = type;
    var->av_line    = line;
    var->av_global  = true;
    var->av_storage = AST_STORAGE_EXTERN;

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
}

// Return the list of locals declared in the current scope.
Ast_Var *Ast_CurrentLocals(void)
{
    return Ast_Locals;
}

// Start the scope of a variably modified name just declared.
void Ast_OpenVmScope(void)
{
    Ast_VmScope *scope = calloc(1, sizeof(Ast_VmScope));
    scope->vs_outer = Ast_CurScope->as_vm;
    Ast_CurScope->as_vm = scope;
}

// Return the innermost variably modified name's scope, or NULL.
Ast_VmScope *Ast_CurrentVmScope(void)
{
    return Ast_CurScope->as_vm;
}

// Return whether outer is scope or a scope it is nested in.
bool Ast_ContainsVmScope(const Ast_VmScope *scope, const Ast_VmScope *outer)
{
    for (; scope != outer; scope = scope->vs_outer) {
        if (! scope) {
            return false;
        }
    }
    return true;
}

// Look up a tag by name.
Ast_Type *Ast_FindTag(const char *name)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_Tag *tag = scope->as_tags; tag; tag = tag->ag_next) {
            if (Str_Equals(tag->ag_name, name)) {
                return tag->ag_type;
            }
        }
    }
    return NULL;
}

// Look up a tag declared directly in the innermost scope.
Ast_Type *Ast_FindTagHere(const char *name)
{
    for (Ast_Tag *tag = Ast_CurScope->as_tags; tag; tag = tag->ag_next) {
        if (Str_Equals(tag->ag_name, name)) {
            return tag->ag_type;
        }
    }
    return NULL;
}

// Bind a tag to a type in the innermost scope.
void Ast_DeclareTag(const char *name, Ast_Type *type)
{
    Ast_Tag *tag = calloc(1, sizeof(Ast_Tag));
    tag->ag_name = Str_Clone(name);
    tag->ag_type = type;
    tag->ag_next = Ast_CurScope->as_tags;
    Ast_CurScope->as_tags = tag;
}

// Look up a typedef name.
Ast_Type *Ast_FindTypedef(const char *name)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_Typedef *def = scope->as_typedefs; def; def = def->ad_next) {
            if (Str_Equals(def->ad_name, name)) {
                return def->ad_type;
            }
        }
    }
    return NULL;
}

// Look up a typedef name declared in the innermost scope.
Ast_Type *Ast_FindTypedefHere(const char *name)
{
    for (Ast_Typedef *def = Ast_CurScope->as_typedefs; def; def = def->ad_next) {
        if (Str_Equals(def->ad_name, name)) {
            return def->ad_type;
        }
    }
    return NULL;
}

// Bind a typedef name to a type in the innermost scope.
void Ast_DeclareTypedef(const char *name, Ast_Type *type)
{
    Ast_Typedef *def = calloc(1, sizeof(Ast_Typedef));
    def->ad_name = Str_Clone(name);
    def->ad_type = type;
    def->ad_next = Ast_CurScope->as_typedefs;
    Ast_CurScope->as_typedefs = def;
}

// Look up an enumeration constant.
bool Ast_FindEnumConst(const char *name, int64_t *value)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_EnumConst *item = scope->as_enums; item; item = item->ae_next) {
            if (Str_Equals(item->ae_name, name)) {
                *value = item->ae_value;
                return true;
            }
        }
    }
    return false;
}

// Return whether an enumeration constant is declared in the innermost scope.
bool Ast_IsEnumConstHere(const char *name)
{
    for (Ast_EnumConst *item = Ast_CurScope->as_enums; item; item = item->ae_next) {
        if (Str_Equals(item->ae_name, name)) {
            return true;
        }
    }
    return false;
}

// Bind an enumeration constant in the innermost scope.
void Ast_DeclareEnumConst(const char *name, int64_t value)
{
    Ast_EnumConst *item = calloc(1, sizeof(Ast_EnumConst));
    item->ae_name  = Str_Clone(name);
    item->ae_value = value;
    item->ae_next  = Ast_CurScope->as_enums;
    Ast_CurScope->as_enums = item;
}

// Intern a decoded string literal of len bytes and return its table slot.
size_t Ast_AddString(char *str, size_t len, size_t width)
{
    if (Ast_NumStrings == Ast_CapStrings) {
        Ast_CapStrings = Ast_CapStrings ? Ast_CapStrings * 2 : AST_STRINGS_FIRST_CAP;
        Ast_Strings = realloc(Ast_Strings, Ast_CapStrings * sizeof(*Ast_Strings));
    }
    Ast_Strings[Ast_NumStrings].as_data  = str;
    Ast_Strings[Ast_NumStrings].as_len   = len;
    Ast_Strings[Ast_NumStrings].as_width = width;
    return Ast_NumStrings++;
}

// Return the number of interned string literals.
size_t Ast_StringCount(void)
{
    return Ast_NumStrings;
}

// Return the interned string literal in the given slot.
Ast_Str *Ast_StringAt(size_t idx)
{
    return &Ast_Strings[idx];
}
