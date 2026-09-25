// C source file for semantic analysis.

// Module header.
#include "lang/sem.h"

// The program being analysed.
static Ast_Func *Sem_Prog;

// The function whose body is being analysed.
static Ast_Func *Sem_CurFunc;

// Return the function of that name defined in this program.
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
int32_t Sem_CountNodes(Ast_Node *list)
{
    int32_t count = 0;
    for (Ast_Node *node = list; node; node = node->an_next) {
        count++;
    }
    return count;
}

// Return whether values of this type address memory.
bool Sem_IsPointer(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_PTR || type->at_kind == AST_TYPE_KIND_ARRAY;
}

// Return whether a node names an object.
bool Sem_IsLvalue(const Ast_Node *node)
{
    if (node->an_kind == AST_NODE_KIND_MEMBER) {
        return Sem_IsLvalue(node->an_lhs);
    }
    return node->an_kind == AST_NODE_KIND_VAR || node->an_kind == AST_NODE_KIND_DEREF
        || node->an_kind == AST_NODE_KIND_COMPOUND;
}

// Return whether this is a struct or union.
bool Sem_IsAggregate(const Ast_Type *type)
{
    return type->at_kind == AST_TYPE_KIND_STRUCT || type->at_kind == AST_TYPE_KIND_UNION;
}

// Return the tag a struct or union was declared with.
const char *Sem_TypeName(const Ast_Type *type)
{
    return type->at_tag ? type->at_tag : "<anonymous>";
}

// Return the type an expression of this type yields.
Ast_Type *Sem_Decay(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY) {
        return Ast_NewPointer(type->at_base);
    }
    return type;
}

// Return whether two types agree but for their qualifiers.
bool Sem_SameType(const Ast_Type *a, const Ast_Type *b)
{
    if (a == b) {
        return true;
    }
    return a->at_kind == b->at_kind && a->at_sign == b->at_sign && a->at_base == b->at_base
        && a->at_members == b->at_members;
}

// Return the type integer promotion gives an operand.
Ast_Type *Sem_Promote(Ast_Type *type)
{
    if (Ast_IsInteger(type) && type->at_kind < AST_TYPE_KIND_INT) {
        return &Ast_TypeInt;
    }
    return type;
}

// Return the type two arithmetic operands meet in.
Ast_Type *Sem_CommonType(Ast_Type *lhs, Ast_Type *rhs)
{
    lhs = Sem_Promote(lhs);
    rhs = Sem_Promote(rhs);

    if (! Ast_IsInteger(lhs) || ! Ast_IsInteger(rhs)) {
        return Ast_IsInteger(lhs) ? rhs : lhs;
    }
    if (lhs->at_sign == rhs->at_sign) {
        return lhs->at_kind >= rhs->at_kind ? lhs : rhs;
    }

    Ast_Type *sign = lhs->at_sign == AST_TYPE_UNSIGNED ? rhs : lhs;
    Ast_Type *unsig = lhs->at_sign == AST_TYPE_UNSIGNED ? lhs : rhs;

    if (unsig->at_kind >= sign->at_kind) {
        return unsig;
    }
    if (sign->at_size > unsig->at_size) {
        return sign;
    }
    return Ast_IntegerType(sign->at_kind, AST_TYPE_UNSIGNED);
}

// Wrap a node in the cast that converts it to type.
Ast_Node *Sem_Convert(Ast_Node *node, Ast_Type *type)
{
    if (! node || ! node->an_type || Sem_SameType(node->an_type, type)) {
        return node;
    }
    if (! Ast_IsInteger(node->an_type) || ! Ast_IsInteger(type)) {
        return node;
    }

    Ast_Node *cast = Ast_NewUnary(AST_NODE_KIND_CAST, node, node->an_line);
    cast->an_type = type;
    return cast;
}

// Convert both operands of an operator to their common type.
void Sem_UsualArith(Ast_Node *node)
{
    Ast_Type *type = Sem_CommonType(node->an_lhs->an_type, node->an_rhs->an_type);

    node->an_lhs = Sem_Convert(node->an_lhs, type);
    node->an_rhs = Sem_Convert(node->an_rhs, type);
    node->an_type = type;
}

// Promote each operand of a shift on its own.
void Sem_PromoteShift(Ast_Node *node)
{
    node->an_lhs = Sem_Convert(node->an_lhs, Sem_Promote(node->an_lhs->an_type));
    node->an_rhs = Sem_Convert(node->an_rhs, Sem_Promote(node->an_rhs->an_type));
    node->an_type = node->an_lhs->an_type;
}

// Narrow a folded value to the type a cast names.
int64_t Sem_Truncate(const Ast_Type *type, int64_t value)
{
    switch (type->at_kind) {
        case AST_TYPE_KIND_BOOL: {
            value = value != 0;
        } break;
        case AST_TYPE_KIND_CHAR: {
            value = type->at_sign == AST_TYPE_UNSIGNED ? (int64_t) (uint8_t) value : (int64_t) (int8_t) value;
        } break;
        case AST_TYPE_KIND_SHORT: {
            value = type->at_sign == AST_TYPE_UNSIGNED ? (int64_t) (uint16_t) value : (int64_t) (int16_t) value;
        } break;
        case AST_TYPE_KIND_INT: {
            value = type->at_sign == AST_TYPE_UNSIGNED ? (int64_t) (uint32_t) value : (int64_t) (int32_t) value;
        } break;
        case AST_TYPE_KIND_LONG:
        case AST_TYPE_KIND_LLONG:
        case AST_TYPE_KIND_VOID:
        case AST_TYPE_KIND_PTR:
        case AST_TYPE_KIND_ARRAY:
        case AST_TYPE_KIND_FUNC:
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_UNION:
        case AST_TYPE_KIND_COUNT: {
            // already as wide as the value is held
        } break;
    }
    return value;
}

// Return the signedness an operator's operands fold with.
Ast_TypeSign Sem_FoldSign(const Ast_Node *node)
{
    const Ast_Type *lhs = node->an_lhs ? node->an_lhs->an_type : NULL;
    const Ast_Type *rhs = node->an_rhs ? node->an_rhs->an_type : NULL;

    if (lhs && Ast_IsInteger(lhs) && lhs->at_sign == AST_TYPE_UNSIGNED && lhs->at_kind >= AST_TYPE_KIND_INT) {
        return AST_TYPE_UNSIGNED;
    }
    if (rhs && Ast_IsInteger(rhs) && rhs->at_sign == AST_TYPE_UNSIGNED && rhs->at_kind >= AST_TYPE_KIND_INT) {
        return AST_TYPE_UNSIGNED;
    }
    return AST_TYPE_SIGNED;
}

// Apply one operator to folded operands.
bool Sem_FoldOp(Ast_NodeKind kind, int64_t lhs, int64_t rhs, Ast_TypeSign sign, Ast_Line line, int64_t *value)
{
    uint64_t ulhs = (uint64_t) lhs;
    uint64_t urhs = (uint64_t) rhs;

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
            Err_AssertAt(line, rhs != 0, ERR_SEM_DIVISION_BY_ZERO);
            if (sign == AST_TYPE_UNSIGNED) {
                *value = (int64_t) (kind == AST_NODE_KIND_DIV ? ulhs / urhs : ulhs % urhs);
            } else {
                *value = kind == AST_NODE_KIND_DIV ? lhs / rhs : lhs % rhs;
            }
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
            *value = sign == AST_TYPE_UNSIGNED ? (int64_t) (ulhs >> rhs) : lhs >> rhs;
        } break;
        case AST_NODE_KIND_EQ: {
            *value = lhs == rhs;
        } break;
        case AST_NODE_KIND_NE: {
            *value = lhs != rhs;
        } break;
        case AST_NODE_KIND_LT: {
            *value = sign == AST_TYPE_UNSIGNED ? ulhs < urhs : lhs < rhs;
        } break;
        case AST_NODE_KIND_LE: {
            *value = sign == AST_TYPE_UNSIGNED ? ulhs <= urhs : lhs <= rhs;
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
            return false;
        }
    }
    return true;
}

// Fold an integer constant expression to its value.
bool Sem_Fold(const Ast_Node *node, int64_t *value)
{
    int64_t lhs = 0;
    int64_t rhs = 0;

    if (! node) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            *value = node->an_val;
        } break;
        case AST_NODE_KIND_SIZEOF: {
            if (! node->an_lhs->an_type) {
                return false;  // the Sem_ pass has not typed the operand yet
            }
            *value = node->an_lhs->an_type->at_size;
        } break;
        case AST_NODE_KIND_CAST: {
            if (! Sem_Fold(node->an_lhs, &lhs)) {
                return false;
            }
            *value = Sem_Truncate(node->an_type, lhs);
        } break;
        case AST_NODE_KIND_COND: {
            if (! Sem_Fold(node->an_cond, &lhs)) {
                return false;
            }
            if (! Sem_Fold(lhs ? node->an_then : node->an_els, value)) {
                return false;
            }
        } break;
        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_NOT:
        case AST_NODE_KIND_BITNOT: {
            if (! Sem_Fold(node->an_lhs, &lhs)) {
                return false;
            }
            if (! Sem_FoldOp(node->an_kind, lhs, 0, Sem_FoldSign(node), node->an_line, value)) {
                return false;
            }
        } break;
        default: {
            if (! Sem_Fold(node->an_lhs, &lhs) || ! Sem_Fold(node->an_rhs, &rhs)) {
                return false;
            }
            if (! Sem_FoldOp(node->an_kind, lhs, rhs, Sem_FoldSign(node), node->an_line, value)) {
                return false;
            }
        } break;
    }
    return true;
}

// Fold an address constant to the symbol it names.
bool Sem_FoldAddr(const Ast_Node *node, const char **symbol)
{
    if (! node) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_STR: {
            *symbol = Str_Format(".Lstr%zu", node->an_str_idx);
        } break;
        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_COMPOUND: {
            if (! node->an_var->av_global) {
                return false;  // a local has no address until its frame exists
            }
            *symbol = node->an_var->av_symbol;
        } break;
        case AST_NODE_KIND_ADDR:
        case AST_NODE_KIND_CAST: {
            if (! Sem_FoldAddr(node->an_lhs, symbol)) {
                return false;
            }
        } break;
        default: {
            return false;
        }
    }
    return true;
}

// Give a function named as a value the pointer type it decays to.
Ast_Type *Sem_FuncAddrType(Ast_Node *node)
{
    Ast_Func *func = Sem_FindFunc(node->an_funcname);
    if (! func) {
        return Ast_NewPointer(&Ast_TypeInt);
    }
    return Ast_NewPointer(Ast_NewFunction(func->af_ret, func->af_params, func->af_nparams, func->af_variadic, func->af_proto));
}

// Give a call the type its callee returns.
Ast_Type *Sem_CallType(Ast_Node *node)
{
    if (! node->an_lhs) {
        Ast_Func *func = Sem_FindFunc(node->an_funcname);
        return func && func->af_ret ? func->af_ret : &Ast_TypeInt;
    }
    Ast_Type *type = Sem_CalleeType(node);
    return type ? type->at_ret : &Ast_TypeInt;
}

// Unwrap the function type an indirect call's callee names.
Ast_Type *Sem_CalleeType(Ast_Node *node)
{
    Ast_Type *type = node->an_lhs->an_type;

    if (type && type->at_kind == AST_TYPE_KIND_PTR) {
        type = type->at_base;
    }
    Err_AssertAt(node->an_line, type && type->at_kind == AST_TYPE_KIND_FUNC, ERR_SEM_CALL_NOT_FUNCTION);
    return type;
}

// Check a call's argument count.
void Sem_CheckArity(Ast_Node *node, int32_t want, Ast_TypeVariadic variadic, Ast_TypeProto proto, const char *what)
{
    int32_t given = Sem_CountNodes(node->an_args);

    if (proto == AST_TYPE_NOPROTO) {
        return;
    }
    if (variadic == AST_TYPE_VARIADIC) {
        Err_AssertAt(node->an_line, given >= want, ERR_SEM_ARGS_TOO_FEW, what, given, want);
        return;
    }
    Err_AssertAt(node->an_line, given == want, ERR_SEM_ARGS_WRONG_COUNT, what, given, want);
}

// Convert a call's arguments to the types its parameters name.
void Sem_ConvertArgs(Ast_Node *node, Ast_Var *params, int32_t nparams, Ast_TypeVariadic variadic, Ast_TypeProto proto)
{
    int32_t i = 0;
    int32_t from = proto == AST_TYPE_PROTO && variadic == AST_TYPE_VARIADIC ? nparams : 0;
    Ast_Node head = {0};
    Ast_Node *tail = &head;
    Ast_Node *arg = node->an_args;
    Ast_Var *param = proto == AST_TYPE_PROTO ? params : NULL;

    while (arg) {
        Ast_Node *next = arg->an_next;
        arg->an_next = NULL;
        if (param && param->av_type) {
            arg = Sem_Convert(arg, param->av_type);
        } else if (i >= from && arg->an_type) {
            arg = Sem_Convert(arg, Sem_Promote(arg->an_type));
        }
        tail->an_next = arg;
        tail = arg;
        arg = next;
        param = param ? param->av_param_next : NULL;
        i++;
    }
    node->an_args = head.an_next;
}

// Check a call and convert its arguments.
void Sem_CheckCall(Ast_Node *node)
{
    if (node->an_lhs) {
        Ast_Type *type = Sem_CalleeType(node);
        Sem_CheckArity(node, type->at_nparams, type->at_variadic, type->at_proto, "a call through a function pointer");
        Sem_ConvertArgs(node, type->at_params, type->at_nparams, type->at_variadic, type->at_proto);
        return;
    }
    Ast_Func *func = Sem_FindFunc(node->an_funcname);
    if (! func) {
        return;
    }
    char *what = Str_Format("'%s'", node->an_funcname);
    Sem_CheckArity(node, func->af_nparams, func->af_variadic, func->af_proto, what);
    Str_Free(what);
    Sem_ConvertArgs(node, func->af_params, func->af_nparams, func->af_variadic, func->af_proto);
}

// Wrap node in a multiplication by size.
Ast_Node *Sem_ScaleBy(Ast_Node *node, int32_t size)
{
    Ast_Node *num = Ast_NewNum(size, node->an_line);
    num->an_type = &Ast_TypeInt;

    Ast_Node *mul = Ast_NewBinary(AST_NODE_KIND_MUL, node, num, node->an_line);
    mul->an_type = &Ast_TypeInt;
    return mul;
}

// Type + and -.
void Sem_Arith(Ast_Node *node)
{
    Ast_Type *lhs = node->an_lhs->an_type;
    Ast_Type *rhs = node->an_rhs->an_type;

    if (! Sem_IsPointer(lhs) && ! Sem_IsPointer(rhs)) {
        Sem_UsualArith(node);
        return;
    }

    if (Sem_IsPointer(lhs) && Sem_IsPointer(rhs)) {
        Err_AssertAt(node->an_line, node->an_kind == AST_NODE_KIND_SUB, ERR_SEM_ADD_POINTERS);

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
        Err_AssertAt(node->an_line, node->an_kind == AST_NODE_KIND_ADD, ERR_SEM_SUB_POINTER_FROM_INT);
        node->an_lhs  = Sem_ScaleBy(node->an_lhs, rhs->at_base->at_size);
        node->an_type = Sem_Decay(rhs);
        return;
    }

    node->an_rhs  = Sem_ScaleBy(node->an_rhs, lhs->at_base->at_size);
    node->an_type = Sem_Decay(lhs);
}

// Return whether the statements under node define a label of this name.
bool Sem_FindLabel(Ast_Node *node, const char *name)
{
    if (! node) {
        return false;
    }
    if (node->an_kind == AST_NODE_KIND_LABEL && strcmp(node->an_funcname, name) == 0) {
        return true;
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
    Err_AssertAt(node->an_line, node->an_kind != AST_NODE_KIND_GOTO || Sem_FindLabel(body, node->an_funcname), ERR_SEM_GOTO_UNDEFINED, node->an_funcname);
    Sem_CheckGotos(node->an_lhs, body);
    Sem_CheckGotos(node->an_then, body);
    Sem_CheckGotos(node->an_els, body);
    Sem_CheckGotos(node->an_body, body);
    Sem_CheckGotos(node->an_next, body);
}

// Attach every case and default of a switch to it.
void Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail)
{
    if (! node || node->an_kind == AST_NODE_KIND_SWITCH) {
        return;
    }

    if (node->an_kind == AST_NODE_KIND_CASE || node->an_kind == AST_NODE_KIND_DEFAULT) {
        for (Ast_Node *seen = sw->an_cases; seen; seen = seen->an_case_next) {
            Err_AssertAt(node->an_line, seen->an_kind != node->an_kind || (node->an_kind != AST_NODE_KIND_DEFAULT && seen->an_val != node->an_val), ERR_SEM_CASE_DUPLICATE);
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

// Annotate a node and everything below it.
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
        case AST_NODE_KIND_NUM: {
            if (! node->an_type) {
                node->an_type = &Ast_TypeInt;
            }
        } break;

        case AST_NODE_KIND_AND:
        case AST_NODE_KIND_OR: {
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_MOD:
        case AST_NODE_KIND_BITAND:
        case AST_NODE_KIND_BITOR:
        case AST_NODE_KIND_BITXOR: {
            Sem_UsualArith(node);
        } break;

        case AST_NODE_KIND_EQ:
        case AST_NODE_KIND_NE:
        case AST_NODE_KIND_LT:
        case AST_NODE_KIND_LE: {
            if (! Sem_IsPointer(node->an_lhs->an_type) && ! Sem_IsPointer(node->an_rhs->an_type)) {
                Sem_UsualArith(node);
            }
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR: {
            Sem_PromoteShift(node);
        } break;

        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_BITNOT: {
            node->an_lhs = Sem_Convert(node->an_lhs, Sem_Promote(node->an_lhs->an_type));
            node->an_type = node->an_lhs->an_type;
        } break;

        case AST_NODE_KIND_NOT: {
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB: {
            Sem_Arith(node);
        } break;

        case AST_NODE_KIND_STR: {
            Ast_Str *str = Ast_StringAt(node->an_str_idx);
            Ast_Type *elem = str->as_width > AST_TYPE_SIZE_CHAR ? &Ast_TypeInt : &Ast_TypeChar;
            node->an_type = Ast_NewArray(elem, (int32_t) (str->as_len / str->as_width + 1));
        } break;

        case AST_NODE_KIND_ADDR: {
            if (node->an_lhs->an_kind == AST_NODE_KIND_FUNCADDR) {
                node->an_type = node->an_lhs->an_type;
                break;
            }
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_ADDRESS_NOT_LVALUE);
            Err_AssertAt(node->an_line, node->an_lhs->an_kind != AST_NODE_KIND_MEMBER || ! node->an_lhs->an_member->am_bits, ERR_SEM_ADDRESS_BITFIELD);
            node->an_type = Ast_NewPointer(node->an_lhs->an_type);
        } break;

        case AST_NODE_KIND_DEREF: {
            if (node->an_lhs->an_type && node->an_lhs->an_type->at_kind == AST_TYPE_KIND_FUNC) {
                node->an_type = node->an_lhs->an_type;
                break;
            }
            Err_AssertAt(node->an_line, Sem_IsPointer(node->an_lhs->an_type), ERR_SEM_DEREF_NOT_POINTER);
            Err_AssertAt(node->an_line, node->an_lhs->an_type->at_base->at_kind != AST_TYPE_KIND_VOID, ERR_SEM_DEREF_VOID);
            Err_AssertAt(node->an_line, node->an_lhs->an_type->at_base->at_complete, ERR_SEM_DEREF_INCOMPLETE);
            node->an_type = node->an_lhs->an_type->at_base;
        } break;

        case AST_NODE_KIND_MEMBER: {
            Ast_Type *type = node->an_lhs->an_type;
            Err_AssertAt(node->an_line, Sem_IsAggregate(type), ERR_SEM_MEMBER_NOT_AGGREGATE, node->an_memname);
            Err_AssertAt(node->an_line, type->at_complete, ERR_SEM_MEMBER_INCOMPLETE, Sem_TypeName(type));
            node->an_member = Ast_FindMember(type, node->an_memname);
            Err_AssertAt(node->an_line, node->an_member, ERR_SEM_MEMBER_UNKNOWN, node->an_memname, Sem_TypeName(type));
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
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
            Err_AssertAt(node->an_line, node->an_lhs->an_type->at_kind != AST_TYPE_KIND_ARRAY, ERR_SEM_ASSIGN_ARRAY);
            Err_AssertAt(node->an_line, ! Sem_IsAggregate(node->an_lhs->an_type) || Sem_SameType(node->an_lhs->an_type, node->an_rhs->an_type), ERR_SEM_ASSIGN_AGGREGATE_MISMATCH);
            node->an_type = node->an_lhs->an_type;
            node->an_rhs  = Sem_Convert(node->an_rhs, node->an_type);
        } break;

        case AST_NODE_KIND_OPASSIGN: {
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
            Ast_Type *type = node->an_lhs->an_type;
            if (Sem_IsPointer(type) && (node->an_op == AST_NODE_KIND_ADD || node->an_op == AST_NODE_KIND_SUB)) {
                node->an_rhs = Sem_ScaleBy(node->an_rhs, type->at_base->at_size);
            }
            node->an_type = type;
        } break;

        case AST_NODE_KIND_POSTINC: {
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
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
            Sem_CheckCall(node);
            node->an_type = Sem_CallType(node);
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            node->an_type = Sem_FuncAddrType(node);
        } break;

        case AST_NODE_KIND_VA_START: {
            Err_AssertAt(node->an_line, Sem_CurFunc->af_variadic != AST_TYPE_FIXED, ERR_SEM_VA_START_FIXED);
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_VA_ARG: {
            // the parser already set an_type from the type it names
        } break;

        case AST_NODE_KIND_CASE: {
            Err_AssertAt(node->an_line, Sem_Fold(node->an_cond, &node->an_val), ERR_SEM_CASE_NOT_CONSTANT);
        } break;

        case AST_NODE_KIND_SWITCH: {
            Ast_Node *tail = NULL;
            Sem_CollectCases(node->an_body, node, &tail);
        } break;

        case AST_NODE_KIND_RETURN: {
            if (node->an_lhs && Sem_CurFunc->af_ret) {
                node->an_lhs = Sem_Convert(node->an_lhs, Sem_CurFunc->af_ret);
            }
        } break;

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
        case AST_NODE_KIND_NOP:
        case AST_NODE_KIND_COUNT: {
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
