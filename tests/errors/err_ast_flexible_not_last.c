// (Test) Compiler error: [ERR_AST_FLEXIBLE_NOT_LAST]
// A flexible array member followed by another member.

struct S { int n; int d[]; int m; };

int main(void)
{
    return 0;
}
