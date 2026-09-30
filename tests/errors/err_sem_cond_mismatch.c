// (Test) Compiler error: [ERR_SEM_COND_MISMATCH]
// Can't choose between a struct and an `int` in a conditional.

struct S { int a; } s;

int f(int n)
{
    return (n ? s : 1).a;
}
