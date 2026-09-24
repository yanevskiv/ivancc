// C source file for the parser's declarator and parameter helpers.

#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"
#include "syntax/ast.h"
#include "syntax/sem.h"
#include "syntax/par.h"

// State for the function definition currently being parsed.
static char            *Par_CurFuncName;
static Ast_Var         *Par_CurParams;
static int32_t          Par_CurNumParams;
static Ast_TypeVariadic Par_CurVariadic;
static Ast_TypeProto    Par_CurProto;
static bool             Par_CurStatic;
static Ast_Type        *Par_CurRetType;
static bool             Par_InFunction;

// Serial number of the next compound literal's object.
static int32_t Par_CompoundCount;

// The type and storage class one declaration's declarators share.
static Ast_Type   *Par_DeclType;
static Ast_Storage Par_DeclStorage;
static char       *Par_DeclName;

// The type the top-level declarator just read works out to.
static Ast_Type   *Par_CurDeclType;

// The program assembled so far.
static Ast_Func *Par_ProgHead;
static Ast_Func *Par_ProgTail;

// Value the next enumerator takes.
static int64_t Par_EnumValue;

// The record __builtin_va_list names.
static Ast_Type *Par_VaList;

// Record the specifier one declaration's declarators share.
void Par_SetDeclSpec(Ast_Storage storage, Ast_Type *type)
{
    Par_DeclStorage = storage;
    Par_DeclType    = type;
}

// Start an enumerator list over.
void Par_ResetEnum(void)
{
    Par_EnumValue = 0;
}

// Empty a parameter list.
void Par_ClearParams(Par_ParamList *list)
{
    list->pl_head     = NULL;
    list->pl_tail     = NULL;
    list->pl_count    = 0;
    list->pl_variadic = AST_TYPE_FIXED;
    list->pl_proto    = AST_TYPE_NOPROTO;
}

// Append one parameter to a list.
void Par_PushParam(Par_ParamList *list, Ast_Var *var)
{
    if (! var) {
        return;
    }
    var->av_param_next = NULL;
    if (list->pl_tail) {
        list->pl_tail->av_param_next = var;
    } else {
        list->pl_head = var;
    }
    list->pl_tail = var;
    list->pl_count++;
}

// Start a declarator for name.
Par_Decl *Par_NewDecl(char *name)
{
    Par_Decl *decl = calloc(1, sizeof(Par_Decl));
    decl->pc_name = name;
    return decl;
}

// Reject a declarator with no name.
void Par_NeedName(Par_Decl *decl, Ast_Line line)
{
    if (! decl->pc_name) {
        Log_ShowErrorAt(decl->pc_line ? decl->pc_line : line, "this declaration needs a name");
    }
}

// Append one derivation to a declarator.
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, Ast_Line line)
{
    Par_Deriv *deriv = calloc(1, sizeof(Par_Deriv));
    deriv->pd_kind = kind;
    deriv->pd_line = line;
    if (decl->pc_tail) {
        decl->pc_tail->pd_next = deriv;
    } else {
        decl->pc_head = deriv;
    }
    decl->pc_tail = deriv;
    return deriv;
}

// Wrap base in one derivation list, outermost first.
Ast_Type *Par_ApplyDerivs(Ast_Type *base, Par_Deriv *deriv)
{
    if (! deriv) {
        return base;
    }
    Ast_Type *inner = Par_ApplyDerivs(base, deriv->pd_next);
    switch (deriv->pd_kind) {
        case PAR_DERIV_POINTER: {
            return Ast_NewPointer(inner);
        }
        case PAR_DERIV_ARRAY: {
            if (inner->at_kind == AST_TYPE_KIND_FUNC) {
                Log_ShowErrorAt(deriv->pd_line, "an array of functions is not a type");
            }
            if (deriv->pd_decor) {
                Log_ShowErrorAt(deriv->pd_line, "'static' and qualifiers in an array declarator are only allowed on a parameter");
            }
            return Ast_NewArray(inner, (int32_t) deriv->pd_len);
        }
        case PAR_DERIV_FUNCTION: {
            if (inner->at_kind == AST_TYPE_KIND_FUNC || inner->at_kind == AST_TYPE_KIND_ARRAY) {
                Log_ShowErrorAt(deriv->pd_line, "a function cannot return a function or an array");
            }
            return Ast_NewFunction(inner, deriv->pd_params.pl_head, deriv->pd_params.pl_count, deriv->pd_params.pl_variadic, deriv->pd_params.pl_proto);
        }
        case PAR_DERIV_COUNT: {
            // empty
        } break;
    }
    return inner;
}

// Give a declarator the type applying it to base yields.
Ast_Type *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl)
{
    return Par_ApplyDerivs(base, decl->pc_head);
}

// Decay a parameter's array or function type to a pointer.
Ast_Type *Par_AdjustParam(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY) {
        return Ast_NewPointer(type->at_base);
    }
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        return Ast_NewPointer(type);
    }
    return type;
}

// Accept `static` and qualifiers on a parameter's outermost array.
void Par_TakeArrayDecor(Par_Decl *decl, Ast_Line line)
{
    for (Par_Deriv *deriv = decl->pc_head; deriv; deriv = deriv->pd_next) {
        if (! deriv->pd_decor) {
            continue;
        }
        if (deriv != decl->pc_head || deriv->pd_kind != PAR_DERIV_ARRAY) {
            Log_ShowErrorAt(line, "'static' and qualifiers are only allowed on a parameter's outermost array");
        }
        if ((deriv->pd_decor & PAR_ARRAY_STATIC) && deriv->pd_empty) {
            Log_ShowErrorAt(line, "'static' in an array declarator needs a length");
        }
        deriv->pd_decor = PAR_ARRAY_NONE;
    }
}

// Build one named parameter.
Ast_Var *Par_MakeParam(Ast_Type *base, Par_Decl *decl, Ast_Line line)
{
    Par_TakeArrayDecor(decl, line);
    Ast_Type *type = Par_AdjustParam(Par_ApplyDecl(base, decl));
    Ast_Var *var = calloc(1, sizeof(Ast_Var));

    var->av_name = decl->pc_name;
    var->av_type = type;
    var->av_line = line;
    return var;
}

// Build one old-style parameter.
Ast_Var *Par_MakeKnrParam(char *name, Ast_Line line)
{
    Ast_Var *var = calloc(1, sizeof(Ast_Var));

    var->av_name = name;
    var->av_line = line;
    return var;
}

// Give an old-style parameter the type its declaration list names.
void Par_SetKnrParam(Par_Decl *decl, Ast_Line line)
{
    Par_NeedName(decl, line);
    Par_TakeArrayDecor(decl, line);
    for (Ast_Var *param = Par_CurParams; param; param = param->av_param_next) {
        if (param->av_name && strcmp(param->av_name, decl->pc_name) == 0) {
            param->av_type = Par_AdjustParam(Par_ApplyDecl(Par_DeclType, decl));
            return;
        }
    }
    Log_ShowErrorAt(line, "'%s' is not a parameter of this function", decl->pc_name);
}

// Reject an old-style parameter the declaration list never typed.
void Par_CheckKnrParams(void)
{
    for (Ast_Var *param = Par_CurParams; param; param = param->av_param_next) {
        if (! param->av_type) {
            Log_ShowErrorAt(param->av_line, "parameter '%s' has no declaration", param->av_name);
        }
    }
}

// Build one unnamed parameter.
Ast_Var *Par_MakeAnonParam(Ast_Type *type, Ast_Line line)
{
    if (type->at_kind == AST_TYPE_KIND_VOID) {
        return NULL;
    }
    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_type = Par_AdjustParam(type);
    var->av_line = line;
    return var;
}

// Give an integer literal the type its spelling and value ask for.
Par_Num Par_NumLiteral(const char *text)
{
    const char *suffix = text;
    Ast_TypeSign sign = AST_TYPE_SIGNED;
    Ast_TypeKind least = AST_TYPE_KIND_INT;

    bool decimal = text[0] != '0';
    uint64_t val = strtoull(text, (char **) &suffix, 0);

    for (const char *p = suffix; *p; p++) {
        if (*p == 'u' || *p == 'U') {
            sign = AST_TYPE_UNSIGNED;
        } else if (least == AST_TYPE_KIND_LONG) {
            least = AST_TYPE_KIND_LLONG;
        } else {
            least = AST_TYPE_KIND_LONG;
        }
    }

    for (Ast_TypeKind kind = least; kind <= AST_TYPE_KIND_LAST_INT; kind++) {
        Ast_Type *type = Ast_IntegerType(kind, sign);
        int32_t bits = type->at_size * AST_BITS_PER_BYTE;
        uint64_t room;

        if (type->at_sign == AST_TYPE_UNSIGNED) {
            room = ~(uint64_t) 0 >> (PAR_VALUE_BITS - bits);
        } else {
            room = ~(uint64_t) 0 >> (PAR_VALUE_BITS - bits + 1);
        }
        if (val <= room) {
            return (Par_Num) {
                .pn_val  = (int64_t) val,
                .pn_type = type
            };
        }
        if (! decimal && type->at_sign != AST_TYPE_UNSIGNED) {
            Ast_Type *alt = Ast_IntegerType(kind, AST_TYPE_UNSIGNED);
            if (val <= ~(uint64_t) 0 >> (PAR_VALUE_BITS - alt->at_size * AST_BITS_PER_BYTE)) {
                return (Par_Num) {
                    .pn_val  = (int64_t) val,
                    .pn_type = alt
                };
            }
        }
    }
    return (Par_Num) {
        .pn_val  = (int64_t) val,
        .pn_type = Ast_IntegerType(AST_TYPE_KIND_LLONG, AST_TYPE_UNSIGNED)
    };
}

// Decode a character literal body into its value and type.
Par_Num Par_CharLiteral(const char *body, size_t len, size_t width)
{
    int64_t value = 0;
    size_t bytes = 0;
    char *data = Str_Unescape(body, len, width, &bytes);

    if (width > STR_NARROW_WIDTH) {
        value = (int32_t) Str_GetValue(data + bytes - width, width);
    } else if (bytes == 1) {
        value = (int8_t) data[0];
    } else {
        for (size_t i = 0; i < bytes; i++) {
            value = (int32_t) ((value << STR_BITS_PER_BYTE) | (uint8_t) data[i]);
        }
    }
    Str_Free(data);
    return (Par_Num) {
        .pn_val  = value,
        .pn_type = &Ast_TypeInt
    };
}

// Re-encode a string literal into elements of the given width.
Ast_Str Par_WidenString(Ast_Str str, size_t width)
{
    size_t n = 0;
    Ast_Str out;

    out.as_data  = calloc(str.as_len / str.as_width + 1, width);
    out.as_width = width;
    for (size_t i = 0; i < str.as_len; i += str.as_width) {
        Str_PutValue(out.as_data, &n, width, Str_GetValue(str.as_data + i, str.as_width));
    }
    out.as_len = n;
    return out;
}

// Join two adjacent string literals into one.
Ast_Str Par_ConcatStrings(Ast_Str left, Ast_Str right)
{
    size_t width = left.as_width > right.as_width ? left.as_width : right.as_width;
    Ast_Str a = Par_WidenString(left, width);
    Ast_Str b = Par_WidenString(right, width);
    Ast_Str out;

    out.as_data  = calloc(a.as_len + b.as_len + width, 1);
    out.as_len   = a.as_len + b.as_len;
    out.as_width = width;
    memcpy(out.as_data, a.as_data, a.as_len);
    memcpy(out.as_data + a.as_len, b.as_data, b.as_len);
    Str_Free(a.as_data);
    Str_Free(b.as_data);
    Str_Free(left.as_data);
    Str_Free(right.as_data);
    return out;
}

// Empty a specifier set.
void Par_ClearSpecs(Par_Specs *specs)
{
    specs->ps_specs = 0;
    specs->ps_qual  = 0;
    specs->ps_type  = NULL;
}

// Add one type specifier keyword to a declaration's set.
Par_Spec Par_AddSpec(Par_Spec specs, Par_Spec spec, Ast_Line line)
{
    if (spec == PAR_SPEC_LONG && (specs & PAR_SPEC_LONG)) {
        spec = PAR_SPEC_LLONG;
    }
    if (specs & spec) {
        Log_ShowErrorAt(line, "a type specifier is repeated");
    }
    return specs | spec;
}

// Merge one specifier or qualifier into a declaration's set.
void Par_TakeSpec(Par_Specs *into, const Par_Specs *one, Ast_Line line)
{
    if (one->ps_type && (into->ps_type || into->ps_specs)) {
        Log_ShowErrorAt(line, "two or more data types in one declaration");
    }
    if (one->ps_specs && into->ps_type) {
        Log_ShowErrorAt(line, "two or more data types in one declaration");
    }
    if (one->ps_specs) {
        into->ps_specs = Par_AddSpec(into->ps_specs, one->ps_specs, line);
    }
    if (one->ps_type) {
        into->ps_type = one->ps_type;
    }
    into->ps_qual |= one->ps_qual;
}

// Return the type a declaration's specifier keywords name.
Ast_Type *Par_SpecType(Par_Spec specs, Ast_Line line)
{
    Par_Spec explicit = specs & (PAR_SPEC_SIGNED | PAR_SPEC_UNSIGNED);
    Ast_TypeSign sign = specs & PAR_SPEC_UNSIGNED ? AST_TYPE_UNSIGNED : AST_TYPE_SIGNED;

    switch (specs & ~(PAR_SPEC_SIGNED | PAR_SPEC_UNSIGNED)) {
        case PAR_SPEC_VOID: {
            if (explicit) {
                Log_ShowErrorAt(line, "'void' cannot be signed or unsigned");
            }
            return &Ast_TypeVoid;
        } break;
        case PAR_SPEC_BOOL: {
            if (explicit) {
                Log_ShowErrorAt(line, "'_Bool' cannot be signed or unsigned");
            }
            return &Ast_TypeBool;
        } break;
        case PAR_SPEC_CHAR: {
            return Ast_IntegerType(AST_TYPE_KIND_CHAR, sign);
        } break;
        case PAR_SPEC_SHORT:
        case PAR_SPEC_SHORT | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_SHORT, sign);
        } break;
        case PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_INT, sign);
        } break;
        case PAR_SPEC_NONE: {
            if (! explicit) {
                Log_ShowErrorAt(line, "a declaration needs a type specifier");
            }
            return Ast_IntegerType(AST_TYPE_KIND_INT, sign);
        } break;
        case PAR_SPEC_LONG:
        case PAR_SPEC_LONG | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_LONG, sign);
        } break;
        case PAR_SPEC_LONG | PAR_SPEC_LLONG:
        case PAR_SPEC_LONG | PAR_SPEC_LLONG | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_LLONG, sign);
        } break;
        default: {
            Log_ShowErrorAt(line, "these type specifiers do not name a type");
        }
    }
    return &Ast_TypeInt;
}

// Return the type one declaration's specifiers name.
Ast_Type *Par_SpecsType(const Par_Specs *specs, Ast_Line line)
{
    Ast_Type *type = specs->ps_type ? specs->ps_type : Par_SpecType(specs->ps_specs, line);
    return Ast_Qualify(type, specs->ps_qual);
}

// Wrap base in the array dimensions listed outermost first.
Ast_Type *Par_ArrayType(Ast_Type *base, Ast_Node *dims)
{
    if (! dims) {
        return base;
    }
    return Ast_NewArray(Par_ArrayType(base, dims->an_next), (int32_t) dims->an_val);
}

// The type __builtin_va_list names.
Ast_Type *Par_VaListType(void)
{
    if (Par_VaList) {
        return Par_VaList;
    }

    Ast_Member *gp = Ast_NewMember("gp_offset", &Ast_TypeInt, 0);
    gp->am_next = Ast_NewMember("fp_offset", &Ast_TypeInt, 0);
    gp->am_next->am_next = Ast_NewMember("overflow_arg_area", Ast_NewPointer(&Ast_TypeVoid), 0);
    gp->am_next->am_next->am_next = Ast_NewMember("reg_save_area", Ast_NewPointer(&Ast_TypeVoid), 0);

    Ast_Type *tag = Ast_NewAggregate(AST_TYPE_KIND_STRUCT, "__va_list_tag");
    Ast_LayoutAggregate(tag, gp, 0);
    Par_VaList = Ast_NewArray(tag, 1);
    return Par_VaList;
}

// Build the node reading the next anonymous argument.
Ast_Node *Par_VaArg(Ast_Node *ap, Ast_Type *type, Ast_Line line)
{
    if (Sem_IsAggregate(type) || type->at_kind == AST_TYPE_KIND_ARRAY) {
        Log_ShowErrorAt(line, "__builtin_va_arg of a struct, union or array is not supported");
    }
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_VA_ARG, ap, line);
    node->an_type = type;
    return node;
}

// Join two member lists, keeping declaration order.
Ast_Member *Par_AppendMembers(Ast_Member *head, Ast_Member *tail)
{
    if (! head) {
        return tail;
    }
    Ast_Member *last = head;
    while (last->am_next) {
        last = last->am_next;
    }
    last->am_next = tail;
    return head;
}

// Narrow a member to the bits a `: width` gave it.
void Par_AddBitfield(Ast_Member *member, Ast_Node *width, Ast_Line line)
{
    int64_t bits = 0;

    if (! Sem_Fold(width, &bits)) {
        Log_ShowErrorAt(line, "a bit-field width is not a constant");
    }
    if (! Ast_IsInteger(member->am_type)) {
        Log_ShowErrorAt(line, "a bit-field must have an integer type");
    }
    if (bits < 0) {
        Log_ShowErrorAt(line, "a bit-field width cannot be negative");
    }
    if (bits > member->am_type->at_size * AST_BITS_PER_BYTE) {
        Log_ShowErrorAt(line, "a bit-field is wider than the type that holds it");
    }
    if (bits == 0 && member->am_name) {
        Log_ShowErrorAt(line, "a bit-field with a name cannot be zero bits wide");
    }
    member->am_bits = (int32_t) bits;
}

// Turn one member declaration's declarators into members of the shared type.
Ast_Member *Par_MakeMembers(Ast_Type *type, Par_Decl *decls)
{
    Ast_Member head = {0};
    Ast_Member *tail = &head;

    for (Par_Decl *decl = decls; decl; decl = decl->pc_next) {
        if (! decl->pc_name && ! decl->pc_bits) {
            Log_ShowErrorAt(decl->pc_line, "this member needs a name");
        }
        tail->am_next = Ast_NewMember(decl->pc_name, Par_ApplyDecl(type, decl), decl->pc_line);
        tail = tail->am_next;
        if (decl->pc_head && decl->pc_head->pd_kind == PAR_DERIV_ARRAY && decl->pc_head->pd_empty) {
            tail->am_flexible = true;
        }
        if (decl->pc_bits) {
            Par_AddBitfield(tail, decl->pc_bits, decl->pc_line);
        }
    }
    return head.am_next;
}

// Open a struct or union definition, binding its tag first.
Ast_Type *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line)
{
    Ast_Type *type = tag ? Ast_FindTagHere(tag) : NULL;

    if (type && type->at_complete) {
        Log_ShowErrorAt(line, "redefinition of '%s'", tag);
    }
    if (type && type->at_kind != kind) {
        Log_ShowErrorAt(line, "'%s' was declared with a different aggregate keyword", tag);
    }
    if (! type) {
        type = Ast_NewAggregate(kind, tag);
        if (tag) {
            Ast_DeclareTag(tag, type);
        }
    }
    return type;
}

// Name a struct or union not yet defined.
Ast_Type *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line)
{
    Ast_Type *type = Ast_FindTag(tag);

    if (type && type->at_kind != kind) {
        Log_ShowErrorAt(line, "'%s' was declared with a different aggregate keyword", tag);
    }
    if (! type) {
        type = Ast_NewAggregate(kind, tag);
        Ast_DeclareTag(tag, type);
    }
    return type;
}

// Declare one enumeration constant and step the next one's value.
void Par_AddEnumConst(const char *name, Ast_Node *value, Ast_Line line)
{
    if (value && ! Sem_Fold(value, &Par_EnumValue)) {
        Log_ShowErrorAt(line, "enumerator '%s' is not a constant", name);
    }
    Ast_DeclareEnumConst(name, Par_EnumValue++);
}

// Build the statement writing one flattened initializer into its object.
Ast_Node *Par_InitStore(Ast_Var *var, int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line)
{
    int32_t at_off = bits ? off - bits->am_offset : off;
    Ast_Type *outer = bits ? bits->am_owner : type;

    Ast_Node *addr = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewUnary(AST_NODE_KIND_ADDR, Ast_NewVarNode(var, line), line), line);
    addr->an_type = Ast_NewPointer(&Ast_TypeChar);

    Ast_Node *at = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewBinary(AST_NODE_KIND_ADD, addr, Ast_NewNum(at_off, line), line), line);
    at->an_type = Ast_NewPointer(outer);

    Ast_Node *slot = Ast_NewUnary(AST_NODE_KIND_DEREF, at, line);
    if (bits) {
        slot = Ast_NewMemberNode(slot, bits->am_name, line);
    }
    return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, Ast_NewBinary(AST_NODE_KIND_ASSIGN, slot, value, line), line);
}

// Record one flattened initializer at a byte offset.
Ast_Node *Par_InitAt(int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_INIT, value, line);
    node->an_val    = off;
    node->an_type   = type;
    node->an_member = bits;
    return node;
}

// Move a cursor to the subobject a designator names.
void Par_Designate(Ast_Type *type, Ast_Node *desig, int32_t *index, Ast_Member **member, Ast_Line line)
{
    if (desig->an_memname) {
        if (! Sem_IsAggregate(type)) {
            Log_ShowErrorAt(line, "'.%s' designates a member of something that is not a struct or union", desig->an_memname);
        }
        *member = Ast_FindMember(type, desig->an_memname);
        if (! *member) {
            Log_ShowErrorAt(line, "no member named '%s' to initialize", desig->an_memname);
        }
        return;
    }

    if (type->at_kind != AST_TYPE_KIND_ARRAY) {
        Log_ShowErrorAt(line, "an index designator needs an array");
    }
    if (desig->an_val < 0 || desig->an_val >= type->at_len) {
        Log_ShowErrorAt(line, "initializer index %ld is outside the array", (long) desig->an_val);
    }
    *index = (int32_t) desig->an_val;
}

// Step a type and offset into the subobject one designator selected.
void Par_Step(Ast_Type **type, int32_t *off, Ast_Node *desig, int32_t index, Ast_Member *member)
{
    if (desig->an_memname) {
        *off += member->am_offset;
        *type = member->am_type;
        return;
    }
    *off += index * (*type)->at_base->at_size;
    *type = (*type)->at_base;
}

// The type an expression already has.
Ast_Type *Par_ExprType(Ast_Node *node)
{
    Ast_Type *type = NULL;

    switch (node->an_kind) {
        case AST_NODE_KIND_VAR: {
            type = node->an_var->av_type;
        } break;
        case AST_NODE_KIND_COMPOUND:
        case AST_NODE_KIND_CAST: {
            type = node->an_type;
        } break;
        case AST_NODE_KIND_ASSIGN: {
            type = Par_ExprType(node->an_lhs);
        } break;
        case AST_NODE_KIND_COMMA: {
            type = Par_ExprType(node->an_rhs);
        } break;
        case AST_NODE_KIND_DEREF: {
            Ast_Type *outer = Par_ExprType(node->an_lhs);
            type = outer ? outer->at_base : NULL;
        } break;
        case AST_NODE_KIND_MEMBER: {
            Ast_Type *outer = Par_ExprType(node->an_lhs);
            Ast_Member *member = outer ? Ast_FindMember(outer, node->an_memname) : NULL;
            type = member ? member->am_type : NULL;
        } break;
        case AST_NODE_KIND_CALL: {
            Ast_Func *func = Par_FindFunction(node->an_funcname);
            type = func ? func->af_ret : NULL;
        } break;
        default: {
            // empty
        } break;
    }
    return type;
}

// Fill one slot from the cursor.
void Par_FlattenSlot(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, Ast_Line line)
{
    Ast_Node *iter = *item;
    Ast_Node *value = iter->an_lhs;

    if (value->an_kind == AST_NODE_KIND_INITLIST) {
        Par_Flatten(type, base, bits, value, tail, line);
        *item = iter->an_next;
        return;
    }
    if (Sem_IsAggregate(type) && Par_ExprType(value) == type) {
        (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
        *tail = (*tail)->an_next;
        *item = iter->an_next;
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        Par_FlattenList(type, base, item, tail, PAR_LIST_UNBRACED, line);
        return;
    }
    (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
    *tail = (*tail)->an_next;
    *item = iter->an_next;
}

// Walk an aggregate's slots from the cursor.
void Par_FlattenList(Ast_Type *type, int32_t base, Ast_Node **item, Ast_Node **tail, Par_List braced, Ast_Line line)
{
    int32_t index = 0;
    Ast_Member *member = type->at_members;

    while (*item) {
        Ast_Node *iter = *item;

        if (iter->an_cond) {
            if (braced == PAR_LIST_UNBRACED) {
                return;
            }
            int32_t off = base;
            Ast_Node *desig = iter->an_cond;
            Ast_Type *slot = type;

            Par_Designate(type, desig, &index, &member, line);
            Par_Step(&slot, &off, desig, index, member);
            Ast_Member *bits = desig->an_memname && member->am_bits ? member : NULL;
            for (Ast_Node *next = desig->an_next; next; next = next->an_next) {
                int32_t at = 0;
                Ast_Member *inner = NULL;
                Par_Designate(slot, next, &at, &inner, line);
                Par_Step(&slot, &off, next, at, inner);
                bits = next->an_memname && inner->am_bits ? inner : NULL;
            }

            iter->an_cond = NULL;
            Par_Flatten(slot, off, bits, iter->an_lhs, tail, line);
            *item = iter->an_next;
            if (desig->an_memname) {
                member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
            } else {
                index++;
            }
            continue;
        }

        if (type->at_kind == AST_TYPE_KIND_ARRAY) {
            if (index >= type->at_len) {
                if (braced == PAR_LIST_UNBRACED) {
                    return;
                }
                Log_ShowErrorAt(line, "too many initializers for an array of %d", type->at_len);
            }
            Par_FlattenSlot(type->at_base, base + index * type->at_base->at_size, NULL, item, tail, line);
            index++;
            continue;
        }

        if (! member) {
            if (braced == PAR_LIST_UNBRACED) {
                return;
            }
            Log_ShowErrorAt(line, "too many initializers for '%s'", Sem_TypeName(type));
        }
        Par_FlattenSlot(member->am_type, base + member->am_offset, member->am_bits ? member : NULL, item, tail, line);
        member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
    }
}

// Flatten one initializer, braced or not, into the object at base.
void Par_Flatten(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, Ast_Line line)
{
    if (init->an_kind == AST_NODE_KIND_COMPOUND && init->an_type == type) {
        for (Ast_Node *item = init->an_items; item; item = item->an_next) {
            (*tail)->an_next = Par_InitAt(base + (int32_t) item->an_val, item->an_type, item->an_member, item->an_lhs, line);
            *tail = (*tail)->an_next;
        }
        return;
    }

    if (init->an_kind != AST_NODE_KIND_INITLIST) {
        if (type->at_kind == AST_TYPE_KIND_ARRAY) {
            Log_ShowErrorAt(line, "an array needs a braced initializer");
        }
        (*tail)->an_next = Par_InitAt(base, type, bits, init, line);
        *tail = (*tail)->an_next;
        return;
    }

    Ast_Node *item = init->an_body;
    if (type->at_kind != AST_TYPE_KIND_ARRAY && ! Sem_IsAggregate(type)) {
        if (! item) {
            Log_ShowErrorAt(line, "an empty initializer list has nothing to assign");
        }
        Par_Flatten(type, base, bits, item->an_lhs, tail, line);
        return;
    }
    Par_FlattenList(type, base, &item, tail, PAR_LIST_BRACED, line);
}

// Flatten an initializer to the scalar writes that fill the object.
Ast_Node *Par_FlattenInit(Ast_Type *type, Ast_Node *init, Ast_Line line)
{
    Ast_Node head = {0};
    Ast_Node *tail = &head;

    Par_Flatten(type, 0, NULL, init, &tail, line);
    return head.an_next;
}

// Lower a flattened initializer to the statements filling a local.
Ast_Node *Par_InitFlat(Ast_Var *var, Ast_Node *flat, Ast_Line line)
{
    Ast_Node *zero = Ast_NewUnary(AST_NODE_KIND_ZERO, Ast_NewVarNode(var, line), line);
    zero->an_val = var->av_type->at_size;

    Ast_Node *tail = zero;
    for (Ast_Node *item = flat; item; item = item->an_next) {
        tail->an_next = Par_InitStore(var, (int32_t) item->an_val, item->an_type, item->an_member, item->an_lhs, line);
        tail = tail->an_next;
    }
    return zero;
}

// Lower a local's initializer to the statements that fill it.
Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, Ast_Line line)
{
    if (init->an_kind != AST_NODE_KIND_INITLIST && var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }
    return Par_InitFlat(var, Par_FlattenInit(var->av_type, init, line), line);
}

// Build the unnamed object a compound literal names.
Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, Ast_Line line)
{
    if (! type->at_complete) {
        Log_ShowErrorAt(line, "a compound literal of an incomplete type has no size");
    }

    Ast_Node *list = Ast_NewNode(AST_NODE_KIND_INITLIST, line);
    list->an_body = items;

    char *name = Str_Format(".compound.%d", Par_CompoundCount++);
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_COMPOUND, line);
    node->an_type  = type;
    node->an_items = Par_FlattenInit(type, list, line);

    if (! Par_InFunction) {
        node->an_var = Ast_DeclareGlobal(name, type, line);
        node->an_var->av_storage = AST_STORAGE_STATIC;
        node->an_var->av_init    = node->an_items;
        return node;
    }

    node->an_var = Ast_DeclareVar(name, type, line);
    node->an_body = Par_InitFlat(node->an_var, node->an_items, line);
    return node;
}

// Reject an object whose type has no size.
void Par_CheckComplete(const char *name, Ast_Type *type, Ast_Line line)
{
    if (! type->at_complete && Par_DeclStorage != AST_STORAGE_EXTERN) {
        Log_ShowErrorAt(line, "'%s' has an incomplete type", name);
    }
}

// Declare one file-scope name of the declaration being parsed.
void Par_AddDeclaredType(const char *name, Ast_Type *type, Ast_Node *init, Ast_Line line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, type);
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        Par_DeclarePrototype(name, type);
        return;
    }
    Par_CheckComplete(name, type, line);
    Ast_Var *var = Ast_DeclareGlobal(name, type, line);
    var->av_storage = Par_DeclStorage;
    var->av_init = init ? Par_FlattenInit(var->av_type, init, line) : NULL;
}

// Declare a variable inside a function.
Ast_Var *Par_DeclareLocal(const char *name, Ast_Type *type, Ast_Line line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, type);
        return NULL;
    }
    Par_CheckComplete(name, type, line);
    if (Par_DeclStorage != AST_STORAGE_STATIC) {
        return Ast_DeclareVar(name, type, line);
    }
    char *symbol = Str_Format("%s.%s", Par_CurFuncName, name);
    Ast_Var *var = Ast_DeclareStaticLocal(name, symbol, type, line);
    var->av_storage = AST_STORAGE_STATIC;
    return var;
}

// Declare one local and build the statement its initializer becomes.
Ast_Node *Par_AddLocal(Par_Decl *decl, Ast_Node *init, Ast_Line line)
{
    Par_NeedName(decl, line);
    Ast_Var *var = Par_DeclareLocal(decl->pc_name, Par_ApplyDecl(Par_DeclType, decl), line);

    if (! init) {
        return Ast_NewNode(AST_NODE_KIND_NOP, line);
    }
    if (! var) {
        Log_ShowErrorAt(line, "a typedef takes no initializer");
    }
    if (var->av_global) {
        var->av_init = Par_FlattenInit(var->av_type, init, line);
        return Ast_NewNode(AST_NODE_KIND_NOP, line);
    }
    return Par_InitLocal(var, init, line);
}

// Find a function already declared or defined under name.
Ast_Func *Par_FindFunction(const char *name)
{
    for (Ast_Func *fn = Par_ProgHead; fn; fn = fn->af_next) {
        if (strcmp(fn->af_name, name) == 0) {
            return fn;
        }
    }
    return NULL;
}

// Append a function to the program.
void Par_AddFunction(Ast_Func *fn)
{
    Ast_Func *seen = Par_FindFunction(fn->af_name);
    if (seen) {
        if (fn->af_body) {
            seen->af_body     = fn->af_body;
            seen->af_locals   = fn->af_locals;
            seen->af_params   = fn->af_params;
            seen->af_nparams  = fn->af_nparams;
            seen->af_variadic = fn->af_variadic;
            if (fn->af_proto == AST_TYPE_PROTO) {
                seen->af_proto = AST_TYPE_PROTO;
            }
        }
        return;
    }

    fn->af_next = NULL;
    if (! Par_ProgHead) {
        Par_ProgHead = Par_ProgTail = fn;
    } else {
        Par_ProgTail->af_next = fn;
        Par_ProgTail = fn;
    }
    Ast_Program = Par_ProgHead;
}

// Record a prototype a declarator spelled out.
void Par_DeclarePrototype(const char *name, Ast_Type *type)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_name     = (char *) name;
    fn->af_ret      = type->at_ret;
    fn->af_params   = type->at_params;
    fn->af_nparams  = type->at_nparams;
    fn->af_variadic = type->at_variadic;
    fn->af_proto    = type->at_proto;
    fn->af_static   = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_AddFunction(fn);
}

// Build the function the parser has just read.
Ast_Func *Par_MakeFunction(Ast_Node *body)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_name     = Par_CurFuncName;
    fn->af_ret      = Par_CurRetType;
    fn->af_body     = body;
    fn->af_params   = Par_CurParams;
    fn->af_nparams  = Par_CurNumParams;
    fn->af_variadic = Par_CurVariadic;
    fn->af_proto    = Par_CurProto;
    fn->af_static   = Par_CurStatic;
    fn->af_locals   = body ? Ast_CurrentLocals() : NULL;
    return fn;
}

// Note the declarator a top-level declaration named.
void Par_BeginExternal(Par_Decl *decl, Ast_Line line)
{
    Par_NeedName(decl, line);
    Ast_Type *type = Par_ApplyDecl(Par_DeclType, decl);

    Par_DeclName    = decl->pc_name;
    Par_CurDeclType = type;
    if (type->at_kind != AST_TYPE_KIND_FUNC) {
        Par_InFunction = false;
        return;
    }

    Par_CurStatic    = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_CurFuncName  = decl->pc_name;
    Par_CurRetType   = type->at_ret;
    Par_CurParams    = type->at_params;
    Par_CurNumParams = type->at_nparams;
    Par_CurVariadic  = type->at_variadic;
    Par_CurProto     = type->at_proto;
    Par_InFunction   = true;

    Par_DeclarePrototype(decl->pc_name, type);

    Ast_BeginScope();
    for (Ast_Var *param = type->at_params; param; param = param->av_param_next) {
        if (param->av_name) {
            Ast_DeclareParam(param);
        }
    }
    (void) line;
}

// Close a top-level declarator that turned out not to be a function definition.
void Par_EndExternal(Ast_Node *init, Ast_Line line)
{
    if (Par_InFunction) {
        Par_AddFunction(Par_MakeFunction(NULL));
        Ast_EndScope();
        Par_InFunction = false;
        return;
    }
    Par_AddDeclaredType(Par_DeclName, Par_CurDeclType, init, line);
}

// Close a function definition.
void Par_EndFunction(Ast_Node *body)
{
    Par_AddFunction(Par_MakeFunction(body));
    Ast_EndScope();
    Par_InFunction = false;
}

// Declare one more top-level name after a comma.
void Par_AddDeclared(Par_Decl *decl, Ast_Node *init, Ast_Line line)
{
    Par_NeedName(decl, line);
    Par_AddDeclaredType(decl->pc_name, Par_ApplyDecl(Par_DeclType, decl), init, line);
}

// Resolve a name used as a value.
Ast_Node *Par_Designator(char *name, Ast_Line line)
{
    Ast_Var *var = Ast_FindVar(name);
    if (var) {
        return Ast_NewVarNode(var, line);
    }

    Ast_Func *fn = Par_FindFunction(name);
    if (! fn) {
        Log_ShowErrorAt(line, "use of undeclared identifier '%s'", name);
    }
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_FUNCADDR, line);
    node->an_funcname = name;
    return node;
}

// Build a call, direct or through a pointer.
Ast_Node *Par_MakeCall(Ast_Node *callee, Ast_Node *args, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_CALL, line);

    node->an_args = args;
    if (callee->an_kind == AST_NODE_KIND_FUNCADDR) {
        node->an_funcname = callee->an_funcname;
    } else {
        node->an_lhs = callee;
    }
    return node;
}
