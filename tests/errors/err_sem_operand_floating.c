// (Test) Compiler error: [ERR_SEM_OPERAND_FLOATING]
// A `%` applied to a `double`.

int f(double d)
{
    return (int) (d % 2);
}

int main(void)
{
    return 0;
}
