// (Test) Status: 132
// A ud2 kills the program with SIGILL, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0x0f, 0x0b # ud2");
#endif
    return 0;
}
