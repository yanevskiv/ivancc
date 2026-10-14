// (Test) Status: 0
// An assignment to a variable takes the address after the value: locals,
// globals and statics of each width and kind, structs copied from a call and
// from a variable, values that assign the same variable first, chains, and the
// assignment's own value.

struct pair {
    long a;
    long b;
    char tag[9];
};

long global;
short gshort;
struct pair gpair;

struct pair make(long a, long b)
{
    struct pair result = {a, b, "pair"};
    return result;
}

int set(int *dst)
{
    *dst = 3;
    return *dst;
}

long bump(void)
{
    global += 100;
    return global;
}

int main(void)
{
    char c;
    unsigned short us;
    int i;
    long l;
    _Bool flag;
    float f;
    double d;
    long double ld;
    int *ptr;
    struct pair local;
    static int counter;

    c = -3;
    us = 70000;
    i = -2147483647 - 1;
    l = 0x123456789AL;
    if (c != -3 || us != 4464 || i != -2147483647 - 1 || l != 0x123456789AL) {
        return 1;
    }
    flag = 5;
    f = 1.5F;
    d = -2.25;
    ld = 3.5L;
    if (flag != 1 || f != 1.5F || d != -2.25 || ld != 3.5L) {
        return 2;
    }
    ptr = &i;
    gshort = -5;
    counter = 41;
    counter = counter + 1;
    if (ptr != &i || *ptr != -2147483647 - 1 || gshort != -5 || counter != 42) {
        return 3;
    }
    local = make(1, 2);
    gpair = make(3, 4);
    if (local.a != 1 || local.b != 2 || gpair.a != 3 || gpair.b != 4 || gpair.tag[3] != 'r') {
        return 4;
    }
    local = gpair;
    gpair = make(5, 6);
    if (local.a != 3 || local.b != 4 || gpair.a != 5) {
        return 5;
    }
    i = set(&i) + 4;
    global = 1;
    global = bump() + 1;
    if (i != 7 || global != 102) {
        return 6;
    }
    l = i = c = 9;
    if (l != 9 || i != 9 || c != 9 || (l = 11) != 11 || (c = 300) != 44) {
        return 7;
    }
    return 0;
}
