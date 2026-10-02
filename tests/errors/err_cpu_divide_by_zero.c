// (Test) Status: 136
// (Test) Emulator error: [ERR_CPU_DIVIDE_BY_ZERO]
// Can't divide by zero.

int main(void)
{
    volatile long d = 0;
    return (int) (5 / d);
}
