// (Test) Return: 200
// The anonymous arguments of a variadic function, walked with a va_list that
// starts in the register save area. Every argument here still fits in a register.

int sum(int n, ...)
{
    __builtin_va_list ap;
    int i;
    int total;

    total = 0;
    __builtin_va_start(ap, n);
    for (i = 0; i < n; i = i + 1) {
        total = total + __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return total;
}

int main()
{
    if (sum(0) != 0) return 1;
    if (sum(1, 7) != 7) return 2;
    if (sum(3, 1, 2, 3) != 6) return 3;

    if (sum(5, 1, 2, 3, 4, 45) != 55) return 4;
    return 200;
}
