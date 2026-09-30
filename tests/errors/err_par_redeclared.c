// (Test) Compiler error: [ERR_PAR_REDECLARED]
// Can't declare one local twice in a block.

int f(void)
{
    int x;
    int x;
    return 0;
}
