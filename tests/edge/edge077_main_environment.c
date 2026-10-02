// (Test) Status: 0
// (Test) Arguments: one
// The environment is run_test's one known variable, after argv's null.

int same(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

int main(int argc, char **argv, char **envp)
{
    if (envp != argv + argc + 1) return 1;
    if (envp[0] == 0) return 2;
    if (! same(envp[0], "IVANCC_TEST=1")) return 3;
    if (envp[1] != 0) return 4;
    return 0;
}
