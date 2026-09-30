// (Test) Status: 200
// __builtin_va_copy copies every field of the cursor: part-way through the
// arguments, from a va_list parameter, and across integer, floating and stack
// arguments, so the copy and the original walk the same values.

typedef __builtin_va_list va_list;

double walk(int n, va_list ap)
{
    va_list mine;
    double total = 0;

    __builtin_va_copy(mine, ap);
    for (int i = 0; i < n; i++) {
        total += i % 2 ? __builtin_va_arg(mine, double) : __builtin_va_arg(mine, long);
    }
    __builtin_va_end(mine);
    return total;
}

double twice(int n, ...)
{
    va_list ap;
    double first;
    double second;

    __builtin_va_start(ap, n);
    first = walk(n, ap);
    second = walk(n, ap);
    __builtin_va_end(ap);
    return first == second ? first : -1;
}

long rest(int n, ...)
{
    va_list ap;
    va_list tail;
    long head;
    long sum = 0;

    __builtin_va_start(ap, n);
    head = __builtin_va_arg(ap, long);
    __builtin_va_copy(tail, ap);
    for (int i = 1; i < n; i++) sum += __builtin_va_arg(ap, long);
    for (int i = 1; i < n; i++) sum -= __builtin_va_arg(tail, long);
    __builtin_va_end(tail);
    __builtin_va_end(ap);
    return sum == 0 ? head : -1;
}

int main()
{
    if (twice(4, 1L, 0.5, 2L, 0.25) != 3.75) return 1;
    if (twice(20, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5, 1L, 0.5) != 15) return 2;
    if (rest(9, 7L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L) != 7) return 3;
    return 200;
}
