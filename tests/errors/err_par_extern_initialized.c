// (Test) Compiler error: [ERR_PAR_EXTERN_INITIALIZED]
// A block-scope `extern` with an initializer.

int f(void)
{
    extern int x = 1;
    return x;
}

int main(void)
{
    return 0;
}
