// (Test) Compiler error: [ERR_SEM_GOTO_INTO_VARMOD_SCOPE]
// A goto past the declaration of a variable-length array into its scope.

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

int main(void)
{
    return 0;
}
