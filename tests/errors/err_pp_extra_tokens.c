// (Test) Compiler warning: [ERR_PP_EXTRA_TOKENS]
// (Test) Status: 0
// Tokens after #endif are ignored with a warning.

#ifdef UNDEFINED
#endif UNDEFINED

int main(void)
{
    return 0;
}
