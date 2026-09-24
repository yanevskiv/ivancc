// (Test) Return: 200
// An array parameter is really a pointer, and C lets its brackets carry `static`
// and qualifiers to say so more loudly: `int a[static 4]` promises the caller
// passes at least four elements. Both decay along with the array, so neither
// changes what the function does, and both are legal only on a parameter's
// outermost array.

int sum(int a[static 4], int n)
{
    int total;
    int i;

    total = 0;
    for (i = 0; i < n; i++) {
        total = total + a[i];
    }
    return total;
}

// A qualifier alone, with no length, which is the `int a[const]` form.
int length(const char s[const])
{
    int n;

    n = 0;
    while (s[n]) {
        n = n + 1;
    }
    return n;
}

// Both at once, in either order, and a length beside them.
int head(int a[const static 2])
{
    return a[0];
}

int tail(int a[static const 2])
{
    return a[1];
}

// The parameter is a pointer however its brackets were written, so it accepts one.
int through(int *p)
{
    return p[0];
}

int main()
{
    int v[4];
    int *p;

    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    v[3] = 4;

    if (sum(v, 4) != 10) return 1;
    if (length("abcde") != 5) return 2;
    if (head(v) != 1) return 3;
    if (tail(v) != 2) return 4;

    // The decayed parameter is an ordinary pointer, so a pointer goes in too.
    p = v;
    if (sum(p, 4) != 10) return 5;
    if (through(v) != 1) return 6;

    // sizeof sees the pointer the array decayed to, not the length in the brackets.
    if (sizeof(v) != 16) return 7;

    if (sum(v, 4) * 4 + head(v) + tail(v) - 1 != 42) return 8;
    return 200;
}
