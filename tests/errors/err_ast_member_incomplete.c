// (Test) Compiler error: [ERR_AST_MEMBER_INCOMPLETE]
// Can't have a member of an incomplete type.

struct T;
struct S { struct T t; };
