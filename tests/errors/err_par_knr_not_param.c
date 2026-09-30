// (Test) Compiler error: [ERR_PAR_KNR_NOT_PARAM]
// An old-style declaration list that declares a name the identifier list lacks.

int f(a)
    int b;
{
    return 0;
}

int main(void)
{
    return 0;
}
