// (Test) Compiler error: [ERR_SEM_ARRAY_LEN_NOT_INTEGER]
// A variable-length array whose length is a `double`.

int f(double d)
{
    int a[d];

    return a[0];
}

int main(void)
{
    return 0;
}
