// (Test) Compiler error: [ERR_PAR_INIT_STRING_WIDTH]
// An array of `int` initialized by a narrow string literal.

int a[4] = "abc";

int main(void)
{
    return 0;
}
