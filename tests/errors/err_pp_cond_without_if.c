// (Test) Compiler error: [ERR_PP_COND_WITHOUT_IF]
// An #endif with no #if open.

#endif

int main(void)
{
    return 0;
}
