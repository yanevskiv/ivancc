// (Test) Compiler error: [ERR_PP_EXPR_OPERATOR_MISSING]
// Two values in #if with no operator between them.

#if 1 2
#endif

int main(void)
{
    return 0;
}
