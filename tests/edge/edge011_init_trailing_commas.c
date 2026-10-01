// (Test) Status: 200
// A trailing comma closes a braced initializer at every depth, in static and
// local objects and in compound literals.

struct P { int a[2]; int b; };

int g[3] = { 1, 2, };
struct P gp = { { 3, 4, }, 5, };
int *gl = (int [2]) { 6, 7, };

int main()
{
    int v[] = { 8, 9, 10, };
    struct P p = { .b = 11, .a = { 12, }, };
    int n = sizeof(v) / sizeof(v[0]);
    int *l = (int [1]) { 13, };

    if (g[1] != 2 || gp.a[1] != 4 || gp.b != 5 || gl[1] != 7) return 1;
    if (n != 3 || v[2] != 10) return 2;
    if (p.a[0] != 12 || p.a[1] != 0 || p.b != 11) return 3;
    if (l[0] != 13) return 4;
    if (((struct P) { { 14, }, }).a[0] != 14) return 5;
    return 200;
}
