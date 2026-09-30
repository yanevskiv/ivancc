// (Test) Compiler error: [ERR_PAR_TYPEDEF_INITIALIZED]
// Can't initialize a typedef.

int f(void)
{
    typedef int T = 1;
    return 0;
}
