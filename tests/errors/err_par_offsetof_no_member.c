// (Test) Compiler error: [ERR_PAR_OFFSETOF_NO_MEMBER]
// Can't take the offset of a member the struct does not have.

struct S { int a; };

unsigned long off = __builtin_offsetof(struct S, b);
