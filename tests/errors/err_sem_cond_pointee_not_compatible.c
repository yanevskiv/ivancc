// (Test) Compiler error: [ERR_SEM_COND_POINTEE_NOT_COMPATIBLE]
// Can't choose between an `int *` and a `long *` in a conditional.

void *f(int n, int *p, long *q)
{
    return n ? p : q;
}
