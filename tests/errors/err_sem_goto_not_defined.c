// (Test) Compiler error: [ERR_SEM_GOTO_NOT_DEFINED]
// Can't goto a label that is never defined.

int main(void)
{
    goto nowhere;
    return 0;
}
