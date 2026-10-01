// (Test) Compiler error: [ERR_AST_MEMBER_NOT_COMPLETE]
// Can't have a member of an incomplete type.

struct T;
struct S { struct T t; };
