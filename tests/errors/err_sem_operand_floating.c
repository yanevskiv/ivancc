// (Test) Compiler error: [ERR_SEM_OPERAND_FLOATING]
// Can't apply `%` to a `double`.

int f(double d)
{
    return (int) (d % 2);
}
