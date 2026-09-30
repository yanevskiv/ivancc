// (Test) Compiler error: [ERR_SEM_MEMBER_UNKNOWN]
// A member the struct lacks.

struct S { int a; } s;

int main(void)
{
    return s.b;
}
