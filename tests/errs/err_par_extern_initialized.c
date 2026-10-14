// (Test) Compiler error: [ERR_PAR_EXTERN_INITIALIZED]
// Can't initialize a block-scope `extern`.

int f(void)
{
    extern int x = 1;
    return x;
}
