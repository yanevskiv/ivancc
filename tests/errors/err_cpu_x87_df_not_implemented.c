// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_DF_NOT_IMPLEMENTED]
// Can't run a DF x87 operation the CPU does not implement.
// Note: DF F8 is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xdf, 0xf8 # DF F8");
#endif
    return 0;
}
