// (Test) Compiler error: [ERR_PAR_BITFIELD_TOO_WIDE]
// Can't have a bit-field wider than its type.

struct S { int a : 33; };
