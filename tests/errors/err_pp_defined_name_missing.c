// (Test) Compiler error: [ERR_PP_DEFINED_NAME_MISSING]
// `defined` applied to a number rather than a name.

#if defined(1)
#endif

int main(void)
{
    return 0;
}
