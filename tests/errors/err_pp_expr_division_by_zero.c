// (Test) Compiler error: [ERR_PP_EXPR_DIVISION_BY_ZERO]
// A division by zero that #if evaluates.

#if 1 / 0
#endif

int main(void)
{
    return 0;
}
