// (Test) Return: 200
// Ten arguments, so the last four sit in the overflow area above the return
// address rather than in the register save area.

int weigh(int n, ...)
{
    __builtin_va_list ap;
    int i;
    int total;

    total = 0;
    __builtin_va_start(ap, n);
    for (i = 0; i < n; i = i + 1) {
        total = total + __builtin_va_arg(ap, int) * (i + 1);
    }
    __builtin_va_end(ap);
    return total;
}

// Two named parameters, so the anonymous ones start two registers further on.
int pick(int k, int n, ...)
{
    __builtin_va_list ap;
    int v;
    int i;

    v = 0;
    __builtin_va_start(ap, n);
    for (i = 0; i <= k; i = i + 1) {
        v = __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return v;
}

int main()
{
    if (pick(0, 5, 11, 22, 33, 44, 55) != 11) return 1;
    if (pick(3, 5, 11, 22, 33, 44, 55) != 44) return 2;
    if (pick(4, 5, 11, 22, 33, 44, 55) != 55) return 3;

    if (weigh(9, 9, 8, 7, 6, 5, 4, 3, 2, 1) != 165) return 4;
    return 200;
}
