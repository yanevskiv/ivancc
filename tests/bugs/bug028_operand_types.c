// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_SCALAR]
// bug028. Operators took operands of any type, so a struct tested as a
// condition compiled and read its first bytes.

struct S { int a; };

int main()
{
    struct S s = { 1 };

    if (s) return 1;
    return 0;
}
