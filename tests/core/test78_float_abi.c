// (Test) Status: 200
// Floating arguments and results cross calls the way the SysV ABI places
// them. A float or a double takes the next of %xmm0 to %xmm7, a long double
// goes on the stack at a 16-byte boundary and comes back in %st, and a small
// struct splits into eightbytes that each pick an integer or an SSE register.

struct F2 { float a; float b; };
struct D2 { double a; double b; };
struct DL { double d; long l; };
struct LD { long l; double d; };
struct F3 { float a; float b; float c; };
struct IF { int i; float f; };
struct E { long double x; };
struct Big { double a; double b; double c; };
union UL { long double x; long l; };

double mix(int a, double b, long c, float d)
{
    return a + b * 2 + c * 3 + d * 4;
}

// The ninth double overflows onto the stack.
double nine(double a, double b, double c, double d, double e, double f, double g, double h, double i)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h + 9 * i;
}

// Integers and doubles run out of registers at different points.
double many(int a1, double d1, int a2, double d2, int a3, double d3, int a4, double d4, int a5, double d5, int a6, double d6, int a7, double d7, int a8, double d8, double d9, double d10)
{
    return a1 + a2 * 2 + a3 * 3 + a4 * 4 + a5 * 5 + a6 * 6 + a7 * 7 + a8 * 8 + d1 * 10 + d2 * 20 + d3 * 30 + d4 * 40 + d5 * 50 + d6 * 60 + d7 * 70 + d8 * 80 + d9 * 90 + d10 * 100;
}

long double ldmix(int a, long double b, double c, long double d, int e)
{
    return a + b * 2 + c * 3 + d * 4 + e * 5;
}

// The stack holds g, padding, x, h, padding, y.
long double ldpad(int a, int b, int c, int d, int e, int f, int g, long double x, int h, long double y)
{
    return a + b + c + d + e + f + g * 100 + x * 1000 + h * 10000 + y * 100000;
}

struct F2 f2(struct F2 x, float s)
{
    struct F2 r;
    r.a = x.a * s;
    r.b = x.b + s;
    return r;
}

struct D2 d2swap(struct D2 x)
{
    struct D2 r;
    r.a = x.b;
    r.b = x.a;
    return r;
}

struct DL dl(struct DL x)
{
    struct DL r;
    r.d = x.d * 2;
    r.l = x.l + 1;
    return r;
}

struct LD ld(struct LD x)
{
    struct LD r;
    r.l = x.l * 3;
    r.d = x.d - 1;
    return r;
}

struct F3 f3(struct F3 x)
{
    struct F3 r;
    r.a = x.c;
    r.b = x.a;
    r.c = x.b;
    return r;
}

struct IF ifn(struct IF x)
{
    struct IF r;
    r.i = x.i + 7;
    r.f = x.f * 2;
    return r;
}

struct E eadd(struct E a, struct E b)
{
    struct E r;
    r.x = a.x + b.x;
    return r;
}

struct Big big(struct Big a, double s)
{
    struct Big r;
    r.a = a.c * s;
    r.b = a.b * s;
    r.c = a.a * s;
    return r;
}

union UL unl(union UL x)
{
    union UL r;
    r.x = x.x * 2;
    return r;
}

// Only one SSE register is left for x, so all of x goes on the stack.
double squeeze(double a, double b, double c, double d, double e, double f, double g, struct D2 x, double h)
{
    return a + b + c + d + e + f + g + x.a * 10 + x.b * 100 + h * 1000;
}

double vsum(int n, ...)
{
    __builtin_va_list ap;
    double t = 0;
    int i;

    __builtin_va_start(ap, n);
    for (i = 0; i < n; i++) {
        t = t * 2 + __builtin_va_arg(ap, double);
    }
    __builtin_va_end(ap);
    return t;
}

long double vmixed(int n, ...)
{
    __builtin_va_list ap;
    long double t = 0;
    int i;

    __builtin_va_start(ap, n);
    for (i = 0; i < n; i++) {
        int k = __builtin_va_arg(ap, int);
        double d = __builtin_va_arg(ap, double);
        long double x = __builtin_va_arg(ap, long double);
        t = t * 3 + k * d + x;
    }
    __builtin_va_end(ap);
    return t;
}

struct L2 { long a; long b; };

// Leading ints and doubles decide which structs still fit in registers.
double vstruct(int ints, int dbls, ...)
{
    __builtin_va_list ap;
    double t = 0;
    int i;
    struct L2 l2;
    struct F2 f2;
    struct DL dl;
    struct LD ld;
    struct F3 f3;
    struct E e;
    struct Big bg;

    __builtin_va_start(ap, dbls);
    for (i = 0; i < ints; i++) {
        t = t * 2 + __builtin_va_arg(ap, int);
    }
    for (i = 0; i < dbls; i++) {
        t = t * 2 + __builtin_va_arg(ap, double);
    }
    l2 = __builtin_va_arg(ap, struct L2);
    t = (t * 2 + l2.a) * 2 + l2.b;
    t = t * 2 + __builtin_va_arg(ap, int);
    f2 = __builtin_va_arg(ap, struct F2);
    t = (t * 2 + f2.a) * 2 + f2.b;
    dl = __builtin_va_arg(ap, struct DL);
    t = (t * 2 + dl.d) * 2 + dl.l;
    ld = __builtin_va_arg(ap, struct LD);
    t = (t * 2 + ld.l) * 2 + ld.d;
    f3 = __builtin_va_arg(ap, struct F3);
    t = ((t * 2 + f3.a) * 2 + f3.b) * 2 + f3.c;
    e = __builtin_va_arg(ap, struct E);
    t = t * 2 + e.x;
    bg = __builtin_va_arg(ap, struct Big);
    t = ((t * 2 + bg.a) * 2 + bg.b) * 2 + bg.c;
    t = t * 2 + __builtin_va_arg(ap, double);
    __builtin_va_end(ap);
    return t;
}

// The value vstruct folds from its arguments.
double expect(int ints, int dbls)
{
    double vals[17] ={ 3, 4, 5, 0.5, 1.5, 4.5, 6, 7, 8.5, 1, 2, 3, 10.5, 1, 2, 3, 12.5 };
    double t = 0;
    int i;

    for (i = 0; i < ints; i++) {
        t = t * 2 + (i + 1);
    }
    for (i = 0; i < dbls; i++) {
        t = t * 2 + (i + 1);
    }
    for (i = 0; i < 17; i++) {
        t = t * 2 + vals[i];
    }
    return t;
}

double scale(double x, float y)
{
    return x * y;
}

double apply(double (*fn)(double, float), double x, float y)
{
    return fn(x, y) + 1;
}

// An old-style float parameter arrives as a double.
double knr(a, b)
    float a;
    double b;
{
    return a + b;
}

double unproto();

int main()
{
    struct F2 a2;
    struct F2 r2;
    struct D2 b2;
    struct D2 s2;
    struct DL c;
    struct DL rc;
    struct LD d;
    struct LD rd;
    struct F3 e3;
    struct F3 r3;
    struct IF g;
    struct IF rg;
    struct E x;
    struct E y;
    struct E rx;
    struct Big bg;
    struct Big rb;
    union UL w;
    union UL rw;
    struct L2 l2;

    if (mix(1, 2.5, 3, 0.5f) != 17) return 1;
    if (nine(1, 2, 3, 4, 5, 6, 7, 8, 9) != 285) return 2;
    if (many(1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10) != 4054) return 3;
    if (ldmix(1, 2.0L, 3.0, 4.0L, 5) != 55) return 4;
    if (ldpad(1, 2, 3, 4, 5, 6, 7, 8.0L, 9, 10.0L) != 1098721) return 5;

    // Each eightbyte of a small struct takes its own class.
    a2.a = 1.5f;
    a2.b = 2.5f;
    r2 = f2(a2, 2.0f);
    if (r2.a != 3.0f || r2.b != 4.5f) return 6;
    b2.a = 1.25;
    b2.b = 2.5;
    s2 = d2swap(b2);
    if (s2.a != 2.5 || s2.b != 1.25) return 7;
    c.d = 1.5;
    c.l = 41;
    rc = dl(c);
    if (rc.d != 3.0 || rc.l != 42) return 8;
    d.l = 5;
    d.d = 2.5;
    rd = ld(d);
    if (rd.l != 15 || rd.d != 1.5) return 9;
    e3.a = 1;
    e3.b = 2;
    e3.c = 3;
    r3 = f3(e3);
    if (r3.a != 3 || r3.b != 1 || r3.c != 2) return 10;
    g.i = 1;
    g.f = 1.5f;
    rg = ifn(g);
    if (rg.i != 8 || rg.f != 3.0f) return 11;

    // A lone long double member returns in %st, and anything else in memory.
    x.x = 1.5L;
    y.x = 2.25L;
    rx = eadd(x, y);
    if (rx.x != 3.75L) return 12;
    bg.a = 1;
    bg.b = 2;
    bg.c = 3;
    rb = big(bg, 2.0);
    if (rb.a != 6 || rb.b != 4 || rb.c != 2) return 13;
    w.x = 1.5L;
    rw = unl(w);
    if (rw.x != 3.0L) return 14;
    b2.a = 2;
    b2.b = 3;
    if (squeeze(1, 2, 3, 4, 5, 6, 7, b2, 8) != 8348) return 15;

    // Anonymous doubles come from the save area, then from the stack.
    if (vsum(4, 1.0, 2.0, 3.0, 4.0) != 26) return 16;
    if (vsum(10, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) != 2036) return 17;
    if (vmixed(2, 1, 0.5, 2.0L, 2, 1.5, 3.0L) != 13.5) return 18;
    if (vsum(2, 1.5f, 2.5f) != 5.5) return 19;

    // A call through a pointer and a call with no prototype.
    if (apply(scale, 1.5, 4.0f) != 7.0) return 20;
    if (knr(1.5f, 2.0) != 3.5) return 21;
    if (unproto(0.25f, 3) != 0.75) return 22;
    if (mix(1, 2.5, 3, 0.5f) + mix(0, 0, 0, 1) != 21) return 23;

    // A struct read by va_arg takes all its registers, or only the stack.
    l2.a = 3;
    l2.b = 4;
    a2.a = 0.5f;
    a2.b = 1.5f;
    c.d = 4.5;
    c.l = 6;
    d.l = 7;
    d.d = 8.5;
    e3.a = 1;
    e3.b = 2;
    e3.c = 3;
    x.x = 10.5L;
    bg.a = 1;
    bg.b = 2;
    bg.c = 3;
    if (vstruct(0, 0, l2, 5, a2, c, d, e3, x, bg, 12.5) != expect(0, 0)) return 24;
    if (vstruct(3, 0, 1, 2, 3, l2, 5, a2, c, d, e3, x, bg, 12.5) != expect(3, 0)) return 25;
    if (vstruct(0, 4, 1.0, 2.0, 3.0, 4.0, l2, 5, a2, c, d, e3, x, bg, 12.5) != expect(0, 4)) return 26;
    return 200;
}

double unproto(x, n)
    double x;
    int n;
{
    return x * n;
}
