// (Test) Compiler error: [ERR_SEM_GOTO_INTO_VARMOD_SCOPE]
// Can't goto into the scope of a variable-length array.

int f(int n)
{
    goto inside;
    {
        int a[n];
    inside:
        a[0] = 0;
    }
    return 0;
}
