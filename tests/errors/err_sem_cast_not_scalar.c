// (Test) Compiler error: [ERR_SEM_CAST_NOT_SCALAR]
// Can't cast an `int` to a struct.

struct S { int a; };

int f(int n)
{
    return ((struct S) n).a;
}
