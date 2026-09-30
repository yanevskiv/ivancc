// (Test) Compiler error: [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER]
// Passing a pointer to const where a pointer to non-const is expected drops
// the qualifier.

int length(char *s) { int n = 0; while (s[n]) n++; return n; }

int main()
{
    const char *s = "abc";

    return length(s);
}
