// (Test) Compiler error: [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER]
// Can't store a pointer to `const int` in an `int *`.
// Note: gcc only warns.

const int c = 1;

int main(void)
{
    int *p = &c;

    return *p;
}
