// (Test) Status: 200
// sizeof has type unsigned long and pointer subtraction type long, in folded
// constants, static initializers, run-time sizes and mixed arithmetic.

struct Pair { char c; long l; };

enum Sizes {
    S_WRAPS    = sizeof(int) - 5 > 0,
    S_NEG_BIG  = -sizeof(char) > 1000,
    S_WIDTH    = sizeof(sizeof(char)),
    S_MIXED    = sizeof(int) + -8 > 0
};

unsigned long g_wrap = sizeof(short) - 3;
int g_cmp = sizeof(long) < -1;

int vla(int n)
{
    int a[n];
    long b[n][2];

    if (sizeof(sizeof a) != 8) return 1;
    if (sizeof(a) - 100 < 100) return 2;
    if ((long *) b[n - 1] - (long *) b[0] != 2 * (n - 1)) return 3;
    if (&b[0] - &b[n - 1] != 1 - n) return 4;
    return 0;
}

int main()
{
    struct Pair ps[5];
    struct Pair *first = &ps[0];
    struct Pair *last = &ps[4];
    long l = -1;
    int i = -1;

    if (S_WRAPS != 1 || S_NEG_BIG != 1 || S_WIDTH != 8 || S_MIXED != 1) return 1;
    if (g_wrap != 18446744073709551615ul || g_cmp != 1) return 2;
    if (last - first != 4 || first - last != -4) return 3;
    if (sizeof(last - first) != sizeof(long)) return 4;
    if (i < sizeof(int)) return 5;
    if (l < sizeof(int)) return 6;
    if (sizeof(ps) / sizeof(ps[0]) != 5) return 7;
    if (sizeof(int) * -1 != 18446744073709551612ul) return 8;
    if ((first - last) * 1000000000L != -4000000000L) return 9;
    if (vla(3) != 0) return 10 + vla(3);
    return 200;
}
