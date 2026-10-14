// (Test) Compiler error: [ERR_SEM_CAST_NOT_SCALAR]
// A cast cannot name a struct type.

struct S { int a; };

int main()
{
    struct S s = (struct S) 1;

    return s.a;
}
