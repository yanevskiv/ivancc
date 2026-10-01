// (Test) Compiler error: [ERR_SEM_INCDEC_NOT_ASSIGNABLE]
// Can't increment a sum.

int f(int n)
{
    (n + 1)++;
    return n;
}
