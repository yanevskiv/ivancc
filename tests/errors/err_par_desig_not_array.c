// (Test) Compiler error: [ERR_PAR_DESIG_NOT_ARRAY]
// Can't designate an index in a struct's initializer.

struct S { int a; } s = { [0] = 1 };
