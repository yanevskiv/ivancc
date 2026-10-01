// (Test) Compiler error: [ERR_PAR_OFFSETOF_THROUGH_POINTER]
// Can't take the offset of an element a pointer points to.

struct S { int a; int *p; };

unsigned long off = __builtin_offsetof(struct S, p[1]);
