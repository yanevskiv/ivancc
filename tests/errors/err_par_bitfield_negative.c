// (Test) Compiler error: [ERR_PAR_BITFIELD_NEGATIVE]
// Can't have a bit-field of negative width.

struct S { int a : -1; };
