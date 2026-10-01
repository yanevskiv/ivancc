// (Test) Compiler error: [ERR_PAR_LOCAL_CONFLICTING_TYPES]
// Can't declare one block-scope `extern` with two different types.

void g(void)
{
    extern int x;
    extern long x;
}
