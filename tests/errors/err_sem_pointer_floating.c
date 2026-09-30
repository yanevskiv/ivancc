// (Test) Compiler error: [ERR_SEM_POINTER_FLOATING]
// A `double` added to a pointer.

int f(double d)
{
    int *p = 0;

    return *(p + d);
}

int main(void)
{
    return 0;
}
