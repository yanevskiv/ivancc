// (Test) Compiler error: [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER]
// bug025. A pointer to const converted to a pointer to non-const, so a const
// object could be written through the copy.

const int cx = 1;

int main()
{
    int *p = &cx;

    *p = 2;
    return cx;
}
