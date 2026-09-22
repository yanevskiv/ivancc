## Lexer

Lexer work is mostly straightforward token recognition, but typedef names add one context-sensitive requirement. The lexer must distinguish identifiers that name types from ordinary identifiers, which requires consulting information maintained by the parser.

### Keywords and operators

The lexer needs two additions for aggregate types and typedefs. It must recognize four new keywords, add the two member-access operators, and distinguish typedef names from ordinary identifiers.

The keyword additions are mechanical. `struct`, `union`, `enum`, and `typedef` each receive their own token, `STRUCT`, `UNION`, `ENUM`, and `TYPEDEF`, matching the corresponding C keyword.

The operator additions are similarly small. `->` is added alongside the other two-character operators and `.` alongside the single-character operators.

Flex always chooses the longest matching rule, so `...` wins over `.` and `->` wins over `-` regardless of their order. We still write the longer forms first because that ordering also works in lexers that choose the first matching rule.

```c
/* Keywords */
"struct"    return STRUCT;
"union"     return UNION;
"enum"      return ENUM;
"typedef"   return TYPEDEF;

/* Operators */
/* "..."    return ELLIPSIS; */
"->"        return ARROW;
"."         return DOT;
/* "-"      return MINUS; */
```

### Typedef names

Typedef names introduce a conflict with ordinary identifiers. If every identifier becomes `IDENT`, then `Point p;` and `count p;` produce the same sequence of tokens. However, if `Point` is a typedef name and `count` is an ordinary identifier, the parser needs to distinguish a valid declaration from invalid syntax.

Nothing in the identifier itself provides that distinction. The lexer therefore looks up each identifier in the parser's current scope chain and returns `TYPEDEF_NAME` when the name refers to a typedef, or `IDENT` otherwise. Whether an identifier becomes `IDENT` or `TYPEDEF_NAME` depends on the typedef bindings visible at the point where that identifier is lexed.

```c
/* Identifiers */
{ALPHA}{ALNUM}* {
    yylval.str = strdup(yytext);
    return Ast_FindTypedef(yytext) ? TYPEDEF_NAME : IDENT;
}
```

### Typedef name registration

Registration has to happen as each typedef declarator is reduced. For example, in `typedef int Foo;`, `Foo` must initially lex as `IDENT` because it is not a typedef name until its declarator has been parsed.

Waiting for the entire declaration to finish is too late when a declaration contains multiple declarators or when the lexer has already been asked for a subsequent token. Each name must become visible to the lexer before it reads a later use of that name.

With Bison, this ordering follows naturally because the reduction occurs while the parser still holds its current lookahead token. Registering the name during that reduction makes the new binding visible before the lexer is asked for the next token.

The example below requires `Foo` and `Bar` to be registered independently as their declarators are reduced. Their later occurrences then reach the parser as `TYPEDEF_NAME`.

```c
typedef int Foo, Bar; /* Foo and Bar are registered as their declarators reduce. */
Foo a;                /* Foo now lexes as TYPEDEF_NAME. */
Bar b;                /* Bar now lexes as TYPEDEF_NAME. */
```

### Typedef names as identifiers

A name classified as `TYPEDEF_NAME` can still appear in positions where C uses an ordinary identifier. Once `Foo` is registered as a typedef, the lexer returns `TYPEDEF_NAME` for later occurrences even when the name is being used as a structure member or label rather than as a type.

Supporting these cases requires the grammar to accept `TYPEDEF_NAME` in identifier positions where C permits a typedef name to be reused. Our grammar does not make that distinction, so these otherwise valid uses are currently rejected.

The structure member in the example below is valid C, but our parser rejects it because `Foo` reaches the grammar as `TYPEDEF_NAME` where it expects `IDENT`. Labels that reuse a typedef name have the same limitation.

```c
typedef int Foo; /* Foo becomes a typedef name. */
struct S {
    int Foo;     /* Foo is a member name here. */
};
```
