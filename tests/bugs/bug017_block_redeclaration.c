// (Test) Compiler error: [ERR_PAR_REDECLARED]
// bug017. A name declared twice in one block compiled: the second declaration
// returned the first variable, whatever its type.

int main()
{
    int x = 5;
    int x;

    return x;
}
