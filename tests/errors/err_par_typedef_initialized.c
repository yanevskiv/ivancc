// (Test) Compiler error: [ERR_PAR_TYPEDEF_INITIALIZED]
// A block-scope typedef with an initializer.

int f(void)
{
    typedef int T = 1;
    return 0;
}

int main(void)
{
    return 0;
}
