// (Test) Compiler error: [ERR_PAR_REDECLARED]
// A local declared twice in one block.

int f(void)
{
    int x;
    int x;
    return 0;
}

int main(void)
{
    return 0;
}
