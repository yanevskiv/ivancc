// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_ARITHMETIC]
// Can't negate a struct.

struct S { int a; } s;

int main(void)
{
    return -s;
}
