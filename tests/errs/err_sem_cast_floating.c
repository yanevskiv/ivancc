// (Test) Compiler error: [ERR_SEM_CAST_FLOATING]
// Can't cast a `double` to a pointer.

int f(double d)
{
    int *p = (int *) d;

    return *p;
}
