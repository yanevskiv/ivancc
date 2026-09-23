#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"
#include "syntax/ast.h"

// The finished program, filled in by the parser.
Ast_Func *Ast_Program;

// The primitive types.
Ast_Type Ast_TypeVoid = { AST_TYPE_KIND_VOID, AST_TYPE_SIZE_VOID, AST_TYPE_ALIGN_VOID, NULL, 0, NULL, NULL, 1 };
Ast_Type Ast_TypeChar = { AST_TYPE_KIND_CHAR, AST_TYPE_SIZE_CHAR, AST_TYPE_ALIGN_CHAR, NULL, 0, NULL, NULL, 1 };
Ast_Type Ast_TypeInt  = { AST_TYPE_KIND_INT,  AST_TYPE_SIZE_INT,  AST_TYPE_ALIGN_INT,  NULL, 0, NULL, NULL, 1 };

// Table of interned string literals, indexed by AST_NODE_KIND_STR slot.
static Ast_Str Ast_Strings[MAX_STRINGS];

// Number of entries currently used in Ast_Strings.
static int Ast_NumStrings;

// Every variable declared at file scope, in declaration order.
Ast_Var *Ast_Globals;

// The last global declared, so the list keeps source order.
static Ast_Var *Ast_GlobalsTail;

// The outermost scope, which holds file-scope tags and typedef names.
static Ast_Scope Ast_FileScope;

// The innermost scope currently open.
static Ast_Scope *Ast_CurScope = &Ast_FileScope;

// Locals of the function currently being parsed.
static Ast_Var *Ast_Locals;

// Round n up to the nearest multiple of align.
int Ast_AlignTo(int n, int align)
{
    return (n + align - 1) / align * align;
}

// Round n down to the multiple of align at or below it.
int Ast_AlignDown(int n, int align)
{
    return n / align * align;
}

// Build the pointer type that points at base.
Ast_Type *Ast_NewPointer(Ast_Type *base)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind     = AST_TYPE_KIND_PTR;
    type->at_size     = AST_TYPE_SIZE_PTR;
    type->at_align    = AST_TYPE_ALIGN_PTR;
    type->at_base     = base;
    type->at_complete = 1;
    return type;
}

// Build the type of an array of len elements of base.
Ast_Type *Ast_NewArray(Ast_Type *base, int len)
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

// Build an as-yet empty struct or union type, which its tag may already name.
Ast_Type *Ast_NewAggregate(Ast_TypeKind kind, const char *tag)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind  = kind;
    type->at_align = 1;
    type->at_tag   = Str_New(tag);
    return type;
}

// Build one member of a struct or union, before layout gives it an offset.
Ast_Member *Ast_NewMember(const char *name, Ast_Type *type, int line)
{
    Ast_Member *member = calloc(1, sizeof(Ast_Member));
    member->am_name = Str_New(name);
    member->am_type = type;
    member->am_line = line;
    return member;
}

// Place one bitfield at the bit cursor, in a fresh unit if it would straddle one, and return where the next starts.
int Ast_PlaceBitfield(Ast_Member *member, int bits)
{
    int unit = member->am_type->at_size * AST_BITS_PER_BYTE;

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

// Drop the members no name can reach, which is every bitfield declared to pad.
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

// Place members in an aggregate, counting in bits so bitfields share a unit, and size it to its widest member.
void Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, int line)
{
    int bits = 0;
    int align = 1;

    for (Ast_Member *member = members; member; member = member->am_next) {
        member->am_owner = type;
        if (member->am_flexible) {
            if (type->at_kind == AST_TYPE_KIND_UNION) {
                Log_ShowErrorAt(member->am_line, "a union cannot have a flexible array member");
            }
            if (member->am_next) {
                Log_ShowErrorAt(member->am_line, "flexible array member '%s' must come last", member->am_name);
            }
            if (member == members) {
                Log_ShowErrorAt(member->am_line, "a struct needs a member before a flexible array member");
            }
            member->am_offset = Ast_AlignTo(bits, member->am_type->at_align * AST_BITS_PER_BYTE) / AST_BITS_PER_BYTE;
            if (member->am_type->at_align > align) {
                align = member->am_type->at_align;
            }
            continue;
        }
        if (! member->am_type->at_complete) {
            Log_ShowErrorAt(member->am_line, "member '%s' has an incomplete type", member->am_name);
        }
        for (Ast_Member *seen = members; seen != member; seen = seen->am_next) {
            if (seen->am_name && member->am_name && strcmp(seen->am_name, member->am_name) == 0) {
                Log_ShowErrorAt(member->am_line, "duplicate member '%s'", member->am_name);
            }
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
        // An unnamed member is a bitfield too: `int :0;` declares a width of none.
        if (member->am_bits || ! member->am_name) {
            bits = Ast_PlaceBitfield(member, bits);
            continue;
        }
        bits = Ast_AlignTo(bits, member->am_type->at_align * AST_BITS_PER_BYTE);
        member->am_offset = bits / AST_BITS_PER_BYTE;
        bits += member->am_type->at_size * AST_BITS_PER_BYTE;
    }

    Ast_Member *named = Ast_NamedMembers(members);
    if (! named) {
        Log_ShowErrorAt(line, "an aggregate must declare at least one named member");
    }

    type->at_members  = named;
    type->at_complete = 1;
    type->at_align    = align;
    type->at_size     = Ast_AlignTo(bits, align * AST_BITS_PER_BYTE) / AST_BITS_PER_BYTE;
}

// Return the named member of an aggregate, or NULL when it has none.
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
Ast_Node *Ast_NewNode(Ast_NodeKind kind, int line)
{
    Ast_Node *node = calloc(1, sizeof(Ast_Node));
    node->an_kind = kind;
    node->an_line = line;
    return node;
}

// Build a binary-operator node with the given operands.
Ast_Node *Ast_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, int line)
{
    Ast_Node *node = Ast_NewNode(kind, line);
    node->an_lhs = lhs;
    node->an_rhs = rhs;
    return node;
}

// Build a unary-operator node with the given operand.
Ast_Node *Ast_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, int line)
{
    Ast_Node *node = Ast_NewNode(kind, line);
    node->an_lhs = lhs;
    return node;
}

// Build an integer-literal node.
Ast_Node *Ast_NewNum(long val, int line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_NUM, line);
    node->an_val = val;
    return node;
}

// Build a node that references a local variable.
Ast_Node *Ast_NewVarNode(Ast_Var *var, int line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_VAR, line);
    node->an_var = var;
    return node;
}

// Build a compound assignment, with op naming the operation it applies.
Ast_Node *Ast_NewOpAssign(Ast_NodeKind op, Ast_Node *lhs, Ast_Node *rhs, int line)
{
    Ast_Node *node = Ast_NewBinary(AST_NODE_KIND_OPASSIGN, lhs, rhs, line);
    node->an_op = op;
    return node;
}

// Build a postfix ++ or --, which steps by step and yields the old value.
Ast_Node *Ast_NewPostInc(Ast_Node *lhs, long step, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_POSTINC, lhs, line);
    node->an_val = step;
    return node;
}

// Build a member access, whose member the Sem_ pass resolves once it has a type.
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_MEMBER, lhs, line);
    node->an_memname = Str_New(name);
    return node;
}

// Declare a static local: it answers to name in its scope but lives under symbol, which carries its function.
Ast_Var *Ast_DeclareStaticLocal(const char *name, const char *symbol, Ast_Type *type, int line)
{
    Ast_Var *var = Ast_DeclareGlobal(symbol, type, line);
    var->av_name = Str_New(name);

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
}

// Start a fresh function: no locals, and one scope for its parameters.
void Ast_BeginScope(void)
{
    Ast_Locals   = NULL;
    Ast_CurScope = &Ast_FileScope;
    Ast_PushScope();
}

// Leave a function, so that what follows it is read at file scope again.
void Ast_EndScope(void)
{
    Ast_CurScope = &Ast_FileScope;
}

// Enter a nested scope, which shadows the ones around it.
void Ast_PushScope(void)
{
    Ast_Scope *scope = calloc(1, sizeof(Ast_Scope));
    scope->as_parent = Ast_CurScope;
    Ast_CurScope = scope;
}

// Leave a scope, keeping the frame slots its variables were given and putting only their names out of reach.
void Ast_PopScope(void)
{
    Ast_CurScope = Ast_CurScope->as_parent;
}

// Look up a variable by name: the innermost scope outwards, then file scope.
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

// Declare a variable at file scope, reusing the slot if it is already there.
Ast_Var *Ast_DeclareGlobal(const char *name, Ast_Type *type, int line)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }

    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_name   = Str_New(name);
    var->av_symbol = var->av_name;
    var->av_type   = type;
    var->av_line   = line;
    var->av_global = 1;

    if (Ast_GlobalsTail) {
        Ast_GlobalsTail->av_next = var;
    } else {
        Ast_Globals = var;
    }
    Ast_GlobalsTail = var;
    return var;
}

// Declare a variable in the innermost scope, reusing a slot declared there and shadowing a name from above.
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, int line)
{
    for (Ast_Var *var = Ast_CurScope->as_vars; var; var = var->av_scope_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }

    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_name   = Str_New(name);
    var->av_symbol = var->av_name;
    var->av_type   = type;
    var->av_line   = line;
    var->av_next   = Ast_Locals;
    Ast_Locals = var;

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
}

// Look up a tag by name, innermost scope outwards.
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

// Look up a tag declared directly in the innermost scope, which decides whether a definition completes or shadows.
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
    tag->ag_name = Str_New(name);
    tag->ag_type = type;
    tag->ag_next = Ast_CurScope->as_tags;
    Ast_CurScope->as_tags = tag;
}

// Look up a typedef name, innermost scope outwards.
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
    def->ad_name = Str_New(name);
    def->ad_type = type;
    def->ad_next = Ast_CurScope->as_typedefs;
    Ast_CurScope->as_typedefs = def;
}

// Look up an enumeration constant, reporting whether the name names one.
int Ast_FindEnumConst(const char *name, long *value)
{
    for (Ast_Scope *scope = Ast_CurScope; scope; scope = scope->as_parent) {
        for (Ast_EnumConst *item = scope->as_enums; item; item = item->ae_next) {
            if (strcmp(item->ae_name, name) == 0) {
                *value = item->ae_value;
                return 1;
            }
        }
    }
    return 0;
}

// Bind an enumeration constant in the innermost scope.
void Ast_DeclareEnumConst(const char *name, long value)
{
    Ast_EnumConst *item = calloc(1, sizeof(Ast_EnumConst));
    item->ae_name  = Str_New(name);
    item->ae_value = value;
    item->ae_next  = Ast_CurScope->as_enums;
    Ast_CurScope->as_enums = item;
}

// Return the list of locals declared in the current scope.
Ast_Var *Ast_CurrentLocals(void)
{
    return Ast_Locals;
}

// Intern a decoded string literal of len bytes and return its table slot.
int Ast_AddString(char *str, int len)
{
    if (Ast_NumStrings >= MAX_STRINGS) {
        Log_ShowError("too many string literals (max %d)", MAX_STRINGS);
    }
    Ast_Strings[Ast_NumStrings].as_data = str;
    Ast_Strings[Ast_NumStrings].as_len  = len;
    return Ast_NumStrings++;
}

// Return the number of interned string literals.
int Ast_StringCount(void)
{
    return Ast_NumStrings;
}

// Return the interned string literal in the given slot.
Ast_Str *Ast_StringAt(int idx)
{
    return &Ast_Strings[idx];
}
