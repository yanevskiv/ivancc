// (Test) Status: 132
// An opcode invalid in 64-bit mode kills the program with SIGILL.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0x06 # push %es");
#endif
    return 0;
}
