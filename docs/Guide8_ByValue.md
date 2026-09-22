# Passing structs by value

A structure could be declared and assigned but never passed to a function or returned from one, because nothing decided where an aggregate travels. This stage adds the SysV classification algorithm and rebuilds the call sequence around it. Five parts cover the work, and the test below is what they exist to compile: [Parser](Guide8.1_Parser.md), [Ast](Guide8.2_Ast.md), [Sem](Guide8.3_Sem.md), [Abi](Guide8.4_Abi.md), and [Gen](Guide8.5_Gen.md).

```c
struct Point { int x; int y; };
struct Quad  { int a; int b; int c; int d; };
struct Big   { int a; int b; int c; int d; int e; int f; };

struct Point add(struct Point a, struct Point b)
{
    struct Point r;

    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

struct Quad quad(int n)
{
    struct Quad q;

    q.a = n;
    q.b = n + 1;
    q.c = n + 2;
    q.d = n + 3;
    return q;
}

int sumq(struct Quad q)
{
    return q.a + q.b + q.c + q.d;
}

struct Big big(int n)
{
    struct Big b;

    b.a = n;
    b.b = n + 1;
    b.c = n + 2;
    b.d = n + 3;
    b.e = n + 4;
    b.f = n + 5;
    return b;
}

int sumb(struct Big b)
{
    return b.a + b.b + b.c + b.d + b.e + b.f;
}

struct Big bump(struct Big b, int n)
{
    if (n <= 0) {
        return b;
    }
    b.a = b.a + 1;
    return bump(b, n - 1);
}

int four(struct Quad a, struct Quad b, struct Quad c, struct Quad d)
{
    return a.a + b.b + c.c + d.d;
}

int mixed(int n, struct Point p, struct Big b, int m, struct Point q)
{
    return n + p.x + b.a + b.f + m + q.y;
}

int main()
{
    struct Point p;
    struct Point q;
    struct Quad  w;
    struct Big   b;

    p.x = 1;  p.y = 2;
    q.x = 10; q.y = 20;
    w.a = 1;  w.b = 2;  w.c = 3;  w.d = 4;
    b.a = 1;  b.b = 2;  b.c = 3;  b.d = 4;  b.e = 5;  b.f = 6;

    if (sizeof(struct Point) != 8) return 1;
    if (sizeof(struct Quad) != 16) return 2;
    if (sizeof(struct Big) != 24) return 3;

    struct Point s = add(p, q);
    if (s.x != 11 || s.y != 22) return 4;

    struct Quad t = quad(1);
    if (t.a != 1 || t.d != 4) return 5;
    if (sumq(t) != 10) return 6;

    struct Big g = big(10);
    if (g.a != 10 || g.f != 15) return 7;
    if (sumb(g) != 75) return 8;

    if (sumq(quad(1)) != 10) return 9;
    if (sumb(big(10)) != 75) return 10;
    if (add(p, q).y != 22) return 11;

    if (four(w, w, w, w) != 10) return 12;
    if (mixed(100, p, b, 200, q) != 328) return 13;

    if (sumb(bump(b, 5)) != 26) return 14;
    if (b.a != 1) return 15;

    return s.x + t.d + g.a + 17;
}
```
