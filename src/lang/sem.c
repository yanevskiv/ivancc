/*
 * C source file for semantic analysis.
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
#include "lang/sem.h"

// The function whose body is being analysed.
static Ast_Func *Sem_CurFunc;

// Serial number of the next temporary a rewrite declares.
static int32_t Sem_TempCount;

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

// Return whether a type is const or holds a const member or element.
bool Sem_HoldsConst(const Ast_Type *type)
{
    if (type->at_qual & AST_QUAL_CONST) {
        return true;
    }
    if (type->at_kind == AST_TYPE_KIND_ARRAY) {
        return Sem_HoldsConst(type->at_base);
    }
    if (Sem_IsAggregate(type)) {
        for (const Ast_Member *member = type->at_members; member; member = member->am_next) {
            if (Sem_HoldsConst(member->am_type)) {
                return true;
            }
        }
    }
    return false;
}

// Return whether an lvalue of this type may be stored to.
bool Sem_IsModifiable(const Ast_Type *type)
{
    return type->at_kind != AST_TYPE_KIND_ARRAY && ! Sem_HoldsConst(type);
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

// Return the type a default argument promotion gives an operand.
Ast_Type *Sem_PromoteArg(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_FLOAT) {
        return &Ast_TypeDouble;
    }
    return Sem_Promote(type);
}

// Return the type two arithmetic operands meet in.
Ast_Type *Sem_CommonType(Ast_Type *lhs, Ast_Type *rhs)
{
    lhs = Sem_Promote(lhs);
    rhs = Sem_Promote(rhs);

    if (Ast_IsFloating(lhs) && Ast_IsFloating(rhs)) {
        return lhs->at_kind >= rhs->at_kind ? lhs : rhs;
    }
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

// Turn a floating value tested for truth into its comparison with zero.
Ast_Node *Sem_Truth(Ast_Node *node)
{
    if (! node || ! node->an_type || ! Ast_IsFloating(node->an_type)) {
        return node;
    }
    return Sem_NewBinary(AST_NODE_KIND_NE, node, Ast_NewFNum(0, node->an_type, node->an_line), node->an_line);
}

// Return the type a value of this type has once arrays and functions decay.
Ast_Type *Sem_ValueType(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        return Ast_NewPointer(type);
    }
    return Sem_Decay(type);
}

// Return whether a node is a null pointer constant.
bool Sem_IsNullPointer(const Ast_Node *node)
{
    int64_t value = 0;
    const Ast_Type *type = node->an_type;

    if (! type) {
        return false;
    }
    if (node->an_kind == AST_NODE_KIND_CAST && type->at_kind == AST_TYPE_KIND_PTR && type->at_base->at_kind == AST_TYPE_KIND_VOID) {
        return Sem_IsNullPointer(node->an_lhs);
    }
    return Ast_IsInteger(type) && Sem_Fold(node, &value) && value == 0;
}

// Refuse a value that simple assignment cannot convert to type to.
void Sem_CheckAssign(const Ast_Type *to, const Ast_Node *from, const char *what, Ast_Line line)
{
    if (! from->an_type) {
        return;
    }
    Err_AssertAt(line, from->an_type->at_kind != AST_TYPE_KIND_VOID, ERR_SEM_VOID_VALUE);

    const Ast_Type *type = Sem_ValueType(from->an_type);
    if (Ast_IsArithmetic(to) && Ast_IsArithmetic(type)) {
        return;
    }
    if (to->at_kind == AST_TYPE_KIND_BOOL && type->at_kind == AST_TYPE_KIND_PTR) {
        return;
    }
    if (Sem_IsAggregate(to) || Sem_IsAggregate(type)) {
        Err_AssertAt(line, Ast_IsCompatibleUnqualified(to, type), ERR_SEM_ASSIGN_INCOMPATIBLE, what);
        return;
    }
    if (to->at_kind == AST_TYPE_KIND_PTR && Sem_IsNullPointer(from)) {
        return;
    }
    Err_AssertAt(line, to->at_kind == AST_TYPE_KIND_PTR && type->at_kind == AST_TYPE_KIND_PTR, ERR_SEM_ASSIGN_INCOMPATIBLE, what);

    const Ast_Type *want = to->at_base;
    const Ast_Type *have = type->at_base;
    bool voids = want->at_kind == AST_TYPE_KIND_VOID || have->at_kind == AST_TYPE_KIND_VOID;
    Err_AssertAt(line, voids || Ast_IsCompatibleUnqualified(want, have), ERR_SEM_ASSIGN_INCOMPATIBLE, what);
    Err_AssertAt(line, ! (have->at_qual & ~want->at_qual), ERR_SEM_ASSIGN_DISCARDS_QUALIFIER, what);
}

// Return the type a conditional with an operand that is not arithmetic yields.
Ast_Type *Sem_CondType(const Ast_Node *node)
{
    Ast_Type *then = Sem_ValueType(node->an_then->an_type);
    Ast_Type *els = Sem_ValueType(node->an_els->an_type);

    if (then->at_kind == AST_TYPE_KIND_VOID || els->at_kind == AST_TYPE_KIND_VOID) {
        Err_AssertAt(node->an_line, then->at_kind == els->at_kind, ERR_SEM_COND_MISMATCH);
        return then;
    }
    if (Sem_IsAggregate(then) || Sem_IsAggregate(els)) {
        Err_AssertAt(node->an_line, Ast_IsCompatibleUnqualified(then, els), ERR_SEM_COND_MISMATCH);
        return then;
    }
    if (then->at_kind == AST_TYPE_KIND_PTR && Sem_IsNullPointer(node->an_els)) {
        return then;
    }
    if (els->at_kind == AST_TYPE_KIND_PTR && Sem_IsNullPointer(node->an_then)) {
        return els;
    }
    Err_AssertAt(node->an_line, then->at_kind == AST_TYPE_KIND_PTR && els->at_kind == AST_TYPE_KIND_PTR, ERR_SEM_COND_MISMATCH);
    if (els->at_base->at_kind == AST_TYPE_KIND_VOID) {
        return els;
    }
    Err_AssertAt(node->an_line, then->at_base->at_kind == AST_TYPE_KIND_VOID || Ast_IsCompatibleUnqualified(then->at_base, els->at_base), ERR_SEM_COND_MISMATCH);
    return then;
}

// Wrap a node in the cast that converts it to type.
Ast_Node *Sem_Convert(Ast_Node *node, Ast_Type *type)
{
    if (! node || ! node->an_type || Sem_SameType(node->an_type, type)) {
        return node;
    }
    bool test = type->at_kind == AST_TYPE_KIND_BOOL && Sem_IsPointer(node->an_type);
    if (! test && (! Ast_IsArithmetic(node->an_type) || ! Ast_IsArithmetic(type))) {
        return node;
    }
    if (type->at_kind == AST_TYPE_KIND_BOOL) {
        node = Sem_Truth(node);
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
        case AST_TYPE_KIND_FLOAT:
        case AST_TYPE_KIND_DOUBLE:
        case AST_TYPE_KIND_LDOUBLE:
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

// Return the type a constant expression folds to.
Ast_Type *Sem_FoldType(const Ast_Node *node)
{
    if (node->an_type) {
        return node->an_type;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_BITNOT:
        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR: {
            return Sem_Promote(Sem_FoldType(node->an_lhs));
        } break;
        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB:
        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_MOD:
        case AST_NODE_KIND_BITAND:
        case AST_NODE_KIND_BITOR:
        case AST_NODE_KIND_BITXOR: {
            return Sem_CommonType(Sem_FoldType(node->an_lhs), Sem_FoldType(node->an_rhs));
        } break;
        case AST_NODE_KIND_COND: {
            return Sem_CommonType(Sem_FoldType(node->an_then), Sem_FoldType(node->an_els));
        } break;
        case AST_NODE_KIND_SIZEOF: {
            return &Ast_TypeULong;
        } break;
        default: {
            return &Ast_TypeInt;
        }
    }
}

// Return the type an operator's operands fold in.
Ast_Type *Sem_FoldOperandType(const Ast_Node *node)
{
    if (! node->an_rhs || node->an_kind == AST_NODE_KIND_SHL || node->an_kind == AST_NODE_KIND_SHR) {
        return Sem_Promote(Sem_FoldType(node->an_lhs));
    }
    return Sem_CommonType(Sem_FoldType(node->an_lhs), Sem_FoldType(node->an_rhs));
}

// Apply one operator to folded operands.
bool Sem_FoldOp(Ast_NodeKind kind, int64_t lhs, int64_t rhs, Ast_TypeSign sign, Ast_Line line, int64_t *value)
{
    uint64_t ulhs = (uint64_t) lhs;
    uint64_t urhs = (uint64_t) rhs;

    switch (kind) {
        case AST_NODE_KIND_ADD: {
            *value = (int64_t) (ulhs + urhs);
        } break;
        case AST_NODE_KIND_SUB: {
            *value = (int64_t) (ulhs - urhs);
        } break;
        case AST_NODE_KIND_MUL: {
            *value = (int64_t) (ulhs * urhs);
        } break;
        case AST_NODE_KIND_DIV:
        case AST_NODE_KIND_MOD: {
            Err_AssertAt(line, rhs != 0, ERR_SEM_DIVISION_BY_ZERO);
            if (sign == AST_TYPE_UNSIGNED) {
                *value = (int64_t) (kind == AST_NODE_KIND_DIV ? ulhs / urhs : ulhs % urhs);
            } else if (rhs == -1) {
                *value = kind == AST_NODE_KIND_DIV ? (int64_t) (0 - ulhs) : 0;
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
            *value = (int64_t) (ulhs << rhs);
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
            *value = (int64_t) (0 - ulhs);
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

// Fold an integer a cast or a comparison makes of floating operands.
bool Sem_FoldFromFloat(const Ast_Node *node, int64_t *value)
{
    long double lhs = 0;
    long double rhs = 0;

    if (! Sem_FoldFloat(node->an_lhs, &lhs)) {
        return false;
    }
    if (node->an_kind == AST_NODE_KIND_CAST) {
        bool high = node->an_type->at_sign == AST_TYPE_UNSIGNED && lhs >= SEM_TWO_TO_63;
        *value = Sem_Truncate(node->an_type, high ? (int64_t) (uint64_t) lhs : (int64_t) lhs);
        return true;
    }
    if (! Sem_FoldFloat(node->an_rhs, &rhs)) {
        return false;
    }

    switch (node->an_kind) {
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
    if (node->an_kind != AST_NODE_KIND_SIZEOF && node->an_lhs && node->an_lhs->an_type && Ast_IsFloating(node->an_lhs->an_type)) {
        return Sem_FoldFromFloat(node, value);
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            *value = node->an_val;
        } break;
        case AST_NODE_KIND_SIZEOF: {
            if (! node->an_lhs->an_type) {
                return false;  // the Sem_ pass has not typed the operand yet
            }
            if (Ast_IsVla(node->an_lhs->an_type)) {
                return false;
            }
            *value = node->an_lhs->an_type->at_size;
        } break;
        case AST_NODE_KIND_CAST: {
            if (Ast_IsFloating(node->an_type) || ! Sem_Fold(node->an_lhs, &lhs)) {
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
            Ast_Type *type = Sem_FoldOperandType(node);
            if (! Sem_FoldOp(node->an_kind, Sem_Truncate(type, lhs), 0, type->at_sign, node->an_line, value)) {
                return false;
            }
        } break;
        default: {
            if (! Sem_Fold(node->an_lhs, &lhs) || ! Sem_Fold(node->an_rhs, &rhs)) {
                return false;
            }
            Ast_Type *type = Sem_FoldOperandType(node);
            if (! Sem_FoldOp(node->an_kind, Sem_Truncate(type, lhs), Sem_Truncate(type, rhs), type->at_sign, node->an_line, value)) {
                return false;
            }
        } break;
    }
    *value = Sem_Truncate(Sem_FoldType(node), *value);
    return true;
}

// Round a folded value to the precision a floating type holds.
long double Sem_RoundFloat(const Ast_Type *type, long double value)
{
    switch (type->at_kind) {
        case AST_TYPE_KIND_FLOAT: {
            return (float) value;
        } break;
        case AST_TYPE_KIND_DOUBLE: {
            return (double) value;
        } break;
        default: {
            return value;
        }
    }
}

// Apply one arithmetic operator to folded doubles.
double Sem_FoldDoubleOp(Ast_NodeKind kind, double lhs, double rhs)
{
    switch (kind) {
        case AST_NODE_KIND_ADD: {
            return lhs + rhs;
        } break;
        case AST_NODE_KIND_SUB: {
            return lhs - rhs;
        } break;
        case AST_NODE_KIND_MUL: {
            return lhs * rhs;
        } break;
        default: {
            return lhs / rhs;
        }
    }
}

// Apply one arithmetic operator to folded operands of a floating type.
long double Sem_FoldFloatOp(Ast_NodeKind kind, const Ast_Type *type, long double lhs, long double rhs)
{
    if (type->at_kind == AST_TYPE_KIND_DOUBLE) {
        return Sem_FoldDoubleOp(kind, (double) lhs, (double) rhs);
    }

    switch (kind) {
        case AST_NODE_KIND_ADD: {
            return Sem_RoundFloat(type, lhs + rhs);
        } break;
        case AST_NODE_KIND_SUB: {
            return Sem_RoundFloat(type, lhs - rhs);
        } break;
        case AST_NODE_KIND_MUL: {
            return Sem_RoundFloat(type, lhs * rhs);
        } break;
        default: {
            return Sem_RoundFloat(type, lhs / rhs);
        }
    }
}

// Fold an arithmetic constant expression to its value as a long double.
bool Sem_FoldFloat(const Ast_Node *node, long double *value)
{
    int64_t cond = 0;
    long double lhs = 0;
    long double rhs = 0;

    if (! node || ! node->an_type) {
        return false;
    }
    if (Ast_IsInteger(node->an_type)) {
        if (! Sem_Fold(node, &cond)) {
            return false;
        }
        *value = node->an_type->at_sign == AST_TYPE_UNSIGNED ? (long double) (uint64_t) cond : (long double) cond;
        return true;
    }
    if (! Ast_IsFloating(node->an_type)) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_FNUM: {
            *value = Sem_RoundFloat(node->an_type, node->an_fval);
        } break;
        case AST_NODE_KIND_CAST: {
            if (! Sem_FoldFloat(node->an_lhs, &lhs)) {
                return false;
            }
            *value = Sem_RoundFloat(node->an_type, lhs);
        } break;
        case AST_NODE_KIND_NEG: {
            if (! Sem_FoldFloat(node->an_lhs, &lhs)) {
                return false;
            }
            *value = -lhs;
        } break;
        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB:
        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV: {
            if (! Sem_FoldFloat(node->an_lhs, &lhs) || ! Sem_FoldFloat(node->an_rhs, &rhs)) {
                return false;
            }
            *value = Sem_FoldFloatOp(node->an_kind, node->an_type, lhs, rhs);
        } break;
        case AST_NODE_KIND_COND: {
            if (! Sem_Fold(node->an_cond, &cond)) {
                return false;
            }
            if (! Sem_FoldFloat(cond ? node->an_then : node->an_els, value)) {
                return false;
            }
        } break;
        default: {
            return false;
        }
    }
    return true;
}

// Fold the address of an object with static storage to a symbol and an offset.
bool Sem_FoldObject(const Ast_Node *node, const char **symbol, int64_t *addend)
{
    if (! node) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_STR: {
            *symbol = Str_Format(".Lstr%zu", node->an_stridx);
            *addend = 0;
        } break;
        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_COMPOUND: {
            if (! node->an_var->av_global) {
                return false;
            }
            *symbol = node->an_var->av_symbol;
            *addend = 0;
        } break;
        case AST_NODE_KIND_MEMBER: {
            if (! node->an_member || node->an_member->am_bits || ! Sem_FoldObject(node->an_lhs, symbol, addend)) {
                return false;
            }
            *addend += node->an_member->am_offset;
        } break;
        case AST_NODE_KIND_DEREF: {
            return Sem_FoldAddr(node->an_lhs, symbol, addend);
        } break;
        default: {
            return false;
        }
    }
    return true;
}

// Fold an address constant to the symbol it points into and an offset.
bool Sem_FoldAddr(const Ast_Node *node, const char **symbol, int64_t *addend)
{
    int64_t value = 0;

    if (! node || ! node->an_type) {
        return false;
    }

    switch (node->an_kind) {
        case AST_NODE_KIND_STR:
        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_COMPOUND:
        case AST_NODE_KIND_MEMBER:
        case AST_NODE_KIND_DEREF: {
            return node->an_type->at_kind == AST_TYPE_KIND_ARRAY && Sem_FoldObject(node, symbol, addend);
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            *symbol = node->an_funcname;
            *addend = 0;
        } break;
        case AST_NODE_KIND_ADDR: {
            if (node->an_lhs->an_kind == AST_NODE_KIND_FUNCADDR) {
                return Sem_FoldAddr(node->an_lhs, symbol, addend);
            }
            return Sem_FoldObject(node->an_lhs, symbol, addend);
        } break;
        case AST_NODE_KIND_CAST: {
            bool wide = Ast_IsInteger(node->an_type) && node->an_type->at_size == Ast_TypeLong.at_size;
            return (Sem_IsPointer(node->an_type) || wide) && Sem_FoldAddr(node->an_lhs, symbol, addend);
        } break;
        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB: {
            bool left = Sem_IsPointer(node->an_lhs->an_type);
            const Ast_Node *base = left ? node->an_lhs : node->an_rhs;
            const Ast_Node *offset = left ? node->an_rhs : node->an_lhs;
            if (! Sem_IsPointer(base->an_type) || ! Sem_FoldAddr(base, symbol, addend) || ! Sem_Fold(offset, &value)) {
                return false;
            }
            *addend += node->an_kind == AST_NODE_KIND_ADD ? value : -value;
        } break;
        case AST_NODE_KIND_COND: {
            if (! Sem_Fold(node->an_cond, &value)) {
                return false;
            }
            return Sem_FoldAddr(value ? node->an_then : node->an_els, symbol, addend);
        } break;
        default: {
            return false;
        }
    }
    return true;
}

// Give a call the type its callee returns.
Ast_Type *Sem_CallType(Ast_Node *node)
{
    Ast_Type *type = Sem_CalleeType(node);
    return type ? type->at_ret : &Ast_TypeInt;
}

// Unwrap the function type a call's callee names.
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
void Sem_CheckArity(Ast_Node *node, int32_t want, Ast_TypeVa va, Ast_TypeProto proto, const char *what)
{
    int32_t given = Sem_CountNodes(node->an_args);

    if (proto == AST_TYPE_NOPROTO) {
        return;
    }
    if (va == AST_TYPE_VA) {
        Err_AssertAt(node->an_line, given >= want, ERR_SEM_ARGS_TOO_FEW, what, given, want);
        return;
    }
    Err_AssertAt(node->an_line, given == want, ERR_SEM_ARGS_WRONG_COUNT, what, given, want);
}

// Convert a call's arguments to the types its parameters name.
void Sem_ConvertArgs(Ast_Node *node, Ast_Var *params, int32_t nparams, Ast_TypeVa va, Ast_TypeProto proto)
{
    int32_t i = 0;
    int32_t from = proto == AST_TYPE_PROTO && va == AST_TYPE_VA ? nparams : 0;
    Ast_Node head = {0};
    Ast_Node *tail = &head;
    Ast_Node *arg = node->an_args;
    Ast_Var *param = proto == AST_TYPE_PROTO ? params : NULL;

    while (arg) {
        Ast_Node *next = arg->an_next;
        arg->an_next = NULL;
        if (param && param->av_type) {
            Sem_CheckAssign(param->av_type, arg, "argument passing", arg->an_line);
            arg = Sem_Convert(arg, param->av_type);
        } else if (i >= from && arg->an_type) {
            arg = Sem_Convert(arg, Sem_PromoteArg(arg->an_type));
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
    Ast_Type *type = Sem_CalleeType(node);
    bool direct = node->an_lhs->an_kind == AST_NODE_KIND_FUNCADDR;
    char *what = direct ? Str_Format("'%s'", node->an_lhs->an_funcname) : Str_Clone("a call through a function pointer");

    Sem_CheckArity(node, type->at_nparams, type->at_va, type->at_proto, what);
    Str_Free(what);
    Sem_ConvertArgs(node, type->at_params, type->at_nparams, type->at_va, type->at_proto);
}

// Declare a nameless local of the function being analysed.
Ast_Var *Sem_NewTemp(Ast_Type *type, Ast_Line line)
{
    Ast_Var *var = calloc(1, sizeof(Ast_Var));

    var->av_name = Str_Format(".tmp.%d", Sem_TempCount++);
    var->av_type = type;
    var->av_line = line;
    var->av_next = Sem_CurFunc->af_locals;
    Sem_CurFunc->af_locals = var;
    return var;
}

// Build and annotate a unary node over an annotated operand.
Ast_Node *Sem_NewUnary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(kind, lhs, line);
    Sem_Annotate(node);
    return node;
}

// Build and annotate a binary node over annotated operands.
Ast_Node *Sem_NewBinary(Ast_NodeKind kind, Ast_Node *lhs, Ast_Node *rhs, Ast_Line line)
{
    Ast_Node *node = Ast_NewBinary(kind, lhs, rhs, line);
    Sem_Annotate(node);
    return node;
}

// Build and annotate a reference to a temporary.
Ast_Node *Sem_TempRef(Ast_Var *var, Ast_Line line)
{
    Ast_Node *node = Ast_NewVarNode(var, line);
    Sem_Annotate(node);
    return node;
}

// Build the assignment that parks the address of an lvalue in a new pointer.
Ast_Node *Sem_PinLvalue(Ast_Node *lhs, Ast_Var **ptr)
{
    bool bits = lhs->an_kind == AST_NODE_KIND_MEMBER && lhs->an_member->am_bits;
    Ast_Node *base = bits ? lhs->an_lhs : lhs;

    *ptr = Sem_NewTemp(Ast_NewPointer(base->an_type), lhs->an_line);
    return Sem_NewBinary(AST_NODE_KIND_ASSIGN, Sem_TempRef(*ptr, lhs->an_line), Sem_NewUnary(AST_NODE_KIND_ADDR, base, lhs->an_line), lhs->an_line);
}

// Build the lvalue a pinned pointer reaches again.
Ast_Node *Sem_Target(Ast_Node *lhs, Ast_Var *ptr)
{
    Ast_Node *target = Sem_NewUnary(AST_NODE_KIND_DEREF, Sem_TempRef(ptr, lhs->an_line), lhs->an_line);

    if (lhs->an_kind == AST_NODE_KIND_MEMBER && lhs->an_member->am_bits) {
        target = Ast_NewMemberNode(target, lhs->an_memname, lhs->an_line);
        Sem_Annotate(target);
    }
    return target;
}

// Return whether an assignment's floating operand calls for a rewrite.
bool Sem_NeedsFloatAssign(const Ast_Node *node)
{
    if (! Sem_CurFunc) {
        return false;
    }
    if (Ast_IsFloating(node->an_lhs->an_type)) {
        return true;
    }
    return node->an_kind == AST_NODE_KIND_OPASSIGN && Ast_IsFloating(node->an_rhs->an_type);
}

// Put another node's contents in place of a node, keeping its place in a list.
void Sem_Replace(Ast_Node *node, const Ast_Node *with)
{
    Ast_Node *next = node->an_next;

    *node = *with;
    node->an_next = next;
}

// Rewrite `lhs op= rhs` as `p = &lhs, *p = *p op rhs`.
void Sem_LowerOpAssign(Ast_Node *node)
{
    Ast_Var *ptr = NULL;
    Ast_Line line = node->an_line;
    Ast_Node *set = Sem_PinLvalue(node->an_lhs, &ptr);
    Ast_Node *value = Sem_NewBinary(node->an_op, Sem_Target(node->an_lhs, ptr), node->an_rhs, line);
    Ast_Node *store = Sem_NewBinary(AST_NODE_KIND_ASSIGN, Sem_Target(node->an_lhs, ptr), value, line);

    Sem_Replace(node, Sem_NewBinary(AST_NODE_KIND_COMMA, set, store, line));
}

// Rewrite `lhs++` as `p = &lhs, old = *p, *p = old + 1, old`.
void Sem_LowerPostInc(Ast_Node *node)
{
    Ast_Var *ptr = NULL;
    Ast_Line line = node->an_line;
    Ast_Node *step = Ast_NewNum(node->an_step, line);
    Ast_Var *old = Sem_NewTemp(node->an_lhs->an_type, line);
    Ast_Node *set = Sem_PinLvalue(node->an_lhs, &ptr);

    Sem_Annotate(step);
    Ast_Node *save = Sem_NewBinary(AST_NODE_KIND_ASSIGN, Sem_TempRef(old, line), Sem_Target(node->an_lhs, ptr), line);
    Ast_Node *sum = Sem_NewBinary(AST_NODE_KIND_ADD, Sem_TempRef(old, line), step, line);
    Ast_Node *store = Sem_NewBinary(AST_NODE_KIND_ASSIGN, Sem_Target(node->an_lhs, ptr), sum, line);
    Ast_Node *first = Sem_NewBinary(AST_NODE_KIND_COMMA, set, save, line);
    Ast_Node *second = Sem_NewBinary(AST_NODE_KIND_COMMA, store, Sem_TempRef(old, line), line);

    Sem_Replace(node, Sem_NewBinary(AST_NODE_KIND_COMMA, first, second, line));
}

// Build the bytes one step of a pointer or array moves.
Ast_Node *Sem_Stride(const Ast_Type *type, Ast_Line line)
{
    if (Ast_IsVla(type->at_base)) {
        return Sem_Convert(Sem_TempRef(type->at_base->at_vsize, line), &Ast_TypeLong);
    }
    Ast_Node *num = Ast_NewNum(type->at_base->at_size, line);
    num->an_type = &Ast_TypeInt;
    return num;
}

// Wrap node in a multiplication by size.
Ast_Node *Sem_ScaleBy(Ast_Node *node, Ast_Node *size)
{
    Ast_Node *mul = Ast_NewBinary(AST_NODE_KIND_MUL, node, size, node->an_line);
    mul->an_type = size->an_type;
    return mul;
}

// Type + and -.
void Sem_Arith(Ast_Node *node)
{
    Ast_Type *lhs = node->an_lhs->an_type;
    Ast_Type *rhs = node->an_rhs->an_type;

    if (! Sem_IsPointer(lhs) && ! Sem_IsPointer(rhs)) {
        Sem_NeedArithmetic(node);
        Sem_UsualArith(node);
        return;
    }
    Err_AssertAt(node->an_line, ! Ast_IsFloating(lhs) && ! Ast_IsFloating(rhs), ERR_SEM_POINTER_FLOATING);
    Err_AssertAt(node->an_line, (Sem_IsPointer(lhs) || Ast_IsInteger(lhs)) && (Sem_IsPointer(rhs) || Ast_IsInteger(rhs)), ERR_SEM_OPERAND_NOT_INTEGER);

    if (Sem_IsPointer(lhs) && Sem_IsPointer(rhs)) {
        Err_AssertAt(node->an_line, node->an_kind == AST_NODE_KIND_SUB, ERR_SEM_ADD_POINTERS);

        Ast_Node *diff = Ast_NewBinary(AST_NODE_KIND_SUB, node->an_lhs, node->an_rhs, node->an_line);
        diff->an_type = &Ast_TypeLong;

        node->an_kind = AST_NODE_KIND_DIV;
        node->an_lhs  = diff;
        node->an_rhs  = Sem_Convert(Sem_Stride(lhs, node->an_line), &Ast_TypeLong);
        node->an_type = &Ast_TypeLong;
        return;
    }

    if (Sem_IsPointer(rhs)) {
        Err_AssertAt(node->an_line, node->an_kind == AST_NODE_KIND_ADD, ERR_SEM_SUB_POINTER_FROM_INT);
        node->an_lhs  = Sem_ScaleBy(node->an_lhs, Sem_Stride(rhs, node->an_line));
        node->an_type = Sem_Decay(rhs);
        return;
    }

    node->an_rhs  = Sem_ScaleBy(node->an_rhs, Sem_Stride(lhs, node->an_line));
    node->an_type = Sem_Decay(lhs);
}

// Reject a floating operand of an operator that takes only integers.
void Sem_NeedInteger(const Ast_Node *node)
{
    bool lhs = node->an_lhs && Ast_IsFloating(node->an_lhs->an_type);
    bool rhs = node->an_rhs && Ast_IsFloating(node->an_rhs->an_type);

    Err_AssertAt(node->an_line, ! lhs && ! rhs, ERR_SEM_OPERAND_FLOATING);

    bool ilhs = ! node->an_lhs || Ast_IsInteger(node->an_lhs->an_type);
    bool irhs = ! node->an_rhs || Ast_IsInteger(node->an_rhs->an_type);
    Err_AssertAt(node->an_line, ilhs && irhs, ERR_SEM_OPERAND_NOT_INTEGER);
}

// Reject an operand of an operator that takes only numbers.
void Sem_NeedArithmetic(const Ast_Node *node)
{
    bool lhs = ! node->an_lhs || Ast_IsArithmetic(node->an_lhs->an_type);
    bool rhs = ! node->an_rhs || Ast_IsArithmetic(node->an_rhs->an_type);

    Err_AssertAt(node->an_line, lhs && rhs, ERR_SEM_OPERAND_NOT_ARITHMETIC);
}

// Return whether a value of this type is a number or a pointer.
bool Sem_IsScalar(const Ast_Type *type)
{
    return Ast_IsArithmetic(type) || Sem_IsPointer(type) || type->at_kind == AST_TYPE_KIND_FUNC;
}

// Reject an operand tested for truth that is neither a number nor a pointer.
void Sem_NeedScalar(const Ast_Node *operand, Ast_Line line)
{
    Err_AssertAt(line, ! operand || ! operand->an_type || Sem_IsScalar(operand->an_type), ERR_SEM_OPERAND_NOT_SCALAR);
}

// Reject operands a compound assignment's operator cannot take.
void Sem_CheckOpAssign(const Ast_Node *node)
{
    const Ast_Type *lhs = node->an_lhs->an_type;
    const Ast_Type *rhs = node->an_rhs->an_type;

    switch (node->an_op) {
        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB: {
            bool step = lhs->at_kind == AST_TYPE_KIND_PTR && Ast_IsInteger(rhs);
            Err_AssertAt(node->an_line, step || (Ast_IsArithmetic(lhs) && Ast_IsArithmetic(rhs)), ERR_SEM_OPERAND_NOT_ARITHMETIC);
        } break;
        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV: {
            Sem_NeedArithmetic(node);
        } break;
        default: {
            Sem_NeedInteger(node);
        }
    }
}

// Reject a cast between a floating type and a non-arithmetic one.
void Sem_CheckCast(Ast_Node *node)
{
    Ast_Type *from = node->an_lhs->an_type;
    Ast_Type *to = node->an_type;

    if (! from || to->at_kind == AST_TYPE_KIND_VOID) {
        return;
    }
    Err_AssertAt(node->an_line, Sem_IsScalar(to) && Sem_IsScalar(from), ERR_SEM_CAST_NOT_SCALAR);
    Err_AssertAt(node->an_line, ! Ast_IsFloating(to) || Ast_IsArithmetic(from), ERR_SEM_CAST_FLOATING);
    Err_AssertAt(node->an_line, ! Ast_IsFloating(from) || Ast_IsArithmetic(to), ERR_SEM_CAST_FLOATING);
    if (to->at_kind == AST_TYPE_KIND_BOOL) {
        node->an_lhs = Sem_Truth(node->an_lhs);
    }
}

// Return the label of this name the statements under node define.
Ast_Node *Sem_FindLabel(Ast_Node *node, const char *name)
{
    Ast_Node *found = NULL;

    if (! node) {
        return NULL;
    }
    if (node->an_kind == AST_NODE_KIND_LABEL && Str_Equals(node->an_funcname, name)) {
        return node;
    }

    Ast_Node *kids[] = { node->an_lhs, node->an_then, node->an_els, node->an_body, node->an_next };
    for (size_t i = 0; i < sizeof(kids) / sizeof(kids[0]) && ! found; i++) {
        found = Sem_FindLabel(kids[i], name);
    }
    return found;
}

// Reject a goto to a label never defined or inside a variably modified scope.
void Sem_CheckGotos(Ast_Node *node, Ast_Node *body)
{
    if (! node) {
        return;
    }
    if (node->an_kind == AST_NODE_KIND_GOTO) {
        Ast_Node *label = Sem_FindLabel(body, node->an_funcname);
        Err_AssertAt(node->an_line, label != NULL, ERR_SEM_GOTO_UNDEFINED, node->an_funcname);
        Err_AssertAt(node->an_line, Ast_ContainsVmScope(node->an_vm, label->an_vm), ERR_SEM_GOTO_INTO_VM_SCOPE, node->an_funcname);
    }
    Sem_CheckGotos(node->an_lhs, body);
    Sem_CheckGotos(node->an_then, body);
    Sem_CheckGotos(node->an_els, body);
    Sem_CheckGotos(node->an_body, body);
    Sem_CheckGotos(node->an_next, body);
}

// Attach every case and default of a switch to it, each case converted to its type.
void Sem_CollectCases(Ast_Node *node, Ast_Node *sw, Ast_Node **tail)
{
    if (! node || node->an_kind == AST_NODE_KIND_SWITCH) {
        return;
    }

    if (node->an_kind == AST_NODE_KIND_CASE) {
        node->an_val = Sem_Truncate(sw->an_cond->an_type, node->an_val);
    }
    if (node->an_kind == AST_NODE_KIND_CASE || node->an_kind == AST_NODE_KIND_DEFAULT) {
        for (Ast_Node *seen = sw->an_cases; seen; seen = seen->an_casenext) {
            Err_AssertAt(node->an_line, seen->an_kind != node->an_kind || (node->an_kind != AST_NODE_KIND_DEFAULT && seen->an_val != node->an_val), ERR_SEM_CASE_DUPLICATE);
        }
        Err_AssertAt(node->an_line, Ast_ContainsVmScope(sw->an_vm, node->an_vm), ERR_SEM_CASE_INTO_VM_SCOPE);
        if (*tail) {
            (*tail)->an_casenext = node;
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

// Annotate one node whose operands are annotated already.
void Sem_Annotate(Ast_Node *node)
{
    switch (node->an_kind) {
        case AST_NODE_KIND_NUM: {
            if (! node->an_type) {
                node->an_type = &Ast_TypeInt;
            }
        } break;

        case AST_NODE_KIND_FNUM: {
            // the parser already set an_type from the literal's suffix
        } break;

        case AST_NODE_KIND_AND:
        case AST_NODE_KIND_OR: {
            Sem_NeedScalar(node->an_lhs, node->an_line);
            Sem_NeedScalar(node->an_rhs, node->an_line);
            node->an_lhs = Sem_Truth(node->an_lhs);
            node->an_rhs = Sem_Truth(node->an_rhs);
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_MUL:
        case AST_NODE_KIND_DIV: {
            Sem_NeedArithmetic(node);
            Sem_UsualArith(node);
        } break;

        case AST_NODE_KIND_MOD:
        case AST_NODE_KIND_BITAND:
        case AST_NODE_KIND_BITOR:
        case AST_NODE_KIND_BITXOR: {
            Sem_NeedInteger(node);
            Sem_UsualArith(node);
        } break;

        case AST_NODE_KIND_EQ:
        case AST_NODE_KIND_NE:
        case AST_NODE_KIND_LT:
        case AST_NODE_KIND_LE: {
            Sem_NeedScalar(node->an_lhs, node->an_line);
            Sem_NeedScalar(node->an_rhs, node->an_line);
            if (! Sem_IsPointer(node->an_lhs->an_type) && ! Sem_IsPointer(node->an_rhs->an_type)) {
                Sem_NeedArithmetic(node);
                Sem_UsualArith(node);
            }
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_SHL:
        case AST_NODE_KIND_SHR: {
            Sem_NeedInteger(node);
            Sem_PromoteShift(node);
        } break;

        case AST_NODE_KIND_NEG:
        case AST_NODE_KIND_BITNOT: {
            if (node->an_kind == AST_NODE_KIND_BITNOT) {
                Sem_NeedInteger(node);
            }
            Sem_NeedArithmetic(node);
            node->an_lhs = Sem_Convert(node->an_lhs, Sem_Promote(node->an_lhs->an_type));
            node->an_type = node->an_lhs->an_type;
        } break;

        case AST_NODE_KIND_NOT: {
            Sem_NeedScalar(node->an_lhs, node->an_line);
            node->an_lhs = Sem_Truth(node->an_lhs);
            node->an_type = &Ast_TypeInt;
        } break;

        case AST_NODE_KIND_ADD:
        case AST_NODE_KIND_SUB: {
            Sem_Arith(node);
        } break;

        case AST_NODE_KIND_STR: {
            Ast_Str *str = Ast_StringAt(node->an_stridx);
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
            Err_AssertAt(node->an_line, node->an_lhs->an_type->at_base->at_complete || Ast_IsUnsized(node->an_lhs->an_type->at_base), ERR_SEM_DEREF_INCOMPLETE);
            node->an_type = node->an_lhs->an_type->at_base;
        } break;

        case AST_NODE_KIND_MEMBER: {
            Ast_Type *type = node->an_lhs->an_type;
            Err_AssertAt(node->an_line, Sem_IsAggregate(type), ERR_SEM_MEMBER_NOT_AGGREGATE, node->an_memname);
            Err_AssertAt(node->an_line, type->at_complete, ERR_SEM_MEMBER_INCOMPLETE, Sem_TypeName(type));
            node->an_member = Ast_FindMember(type, node->an_memname);
            Err_AssertAt(node->an_line, node->an_member, ERR_SEM_MEMBER_UNKNOWN, node->an_memname, Sem_TypeName(type));
            node->an_type = Ast_Qualify(node->an_member->am_type, node->an_member->am_type->at_qual | type->at_qual);
        } break;

        case AST_NODE_KIND_CAST: {
            Sem_CheckCast(node);
        } break;

        case AST_NODE_KIND_SIZEOF: {
            Ast_Type *type = node->an_lhs->an_type;
            Err_AssertAt(node->an_line, type->at_complete, ERR_SEM_SIZEOF_INCOMPLETE);
            if (Ast_IsVla(type)) {
                Sem_Replace(node, Sem_NewBinary(AST_NODE_KIND_COMMA, node->an_lhs, Sem_TempRef(type->at_vsize, node->an_line), node->an_line));
                break;
            }
            node->an_kind = AST_NODE_KIND_NUM;
            node->an_val  = node->an_lhs->an_type->at_size;
            node->an_lhs  = NULL;
            node->an_type = &Ast_TypeULong;
        } break;

        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_COMPOUND: {
            node->an_type = node->an_var->av_type;
        } break;

        case AST_NODE_KIND_ASSIGN: {
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
            Err_AssertAt(node->an_line, node->an_lhs->an_type->at_kind != AST_TYPE_KIND_ARRAY, ERR_SEM_ASSIGN_ARRAY);
            Err_AssertAt(node->an_line, node->an_initstore || Sem_IsModifiable(node->an_lhs->an_type), ERR_SEM_ASSIGN_CONST);
            Sem_CheckAssign(node->an_lhs->an_type, node->an_rhs, node->an_initstore ? "initialization" : "assignment", node->an_line);
            node->an_type = node->an_lhs->an_type;
            node->an_rhs  = Sem_Convert(node->an_rhs, node->an_type);
        } break;

        case AST_NODE_KIND_OPASSIGN: {
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
            Err_AssertAt(node->an_line, Sem_IsModifiable(node->an_lhs->an_type), ERR_SEM_ASSIGN_CONST);
            Sem_CheckOpAssign(node);
            if (Sem_NeedsFloatAssign(node)) {
                Sem_LowerOpAssign(node);
                break;
            }
            Ast_Type *type = node->an_lhs->an_type;
            if (Sem_IsPointer(type) && (node->an_op == AST_NODE_KIND_ADD || node->an_op == AST_NODE_KIND_SUB)) {
                node->an_rhs = Sem_ScaleBy(node->an_rhs, Sem_Stride(type, node->an_line));
            }
            node->an_type = type;
        } break;

        case AST_NODE_KIND_POSTINC: {
            Ast_Type *type = node->an_lhs->an_type;
            Err_AssertAt(node->an_line, Sem_IsLvalue(node->an_lhs), ERR_SEM_NOT_ASSIGNABLE);
            Err_AssertAt(node->an_line, Sem_IsModifiable(type), ERR_SEM_ASSIGN_CONST);
            Err_AssertAt(node->an_line, Ast_IsArithmetic(type) || type->at_kind == AST_TYPE_KIND_PTR, ERR_SEM_OPERAND_NOT_ARITHMETIC);
            if (Sem_NeedsFloatAssign(node) || (Sem_IsPointer(type) && Ast_IsVla(type->at_base))) {
                Sem_LowerPostInc(node);
                break;
            }
            if (Sem_IsPointer(type)) {
                node->an_step *= type->at_base->at_size;
            }
            node->an_type = type;
        } break;

        case AST_NODE_KIND_COND: {
            Sem_NeedScalar(node->an_cond, node->an_line);
            node->an_cond = Sem_Truth(node->an_cond);
            if (! Ast_IsArithmetic(node->an_then->an_type) || ! Ast_IsArithmetic(node->an_els->an_type)) {
                node->an_type = Sem_CondType(node);
                break;
            }
            node->an_type = Sem_CommonType(node->an_then->an_type, node->an_els->an_type);
            node->an_then = Sem_Convert(node->an_then, node->an_type);
            node->an_els  = Sem_Convert(node->an_els, node->an_type);
        } break;

        case AST_NODE_KIND_COMMA: {
            node->an_type = node->an_rhs->an_type;
        } break;

        case AST_NODE_KIND_CALL: {
            Sem_CheckCall(node);
            node->an_type = Sem_CallType(node);
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            Ast_Func *func = Ast_FindFunction(node->an_funcname);
            node->an_type = Ast_NewPointer(func ? func->af_type : &Ast_TypeInt);
        } break;

        case AST_NODE_KIND_VA_START: {
            Err_AssertAt(node->an_line, Sem_CurFunc->af_type->at_va != AST_TYPE_FIXED, ERR_SEM_VA_START_FIXED);
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
            node->an_cond = Sem_Convert(node->an_cond, Sem_Promote(node->an_cond->an_type));
            Sem_CollectCases(node->an_body, node, &tail);
        } break;

        case AST_NODE_KIND_RETURN: {
            Ast_Type *ret = Sem_CurFunc->af_type->at_ret;
            if (ret->at_kind == AST_TYPE_KIND_VOID) {
                Err_AssertAt(node->an_line, ! node->an_lhs || node->an_lhs->an_type->at_kind == AST_TYPE_KIND_VOID, ERR_SEM_RETURN_VALUE_IN_VOID);
                break;
            }
            Err_AssertAt(node->an_line, node->an_lhs, ERR_SEM_RETURN_NO_VALUE);
            Sem_CheckAssign(ret, node->an_lhs, "return", node->an_line);
            node->an_lhs = Sem_Convert(node->an_lhs, ret);
        } break;

        case AST_NODE_KIND_IF:
        case AST_NODE_KIND_FOR:
        case AST_NODE_KIND_DO: {
            Sem_NeedScalar(node->an_cond, node->an_line);
            node->an_cond = Sem_Truth(node->an_cond);
        } break;

        case AST_NODE_KIND_VSIZE: {
            Err_AssertAt(node->an_line, Ast_IsInteger(node->an_lhs->an_type), ERR_SEM_ARRAY_LEN_NOT_INTEGER);
            Ast_Node *bytes = Sem_NewBinary(AST_NODE_KIND_MUL, Sem_Convert(node->an_lhs, &Ast_TypeULong), node->an_rhs, node->an_line);
            Sem_Replace(node, Sem_NewBinary(AST_NODE_KIND_ASSIGN, Sem_TempRef(node->an_var, node->an_line), bytes, node->an_line));
        } break;

        case AST_NODE_KIND_GOTO:
        case AST_NODE_KIND_LABEL:
        case AST_NODE_KIND_DEFAULT:
        case AST_NODE_KIND_BREAK:
        case AST_NODE_KIND_CONTINUE:
        case AST_NODE_KIND_BLOCK:
        case AST_NODE_KIND_DECL:
        case AST_NODE_KIND_VLA:
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
    Sem_Annotate(node);
}

// Annotate every file-scope initializer and convert it to the slot it fills.
void Sem_AnalyzeGlobals(void)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        for (Ast_Node *item = var->av_init; item; item = item->an_next) {
            Sem_Node(item->an_lhs);
            Sem_CheckAssign(item->an_type, item->an_lhs, "initialization", item->an_line);
            if (Ast_IsArithmetic(item->an_type)) {
                item->an_lhs = Sem_Convert(item->an_lhs, item->an_type);
            }
        }
    }
}

// Annotate every node with its type and reject what the grammar cannot.
void Sem_Analyze(Ast_Func *prog)
{
    Sem_CurFunc = NULL;
    Sem_AnalyzeGlobals();

    for (Ast_Func *func = prog; func; func = func->af_next) {
        if (! func->af_body) {
            continue;  // a prototype declares a signature and nothing to walk
        }
        Sem_CurFunc = func;
        Sem_Node(func->af_body);
        Sem_CheckGotos(func->af_body, func->af_body);
    }
}
