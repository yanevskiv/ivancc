// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_ARITHMETIC]
// `++` takes a number or a pointer, not a struct.

struct S { int a; };

int main()
{
    struct S s = { 1 };

    s++;
    return s.a;
}
