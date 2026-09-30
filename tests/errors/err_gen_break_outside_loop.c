// (Test) Compiler error: [ERR_GEN_BREAK_OUTSIDE_LOOP]
// Can't break outside a loop or a switch.

int main(void)
{
    break;
    return 0;
}
