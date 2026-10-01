// (Test) Compiler error: [ERR_PAR_FUNCTION_CONFLICTING_TYPES]
// A definition whose parameters disagree with an earlier prototype's is a
// conflicting declaration, as is a qualifier that differs below the top level.

int f(const char *s);

int f(char *s)
{
    return s[0];
}

int main()
{
    return 0;
}
