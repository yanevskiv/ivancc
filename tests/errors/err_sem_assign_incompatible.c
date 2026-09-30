// (Test) Compiler error: [ERR_SEM_ASSIGN_INCOMPATIBLE]
// An `int` initialized by a struct.

struct S { int a; } s;

int main(void)
{
    int x = s;

    return x;
}
