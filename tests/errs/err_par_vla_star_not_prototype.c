// (Test) Compiler error: [ERR_PAR_VLA_STAR_NOT_PROTOTYPE]
// Can't use `[*]` outside a prototype's parameters.

void f(int n)
{
    int a[*];
}
