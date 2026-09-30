// (Test) Compiler error: [ERR_PAR_CONFLICTING_TYPES]
// bug018. A file-scope object declared again with another type compiled, and
// kept the first type.

int x;
double x;

int main()
{
    return 0;
}
