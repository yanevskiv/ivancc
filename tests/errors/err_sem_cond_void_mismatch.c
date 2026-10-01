// (Test) Compiler error: [ERR_SEM_COND_VOID_MISMATCH]
// Can't choose between `void` and an `int` in a conditional.
// Note: gcc accepts it, and refuses it only with -pedantic-errors.

int f(int n)
{
    return (n ? (void) 0 : 1, 0);
}
