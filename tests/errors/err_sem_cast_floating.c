// (Test) Compiler error: [ERR_SEM_CAST_FLOATING]
// A `double` cast to a pointer.

int f(double d)
{
    int *p = (int *) d;

    return *p;
}

int main(void)
{
    return 0;
}
