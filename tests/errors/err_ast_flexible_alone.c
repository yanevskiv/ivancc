// (Test) Compiler error: [ERR_AST_FLEXIBLE_ALONE]
// Can't have a flexible array member as a struct's only member.

struct S { int d[]; };
