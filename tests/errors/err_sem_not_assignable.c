// (Test) Compiler error: [ERR_SEM_NOT_ASSIGNABLE]
// Can't assign to a sum.

int f(int n)
{
    n + 1 = 2;
    return n;
}
