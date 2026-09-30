// (Test) Compiler error: [ERR_PAR_DESIG_NO_MEMBER]
// A designator naming a member the struct lacks.

struct S { int a; } s = { .b = 1 };

int main(void)
{
    return 0;
}
