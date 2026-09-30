// (Test) Compiler error: [ERR_AST_AGGREGATE_UNNAMED]
// Can't have a struct with no named member.
// Note: gcc accepts it.

struct S { int : 3; };
