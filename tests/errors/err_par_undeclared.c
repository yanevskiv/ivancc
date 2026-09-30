// (Test) Compiler error: [ERR_PAR_UNDECLARED]
// A name used with no declaration in scope.

int f(void)
{
    return y;
}

int main(void)
{
    return 0;
}
