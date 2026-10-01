// (Test) Status: 136
// LONG_MIN / -1 overflows the quotient and kills the program with SIGFPE.

int main(void)
{
    volatile long n = -9223372036854775807L - 1;
    volatile long d = -1;
    return (int) (n / d);
}
