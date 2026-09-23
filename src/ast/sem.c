#include <string.h>

#include "util/log.h"
#include "ast/sem.h"

// The program being analysed, for resolving calls against its definitions.
static Ast_Func *Sem_Prog;

// The function whose body is being analysed.
static Ast_Func *Sem_CurFunc;

// Return the function of that name defined in this program, or NULL.
Ast_Func *Sem_FindFunc(const char *name)
{
    for (Ast_Func *func = Sem_Prog; func; func = func->af_next) {
        if (strcmp(func->af_name, name) == 0) {
            return func;
        }
    }
    return NULL;
}

// Return the length of a node list.
int Sem_CountNodes(Ast_Node *list)
{
    int count = 0;
    for (Ast_Node *node = list; node; node = node->an_next) {
        count++;
    }
    return count;
}

// Return whether values of this type address memory: a pointer, or an array.
int Sem_IsPointer(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_PTR || type->at_kind == AST_TYPE_KIND_ARRAY;
}

// Return whether a node names an object, which a member does only when what holds it does.
int Sem_IsLvalue(const Ast_Node *node)
{
    if (node->an_kind == AST_NODE_KIND_MEMBER) {
        return Sem_IsLvalue(node->an_lhs);
    }
    return node->an_kind == AST_NODE_KIND_VAR || node->an_kind == AST_NODE_KIND_DEREF
        || node->an_kind == AST_NODE_KIND_COMPOUND;
}

// Return whether this is a struct or union, which is to say a type whose values
// are moved whole rather than held in a register.
int Sem_IsAggregate(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_STRUCT || type->at_kind == AST_TYPE_KIND_UNION;
}

// Return the tag a struct or union was declared with, for a diagnostic to name.
const char *Sem_TypeName(const Ast_Type *type)
{
    return type->at_tag ? type->at_tag : "<anonymous>";
}

// Return the type an expression of this type yields; an array yields a pointer.
Ast_Type *Sem_Decay(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY) {
        return Ast_NewPointer(type->at_base);
    }
    return type;
}

// Narrow a folded value to the type a cast names, as the generated code would.
long Sem_Truncate(const Ast_Type *type, long value)
{
    switch (type->at_kind) {
        case AST_TYPE_KIND_CHAR: {
            value = (signed char) value;
        } break;
        case AST_TYPE_KIND_INT: {
            value = (int) value;
        } break;
        case AST_TYPE_KIND_VOID:
        case AST_TYPE_KIND_PTR:
        case AST_TYPE_KIND_ARRAY:
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_UNION: {
            // already as wide as the value is held
        } break;
    }
    return value;
}

// Apply one operator to folded operands, rejecting the division by zero C leaves undefined.
int Sem_FoldOp(Ast_NodeKind kind, long lhs, long rhs, int line, long *value)
{
    switch (kind) {
        case AST_NODE_KIND_ADD: {
            *value = lhs + rhs;
        } break;
        case AST_NODE_KIND_SUB: {
            *value = lhs - rhs;
        } break;
        case AST_NODE_KIND_MUL: {
            *value = lhs * rhs;
        } break;
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_MOD: {
            if (rhs == 0) {
                Log_ShowErrorAt(line, "division by zero in a constant expression");
            }
            *value = kind == AST_NODE_KIND_DIV ? lhs / rhs : lhs % rhs;
        } break;
        case AST_NODE_KIND_BITAND: {
            *value = lhs & rhs;
        } break;
        case AST_NODE_KIND_BITOR: {
            *value = lhs | rhs;
        } break;
        case AST_NODE_KIND_BITXOR: {
            *value = lhs ^ rhs;
        } break;
        case AST_NODE_KIND_SHL: {
            *value = lhs << rhs;
        } break;
        case AST_NODE_KIND_SHR: {
            *value = lhs >> rhs;
        } break;
        case AST_NODE_KIND_EQ: {
            *value = lhs == rhs;
        } break;
        case AST_NODE_KIND_NE: {
            *value = lhs != rhs;
        } break;
        case AST_NODE_KIND_LT: {
            *value = lhs < rhs;
        } break;
        case AST_NODE_KIND_LE: {
            *value = lhs <= rhs;
        } break;
        case AST_NODE_KIND_AND: {
            *value = lhs && rhs;
        } break;
        case AST_NODE_KIND_OR: {
            *value = lhs || rhs;
        } break;
        case AST_NODE_KIND_NEG: {
            *value = -lhs;
        } break;
        case AST_NODE_KIND_NOT: {
            *value = ! lhs;
        } break;
        case AST_NODE_KIND_BITNOT: {
            *value = ~lhs;
        } break;
        default: {
            return 0;
        }
    }
    return 1;
}

// Fold an integer constant expression to its value, or return false when it is not one.
int Sem_Fold(const Ast_Node *node, long *value)
{
    long lhs = 0;
    long rhs = 0;

    if (! node) {
        return 0;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            *value = node->an_val;
        } break;
        case AST_NODE_KIND_SIZEOF: {
            if (! node->an_lhs->an_type) {
                return 0;  // the Sem_ pass has not typed the operand yet
            }
            *value = node->an_lhs->an_type->at_size;
        } break;
        case AST_NODE_KIND_CAST: {
            if (! Sem_Fold(node->an_lhs, &lhs)) {
                return 0;
            }
            *value = Sem_Truncate(node->an_type, lhs);
        } break;
        case AST_NODE_KIND_COND: {
            if (! Sem_Fold(node->an_cond, &lhs)) {
                return 0;
            }
            if (! Sem_Fold(lhs ? node->an_then : node->an_els, value)) {
                return 0;
            }
        } break;
        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_NOT:
        case AST_NODE_KIND_BITNOT: {
            if (! Sem_Fold(node->an_lhs, &lhs)) {
                return 0;
            }
            if (! Sem_FoldOp(node->an_kind, lhs, 0, node->an_line, value)) {
                return 0;
            }
        } break;
        default: {
            if (! Sem_Fold(node->an_lhs, &lhs) || ! Sem_Fold(node->an_rhs, &rhs)) {
                return 0;
            }
            if (! Sem_FoldOp(node->an_kind, lhs, rhs, node->an_line, value)) {
                return 0;
            }
        } break;
    }
    return 1;
}

// Check a call against the callee's definition, if this program has one.
void Sem_CheckCall(Ast_Node *node)
{
    Ast_Func *func = Sem_FindFunc(node->an_funcname);
    if (! func) {
        return;
    }
    int given = Sem_CountNodes(node->an_args);
    if (func->af_variadic) {
        if (given < func->af_nparams) {
            Log_ShowErrorAt(node->an_line, "too few arguments to '%s': got %d, expected at least %d", node->an_funcname, given, func->af_nparams);
        }
        return;
    }
    if (given != func->af_nparams) {
        Log_ShowErrorAt(node->an_line, "wrong number of arguments to '%s': got %d, expected %d", node->an_funcname, given, func->af_nparams);
    }
}

// Attach every case and default of a switch to it, in source order. A nested
// switch owns its own cases, so the walk stops there.
void Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail)
{
    if (! node || node->an_kind == AST_NODE_KIND_SWITCH) {
        return;
    }

    if (node->an_kind == AST_NODE_KIND_CASE || node->an_kind == AST_NODE_KIND_DEFAULT) {
        for (Ast_Node *seen = sw->an_cases; seen; seen = seen->an_case_next) {
            if (seen->an_kind == node->an_kind
                && (node->an_kind == AST_NODE_KIND_DEFAULT || seen->an_val == node->an_val)) {
                Log_ShowErrorAt(node->an_line, "duplicate case in switch");
            }
        }
        if (*tail) {
            (*tail)->an_case_next = node;
        } else {
            sw->an_cases = node;
        }
        *tail = node;
    }

    Sem_CollectCases(node->an_lhs, sw, tail);
    Sem_CollectCases(node->an_then, sw, tail);
    Sem_CollectCases(node->an_els, sw, tail);
    Sem_CollectCases(node->an_body, sw, tail);
    Sem_CollectCases(node->an_next, sw, tail);
}

// Return whether the statements under node define a label of this name.
int Sem_FindLabel(Ast_Node *node, const char *name)
{
    if (! node) {
        return 0;
    }
    if (node->an_kind == AST_NODE_KIND_LABEL && strcmp(node->an_funcname, name) == 0) {
        return 1;
    }
    return Sem_FindLabel(node->an_lhs, name) || Sem_FindLabel(node->an_then, name)
        || Sem_FindLabel(node->an_els, name) || Sem_FindLabel(node->an_body, name)
        || Sem_FindLabel(node->an_next, name);
}

// Reject a goto that names a label its function never defines.
void Sem_CheckGotos(Ast_Node *node, Ast_Node *body)
{
    if (! node) {
        return;
    }
    if (node->an_kind == AST_NODE_KIND_GOTO && ! Sem_FindLabel(body, node->an_funcname)) {
        Log_ShowErrorAt(node->an_line, "goto names an undefined label '%s'", node->an_funcname);
    }
    Sem_CheckGotos(node->an_lhs, body);
    Sem_CheckGotos(node->an_then, body);
    Sem_CheckGotos(node->an_els, body);
    Sem_CheckGotos(node->an_body, body);
    Sem_CheckGotos(node->an_next, body);
}

// Wrap node in a multiplication by size, so it steps whole elements.
Ast_Node *Sem_ScaleBy(Ast_Node *node, int size)
{
    Ast_Node *num = Ast_NewNum(size, node->an_line);
    num->an_type = &Ast_TypeInt;

    Ast_Node *mul = Ast_NewBinary(AST_NODE_KIND_MUL, node, num, node->an_line);
    mul->an_type = &Ast_TypeInt;
    return mul;
}

// Type + and -, scaling an integer operand against a pointer and reducing p - q.
void Sem_Arith(Ast_Node *node)
{
    Ast_Type *lhs = node->an_lhs->an_type;
    Ast_Type *rhs = node->an_rhs->an_type;

    if (! Sem_IsPointer(lhs) && ! Sem_IsPointer(rhs)) {
        node->an_type = &Ast_TypeInt;
        return;
    }

    if (Sem_IsPointer(lhs) && Sem_IsPointer(rhs)) {
        if (node->an_kind != AST_NODE_KIND_SUB) {
            Log_ShowErrorAt(node->an_line, "cannot add two pointers");
        }

        // A pointer difference counts elements, not the bytes between them.
        Ast_Node *diff = Ast_NewBinary(AST_NODE_KIND_SUB, node->an_lhs, node->an_rhs, node->an_line);
        diff->an_type = &Ast_TypeInt;

        node->an_kind = AST_NODE_KIND_DIV;
        node->an_lhs  = diff;
        node->an_rhs  = Ast_NewNum(lhs->at_base->at_size, node->an_line);
        node->an_rhs->an_type = &Ast_TypeInt;
        node->an_type = &Ast_TypeInt;
        return;
    }

    if (Sem_IsPointer(rhs)) {
        if (node->an_kind != AST_NODE_KIND_ADD) {
            Log_ShowErrorAt(node->an_line, "cannot subtract a pointer from an integer");
        }
        node->an_lhs  = Sem_ScaleBy(node->an_lhs, rhs->at_base->at_size);
        node->an_type = Sem_Decay(rhs);
        return;
    }

    node->an_rhs  = Sem_ScaleBy(node->an_rhs, lhs->at_base->at_size);
    node->an_type = Sem_Decay(lhs);
}

// Annotate a node and everything below it, depth first.
void Sem_Node(Ast_Node *node)
{
    if (! node) {
        return;
    }

    Sem_Node(node->an_lhs);
    Sem_Node(node->an_rhs);
    Sem_Node(node->an_cond);
    Sem_Node(node->an_then);
    Sem_Node(node->an_els);
    Sem_Node(node->an_init);
    Sem_Node(node->an_inc);
    Sem_Node(node->an_body);
    Sem_Node(node->an_args);
    Sem_Node(node->an_next);

    switch (node->an_kind) {
        case AST_NODE_KIND_NUM:
        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_MOD:
        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_NOT:
        case AST_NODE_KIND_BITNOT:
        case AST_NODE_KIND_BITAND:
        case AST_NODE_KIND_BITOR:
        case AST_NODE_KIND_BITXOR:
        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR:
        case AST_NODE_KIND_EQ:
        case AST_NODE_KIND_NE:
        case AST_NODE_KIND_LT:
        case AST_NODE_KIND_LE:
        case AST_NODE_KIND_AND:
        case AST_NODE_KIND_OR: {
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB: {
            Sem_Arith(node);
        } break;

        case AST_NODE_KIND_STR: {
            Ast_Str *str = Ast_StringAt(node->an_str_idx);
            node->an_type = Ast_NewArray(&Ast_TypeChar, str->as_len + 1);
        } break;

        case AST_NODE_KIND_ADDR: {
            if (! Sem_IsLvalue(node->an_lhs)) {
                Log_ShowErrorAt(node->an_line, "cannot take the address of this expression");
            }
            if (node->an_lhs->an_kind == AST_NODE_KIND_MEMBER && node->an_lhs->an_member->am_bits) {
                Log_ShowErrorAt(node->an_line, "cannot take the address of a bit-field");
            }
            node->an_type = Ast_NewPointer(node->an_lhs->an_type);
        } break;

        case AST_NODE_KIND_DEREF: {
            if (! Sem_IsPointer(node->an_lhs->an_type)) {
                Log_ShowErrorAt(node->an_line, "indirection requires a pointer operand");
            }
            if (node->an_lhs->an_type->at_base->at_kind == AST_TYPE_KIND_VOID) {
                Log_ShowErrorAt(node->an_line, "cannot dereference a pointer to void");
            }
            if (! node->an_lhs->an_type->at_base->at_complete) {
                Log_ShowErrorAt(node->an_line, "cannot dereference a pointer to an incomplete type");
            }
            node->an_type = node->an_lhs->an_type->at_base;
        } break;

        case AST_NODE_KIND_MEMBER: {
            Ast_Type *type = node->an_lhs->an_type;
            if (! Sem_IsAggregate(type)) {
                Log_ShowErrorAt(node->an_line, "request for member '%s' in something that is not a struct or union", node->an_memname);
            }
            if (! type->at_complete) {
                Log_ShowErrorAt(node->an_line, "'%s' is an incomplete type", Sem_TypeName(type));
            }
            node->an_member = Ast_FindMember(type, node->an_memname);
            if (! node->an_member) {
                Log_ShowErrorAt(node->an_line, "no member named '%s' in '%s'", node->an_memname, Sem_TypeName(type));
            }
            node->an_type = node->an_member->am_type;
        } break;

        case AST_NODE_KIND_CAST: {
            // the parser already set an_type from the type it names
        } break;

        case AST_NODE_KIND_SIZEOF: {
            node->an_kind = AST_NODE_KIND_NUM;
            node->an_val  = node->an_lhs->an_type->at_size;
            node->an_lhs  = NULL;
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_COMPOUND: {
            node->an_type = node->an_var->av_type;
        } break;

        case AST_NODE_KIND_ASSIGN: {
            if (! Sem_IsLvalue(node->an_lhs)) {
                Log_ShowErrorAt(node->an_line, "expression is not assignable");
            }
            if (node->an_lhs->an_type->at_kind == AST_TYPE_KIND_ARRAY) {
                Log_ShowErrorAt(node->an_line, "cannot assign to an array");
            }
            if (Sem_IsAggregate(node->an_lhs->an_type) && node->an_lhs->an_type != node->an_rhs->an_type) {
                Log_ShowErrorAt(node->an_line, "cannot assign a value of a different struct or union type");
            }
            node->an_type = node->an_lhs->an_type;
        } break;

        case AST_NODE_KIND_OPASSIGN: {
            if (! Sem_IsLvalue(node->an_lhs)) {
                Log_ShowErrorAt(node->an_line, "expression is not assignable");
            }
            Ast_Type *type = node->an_lhs->an_type;
            if (Sem_IsPointer(type) && (node->an_op == AST_NODE_KIND_ADD || node->an_op == AST_NODE_KIND_SUB)) {
                node->an_rhs = Sem_ScaleBy(node->an_rhs, type->at_base->at_size);
            }
            node->an_type = type;
        } break;

        case AST_NODE_KIND_POSTINC: {
            if (! Sem_IsLvalue(node->an_lhs)) {
                Log_ShowErrorAt(node->an_line, "expression is not assignable");
            }
            Ast_Type *type = node->an_lhs->an_type;
            if (Sem_IsPointer(type)) {
                node->an_val *= type->at_base->at_size;
            }
            node->an_type = type;
        } break;

        case AST_NODE_KIND_COND: {
            node->an_type = node->an_then->an_type;
        } break;

        case AST_NODE_KIND_COMMA: {
            node->an_type = node->an_rhs->an_type;
        } break;

        case AST_NODE_KIND_CALL: {
            Ast_Func *func = Sem_FindFunc(node->an_funcname);
            Sem_CheckCall(node);
            node->an_type = func && func->af_ret ? func->af_ret : &Ast_TypeInt;
        } break;

        // Only va_start needs a variadic function; a va_list can be handed on.
        case AST_NODE_KIND_VA_START: {
            if (! Sem_CurFunc->af_variadic) {
                Log_ShowErrorAt(node->an_line, "__builtin_va_start outside a variadic function");
            }
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_VA_ARG: {
            // the parser already set an_type from the type it names
        } break;

        case AST_NODE_KIND_CASE: {
            if (! Sem_Fold(node->an_cond, &node->an_val)) {
                Log_ShowErrorAt(node->an_line, "case label is not a constant");
            }
        } break;

        case AST_NODE_KIND_SWITCH: {
            Ast_Node *tail = NULL;
            Sem_CollectCases(node->an_body, node, &tail);
        } break;

        case AST_NODE_KIND_RETURN:
        case AST_NODE_KIND_GOTO:
        case AST_NODE_KIND_LABEL:
        case AST_NODE_KIND_DEFAULT:
        case AST_NODE_KIND_IF:
        case AST_NODE_KIND_FOR:
        case AST_NODE_KIND_DO:
        case AST_NODE_KIND_BREAK:
        case AST_NODE_KIND_CONTINUE:
        case AST_NODE_KIND_BLOCK:
        case AST_NODE_KIND_EXPR_STMT:
        case AST_NODE_KIND_INIT:
        case AST_NODE_KIND_INITLIST:
        case AST_NODE_KIND_DESIGNATOR:
        case AST_NODE_KIND_ZERO:
        case AST_NODE_KIND_NOP: {
            // empty
        } break;
    }
}

// Annotate every node with its type and reject what the grammar cannot.
void Sem_Analyze(Ast_Func *prog)
{
    Sem_Prog = prog;

    for (Ast_Func *func = prog; func; func = func->af_next) {
        if (! func->af_body) {
            continue;  // a prototype declares a signature and nothing to walk
        }
        Sem_CurFunc = func;
        Sem_Node(func->af_body);
        Sem_CheckGotos(func->af_body, func->af_body);
    }
}
