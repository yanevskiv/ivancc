// (Test) Compiler error: [ERR_PAR_OFFSETOF_NOT_ARRAY]
// Can't take the offset of an index into something that is not an array.

struct S { int a; int b; };

unsigned long off = __builtin_offsetof(struct S, b[1]);
