// (Test) Compiler error: [ERR_SEM_CASE_INTO_VARMOD_SCOPE]
// Can't jump from a switch into the scope of a variable-length array.

int f(int n)
{
    switch (n) {
        int a[n];
        case 1:
            a[0] = 0;
    }
    return 0;
}
