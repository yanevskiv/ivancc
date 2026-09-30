// (Test) Compiler error: [ERR_SEM_GOTO_UNDEFINED]
// A goto to a label the function never defines.

int main(void)
{
    goto nowhere;
    return 0;
}
