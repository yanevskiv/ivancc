// C source file for the abstract syntax tree.

// Take every include from the module's header.
#include "lang/ast.h"

// The finished program, filled in by the parser.
Ast_Func *Ast_Program;

// Every variable declared at file scope.
Ast_Var *Ast_Globals;

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
static Ast_Str Ast_Strings[AST_MAX_STRINGS];

// Number of entries currently used in Ast_Strings.
static size_t Ast_NumStrings;

// The last global declared.
static Ast_Var *Ast_GlobalsTail;

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

// Build a function type.
Ast_Type *Ast_NewFunction(Ast_Type *ret, Ast_Var *params, int32_t nparams, Ast_TypeVariadic variadic, Ast_TypeProto proto)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind     = AST_TYPE_KIND_FUNC;
    type->at_size     = AST_TYPE_SIZE_FUNC;
    type->at_align    = AST_TYPE_ALIGN_FUNC;
    type->at_ret      = ret;
    type->at_params   = params;
    type->at_nparams  = nparams;
    type->at_variadic = variadic;
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
        Err_AssertAt(member->am_line, member->am_type->at_complete, ERR_AST_MEMBER_INCOMPLETE, member->am_name);
        for (Ast_Member *seen = members; seen != member; seen = seen->am_next) {
            Err_AssertAt(member->am_line, ! seen->am_name || ! member->am_name || strcmp(seen->am_name, member->am_name) != 0, ERR_AST_MEMBER_DUPLICATE, member->am_name);
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
    Err_AssertAt(line, named, ERR_AST_AGGREGATE_UNNAMED);

    type->at_members  = named;
    type->at_complete = AST_TYPE_COMPLETE;
    type->at_align    = align;
    type->at_size     = Ast_AlignTo(bits, align * AST_BITS_PER_BYTE) / AST_BITS_PER_BYTE;
}

// Return the named member of an aggregate.
Ast_Member *Ast_FindMember(const Ast_Type *type, const char *name)
{
    for (Ast_Member *member = type->at_members; member; member = member->am_next) {
        if (strcmp(member->am_name, name) == 0) {
            return member;
        }
    }
    return NULL;
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
    node->an_val = step;
    return node;
}

// Build a member access.
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_MEMBER, lhs, line);
    node->an_memname = Str_Clone(name);
    return node;
}

// Start a fresh function.
void Ast_BeginScope(void)
{
    Ast_Locals   = NULL;
    Ast_CurScope = &Ast_FileScope;
    Ast_PushScope();
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
            if (strcmp(var->av_name, name) == 0) {
                return var;
            }
        }
    }
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }
    return NULL;
}

// Declare a variable in the innermost scope, shadowing a name from above.
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, Ast_Line line)
{
    for (Ast_Var *var = Ast_CurScope->as_vars; var; var = var->av_scope_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }

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

// Declare a variable at file scope, reusing the slot if it is already there.
Ast_Var *Ast_DeclareGlobal(const char *name, Ast_Type *type, Ast_Line line)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }

    Ast_Var *var = calloc(1, sizeof(Ast_Var));
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

// Return the list of locals declared in the current scope.
Ast_Var *Ast_CurrentLocals(void)
{
    return Ast_Locals;
}

// Look up a tag by name.
Ast_Type *Ast_FindTag(const char *name)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_Tag *tag = scope->as_tags; tag; tag = tag->ag_next) {
            if (strcmp(tag->ag_name, name) == 0) {
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
        if (strcmp(tag->ag_name, name) == 0) {
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
            if (strcmp(def->ad_name, name) == 0) {
                return def->ad_type;
            }
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
            if (strcmp(item->ae_name, name) == 0) {
                *value = item->ae_value;
                return true;
            }
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
    Err_Assert(Ast_NumStrings < AST_MAX_STRINGS, ERR_AST_TOO_MANY_STRINGS, AST_MAX_STRINGS);
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
