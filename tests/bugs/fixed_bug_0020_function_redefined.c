// (Test) Compiler error: [ERR_PAR_FUNCTION_REDEFINED]
// bug_0020. A second definition of a function compiled, and the later body
// silently replaced the first.

int f(void) { return 1; }
int f(void) { return 2; }

int main()
{
    return f();
}
