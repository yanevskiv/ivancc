// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_DE_NOT_IMPLEMENTED]
// Can't run a DE x87 operation the CPU does not implement.
// Note: the hardware runs `fcompp`, so a ud2 after it kills the native
// run with SIGILL too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xde, 0xd9 # fcompp\n"
             ".byte 0x0f, 0x0b # ud2");
#endif
    return 0;
}
