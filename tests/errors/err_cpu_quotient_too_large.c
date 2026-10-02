// (Test) Status: 136
// (Test) Emulator error: [ERR_CPU_QUOTIENT_TOO_LARGE]
// Can't divide when the quotient overflows its register.

int main(void)
{
    volatile long n = -9223372036854775807L - 1;
    volatile long d = -1;
    return (int) (n / d);
}
