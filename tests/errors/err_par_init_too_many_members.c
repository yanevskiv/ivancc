// (Test) Compiler error: [ERR_PAR_INIT_TOO_MANY_MEMBERS]
// Can't give a struct more initializers than members.
// Note: gcc only warns.

struct S { int a; } s = { 1, 2 };
