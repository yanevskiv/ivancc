// (Test) Compiler error: [ERR_PAR_ARRAY_STATIC_NO_LEN]
// A parameter's `[static]` with no length after it.

void f(int a[static]);

int main(void)
{
    return 0;
}
