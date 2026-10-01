// (Test) Status: 139
// A load through a null pointer kills the program with SIGSEGV.

int main(void)
{
    volatile int *p = 0;
    return *p;
}
