// (Test) Compiler error: [ERR_PAR_BITFIELD_NOT_INTEGER]
// Can't have a bit-field of floating type.

struct S { double a : 3; };
