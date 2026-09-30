// (Test) Compiler error: [ERR_PP_LINE_NAME_INVALID]
// A #line whose file name is not a string literal.

#line 5 name

int main(void)
{
    return 0;
}
