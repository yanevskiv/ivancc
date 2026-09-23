/* Grammar for the cc compiler: binary operators are flat, taking precedence from the %left and %right lists. */

%code requires {
    #include "syntax/par.h"
}

%code {
#include <stdio.h>

#include "util/log.h"
#include "syntax/ast.h"
#include "syntax/sem.h"

// Give a rule the line of its first token, or of the preceding one if empty.
#define YYLLOC_DEFAULT(cur, rhs, n)  ((cur) = (n) ? YYRHSLOC(rhs, 1) : YYRHSLOC(rhs, 0))

int  yylex(void);
void yyerror(const char *s);
}

%locations
%define api.location.type {int}

%union {
    long        num;
    char       *str;
    Ast_Str     str_lit;
    Ast_Node   *node;
    Ast_Type   *type;
    Ast_Member *member;
    Par_Decl   *decl;
    Par_ParamList params;
    Ast_Var    *var;
}

%token <num>     NUM
%token <str>     IDENT
%token <str_lit> STR
%token INT CHAR VOID CONST RETURN IF ELSE FOR WHILE DO BREAK CONTINUE SIZEOF
%token STRUCT UNION ENUM TYPEDEF
%token <str> TYPEDEF_NAME
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
%type <node> initializer init_list init_item designators designator
%type <node> cast unary postfix primary array_dims
%type <var>  param
%type <decl> member_declarators member_declarator
%type <node> enumerator_opt
%type <member> members member_decl
%type <type> type_name base decl_spec
%type <decl> declarator direct_declarator
%type <params> params param_list ident_list
%type <str>  tag_name
%type <num>  stars storage struct_or_union array_len array_decor

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

/* A declaration and a definition share `storage decl_spec`, so the specifier is recorded first. */
external_decl
    : storage decl_spec
        { Par_SetDeclSpec($1, $2); }
      external_tail
    ;

/* A struct, union or enum declaration stands alone; anything else goes on to name something. */
external_tail
    : SEMI
    | declarator { Par_BeginExternal($1, @1); } decl_tail
    ;

/* Only a body settles that a function declarator was a definition, so the scope is opened before it. */
decl_tail
    : knr_opt compound_stmt { Par_EndFunction($2); }
    | SEMI                 { Par_EndExternal(NULL, @1); }
    | ASSIGN initializer   { Par_EndExternal($2, @2); } global_rest SEMI
    | COMMA                { Par_EndExternal(NULL, @1); } global_decl global_rest SEMI
    ;

global_rest
    : /* empty */
    | global_rest COMMA global_decl
    ;

/* The declaration list an old-style definition puts between its `)` and its `{`. */
knr_opt
    : /* empty */          { Par_CheckKnrParams(); }
    | knr_decls            { Par_CheckKnrParams(); }
    ;

knr_decls
    : knr_decl
    | knr_decls knr_decl
    ;

knr_decl
    : storage decl_spec { Par_SetDeclSpec($1, $2); } knr_declarators SEMI
    ;

knr_declarators
    : declarator                       { Par_SetKnrParam($1, @1); }
    | knr_declarators COMMA declarator { Par_SetKnrParam($3, @3); }
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
    : declarator                       { Par_AddDeclared($1, NULL, @1); }
    | declarator ASSIGN initializer    { Par_AddDeclared($1, $3, @1); }
    ;

/* An empty list is `int f()`, which specifies nothing; `(void)` is how C spells a list of none. */
params
    : /* empty */          { Par_ClearParams(&$$); }
    | param_list           { $$ = $1; }
    | param_list COMMA ELLIPSIS
        { $$ = $1; $$.pl_variadic = 1; }
    | ident_list           { $$ = $1; }
    ;

/* An old-style definition names its parameters here and types them in the declaration list below. */
ident_list
    : IDENT
        { Par_ClearParams(&$$); Par_PushParam(&$$, Par_MakeKnrParam($1, @1)); }
    | ident_list COMMA IDENT
        { $$ = $1; Par_PushParam(&$$, Par_MakeKnrParam($3, @3)); }
    ;

param_list
    : param                { Par_ClearParams(&$$); $$.pl_proto = 1; Par_PushParam(&$$, $1); }
    | param_list COMMA param
        { $$ = $1; Par_PushParam(&$$, $3); }
    ;

param
    : decl_spec declarator
        { $$ = Par_MakeParam($1, $2, @1); }
    | decl_spec stars array_dims
        { Ast_Type *t = $1;
          for (int i = 0; i < $2; i++) { t = Ast_NewPointer(t); }
          $$ = Par_MakeAnonParam(Par_ArrayType(t, $3), @1); }
    ;

/* ---- types -------------------------------------------------------- */

/* The part of a declaration every declarator in it shares, with no pointers or dimensions of its own. */
decl_spec
    : quals base           { $$ = $2; }
    ;

/* An abstract declarator, for a cast, a sizeof, a compound literal or an unnamed parameter. */
type_name
    : decl_spec stars array_dims
        { Ast_Type *t = $1;
          for (int i = 0; i < $2; i++) { t = Ast_NewPointer(t); }
          $$ = Par_ArrayType(t, $3); }
    ;

/* A declarator reads outward from the name: the pointers outside it are recorded after its suffixes. */
declarator
    : stars direct_declarator
        { $$ = $2;
          for (int i = 0; i < $1; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @1); } }
    ;

direct_declarator
    : IDENT                                  { $$ = Par_NewDecl($1); $$->pc_line = @1; }
    | LPAREN declarator RPAREN               { $$ = $2; }
    /* A declarator with no name to give, which only a parameter may be: `int (*)(int)`. */
    | LPAREN stars RPAREN
        { $$ = Par_NewDecl(NULL); $$->pc_line = @1;
          for (int i = 0; i < $2; i++) { Par_AddDeriv($$, PAR_DERIV_POINTER, @2); } }
    | direct_declarator LSQUARE array_decor array_len RSQUARE
        { $$ = $1; Par_Deriv *d = Par_AddDeriv($$, PAR_DERIV_ARRAY, @2); d->pd_len = $4; d->pd_decor = $3; }
    | direct_declarator LSQUARE array_decor RSQUARE
        { $$ = $1; Par_Deriv *d = Par_AddDeriv($$, PAR_DERIV_ARRAY, @2); d->pd_empty = 1; d->pd_decor = $3; }
    | direct_declarator LPAREN { Ast_PushScope(); } params RPAREN
        { Ast_PopScope(); $$ = $1; Par_AddDeriv($$, PAR_DERIV_FUNCTION, @2)->pd_params = $4; }
    ;

quals
    : /* empty */
    | quals CONST
    ;

/* `int a[static 4]` and `int a[const 4]`, which only a parameter's outermost array may carry. */
array_decor
    : /* empty */          { $$ = 0; }
    | STATIC quals         { $$ = PAR_ARRAY_STATIC; }
    | qual_list            { $$ = PAR_ARRAY_QUAL; }
    | qual_list STATIC     { $$ = PAR_ARRAY_QUAL | PAR_ARRAY_STATIC; }
    ;

qual_list
    : CONST
    | qual_list CONST
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
    | ENUM tag_name LBRACE { Par_ResetEnum(); } enumerators RBRACE
        { Ast_DeclareTag($2, &Ast_TypeInt); $$ = &Ast_TypeInt; }
    | ENUM LBRACE { Par_ResetEnum(); } enumerators RBRACE
        { $$ = &Ast_TypeInt; }
    | ENUM tag_name        { $$ = &Ast_TypeInt; }
    | TYPEDEF_NAME         { $$ = Ast_FindTypedef($1); }
    | BUILTIN_VA_LIST      { $$ = Par_VaListType(); }
    ;

struct_or_union
    : STRUCT               { $$ = AST_TYPE_KIND_STRUCT; }
    | UNION                { $$ = AST_TYPE_KIND_UNION; }
    ;

/* A tag shares no namespace with ordinary identifiers, so a name a typedef bound is a tag here. */
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; }
    ;

members
    : /* empty */          { $$ = NULL; }
    | members member_decl  { $$ = Par_AppendMembers($1, $2); }
    ;

member_decl
    : decl_spec member_declarators SEMI  { $$ = Par_MakeMembers($1, $2); }
    ;

member_declarators
    : member_declarator                          { $$ = $1; }
    | member_declarators COMMA member_declarator
        { Par_Decl *last = $1;
          while (last->pc_next) { last = last->pc_next; }
          last->pc_next = $3; $$ = $1; }
    ;

/* A member is a declarator, optionally narrowed to a bit-field width. */
member_declarator
    : declarator           { $$ = $1; $$->pc_line = @1; }
    | declarator COLON expr { $$ = $1; $$->pc_line = @1; $$->pc_bits = $3; }
    | COLON expr           { $$ = Par_NewDecl(NULL); $$->pc_line = @1; $$->pc_bits = $2; }
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
    : storage decl_spec { Par_SetDeclSpec($1, $2); } decl_body
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
    : declarator                       { $$ = Par_AddLocal($1, NULL, @1); }
    | declarator ASSIGN initializer    { $$ = Par_AddLocal($1, $3, @1); }
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

/* A length must fold to a constant here, since a variable one would be a VLA. */
array_len
    : expr
        { long val;
          if (! Sem_Fold($1, &val)) {
              Log_ShowErrorAt(@1, "an array length is not a constant");
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
          else { $$ = Par_Designator($1, @1); } }
    | LPAREN expr_comma RPAREN { $$ = $2; }
    | BUILTIN_VA_START LPAREN expr COMMA expr RPAREN
        { $$ = Ast_NewUnary(AST_NODE_KIND_VA_START, $3, @1); }
    | BUILTIN_VA_ARG LPAREN expr COMMA type_name RPAREN
        { $$ = Par_VaArg($3, $5, @1); }
    /* va_end has nothing to undo, so it becomes the evaluation of its operand. */
    | BUILTIN_VA_END LPAREN expr RPAREN
        { $$ = $3; }
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

// Report a parse error and stop.
void yyerror(const char *s)
{
    fprintf(stderr, "cc: parse error: %s near line %d\n", s, yylloc);
    exit(1);
}
