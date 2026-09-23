#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"
#include "syntax/ast.h"
#include "syntax/sem.h"
#include "syntax/par.h"

// State for the function definition currently being parsed.
static char     *Par_CurFuncName;
static Ast_Var  *Par_CurParams;
static int       Par_CurNumParams;
static int       Par_CurVariadic;
static int       Par_CurStatic;
static Ast_Type *Par_CurRetType;
static int       Par_InFunction;

// Serial number the next compound literal names its object with.
static int Par_CompoundCount;

// The type and storage class the declarators being parsed all share.
static Ast_Type   *Par_DeclType;
static Ast_Storage Par_DeclStorage;
static char       *Par_DeclName;

// The type the top-level declarator just read works out to, which its tail needs after the fact.
static Ast_Type   *Par_CurDeclType;

// The program assembled so far, as functions are reduced.
static Ast_Func *Par_ProgHead;
static Ast_Func *Par_ProgTail;

// Value the next enumerator takes, which `= n` resets.
static long Par_EnumValue;

// The record __builtin_va_list names, built on first use and shared after.
static Ast_Type *Par_VaList;

int  yylex(void);
void yyerror(const char *s);

// Record the specifier every declarator of one declaration shares, before the declarators are read.
void Par_SetDeclSpec(Ast_Storage storage, Ast_Type *type)
{
    Par_DeclStorage = storage;
    Par_DeclType    = type;
}

// Start an enumerator list over, whose first constant is 0 unless one says otherwise.
void Par_ResetEnum(void)
{
    Par_EnumValue = 0;
}

// Empty a parameter list, which starts out promising nothing about the parameters.
void Par_ClearParams(Par_ParamList *list)
{
    list->pl_head     = NULL;
    list->pl_tail     = NULL;
    list->pl_count    = 0;
    list->pl_variadic = 0;
    list->pl_proto    = 0;
}

// Append one parameter to a list, which a lone `void` leaves empty rather than one long.
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

// Start a declarator for name, which is NULL for the abstract declarator a cast or a parameter may use.
Par_Decl *Par_NewDecl(char *name)
{
    Par_Decl *decl = calloc(1, sizeof(Par_Decl));
    decl->pc_name = name;
    return decl;
}

// Append one derivation to a declarator, which records it further out from the name than the last.
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, int line)
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

// Wrap base in one derivation list, outermost first, so the step nearest the name is applied last.
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
            return Ast_NewArray(inner, (int) deriv->pd_len);
        }
        case PAR_DERIV_FUNCTION: {
            if (inner->at_kind == AST_TYPE_KIND_FUNC || inner->at_kind == AST_TYPE_KIND_ARRAY) {
                Log_ShowErrorAt(deriv->pd_line, "a function cannot return a function or an array");
            }
            return Ast_NewFunction(inner, deriv->pd_params.pl_head, deriv->pd_params.pl_count, deriv->pd_params.pl_variadic, deriv->pd_params.pl_proto);
        }
    }
    return inner;
}

// Give a declarator the type that applying it to base yields.
Ast_Type *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl)
{
    return Par_ApplyDerivs(base, decl->pc_head);
}

// Adjust a parameter's declared type the way C does: an array becomes a pointer, and so does a function.
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

// Build one named parameter, which a function definition later redeclares in its body's scope.
Ast_Var *Par_MakeParam(Ast_Type *base, Par_Decl *decl, int line)
{
    Ast_Type *type = Par_AdjustParam(Par_ApplyDecl(base, decl));
    Ast_Var  *var  = calloc(1, sizeof(Ast_Var));

    var->av_name = decl->pc_name;
    var->av_type = type;
    var->av_line = line;
    return var;
}

// Build one unnamed parameter, which a lone `void` declares none of.
Ast_Var *Par_MakeAnonParam(Ast_Type *type, int line)
{
    if (type->at_kind == AST_TYPE_KIND_VOID) {
        return NULL;
    }
    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_type = Par_AdjustParam(type);
    var->av_line = line;
    return var;
}

// Wrap base in the array dimensions listed outermost first.
Ast_Type *Par_ArrayType(Ast_Type *base, Ast_Node *dims)
{
    if (! dims) {
        return base;
    }
    return Ast_NewArray(Par_ArrayType(base, dims->an_next), (int) dims->an_val);
}

// The type __builtin_va_list names: the SysV record, as an array of one so passing it hands on its address.
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

// Build the node reading the next anonymous argument, which only a type one eightbyte carries may name.
Ast_Node *Par_VaArg(Ast_Node *ap, Ast_Type *type, int line)
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

// Narrow a member to the bits a `: width` gave it, rejecting a width C cannot grant.
void Par_AddBitfield(Ast_Member *member, Ast_Node *width, int line)
{
    long bits = 0;
    Ast_TypeKind kind = member->am_type->at_kind;

    if (! Sem_Fold(width, &bits)) {
        Log_ShowErrorAt(line, "a bit-field width is not a constant");
    }
    if (kind != AST_TYPE_KIND_INT && kind != AST_TYPE_KIND_CHAR) {
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
    member->am_bits = (int) bits;
}

// Turn one member declaration's declarators into members of the shared type.
Ast_Member *Par_MakeMembers(Ast_Type *type, Par_Decl *decls)
{
    Ast_Member head = {0};
    Ast_Member *tail = &head;

    for (Par_Decl *decl = decls; decl; decl = decl->pc_next) {
        tail->am_next = Ast_NewMember(decl->pc_name, Par_ApplyDecl(type, decl), decl->pc_line);
        tail = tail->am_next;
        // `T d[]` last in a struct is a flexible array member, which takes no space of its own.
        if (decl->pc_head && decl->pc_head->pd_kind == PAR_DERIV_ARRAY && decl->pc_head->pd_empty) {
            tail->am_flexible = 1;
        }
        if (decl->pc_bits) {
            Par_AddBitfield(tail, decl->pc_bits, decl->pc_line);
        }
    }
    return head.am_next;
}

// Open a struct or union definition, binding its tag first so a member may point back at the type.
Ast_Type *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, int line)
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

// Name a struct or union not yet defined, which makes `struct node *next;` legal inside `struct node`.
Ast_Type *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, int line)
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

// Declare one enumeration constant and step the value the next one takes.
void Par_AddEnumConst(const char *name, Ast_Node *value, int line)
{
    if (value && ! Sem_Fold(value, &Par_EnumValue)) {
        Log_ShowErrorAt(line, "enumerator '%s' is not a constant", name);
    }
    Ast_DeclareEnumConst(name, Par_EnumValue++);
}

// Build the statement writing one flattened initializer into its object, or into a member of it for a bitfield.
Ast_Node *Par_InitStore(Ast_Var *var, int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line)
{
    int at_off = bits ? off - bits->am_offset : off;
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

// Record one flattened initializer: its value, the slot's type and bitfield, and the slot's byte offset.
Ast_Node *Par_InitAt(int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_INIT, value, line);
    node->an_val    = off;
    node->an_type   = type;
    node->an_member = bits;
    return node;
}

// Move a cursor to the subobject a designator names.
void Par_Designate(Ast_Type *type, Ast_Node *desig, int *index, Ast_Member **member, int line)
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
        Log_ShowErrorAt(line, "initializer index %ld is outside the array", desig->an_val);
    }
    *index = (int) desig->an_val;
}

// Step a type and offset into the subobject one designator selected.
void Par_Step(Ast_Type **type, int *off, Ast_Node *desig, int index, Ast_Member *member)
{
    if (desig->an_memname) {
        *off += member->am_offset;
        *type = member->am_type;
        return;
    }
    *off += index * (*type)->at_base->at_size;
    *type = (*type)->at_base;
}

// The type an expression already has, for the forms the parser can answer without the Sem_ pass, else NULL.
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

// Fill one slot from the cursor, descending into an aggregate left unbraced unless the value fits it whole.
void Par_FlattenSlot(Ast_Type *type, int base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, int line)
{
    Ast_Node *value = (*item)->an_lhs;

    if (value->an_kind == AST_NODE_KIND_INITLIST) {
        Par_Flatten(type, base, bits, value, tail, line);
        *item = (*item)->an_next;
        return;
    }
    if (Sem_IsAggregate(type) && Par_ExprType(value) == type) {
        (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
        *tail = (*tail)->an_next;
        *item = (*item)->an_next;
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        Par_FlattenList(type, base, item, tail, 0, line);
        return;
    }
    (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
    *tail = (*tail)->an_next;
    *item = (*item)->an_next;
}

// Walk an aggregate's slots from the cursor: a braced list ends with its items, an elided one when full.
void Par_FlattenList(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int braced, int line)
{
    int index = 0;
    Ast_Member *member = type->at_members;

    while (*item) {
        if ((*item)->an_cond) {
            if (! braced) {
                return;
            }
            int off = base;
            Ast_Node *desig = (*item)->an_cond;
            Ast_Type *slot = type;

            Par_Designate(type, desig, &index, &member, line);
            Par_Step(&slot, &off, desig, index, member);
            Ast_Member *bits = desig->an_memname && member->am_bits ? member : NULL;
            for (Ast_Node *next = desig->an_next; next; next = next->an_next) {
                int at = 0;
                Ast_Member *inner = NULL;
                Par_Designate(slot, next, &at, &inner, line);
                Par_Step(&slot, &off, next, at, inner);
                bits = next->an_memname && inner->am_bits ? inner : NULL;
            }

            (*item)->an_cond = NULL;
            Par_Flatten(slot, off, bits, (*item)->an_lhs, tail, line);
            *item = (*item)->an_next;
            if (desig->an_memname) {
                member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
            } else {
                index++;
            }
            continue;
        }

        if (type->at_kind == AST_TYPE_KIND_ARRAY) {
            if (index >= type->at_len) {
                if (! braced) {
                    return;
                }
                Log_ShowErrorAt(line, "too many initializers for an array of %d", type->at_len);
            }
            Par_FlattenSlot(type->at_base, base + index * type->at_base->at_size, NULL, item, tail, line);
            index++;
            continue;
        }

        if (! member) {
            if (! braced) {
                return;
            }
            Log_ShowErrorAt(line, "too many initializers for '%s'", Sem_TypeName(type));
        }
        Par_FlattenSlot(member->am_type, base + member->am_offset, member->am_bits ? member : NULL, item, tail, line);
        member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
    }
}

// Flatten one initializer, braced or not, into the object at base.
void Par_Flatten(Ast_Type *type, int base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, int line)
{
    // A literal of the slot's own type fills it with the items it holds, which is the copy C asks for.
    if (init->an_kind == AST_NODE_KIND_COMPOUND && init->an_type == type) {
        for (Ast_Node *item = init->an_items; item; item = item->an_next) {
            (*tail)->an_next = Par_InitAt(base + (int) item->an_val, item->an_type, item->an_member, item->an_lhs, line);
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
    Par_FlattenList(type, base, &item, tail, 1, line);
}

// Flatten an initializer to the list of scalar writes that fill the object.
Ast_Node *Par_FlattenInit(Ast_Type *type, Ast_Node *init, int line)
{
    Ast_Node head = {0};
    Ast_Node *tail = &head;

    Par_Flatten(type, 0, NULL, init, &tail, line);
    return head.an_next;
}

// Lower an already flattened initializer to the statements filling a local, zeroing the whole object first.
Ast_Node *Par_InitFlat(Ast_Var *var, Ast_Node *flat, int line)
{
    Ast_Node *zero = Ast_NewUnary(AST_NODE_KIND_ZERO, Ast_NewVarNode(var, line), line);
    zero->an_val = var->av_type->at_size;

    Ast_Node *tail = zero;
    for (Ast_Node *item = flat; item; item = item->an_next) {
        tail->an_next = Par_InitStore(var, (int) item->an_val, item->an_type, item->an_member, item->an_lhs, line);
        tail = tail->an_next;
    }
    return zero;
}

// Lower a local's initializer to the statements that fill it.
Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, int line)
{
    if (init->an_kind != AST_NODE_KIND_INITLIST && var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }
    return Par_InitFlat(var, Par_FlattenInit(var->av_type, init, line), line);
}

// Build the unnamed object a compound literal names, hanging the statements that fill it off the node.
Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, int line)
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

    // Outside a function the object has static storage, so the linker lays it down and nothing has to run to fill it.
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

// Reject an object whose type has no size here; an extern is exempt, being sized in another file.
void Par_CheckComplete(const char *name, Ast_Type *type, int line)
{
    if (! type->at_complete && Par_DeclStorage != AST_STORAGE_EXTERN) {
        Log_ShowErrorAt(line, "'%s' has an incomplete type", name);
    }
}

// Declare one file-scope name of the declaration being parsed, which a function type makes a prototype.
void Par_AddDeclaredType(const char *name, Ast_Type *type, Ast_Node *init, int line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, type);
        return;
    }
    // A declarator that worked out to a function type declares a prototype, not an object.
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        Par_DeclarePrototype(name, type);
        return;
    }
    Par_CheckComplete(name, type, line);
    Ast_Var *var = Ast_DeclareGlobal(name, type, line);
    var->av_storage = Par_DeclStorage;
    var->av_init = init ? Par_FlattenInit(var->av_type, init, line) : NULL;
}

// Declare a variable inside a function, which `static` moves to file scope.
Ast_Var *Par_DeclareLocal(const char *name, Ast_Type *type, int line)
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

// Declare one local the declaration being parsed names, and build the statement its initializer becomes.
Ast_Node *Par_AddLocal(Par_Decl *decl, Ast_Node *init, int line)
{
    Ast_Var *var = Par_DeclareLocal(decl->pc_name, Par_ApplyDecl(Par_DeclType, decl), line);

    if (! init) {
        return Ast_NewNode(AST_NODE_KIND_NOP, line);
    }
    if (! var) {
        Log_ShowErrorAt(line, "a typedef takes no initializer");
    }
    // A `static` local lives in .data, so its initializer is an image rather than a statement.
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

// Append a function to the program, or fill in one a prototype declared.
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

// Record a prototype a declarator spelled out, so a call can find its return type and check its arity.
void Par_DeclarePrototype(const char *name, Ast_Type *type)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_name     = (char *) name;
    fn->af_ret      = type->at_ret;
    fn->af_params   = type->at_params;
    fn->af_nparams  = type->at_nparams;
    fn->af_variadic = type->at_variadic;
    fn->af_static   = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_AddFunction(fn);
}

// Build the function the parser has just read a parameter list for.
Ast_Func *Par_MakeFunction(Ast_Node *body)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_name     = Par_CurFuncName;
    fn->af_ret      = Par_CurRetType;
    fn->af_body     = body;
    fn->af_params   = Par_CurParams;
    fn->af_nparams  = Par_CurNumParams;
    fn->af_variadic = Par_CurVariadic;
    fn->af_static   = Par_CurStatic;
    fn->af_locals   = body ? Ast_CurrentLocals() : NULL;
    return fn;
}

// Note the declarator a top-level declaration named, opening a body scope when it declares a function.
void Par_BeginExternal(Par_Decl *decl, int line)
{
    Ast_Type *type = Par_ApplyDecl(Par_DeclType, decl);

    Par_DeclName    = decl->pc_name;
    Par_CurDeclType = type;
    if (type->at_kind != AST_TYPE_KIND_FUNC) {
        Par_InFunction = 0;
        return;
    }

    Par_CurStatic    = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_CurFuncName  = decl->pc_name;
    Par_CurRetType   = type->at_ret;
    Par_CurParams    = type->at_params;
    Par_CurNumParams = type->at_nparams;
    Par_CurVariadic  = type->at_variadic;
    Par_InFunction   = 1;

    // Declaring it before the body is what lets the body call it, which is how recursion resolves.
    Par_DeclarePrototype(decl->pc_name, type);

    // The parameters were built without a scope, so the body's scope is where they become visible.
    Ast_BeginScope();
    for (Ast_Var *param = type->at_params; param; param = param->av_param_next) {
        if (param->av_name) {
            Ast_DeclareParam(param);
        }
    }
    (void) line;
}

// Close a top-level declarator that turned out not to be a function definition.
void Par_EndExternal(Ast_Node *init, int line)
{
    if (Par_InFunction) {
        Par_AddFunction(Par_MakeFunction(NULL));
        Ast_EndScope();
        Par_InFunction = 0;
        return;
    }
    Par_AddDeclaredType(Par_DeclName, Par_CurDeclType, init, line);
}

// Close a function definition, which is the one case a body follows the declarator.
void Par_EndFunction(Ast_Node *body)
{
    Par_AddFunction(Par_MakeFunction(body));
    Ast_EndScope();
    Par_InFunction = 0;
}

// Declare one more top-level name after a comma, which shares the declaration's specifier.
void Par_AddDeclared(Par_Decl *decl, Ast_Node *init, int line)
{
    Par_AddDeclaredType(decl->pc_name, Par_ApplyDecl(Par_DeclType, decl), init, line);
}

// Resolve a name used as a value: a variable, or a function, which names its own address.
Ast_Node *Par_Designator(char *name, int line)
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

// Build a call, which a callee naming a function directly makes a direct one.
Ast_Node *Par_MakeCall(Ast_Node *callee, Ast_Node *args, int line)
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
