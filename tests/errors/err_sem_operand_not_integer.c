// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_INTEGER]
// A `%` applied to a pointer.

int main(void)
{
    int *p = 0;

    return (int) (p % 2);
}
