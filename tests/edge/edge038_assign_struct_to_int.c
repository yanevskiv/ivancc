// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_COMPATIBLE]
// A struct cannot initialize an integer.

struct S { int a; };

int main()
{
    struct S s = { 1 };
    int v = s;

    return v;
}
