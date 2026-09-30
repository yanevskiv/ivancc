// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_SCALAR]
// A struct tested for truth by `!`.

struct S { int a; } s;

int main(void)
{
    return !s;
}
