// (Test) Compiler error: [ERR_PP_EXPR_EMPTY]
// An #if with nothing to evaluate.

#if
#endif

int main(void)
{
    return 0;
}
