// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_ARITHMETIC]
// A string literal decays to a pointer, which unary minus cannot negate.

int main()
{
    return -"abc" != 0;
}
