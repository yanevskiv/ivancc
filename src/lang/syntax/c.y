/*
 * Grammar for the cc compiler.
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

%code requires {
    #include "lang/par.h"
}

%code {

#include "util/console/err.h"
#include "lang/ast.h"
#include "lang/sem.h"

// Give a rule the line of its first token.
#define YYLLOC_DEFAULT(cur, rhs, n)  ((cur) = (n) ? YYRHSLOC(rhs, 1) : YYRHSLOC(rhs, 0))

int  yylex(void);
void yyerror(const char *s);
}

%locations
%define api.location.type {Ast_Line}

%union {
    int64_t         val;
    Ast_TypeKind    kind;
    Ast_Qual        qual;
    Par_ArrayDecor  decor;
    Par_Num         num;
    Par_FNum        fnum;
    Par_Specs       specs;
    Par_Decl       *decl;
    Par_ParamList   params;
    Ast_Str         str;
    Ast_Node       *node;
    Ast_Type       *type;
    Ast_Member     *member;
    Ast_Var        *var;
    char           *name;
}

%token <num>  NUM
%token <fnum> FNUM
%token <name> IDENT
%token <str>  STR
%token INT CHAR SHORT LONG SIGNED UNSIGNED BOOL FLOAT DOUBLE VOID RETURN IF ELSE FOR WHILE DO BREAK CONTINUE SIZEOF
%token CONST VOLATILE RESTRICT
%token STRUCT UNION ENUM TYPEDEF
%token <name> TYPEDEF_NAME
%token SWITCH CASE DEFAULT GOTO
%token STATIC EXTERN REGISTER AUTO INLINE
%token BUILTIN_VA_LIST BUILTIN_VA_START BUILTIN_VA_ARG BUILTIN_VA_END
%token ADD SUB MUL DIV MOD ASSIGN NOT AMP PIPE CARET TILDE SHL SHR
%token INC DEC QUESTION COLON
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token AND_ASSIGN OR_ASSIGN XOR_ASSIGN SHL_ASSIGN SHR_ASSIGN
%token EQ NE LT GT LE GE AND OR
%token LPAREN RPAREN LSQUARE RSQUARE LBRACE RBRACE SEMI COMMA ELLIPSIS DOT ARROW

%type <node> stmt stmt_list compound_stmt decl decl_body local_list local_decl
%type <node> for_init expr expr_comma expr_opt args arg_list
%type <node> initializer init_body init_list init_item designators designator
%type <node> cast unary postfix primary
%type <str>  string
%type <var>  param
%type <decl> member_declarators member_declarator
%type <node> enumerator_opt
%type <member> members member_decl
%type <type> type_name decl_spec param_spec
%type <specs> spec_seq spec named_type
%type <decl> declarator direct_declarator abstract_declarator direct_abstract param_dims
%type <params> params param_list ident_list
%type <name> tag_name
%type <val>     stars pointer array_len
%type <kind>    struct_or_union
%type <qual>    qual
%type <decor>   array_decor

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

/* ---- top level ----------------------------------------------------- */

/* A whole source file. */
translation_unit
    : /* empty */
    | translation_unit external_decl
    ;

/* A file-scope declaration or function definition. */
external_decl
    : spec_seq
        { Par_SetDeclSpec(&$1, PAR_STORAGE_ANY, @1); }
      external_tail
    ;

/* The rest of an external declaration after its specifier. */
external_tail
    : SEMI
    | declarator { Par_BeginExternal($1, @1); } decl_tail
    ;

/* The rest of an external declaration after its declarator. */
decl_tail
    : knr_opt compound_stmt { Par_EndFunction($2); }
    | SEMI                 { Par_EndExternal(NULL, @1); }
    | ASSIGN initializer   { Par_EndExternal($2, @2); } global_rest SEMI
    | COMMA                { Par_EndExternal(NULL, @1); } global_decl global_rest SEMI
    ;

/* The declarators after the first in a file-scope declaration. */
global_rest
    : /* empty */
    | global_rest COMMA global_decl
    ;

/* An old-style definition's parameter declarations. */
knr_opt
    : /* empty */          { Par_BeginBody(); }
    | knr_decls            { Par_BeginBody(); }
    ;

/* One or more old-style parameter declarations. */
knr_decls
    : knr_decl
    | knr_decls knr_decl
    ;

/* One old-style parameter declaration. */
knr_decl
    : spec_seq { Par_SetDeclSpec(&$1, PAR_STORAGE_REGISTER, @1); } knr_declarators SEMI
    ;

/* The parameter names one old-style declaration types. */
knr_declarators
    : declarator                       { Par_SetKnrParam($1, @1); }
    | knr_declarators COMMA declarator { Par_SetKnrParam($3, @3); }
    ;

/* One file-scope declarator, with an optional initializer. */
global_decl
    : declarator                       { Par_AddDeclared($1, NULL, @1); }
    | declarator ASSIGN initializer    { Par_AddDeclared($1, $3, @1); }
    ;

/* A function declarator's parameter list. */
params
    : /* empty */          { Par_ClearParams(&$$); }
    | param_list           { $$ = $1; }
    | param_list COMMA ELLIPSIS
        { $$ = $1; $$.pl_variadic = AST_TYPE_VARIADIC; }
    | ident_list           { $$ = $1; }
    ;

/* An old-style definition's parameter names. */
ident_list
    : IDENT
        { Par_ClearParams(&$$); Par_PushParam(&$$, Par_MakeKnrParam($1, @1)); }
    | ident_list COMMA IDENT
        { $$ = $1; Par_PushParam(&$$, Par_MakeKnrParam($3, @3)); }
    ;

/* A prototype's parameters. */
param_list
    : param                { Par_ClearParams(&$$); $$.pl_proto = AST_TYPE_PROTO; Par_PushParam(&$$, $1); }
    | param_list COMMA param
        { $$ = $1; Par_PushParam(&$$, $3); }
    ;

/* One prototype parameter, named or not. */
param
    : param_spec declarator
        { $$ = Par_MakeParam($1, $2, @1); }
    | param_spec stars param_dims
        { Par_Decl *d = $3;
          for (int64_t i = 0; i < $2; i++) { Par_AddDeriv(d, PAR_DERIV_POINTER, @2); }
          $$ = Par_MakeAnonParam(Par_ApplyDecl($1, d), @1); }
    ;

/* The dimensions an unnamed parameter carries. */
param_dims
    : /* empty */          { $$ = Par_NewDecl(NULL); }
    | param_dims LSQUARE expr RSQUARE
        { $$ = $1; Par_SetArrayLen(Par_AddDeriv($$, PAR_DERIV_ARRAY, @2), $3); }
    | param_dims LSQUARE MUL RSQUARE
        { $$ = $1; Par_AddDeriv($$, PAR_DERIV_ARRAY, @2)->pd_star = true; }
    ;

/* ---- types --------------------------------------------------------- */

/* The type a member declaration or a type name opens with. */
decl_spec
    : spec_seq
        { Par_SpecsStorage(&$1, PAR_STORAGE_FORBIDDEN, @1); $$ = Par_SpecsType(&$1, @1); }
    ;

/* The type a parameter opens with. */
param_spec
    : spec_seq
        { Par_SpecsStorage(&$1, PAR_STORAGE_REGISTER, @1); $$ = Par_SpecsType(&$1, @1); }
    ;

/* The specifiers and qualifiers a declaration opens with. */
spec_seq
    : spec                 { Par_ClearSpecs(&$$); Par_TakeSpec(&$$, &$1, @1); }
    | spec_seq spec        { $$ = $1; Par_TakeSpec(&$$, &$2, @2); }
    ;

/* One type specifier or qualifier. */
spec
    : INT                  { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_INT; }
    | CHAR                 { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_CHAR; }
    | SHORT                { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_SHORT; }
    | LONG                 { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_LONG; }
    | SIGNED               { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_SIGNED; }
    | UNSIGNED             { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_UNSIGNED; }
    | BOOL                 { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_BOOL; }
    | FLOAT                { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_FLOAT; }
    | DOUBLE               { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_DOUBLE; }
    | VOID                 { Par_ClearSpecs(&$$); $$.ps_specs = PAR_SPEC_VOID; }
    | qual                 { Par_ClearSpecs(&$$); $$.ps_qual = $1; }
    | named_type           { $$ = $1; }
    | STATIC               { $$ = Par_StorageSpec(AST_STORAGE_STATIC); }
    | EXTERN               { $$ = Par_StorageSpec(AST_STORAGE_EXTERN); }
    | TYPEDEF              { $$ = Par_StorageSpec(AST_STORAGE_TYPEDEF); }
    | AUTO                 { $$ = Par_StorageSpec(AST_STORAGE_NONE); }
    | REGISTER             { $$ = Par_StorageSpec(AST_STORAGE_NONE); $$.ps_register = true; }
    | INLINE               { Par_ClearSpecs(&$$); $$.ps_inline = true; }
    ;

/* A type written without a name, as a cast or a sizeof takes. */
type_name
    : decl_spec                      { $$ = Par_ApplyDecl($1, Par_NewDecl(NULL)); }
    | decl_spec abstract_declarator  { $$ = Par_ApplyDecl($1, $2); }
    ;

/* The pointers, arrays and parameters a type name wraps its specifier in. */
abstract_declarator
    : pointer
        { $$ = Par_NewDecl(NULL);
          for (int64_t i = 0; i < $1; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @1); } }
    | pointer direct_abstract
        { $$ = $2;
          for (int64_t i = 0; i < $1; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @1); } }
    | direct_abstract      { $$ = $1; }
    ;

/* An abstract declarator without its leading pointers. */
direct_abstract
    : LPAREN abstract_declarator RPAREN
        { $$ = $2; }
    | LSQUARE expr RSQUARE
        { $$ = Par_NewDecl(NULL); Par_SetArrayLen(Par_AddDeriv($$, PAR_DERIV_ARRAY, @1), $2); }
    | direct_abstract LSQUARE expr RSQUARE
        { $$ = $1; Par_SetArrayLen(Par_AddDeriv($$, PAR_DERIV_ARRAY, @2), $3); }
    | LSQUARE RSQUARE
        { $$ = Par_NewDecl(NULL); Par_AddDeriv($$, PAR_DERIV_ARRAY, @1)->pd_empty = true; }
    | direct_abstract LSQUARE RSQUARE
        { $$ = $1; Par_AddDeriv($$, PAR_DERIV_ARRAY, @2)->pd_empty = true; }
    | LPAREN { Ast_PushScope(); } params RPAREN
        { Ast_PopScope(); $$ = Par_NewDecl(NULL); Par_AddDeriv($$, PAR_DERIV_FUNCTION, @1)->pd_params = $3; }
    | direct_abstract LPAREN { Ast_PushScope(); } params RPAREN
        { Ast_PopScope(); $$ = $1; Par_AddDeriv($$, PAR_DERIV_FUNCTION, @2)->pd_params = $4; }
    ;

/* A run of one or more pointer stars, each able to carry qualifiers of its own. */
pointer
    : stars MUL            { $$ = $1 + 1; }
    | stars MUL qual_list  { $$ = $1 + 1; }
    ;

/* A name with the pointers, arrays and parameters around it. */
declarator
    : stars direct_declarator
        { $$ = $2;
          for (int64_t i = 0; i < $1; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @1); } }
    ;

/* A declarator without its leading pointers. */
direct_declarator
    : IDENT                                  { $$ = Par_NewDecl($1); $$->pc_line = @1; }
    | LPAREN declarator RPAREN               { $$ = $2; }
    /* An unnamed declarator. */
    | LPAREN stars RPAREN
        { $$ = Par_NewDecl(NULL); $$->pc_line = @1;
          for (int64_t i = 0; i < $2; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @2); } }
    | direct_declarator LSQUARE array_decor expr RSQUARE
        { $$ = $1; Par_Deriv *d = Par_AddDeriv($$, PAR_DERIV_ARRAY, @2); Par_SetArrayLen(d, $4); d->pd_decor = $3; }
    | direct_declarator LSQUARE array_decor RSQUARE
        { $$ = $1; Par_Deriv *d = Par_AddDeriv($$, PAR_DERIV_ARRAY, @2); d->pd_empty = true; d->pd_decor = $3; }
    | direct_declarator LSQUARE array_decor MUL RSQUARE
        { $$ = $1; Par_Deriv *d = Par_AddDeriv($$, PAR_DERIV_ARRAY, @2); d->pd_star = true; d->pd_decor = $3; }
    | direct_declarator LPAREN { Ast_PushScope(); } params RPAREN
        { Ast_PopScope(); $$ = $1; Par_AddDeriv($$, PAR_DERIV_FUNCTION, @2)->pd_params = $4; }
    ;


/* The `static` and qualifiers a parameter's outermost array may carry. */
array_decor
    : /* empty */          { $$ = PAR_ARRAY_NONE; }
    | STATIC               { $$ = PAR_ARRAY_STATIC; }
    | STATIC qual_list     { $$ = PAR_ARRAY_STATIC | PAR_ARRAY_QUAL; }
    | qual_list            { $$ = PAR_ARRAY_QUAL; }
    | qual_list STATIC     { $$ = PAR_ARRAY_QUAL | PAR_ARRAY_STATIC; }
    ;

/* One or more type qualifiers. */
qual_list
    : qual
    | qual_list qual
    ;

/* One type qualifier. */
qual
    : CONST                { $$ = AST_QUAL_CONST; }
    | VOLATILE             { $$ = AST_QUAL_VOLATILE; }
    | RESTRICT             { $$ = AST_QUAL_RESTRICT; }
    ;

/* A type named rather than spelled out of keywords. */
named_type
    : struct_or_union tag_name LBRACE
        { $<type>$ = Par_BeginAggregate($1, $2, @2); }
      members RBRACE
        { Ast_LayoutAggregate($<type>4, $5, @1); Par_ClearSpecs(&$$); $$.ps_type = $<type>4; }
    | struct_or_union LBRACE
        { $<type>$ = Par_BeginAggregate($1, NULL, @1); }
      members RBRACE
        { Ast_LayoutAggregate($<type>3, $4, @1); Par_ClearSpecs(&$$); $$.ps_type = $<type>3; }
    | struct_or_union tag_name
        { Par_ClearSpecs(&$$); $$.ps_type = Par_ReferenceAggregate($1, $2, @2); }
    | ENUM tag_name LBRACE { Par_ResetEnum(); } enumerators RBRACE
        { Ast_DeclareTag($2, &Ast_TypeInt); Par_ClearSpecs(&$$); $$.ps_type = &Ast_TypeInt; }
    | ENUM LBRACE { Par_ResetEnum(); } enumerators RBRACE
        { Par_ClearSpecs(&$$); $$.ps_type = &Ast_TypeInt; }
    | ENUM tag_name        { Par_ClearSpecs(&$$); $$.ps_type = &Ast_TypeInt; }
    | TYPEDEF_NAME         { Par_ClearSpecs(&$$); $$.ps_type = Ast_FindTypedef($1); }
    | BUILTIN_VA_LIST      { Par_ClearSpecs(&$$); $$.ps_type = Par_VaListType(); }
    ;

/* The keyword that opens an aggregate. */
struct_or_union
    : STRUCT               { $$ = AST_TYPE_KIND_STRUCT; }
    | UNION                { $$ = AST_TYPE_KIND_UNION; }
    ;

/* A struct, union or enum tag. */
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; }
    ;

/* The members of a struct or union. */
members
    : /* empty */          { $$ = NULL; }
    | members member_decl  { $$ = Par_AppendMembers($1, $2); }
    ;

/* One member declaration. */
member_decl
    : decl_spec member_declarators SEMI  { $$ = Par_MakeMembers($1, $2); }
    ;

/* The members one declaration names. */
member_declarators
    : member_declarator                          { $$ = $1; }
    | member_declarators COMMA member_declarator
        { Par_Decl *last = $1;
          while (last->pc_next) { last = last->pc_next; }
          last->pc_next = $3; $$ = $1; }
    ;

/* A member, optionally narrowed to a bit-field width. */
member_declarator
    : declarator           { $$ = $1; $$->pc_line = @1; }
    | declarator COLON expr { $$ = $1; $$->pc_line = @1; $$->pc_bits = $3; }
    | COLON expr           { $$ = Par_NewDecl(NULL); $$->pc_line = @1; $$->pc_bits = $2; }
    ;

/* The constants an enum declares. */
enumerators
    : enumerator
    | enumerators COMMA
    | enumerators COMMA enumerator
    ;

/* One enum constant. */
enumerator
    : IDENT enumerator_opt { Par_AddEnumConst($1, $2, @1); }
    ;

/* The value an enum constant may fix. */
enumerator_opt
    : /* empty */          { $$ = NULL; }
    | ASSIGN expr          { $$ = $2; }
    ;

/* A run of pointer stars, each able to carry qualifiers of its own. */
stars
    : /* empty */          { $$ = 0; }
    | stars MUL            { $$ = $1 + 1; }
    | stars MUL qual_list  { $$ = $1 + 1; }
    ;

/* ---- statements ---------------------------------------------------- */

/* A braced block. */
compound_stmt
    : LBRACE { Ast_PushScope(); } stmt_list RBRACE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_BLOCK, @1); n->an_body = $3;
          Ast_PopScope(); $$ = n; }
    ;

/* The statements in a block. */
stmt_list
    : /* empty */          { $$ = NULL; }
    | stmt stmt_list       { $1->an_next = $2; $$ = $1; }
    ;

/* One statement. */
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
    | SWITCH LPAREN expr_comma RPAREN { $<node>$ = Par_NewJump(AST_NODE_KIND_SWITCH, @1); } stmt
        { Ast_Node *n = $<node>5; n->an_cond = $3; n->an_body = $6; $$ = n; }
    | CASE expr COLON { $<node>$ = Par_NewJump(AST_NODE_KIND_CASE, @1); } stmt
        { Ast_Node *n = $<node>4; n->an_cond = $2; n->an_lhs = $5; $$ = n; }
    | DEFAULT COLON { $<node>$ = Par_NewJump(AST_NODE_KIND_DEFAULT, @1); } stmt
        { Ast_Node *n = $<node>3; n->an_lhs = $4; $$ = n; }
    | GOTO IDENT SEMI
        { Ast_Node *n = Par_NewJump(AST_NODE_KIND_GOTO, @1); n->an_funcname = $2; $$ = n; }
    | IDENT COLON { $<node>$ = Par_NewJump(AST_NODE_KIND_LABEL, @1); } stmt
        { Ast_Node *n = $<node>3; n->an_funcname = $1; n->an_lhs = $4; $$ = n; }
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

/* A for-loop's first clause. */
for_init
    : expr_opt SEMI        { $$ = $1 ? Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, $1, @1) : NULL; }
    | decl SEMI            { $$ = $1; }
    ;

/* A block-scope declaration. */
decl
    : spec_seq { Par_SetDeclSpec(&$1, PAR_STORAGE_ANY, @1); } decl_body
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DECL, @1); n->an_body = $3; $$ = n; }
    ;

/* The declarators a block-scope declaration names. */
decl_body
    : /* empty */          { $$ = NULL; }
    | local_list           { $$ = $1; }
    ;

/* One or more local declarators. */
local_list
    : local_decl                 { $$ = $1; }
    | local_list COMMA local_decl
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

/* One local declarator, with an optional initializer. */
local_decl
    : declarator                       { $$ = Par_AddLocal($1, NULL, @1); }
    | declarator ASSIGN initializer    { $$ = Par_AddLocal($1, $3, @1); }
    ;

/* A scalar initializer. */
initializer
    : expr                       { $$ = $1; }
    | LBRACE init_body RBRACE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_INITLIST, @1); n->an_body = $2; $$ = n; }
    ;

/* The items between an initializer's braces, with an optional trailing comma. */
init_body
    : /* empty */                { $$ = NULL; }
    | init_list                  { $$ = $1; }
    | init_list COMMA            { $$ = $1; }
    ;

/* The items in a braced initializer. */
init_list
    : init_item                  { $$ = $1; }
    | init_list COMMA init_item
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

/* One initializer item, with the designators that aim it. */
init_item
    : initializer                { $$ = Ast_NewUnary(AST_NODE_KIND_INIT, $1, @1); }
    | designators ASSIGN initializer
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_INIT, $3, @1); n->an_cond = $1; $$ = n; }
    ;

/* The designators aiming one initializer item. */
designators
    : designator                 { $$ = $1; }
    | designators designator
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $2; $$ = $1; }
    ;

/* One designator, an array index or a member name. */
designator
    : LSQUARE array_len RSQUARE
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_val = $2; $$ = n; }
    | DOT IDENT
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_memname = $2; $$ = n; }
    ;

/* An array length. */
array_len
    : expr
        { int64_t val;
          Err_AssertAt(@1, Sem_Fold($1, &val), ERR_PAR_ARRAY_LEN_NOT_CONSTANT);
          $$ = val; }
    ;

/* A comma expression. */
expr_comma
    : expr                       { $$ = $1; }
    | expr_comma COMMA expr      { $$ = Ast_NewBinary(AST_NODE_KIND_COMMA, $1, $3, @2); }
    ;

/* An expression a for-clause may leave out. */
expr_opt
    : /* empty */          { $$ = NULL; }
    | expr_comma           { $$ = $1; }
    ;

/* ---- expressions --------------------------------------------------- */

/* An expression, with every binary operator on one rule. */
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

/* A cast. */
cast
    : unary                { $$ = $1; }
    | LPAREN type_name RPAREN cast
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_CAST, $4, @1); n->an_type = $2;
          $$ = Par_WithSizes($2, n, @1); }
    ;

/* A prefix operator applied to an expression. */
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
        { $$ = Par_SizeOfType($3, @1); }
    ;

/* A postfix operator applied to an expression. */
postfix
    : postfix LPAREN args RPAREN
        { $$ = Par_MakeCall($1, $3, @2); }
    | primary              { $$ = $1; }
    | postfix INC          { $$ = Ast_NewPostInc($1, 1, @2); }
    | postfix DEC          { $$ = Ast_NewPostInc($1, -1, @2); }
    | postfix LSQUARE expr RSQUARE
        { Ast_Node *n = Ast_NewBinary(AST_NODE_KIND_ADD, $1, $3, @2);
          $$ = Ast_NewUnary(AST_NODE_KIND_DEREF, n, @2); }
    | postfix DOT IDENT    { $$ = Ast_NewMemberNode($1, $3, @2); }
    | postfix ARROW IDENT
        { $$ = Ast_NewMemberNode(Ast_NewUnary(AST_NODE_KIND_DEREF, $1, @2), $3, @2); }
    | LPAREN type_name RPAREN LBRACE init_body RBRACE
        { $$ = Par_CompoundLiteral($2, $5, @1); }
    ;

/* An operand that stands alone. */
primary
    : NUM                  { $$ = Ast_NewNum($1.pn_val, @1); $$->an_type = $1.pn_type; }
    | FNUM                 { $$ = Ast_NewFNum($1.pf_val, $1.pf_type, @1); }
    | string               { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_STR, @1);
                             n->an_str_idx = Ast_AddString($1.as_data, $1.as_len, $1.as_width); $$ = n; }
    | IDENT
        { int64_t val;
          if (Ast_FindEnumConst($1, &val)) { $$ = Ast_NewNum(val, @1); }
          else { $$ = Par_Designator($1, @1); } }
    | LPAREN expr_comma RPAREN { $$ = $2; }
    | BUILTIN_VA_START LPAREN expr COMMA expr RPAREN
        { $$ = Ast_NewUnary(AST_NODE_KIND_VA_START, $3, @1); }
    | BUILTIN_VA_ARG LPAREN expr COMMA type_name RPAREN
        { $$ = Par_VaArg($3, $5, @1); }
    /* va_end has nothing to undo. */
    | BUILTIN_VA_END LPAREN expr RPAREN
        { $$ = $3; }
    ;

/* One string literal, or several written next to each other. */
string
    : STR                  { $$ = $1; }
    | string STR           { $$ = Par_ConcatStrings($1, $2); }
    ;

/* A call's argument list. */
args
    : /* empty */          { $$ = NULL; }
    | arg_list             { $$ = $1; }
    ;

/* A call's arguments. */
arg_list
    : expr                 { $$ = $1; }
    | expr COMMA arg_list  { $1->an_next = $3; $$ = $1; }
    ;

%%

// Report a parse error and stop.
void yyerror(const char *s)
{
    Err_RaiseAt(yylloc, ERR_PAR_SYNTAX, s);
}
