// (Test) Compiler error: [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER]
// A pointer to `const int` stored in a plain `int *`.

const int c = 1;

int main(void)
{
    int *p = &c;

    return *p;
}
