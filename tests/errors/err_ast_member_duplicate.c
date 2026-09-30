// (Test) Compiler error: [ERR_AST_MEMBER_DUPLICATE]
// Two members of one struct with the same name.

struct S { int a; int a; };

int main(void)
{
    return 0;
}
