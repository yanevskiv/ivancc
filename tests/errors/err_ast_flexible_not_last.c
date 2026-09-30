// (Test) Compiler error: [ERR_AST_FLEXIBLE_NOT_LAST]
// Can't have a member after a flexible array member.

struct S { int n; int d[]; int m; };
