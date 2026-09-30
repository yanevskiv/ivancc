// (Test) Compiler error: [ERR_PAR_VLA_STAR_NOT_PROTOTYPE]
// `[*]` on a local rather than on a prototype's parameter.

void f(int n)
{
    int a[*];
}

int main(void)
{
    return 0;
}
