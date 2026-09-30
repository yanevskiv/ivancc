// (Test) Compiler error: [ERR_PAR_TAG_REDEFINED]
// Can't define one struct tag twice in a scope.

struct S { int a; };
struct S { int b; };
