// (Test) Compiler error: [ERR_SEM_POINTER_FLOATING]
// Can't add a `double` to a pointer.

int f(double d)
{
    int *p = 0;

    return *(p + d);
}
