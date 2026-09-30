// (Test) Compiler error: [ERR_PP_EXPR_FLOAT]
// A floating constant in #if.

#if 1.0
#endif

int main(void)
{
    return 0;
}
