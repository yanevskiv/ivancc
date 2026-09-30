// (Test) Compiler error: [ERR_SEM_NOT_ASSIGNABLE]
// An assignment to a sum.

int f(int n)
{
    n + 1 = 2;
    return n;
}

int main(void)
{
    return 0;
}
