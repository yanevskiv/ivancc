// (Test) Compiler error: [ERR_PAR_SYNTAX]
// A comma alone between braces is no initializer list, nested inside another.

struct P { int a[2]; int b; };

int main()
{
    struct P p = { { , }, 1 };
    return p.b;
}
