// (Test) Compiler error: [ERR_PAR_MEMBER_UNNAMED]
// Can't have a member whose declarator names nothing.

struct S { int (*)(void); int b; };
