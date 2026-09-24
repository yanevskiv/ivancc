// (Test) Return: 200
// Designated initializers walking into subobjects, and the braces C lets a
// nested initializer leave out. What no item reaches stays zero, and a
// compound literal is filled by the same rules a declaration is.

struct Point { int x; int y; };
struct Line  { struct Point a; struct Point b; };
struct Rec   { int n; char c; struct Point p; };

struct Line  line   = {{1, 2}, {3, 4}};
struct Line  chain  = {.b.x = 7, .a.y = 3};
struct Rec   rec    = {.n = 5, .p = {.y = 9}};
int          grid[2][3] = {{1, 2, 3}, {4, 5, 6}};
int          flat[2][2] = {1, 2, 3, 4};
int          sparse[6]  = {[4] = 40, [1] = 10};

int span(struct Line l)
{
    return l.b.x - l.a.x + l.b.y - l.a.y;
}

int main()
{
    struct Line  l = {{1, 2}, {3, 4}};
    struct Rec   r = {.c = 'z', .n = 3};
    int          m[2][2] = {{1, 2}, {3, 4}};
    int          e[2][2] = {1, 2, 3, 4};

    if (line.a.x != 1 || line.b.y != 4) return 1;
    if (chain.b.x != 7 || chain.a.y != 3 || chain.a.x != 0) return 2;
    if (rec.n != 5 || rec.p.y != 9 || rec.p.x != 0 || rec.c != 0) return 3;
    if (grid[0][2] != 3 || grid[1][0] != 4) return 4;
    if (flat[0][1] != 2 || flat[1][1] != 4) return 5;
    if (sparse[1] != 10 || sparse[4] != 40 || sparse[0] != 0) return 6;

    if (l.a.x != 1 || l.b.y != 4) return 7;
    if (r.c != 'z' || r.n != 3 || r.p.x != 0) return 8;
    if (m[0][1] != 2 || m[1][0] != 3) return 9;
    if (e[0][1] != 2 || e[1][1] != 4) return 10;

    if (span((struct Line){{1, 2}, {4, 6}}) != 7) return 11;
    if (span((struct Line){1, 2, 4, 6}) != 7) return 12;
    if ((struct Line){.b.y = 6}.b.y != 6) return 13;

    if (sparse[4] + r.n - 1 != 42) return 14;
    return 200;
}
