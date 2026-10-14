// (Test) Compiler error: [ERR_SEM_ARRAY_LEN_NOT_INTEGER]
// Can't give an array a `double` length.

int f(double d)
{
    int a[d];

    return a[0];
}
