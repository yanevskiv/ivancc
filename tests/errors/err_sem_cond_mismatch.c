// (Test) Compiler error: [ERR_SEM_COND_MISMATCH]
// A conditional choosing between a struct and an `int`.

struct S { int a; } s;

int f(int n)
{
    return (n ? s : 1).a;
}

int main(void)
{
    return 0;
}
