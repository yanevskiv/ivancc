// (Test) Return: 42
// Struct declaration, member access and whole-struct assignment. The layout is
// what sizeof has to agree with gcc about: padding between members and after
// the last one. A member may be named in an initializer, and a `?:`, an
// assignment and a comma all yield a struct a member still reads out of.

struct Point {
    int x;
    int y;
};

struct Padded {
    char c;
    int  n;
    char d;
};

int main()
{
    struct Point p;
    struct Point q;
    struct Point r;
    struct Padded pad;

    if (sizeof(struct Point) != 8) return 1;
    if (sizeof(struct Padded) != 12) return 2;

    p.x = 20;
    p.y = 7;
    q = p;
    q.y += 8;

    if (p.y != 7) return 3;

    struct Point init = {1, 2};
    struct Point half = {4};
    struct Point named = {.y = 5};
    if (init.x != 1 || init.y != 2) return 5;
    if (half.x != 4 || half.y != 0) return 6;
    if (named.x != 0 || named.y != 5) return 11;

    if ((1 ? p : q).y != 7) return 7;
    if ((0 ? p : q).y != 15) return 8;
    if ((r = p).y != 7) return 9;
    if ((p, r).x != 20) return 10;

    pad.c = 1;
    pad.n = 5;
    pad.d = 2;
    if (pad.c != 1 || pad.n != 5 || pad.d != 2) return 4;

    return q.x + q.y + pad.n + pad.c + pad.d - 1;
}
