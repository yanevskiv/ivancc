// (Test) Compiler error: [ERR_SEM_ADDRESS_NOT_LVALUE]
// The address of a sum.

int f(int n)
{
    int *p = &(n + 1);

    return *p;
}

int main(void)
{
    return 0;
}
