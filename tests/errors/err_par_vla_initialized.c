// (Test) Compiler error: [ERR_PAR_VLA_INITIALIZED]
// Can't initialize a variable-length array.

int f(int n)
{
    int a[n] = { 0 };
    return a[0];
}
