// (Test) Return: 200
// Compound literals: `(T){...}` names an unnamed object of type T, filled the
// way a declaration's initializer fills a variable. Inside a function the
// object has automatic storage and is refilled on each evaluation; outside one
// it has static storage, so the linker lays it down and its address is a
// constant.

struct Point { int x; int y; };
union Word   { int n; char b[4]; };

struct Point origin = (struct Point){3, 4};
int         *triple = (int[3]){7, 8, 9};
struct Point *anchor = &(struct Point){11, 12};

int sum(struct Point p)
{
    return p.x + p.y;
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

    // An array literal decays to a pointer where a pointer is wanted.
    if (third((int[3]){7, 8, 9}) != 9) return 7;
    if ((int[3]){7, 8, 9}[1] != 8) return 8;

    // A union literal fills its first member, leaving the rest of it zero.
    if ((union Word){0x4241}.b[0] != 'A') return 9;
    if ((union Word){0x4241}.b[2] != 0) return 10;

    // As a declaration's initializer, which copies the literal into the object.
    struct Point r = (struct Point){2, 3};
    if (r.x != 2 || r.y != 3) return 11;

    // Refilled on each evaluation, so the store below never accumulates.
    total = 0;
    for (i = 0; i < 3; i++) {
        struct Point *s = &(struct Point){0, 0};
        s->x = s->x + 1;
        total = total + s->x;
    }
    if (total != 3) return 12;

    // With static storage nothing runs to fill the object.
    if (origin.x != 3 || origin.y != 4) return 13;
    if (triple[0] != 7 || triple[2] != 9) return 14;
    if (anchor->x != 11 || anchor->y != 12) return 15;

    static struct Point kept = (struct Point){5, 6};
    if (kept.x != 5 || kept.y != 6) return 16;

    if (sum((struct Point){20, 22}) != 42) return 17;
    return 200;
}
