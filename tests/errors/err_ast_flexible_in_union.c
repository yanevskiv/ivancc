// (Test) Compiler error: [ERR_AST_FLEXIBLE_IN_UNION]
// A flexible array member in a union.

union U { int n; int d[]; };

int main(void)
{
    return 0;
}
