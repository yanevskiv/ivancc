// (Test) Compiler error: [ERR_PAR_BITFIELD_NAMED_ZERO]
// Can't have a named bit-field of zero width.

struct S { int a : 0; };
