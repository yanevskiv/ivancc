## Lexer

Lexer recognition normally reduces fixed spellings to tokens, but typedef names require parser scope information that no standalone rule can provide. The stage adds aggregate keywords and member operators, then makes identifier lexing return `TYPEDEF_NAME` when a visible typedef binds the name. This keeps the grammar's lookahead sufficient for the aggregate tests, while later context tracking can narrow positions where typedef names are legal.

### Add: `STRUCT`, `UNION`, `ENUM` and `TYPEDEF`

A keyword is a rule that matches a fixed spelling and returns a token carrying no value. Each gains a rule of its own, returning `STRUCT`, `UNION`, `ENUM` or `TYPEDEF`. Everything else about them is mechanical, which is why they are worth writing before the interesting problem.

```c
/* Keywords */
"struct"    return STRUCT;
"union"     return UNION;
"enum"      return ENUM;
"typedef"   return TYPEDEF;
```

### Add: `ARROW` and `DOT`

The member operators are `->` and `.`, which reach a member through a pointer and through an object. `ARROW` joins the other two-character operators and `DOT` the single-character ones. Writing the longer forms first costs nothing and keeps the rules correct in a lexer that takes the first match instead.

```c
/* Operators */
/* "..."    return ELLIPSIS; */
"->"        return ARROW;
"."         return DOT;
/* "-"      return MINUS; */
```

### Modify: `{ALPHA}{ALNUM}*`

The identifier rule returned `IDENT` for every name it matched, which every earlier stage was satisfied by. The rule now looks each name up in the parser's scope chain and returns `TYPEDEF_NAME` when a typedef binds it. Two consequences follow from that, and both look like bugs the first time they appear.

```c
/* Identifiers */
{ALPHA}{ALNUM}* {
    yylval.str = strdup(yytext);
    return Ast_FindTypedef(yytext) ? TYPEDEF_NAME : IDENT;
}
```

### Add: `TYPEDEF_NAME`

`TYPEDEF_NAME` is the token a bound name now produces, and the parser accepts it wherever a type specifier belongs. Registration therefore happens as each declarator reduces rather than at the semicolon. Each name has to become visible before the lexer reads a later use of it.

```c
typedef int Foo, Bar; /* Foo and Bar are registered as their declarators reduce. */
Foo a;                /* Foo now lexes as TYPEDEF_NAME. */
Bar b;                /* Bar now lexes as TYPEDEF_NAME. */
```

### Modify: `IDENT`

`IDENT` used to cover every identifier the lexer matched, including one that names a structure member or a label. The grammar keeps `IDENT` for the positions a type specifier cannot appear in. A production compiler narrows the loss by tracking whether a declarator is expected, which this one does not.

```c
typedef int Foo; /* Foo becomes a typedef name. */
struct S {
    int Foo;     /* an IDENT position that now receives TYPEDEF_NAME */
};
```
