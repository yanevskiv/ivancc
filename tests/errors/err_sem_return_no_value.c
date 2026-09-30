// (Test) Compiler error: [ERR_SEM_RETURN_NO_VALUE]
// An `int` function returning nothing.

int f(void)
{
    return;
}

int main(void)
{
    return 0;
}
