// (Test) Compiler error: [ERR_SEM_OPASSIGN_NOT_ASSIGNABLE]
// Can't assign with `op=` to a sum.

int f(int n)
{
    n + 1 += 2;
    return n;
}
