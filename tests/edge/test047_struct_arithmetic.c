// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_ARITHMETIC]
// A struct is no operand of `+`.

struct S { int a; };

int main()
{
    struct S s = { 1 };

    return s + 1;
}
