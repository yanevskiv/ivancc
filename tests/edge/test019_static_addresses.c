// (Test) Status: 200
// Address constants in static initializers: elements, members, offsets either
// way, string literals, dereferenced addresses, constant conditions, and
// functions named alone or with `&`, in tables and in structs.

struct S { int a; int b[3]; };
struct Pt { char tag; long x; long y; };
struct Ops { int (*unary)(int); int (*binary)(int, int); };

int inc(int x) { return x + 1; }
static int add(int a, int b) { return a + b; }

int a[4] = { 10, 11, 12, 13 };
int m[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
char buf[8] = "abcdefg";
int x = 20;
int y = 30;
struct S s = { 40, { 41, 42, 43 } };
struct Pt pts[2] = { { 'p', 1, 2 }, { 'q', 3, 4 } };

int *p_elem = &a[2];
int *p_plus = a + 3;
int *p_back = &a[3] - 2;
int *p_row = m[1];
int *p_cell = &m[1][2];
int (*p_rowp)[3] = m + 1;
char *p_str = "str" + 2;
char *p_buf = &buf[3] - 3;
int *p_member = &s.a;
int *p_inner = &s.b[1];
int *p_arrow = &(&s)->b[2];
int *p_star = &*&x;
int *p_cond = 0 ? &x : &y;
long *p_field = &pts[1].y;
char *p_cast = (char *) &y + 0;
int (*f_bare)(int) = inc;
int (*f_amp)(int) = &inc;
int (*f_table[3])(int, int) = { add, &add, 0 };
struct Ops ops = { inc, add };
const char *names[] = { "zero", "one" + 1, "two" };

int local(void)
{
    static int k = 5;
    static int *pk = &k;
    static int **ppk = &pk;
    return **ppk;
}

int main()
{
    if (*p_elem != 12 || *p_plus != 13 || *p_back != 11) return 1;
    if (*p_row != 4 || *p_cell != 6 || (*p_rowp)[1] != 5) return 2;
    if (*p_str != 'r' || *p_buf != 'a') return 3;
    if (*p_member != 40 || *p_inner != 42 || *p_arrow != 43) return 4;
    if (*p_star != 20 || *p_cond != 30 || *p_field != 4) return 5;
    if (*(int *) p_cast != 30) return 6;
    if (f_bare(1) != 2 || f_amp(2) != 3) return 7;
    if (f_table[0](1, 2) != 3 || f_table[1](3, 4) != 7 || f_table[2]) return 8;
    if (ops.unary(5) != 6 || ops.binary(5, 6) != 11) return 9;
    if (names[1][0] != 'n' || names[2][2] != 'o') return 10;
    if (local() != 5) return 11;
    return 200;
}
