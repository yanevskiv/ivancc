// (Test) Return: 55
// The anonymous arguments of a variadic function, reached by index through
// the register save area. Every argument here still fits in a register.

int sum(int n, ...)
{
    int i;
    int total;

    total = 0;
    for (i = 0; i < n; i = i + 1) {
        total = total + __builtin_va_arg(i);
    }
    return total;
}

int main()
{
    if (sum(0) != 0) return 1;
    if (sum(1, 7) != 7) return 2;
    if (sum(3, 1, 2, 3) != 6) return 3;

    return sum(5, 1, 2, 3, 4, 45);
}
