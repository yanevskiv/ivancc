// (Test) Compiler error: [ERR_PP_COND_AFTER_ELSE]
// An #elif after the #else of the same group.

#if 1
#else
#elif 1
#endif

int main(void)
{
    return 0;
}
