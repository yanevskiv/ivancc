// (Test) Status: 34
// bug035. A call through anything but a bare function name, given for a
// struct member or element of a braced initializer, crashed the compiler.
// `Par_ExprType` looked the call up by its callee's name, which only a direct
// call has. Now each item's type comes from its callee's type.

struct S { int a, b; };
struct T { struct S s; };
struct W { struct S s; int c; };

struct S s = {1, 2};

struct S mk(void) { return s; }
struct T mt(void) { struct T t = {{3, 4}}; return t; }
struct S *ps(void) { return &s; }
int seven(void) { return 7; }

struct S (*fp)(void) = mk;
struct T (*ft)(void) = mt;
struct S *(*pp)(void) = ps;
int (*ip)(void) = seven;

int main(void)
{
    struct W w1 = { fp(), 3 };
    struct W w2 = { (*mk)(), 3 };
    struct W w3 = { (&mk)(), 3 };
    struct W w4 = { (**fp)(), 3 };
    struct W w5 = { ft().s, 3 };
    struct W w6 = { *pp(), 3 };
    struct W w7 = { ip(), 8, 9 };
    struct S arr[2] = { fp(), (*fp)() };
    int sum = 0;

    sum += w1.s.b + w1.c;
    sum += w2.s.b + w2.c;
    sum += w3.s.b + w3.c;
    sum += w4.s.b + w4.c;
    sum += w5.s.b + w5.c - 2;
    sum += w6.s.b + w6.c;
    sum += w7.s.a == 7 && w7.s.b == 8 && w7.c == 9;
    sum += arr[0].a + arr[1].b;
    return sum;
}
