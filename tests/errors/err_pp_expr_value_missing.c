// (Test) Compiler error: [ERR_PP_EXPR_VALUE_MISSING]
// A binary operator in #if with no right operand.

#if 1 +
#endif

int main(void)
{
    return 0;
}
