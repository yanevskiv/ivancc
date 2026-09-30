// (Test) Compiler error: [ERR_GEN_CONTINUE_OUTSIDE_LOOP]
// A continue with no loop around it.

int main(void)
{
    continue;
    return 0;
}
