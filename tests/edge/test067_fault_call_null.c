// (Test) Status: 139
// A call through a null function pointer kills the program with SIGSEGV.

int main(void)
{
    int (*volatile f)(void) = 0;
    return f();
}
