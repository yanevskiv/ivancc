// (Test) Compiler error: [ERR_PAR_KNR_NOT_PARAM]
// Can't declare a name an old-style parameter list does not have.

int f(a)
    int b;
{
    return 0;
}
