// (Test) Compiler error: [ERR_PP_DEFINED_PAREN_MISSING]
// `defined(` with no closing parenthesis.

#if defined(X
#endif

int main(void)
{
    return 0;
}
