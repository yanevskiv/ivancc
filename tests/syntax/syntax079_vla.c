// (Test) Status: 200
// Variable-length arrays. A block-scope array whose length is not a constant
// takes that length when its declaration runs, and lives below the frame
// until its scope ends. Every way out of the scope gives the stack back:
// falling off the end, break, continue, goto and return. The length is
// evaluated once, and the array stays 16-byte aligned so calls still are.

struct P {
    int    x;
    double y;
};

static int calls;

int len(int n)
{
    calls++;
    return n;
}

// The sum of a[0] to a[n - 1] through a plain pointer.
long sum(int *a, int n)
{
    long s = 0;
    for (int i = 0; i < n; i++) {
        s += a[i];
    }
    return s;
}

// Whether a 16-byte local in this frame sits on a 16-byte boundary.
int aligned(void)
{
    long double x = 1.5L;
    return (long) &x % 16 == 0 && x == 1.5L;
}

// A recursion where every level keeps its own array.
int depth(int n)
{
    int a[n + 1];
    for (int i = 0; i <= n; i++) {
        a[i] = n;
    }
    int below = n > 0 ? depth(n - 1) : 0;
    for (int i = 0; i <= n; i++) {
        if (a[i] != n) {
            return -1000;
        }
    }
    return below + n;
}

// The variadic arguments survive an array allocated before they are read.
int vsum(int n, ...)
{
    __builtin_va_list ap;
    int a[n];
    int s = 0;

    __builtin_va_start(ap, n);
    for (int i = 0; i < n; i++) {
        a[i] = __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    for (int i = 0; i < n; i++) {
        s += a[i];
    }
    return s;
}

// Every kind of exit from an array's scope leaves %rsp where it found it.
int exits(int n)
{
    int one = 1;
    char *before;
    char *after;
    char *seen[4];

    { char m[one]; before = m; }

    for (int i = 0; i < 100; i++) {
        int a[n];
        a[0] = i;
        if (i == 50) {
            break;
        }
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        int a[n];
        char *here = (char *) a;
        if (i == 0) {
            seen[0] = here;
        }
        if (here != seen[0]) {
            return 2;
        }
        if (i % 2) {
            continue;
        }
        a[1] = i;
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 3;
    }

    int k = 0;
    do {
        double d[n + k];
        d[k] = k;
        if (k++ < 20) {
            continue;
        }
    } while (k < 40);
    { char m[one]; after = m; }
    if (after != before) {
        return 4;
    }

    k = 0;
again:
    {
        int a[n];
        a[n - 1] = k;
        if (++k < 30) {
            goto again;
        }
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 5;
    }

    {
        int a[n];
        {
            char b[n * 3];
            b[0] = a[0] = 1;
            goto out;
        }
    }
out:
    { char m[one]; after = m; }
    if (after != before) {
        return 6;
    }

    {
        k = 0;
    top:;
        int a[n];
        seen[k] = (char *) a;
        if (++k < 3) {
            goto top;
        }
        if (seen[0] != seen[1] || seen[1] != seen[2]) {
            return 7;
        }
    }

    for (k = 0; k < 3; k++) {
        switch (k) {
            case 1: {
                int a[n];
                a[0] = 1;
                break;
            }
            default: {
                break;
            }
        }
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 8;
    }

    for (int i = 0, a[n]; i < n; i++) {
        a[i] = i;
        if (i == 2) {
            break;
        }
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 9;
    }

    if (n > 0) {
        int a[n];
        a[0] = 0;
    } else {
        int b[n + 1];
        b[0] = 0;
    }
    { char m[one]; after = m; }
    if (after != before) {
        return 10;
    }
    return 0;
}

int main(void)
{
    int n = 10;
    int a[n];

    // The elements are ordinary lvalues, and the array decays to a pointer.
    for (int i = 0; i < n; i++) {
        a[i] = i * i;
    }
    if (a[3] != 9 || a[9] != 81) return 1;
    if (sum(a, n) != 285) return 2;
    if (*(a + 4) != 16 || &a[n - 1] - &a[0] != 9) return 3;
    int *p = a + 5;
    if (p[-1] != 16 || (*&a)[2] != 4) return 4;

    // The length is evaluated once, when the declaration runs.
    int k = 3;
    char c[k++];
    if (k != 4) return 5;
    c[0] = 'x';
    c[2] = 'z';
    k = 100;
    if (c[0] != 'x' || c[2] != 'z') return 6;
    long d[len(4)];
    if (calls != 1) return 7;
    d[3] = 1L << 40;
    if (d[3] >> 40 != 1) return 8;

    // Any integer type gives the length, a const one included.
    unsigned char uc = 3;
    short s[uc];
    s[2] = -7;
    const int m = 5;
    long double ld[m];
    for (int i = 0; i < m; i++) {
        ld[i] = i / 2.0L;
    }
    if (s[2] != -7 || ld[3] != 1.5L) return 9;

    // The elements may be structs, or arrays of a constant length.
    struct P ps[n];
    for (int i = 0; i < n; i++) {
        ps[i].x = i;
        ps[i].y = i * 0.5;
    }
    if (ps[7].x != 7 || ps[7].y != 3.5) return 10;
    int grid[n][4];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            grid[i][j] = i * 4 + j;
        }
    }
    if (grid[9][3] != 39 || grid[2][1] != 9) return 11;

    // Arrays that come and go below leave the earlier ones intact.
    for (int r = 1; r < 20; r++) {
        char tmp[r * 7];
        for (int i = 0; i < r * 7; i++) {
            tmp[i] = (char) i;
        }
        if (tmp[r * 7 - 1] != (char) (r * 7 - 1)) return 12;
    }
    if (a[9] != 81 || c[2] != 'z' || ps[9].x != 9 || grid[5][0] != 20) return 13;

    // A call from inside the scope of an odd-sized array sees an aligned stack.
    for (int r = 1; r < 4; r++) {
        char odd[r];
        odd[0] = 0;
        if (! aligned()) return 14;
        {
            char tiny[r + 2];
            tiny[0] = 0;
            if (! aligned()) return 15;
        }
    }

    // Recursion, variadic functions, and every way out of a scope.
    if (depth(6) != 21) return 16;
    if (vsum(4, 10, 20, 30, 40) != 100) return 17;
    int why = exits(n);
    if (why) return 20 + why;
    return 200;
}
