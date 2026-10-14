// (Test) Compiler error: [ERR_SEM_CASE_DUPLICATE]
// Can't have two case labels with the same value.

int f(int n)
{
    switch (n) {
        case 1: {
            return 1;
        } break;
        case 1: {
            return 2;
        } break;
    }
    return 0;
}
