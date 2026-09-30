// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// A member of a const struct is read-only, reached with `.` or `->`.

struct S { int a; };

int main()
{
    const struct S s = { 1 };
    const struct S *p = &s;

    p->a = 2;
    return s.a;
}
