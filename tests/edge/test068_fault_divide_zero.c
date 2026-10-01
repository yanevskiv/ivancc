// (Test) Status: 136
// A division by zero kills the program with SIGFPE.

int main(void)
{
    volatile long d = 0;
    return (int) (5 / d);
}
