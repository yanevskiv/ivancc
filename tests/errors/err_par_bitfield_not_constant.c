// (Test) Compiler error: [ERR_PAR_BITFIELD_NOT_CONSTANT]
// Can't have a bit-field width known only at run time.

int n;
struct S { int a : n; };
