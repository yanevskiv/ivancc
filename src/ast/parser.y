/* parser.y - grammar for the cc compiler (a small subset of C).
 *
 * Generates build/parser.tab.c and build/parser.tab.h
 * (via `bison -d -o build/parser.tab.c`).
 *
 * Binary operators are deliberately flat: their precedence comes from the
 * %left / %right declarations below rather than from a tower of non-terminals.
 * The prefix and postfix operators do need levels of their own, because
 * `sizeof` takes a unary-expression: that is what stops `sizeof (int) * n`
 * from parsing as `sizeof ((int) * n)`.
 */

%code requires {
    #include "ast/ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/log.h"
#include "util/str.h"
#include "ast/ast.h"
#include "ast/sem.h"

int  yylex(void);
void yyerror(const char *s);

/* Give a rule the line of its first token, or of the preceding one if empty. */
#define YYLLOC_DEFAULT(cur, rhs, n)  ((cur) = (n) ? YYRHSLOC(rhs, 1) : YYRHSLOC(rhs, 0))

/* State for the function definition currently being parsed. */
static char     *Par_CurFuncName;
static Ast_Var  *Par_CurParams;
static Ast_Var  *Par_CurParamsTail;
static int       Par_CurNumParams;
static int       Par_CurVariadic;
static int       Par_CurStatic;
static Ast_Type *Par_CurRetType;
static int       Par_InFunction;

/* Serial number the next compound literal names its object with. */
static int Par_CompoundCount;

/* The type and storage class the declarators being parsed all share. */
static Ast_Type   *Par_DeclType;
static Ast_Storage Par_DeclStorage;
static char       *Par_DeclName;

/* The program assembled so far, as functions are reduced. */
static Ast_Func *Par_ProgHead;
static Ast_Func *Par_ProgTail;

/* Append a parameter to the function currently being parsed. */
static void Par_AddParam(Ast_Var *v)
{
    v->av_param_next = NULL;
    if (! Par_CurParams) {
        Par_CurParams = Par_CurParamsTail = v;
    } else {
        Par_CurParamsTail->av_param_next = v;
        Par_CurParamsTail = v;
    }
    Par_CurNumParams++;
}

/* Count a parameter a prototype left unnamed, which a definition cannot have.
   A lone `void` names no parameter at all, being how C spells an empty list. */
static void Par_AddAnonParam(Ast_Type *type)
{
    if (type->at_kind != AST_TYPE_KIND_VOID) {
        Par_CurNumParams++;
    }
}

/* Wrap base in the array dimensions listed outermost first. */
static Ast_Type *Par_ArrayType(Ast_Type *base, Ast_Node *dims)
{
    if (! dims) {
        return base;
    }
    return Ast_NewArray(Par_ArrayType(base, dims->an_next), (int) dims->an_val);
}

/* Give a parameter its adjusted type: an array parameter is really a pointer. */
static Ast_Type *Par_ParamType(Ast_Type *base, Ast_Node *dims)
{
    if (! dims) {
        return base;
    }
    return Ast_NewPointer(Par_ArrayType(base, dims->an_next));
}

/* Value the next enumerator takes, which `= n` resets. */
static long Par_EnumValue;

/* Join two member lists, keeping declaration order. */
static Ast_Member *Par_AppendMembers(Ast_Member *head, Ast_Member *tail)
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

/* Narrow a member to the bits a `: width` gave it, rejecting a width C cannot grant. */
static void Par_AddBitfield(Ast_Member *member, Ast_Node *width, int line)
{
    Ast_TypeKind kind = member->am_type->at_kind;

    if (width->an_kind != AST_NODE_KIND_NUM) {
        Log_ShowErrorAt(line, "a bit-field width is not a constant");
    }
    if (kind != AST_TYPE_KIND_INT && kind != AST_TYPE_KIND_CHAR) {
        Log_ShowErrorAt(line, "a bit-field must have an integer type");
    }
    if (width->an_val < 0) {
        Log_ShowErrorAt(line, "a bit-field width cannot be negative");
    }
    if (width->an_val > member->am_type->at_size * AST_BITS_PER_BYTE) {
        Log_ShowErrorAt(line, "a bit-field is wider than the type that holds it");
    }
    if (width->an_val == 0 && member->am_name) {
        Log_ShowErrorAt(line, "a bit-field with a name cannot be zero bits wide");
    }
    member->am_bits = (int) width->an_val;
}

/* Turn one member declaration's declarators into members of the shared type. */
static Ast_Member *Par_MakeMembers(Ast_Type *type, Ast_Node *decls)
{
    Ast_Member head = {0};
    Ast_Member *tail = &head;

    for (Ast_Node *decl = decls; decl; decl = decl->an_next) {
        tail->am_next = Ast_NewMember(decl->an_memname, Par_ArrayType(type, decl->an_lhs), decl->an_line);
        tail = tail->am_next;
        if (decl->an_val) {
            tail->am_type = Ast_NewArray(type, 0);
            tail->am_flexible = 1;
        }
        if (decl->an_rhs) {
            Par_AddBitfield(tail, decl->an_rhs, decl->an_line);
        }
    }
    return head.am_next;
}

/* Open a struct or union definition, binding its tag before the members are
   read so that a member may point back at the type being defined. */
static Ast_Type *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, int line)
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

/* Name a struct or union that may not have been defined yet, which is what
   makes `struct node *next;` legal inside `struct node`. */
static Ast_Type *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, int line)
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

/* Declare one enumeration constant and step the value the next one takes. */
static void Par_AddEnumConst(const char *name, Ast_Node *value, int line)
{
    if (value) {
        if (value->an_kind != AST_NODE_KIND_NUM) {
            Log_ShowErrorAt(line, "enumerator '%s' is not a constant", name);
        }
        Par_EnumValue = value->an_val;
    }
    Ast_DeclareEnumConst(name, Par_EnumValue++);
}

/* Build the statement writing one flattened initializer into its object, as `*(T *)((char *) &var + off) = value` or, for a bitfield, as a member of it. */
static Ast_Node *Par_InitStore(Ast_Var *var, int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line)
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

/* Record one flattened initializer: a value, the slot's type and bitfield if it has one, and the byte offset of that slot within the object. */
static Ast_Node *Par_InitAt(int off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_INIT, value, line);
    node->an_val    = off;
    node->an_type   = type;
    node->an_member = bits;
    return node;
}

/* Move a cursor to the subobject a designator names. */
static void Par_Designate(Ast_Type *type, Ast_Node *desig, int *index, Ast_Member **member, int line)
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

/* Step a type and offset into the subobject one designator selected. */
static void Par_Step(Ast_Type **type, int *off, Ast_Node *desig, int index, Ast_Member *member)
{
    if (desig->an_memname) {
        *off += member->am_offset;
        *type = member->am_type;
        return;
    }
    *off += index * (*type)->at_base->at_size;
    *type = (*type)->at_base;
}

static void Par_Flatten(Ast_Type *type, int base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, int line);
static void Par_FlattenList(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int braced, int line);
static Ast_Func *Par_FindFunction(const char *name);

/* The type an expression already has, for the forms the parser can answer without the Sem_ pass, or NULL where it cannot tell. */
static Ast_Type *Par_ExprType(Ast_Node *node)
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

/* Fill one slot from the cursor, descending into an aggregate the source left unbraced unless the value already has the slot's own type. */
static void Par_FlattenSlot(Ast_Type *type, int base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, int line)
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

/* Walk the slots of an array, struct or union, taking items from the cursor. A
   braced list ends with its items; an elided one ends when the object is full. */
static void Par_FlattenList(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int braced, int line)
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

/* Flatten one initializer, braced or not, into the object at base. */
static void Par_Flatten(Ast_Type *type, int base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, int line)
{
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

/* Flatten an initializer to the list of scalar writes that fill the object. */
static Ast_Node *Par_FlattenInit(Ast_Type *type, Ast_Node *init, int line)
{
    Ast_Node head = {0};
    Ast_Node *tail = &head;

    Par_Flatten(type, 0, NULL, init, &tail, line);
    return head.an_next;
}

/* Lower a local's initializer to the statements that fill it, zeroing the whole object first so what the list leaves out is zero. */
static Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, int line)
{
    if (init->an_kind != AST_NODE_KIND_INITLIST && var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }

    Ast_Node *zero = Ast_NewUnary(AST_NODE_KIND_ZERO, Ast_NewVarNode(var, line), line);
    zero->an_val = var->av_type->at_size;

    Ast_Node *tail = zero;
    for (Ast_Node *item = Par_FlattenInit(var->av_type, init, line); item; item = item->an_next) {
        tail->an_next = Par_InitStore(var, (int) item->an_val, item->an_type, item->an_member, item->an_lhs, line);
        tail = tail->an_next;
    }
    return zero;
}

/* Build the unnamed object a compound literal names, hanging the statements that fill it off the node so each evaluation runs them again. */
static Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, int line)
{
    if (! Par_InFunction) {
        Log_ShowErrorAt(line, "a compound literal outside a function needs static storage, which is not supported");
    }
    if (! type->at_complete) {
        Log_ShowErrorAt(line, "a compound literal of an incomplete type has no size");
    }

    Ast_Node *list = Ast_NewNode(AST_NODE_KIND_INITLIST, line);
    list->an_body = items;

    Ast_Var *var = Ast_DeclareVar(Str_Format(".compound.%d", Par_CompoundCount++), type, line);
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_COMPOUND, line);
    node->an_var  = var;
    node->an_type = type;
    node->an_body = Par_InitLocal(var, list, line);
    return node;
}

/* Reject an object declared with a type whose size is not known here. An
   extern is exempt: the definition that sizes it is in another file. */
static void Par_CheckComplete(const char *name, Ast_Type *type, int line)
{
    if (! type->at_complete && Par_DeclStorage != AST_STORAGE_EXTERN) {
        Log_ShowErrorAt(line, "'%s' has an incomplete type", name);
    }
}

/* Declare one file-scope variable of the declaration being parsed. */
static void Par_AddGlobal(const char *name, Ast_Node *dims, Ast_Node *init, int line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, Par_ArrayType(Par_DeclType, dims));
        return;
    }
    Par_CheckComplete(name, Par_ArrayType(Par_DeclType, dims), line);
    Ast_Var *var = Ast_DeclareGlobal(name, Par_ArrayType(Par_DeclType, dims), line);
    var->av_storage = Par_DeclStorage;
    var->av_init = init ? Par_FlattenInit(var->av_type, init, line) : NULL;
}

/* Declare a variable inside a function, which `static` moves to file scope. */
static Ast_Var *Par_DeclareLocal(const char *name, Ast_Type *type, int line)
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

/* Find a function already declared or defined under name. */
static Ast_Func *Par_FindFunction(const char *name)
{
    for (Ast_Func *fn = Par_ProgHead; fn; fn = fn->af_next) {
        if (strcmp(fn->af_name, name) == 0) {
            return fn;
        }
    }
    return NULL;
}

/* Append a function to the program, or fill in one a prototype declared. */
static void Par_AddFunction(Ast_Func *fn)
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

/* Build the function the parser has just read a parameter list for. */
static Ast_Func *Par_MakeFunction(Ast_Node *body)
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
%}

%locations
%define api.location.type {int}

%union {
    long        num;
    char       *str;
    Ast_Str     str_lit;
    Ast_Node   *node;
    Ast_Type   *type;
    Ast_Member *member;
}

%token <num>     NUM
%token <str>     IDENT
%token <str_lit> STR
%token INT CHAR VOID CONST RETURN IF ELSE FOR WHILE DO BREAK CONTINUE SIZEOF
%token STRUCT UNION ENUM TYPEDEF
%token <str> TYPEDEF_NAME
%token SWITCH CASE DEFAULT GOTO
%token STATIC EXTERN REGISTER AUTO INLINE
%token BUILTIN_VA_ARG
%token ADD SUB MUL DIV MOD ASSIGN NOT AMP PIPE CARET TILDE SHL SHR
%token INC DEC QUESTION COLON
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token AND_ASSIGN OR_ASSIGN XOR_ASSIGN SHL_ASSIGN SHR_ASSIGN
%token EQ NE LT GT LE GE AND OR
%token LPAREN RPAREN LSQUARE RSQUARE LBRACE RBRACE SEMI COMMA ELLIPSIS DOT ARROW

%type <node> stmt stmt_list compound_stmt decl decl_body local_list local_decl
%type <node> for_init expr expr_comma expr_opt args arg_list
%type <node> initializer init_list init_item designators designator
%type <node> cast unary postfix primary array_dims param_dims
%type <node> member_declarators member_declarator enumerator_opt
%type <member> members member_decl
%type <type> type_name base
%type <str>  tag_name
%type <num>  stars storage struct_or_union array_len

/* Lowest precedence first. */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%right AND_ASSIGN OR_ASSIGN XOR_ASSIGN SHL_ASSIGN SHR_ASSIGN
%right QUESTION COLON
%left OR
%left AND
%left PIPE
%left CARET
%left AMP
%left EQ NE
%left LT GT LE GE
%left SHL SHR
%left ADD SUB
%left MUL DIV MOD

%start translation_unit

%%

/* ---- top level: function definitions and prototypes ---------------- */

translation_unit
    : /* empty */
    | translation_unit external_decl
    ;

/* A declaration and a definition share `storage type_name IDENT`, so the name
   is recorded before the parser decides which of the two it is reading. */
external_decl
    : storage type_name
        { Par_DeclStorage = $1; Par_DeclType = $2; }
      external_tail
    ;

/* A struct, union or enum declaration stands alone; anything else goes on to
   name something, and a definition and a declaration share the name itself. */
external_tail
    : SEMI
    | IDENT { Par_DeclName = $1; } decl_tail
    ;

decl_tail
    : LPAREN
        {
            Par_CurStatic     = Par_DeclStorage == AST_STORAGE_STATIC;
            Par_CurFuncName   = Par_DeclName;
            Par_CurRetType    = Par_DeclType;
            Par_CurParams     = NULL;
            Par_CurParamsTail = NULL;
            Par_CurNumParams  = 0;
            Par_CurVariadic   = 0;
            Par_InFunction    = 1;
            Ast_BeginScope();
        }
      params RPAREN func_tail
    | array_dims               { Par_AddGlobal(Par_DeclName, $1, NULL, @1); } global_rest SEMI
    | array_dims ASSIGN initializer { Par_AddGlobal(Par_DeclName, $1, $3, @1); } global_rest SEMI
    ;

global_rest
    : /* empty */
    | global_rest COMMA global_decl
    ;

/* Storage classes. register, auto and inline parse and do nothing. */
storage
    : /* empty */          { $$ = AST_STORAGE_NONE; }
    | STATIC               { $$ = AST_STORAGE_STATIC; }
    | EXTERN               { $$ = AST_STORAGE_EXTERN; }
    | TYPEDEF              { $$ = AST_STORAGE_TYPEDEF; }
    | REGISTER             { $$ = AST_STORAGE_NONE; }
    | AUTO                 { $$ = AST_STORAGE_NONE; }
    | INLINE               { $$ = AST_STORAGE_NONE; }
    ;

global_decl
    : IDENT array_dims             { Par_AddGlobal($1, $2, NULL, @1); }
    | IDENT array_dims ASSIGN initializer { Par_AddGlobal($1, $2, $4, @1); }
    ;

func_tail
    : compound_stmt
        { Par_AddFunction(Par_MakeFunction($1)); Ast_EndScope(); Par_InFunction = 0; }
    | SEMI  /* a prototype: kept, so a call can find the return type */
        { Par_AddFunction(Par_MakeFunction(NULL)); Ast_EndScope(); Par_InFunction = 0; }
    ;

params
    : /* empty */
    | param_list
    ;

param_list
    : param
    | param_list COMMA param
    ;

param
    : type_name IDENT param_dims
        { Par_AddParam(Ast_DeclareVar($2, Par_ParamType($1, $3), @2)); }
    | type_name         /* unnamed parameter, e.g. `void` */
        { Par_AddAnonParam($1); }
    | ELLIPSIS          { Par_CurVariadic = 1; }
    ;

/* A parameter may leave its first dimension empty, as `int a[]` does. */
param_dims
    : array_dims                  { $$ = $1; }
    | LSQUARE RSQUARE array_dims
        { Ast_Node *n = Ast_NewNum(0, @1); n->an_next = $3; $$ = n; }
    ;

/* ---- types -------------------------------------------------------- */

type_name
    : quals base stars array_dims
        { Ast_Type *t = $2;
          for (int i = 0; i < $3; i++) { t = Ast_NewPointer(t); }
          $$ = Par_ArrayType(t, $4); }
    ;

quals
    : /* empty */
    | quals CONST
    ;

base
    : INT                  { $$ = &Ast_TypeInt; }
    | CHAR                 { $$ = &Ast_TypeChar; }
    | VOID                 { $$ = &Ast_TypeVoid; }
    | struct_or_union tag_name LBRACE
        { $<type>$ = Par_BeginAggregate($1, $2, @2); }
      members RBRACE
        { Ast_LayoutAggregate($<type>4, $5, @1); $$ = $<type>4; }
    | struct_or_union LBRACE
        { $<type>$ = Par_BeginAggregate($1, NULL, @1); }
      members RBRACE
        { Ast_LayoutAggregate($<type>3, $4, @1); $$ = $<type>3; }
    | struct_or_union tag_name
        { $$ = Par_ReferenceAggregate($1, $2, @2); }
    | ENUM tag_name LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { Ast_DeclareTag($2, &Ast_TypeInt); $$ = &Ast_TypeInt; }
    | ENUM LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { $$ = &Ast_TypeInt; }
    | ENUM tag_name        { $$ = &Ast_TypeInt; }
    | TYPEDEF_NAME         { $$ = Ast_FindTypedef($1); }
    ;

struct_or_union
    : STRUCT               { $$ = AST_TYPE_KIND_STRUCT; }
    | UNION                { $$ = AST_TYPE_KIND_UNION; }
    ;

/* A tag shares no namespace with ordinary identifiers, so a name already bound
   by a typedef -- as `typedef struct node node;` binds one -- is a tag here. */
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; }
    ;

members
    : /* empty */          { $$ = NULL; }
    | members member_decl  { $$ = Par_AppendMembers($1, $2); }
    ;

member_decl
    : type_name member_declarators SEMI  { $$ = Par_MakeMembers($1, $2); }
    ;

member_declarators
    : member_declarator                          { $$ = $1; }
    | member_declarators COMMA member_declarator
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

/* A member carries its name, its dimensions, an_val for a flexible array and an_rhs for a bitfield width; the declaration supplies the type. */
member_declarator
    : IDENT array_dims
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_memname = $1; n->an_lhs = $2; $$ = n; }
    | IDENT LSQUARE RSQUARE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_memname = $1; n->an_val = 1; $$ = n; }
    | IDENT COLON expr
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_memname = $1; n->an_rhs = $3; $$ = n; }
    | COLON expr
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_rhs = $2; $$ = n; }
    ;

enumerators
    : enumerator
    | enumerators COMMA
    | enumerators COMMA enumerator
    ;

enumerator
    : IDENT enumerator_opt { Par_AddEnumConst($1, $2, @1); }
    ;

enumerator_opt
    : /* empty */          { $$ = NULL; }
    | ASSIGN expr          { $$ = $2; }
    ;

stars
    : /* empty */          { $$ = 0; }
    | stars MUL            { $$ = $1 + 1; }
    ;

/* ---- statements ---------------------------------------------------- */

compound_stmt
    : LBRACE { Ast_PushScope(); } stmt_list RBRACE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_BLOCK, @1); n->an_body = $3;
          Ast_PopScope(); $$ = n; }
    ;

stmt_list
    : /* empty */          { $$ = NULL; }
    | stmt stmt_list       { $1->an_next = $2; $$ = $1; }
    ;

stmt
    : RETURN expr SEMI     { $$ = Ast_NewUnary(AST_NODE_KIND_RETURN, $2, @1); }
    | RETURN SEMI          { $$ = Ast_NewUnary(AST_NODE_KIND_RETURN, NULL, @1); }
    | IF LPAREN expr RPAREN stmt %prec LOWER_THAN_ELSE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_IF, @1);
          n->an_cond = $3; n->an_then = $5; $$ = n; }
    | IF LPAREN expr RPAREN stmt ELSE stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_IF, @1);
          n->an_cond = $3; n->an_then = $5; n->an_els = $7; $$ = n; }
    | FOR LPAREN { Ast_PushScope(); } for_init expr_opt SEMI expr_opt RPAREN stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_FOR, @1);
          n->an_init = $4; n->an_cond = $5; n->an_inc = $7; n->an_body = $9;
          Ast_PopScope(); $$ = n; }
    | DO stmt WHILE LPAREN expr_comma RPAREN SEMI
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DO, @1);
          n->an_body = $2; n->an_cond = $5; $$ = n; }
    | SWITCH LPAREN expr_comma RPAREN stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_SWITCH, @1);
          n->an_cond = $3; n->an_body = $5; $$ = n; }
    | CASE expr COLON stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_CASE, @1);
          n->an_cond = $2; n->an_lhs = $4; $$ = n; }
    | DEFAULT COLON stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DEFAULT, @1); n->an_lhs = $3; $$ = n; }
    | GOTO IDENT SEMI
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_GOTO, @1); n->an_funcname = $2; $$ = n; }
    | IDENT COLON stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_LABEL, @1);
          n->an_funcname = $1; n->an_lhs = $3; $$ = n; }
    | BREAK SEMI           { $$ = Ast_NewNode(AST_NODE_KIND_BREAK, @1); }
    | CONTINUE SEMI        { $$ = Ast_NewNode(AST_NODE_KIND_CONTINUE, @1); }
    | WHILE LPAREN expr RPAREN stmt
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_FOR, @1);
          n->an_cond = $3; n->an_body = $5; $$ = n; }
    | compound_stmt        { $$ = $1; }
    | decl SEMI            { $$ = $1; }
    | expr_comma SEMI      { $$ = Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, $1, @1); }
    | SEMI                 { $$ = Ast_NewNode(AST_NODE_KIND_NOP, @1); }
    ;

/* A for-loop's first clause, which may declare the variable it counts with. */
for_init
    : expr_opt SEMI        { $$ = $1 ? Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, $1, @1) : NULL; }
    | decl SEMI            { $$ = $1; }
    ;

decl
    : storage type_name { Par_DeclType = $2; Par_DeclStorage = $1; } decl_body
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_BLOCK, @2); n->an_body = $4; $$ = n; }
    ;

decl_body
    : /* empty */          { $$ = NULL; }
    | local_list           { $$ = $1; }
    ;

local_list
    : local_decl                 { $$ = $1; }
    | local_list COMMA local_decl
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

local_decl
    : IDENT array_dims
        { Par_DeclareLocal($1, Par_ArrayType(Par_DeclType, $2), @1);
          $$ = Ast_NewNode(AST_NODE_KIND_NOP, @1); }
    | IDENT array_dims ASSIGN initializer
        { Ast_Var *v = Par_DeclareLocal($1, Par_ArrayType(Par_DeclType, $2), @1);
          if (! v) {
              Log_ShowErrorAt(@1, "a typedef takes no initializer");
          } else if (v->av_global) {
              v->av_init = Par_FlattenInit(v->av_type, $4, @1);
              $$ = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          } else {
              $$ = Par_InitLocal(v, $4, @1);
          }
        }
    ;

/* A scalar initializer, or a braced list of items. */
initializer
    : expr                       { $$ = $1; }
    | LBRACE init_list RBRACE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_INITLIST, @1); n->an_body = $2; $$ = n; }
    ;

init_list
    : /* empty */                { $$ = NULL; }
    | init_item                  { $$ = $1; }
    | init_list COMMA            { $$ = $1; }
    | init_list COMMA init_item
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

/* One item, which a designator list may aim at a subobject of its own. */
init_item
    : initializer                { $$ = Ast_NewUnary(AST_NODE_KIND_INIT, $1, @1); }
    | designators ASSIGN initializer
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_INIT, $3, @1); n->an_cond = $1; $$ = n; }
    ;

designators
    : designator                 { $$ = $1; }
    | designators designator
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $2; $$ = $1; }
    ;

designator
    : LSQUARE array_len RSQUARE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_val = $2; $$ = n; }
    | DOT IDENT
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_memname = $2; $$ = n; }
    ;

array_dims
    : /* empty */          { $$ = NULL; }
    | LSQUARE array_len RSQUARE array_dims
        { Ast_Node *n = Ast_NewNum($2, @1); n->an_next = $4; $$ = n; }
    ;

/* An array's length is a constant expression, of which we fold the two forms
   that reach a declarator: a literal, and an enumeration constant. */
array_len
    : NUM                  { $$ = $1; }
    | IDENT
        { long val;
          if (! Ast_FindEnumConst($1, &val)) {
              Log_ShowErrorAt(@1, "'%s' is not a constant", $1);
          }
          $$ = val; }
    ;

/* A comma expression, which an argument list deliberately cannot contain. */
expr_comma
    : expr                       { $$ = $1; }
    | expr_comma COMMA expr      { $$ = Ast_NewBinary(AST_NODE_KIND_COMMA, $1, $3, @2); }
    ;

expr_opt
    : /* empty */          { $$ = NULL; }
    | expr_comma           { $$ = $1; }
    ;

/* ---- expressions --------------------------------------------------- */

expr
    : cast                 { $$ = $1; }
    | expr ADD expr        { $$ = Ast_NewBinary(AST_NODE_KIND_ADD, $1, $3, @2); }
    | expr SUB expr        { $$ = Ast_NewBinary(AST_NODE_KIND_SUB, $1, $3, @2); }
    | expr MUL expr        { $$ = Ast_NewBinary(AST_NODE_KIND_MUL, $1, $3, @2); }
    | expr DIV expr        { $$ = Ast_NewBinary(AST_NODE_KIND_DIV, $1, $3, @2); }
    | expr MOD expr        { $$ = Ast_NewBinary(AST_NODE_KIND_MOD, $1, $3, @2); }
    | expr EQ expr         { $$ = Ast_NewBinary(AST_NODE_KIND_EQ, $1, $3, @2); }
    | expr NE expr         { $$ = Ast_NewBinary(AST_NODE_KIND_NE, $1, $3, @2); }
    | expr LT expr         { $$ = Ast_NewBinary(AST_NODE_KIND_LT, $1, $3, @2); }
    | expr GT expr         { $$ = Ast_NewBinary(AST_NODE_KIND_LT, $3, $1, @2); }  /* a>b  == b<a  */
    | expr LE expr         { $$ = Ast_NewBinary(AST_NODE_KIND_LE, $1, $3, @2); }
    | expr GE expr         { $$ = Ast_NewBinary(AST_NODE_KIND_LE, $3, $1, @2); }  /* a>=b == b<=a */
    | expr AMP expr        { $$ = Ast_NewBinary(AST_NODE_KIND_BITAND, $1, $3, @2); }
    | expr PIPE expr       { $$ = Ast_NewBinary(AST_NODE_KIND_BITOR, $1, $3, @2); }
    | expr CARET expr      { $$ = Ast_NewBinary(AST_NODE_KIND_BITXOR, $1, $3, @2); }
    | expr SHL expr        { $$ = Ast_NewBinary(AST_NODE_KIND_SHL, $1, $3, @2); }
    | expr SHR expr        { $$ = Ast_NewBinary(AST_NODE_KIND_SHR, $1, $3, @2); }
    | expr AND expr        { $$ = Ast_NewBinary(AST_NODE_KIND_AND, $1, $3, @2); }
    | expr OR expr         { $$ = Ast_NewBinary(AST_NODE_KIND_OR, $1, $3, @2); }
    | expr ASSIGN expr     { $$ = Ast_NewBinary(AST_NODE_KIND_ASSIGN, $1, $3, @2); }
    | expr QUESTION expr COLON expr
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_COND, @2);
          n->an_cond = $1; n->an_then = $3; n->an_els = $5; $$ = n; }
    | expr ADD_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_ADD, $1, $3, @2); }
    | expr SUB_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_SUB, $1, $3, @2); }
    | expr MUL_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_MUL, $1, $3, @2); }
    | expr DIV_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_DIV, $1, $3, @2); }
    | expr MOD_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_MOD, $1, $3, @2); }
    | expr AND_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_BITAND, $1, $3, @2); }
    | expr OR_ASSIGN expr  { $$ = Ast_NewOpAssign(AST_NODE_KIND_BITOR, $1, $3, @2); }
    | expr XOR_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_BITXOR, $1, $3, @2); }
    | expr SHL_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_SHL, $1, $3, @2); }
    | expr SHR_ASSIGN expr { $$ = Ast_NewOpAssign(AST_NODE_KIND_SHR, $1, $3, @2); }
    ;

cast
    : unary                { $$ = $1; }
    | LPAREN type_name RPAREN cast
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_CAST, $4, @1); n->an_type = $2; $$ = n; }
    ;

unary
    : postfix              { $$ = $1; }
    | SUB cast             { $$ = Ast_NewUnary(AST_NODE_KIND_NEG, $2, @1); }
    | NOT cast             { $$ = Ast_NewUnary(AST_NODE_KIND_NOT, $2, @1); }
    | TILDE cast           { $$ = Ast_NewUnary(AST_NODE_KIND_BITNOT, $2, @1); }
    | ADD cast             { $$ = $2; }
    | MUL cast             { $$ = Ast_NewUnary(AST_NODE_KIND_DEREF, $2, @1); }
    | AMP cast             { $$ = Ast_NewUnary(AST_NODE_KIND_ADDR, $2, @1); }
    | INC unary            { $$ = Ast_NewOpAssign(AST_NODE_KIND_ADD, $2, Ast_NewNum(1, @1), @1); }
    | DEC unary            { $$ = Ast_NewOpAssign(AST_NODE_KIND_SUB, $2, Ast_NewNum(1, @1), @1); }
    | SIZEOF unary         { $$ = Ast_NewUnary(AST_NODE_KIND_SIZEOF, $2, @1); }
    | SIZEOF LPAREN type_name RPAREN
        { $$ = Ast_NewNum($3->at_size, @1); }
    ;

postfix
    : primary              { $$ = $1; }
    | postfix INC          { $$ = Ast_NewPostInc($1, 1, @2); }
    | postfix DEC          { $$ = Ast_NewPostInc($1, -1, @2); }
    | postfix LSQUARE expr RSQUARE
        { Ast_Node *n = Ast_NewBinary(AST_NODE_KIND_ADD, $1, $3, @2);
          $$ = Ast_NewUnary(AST_NODE_KIND_DEREF, n, @2); }
    | postfix DOT IDENT    { $$ = Ast_NewMemberNode($1, $3, @2); }
    | postfix ARROW IDENT
        { $$ = Ast_NewMemberNode(Ast_NewUnary(AST_NODE_KIND_DEREF, $1, @2), $3, @2); }
    | LPAREN type_name RPAREN LBRACE init_list RBRACE
        { $$ = Par_CompoundLiteral($2, $5, @1); }
    ;

primary
    : NUM                  { $$ = Ast_NewNum($1, @1); }
    | STR                  { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_STR, @1);
                             n->an_str_idx = Ast_AddString($1.as_data, $1.as_len); $$ = n; }
    | IDENT
        { long val;
          if (Ast_FindEnumConst($1, &val)) { $$ = Ast_NewNum(val, @1); }
          else {
              Ast_Var *v = Ast_FindVar($1);
              if (! v) Log_ShowErrorAt(@1, "use of undeclared identifier '%s'", $1);
              $$ = Ast_NewVarNode(v, @1);
          } }
    | IDENT LPAREN args RPAREN
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_CALL, @1);
          n->an_funcname = $1; n->an_args = $3; $$ = n; }
    | LPAREN expr_comma RPAREN { $$ = $2; }
    | BUILTIN_VA_ARG LPAREN expr RPAREN
        { $$ = Ast_NewUnary(AST_NODE_KIND_VA_ARG, $3, @1); }
    ;

args
    : /* empty */          { $$ = NULL; }
    | arg_list             { $$ = $1; }
    ;

arg_list
    : expr                 { $$ = $1; }
    | expr COMMA arg_list  { $1->an_next = $3; $$ = $1; }
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "cc: parse error: %s near line %d\n", s, yylloc);
    exit(1);
}
