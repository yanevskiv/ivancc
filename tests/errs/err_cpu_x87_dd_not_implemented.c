// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_DD_NOT_IMPLEMENTED]
// Can't run a DD x87 operation the CPU does not implement.
// Note: DD F0 is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xdd, 0xf0 # DD F0");
#endif
    return 0;
}
