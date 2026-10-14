// (Test) Compiler warning: [ERR_PP_EXTRA_TOKENS]
// (Test) Status: 0
// Shouldn't put tokens after #endif. They are ignored.

#ifdef UNDEFINED
#endif UNDEFINED

int main(void)
{
    return 0;
}
