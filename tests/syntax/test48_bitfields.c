// (Test) Return: 42
// Bitfields, which pack several members into one storage unit of their declared
// type. The unit is what a load and a store reach, so reading a field shifts it
// out and sign-extends it, and writing one has to leave its neighbours alone.

struct Flags {
    int a : 3;
    int b : 5;
    int c : 24;
};

struct Split {
    char lo : 3;
    char hi : 5;
    int  n  : 20;
};

// An unnamed field reserves bits no name can reach, a width of none moves the
// next field to the start of a fresh unit, and any constant expression is one.
struct Padded {
    int a : 4;
    int   : 2;
    int b : 4;
    int   : 0;
    int c : 4 * 2;
};

union Bits {
    int n;
    int low : 8;
};

struct Flags global = {1, 2, 3};

int sum(struct Flags *f)
{
    return f->a + f->b + f->c;
}

int bump(struct Flags *f)
{
    f->a = f->a + 1;
    return f->a;
}

int main()
{
    struct Flags f;
    struct Split s;
    struct Padded p;
    union Bits u;

    if (sizeof(struct Flags) != 4) return 1;
    if (sizeof(struct Split) != 4) return 2;
    if (sizeof(struct Padded) != 8) return 3;
    if (sizeof(union Bits) != 4) return 4;

    f.a = 3;
    f.b = 15;
    f.c = 1000;
    if (f.a != 3) return 5;
    if (f.b != 15) return 6;
    if (f.c != 1000) return 7;

    // Three signed bits hold -4 through 3, and writing one leaves the rest be.
    f.a = -4;
    if (f.a != -4) return 8;
    if (f.b != 15 || f.c != 1000) return 9;

    f.b = 20;
    if (f.b != -12) return 10;

    f.a += 5;
    if (f.a != 1) return 11;
    f.a++;
    if (f.a != 2) return 12;
    if (f.a++ != 2 || f.a != 3) return 13;
    f.a = 2;

    s.lo = 3;
    s.hi = -16;
    s.n = 100000;
    if (s.lo != 3 || s.hi != -16) return 14;
    if (s.n != 100000) return 15;

    p.a = 5;
    p.b = 6;
    p.c = 100;
    if (p.a != 5 || p.b != 6 || p.c != 100) return 16;

    u.n = 0;
    u.low = 65;
    if (u.n != 65) return 17;

    struct Flags i = {1, 2, 3};
    if (i.a != 1 || i.b != 2 || i.c != 3) return 18;

    struct Flags d = {.c = 7, .a = 2};
    if (d.a != 2 || d.b != 0 || d.c != 7) return 19;

    if (global.a != 1 || global.b != 2 || global.c != 3) return 20;

    struct Flags v;
    v.a = 1;
    v.b = 2;
    v.c = 3;
    if (sum(&v) != 6) return 21;
    if (bump(&v) != 2) return 22;
    if (v.a != 2) return 23;

    struct Flags *r = &v;
    r->b = 9;
    if (v.b != 9) return 24;

    struct Flags arr[2];
    arr[0].a = 1;
    arr[0].c = 3;
    arr[1].a = 3;
    arr[1].c = 6;
    if (arr[0].a != 1 || arr[0].c != 3) return 25;
    if (arr[1].a != 3 || arr[1].c != 6) return 26;

    return p.c + f.a - 60;
}
