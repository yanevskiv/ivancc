// (Test) Compiler warning: [ERR_PP_WARNING_DIRECTIVE]
// (Test) Status: 0
// Shouldn't reach a #warning. The compile carries on past it.

#warning carry on

int main(void)
{
    return 0;
}
