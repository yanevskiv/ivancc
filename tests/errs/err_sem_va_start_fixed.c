// (Test) Compiler error: [ERR_SEM_VA_START_FIXED]
// Can't use `__builtin_va_start` in a function that is not variadic.

int f(int n)
{
    __builtin_va_list ap;

    __builtin_va_start(ap, n);
    return 0;
}
