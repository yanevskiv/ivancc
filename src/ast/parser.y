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
#include "util/log.h"
#include "util/str.h"
#include "ast/ast.h"

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

/* Build the statement `var[index] = value`. */
static Ast_Node *Par_InitElement(Ast_Var *var, long index, Ast_Node *value, int line)
{
    Ast_Node *at = Ast_NewBinary(AST_NODE_KIND_ADD, Ast_NewVarNode(var, line), Ast_NewNum(index, line), line);
    Ast_Node *elem = Ast_NewUnary(AST_NODE_KIND_DEREF, at, line);
    Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, elem, value, line);
    return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
}

/* Index the next element of an initializer list takes, which a designator
   resets. C numbers from zero and steps by one unless told otherwise. */
static long Par_InitIndex;

/* Lower a local's initializer to the statements that fill it: every element
   zeroed first, so that what the list leaves out is zero as C requires. */
static Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, int line)
{
    if (init->an_kind != AST_NODE_KIND_INIT) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }

    if (var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Log_ShowErrorAt(line, "'%s' is not an array, so it takes no initializer list", var->av_name);
    }

    Ast_Node head = {0};
    Ast_Node *tail = &head;
    for (int i = 0; i < var->av_type->at_len; i++) {
        tail->an_next = Par_InitElement(var, i, Ast_NewNum(0, line), line);
        tail = tail->an_next;
    }
    for (Ast_Node *item = init; item; item = item->an_next) {
        tail->an_next = Par_InitElement(var, item->an_val, item->an_lhs, line);
        tail = tail->an_next;
    }
    return head.an_next;
}

/* Declare one file-scope variable of the declaration being parsed. */
static void Par_AddGlobal(const char *name, Ast_Node *dims, Ast_Node *init, int line)
{
    Ast_Var *var = Ast_DeclareGlobal(name, Par_ArrayType(Par_DeclType, dims), line);
    var->av_storage = Par_DeclStorage;
    var->av_init = init;
}

/* Declare a variable inside a function, which `static` moves to file scope. */
static Ast_Var *Par_DeclareLocal(const char *name, Ast_Type *type, int line)
{
    if (Par_DeclStorage != AST_STORAGE_STATIC) {
        return Ast_DeclareVar(name, type, line);
    }
    char *symbol = Str_Format("%s.%s", Par_CurFuncName, name);
    Ast_Var *var = Ast_DeclareStaticLocal(name, symbol, type, line);
    var->av_storage = AST_STORAGE_STATIC;
    return var;
}

/* Append a finished function to the program. */
static void Par_AddFunction(Ast_Func *fn)
{
    fn->af_next = NULL;
    if (! Par_ProgHead) {
        Par_ProgHead = Par_ProgTail = fn;
    } else {
        Par_ProgTail->af_next = fn;
        Par_ProgTail = fn;
    }
    Ast_Program = Par_ProgHead;
}
%}

%locations
%define api.location.type {int}

%union {
    long      num;
    char     *str;
    Ast_Str   str_lit;
    Ast_Node *node;
    Ast_Type *type;
}

%token <num>     NUM
%token <str>     IDENT
%token <str_lit> STR
%token INT CHAR VOID CONST RETURN IF ELSE FOR WHILE DO BREAK CONTINUE SIZEOF
%token SWITCH CASE DEFAULT GOTO
%token STATIC EXTERN REGISTER AUTO INLINE
%token BUILTIN_VA_ARG
%token ADD SUB MUL DIV MOD ASSIGN NOT AMP PIPE CARET TILDE SHL SHR
%token INC DEC QUESTION COLON
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token AND_ASSIGN OR_ASSIGN XOR_ASSIGN SHL_ASSIGN SHR_ASSIGN
%token EQ NE LT GT LE GE AND OR
%token LPAREN RPAREN LSQUARE RSQUARE LBRACE RBRACE SEMI COMMA ELLIPSIS

%type <node> stmt stmt_list compound_stmt decl local_list local_decl
%type <node> for_init expr expr_comma expr_opt args arg_list
%type <node> initializer init_list init_item
%type <node> cast unary postfix primary array_dims param_dims
%type <type> type_name base
%type <num>  stars storage

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
    : storage type_name IDENT
        { Par_DeclStorage = $1; Par_DeclType = $2; Par_DeclName = $3; }
      decl_tail
    ;

decl_tail
    : LPAREN
        {
            Par_CurStatic     = Par_DeclStorage == AST_STORAGE_STATIC;
            Par_CurFuncName   = Par_DeclName;
            Par_CurParams     = NULL;
            Par_CurParamsTail = NULL;
            Par_CurNumParams  = 0;
            Par_CurVariadic   = 0;
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
        {
            Ast_Func *fn = calloc(1, sizeof(Ast_Func));
            fn->af_name     = Par_CurFuncName;
            fn->af_body     = $1;
            fn->af_params   = Par_CurParams;
            fn->af_nparams  = Par_CurNumParams;
            fn->af_variadic = Par_CurVariadic;
            fn->af_static   = Par_CurStatic;
            fn->af_locals   = Ast_CurrentLocals();
            Par_AddFunction(fn);
        }
    | SEMI  /* a prototype, e.g. `int printf(const char *, ...);` -- discard */
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
    : quals base stars
        { Ast_Type *t = $2;
          for (int i = 0; i < $3; i++) { t = Ast_NewPointer(t); }
          $$ = t; }
    ;

quals
    : /* empty */
    | quals CONST
    ;

base
    : INT                  { $$ = &Ast_TypeInt; }
    | CHAR                 { $$ = &Ast_TypeChar; }
    | VOID                 { $$ = &Ast_TypeVoid; }
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
    : storage type_name { Par_DeclType = $2; Par_DeclStorage = $1; } local_list
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_BLOCK, @2); n->an_body = $4; $$ = n; }
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
          if (v->av_global) {
              v->av_init = $4;
              $$ = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          } else {
              $$ = Par_InitLocal(v, $4, @1);
          }
        }
    ;

/* A scalar initializer, or a braced list of elements. */
initializer
    : expr                       { $$ = $1; }
    | LBRACE { Par_InitIndex = 0; } init_list RBRACE { $$ = $3; }
    ;

init_list
    : init_item                  { $$ = $1; }
    | init_list COMMA            { $$ = $1; }
    | init_list COMMA init_item
        { Ast_Node *last = $1;
          while (last->an_next) { last = last->an_next; }
          last->an_next = $3; $$ = $1; }
    ;

init_item
    : expr
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_INIT, $1, @1);
          n->an_val = Par_InitIndex++; $$ = n; }
    | LSQUARE NUM RSQUARE ASSIGN expr
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_INIT, $5, @1);
          Par_InitIndex = $2; n->an_val = Par_InitIndex++; $$ = n; }
    | LBRACE init_list RBRACE
        { Log_ShowErrorAt(@1, "nested initializer lists are not supported yet"); $$ = $2; }
    ;

array_dims
    : /* empty */          { $$ = NULL; }
    | LSQUARE NUM RSQUARE array_dims
        { Ast_Node *n = Ast_NewNum($2, @1); n->an_next = $4; $$ = n; }
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
    ;

primary
    : NUM                  { $$ = Ast_NewNum($1, @1); }
    | STR                  { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_STR, @1);
                             n->an_str_idx = Ast_AddString($1.as_data, $1.as_len); $$ = n; }
    | IDENT
        { Ast_Var *v = Ast_FindVar($1);
          if (! v) Log_ShowErrorAt(@1, "use of undeclared identifier '%s'", $1);
          $$ = Ast_NewVarNode(v, @1); }
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
