// (Test) Compiler error: [ERR_PAR_VA_ARG_TYPE]
// Can't take a `void` from `__builtin_va_arg`.

int f(int n, ...)
{
    __builtin_va_list ap;

    __builtin_va_start(ap, n);
    __builtin_va_arg(ap, void);
    return 0;
}
