// (Test) Compiler error: [ERR_PAR_OFFSETOF_BITFIELD]
// Can't take the offset of a bit-field.

struct S { int a; int b : 3; };

unsigned long off = __builtin_offsetof(struct S, b);
