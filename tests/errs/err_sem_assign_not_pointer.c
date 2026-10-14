// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_POINTER]
// Can't assign an `int` to a pointer.

int *f(int x)
{
    int *p;
    p = x;
    return p;
}
