// (Test) Return: 42
// A function named without calling it is its own address, and a pointer holding
// one is called by naming the pointer. The declarator is what says so: the
// parentheses in `int (*f)(int, int)` bind the star before the parameter list,
// which is the difference between a pointer to a function and a function
// returning a pointer.

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int twice(int n)
{
    return n + n;
}

// A function pointer is an ordinary parameter, so a function can take an operation.
int apply(int (*op)(int, int), int a, int b)
{
    return op(a, b);
}

// A function returning a pointer, which is what the parentheses above rule out.
int *first(int *p)
{
    return p;
}

struct Ops {
    int (*op)(int, int);
    int   tag;
};

int main()
{
    int (*f)(int, int);
    int (*table[2])(int, int);
    int (*g)(int);
    struct Ops ops;
    int arr[3];
    int *p;

    // A pointer to a function is a pointer, whatever it points at.
    if (sizeof(f) != 8) return 1;
    if (sizeof(table) != 16) return 2;

    f = add;
    if (f(20, 22) != 42) return 3;

    f = sub;
    if (f(50, 8) != 42) return 4;

    // Dereferencing a function pointer yields the function, which decays right back.
    if ((*f)(50, 8) != 42) return 5;
    if ((**f)(50, 8) != 42) return 6;

    // The address-of operator on a function name is allowed and changes nothing.
    f = &add;
    if (f(40, 2) != 42) return 7;

    if (apply(add, 20, 22) != 42) return 8;
    if (apply(sub, 50, 8) != 42) return 9;
    if (apply(f, 40, 2) != 42) return 10;

    table[0] = add;
    table[1] = sub;
    if (table[0](20, 22) != 42) return 11;
    if (table[1](50, 8) != 42) return 12;

    ops.op = add;
    ops.tag = 7;
    if (ops.op(35, 7) != 42) return 13;

    // A pointer compares against the function it was taken from.
    f = add;
    if (f != add) return 14;
    if (f == sub) return 15;
    if (! f) return 16;

    g = twice;
    if (g(21) != 42) return 17;

    arr[0] = 42;
    p = first(arr);
    if (*p != 42) return 18;

    return apply(table[0], 20, 22);
}
