// (Test) Compiler error: [ERR_SEM_VA_START_FIXED]
// `__builtin_va_start` in a function that takes no variable arguments.

int f(int n)
{
    __builtin_va_list ap;

    __builtin_va_start(ap, n);
    return 0;
}

int main(void)
{
    return 0;
}
