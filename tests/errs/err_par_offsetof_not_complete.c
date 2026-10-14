// (Test) Compiler error: [ERR_PAR_OFFSETOF_NOT_COMPLETE]
// Can't take the offset of a member of an incomplete struct or union.

struct S;

unsigned long off = __builtin_offsetof(struct S, x);
