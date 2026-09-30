// (Test) Compiler error: [ERR_SEM_RETURN_NO_VALUE]
// bug027. A bare return in a function returning a value compiled, and the
// caller read whatever the register held.

int f(int x)
{
    if (x) return;
    return 1;
}

int main()
{
    return f(1);
}
