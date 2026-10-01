// (Test) Status: 139
// A store through a null pointer kills the program with SIGSEGV.

int main(void)
{
    volatile int *p = 0;
    *p = 1;
    return 0;
}
