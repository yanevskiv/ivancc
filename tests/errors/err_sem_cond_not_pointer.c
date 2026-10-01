// (Test) Compiler error: [ERR_SEM_COND_NOT_POINTER]
// Can't choose between a pointer and an `int` in a conditional.

int *f(int n, int *p)
{
    return n ? p : 1;
}
