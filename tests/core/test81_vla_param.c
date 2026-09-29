// (Test) Status: 200
// Variable-length array parameters. A parameter's lengths may name the ones
// before it, and are evaluated once on entry, the outermost one included,
// though it decays away. `[*]` stands for a length in a prototype. A goto or
// switch may leave the scope of a variably modified name or jump back to
// before it, but never into it.

static int calls;

int count(int n)
{
    calls++;
    return n;
}

// Each row's sum, weighted by its row number.
long weigh(int n, int m, int a[n][m])
{
    long s = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            s += a[i][j] * (i + 1);
        }
    }
    return s;
}

// The size of a row, fixed on entry however n changes after.
long row(int n, int a[count(2)][n])
{
    n = 100;
    return sizeof *a + sizeof a[0][0];
}

// Fill rows through a pointer to one, stepping it row by row.
void fill(int n, int (*p)[n], int rows)
{
    for (int i = 0; i < rows; i++, p++) {
        for (int j = 0; j < n; j++) {
            (*p)[j] = i * 10 + j;
        }
    }
}

long cube(int x, int y, int z, short c[x][y][z])
{
    c[x - 1][y - 1][z - 1] = 7;
    return sizeof c[0] * 100 + sizeof c[0][0];
}

// An old-style definition types n before the array that names it.
int knr(n, a)
    int n;
    int a[][n];
{
    return a[1][n - 1] + (int) sizeof a[0];
}

int anon(int n, int [n][n]);
int star(int, int [*][*]);
int apply(int n, int (*f)(int k, int b[k][k]), int b[n][n]);

int anon(int n, int b[n][n])
{
    return b[n - 1][n - 1];
}

int star(int n, int b[n][n])
{
    return (int) sizeof b[0];
}

int apply(int n, int (*f)(int k, int b[k][k]), int b[n][n])
{
    return f(n, b);
}

int last(int n, const int a[static n])
{
    return a[n - 1];
}

// Count passes back through a declaration, which gets fresh storage each time.
int again(int n)
{
    int passes = 0;
    char *first = 0;

top:
    ;
    char a[n];
    if (! first) {
        first = a;
    }
    if (a != first) {
        return -1;
    }
    if (++passes < 1000) {
        goto top;
    }
    return passes;
}

// Leave a variable-length array's block by goto and by a switch case.
int leave(int n)
{
    int r = 0;

    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0: {
                int a[n];
                a[n - 1] = 5;
                r += a[n - 1];
                goto next;
            }
            case 1: {
                int (*p)[n] = 0;
                r += (int) sizeof *p;
            } break;
            default:
                r += 100;
        }
    next:
        ;
    }
    return r;
}

int main(void)
{
    int a[3][4];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            a[i][j] = i + j;
        }
    }

    // Lengths name earlier parameters, and index the array they shape.
    if (weigh(3, 4, a) != 68) return 1;
    if (weigh(2, 6, (int (*)[6]) a) != 51) return 2;

    // The outer length is evaluated though it decays away.
    if (row(4, a) != 20 || calls != 1) return 3;

    // A pointer to a variable-length array steps by the size it was given.
    int n = 5;
    int b[4][n];
    fill(n, b, 4);
    if (b[3][4] != 34 || b[2][1] != 21 || b[0][0] != 0) return 4;
    fill(2, (int (*)[2]) &b[0][0], 10);
    if (b[3][4] != 91 || b[0][3] != 11) return 5;

    short c[2][3][5];
    if (cube(2, 3, 5, c) != 3010 || c[1][2][4] != 7) return 6;
    if (knr(4, a) != 20) return 7;

    // Unnamed parameters, `[*]`, and a pointer to a function taking one.
    if (anon(3, (int (*)[3]) a) != 2) return 8;
    if (star(6, 0) != 24) return 9;
    if (apply(3, anon, (int (*)[3]) a) != 2) return 10;
    if (last(4, a[2]) != 5) return 11;

    // Jumps back before, or out of, a variably modified name's scope.
    if (again(64) != 1000) return 12;
    if (leave(3) != 117) return 13;
    return 200;
}
