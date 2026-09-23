// (Test) Return: 42
// Compound literals: `(T){...}` names an unnamed object of type T, filled the
// way a declaration's initializer fills a variable. The object is an lvalue
// with automatic storage, so it can be addressed, assigned to and passed on.

struct Point { int x; int y; };
struct Line  { struct Point a; struct Point b; };
union Word   { int n; char b[4]; };

int sum(struct Point p)
{
    return p.x + p.y;
}

int span(struct Line l)
{
    return l.b.x - l.a.x + l.b.y - l.a.y;
}

int shift(struct Point *p, int by)
{
    p->x = p->x + by;
    return p->x;
}

int third(int *a)
{
    return a[2];
}

int main()
{
    struct Point p;
    struct Point *q;
    int total;
    int i;

    // In expression position: as an argument, and as the source of an assign.
    if (sum((struct Point){3, 4}) != 7) return 1;
    p = (struct Point){10, 20};
    if (p.x != 10 || p.y != 20) return 2;

    // A member read straight off the literal, with no object to name it.
    if ((struct Point){5, 6}.y != 6) return 3;

    // The literal is an lvalue, so its address outlives the full expression.
    q = &(struct Point){1, 2};
    if (shift(q, 8) != 9 || q->y != 2) return 4;

    // What the item list leaves out is zero, as in any initializer.
    if ((struct Point){.y = 9}.x != 0) return 5;
    if ((struct Point){.y = 9}.y != 9) return 6;

    // Nested braces and elision both work, because the flattener is shared.
    if (span((struct Line){{1, 2}, {4, 6}}) != 7) return 7;
    if (span((struct Line){1, 2, 4, 6}) != 7) return 8;

    // An array literal decays to a pointer where a pointer is wanted.
    if (third((int[3]){7, 8, 9}) != 9) return 9;
    if ((int[3]){7, 8, 9}[1] != 8) return 10;

    // A union literal fills its first member, leaving the rest of it zero.
    if ((union Word){0x4241}.b[0] != 'A') return 11;
    if ((union Word){0x4241}.b[2] != 0) return 12;

    // As a declaration's initializer, which copies the literal into the object.
    struct Point r = (struct Point){2, 3};
    if (r.x != 2 || r.y != 3) return 13;

    // Refilled on each evaluation, so the store below never accumulates.
    total = 0;
    for (i = 0; i < 3; i++) {
        struct Point *s = &(struct Point){0, 0};
        s->x = s->x + 1;
        total = total + s->x;
    }
    if (total != 3) return 14;

    return sum((struct Point){20, 15}) + span((struct Line){{0, 0}, {4, 3}});
}
