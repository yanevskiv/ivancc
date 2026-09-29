// (Test) Status: 200
// Floating types. A float and a double do their arithmetic in SSE registers,
// and a long double does its in the x87's 80-bit format. Where a floating
// operand meets an integer, the usual arithmetic conversions pick the floating
// type, and a conversion back to an integer truncates toward zero.

struct Point {
    char        tag;
    float       x;
    double      y;
    long double z;
    int         bits : 5;
};

float gf = 1.5f;
double gd = -2.25;
long double gl = 3.0L / 4;
double garr[3] = { 1, 2.5, 1e10 };
int gi = 7.9;
unsigned long gu = 1.8e19;
struct Point gp = { 'a', 2.5f, -1e-3, 7.0L, 3 };

int calls;

double *next(double *p)
{
    calls++;
    return p + 1;
}

int main()
{
    float f = 0.1f;
    double d = 0.1;
    long double l = 0.1L;
    int i = 3;
    unsigned long u = 18000000000000000000UL;
    double arr[4] = { 0.5, 1.5, 2.5, 3.5 };
    double *q = arr;
    struct Point p;
    long double la[3];
    long double t;
    unsigned char uc;
    signed char sc;
    unsigned int ui;

    if (sizeof(float) != 4) return 1;
    if (sizeof(double) != 8) return 2;
    if (sizeof(long double) != 16) return 3;
    if (sizeof(struct Point) != 48) return 4;

    // Each type rounds to its own precision.
    if (f + f != 0.2f) return 5;
    if (d * 3 == 0.3) return 6;
    if (l * 10 != 1.0L) return 7;
    if ((double) 0.1f == 0.1) return 8;
    if ((float) (long double) 0.1f != 0.1f) return 9;
    f = 16777217;
    if (f != 16777216.0f) return 10;

    // An integer operand converts to the floating one's type.
    if (i / 2.0 != 1.5) return 11;
    if ((1 ? 1 : 2.5) != 1.0) return 12;
    if (sizeof(1 ? 1 : 2.5) != 8) return 13;

    // A conversion to an integer truncates toward zero.
    if ((int) (d * 100) != 10) return 14;
    if ((int) -2.5 != -2) return 15;
    if ((long) -2.5L != -2) return 16;
    uc = 250.7;
    if (uc != 250) return 17;
    sc = -3.99;
    if (sc != -3) return 18;
    ui = 4000000000.5;
    if (ui != 4000000000u) return 19;

    // An unsigned long past 2^63 survives the trip both ways.
    if ((double) u != 1.8e19) return 20;
    if ((unsigned long) (double) u != u) return 21;
    if ((unsigned long) (long double) u != u) return 22;
    if ((long double) u != 18000000000000000000.0L) return 23;
    if ((unsigned long) (float) 9300000000000000000UL != 9300000300729368576UL) return 24;

    // File-scope objects are filled in at compile time.
    if (gf != 1.5) return 25;
    if (gd != -2.25) return 26;
    if (gl != 0.75) return 27;
    if (garr[2] != 10000000000.0) return 28;
    if (gi != 7) return 29;
    if (gu != 18000000000000000000UL) return 30;

    // Comparisons, where NaN is unordered with everything, itself included.
    if (-d != -0.1) return 31;
    if (-l >= 0) return 32;
    if (-0.0 != 0.0) return 33;
    if (! (d < 0.2 && d <= 0.1 && d > 0.05 && d >= 0.1)) return 34;
    d = 0.0 / 0.0;
    if (d == d) return 35;
    if (! (d != d)) return 36;
    if (d < 1 || d <= 1 || d > 1 || d >= 1) return 37;

    // A floating value tested for truth compares unequal to zero.
    if (0.0) return 38;
    if (! 0.5) return 39;
    if ((_Bool) 0.25 != 1) return 40;
    i = 0;
    for (d = 0; d < 1; d += 0.25) {
        i++;
    }
    if (i != 4) return 41;
    while (0.0L) {
        return 42;
    }

    // Compound assignment and ++ convert through the wider type and back.
    d = 0.1;
    d += 1;
    if (d != 1.1) return 43;
    d++;
    if (d != 2.1) return 44;
    l -= 0.5;
    if (l > -0.39 || l < -0.41) return 45;
    i = 3;
    i += 2.7;
    if (i != 5) return 46;
    i *= 1.5;
    if (i != 7) return 47;
    *next(q) += 1;
    if (calls != 1 || arr[1] != 2.5) return 48;
    q[2]++;
    if (arr[2] != 3.5) return 49;

    // Members, bit-fields and arrays hold floating values like any other.
    p = gp;
    if (p.x != 2.5f || p.y != -1e-3 || p.z != 7) return 50;
    p.z *= 2;
    if (p.z != 14) return 51;
    p.x /= 2;
    if (p.x != 1.25f) return 52;
    p.bits += 1.9;
    if (p.bits != 4) return 53;
    la[0] = 1;
    la[1] = 2;
    la[2] = la[0] + la[1];
    if (la[2] != 3) return 54;
    t = la[2]--;
    if (t != 3 || la[2] != 2) return 55;

    // The x87's range reaches past a double's.
    t = 1e300L;
    if (t * t / t != t) return 56;

    return 200;
}
