// (Test) Compiler error: [ERR_PAR_DESIG_NO_MEMBER]
// Can't designate a member the struct does not have.

struct S { int a; } s = { .b = 1 };
