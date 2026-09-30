// (Test) Compiler error: [ERR_GEN_BREAK_OUTSIDE_LOOP]
// A break with no loop or switch around it.

int main(void)
{
    break;
    return 0;
}
