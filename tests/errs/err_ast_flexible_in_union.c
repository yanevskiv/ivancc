// (Test) Compiler error: [ERR_AST_FLEXIBLE_IN_UNION]
// Can't have a flexible array member in a union.

union U { int n; int d[]; };
