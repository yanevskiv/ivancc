// (Test) Compiler error: [ERR_SEM_CAST_NOT_SCALAR]
// An `int` cast to a struct.

struct S { int a; };

int f(int n)
{
    return ((struct S) n).a;
}

int main(void)
{
    return 0;
}
