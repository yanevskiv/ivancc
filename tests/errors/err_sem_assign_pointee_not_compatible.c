// (Test) Compiler error: [ERR_SEM_ASSIGN_POINTEE_NOT_COMPATIBLE]
// Can't assign an `int *` to a `long *`.

long *f(int *p)
{
    long *q;
    q = p;
    return q;
}
