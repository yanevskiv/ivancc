// (Test) Status: 200
// bug010. A storage class or `inline` was allowed only once and only first, so
// `static inline`, `int static` and `unsigned typedef long` were syntax errors.

static inline int one(void) { return 1; }
inline static int two(void) { return 2; }
int static three = 3;
const static int four = 4;
long extern five;
long five = 5;
unsigned typedef long ulong;
struct S { int a; } typedef S;

int reg(register int x) { return x; }

int main()
{
    int static counter;
    ulong big = 0xffffffffffffffff;
    S s = { 6 };

    if (one() + two() + three + four + five != 15) return 1;
    if (big + 1 != 0) return 2;
    if (s.a != 6 || reg(7) != 7 || counter != 0) return 3;
    return 200;
}
