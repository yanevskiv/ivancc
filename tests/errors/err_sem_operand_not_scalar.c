// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_SCALAR]
// Can't test a struct for truth.

struct S { int a; } s;

int main(void)
{
    return !s;
}
