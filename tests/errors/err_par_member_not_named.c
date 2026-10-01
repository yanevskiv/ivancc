// (Test) Compiler error: [ERR_PAR_MEMBER_NOT_NAMED]
// Can't have a member whose declarator names nothing.

struct S { int (*)(void); int b; };
