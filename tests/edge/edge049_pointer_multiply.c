// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_ARITHMETIC]
// A pointer cannot be multiplied, directly or by compound assignment.

int x;

int main()
{
    int *p = &x;

    p *= 2;
    return 0;
}
