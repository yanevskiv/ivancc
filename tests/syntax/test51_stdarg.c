// (Test) Return: 200
// A real va_list: the four-field record the SysV ABI defines, holding how far
// into the register save area a walk has come and where the overflow area
// carries on. The walk lives in that object, so it can be handed to a callee.

int total(int n, __builtin_va_list ap)
{
    int t;
    int i;

    t = 0;
    for (i = 0; i < n; i = i + 1) {
        t = t + __builtin_va_arg(ap, int);
    }
    return t;
}

int forward(int n, ...)
{
    __builtin_va_list ap;
    int t;

    __builtin_va_start(ap, n);
    t = total(n, ap);
    __builtin_va_end(ap);
    return t;
}

// Two lists over the same arguments, each stepping on its own.
int twice(int n, ...)
{
    __builtin_va_list a;
    __builtin_va_list b;
    int t;
    int i;

    __builtin_va_start(a, n);
    __builtin_va_start(b, n);
    t = 0;
    for (i = 0; i < n; i = i + 1) {
        t = t + __builtin_va_arg(a, int) - __builtin_va_arg(b, int);
    }
    __builtin_va_end(a);
    __builtin_va_end(b);
    return t;
}

char *last(int n, ...)
{
    __builtin_va_list ap;
    char *s;
    int i;

    s = 0;
    __builtin_va_start(ap, n);
    for (i = 0; i < n; i = i + 1) {
        s = __builtin_va_arg(ap, char *);
    }
    __builtin_va_end(ap);
    return s;
}

// Five named parameters, so one anonymous argument still reaches a register
// and the rest arrive above the return address.
int after(int a, int b, int c, int d, int e, ...)
{
    __builtin_va_list ap;
    int t;
    int i;

    t = a + b + c + d + e;
    __builtin_va_start(ap, e);
    for (i = 0; i < 4; i = i + 1) {
        t = t + __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return t;
}

int main()
{
    if (forward(3, 1, 2, 3) != 6) return 1;
    if (forward(0) != 0) return 2;
    if (forward(8, 1, 2, 3, 4, 5, 6, 7, 8) != 36) return 3;
    if (twice(5, 9, 8, 7, 6, 5) != 0) return 4;

    char *s = last(3, "a", "bb", "ccc");
    if (s[0] != 'c' || s[3] != 0) return 5;

    if (after(1, 2, 3, 4, 5, 10, 20, 30, 40) != 115) return 6;

    if (forward(4, 10, 11, 12, 9) != 42) return 7;
    return 200;
}
