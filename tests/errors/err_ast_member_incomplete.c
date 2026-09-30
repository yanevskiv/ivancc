// (Test) Compiler error: [ERR_AST_MEMBER_INCOMPLETE]
// A member of a struct declared but never defined.

struct T;
struct S { struct T t; };

int main(void)
{
    return 0;
}
