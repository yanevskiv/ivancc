// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_D9_NOT_IMPLEMENTED]
// Can't run a D9 x87 operation the CPU does not implement.
// Note: D9 D1 is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xd9, 0xd1 # D9 D1");
#endif
    return 0;
}
