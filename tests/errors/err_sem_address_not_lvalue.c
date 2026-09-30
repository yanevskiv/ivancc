// (Test) Compiler error: [ERR_SEM_ADDRESS_NOT_LVALUE]
// Can't take the address of a sum.

int f(int n)
{
    int *p = &(n + 1);

    return *p;
}
