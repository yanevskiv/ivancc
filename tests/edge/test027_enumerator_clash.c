// (Test) Compiler error: [ERR_PAR_ENUM_REDECLARED]
// An enumerator and a variable in one block share the ordinary name space.

int main()
{
    int A = 1;
    enum { A = 2 };

    return A;
}
