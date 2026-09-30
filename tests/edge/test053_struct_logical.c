// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_SCALAR]
// A struct cannot be tested for truth by `&&`, `!`, `?:` or a loop.

struct S { int a; };

int main()
{
    struct S s = { 1 };

    while (s && 1) return 1;
    return 0;
}
