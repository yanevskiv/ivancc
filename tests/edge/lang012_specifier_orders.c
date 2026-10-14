// (Test) Status: 200
// Storage-class and function specifiers in any position among the type
// specifiers and qualifiers, at file scope, in blocks, on parameters and on
// old-style parameter declarations.

typedef int static_t;
int typedef int_t;
long unsigned typedef long_t;
unsigned static const short table[2] = { 7, 8 };
volatile extern int shared;
int volatile shared = 9;
static int inline twice(int x) { return 2 * x; }
extern inline int thrice(int x);
int thrice(int x) { return 3 * x; }
int static (*pick)(int);
enum Mode { LOW, HIGH } static mode = HIGH;
struct Point { int x; int y; } const typedef Point;

int knr(a, b)
    int register a;
    register long b;
{
    return a + (int) b;
}

int sum(const register int a, int register const b)
{
    return a + b;
}

int main()
{
    int auto a = 1;
    auto int b = 2;
    int register c = 3;
    char typedef ch;
    ch d = 4;
    int static counted = 5;
    int volatile extern shared;
    Point p = { 10, 20 };
    int_t e = 6;
    long_t f = 7;
    static_t g = 8;

    pick = twice;
    if (a + b + c + d + counted + e + f + g != 36) return 1;
    if (table[0] + table[1] != 15 || shared != 9) return 2;
    if (twice(4) != 8 || thrice(4) != 12 || pick(5) != 10) return 3;
    if (mode != HIGH || p.x + p.y != 30) return 4;
    if (knr(1, 2) != 3 || sum(3, 4) != 7) return 5;
    return 200;
}
