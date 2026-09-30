// (Test) Compiler error: [ERR_SEM_MEMBER_UNKNOWN]
// Can't access a member the struct does not have.

struct S { int a; } s;

int main(void)
{
    return s.b;
}
