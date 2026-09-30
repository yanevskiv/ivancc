// (Test) Status: 200
// Redeclarations C99 allows stay accepted: compatible file-scope objects and
// functions, prototypes repeated or completed, externs repeated in a block,
// and names shadowed in inner scopes.

extern int a[];
int a[3] = { 1, 2, 3 };
extern int a[3];
int t;
int t;
extern int t;
static int s;
static int s = 4;
struct P;
extern struct P p;
struct P { int x; } p = { 5 };

int f();
int f(int x);
int f(const int x) { return x + 1; }
int f(int);
long g(long (*op)(long), long v);
long g(long (*op)(long), long v) { return op(v); }
long neg(long v) { return -v; }
int old();
int old(n) int n; { return n * 2; }
enum { A = 7 };

int main()
{
    extern int t;
    extern int t;
    int f(int);
    int shadow = A;

    {
        int shadow = 8;
        enum { A = 9 };
        if (shadow != 8 || A != 9) return 1;
        {
            double shadow = 0.5;
            if (shadow != 0.5) return 2;
        }
    }
    if (shadow != 7 || A != 7) return 3;
    if (a[2] != 3 || t != 0 || s != 4 || p.x != 5) return 4;
    if (f(1) != 2 || g(neg, 3) != -3 || old(4) != 8) return 5;
    return 200;
}
