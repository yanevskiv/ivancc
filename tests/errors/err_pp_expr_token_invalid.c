// (Test) Compiler error: [ERR_PP_EXPR_TOKEN_INVALID]
// A string literal is no value in #if.

#if "a"
#endif

int main(void)
{
    return 0;
}
