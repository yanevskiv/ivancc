// (Test) Status: 200
// Conversions simple assignment allows: void pointers both ways, every null
// pointer constant, pointers to _Bool, qualifiers added to the pointed-to
// type, compatible structs and function pointers, decayed arrays and
// functions, and conditionals mixing pointers with null and void pointers.

struct P { int x; int y; };

int x = 3;
void *vp = &x;
int *ip = (void *) 0;
const int *cp = &x;
char *str = "abc";
int (*fp)(const void *, const void *);

int cmp(const void *a, const void *b) { return *(const int *) a - *(const int *) b; }
void *pass(void *p) { return p; }
const char *name(void) { return "name"; }
struct P make(int v) { struct P p = { v, v }; return p; }
_Bool truth(int *p) { return p; }

int main()
{
    int a[3] = { 1, 2, 3 };
    int *p = a;
    int *q = 0;
    long *lp = 0L;
    char *cz = '\0';
    double *dz = 1 - 1;
    const int * const * ppc;
    struct P s;
    struct P t = make(4);
    int (*f)(const void *, const void *) = cmp;
    void (*reset)(void) = 0;
    int *either;

    fp = &cmp;
    vp = p;
    p = vp;
    cp = p;
    ppc = &cp;
    s = t;
    either = x ? p : 0;
    either = ! x ? 0 : either;
    vp = x ? (void *) p : (void *) lp;
    if (q || lp || cz || dz || reset || ip) return 1;
    if (*(int *) pass(a + 1) != 2 || **ppc != 1) return 2;
    if (s.y != 4 || name()[0] != 'n' || str[2] != 'c') return 3;
    if (f(&a[2], &a[0]) != 2 || fp(&a[0], &a[1]) != -1) return 4;
    if (! truth(p) || truth(q) || either != p) return 5;
    return 200;
}
