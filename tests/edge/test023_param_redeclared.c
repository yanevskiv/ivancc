// (Test) Compiler error: [ERR_PAR_REDECLARED]
// A function body's outermost block shares its parameters' scope, so it cannot
// declare a parameter's name again.

int f(int a)
{
    int a = 2;
    return a;
}

int main()
{
    return f(1);
}
