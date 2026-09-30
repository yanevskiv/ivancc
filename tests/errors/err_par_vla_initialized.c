// (Test) Compiler error: [ERR_PAR_VLA_INITIALIZED]
// A variable-length array with an initializer.

int f(int n)
{
    int a[n] = { 0 };
    return a[0];
}

int main(void)
{
    return 0;
}
