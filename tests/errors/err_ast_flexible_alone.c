// (Test) Compiler error: [ERR_AST_FLEXIBLE_ALONE]
// A struct whose only member is a flexible array.

struct S { int d[]; };

int main(void)
{
    return 0;
}
