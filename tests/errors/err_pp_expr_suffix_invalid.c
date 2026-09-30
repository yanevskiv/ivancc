// (Test) Compiler error: [ERR_PP_EXPR_SUFFIX_INVALID]
// An integer in #if with a suffix no integer takes.

#if 1x
#endif

int main(void)
{
    return 0;
}
