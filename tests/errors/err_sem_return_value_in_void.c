// (Test) Compiler error: [ERR_SEM_RETURN_VALUE_IN_VOID]
// Can't return a value from a `void` function.

void f(void)
{
    return 1;
}
