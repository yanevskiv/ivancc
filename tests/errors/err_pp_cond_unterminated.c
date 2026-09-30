// (Test) Compiler error: [ERR_PP_COND_UNTERMINATED]
// An #if with no #endif before the end of the file.

#if 1

int main(void)
{
    return 0;
}
