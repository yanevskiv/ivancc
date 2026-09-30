// (Test) Compiler error: [ERR_SEM_DEREF_NOT_POINTER]
// A `*` applied to an `int`.

int f(int n)
{
    return *n;
}

int main(void)
{
    return 0;
}
