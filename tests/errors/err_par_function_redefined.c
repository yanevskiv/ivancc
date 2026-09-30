// (Test) Compiler error: [ERR_PAR_FUNCTION_REDEFINED]
// Can't give one function two bodies.

int f(void)
{
    return 0;
}

int f(void)
{
    return 1;
}
