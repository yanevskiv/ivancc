// (Test) Compiler error: [ERR_SEM_CASE_NOT_CONSTANT]
// A case label that is a variable.

int f(int n)
{
    switch (n) {
        case n: {
            return 1;
        } break;
    }
    return 0;
}

int main(void)
{
    return 0;
}
