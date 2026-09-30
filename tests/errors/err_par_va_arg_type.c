// (Test) Compiler error: [ERR_PAR_VA_ARG_TYPE]
// `__builtin_va_arg` asked for a `void`.

int f(int n, ...)
{
    __builtin_va_list ap;

    __builtin_va_start(ap, n);
    __builtin_va_arg(ap, void);
    return 0;
}

int main(void)
{
    return 0;
}
