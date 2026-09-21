#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "ast/ast.h"

// The finished program, filled in by the parser.
Ast_Func *Ast_Program;

// The primitive types.
Ast_Type Ast_TypeVoid = { AST_TYPE_KIND_VOID, AST_TYPE_SIZE_VOID, AST_TYPE_ALIGN_VOID, NULL, 0 };
Ast_Type Ast_TypeChar = { AST_TYPE_KIND_CHAR, AST_TYPE_SIZE_CHAR, AST_TYPE_ALIGN_CHAR, NULL, 0 };
Ast_Type Ast_TypeInt  = { AST_TYPE_KIND_INT,  AST_TYPE_SIZE_INT,  AST_TYPE_ALIGN_INT,  NULL, 0 };

// Table of interned string literals, indexed by AST_NODE_KIND_STR slot.
static Ast_Str Ast_Strings[MAX_STRINGS];

// Number of entries currently used in Ast_Strings.
static int Ast_NumStrings;

// Every variable declared at file scope, in declaration order.
Ast_Var *Ast_Globals;

// The last global declared, so the list keeps source order.
static Ast_Var *Ast_GlobalsTail;

// The innermost scope currently open.
static Ast_Scope *Ast_CurScope;

// Locals of the function currently being parsed.
static Ast_Var *Ast_Locals;

// Build the pointer type that points at base.
Ast_Type *Ast_NewPointer(Ast_Type *base)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind  = AST_TYPE_KIND_PTR;
    type->at_size  = AST_TYPE_SIZE_PTR;
    type->at_align = AST_TYPE_ALIGN_PTR;
    type->at_base  = base;
    return type;
}

// Build the type of an array of len elements of base.
Ast_Type *Ast_NewArray(Ast_Type *base, int len)
{
    Ast_Type *type = calloc(1, sizeof(Ast_Type));
    type->at_kind  = AST_TYPE_KIND_ARRAY;
    type->at_size  = base->at_size * len;
    type->at_align = base->at_align;
    type->at_base  = base;
    type->at_len   = len;
    return type;
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

// Start a fresh function: no locals, and one scope for its parameters.
void Ast_BeginScope(void)
{
    Ast_Locals   = NULL;
    Ast_CurScope = NULL;
    Ast_PushScope();
}

// Enter a nested scope, which shadows the ones around it.
void Ast_PushScope(void)
{
    Ast_Scope *scope = calloc(1, sizeof(Ast_Scope));
    scope->as_parent = Ast_CurScope;
    Ast_CurScope = scope;
}

// Leave a scope. Its variables keep their frame slots, which the code
// generator has already been told about; only the names go out of reach.
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
    var->av_name   = strdup(name);
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

// Declare a variable in the innermost scope. A name already declared in that
// same scope keeps its slot; one from an enclosing scope is shadowed instead.
Ast_Var *Ast_DeclareVar(const char *name, Ast_Type *type, int line)
{
    for (Ast_Var *var = Ast_CurScope->as_vars; var; var = var->av_scope_next) {
        if (strcmp(var->av_name, name) == 0) {
            return var;
        }
    }

    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_name = strdup(name);
    var->av_type = type;
    var->av_line = line;
    var->av_next = Ast_Locals;
    Ast_Locals = var;

    var->av_scope_next = Ast_CurScope->as_vars;
    Ast_CurScope->as_vars = var;
    return var;
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
