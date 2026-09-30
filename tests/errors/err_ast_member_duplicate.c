// (Test) Compiler error: [ERR_AST_MEMBER_DUPLICATE]
// Can't have two members with the same name.

struct S { int a; int a; };
