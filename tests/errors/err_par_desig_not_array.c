// (Test) Compiler error: [ERR_PAR_DESIG_NOT_ARRAY]
// An index designator in the initializer of a struct.

struct S { int a; } s = { [0] = 1 };

int main(void)
{
    return 0;
}
