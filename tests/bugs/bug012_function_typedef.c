// (Test) Status: 200
// bug012. A typedef of a function type declared a prototype instead, so the
// name never became a type.

typedef int F(int);

F twice;

int apply(F *g, int x)
{
    return g(x);
}

int twice(int x)
{
    return 2 * x;
}

int main()
{
    F *p = twice;

    if (apply(twice, 3) != 6) return 1;
    if (p(4) != 8) return 2;
    return 200;
}
