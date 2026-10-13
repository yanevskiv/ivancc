// (Test) Compiler error: [ERR_PAR_ALIGNOF_NOT_COMPLETE]
// Can't take the `_Alignof` of an incomplete type.

struct S;
unsigned long n = _Alignof(struct S);
