// (Test) Compiler error: [ERR_PP_EXPR_COLON_MISSING]
// A `?` in #if with no `:` after it.

#if 1 ? 2
#endif

int main(void)
{
    return 0;
}
