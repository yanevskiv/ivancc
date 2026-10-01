// (Test) Compiler error: [ERR_PAR_FUNCTION_CONFLICTING_TYPES]
// bug019. A function declared again with another type compiled, as a header's
// prototype that disagrees with the definition does.

int f(void);

long f(void)
{
    return 0;
}

int main()
{
    return 0;
}
