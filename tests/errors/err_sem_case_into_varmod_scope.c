// (Test) Compiler error: [ERR_SEM_CASE_INTO_VARMOD_SCOPE]
// A case label inside the scope of a variable-length array its switch skips.

int f(int n)
{
    switch (n) {
        int a[n];
        case 1:
            a[0] = 0;
    }
    return 0;
}

int main(void)
{
    return 0;
}
