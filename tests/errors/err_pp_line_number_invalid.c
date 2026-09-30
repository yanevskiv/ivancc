// (Test) Compiler error: [ERR_PP_LINE_NUMBER_INVALID]
// A #line whose line number is a name.

#line x

int main(void)
{
    return 0;
}
