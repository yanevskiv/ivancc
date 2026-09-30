// (Test) Compiler error: [ERR_SEM_CASE_DUPLICATE]
// Two case labels of one switch with the same value.

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

int main(void)
{
    return 0;
}
