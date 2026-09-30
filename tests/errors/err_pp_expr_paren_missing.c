// (Test) Compiler error: [ERR_PP_EXPR_PAREN_MISSING]
// A parenthesis in #if that never closes.

#if (1
#endif

int main(void)
{
    return 0;
}
