// (Test) Compiler error: [ERR_SEM_CASE_NOT_CONSTANT]
// Can't have a case label known only at run time.

int f(int n)
{
    switch (n) {
        case n: {
            return 1;
        } break;
    }
    return 0;
}
