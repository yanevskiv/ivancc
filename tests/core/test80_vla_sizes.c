// (Test) Status: 200
// Run-time sizes and variably modified types. A variable-length array's size
// is fixed when its declarator runs and kept with its type, so `sizeof`, the
// step of a pointer to one, and the rows of a multidimensional one all read
// it later. A typedef keeps the size it had where it was declared, and a type
// name in a cast or `sizeof` sizes its arrays where it is evaluated.

struct P {
    int  x;
    char c;
};

static int calls;

int count(int n)
{
    calls++;
    return n;
}

// The sum of the rows of an n by m array, read through a pointer to a row.
long rows(int *base, int n, int m)
{
    int (*row)[m] = (int (*)[m]) base;
    long s = 0;

    for (int i = 0; i < n; i++, row++) {
        for (int j = 0; j < m; j++) {
            s += (*row)[j];
        }
    }
    return s;
}

// The size of an array whose length is set by the caller, one call deep.
long depth(int n)
{
    char a[n];
    long here = sizeof a;
    return n > 1 ? here + depth(n - 1) : here;
}

int main(void)
{
    int n = 5;
    int m = 3;

    // sizeof reads the size a declaration fixed, and a type name's own.
    int a[n];
    if (sizeof a != 20 || sizeof a / sizeof a[0] != 5) return 1;
    if (sizeof(int[n]) != 20 || sizeof(char[m]) != 3) return 2;
    struct P ps[n];
    if (sizeof ps != 5 * sizeof(struct P)) return 3;
    long double ld[m];
    if (sizeof ld != 48) return 4;

    // Nested arrays, the inner length or the outer one or both variable.
    int b[n][m];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            b[i][j] = i * 10 + j;
        }
    }
    if (b[4][2] != 42 || b[1][0] != 10) return 5;
    if (sizeof b != 60 || sizeof b[0] != 12 || sizeof b[0][0] != 4) return 6;
    int c[3][n];
    c[2][4] = 77;
    if (sizeof c != 60 || sizeof c[1] != 20 || *(*(c + 2) + 4) != 77) return 7;
    int d[n][2];
    d[4][1] = 9;
    if (sizeof d != 40 || sizeof d[0] != 8 || d[4][1] != 9) return 8;
    short e[2][n][m];
    e[1][4][2] = -3;
    if (sizeof e != 60 || sizeof e[1] != 30 || sizeof e[1][4] != 6) return 9;
    if (e[1][4][2] != -3 || (char *) &e[1][4][2] - (char *) e != 58) return 10;

    // A pointer to a variable-length array steps by its size.
    int (*p)[m] = b;
    if (p[2][1] != 21 || (*(p + 3))[2] != 32) return 11;
    p++;
    if ((*p)[0] != 10) return 12;
    p += 2;
    if ((*p)[1] != 31 || p - b != 3 || b - p != -3) return 13;
    --p;
    p -= 1;
    if (p[0][0] != 10) return 14;
    if ((char *) (p + 1) - (char *) p != 12 || sizeof *p != 12) return 15;
    if ((char *) (&a + 1) - (char *) &a != 20) return 16;
    if (rows(&b[0][0], n, m) != 315) return 17;

    // A typedef keeps the size it had where it was declared.
    typedef int Row[n];
    n = 100;
    Row r;
    Row *rp = &r;
    if (sizeof(Row) != 20 || sizeof r != 20 || sizeof *rp != 20) return 18;
    r[4] = 8;
    if ((*rp)[4] != 8) return 19;
    Row grid[2];
    if (sizeof grid != 40) return 20;
    n = 5;

    // Each length is evaluated once, where its declarator or type name runs.
    char f[count(4)][m + 1];
    if (sizeof f != 16 || calls != 1) return 21;
    if (sizeof(char[count(7)]) != 7 || calls != 2) return 22;
    if (sizeof(int (*)[count(9)]) != sizeof(void *) || calls != 2) return 23;
    typedef char Buf[count(6)];
    Buf x;
    Buf y;
    if (sizeof x + sizeof y != 12 || calls != 3) return 24;
    int (*q)[n] = (int (*)[count(n)]) &a[0];
    if (calls != 4 || sizeof *q != 20) return 25;

    // The operand of a sizeof is evaluated only when its size is variable.
    int i = 0;
    int s = sizeof b[i++];
    if (s != 12 || i != 1) return 26;
    s = sizeof a[i++];
    if (s != 4 || i != 1) return 27;
    s = sizeof *p++;
    if (s != 12 || p - b != 2) return 28;

    // Sizes follow the values each pass through the declaration sees.
    long total = 0;
    for (int k = 1; k <= 4; k++) {
        int v[k][k];
        v[k - 1][k - 1] = k;
        total += sizeof v + v[k - 1][k - 1];
    }
    if (total != (1 + 4 + 9 + 16) * 4 + 10) return 29;
    if (depth(4) != 10) return 30;

    // A static pointer to a variable-length array is sized as it is reached.
    static int (*sp)[m];
    sp = b;
    if (sizeof *sp != 12 || sp[1][2] != 12) return 31;
    return 200;
}
