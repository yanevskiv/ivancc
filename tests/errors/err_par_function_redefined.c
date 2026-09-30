// (Test) Compiler error: [ERR_PAR_FUNCTION_REDEFINED]
// A function given two bodies.

int f(void)
{
    return 0;
}

int f(void)
{
    return 1;
}

int main(void)
{
    return 0;
}
