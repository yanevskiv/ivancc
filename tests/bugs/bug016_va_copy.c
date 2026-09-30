// (Test) Status: 200
// bug016. There was no __builtin_va_copy, so a variadic function could not walk
// its arguments twice, as vprintf does to measure and then to write.

typedef __builtin_va_list va_list;

int sum_twice(int n, ...)
{
    va_list ap;
    va_list again;
    int first = 0;
    int second = 0;

    __builtin_va_start(ap, n);
    __builtin_va_copy(again, ap);
    for (int i = 0; i < n; i++) first += __builtin_va_arg(ap, int);
    for (int i = 0; i < n; i++) second += __builtin_va_arg(again, int);
    __builtin_va_end(again);
    __builtin_va_end(ap);
    return first == second ? first : -1;
}

int main()
{
    if (sum_twice(3, 1, 2, 3) != 6) return 1;
    if (sum_twice(8, 1, 2, 3, 4, 5, 6, 7, 8) != 36) return 2;
    return 200;
}
